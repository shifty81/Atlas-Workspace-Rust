use std::collections::{HashMap, VecDeque};

pub type BehaviorNodeId = u32;
pub type BehaviorPortId = u16;

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
#[repr(u8)]
pub enum BehaviorPinType {
    Float = 0,
    Bool,
    Action,
    Perception,
    EmotionState,
}

#[derive(Debug, Clone)]
pub struct BehaviorValue {
    pub pin_type: BehaviorPinType,
    pub data: Vec<f32>,
}

#[derive(Debug, Clone)]
pub struct BehaviorPort {
    pub name: String,
    pub pin_type: BehaviorPinType,
}

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct BehaviorEdge {
    pub from_node: BehaviorNodeId,
    pub from_port: BehaviorPortId,
    pub to_node: BehaviorNodeId,
    pub to_port: BehaviorPortId,
}

#[derive(Debug, Clone, Default)]
pub struct AIContext {
    pub threat_level: f32,
    pub health_percent: f32,
    pub ammo_percent: f32,
    pub morale: f32,
    pub tick: u32,
}

pub trait BehaviorNode: Send + Sync {
    fn name(&self) -> &str;
    fn category(&self) -> &str;
    fn inputs(&self) -> Vec<BehaviorPort>;
    fn outputs(&self) -> Vec<BehaviorPort>;
    fn evaluate(&mut self, ctx: &AIContext, inputs: &[BehaviorValue], outputs: &mut Vec<BehaviorValue>);
}

struct NodeEntry {
    id: BehaviorNodeId,
    node: Box<dyn BehaviorNode + Send + Sync>,
}

pub struct BehaviorGraph {
    nodes: Vec<NodeEntry>,
    edges: Vec<BehaviorEdge>,
    next_id: BehaviorNodeId,
    execution_order: Vec<BehaviorNodeId>,
    compiled: bool,
    outputs: HashMap<(BehaviorNodeId, BehaviorPortId), BehaviorValue>,
}

impl Default for BehaviorGraph {
    fn default() -> Self {
        Self::new()
    }
}

impl BehaviorGraph {
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

    pub fn add_node(&mut self, node: Box<dyn BehaviorNode + Send + Sync>) -> BehaviorNodeId {
        let id = self.next_id;
        self.next_id += 1;
        self.nodes.push(NodeEntry { id, node });
        self.compiled = false;
        id
    }

    pub fn remove_node(&mut self, id: BehaviorNodeId) {
        self.nodes.retain(|e| e.id != id);
        self.edges.retain(|e| e.from_node != id && e.to_node != id);
        self.compiled = false;
    }

    pub fn add_edge(&mut self, edge: BehaviorEdge) {
        self.edges.push(edge);
        self.compiled = false;
    }

    pub fn remove_edge(&mut self, edge: &BehaviorEdge) {
        self.edges.retain(|e| e != edge);
        self.compiled = false;
    }

    pub fn compile(&mut self) -> bool {
        let node_ids: Vec<BehaviorNodeId> = self.nodes.iter().map(|e| e.id).collect();
        let mut in_degree: HashMap<BehaviorNodeId, usize> = node_ids.iter().map(|&id| (id, 0)).collect();
        let mut adj: HashMap<BehaviorNodeId, Vec<BehaviorNodeId>> = node_ids.iter().map(|&id| (id, Vec::new())).collect();

        for edge in &self.edges {
            if let Some(deg) = in_degree.get_mut(&edge.to_node) {
                *deg += 1;
            }
            adj.entry(edge.from_node).or_default().push(edge.to_node);
        }

        let mut queue: VecDeque<BehaviorNodeId> = in_degree.iter()
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
            log::error!("BehaviorGraph: cycle detected");
            return false;
        }

        self.execution_order = order;
        self.compiled = true;
        true
    }

    pub fn execute(&mut self, ctx: &AIContext) -> bool {
        if !self.compiled {
            return false;
        }
        self.outputs.clear();

        let order = self.execution_order.clone();
        for node_id in &order {
            let input_edges: Vec<BehaviorEdge> = self.edges.iter()
                .filter(|e| e.to_node == *node_id)
                .cloned()
                .collect();

            let node_entry = match self.nodes.iter_mut().find(|e| e.id == *node_id) {
                Some(e) => e,
                None => continue,
            };
            let num_inputs = node_entry.node.inputs().len();
            let mut inputs = vec![BehaviorValue { pin_type: BehaviorPinType::Float, data: Vec::new() }; num_inputs];
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
            let mut node_outputs = vec![BehaviorValue { pin_type: BehaviorPinType::Float, data: Vec::new() }; num_outputs];
            node_entry.node.evaluate(ctx, &inputs, &mut node_outputs);

            for (port_idx, out_val) in node_outputs.into_iter().enumerate() {
                self.outputs.insert((*node_id, port_idx as BehaviorPortId), out_val);
            }
        }
        true
    }

    pub fn get_output(&self, node: BehaviorNodeId, port: BehaviorPortId) -> Option<&BehaviorValue> {
        self.outputs.get(&(node, port))
    }

    pub fn node_count(&self) -> usize {
        self.nodes.len()
    }

    pub fn is_compiled(&self) -> bool {
        self.compiled
    }

    pub fn serialize_state(&self) -> Vec<u8> {
        let mut buf = Vec::new();
        let count = self.outputs.len() as u32;
        buf.extend_from_slice(&count.to_le_bytes());
        for (&(node_id, port_id), val) in &self.outputs {
            let key: u64 = ((node_id as u64) << 16) | (port_id as u64);
            buf.extend_from_slice(&key.to_le_bytes());
            buf.push(val.pin_type as u8);
            let data_len = val.data.len() as u32;
            buf.extend_from_slice(&data_len.to_le_bytes());
            for &f in &val.data {
                buf.extend_from_slice(&f.to_le_bytes());
            }
        }
        buf
    }

    pub fn deserialize_state(&mut self, data: &[u8]) -> bool {
        if data.len() < 4 {
            return false;
        }
        let count = u32::from_le_bytes(data[0..4].try_into().unwrap()) as usize;
        let mut off = 4;
        self.outputs.clear();
        for _ in 0..count {
            if off + 13 > data.len() {
                return false;
            }
            let key = u64::from_le_bytes(data[off..off + 8].try_into().unwrap());
            off += 8;
            let pin_type_byte = data[off];
            off += 1;
            let data_len = u32::from_le_bytes(data[off..off + 4].try_into().unwrap()) as usize;
            off += 4;
            if off + data_len * 4 > data.len() {
                return false;
            }
            let mut float_data = Vec::with_capacity(data_len);
            for i in 0..data_len {
                let f = f32::from_le_bytes(data[off + i * 4..off + i * 4 + 4].try_into().unwrap());
                float_data.push(f);
            }
            off += data_len * 4;
            let pin_type = match pin_type_byte {
                0 => BehaviorPinType::Float,
                1 => BehaviorPinType::Bool,
                2 => BehaviorPinType::Action,
                3 => BehaviorPinType::Perception,
                4 => BehaviorPinType::EmotionState,
                _ => BehaviorPinType::Float,
            };
            let node_id = (key >> 16) as BehaviorNodeId;
            let port_id = (key & 0xFFFF) as BehaviorPortId;
            self.outputs.insert((node_id, port_id), BehaviorValue { pin_type, data: float_data });
        }
        true
    }
}
