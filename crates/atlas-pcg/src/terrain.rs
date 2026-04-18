//! Heightmap terrain generator.
//!
//! Generates tileable heightmaps using fbm noise, suitable for planet surfaces,
//! asteroid terrain, and zone ground planes.

use crate::noise_util::fbm;
use crate::rng::DeterministicRng;
use rayon::prelude::*;

/// Configuration for terrain generation.
#[derive(Clone, Debug)]
pub struct TerrainConfig {
    /// Width of the heightmap in samples.
    pub width:       usize,
    /// Height (depth) of the heightmap in samples.
    pub height:      usize,
    /// World-space scale of one heightmap cell.
    pub cell_size:   f32,
    /// Maximum terrain elevation (metres).
    pub max_height:  f32,
    /// Noise octaves.
    pub octaves:     usize,
    /// Base noise frequency.
    pub frequency:   f64,
    /// FBM persistence.
    pub persistence: f64,
    /// FBM lacunarity.
    pub lacunarity:  f64,
    /// RNG seed.
    pub seed:        u64,
}

impl Default for TerrainConfig {
    fn default() -> Self {
        Self {
            width:       256,
            height:      256,
            cell_size:   1.0,
            max_height:  100.0,
            octaves:     6,
            frequency:   1.0,
            persistence: 0.5,
            lacunarity:  2.0,
            seed:        42,
        }
    }
}

/// A 2-D heightmap.
pub struct HeightMap {
    pub width:  usize,
    pub height: usize,
    /// Row-major heights in `[0, max_height]`.
    pub data:   Vec<f32>,
    /// World-space cell size.
    pub cell_size: f32,
}

impl HeightMap {
    /// Height at cell `(x, z)`.
    pub fn get(&self, x: usize, z: usize) -> f32 {
        self.data[z * self.width + x]
    }

    /// Bilinearly interpolated height at world position `(wx, wz)`.
    pub fn sample(&self, wx: f32, wz: f32) -> f32 {
        let gx = (wx / self.cell_size).clamp(0.0, (self.width  - 1) as f32);
        let gz = (wz / self.cell_size).clamp(0.0, (self.height - 1) as f32);
        let x0 = gx.floor() as usize;
        let z0 = gz.floor() as usize;
        let x1 = (x0 + 1).min(self.width  - 1);
        let z1 = (z0 + 1).min(self.height - 1);
        let fx = gx - x0 as f32;
        let fz = gz - z0 as f32;
        let h00 = self.get(x0, z0);
        let h10 = self.get(x1, z0);
        let h01 = self.get(x0, z1);
        let h11 = self.get(x1, z1);
        let h0 = h00 + fx * (h10 - h00);
        let h1 = h01 + fx * (h11 - h01);
        h0 + fz * (h1 - h0)
    }

    /// Approximate surface normal at cell `(x, z)` (Sobel filter).
    pub fn normal(&self, x: usize, z: usize) -> [f32; 3] {
        let l = if x > 0 { self.get(x - 1, z) } else { self.get(x, z) };
        let r = if x + 1 < self.width  { self.get(x + 1, z) } else { self.get(x, z) };
        let d = if z > 0 { self.get(x, z - 1) } else { self.get(x, z) };
        let u = if z + 1 < self.height { self.get(x, z + 1) } else { self.get(x, z) };
        let dx = (r - l) * 0.5;
        let dz = (u - d) * 0.5;
        let len = (dx * dx + 1.0 + dz * dz).sqrt();
        [-dx / len, 1.0 / len, -dz / len]
    }
}

/// Procedural terrain generator.
pub struct TerrainGenerator {
    config: TerrainConfig,
}

impl TerrainGenerator {
    pub fn new(config: TerrainConfig) -> Self {
        Self { config }
    }

    /// Generate a heightmap from the current configuration.
    ///
    /// Uses Rayon for parallel row generation.
    pub fn generate(&self) -> HeightMap {
        let cfg = &self.config;
        let mut rng = DeterministicRng::new(cfg.seed);

        // Per-seed offset so different seeds produce different terrain.
        let offset_x = rng.next_float_range(-1000.0, 1000.0) as f64;
        let offset_z = rng.next_float_range(-1000.0, 1000.0) as f64;

        let w = cfg.width;
        let h = cfg.height;

        // Generate all rows in parallel
        let data: Vec<f32> = (0..h)
            .into_par_iter()
            .flat_map(|z| {
                (0..w).map(move |x| {
                    let nx = (x as f64 / w as f64) * cfg.frequency + offset_x;
                    let nz = (z as f64 / h as f64) * cfg.frequency + offset_z;
                    let raw = fbm(nx, nz, 0.0, cfg.octaves, 1.0, cfg.persistence, cfg.lacunarity);
                    // Map [-1, 1] → [0, max_height]
                    let normalised = (raw as f32 + 1.0) * 0.5;
                    normalised * cfg.max_height
                }).collect::<Vec<f32>>()
            })
            .collect();

        HeightMap {
            width: w,
            height: h,
            data,
            cell_size: cfg.cell_size,
        }
    }

    pub fn config(&self) -> &TerrainConfig { &self.config }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn heightmap_in_range() {
        let gen = TerrainGenerator::new(TerrainConfig {
            width: 64, height: 64, max_height: 100.0, seed: 7, ..Default::default()
        });
        let hm = gen.generate();
        for &v in &hm.data {
            assert!(v.is_finite(), "non-finite height");
            assert!(v >= 0.0 && v <= 100.0, "height {} out of range", v);
        }
    }

    #[test]
    fn heightmap_deterministic() {
        let make = || TerrainGenerator::new(TerrainConfig { width: 32, height: 32, seed: 99, ..Default::default() }).generate();
        let a = make();
        let b = make();
        assert_eq!(a.data, b.data);
    }

    #[test]
    fn sample_interpolation() {
        let gen = TerrainGenerator::new(TerrainConfig { width: 16, height: 16, cell_size: 1.0, seed: 1, ..Default::default() });
        let hm = gen.generate();
        // sample at exact grid point should equal get()
        let h = hm.get(3, 5);
        let s = hm.sample(3.0, 5.0);
        assert!((h - s).abs() < 1e-4);
    }
}
