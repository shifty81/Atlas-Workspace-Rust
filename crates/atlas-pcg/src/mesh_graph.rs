//! Procedural mesh node graph.
//!
//! Rust port of the C++ `atlas::procedural::ProceduralMeshGraph`.
//! Nodes are evaluated in topological order (no cycles); each node
//! transforms or combines mesh data.

use std::collections::HashMap;

/// Triangle mesh data (interleaved vertices + indexed triangles).
#[derive(Clone, Debug, Default)]
pub struct MeshData {
    /// Interleaved XYZ vertex positions (length is a multiple of 3).
    pub vertices: Vec<f32>,
    /// Interleaved XYZ vertex normals (same length as `vertices`).
    pub normals: Vec<f32>,
    /// Triangle indices (length is a multiple of 3).
    pub indices: Vec<u32>,
}

impl MeshData {
    pub fn vertex_count(&self) -> usize { self.vertices.len() / 3 }
    pub fn triangle_count(&self) -> usize { self.indices.len() / 3 }

    pub fn clear(&mut self) {
        self.vertices.clear();
        self.normals.clear();
        self.indices.clear();
    }

    pub fn is_valid(&self) -> bool {
        self.vertices.len() % 3 == 0
            && self.normals.len() == self.vertices.len()
            && self.indices.len() % 3 == 0
    }
}

/// Node type in the procedural mesh graph.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
#[repr(u8)]
pub enum MeshNodeType {
    Primitive,
    Transform,
    Merge,
    Subdivide,
    Noise,
    Output,
}

/// A single node in the mesh graph.
#[derive(Clone, Debug)]
pub struct MeshNode {
    pub id:         u32,
    pub node_type:  MeshNodeType,
    pub properties: Vec<(String, String)>,
}

impl MeshNode {
    pub fn get_property(&self, key: &str) -> Option<&str> {
        self.properties.iter().find(|(k, _)| k == key).map(|(_, v)| v.as_str())
    }

    pub fn get_property_or<'a>(&'a self, key: &str, default: &'a str) -> &'a str {
        self.get_property(key).unwrap_or(default)
    }
}

/// A directed edge in the mesh graph.
#[derive(Clone, Debug)]
pub struct MeshEdge {
    pub from_node: u32,
    pub from_port: u16,
    pub to_node:   u32,
    pub to_port:   u16,
}

/// Procedural mesh node graph.
///
/// Nodes are compiled into topological execution order and then evaluated to
/// produce the final [`MeshData`] output.
pub struct MeshGraph {
    next_id:         u32,
    nodes:           HashMap<u32, MeshNode>,
    edges:           Vec<MeshEdge>,
    execution_order: Vec<u32>,
    outputs:         HashMap<u64, MeshData>, // key = (node_id << 16 | port)
    compiled:        bool,
}

impl MeshGraph {
    pub fn new() -> Self {
        Self {
            next_id:         1,
            nodes:           HashMap::new(),
            edges:           Vec::new(),
            execution_order: Vec::new(),
            outputs:         HashMap::new(),
            compiled:        false,
        }
    }

    /// Add a node and return its ID.
    pub fn add_node(&mut self, node_type: MeshNodeType) -> u32 {
        let id = self.next_id;
        self.next_id += 1;
        self.nodes.insert(id, MeshNode { id, node_type, properties: Vec::new() });
        self.compiled = false;
        id
    }

    /// Set a property on a node.
    pub fn set_node_property(&mut self, id: u32, key: impl Into<String>, value: impl Into<String>) {
        if let Some(node) = self.nodes.get_mut(&id) {
            let key = key.into();
            if let Some(entry) = node.properties.iter_mut().find(|(k, _)| k == &key) {
                entry.1 = value.into();
            } else {
                node.properties.push((key, value.into()));
            }
        }
        self.compiled = false;
    }

    /// Add a directed edge.
    pub fn add_edge(&mut self, edge: MeshEdge) {
        self.edges.push(edge);
        self.compiled = false;
    }

    /// Remove a node and all edges connected to it.
    pub fn remove_node(&mut self, id: u32) {
        self.nodes.remove(&id);
        self.edges.retain(|e| e.from_node != id && e.to_node != id);
        self.compiled = false;
    }

