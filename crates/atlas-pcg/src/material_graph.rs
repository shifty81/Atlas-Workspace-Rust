//! Procedural material node graph.
//!
//! Analogous to the C++ `atlas::procedural::ProceduralMaterialGraph`.
//! Nodes are evaluated in topological order to produce material parameters
//! (base colour, roughness, metallic, normal offset).

use std::collections::HashMap;

/// Node type in the material graph.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum MaterialNodeType {
    /// Constant colour or scalar value.
    Constant,
    /// Read from a texture or noise pattern.
    Sample,
    /// Blend two inputs.
    Blend,
    /// Apply a math operation (add, multiply, power, etc.).
    Math,
    /// Final output (base_color, roughness, metallic, normal).
    Output,
}

/// A single material parameter set produced by an Output node.
#[derive(Clone, Debug, Default)]
pub struct MaterialOutput {
    pub base_color: [f32; 4],
    pub roughness:  f32,
    pub metallic:   f32,
    pub emissive:   [f32; 3],
    pub normal:     [f32; 3],
}

/// A node in the material graph.
#[derive(Clone, Debug)]
pub struct MaterialNode {
    pub id:         u32,
    pub node_type:  MaterialNodeType,
    pub properties: Vec<(String, String)>,
}

impl MaterialNode {
    pub fn get_property(&self, key: &str) -> Option<&str> {
        self.properties.iter().find(|(k, _)| k == key).map(|(_, v)| v.as_str())
    }
}

/// Procedural material node graph.
pub struct MaterialGraph {
    next_id:  u32,
    nodes:    HashMap<u32, MaterialNode>,
    edges:    Vec<(u32, u32)>, // (from, to)
    output:   Option<MaterialOutput>,
    compiled: bool,
}

impl MaterialGraph {
    pub fn new() -> Self {
        Self {
            next_id:  1,
            nodes:    HashMap::new(),
            edges:    Vec::new(),
            output:   None,
            compiled: false,
        }
    }

    pub fn add_node(&mut self, node_type: MaterialNodeType) -> u32 {
        let id = self.next_id;
        self.next_id += 1;
        self.nodes.insert(id, MaterialNode { id, node_type, properties: Vec::new() });
        self.compiled = false;
        id
    }

    pub fn set_property(&mut self, id: u32, key: impl Into<String>, value: impl Into<String>) {
        if let Some(node) = self.nodes.get_mut(&id) {
            let key = key.into();
            if let Some(e) = node.properties.iter_mut().find(|(k, _)| k == &key) {
                e.1 = value.into();
            } else {
                node.properties.push((key, value.into()));
            }
        }
        self.compiled = false;
    }

    pub fn connect(&mut self, from: u32, to: u32) {
        self.edges.push((from, to));
        self.compiled = false;
    }

    /// Compile and evaluate the graph.  Returns the material output or `None`
    /// if there is no Output node.
    pub fn evaluate(&mut self) -> Option<&MaterialOutput> {
        // Find Output node
        let output_id = self.nodes.values()
            .find(|n| n.node_type == MaterialNodeType::Output)?
            .id;

        let output_node = self.nodes.get(&output_id)?.clone();
        let out = self.evaluate_output(&output_node);
        self.output = Some(out);
        self.compiled = true;
        self.output.as_ref()
    }

    pub fn get_output(&self) -> Option<&MaterialOutput> {
        self.output.as_ref()
    }

    pub fn node_count(&self) -> usize { self.nodes.len() }
    pub fn is_compiled(&self) -> bool { self.compiled }

    fn evaluate_output(&self, node: &MaterialNode) -> MaterialOutput {
        // Collect upstream nodes connected to this output
        let upstream: Vec<&MaterialNode> = self.edges.iter()
            .filter(|(_, to)| *to == node.id)
            .filter_map(|(from, _)| self.nodes.get(from))
            .collect();

        let mut out = MaterialOutput::default();
        // Default values
        out.base_color = [0.8, 0.8, 0.8, 1.0];
        out.roughness  = 0.5;
        out.metallic   = 0.0;
        out.normal     = [0.0, 1.0, 0.0];

        for up in upstream {
            match up.node_type {
                MaterialNodeType::Constant => {
                    if let Some(r) = up.get_property("r").and_then(|v| v.parse().ok()) {
                        out.base_color[0] = r;
                    }
                    if let Some(g) = up.get_property("g").and_then(|v| v.parse().ok()) {
                        out.base_color[1] = g;
                    }
                    if let Some(b) = up.get_property("b").and_then(|v| v.parse().ok()) {
                        out.base_color[2] = b;
                    }
                    if let Some(rough) = up.get_property("roughness").and_then(|v| v.parse().ok()) {
                        out.roughness = rough;
                    }
                    if let Some(metal) = up.get_property("metallic").and_then(|v| v.parse().ok()) {
                        out.metallic = metal;
                    }
                }
                _ => {}
            }
        }
        out
    }
}

impl Default for MaterialGraph {
    fn default() -> Self { Self::new() }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn constant_output() {
        let mut g = MaterialGraph::new();
        let const_node = g.add_node(MaterialNodeType::Constant);
        g.set_property(const_node, "r", "0.2");
        g.set_property(const_node, "g", "0.5");
        g.set_property(const_node, "b", "0.8");
        g.set_property(const_node, "roughness", "0.3");
        let out_node = g.add_node(MaterialNodeType::Output);
        g.connect(const_node, out_node);

        let out = g.evaluate().unwrap();
        assert!((out.base_color[0] - 0.2).abs() < 1e-4);
        assert!((out.roughness - 0.3).abs() < 1e-4);
    }

    #[test]
    fn no_output_node_returns_none() {
        let mut g = MaterialGraph::new();
        g.add_node(MaterialNodeType::Constant);
        assert!(g.evaluate().is_none());
    }
}
