use std::collections::{HashMap, VecDeque};

pub type AnimNodeId = u32;
pub type AnimPortId = u16;

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
#[repr(u8)]
pub enum AnimPinType {
    Float = 0,
    Pose,
    Modifier,
    Trigger,
    Mask,
}

#[derive(Debug, Clone)]
pub struct AnimValue {
    pub pin_type: AnimPinType,
    pub data: Vec<f32>,
}

#[derive(Debug, Clone)]
pub struct AnimPort {
    pub name: String,
    pub pin_type: AnimPinType,
}

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct AnimEdge {
    pub from_node: AnimNodeId,
    pub from_port: AnimPortId,
    pub to_node: AnimNodeId,
    pub to_port: AnimPortId,
}

#[derive(Debug, Clone, Default)]
pub struct AnimContext {
    pub delta_time: f32,
    pub speed: f32,
    pub fatigue: f32,
    pub damage_level: f32,
    pub tick: u32,
}

pub trait AnimNode: Send + Sync {
    fn name(&self) -> &str;
    fn category(&self) -> &str;
    fn inputs(&self) -> Vec<AnimPort>;
    fn outputs(&self) -> Vec<AnimPort>;
    fn evaluate(&mut self, ctx: &AnimContext, inputs: &[AnimValue], outputs: &mut Vec<AnimValue>);
}

struct NodeEntry {
    id: AnimNodeId,
    node: Box<dyn AnimNode + Send + Sync>,
}

pub struct AnimationGraph {
    nodes: Vec<NodeEntry>,
    edges: Vec<AnimEdge>,
    next_id: AnimNodeId,
    execution_order: Vec<AnimNodeId>,
    compiled: bool,
    outputs: HashMap<(AnimNodeId, AnimPortId), AnimValue>,
}

impl Default for AnimationGraph {
    fn default() -> Self {
        Self::new()
    }
}

impl AnimationGraph {
    pub fn new() -> Self {
        Self {
            nodes: Vec::new(),
            edges: Vec::new(),
            next_id: 0,
            execution_order: Vec::new(),
            compiled: false,
            outputs: HashMap::new(),
        }
    }

    pub fn add_node(&mut self, node: Box<dyn AnimNode + Send + Sync>) -> AnimNodeId {
        let id = self.next_id;
        self.next_id += 1;
        self.nodes.push(NodeEntry { id, node });
        self.compiled = false;
        id
    }

    pub fn remove_node(&mut self, id: AnimNodeId) {
        self.nodes.retain(|e| e.id != id);
        self.edges.retain(|e| e.from_node != id && e.to_node != id);
        self.compiled = false;
    }

    pub fn add_edge(&mut self, edge: AnimEdge) {
        self.edges.push(edge);
        self.compiled = false;
    }

    pub fn remove_edge(&mut self, edge: &AnimEdge) {
        self.edges.retain(|e| e != edge);
        self.compiled = false;
    }

    pub fn compile(&mut self) -> bool {
        let node_ids: Vec<AnimNodeId> = self.nodes.iter().map(|e| e.id).collect();
        let mut in_degree: HashMap<AnimNodeId, usize> = node_ids.iter().map(|&id| (id, 0)).collect();
        let mut adj: HashMap<AnimNodeId, Vec<AnimNodeId>> = node_ids.iter().map(|&id| (id, Vec::new())).collect();

        for edge in &self.edges {
            if let Some(deg) = in_degree.get_mut(&edge.to_node) {
                *deg += 1;
            }
            adj.entry(edge.from_node).or_default().push(edge.to_node);
        }

        let mut queue: VecDeque<AnimNodeId> = in_degree.iter()
            .filter(|(_, &d)| d == 0)
            .map(|(&id, _)| id)
            .collect();
        let mut order = Vec::new();

        while let Some(id) = queue.pop_front() {
            order.push(id);
            if let Some(neighbors) = adj.get(&id) {
                for &next in neighbors {
                    let deg = in_degree.get_mut(&next).unwrap();
                    *deg -= 1;
                    if *deg == 0 {
                        queue.push_back(next);
                    }
                }
            }
        }

        if order.len() != node_ids.len() {
            log::error!("AnimationGraph: cycle detected");
            return false;
        }

        self.execution_order = order;
        self.compiled = true;
        true
    }