    /// Compile the graph: check for cycles and build the execution order.
    /// Returns `false` if a cycle is detected.
    pub fn compile(&mut self) -> bool {
        if self.has_cycle() {
            return false;
        }
        // Topological sort (Kahn's algorithm) — O(V + E)
        use std::collections::VecDeque;
        let mut in_degree: HashMap<u32, usize> = self.nodes.keys().map(|&id| (id, 0)).collect();
        for edge in &self.edges {
            *in_degree.entry(edge.to_node).or_insert(0) += 1;
        }
        // Collect zero-in-degree nodes into a sorted deque for determinism.
        let mut ready: Vec<u32> = in_degree.iter()
            .filter_map(|(&id, &deg)| if deg == 0 { Some(id) } else { None })
            .collect();
        ready.sort_unstable();
        let mut queue: VecDeque<u32> = ready.into();

        let mut order = Vec::new();
        while let Some(id) = queue.pop_front() {
            order.push(id);
            let mut successors: Vec<u32> = self.edges.iter()
                .filter(|e| e.from_node == id)
                .map(|e| e.to_node)
                .collect();
            successors.sort_unstable(); // deterministic successor ordering
            for succ in successors {
                let deg = in_degree.get_mut(&succ).unwrap();
                *deg -= 1;
                if *deg == 0 {
                    queue.push_back(succ);
                }
            }
        }
        self.execution_order = order;
        self.compiled = true;
        true
    }

    /// Execute the compiled graph.  Returns `false` if not compiled.
    pub fn execute(&mut self) -> bool {
        if !self.compiled {
            return false;
        }
        self.outputs.clear();
        let order = self.execution_order.clone();
        for id in &order {
            if let Some(node) = self.nodes.get(id).cloned() {
                self.execute_node(&node);
            }
        }
        true
    }

    /// Return the output mesh of the `Output` node (if any).
    pub fn get_output(&self) -> Option<&MeshData> {
        let output_id = self.nodes.values()
            .find(|n| n.node_type == MeshNodeType::Output)?
            .id;
        let key = (output_id as u64) << 16;
        self.outputs.get(&key)
    }

    pub fn node_count(&self) -> usize { self.nodes.len() }
    pub fn is_compiled(&self) -> bool { self.compiled }

    // ── Private ──────────────────────────────────────────────────────────

    fn has_cycle(&self) -> bool {
        let mut visited = std::collections::HashSet::new();
        let mut rec_stack = std::collections::HashSet::new();
        let ids: Vec<u32> = self.nodes.keys().copied().collect();
        for &id in &ids {
            if self.dfs_cycle(id, &mut visited, &mut rec_stack) {
                return true;
            }
        }
        false
    }

    fn dfs_cycle(
        &self,
        id: u32,
        visited: &mut std::collections::HashSet<u32>,
        rec_stack: &mut std::collections::HashSet<u32>,
    ) -> bool {
        if rec_stack.contains(&id) { return true; }
        if visited.contains(&id)   { return false; }
        visited.insert(id);
        rec_stack.insert(id);
        for edge in &self.edges {
            if edge.from_node == id && self.dfs_cycle(edge.to_node, visited, rec_stack) {
                return true;
            }
        }
        rec_stack.remove(&id);
        false
    }

    fn execute_node(&mut self, node: &MeshNode) {
        let key = (node.id as u64) << 16;
        let mesh = match node.node_type {
            MeshNodeType::Primitive => self.gen_primitive(node),
            MeshNodeType::Merge     => self.gen_merge(node),
            MeshNodeType::Subdivide => self.gen_subdivide(node),
            MeshNodeType::Noise     => self.gen_noise(node),
            MeshNodeType::Transform => self.gen_transform(node),
            MeshNodeType::Output    => self.gen_output(node),
        };
        self.outputs.insert(key, mesh);
    }

    fn gen_primitive(&self, node: &MeshNode) -> MeshData {
        let kind = node.get_property_or("shape", "box");
        match kind {
            "sphere" => generate_uv_sphere(
                node.get_property("radius").and_then(|v| v.parse().ok()).unwrap_or(1.0),
                node.get_property("stacks").and_then(|v| v.parse().ok()).unwrap_or(8),
                node.get_property("slices").and_then(|v| v.parse().ok()).unwrap_or(8),
            ),
            _ => generate_box(
                node.get_property("width").and_then(|v| v.parse().ok()).unwrap_or(1.0),
                node.get_property("height").and_then(|v| v.parse().ok()).unwrap_or(1.0),
                node.get_property("depth").and_then(|v| v.parse().ok()).unwrap_or(1.0),
            ),
        }
    }

