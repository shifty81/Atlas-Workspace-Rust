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