    pub fn execute(&mut self, ctx: &AnimContext) -> bool {
        if !self.compiled {
            return false;
        }
        self.outputs.clear();

        let order = self.execution_order.clone();
        for node_id in &order {
            let input_edges: Vec<AnimEdge> = self.edges.iter()
                .filter(|e| e.to_node == *node_id)
                .cloned()
                .collect();

            let node_entry = match self.nodes.iter_mut().find(|e| e.id == *node_id) {
                Some(e) => e,
                None => continue,
            };
            let num_inputs = node_entry.node.inputs().len();
            let mut inputs = vec![AnimValue { pin_type: AnimPinType::Float, data: Vec::new() }; num_inputs];
            for edge in &input_edges {
                let key = (edge.from_node, edge.from_port);
                if let Some(val) = self.outputs.get(&key) {
                    let port_idx = edge.to_port as usize;
                    if port_idx < inputs.len() {
                        inputs[port_idx] = val.clone();
                    }
                }
            }

            let num_outputs = node_entry.node.outputs().len();
            let mut node_outputs = vec![AnimValue { pin_type: AnimPinType::Float, data: Vec::new() }; num_outputs];
            node_entry.node.evaluate(ctx, &inputs, &mut node_outputs);

            for (port_idx, out_val) in node_outputs.into_iter().enumerate() {
                self.outputs.insert((*node_id, port_idx as AnimPortId), out_val);
            }
        }
        true
    }

    pub fn get_output(&self, node: AnimNodeId, port: AnimPortId) -> Option<&AnimValue> {
        self.outputs.get(&(node, port))
    }

    pub fn node_count(&self) -> usize {
        self.nodes.len()
    }

