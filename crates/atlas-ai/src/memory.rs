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

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn store_and_recall() {
        let mut m = AIMemory::new();
        m.store("enemy", "Soldier", 1.0, 0.1, 0);
        assert!(m.has("enemy"));
        let e = m.recall("enemy").unwrap();
        assert_eq!(e.key, "enemy");
        assert_eq!(e.value, "Soldier");
        assert!((e.strength - 1.0).abs() < f32::EPSILON);
    }

    #[test]
    fn forget() {
        let mut m = AIMemory::new();
        m.store("pos", "north", 1.0, 0.0, 0);
        m.forget("pos");
        assert!(!m.has("pos"));
    }

    #[test]
    fn tick_decays_strength() {
        let mut m = AIMemory::new();
        m.store("threat", "high", 1.0, 0.5, 0); // decay 50% per tick
        m.tick(1);
        let e = m.recall("threat").unwrap();
        assert!((e.strength - 0.5).abs() < 0.01);
    }

    #[test]
    fn tick_removes_zero_strength() {
        let mut m = AIMemory::new();
        m.store("gone", "x", 0.01, 1.0, 0); // decay rate = 100% → strength = 0 after one tick
        m.tick(1);
        // strength *= (1 - 1.0) = 0.0 → removed
        assert!(!m.has("gone"));
        assert_eq!(m.count(), 0);
    }

    #[test]
    fn clear() {
        let mut m = AIMemory::new();
        m.store("a", "1", 1.0, 0.0, 0);
        m.store("b", "2", 1.0, 0.0, 0);
        assert_eq!(m.count(), 2);
        m.clear();
        assert_eq!(m.count(), 0);
    }

    #[test]
    fn overwrite_existing_entry() {
        let mut m = AIMemory::new();
        m.store("key", "old", 1.0, 0.0, 0);
        m.store("key", "new", 0.8, 0.0, 1);
        let e = m.recall("key").unwrap();
        assert_eq!(e.value, "new");
        assert!((e.strength - 0.8).abs() < f32::EPSILON);
        assert_eq!(m.count(), 1);
    }

    #[test]
    fn recall_nonexistent_returns_none() {
        let m = AIMemory::new();
        assert!(m.recall("x").is_none());
    }
}
