#[derive(Debug, Clone, Default)]
pub struct HashEntry {
    pub tick: u64,
    pub hash: u64,
}

#[derive(Debug, Default)]
pub struct StateHasher {
    current_tick: u64,
    current_hash: u64,
    history: Vec<HashEntry>,
    seed: u64,
}

impl StateHasher {
    pub fn new() -> Self { Self::default() }

    pub fn reset(&mut self, seed: u64) {
        self.seed = seed;
        self.current_hash = seed;
        self.current_tick = 0;
        self.history.clear();
    }

    pub fn advance_tick(&mut self, tick: u64, state_data: &[u8], state_size: usize, input_data: &[u8], input_size: usize) {
        self.current_tick = tick;
        let state_slice = &state_data[..state_size.min(state_data.len())];
        let input_slice = &input_data[..input_size.min(input_data.len())];
        let mut h = self.current_hash;
        h = Self::hash_combine(h, state_slice);
        h = Self::hash_combine(h, input_slice);
        self.current_hash = h;
        self.history.push(HashEntry { tick, hash: h });
    }

    pub fn current_hash(&self) -> u64 { self.current_hash }
    pub fn current_tick(&self) -> u64 { self.current_tick }
    pub fn history(&self) -> &[HashEntry] { &self.history }

    pub fn find_divergence(&self, other: &StateHasher) -> i64 {
        for (a, b) in self.history.iter().zip(other.history.iter()) {
            if a.hash != b.hash {
                return a.tick as i64;
            }
        }
        -1
    }

    pub fn hash_combine(prev: u64, data: &[u8]) -> u64 {
        let mut h = prev;
        for &byte in data {
            h ^= byte as u64;
            h = h.wrapping_mul(0x100000001b3u64);
        }
        h
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn hash_advances_with_data() {
        let mut h = StateHasher::new();
        h.reset(42);
        h.advance_tick(1, b"state", 5, b"input", 5);
        assert_eq!(h.current_tick(), 1);
        assert!(!h.history().is_empty());
    }

    #[test]
    fn same_data_gives_same_hash() {
        let mut h1 = StateHasher::new();
        let mut h2 = StateHasher::new();
        h1.reset(0);
        h2.reset(0);
        h1.advance_tick(1, b"state", 5, b"input", 5);
        h2.advance_tick(1, b"state", 5, b"input", 5);
        assert_eq!(h1.current_hash(), h2.current_hash());
    }

    #[test]
    fn different_data_gives_different_hash() {
        let mut h1 = StateHasher::new();
        let mut h2 = StateHasher::new();
        h1.reset(0);
        h2.reset(0);
        h1.advance_tick(1, b"AAAA", 4, b"", 0);
        h2.advance_tick(1, b"BBBB", 4, b"", 0);
        assert_ne!(h1.current_hash(), h2.current_hash());
    }

    #[test]
    fn divergence_detected() {
        let mut h1 = StateHasher::new();
        let mut h2 = StateHasher::new();
        h1.reset(0);
        h2.reset(0);
        // Tick 1: same
        h1.advance_tick(1, b"same", 4, b"", 0);
        h2.advance_tick(1, b"same", 4, b"", 0);
        // Tick 2: diverge
        h1.advance_tick(2, b"aaa", 3, b"", 0);
        h2.advance_tick(2, b"bbb", 3, b"", 0);
        assert_eq!(h1.find_divergence(&h2), 2);
    }

    #[test]
    fn no_divergence_returns_minus_one() {
        let mut h1 = StateHasher::new();
        let mut h2 = StateHasher::new();
        h1.reset(7);
        h2.reset(7);
        h1.advance_tick(1, b"xyz", 3, b"", 0);
        h2.advance_tick(1, b"xyz", 3, b"", 0);
        assert_eq!(h1.find_divergence(&h2), -1);
    }

    #[test]
    fn reset_clears_history() {
        let mut h = StateHasher::new();
        h.reset(0);
        h.advance_tick(1, b"x", 1, b"", 0);
        assert!(!h.history().is_empty());
        h.reset(0);
        assert!(h.history().is_empty());
    }

    #[test]
    fn hash_combine_is_deterministic() {
        let a = StateHasher::hash_combine(0, b"atlas");
        let b = StateHasher::hash_combine(0, b"atlas");
        assert_eq!(a, b);
    }
}
