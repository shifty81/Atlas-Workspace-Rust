//! Terrain mesh generation from PCG heightmap data.
//!
//! [`TerrainMesh`] bridges `atlas-pcg`'s `TerrainGenerator` output to the
//! GPU upload path in `atlas-renderer`.  On headless / non-Vulkan builds the
//! mesh is stored as CPU-side vertex/index data that can still be inspected
//! in tests without a GPU.
//!
//! ## Coordinate convention
//!
//! - X axis: column (East)
//! - Y axis: height (Up)
//! - Z axis: row (South)
//! - Cell spacing: `cell_size` metres
//!
//! ## Usage
//!
//! ```rust
//! use atlas_renderer::terrain_mesh::TerrainMesh;
//!
//! // 4×4 heightmap with 1.0 m cell size
//! let heights: Vec<f32> = (0..16).map(|i| i as f32 * 0.5).collect();
//! let mesh = TerrainMesh::from_heightmap(&heights, 4, 4, 1.0);
//!
//! assert_eq!(mesh.vertex_count(), 16);
//! assert!(mesh.index_count() > 0);
//! assert!(mesh.is_valid());
//! ```

// ── TerrainVertex ────────────────────────────────────────────────────────────

/// A single terrain vertex: position + packed normal + UV.
#[derive(Debug, Clone, Copy, PartialEq)]
#[repr(C)]
pub struct TerrainVertex {
    /// World-space position (X=East, Y=Up, Z=South).
    pub position: [f32; 3],
    /// Surface normal (unit vector).
    pub normal:   [f32; 3],
    /// UV texture coordinates (0..1 across the whole terrain patch).
    pub uv:       [f32; 2],
}

// ── TerrainMesh ─────────────────────────────────────────────────────────────

/// CPU-side terrain mesh produced from a flat heightmap array.
///
/// Call [`TerrainMesh::from_heightmap`] to build the mesh, then upload it
/// to the GPU via the `buffer.rs` helpers when a Vulkan device is available.
#[derive(Debug, Clone)]
pub struct TerrainMesh {
    /// Interleaved vertex data (position, normal, UV).
    pub vertices: Vec<TerrainVertex>,
    /// Triangle list indices (every 3 values = one triangle).
    pub indices:  Vec<u32>,
    /// Width of the source heightmap in samples.
    pub width:    u32,
    /// Height (depth / row count) of the source heightmap in samples.
    pub height:   u32,
    /// World-space cell size in metres.
    pub cell_size: f32,
}

impl TerrainMesh {
    /// Build a [`TerrainMesh`] from a flat heightmap array.
    ///
    /// # Parameters
    ///
    /// * `heights`   — row-major array of `width * height` height values (metres).
    /// * `width`     — number of samples along the X axis.
    /// * `height`    — number of samples along the Z axis.
    /// * `cell_size` — world-space distance between adjacent samples (metres).
    ///
    /// # Panics
    ///
    /// Panics if `heights.len() != width * height` or if `width < 2` or `height < 2`.
    pub fn from_heightmap(
        heights:   &[f32],
        width:     u32,
        height:    u32,
        cell_size: f32,
    ) -> Self {
        assert_eq!(
            heights.len(),
            (width * height) as usize,
            "heights array length must equal width * height"
        );
        assert!(width >= 2,  "width must be >= 2 to form triangles");
        assert!(height >= 2, "height must be >= 2 to form triangles");

        let w = width  as usize;
        let h = height as usize;

        // ── Build vertices ───────────────────────────────────────────────
        let mut vertices = Vec::with_capacity(w * h);

        for row in 0..h {
            for col in 0..w {
                let idx = row * w + col;
                let y   = heights[idx];
                let x   = col as f32 * cell_size;
                let z   = row as f32 * cell_size;

                // Finite-difference surface normal
                let normal = compute_normal(heights, col, row, w, h, cell_size);

                let u = col as f32 / (w - 1) as f32;
                let v = row as f32 / (h - 1) as f32;

                vertices.push(TerrainVertex {
                    position: [x, y, z],
                    normal,
                    uv: [u, v],
                });
            }
        }

        // ── Build indices (triangle strip quads) ─────────────────────────
        // Each quad (col, row) → (col+1, row) → (col, row+1) → (col+1, row+1)
        // Split into two CCW triangles:
        //   (row, col)      (row, col+1)    (row+1, col)
        //   (row+1, col+1)  (row+1, col)    (row, col+1)
        let quad_count = (w - 1) * (h - 1);
        let mut indices = Vec::with_capacity(quad_count * 6);

        for row in 0..(h - 1) {
            for col in 0..(w - 1) {
                let tl = (row * w + col)         as u32;
                let tr = (row * w + col + 1)     as u32;
                let bl = ((row + 1) * w + col)   as u32;
                let br = ((row + 1) * w + col + 1) as u32;

                // Triangle 1: tl → bl → tr
                indices.push(tl);
                indices.push(bl);
                indices.push(tr);
                // Triangle 2: tr → bl → br
                indices.push(tr);
                indices.push(bl);
                indices.push(br);
            }
        }

        Self { vertices, indices, width, height, cell_size }
    }

