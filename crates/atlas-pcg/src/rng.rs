//! Deterministic pseudo-random number generator (xorshift64*).
//!
//! Faithful Rust port of the C++ `atlas::procedural::DeterministicRNG`.
//!
//! Properties:
//! - Period: 2^64 − 1
//! - No floating-point in state transitions
//! - No OS or standard-library random dependencies

/// Deterministic xorshift64* generator.
///
/// Given the same seed, the exact same sequence of numbers is produced on
/// every platform and every run.
#[derive(Clone, Debug)]
pub struct DeterministicRng {
    state: u64,
}

impl DeterministicRng {
    /// Construct with a seed.  Seed `0` is remapped to `1` because xorshift
    /// requires a non-zero state.
    pub fn new(seed: u64) -> Self {
        Self { state: if seed == 0 { 1 } else { seed } }
    }

    /// Re-seed the generator.
    pub fn seed(&mut self, seed: u64) {
        self.state = if seed == 0 { 1 } else { seed };
    }

    /// Return the current internal state (= effective seed).
    pub fn get_seed(&self) -> u64 {
        self.state
    }

    /// Advance the state and return the next 64-bit unsigned integer.
    #[inline]
    pub fn next(&mut self) -> u64 {
        self.state ^= self.state >> 12;
        self.state ^= self.state << 25;
        self.state ^= self.state >> 27;
        self.state.wrapping_mul(2_685_821_657_736_338_717)
    }

    /// Return a `u32` in `[0, max)`.
    pub fn next_u32(&mut self, max: u32) -> u32 {
        if max == 0 {
            return 0;
        }
        (self.next() % max as u64) as u32
    }

    /// Return a `f32` in `[0, 1)`.
    pub fn next_float(&mut self) -> f32 {
        (self.next() >> 40) as f32 / 16_777_216.0
    }

    /// Return a `f32` in `[min, max)`.
    pub fn next_float_range(&mut self, min: f32, max: f32) -> f32 {
        min + self.next_float() * (max - min)
    }

    /// Return an `i32` in `[min, max]` (inclusive).
    pub fn next_int_range(&mut self, min: i32, max: i32) -> i32 {
        if min >= max {
            return min;
        }
        let range = (max - min + 1) as u32;
        min + self.next_u32(range) as i32
    }

    /// Return `true` with the given probability.
    pub fn next_bool(&mut self, probability: f32) -> bool {
        self.next_float() < probability
    }

    /// Derive an independent child generator by mixing the current state with
    /// a domain-specific `salt`.  Different salts produce completely different
    /// streams, even from the same parent state.
    pub fn fork(&self, salt: u64) -> DeterministicRng {
        let mixed = self.state
            ^ salt
                .wrapping_mul(6_364_136_223_846_793_005)
                .wrapping_add(1_442_695_040_888_963_407);
        DeterministicRng::new(mixed)
    }
}

impl Default for DeterministicRng {
    fn default() -> Self {
        Self::new(1)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn deterministic_sequence() {
        let mut a = DeterministicRng::new(42);
        let mut b = DeterministicRng::new(42);
        for _ in 0..1000 {
            assert_eq!(a.next(), b.next());
        }
    }

    #[test]
    fn seed_zero_remapped() {
        let rng = DeterministicRng::new(0);
        assert_eq!(rng.get_seed(), 1);
    }

    #[test]
    fn float_in_range() {
        let mut rng = DeterministicRng::new(99);
        for _ in 0..10_000 {
            let f = rng.next_float();
            assert!((0.0..1.0).contains(&f));
        }
    }

    #[test]
    fn int_range_inclusive() {
        let mut rng = DeterministicRng::new(7);
        for _ in 0..1_000 {
            let v = rng.next_int_range(-5, 5);
            assert!((-5..=5).contains(&v));
        }
    }

    #[test]
    fn fork_produces_different_streams() {
        let parent = DeterministicRng::new(1);
        let mut child_a = parent.fork(1);
        let mut child_b = parent.fork(2);
        // Different salts must diverge.
        let va: Vec<u64> = (0..10).map(|_| child_a.next()).collect();
        let vb: Vec<u64> = (0..10).map(|_| child_b.next()).collect();
        assert_ne!(va, vb);
    }
}
