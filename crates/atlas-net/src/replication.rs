use std::collections::HashSet;

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum ReplicateFrequency {
    EveryTick,
    OnChange,
    Manual,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum ReplicateDirection {
    ServerToClient,
    ClientToServer,
    Bidirectional,
}

#[derive(Debug, Clone)]
pub struct ReplicationRule {
    pub type_tag: u32,
    pub component_name: String,
    pub frequency: ReplicateFrequency,
    pub direction: ReplicateDirection,
    pub reliable: bool,
    pub priority: u8,
}

#[derive(Default)]
pub struct ReplicationManager {
    rules: Vec<ReplicationRule>,
    dirty: HashSet<(u32, u32)>, // (type_tag, entity_id)
    manual_triggers: HashSet<u32>,
}

impl ReplicationManager {
    pub fn new() -> Self {
        Self::default()
    }

    pub fn add_rule(&mut self, rule: ReplicationRule) {
        self.rules.retain(|r| r.type_tag != rule.type_tag);
        self.rules.push(rule);
    }

    pub fn remove_rule(&mut self, type_tag: u32) {
        self.rules.retain(|r| r.type_tag != type_tag);
    }

    pub fn has_rule(&self, type_tag: u32) -> bool {
        self.rules.iter().any(|r| r.type_tag == type_tag)
    }

    pub fn get_rule(&self, type_tag: u32) -> Option<&ReplicationRule> {
        self.rules.iter().find(|r| r.type_tag == type_tag)
    }

    pub fn rules(&self) -> &[ReplicationRule] {
        &self.rules
    }

    pub fn rule_count(&self) -> usize {
        self.rules.len()
    }

    pub fn mark_dirty(&mut self, type_tag: u32, entity_id: u32) {
        self.dirty.insert((type_tag, entity_id));
    }

    pub fn is_dirty(&self, type_tag: u32, entity_id: u32) -> bool {
        self.dirty.contains(&(type_tag, entity_id))
    }

    pub fn clear_dirty(&mut self) {
        self.dirty.clear();
    }

    pub fn trigger_manual_replication(&mut self, type_tag: u32) {
        self.manual_triggers.insert(type_tag);
    }

    /// Collect reliable dirty entries. Binary format: [count: u32][type_tag: u32, entity_id: u32]*
    pub fn collect_delta(&mut self, _tick: u32) -> Vec<u8> {
        let reliable: Vec<(u32, u32)> = self.dirty.iter()
            .filter(|(tt, _)| {
                self.rules.iter().any(|r| r.type_tag == *tt && r.reliable)
            })
            .copied()
            .collect();
        let mut buf = Vec::new();
        buf.extend_from_slice(&(reliable.len() as u32).to_le_bytes());
        for (tt, eid) in &reliable {
            buf.extend_from_slice(&tt.to_le_bytes());
            buf.extend_from_slice(&eid.to_le_bytes());
        }
        buf
    }

    pub fn collect_unreliable_delta(&mut self, _tick: u32) -> Vec<u8> {
        let unreliable: Vec<(u32, u32)> = self.dirty.iter()
            .filter(|(tt, _)| {
                self.rules.iter().any(|r| r.type_tag == *tt && !r.reliable)
            })
            .copied()
            .collect();
        let mut buf = Vec::new();
        buf.extend_from_slice(&(unreliable.len() as u32).to_le_bytes());
        for (tt, eid) in &unreliable {
            buf.extend_from_slice(&tt.to_le_bytes());
            buf.extend_from_slice(&eid.to_le_bytes());
        }
        buf
    }

    pub fn apply_delta(&mut self, data: &[u8]) -> bool {
        if data.len() < 4 {
            return false;
        }
        let count = u32::from_le_bytes(data[0..4].try_into().unwrap()) as usize;
        if data.len() < 4 + count * 8 {
            return false;
        }
        true
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    fn make_rule(tag: u32, name: &str) -> ReplicationRule {
        ReplicationRule {
            type_tag: tag,
            component_name: name.into(),
            frequency: ReplicateFrequency::OnChange,
            direction: ReplicateDirection::ServerToClient,
            reliable: true,
            priority: 1,
        }
    }

    #[test]
    fn add_and_query_rule() {
        let mut mgr = ReplicationManager::new();
        mgr.add_rule(make_rule(1, "Transform"));
        assert!(mgr.has_rule(1));
        assert_eq!(mgr.rule_count(), 1);
        let r = mgr.get_rule(1).unwrap();
        assert_eq!(r.component_name, "Transform");
    }

    #[test]
    fn remove_rule() {
        let mut mgr = ReplicationManager::new();
        mgr.add_rule(make_rule(1, "Transform"));
        mgr.remove_rule(1);
        assert!(!mgr.has_rule(1));
        assert_eq!(mgr.rule_count(), 0);
    }

    #[test]
    fn add_same_tag_replaces() {
        let mut mgr = ReplicationManager::new();
        mgr.add_rule(make_rule(1, "A"));
        mgr.add_rule(make_rule(1, "B"));
        assert_eq!(mgr.rule_count(), 1);
        assert_eq!(mgr.get_rule(1).unwrap().component_name, "B");
    }
}