    pub fn is_compiled(&self) -> bool {
        self.compiled
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    // ── Minimal concrete node implementations ─────────────────────────────────

    /// A passthrough node: copies its single Float input to its single Float output.
    struct PassthroughNode;
    impl AnimNode for PassthroughNode {
        fn name(&self) -> &str { "Passthrough" }
        fn category(&self) -> &str { "test" }
        fn inputs(&self) -> Vec<AnimPort> {
            vec![AnimPort { name: "in".into(), pin_type: AnimPinType::Float }]
        }
        fn outputs(&self) -> Vec<AnimPort> {
            vec![AnimPort { name: "out".into(), pin_type: AnimPinType::Float }]
        }
        fn evaluate(&mut self, _ctx: &AnimContext, inputs: &[AnimValue], outputs: &mut Vec<AnimValue>) {
            if let Some(v) = inputs.first() {
                outputs[0] = v.clone();
            }
        }
    }

    /// A constant source node: always emits a fixed float value.
    struct ConstantNode(f32);
    impl AnimNode for ConstantNode {
        fn name(&self) -> &str { "Constant" }
        fn category(&self) -> &str { "test" }
        fn inputs(&self) -> Vec<AnimPort> { vec![] }
        fn outputs(&self) -> Vec<AnimPort> {
            vec![AnimPort { name: "value".into(), pin_type: AnimPinType::Float }]
        }
        fn evaluate(&mut self, _ctx: &AnimContext, _inputs: &[AnimValue], outputs: &mut Vec<AnimValue>) {
            outputs[0] = AnimValue { pin_type: AnimPinType::Float, data: vec![self.0] };
        }
    }

    /// An adder node: sums the first f32 of each input.
    struct AdderNode;
    impl AnimNode for AdderNode {
        fn name(&self) -> &str { "Adder" }
        fn category(&self) -> &str { "test" }
        fn inputs(&self) -> Vec<AnimPort> {
            vec![
                AnimPort { name: "a".into(), pin_type: AnimPinType::Float },
                AnimPort { name: "b".into(), pin_type: AnimPinType::Float },
            ]
        }
        fn outputs(&self) -> Vec<AnimPort> {
            vec![AnimPort { name: "sum".into(), pin_type: AnimPinType::Float }]
        }
        fn evaluate(&mut self, _ctx: &AnimContext, inputs: &[AnimValue], outputs: &mut Vec<AnimValue>) {
            let a = inputs.first().and_then(|v| v.data.first()).copied().unwrap_or(0.0);
            let b = inputs.get(1).and_then(|v| v.data.first()).copied().unwrap_or(0.0);
            outputs[0] = AnimValue { pin_type: AnimPinType::Float, data: vec![a + b] };
        }
    }

    fn default_ctx() -> AnimContext { AnimContext::default() }

    #[test]
    fn single_node_compile_execute() {
        let mut g = AnimationGraph::new();
        let id = g.add_node(Box::new(ConstantNode(7.0)));
        assert_eq!(g.node_count(), 1);
        assert!(!g.is_compiled());
        assert!(g.compile());
        assert!(g.is_compiled());
        assert!(g.execute(&default_ctx()));
        let out = g.get_output(id, 0).unwrap();
        assert_eq!(out.data, vec![7.0]);
    }

    #[test]
    fn chained_passthrough() {
        let mut g = AnimationGraph::new();
        let src = g.add_node(Box::new(ConstantNode(42.0)));
        let pass = g.add_node(Box::new(PassthroughNode));
        g.add_edge(AnimEdge { from_node: src, from_port: 0, to_node: pass, to_port: 0 });
        assert!(g.compile());
        assert!(g.execute(&default_ctx()));
        let out = g.get_output(pass, 0).unwrap();
        assert_eq!(out.data, vec![42.0]);
    }

    #[test]
    fn adder_node() {
        let mut g = AnimationGraph::new();
        let a = g.add_node(Box::new(ConstantNode(3.0)));
        let b = g.add_node(Box::new(ConstantNode(4.0)));
        let adder = g.add_node(Box::new(AdderNode));
        g.add_edge(AnimEdge { from_node: a, from_port: 0, to_node: adder, to_port: 0 });
        g.add_edge(AnimEdge { from_node: b, from_port: 0, to_node: adder, to_port: 1 });
        assert!(g.compile());
        g.execute(&default_ctx());
        let out = g.get_output(adder, 0).unwrap();
        assert_eq!(out.data, vec![7.0]);
    }

    #[test]
    fn remove_node_invalidates_compile() {
        let mut g = AnimationGraph::new();
        let id = g.add_node(Box::new(ConstantNode(1.0)));
        g.compile();
        assert!(g.is_compiled());
        g.remove_node(id);
        assert!(!g.is_compiled());
        assert_eq!(g.node_count(), 0);
    }

    #[test]
    fn remove_edge_invalidates_compile() {
        let mut g = AnimationGraph::new();
        let src = g.add_node(Box::new(ConstantNode(1.0)));
        let pass = g.add_node(Box::new(PassthroughNode));
        let edge = AnimEdge { from_node: src, from_port: 0, to_node: pass, to_port: 0 };
        g.add_edge(edge.clone());
        g.compile();
        assert!(g.is_compiled());
        g.remove_edge(&edge);
        assert!(!g.is_compiled());
    }

    #[test]
    fn cycle_detection() {
        // A → B → A creates a cycle; compile should fail
        let mut g = AnimationGraph::new();
        let a = g.add_node(Box::new(PassthroughNode));
        let b = g.add_node(Box::new(PassthroughNode));
        g.add_edge(AnimEdge { from_node: a, from_port: 0, to_node: b, to_port: 0 });
        g.add_edge(AnimEdge { from_node: b, from_port: 0, to_node: a, to_port: 0 });
        assert!(!g.compile());
    }

    #[test]
    fn execute_without_compile_returns_false() {
        let mut g = AnimationGraph::new();
        g.add_node(Box::new(ConstantNode(1.0)));
        // Never compiled
        assert!(!g.execute(&default_ctx()));
    }

    #[test]
    fn empty_graph_compiles_and_executes() {
        let mut g = AnimationGraph::new();
        assert!(g.compile());
        assert!(g.execute(&default_ctx()));
    }

    #[test]
    fn get_output_before_execute_is_none() {
        let mut g = AnimationGraph::new();
        let id = g.add_node(Box::new(ConstantNode(5.0)));
        g.compile();
        // Not yet executed
        assert!(g.get_output(id, 0).is_none());
    }
}
