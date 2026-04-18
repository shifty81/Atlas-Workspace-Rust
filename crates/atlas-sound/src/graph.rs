use std::collections::HashMap;
use std::collections::VecDeque;

pub type SoundNodeId = u32;
pub type SoundPortId = u16;

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
#[repr(u8)]
pub enum SoundPinType {
    AudioBuffer = 0,
    Float,
    Seed,
    Trigger,
    Envelope,
}

#[derive(Debug, Clone)]
pub struct SoundValue {
    pub pin_type: SoundPinType,
    pub data: Vec<f32>,
}

#[derive(Debug, Clone)]
pub struct SoundPort {
    pub name: String,
    pub pin_type: SoundPinType,
}

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct SoundEdge {
    pub from_node: SoundNodeId,
    pub from_port: SoundPortId,
    pub to_node: SoundNodeId,
    pub to_port: SoundPortId,
}

#[derive(Debug, Clone, Default)]
pub struct SoundContext {
    pub sample_rate: u32,
    pub buffer_size: u32,
    pub seed: u64,
}

pub trait SoundNode: Send + Sync {
    fn name(&self) -> &str;
    fn category(&self) -> &str;
    fn inputs(&self) -> Vec<SoundPort>;
    fn outputs(&self) -> Vec<SoundPort>;
    fn evaluate(&mut self, ctx: &SoundContext, inputs: &[SoundValue], outputs: &mut Vec<SoundValue>);
}

struct NodeEntry {
    id: SoundNodeId,
    node: Box<dyn SoundNode + Send + Sync>,
}

pub struct SoundGraph {
    nodes: Vec<NodeEntry>,
    edges: Vec<SoundEdge>,
    next_id: SoundNodeId,
    execution_order: Vec<SoundNodeId>,
    compiled: bool,
    outputs: HashMap<(SoundNodeId, SoundPortId), SoundValue>,
}

impl Default for SoundGraph {
    fn default() -> Self { Self::new() }
}

impl SoundGraph {
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

    pub fn add_node(&mut self, node: Box<dyn SoundNode + Send + Sync>) -> SoundNodeId {
        let id = self.next_id;
        self.next_id += 1;
        self.nodes.push(NodeEntry { id, node });
        self.compiled = false;
        id
    }

    pub fn remove_node(&mut self, id: SoundNodeId) {
        self.nodes.retain(|e| e.id != id);
        self.edges.retain(|e| e.from_node != id && e.to_node != id);
        self.compiled = false;
    }

    pub fn add_edge(&mut self, edge: SoundEdge) {
        self.edges.push(edge);
        self.compiled = false;
    }

    pub fn remove_edge(&mut self, edge: &SoundEdge) {
        self.edges.retain(|e| e != edge);
        self.compiled = false;
    }

    pub fn compile(&mut self) -> bool {
        let node_ids: Vec<SoundNodeId> = self.nodes.iter().map(|e| e.id).collect();
        let mut in_degree: HashMap<SoundNodeId, usize> = node_ids.iter().map(|&id| (id, 0)).collect();
        let mut adj: HashMap<SoundNodeId, Vec<SoundNodeId>> = node_ids.iter().map(|&id| (id, Vec::new())).collect();

        for edge in &self.edges {
            if let Some(deg) = in_degree.get_mut(&edge.to_node) {
                *deg += 1;
            }
            adj.entry(edge.from_node).or_default().push(edge.to_node);
        }

        let mut queue: VecDeque<SoundNodeId> = in_degree.iter()
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
            return false;
        }

        self.execution_order = order;
        self.compiled = true;
        true
    }

    pub fn execute(&mut self, ctx: &SoundContext) -> bool {
        if !self.compiled {
            return false;
        }
        self.outputs.clear();

        let order = self.execution_order.clone();
        for node_id in &order {
            let input_edges: Vec<SoundEdge> = self.edges.iter()
                .filter(|e| e.to_node == *node_id)
                .cloned()
                .collect();

            let node_entry = match self.nodes.iter_mut().find(|e| e.id == *node_id) {
                Some(e) => e,
                None => continue,
            };
            let num_inputs = node_entry.node.inputs().len();
            let mut inputs = vec![SoundValue { pin_type: SoundPinType::Float, data: Vec::new() }; num_inputs];
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
            let mut node_outputs = vec![SoundValue { pin_type: SoundPinType::Float, data: Vec::new() }; num_outputs];
            node_entry.node.evaluate(ctx, &inputs, &mut node_outputs);

            for (port_idx, out_val) in node_outputs.into_iter().enumerate() {
                self.outputs.insert((*node_id, port_idx as SoundPortId), out_val);
            }
        }
        true
    }

    pub fn get_output(&self, node: SoundNodeId, port: SoundPortId) -> Option<&SoundValue> {
        self.outputs.get(&(node, port))
    }

    pub fn node_count(&self) -> usize {
        self.nodes.len()
    }

    pub fn is_compiled(&self) -> bool {
        self.compiled
    }
}