    fn gen_merge(&self, node: &MeshNode) -> MeshData {
        // Collect all inputs into this node and concatenate
        let inputs: Vec<u64> = self.edges.iter()
            .filter(|e| e.to_node == node.id)
            .map(|e| (e.from_node as u64) << 16 | e.from_port as u64)
            .collect();

        let mut merged = MeshData::default();
        for key in inputs {
            if let Some(src) = self.outputs.get(&key) {
                let base = merged.vertex_count() as u32;
                merged.vertices.extend_from_slice(&src.vertices);
                merged.normals.extend_from_slice(&src.normals);
                merged.indices.extend(src.indices.iter().map(|&i| i + base));
            }
        }
        merged
    }

    fn gen_subdivide(&self, node: &MeshNode) -> MeshData {
        let src_key = self.edges.iter()
            .find(|e| e.to_node == node.id)
            .map(|e| (e.from_node as u64) << 16 | e.from_port as u64);

        let mesh = src_key.and_then(|k| self.outputs.get(&k)).cloned().unwrap_or_default();
        let levels: u32 = node.get_property("levels").and_then(|v| v.parse().ok()).unwrap_or(1);
        subdivide_mesh(mesh, levels)
    }

    fn gen_noise(&self, node: &MeshNode) -> MeshData {
        let src_key = self.edges.iter()
            .find(|e| e.to_node == node.id)
            .map(|e| (e.from_node as u64) << 16 | e.from_port as u64);

        let mut mesh = src_key.and_then(|k| self.outputs.get(&k)).cloned().unwrap_or_default();
        let scale: f32 = node.get_property("scale").and_then(|v| v.parse().ok()).unwrap_or(0.1);

        // Displace vertices along their normal direction using simple noise.
        let n = mesh.vertex_count();
        for i in 0..n {
            let x = mesh.vertices[i * 3];
            let y = mesh.vertices[i * 3 + 1];
            let z = mesh.vertices[i * 3 + 2];
            let disp = simple_value_noise(x, y, z) * scale;
            let nx = mesh.normals[i * 3];
            let ny = mesh.normals[i * 3 + 1];
            let nz = mesh.normals[i * 3 + 2];
            mesh.vertices[i * 3]     += nx * disp;
            mesh.vertices[i * 3 + 1] += ny * disp;
            mesh.vertices[i * 3 + 2] += nz * disp;
        }
        mesh
    }

    fn gen_transform(&self, node: &MeshNode) -> MeshData {
        let src_key = self.edges.iter()
            .find(|e| e.to_node == node.id)
            .map(|e| (e.from_node as u64) << 16 | e.from_port as u64);

        let mut mesh = src_key.and_then(|k| self.outputs.get(&k)).cloned().unwrap_or_default();
        let sx: f32 = node.get_property("scale_x").and_then(|v| v.parse().ok()).unwrap_or(1.0);
        let sy: f32 = node.get_property("scale_y").and_then(|v| v.parse().ok()).unwrap_or(1.0);
        let sz: f32 = node.get_property("scale_z").and_then(|v| v.parse().ok()).unwrap_or(1.0);
        let tx: f32 = node.get_property("tx").and_then(|v| v.parse().ok()).unwrap_or(0.0);
        let ty: f32 = node.get_property("ty").and_then(|v| v.parse().ok()).unwrap_or(0.0);
        let tz: f32 = node.get_property("tz").and_then(|v| v.parse().ok()).unwrap_or(0.0);

        let n = mesh.vertex_count();
        for i in 0..n {
            mesh.vertices[i * 3]     = mesh.vertices[i * 3] * sx + tx;
            mesh.vertices[i * 3 + 1] = mesh.vertices[i * 3 + 1] * sy + ty;
            mesh.vertices[i * 3 + 2] = mesh.vertices[i * 3 + 2] * sz + tz;
        }
        mesh
    }

    fn gen_output(&self, node: &MeshNode) -> MeshData {
        // Passthrough: gather merged input
        self.gen_merge(node)
    }
}

impl Default for MeshGraph {
    fn default() -> Self { Self::new() }
}

// ── Mesh generation helpers ───────────────────────────────────────────────────

