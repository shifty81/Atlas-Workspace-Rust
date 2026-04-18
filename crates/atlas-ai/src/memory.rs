use std::collections::HashMap;

#[derive(Debug, Clone)]
pub struct MemoryEntry {
    pub key: String,
    pub value: String,
    pub strength: f32,
    pub decay_rate: f32,
    pub created_tick: u64,
}

#[derive(Default)]
pub struct AIMemory {
    entries: HashMap<String, MemoryEntry>,
}

impl AIMemory {
    pub fn new() -> Self {
        Self::default()
    }

    pub fn store(&mut self, key: impl Into<String>, value: impl Into<String>, strength: f32, decay_rate: f32, tick: u64) {
        let key = key.into();
        self.entries.insert(key.clone(), MemoryEntry {
            key,
            value: value.into(),
            strength,
            decay_rate,
            created_tick: tick,
        });
    }

    pub fn recall(&self, key: &str) -> Option<&MemoryEntry> {
        self.entries.get(key)
    }

    pub fn has(&self, key: &str) -> bool {
        self.entries.contains_key(key)
    }

    pub fn forget(&mut self, key: &str) {
        self.entries.remove(key);
    }

    pub fn clear(&mut self) {
        self.entries.clear();
    }

    pub fn tick(&mut self, _current_tick: u64) {
        self.entries.retain(|_, entry| {
            entry.strength *= 1.0 - entry.decay_rate;
            entry.strength > 0.0
        });
    }

    pub fn count(&self) -> usize {
        self.entries.len()
    }
}