    /// Number of vertices in the mesh.
    pub fn vertex_count(&self) -> usize { self.vertices.len() }

    /// Number of index values (3 per triangle).
    pub fn index_count(&self) -> usize { self.indices.len() }

    /// Number of triangles in the mesh.
    pub fn triangle_count(&self) -> usize { self.indices.len() / 3 }

    /// Returns `true` if the mesh has at least one triangle and all index values
    /// are within the vertex buffer bounds.
    pub fn is_valid(&self) -> bool {
        if self.vertices.is_empty() || self.indices.is_empty() {
            return false;
        }
        let vcount = self.vertices.len() as u32;
        self.indices.iter().all(|&i| i < vcount)
    }

    /// Return the AABB of the mesh as `(min, max)` triples `[x, y, z]`.
    pub fn aabb(&self) -> ([f32; 3], [f32; 3]) {
        let mut min = [f32::INFINITY;    3];
        let mut max = [f32::NEG_INFINITY; 3];
        for v in &self.vertices {
            for i in 0..3 {
                min[i] = min[i].min(v.position[i]);
                max[i] = max[i].max(v.position[i]);
            }
        }
        (min, max)
    }
}

// ── Normal computation ───────────────────────────────────────────────────────

/// Compute the surface normal at `(col, row)` using central finite differences.
///
/// At the boundary, one-sided differences are used instead of central ones.
fn compute_normal(
    heights:   &[f32],
    col:       usize,
    row:       usize,
    w:         usize,
    h:         usize,
    cell_size: f32,
) -> [f32; 3] {
    let sample = |c: usize, r: usize| heights[r * w + c];

    // dY/dX
    let (hl, hr) = if col == 0 {
        (sample(col, row), sample(col + 1, row))
    } else if col == w - 1 {
        (sample(col - 1, row), sample(col, row))
    } else {
        (sample(col - 1, row), sample(col + 1, row))
    };
    let dydx = (hr - hl) / (2.0 * cell_size);

    // dY/dZ
    let (ht, hb) = if row == 0 {
        (sample(col, row), sample(col, row + 1))
    } else if row == h - 1 {
        (sample(col, row - 1), sample(col, row))
    } else {
        (sample(col, row - 1), sample(col, row + 1))
    };
    let dydz = (hb - ht) / (2.0 * cell_size);

    // Cross product of tangent vectors: T_x = (1, dydx, 0), T_z = (0, dydz, 1)
    // N = T_x × T_z = (-dydx, 1, -dydz), then normalise
    let nx = -dydx;
    let ny = 1.0_f32;
    let nz = -dydz;
    let len = (nx * nx + ny * ny + nz * nz).sqrt();
    if len < 1e-8 {
        [0.0, 1.0, 0.0]
    } else {
        [nx / len, ny / len, nz / len]
    }
}