fn generate_box(w: f32, h: f32, d: f32) -> MeshData {
    let hw = w * 0.5;
    let hh = h * 0.5;
    let hd = d * 0.5;

    #[rustfmt::skip]
    let verts: &[f32] = &[
        // Front (+Z)
        -hw, -hh,  hd,   hw, -hh,  hd,   hw,  hh,  hd,  -hw,  hh,  hd,
        // Back  (-Z)
         hw, -hh, -hd,  -hw, -hh, -hd,  -hw,  hh, -hd,   hw,  hh, -hd,
        // Left  (-X)
        -hw, -hh, -hd,  -hw, -hh,  hd,  -hw,  hh,  hd,  -hw,  hh, -hd,
        // Right (+X)
         hw, -hh,  hd,   hw, -hh, -hd,   hw,  hh, -hd,   hw,  hh,  hd,
        // Top   (+Y)
        -hw,  hh,  hd,   hw,  hh,  hd,   hw,  hh, -hd,  -hw,  hh, -hd,
        // Bottom(-Y)
        -hw, -hh, -hd,   hw, -hh, -hd,   hw, -hh,  hd,  -hw, -hh,  hd,
    ];

    #[rustfmt::skip]
    let normals: &[f32] = &[
         0.0,  0.0,  1.0,  0.0,  0.0,  1.0,  0.0,  0.0,  1.0,  0.0,  0.0,  1.0,
         0.0,  0.0, -1.0,  0.0,  0.0, -1.0,  0.0,  0.0, -1.0,  0.0,  0.0, -1.0,
        -1.0,  0.0,  0.0, -1.0,  0.0,  0.0, -1.0,  0.0,  0.0, -1.0,  0.0,  0.0,
         1.0,  0.0,  0.0,  1.0,  0.0,  0.0,  1.0,  0.0,  0.0,  1.0,  0.0,  0.0,
         0.0,  1.0,  0.0,  0.0,  1.0,  0.0,  0.0,  1.0,  0.0,  0.0,  1.0,  0.0,
         0.0, -1.0,  0.0,  0.0, -1.0,  0.0,  0.0, -1.0,  0.0,  0.0, -1.0,  0.0,
    ];

    // Two triangles per face × 6 faces
    let indices: Vec<u32> = (0..6_u32).flat_map(|face| {
        let b = face * 4;
        [b, b + 1, b + 2, b, b + 2, b + 3]
    }).collect();

    MeshData {
        vertices: verts.to_vec(),
        normals:  normals.to_vec(),
        indices,
    }
}

fn generate_uv_sphere(radius: f32, stacks: u32, slices: u32) -> MeshData {
    use std::f32::consts::PI;

    let mut vertices = Vec::new();
    let mut normals  = Vec::new();
    let mut indices  = Vec::new();

    for stack in 0..=stacks {
        let phi = PI * stack as f32 / stacks as f32;
        let (sin_phi, cos_phi) = phi.sin_cos();
        for slice in 0..=slices {
            let theta = 2.0 * PI * slice as f32 / slices as f32;
            let (sin_theta, cos_theta) = theta.sin_cos();
            let x = sin_phi * cos_theta;
            let y = cos_phi;
            let z = sin_phi * sin_theta;
            vertices.extend_from_slice(&[x * radius, y * radius, z * radius]);
            normals.extend_from_slice(&[x, y, z]);
        }
    }

    let row = slices + 1;
    for stack in 0..stacks {
        for slice in 0..slices {
            let a = stack * row + slice;
            let b = a + row;
            indices.extend_from_slice(&[a, b, a + 1, b, b + 1, a + 1]);
        }
    }

    MeshData { vertices, normals, indices }
}

fn subdivide_mesh(mesh: MeshData, levels: u32) -> MeshData {
    if levels == 0 { return mesh; }
    // Simple midpoint subdivision: each triangle → 4 triangles.
    let mut current = mesh;
    for _ in 0..levels {
        current = subdivide_once(current);
    }
    current
}

