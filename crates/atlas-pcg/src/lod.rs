//! LOD baking graph.
//!
//! Represents the LOD (level-of-detail) baking pipeline used to pre-compute
//! multiple mesh LODs from a high-detail procedural mesh.

use crate::mesh_graph::MeshData;

/// A single LOD level.
#[derive(Clone, Debug)]
pub struct LodNode {
    pub level:           u32,
    pub mesh:            Option<MeshData>,
    pub screen_coverage: f32, // fraction of screen at which this LOD activates
}

/// Graph that bakes a chain of decreasing-detail LOD meshes from a source.
pub struct LodBakingGraph {
    source: Option<MeshData>,
    nodes:  Vec<LodNode>,
}

impl LodBakingGraph {
    pub fn new() -> Self {
        Self { source: None, nodes: Vec::new() }
    }

    /// Set the high-detail source mesh.
    pub fn set_source(&mut self, mesh: MeshData) {
        self.source = Some(mesh);
    }

    /// Add an LOD level with the given screen-coverage threshold.
    pub fn add_lod(&mut self, screen_coverage: f32) -> u32 {
        let level = self.nodes.len() as u32;
        self.nodes.push(LodNode { level, mesh: None, screen_coverage });
        level
    }

    /// Bake all LOD levels.  Each level halves the triangle count of the
    /// previous by removing every other triangle pair.
    pub fn bake(&mut self) {
        let mut current = match &self.source {
            Some(m) => m.clone(),
            None    => return,
        };

        for node in &mut self.nodes {
            node.mesh = Some(current.clone());
            // Decimate: keep every other triangle
            current = decimate(&current);
        }
    }

    /// Return the mesh at a given LOD level, or `None`.
    pub fn get_lod(&self, level: u32) -> Option<&MeshData> {
        self.nodes.iter()
            .find(|n| n.level == level)
            .and_then(|n| n.mesh.as_ref())
    }

    pub fn lod_count(&self) -> usize { self.nodes.len() }
}

impl Default for LodBakingGraph {
    fn default() -> Self { Self::new() }
}

fn decimate(mesh: &MeshData) -> MeshData {
    if mesh.indices.len() < 6 {
        return mesh.clone();
    }
    // Simple decimation: keep every other triangle
    let indices: Vec<u32> = mesh.indices
        .chunks(6)
        .flat_map(|chunk| {
            if chunk.len() >= 3 { &chunk[..3] } else { chunk }
        })
        .copied()
        .collect();
    MeshData {
        vertices: mesh.vertices.clone(),
        normals:  mesh.normals.clone(),
        indices,
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::mesh_graph::generate_box_test_mesh;

    #[test]
    fn bake_produces_lods() {
        let src = generate_box_test_mesh();
        let base_tris = src.triangle_count();
        let mut graph = LodBakingGraph::new();
        graph.set_source(src);
        graph.add_lod(1.0);
        graph.add_lod(0.5);
        graph.add_lod(0.1);
        graph.bake();

        let lod0 = graph.get_lod(0).unwrap();
        assert_eq!(lod0.triangle_count(), base_tris);

        let lod1 = graph.get_lod(1).unwrap();
        assert!(lod1.triangle_count() <= base_tris);
    }
}