// ── Tests ────────────────────────────────────────────────────────────────────

#[cfg(test)]
mod tests {
    use super::*;

    fn flat_heightmap(w: u32, h: u32, y: f32) -> Vec<f32> {
        vec![y; (w * h) as usize]
    }

    fn ramp_heightmap(w: u32, h: u32) -> Vec<f32> {
        (0..(w * h) as usize).map(|i| i as f32 * 0.1).collect()
    }

    #[test]
    fn flat_mesh_vertex_and_index_count() {
        let heights = flat_heightmap(4, 4, 0.0);
        let mesh = TerrainMesh::from_heightmap(&heights, 4, 4, 1.0);
        assert_eq!(mesh.vertex_count(), 16);
        // 3×3 quads × 2 triangles × 3 indices = 54
        assert_eq!(mesh.index_count(), 54);
        assert_eq!(mesh.triangle_count(), 18);
    }

    #[test]
    fn mesh_is_valid() {
        let heights = ramp_heightmap(8, 8);
        let mesh = TerrainMesh::from_heightmap(&heights, 8, 8, 2.5);
        assert!(mesh.is_valid());
    }

    #[test]
    fn flat_mesh_normals_point_up() {
        let heights = flat_heightmap(4, 4, 5.0);
        let mesh = TerrainMesh::from_heightmap(&heights, 4, 4, 1.0);
        for v in &mesh.vertices {
            let [nx, ny, nz] = v.normal;
            assert!((nx).abs() < 1e-5, "nx should be ~0");
            assert!((nz).abs() < 1e-5, "nz should be ~0");
            assert!((ny - 1.0).abs() < 1e-5, "ny should be ~1");
        }
    }

    #[test]
    fn uv_corners() {
        let heights = flat_heightmap(3, 3, 0.0);
        let mesh = TerrainMesh::from_heightmap(&heights, 3, 3, 1.0);
        // Top-left corner (col=0, row=0) → UV (0, 0)
        assert_eq!(mesh.vertices[0].uv, [0.0, 0.0]);
        // Bottom-right corner (col=2, row=2) → UV (1, 1)
        assert_eq!(mesh.vertices[8].uv, [1.0, 1.0]);
    }

    #[test]
    fn aabb_correct() {
        let heights = flat_heightmap(4, 4, 3.0);
        let mesh = TerrainMesh::from_heightmap(&heights, 4, 4, 2.0);
        let (min, max) = mesh.aabb();
        assert!((min[1] - 3.0).abs() < 1e-5);
        assert!((max[1] - 3.0).abs() < 1e-5);
        assert!((min[0]).abs() < 1e-5);
        assert!((max[0] - 6.0).abs() < 1e-5); // 3 cells × 2.0 m
    }

    #[test]
    fn ramp_mesh_positive_heights() {
        let heights = ramp_heightmap(5, 5);
        let mesh = TerrainMesh::from_heightmap(&heights, 5, 5, 1.0);
        assert!(mesh.is_valid());
        let (_, max) = mesh.aabb();
        assert!(max[1] > 0.0);
    }

    #[test]
    fn cell_size_scales_positions() {
        let heights = flat_heightmap(3, 3, 0.0);
        let mesh = TerrainMesh::from_heightmap(&heights, 3, 3, 10.0);
        // Col=2, row=0 → X should be 20.0
        assert!((mesh.vertices[2].position[0] - 20.0).abs() < 1e-5);
    }

    #[test]
    #[should_panic(expected = "heights array length must equal width * height")]
    fn panics_on_wrong_length() {
        let heights = vec![0.0f32; 10]; // wrong size for 4×4
        TerrainMesh::from_heightmap(&heights, 4, 4, 1.0);
    }

    #[test]
    #[should_panic(expected = "width must be >= 2")]
    fn panics_on_width_one() {
        let heights = vec![0.0f32; 3];
        TerrainMesh::from_heightmap(&heights, 1, 3, 1.0);
    }
}