fn subdivide_once(mesh: MeshData) -> MeshData {
    let mut verts   = mesh.vertices.clone();
    let mut normals = mesh.normals.clone();
    let mut indices = Vec::new();

    let tri_count = mesh.indices.len() / 3;
    for t in 0..tri_count {
        let i0 = mesh.indices[t * 3]     as usize;
        let i1 = mesh.indices[t * 3 + 1] as usize;
        let i2 = mesh.indices[t * 3 + 2] as usize;

        let mid = |a: usize, b: usize, v: &mut Vec<f32>, n: &mut Vec<f32>| -> u32 {
            let mx = (v[a * 3] + v[b * 3]) * 0.5;
            let my = (v[a * 3 + 1] + v[b * 3 + 1]) * 0.5;
            let mz = (v[a * 3 + 2] + v[b * 3 + 2]) * 0.5;
            v.extend_from_slice(&[mx, my, mz]);
            let len = (mx * mx + my * my + mz * mz).sqrt().max(1e-9);
            n.extend_from_slice(&[mx / len, my / len, mz / len]);
            (v.len() / 3 - 1) as u32
        };

        let m01 = mid(i0, i1, &mut verts, &mut normals);
        let m12 = mid(i1, i2, &mut verts, &mut normals);
        let m20 = mid(i2, i0, &mut verts, &mut normals);

        let [i0, i1, i2] = [i0 as u32, i1 as u32, i2 as u32];
        indices.extend_from_slice(&[i0, m01, m20]);
        indices.extend_from_slice(&[i1, m12, m01]);
        indices.extend_from_slice(&[i2, m20, m12]);
        indices.extend_from_slice(&[m01, m12, m20]);
    }

    MeshData { vertices: verts, normals, indices }
}

/// Minimal value-noise implementation used by the Noise node.
fn simple_value_noise(x: f32, y: f32, z: f32) -> f32 {
    let ix = x.floor() as i64;
    let iy = y.floor() as i64;
    let iz = z.floor() as i64;
    let hash = |a: i64, b: i64, c: i64| -> f32 {
        let h = (a.wrapping_mul(374761393))
            .wrapping_add(b.wrapping_mul(668265263))
            .wrapping_add(c.wrapping_mul(2147483647));
        (h as u64 as f32) / (u64::MAX as f32)
    };
    // Trilinear blend
    let fx = x - x.floor();
    let fy = y - y.floor();
    let fz = z - z.floor();
    let c000 = hash(ix, iy, iz);
    let c100 = hash(ix + 1, iy, iz);
    let c010 = hash(ix, iy + 1, iz);
    let c110 = hash(ix + 1, iy + 1, iz);
    let c001 = hash(ix, iy, iz + 1);
    let c101 = hash(ix + 1, iy, iz + 1);
    let c011 = hash(ix, iy + 1, iz + 1);
    let c111 = hash(ix + 1, iy + 1, iz + 1);
    let x00 = c000 + fx * (c100 - c000);
    let x10 = c010 + fx * (c110 - c010);
    let x01 = c001 + fx * (c101 - c001);
    let x11 = c011 + fx * (c111 - c011);
    let y0 = x00 + fy * (x10 - x00);
    let y1 = x01 + fy * (x11 - x01);
    y0 + fz * (y1 - y0)
}

/// Convenience helper exposed to sibling modules for tests.
#[cfg(test)]
pub fn generate_box_test_mesh() -> MeshData {
    generate_box(1.0, 1.0, 1.0)
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn box_mesh_valid() {
        let mesh = generate_box(1.0, 1.0, 1.0);
        assert!(mesh.is_valid());
        assert_eq!(mesh.vertex_count(), 24); // 4 verts × 6 faces
        assert_eq!(mesh.triangle_count(), 12); // 2 tris × 6 faces
    }

    #[test]
    fn sphere_mesh_valid() {
        let mesh = generate_uv_sphere(1.0, 8, 8);
        assert!(mesh.is_valid());
    }

    #[test]
    fn graph_compile_and_execute() {
        let mut g = MeshGraph::new();
        let prim = g.add_node(MeshNodeType::Primitive);
        g.set_node_property(prim, "shape", "box");
        let out = g.add_node(MeshNodeType::Output);
        g.add_edge(MeshEdge { from_node: prim, from_port: 0, to_node: out, to_port: 0 });

        assert!(g.compile());
        assert!(g.execute());
        let mesh = g.get_output().expect("output mesh");
        assert!(mesh.is_valid());
        assert!(mesh.triangle_count() > 0);
    }

    #[test]
    fn graph_detects_cycle() {
        let mut g = MeshGraph::new();
        let a = g.add_node(MeshNodeType::Transform);
        let b = g.add_node(MeshNodeType::Transform);
        g.add_edge(MeshEdge { from_node: a, from_port: 0, to_node: b, to_port: 0 });
        g.add_edge(MeshEdge { from_node: b, from_port: 0, to_node: a, to_port: 0 });
        assert!(!g.compile());
    }

    #[test]
    fn subdivide_increases_triangle_count() {
        let mesh = generate_box(1.0, 1.0, 1.0);
        let base = mesh.triangle_count();
        let sub = subdivide_mesh(mesh, 1);
        assert_eq!(sub.triangle_count(), base * 4);
    }
}
