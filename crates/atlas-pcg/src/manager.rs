//! Universe-seed authority for all PCG domains.
//!
//! Faithful Rust port of the C++ `atlas::procedural::PCGManager`.

use crate::domain::{PcgContext, PcgDomain, SeedLevel};
use crate::rng::DeterministicRng;

/// Central PCG seed authority.
///
/// Owns the universe seed and provides isolated, deterministic RNG contexts
/// for each of the 16 PCG domains.  This is the single source of truth for
/// all procedural generation — no subsystem should create its own RNG.
///
/// # Example
/// ```
/// use atlas_pcg::{PcgManager, PcgDomain, SeedLevel};
///
/// let mut mgr = PcgManager::new(42);
/// let mut ctx = mgr.create_context(PcgDomain::Planet, SeedLevel::Object, 7);
/// let radius = ctx.rng.next_float_range(500.0, 15_000.0);
/// ```
pub struct PcgManager {
    universe_seed: u64,
    version:       u32,
    domain_seeds:  [u64; PcgDomain::COUNT],
}

impl PcgManager {
    /// Create with the given universe seed.
    pub fn new(universe_seed: u64) -> Self {
        let mut mgr = Self {
            universe_seed,
            version:      1,
            domain_seeds: [0; PcgDomain::COUNT],
        };
        mgr.derive_all_domain_seeds();
        mgr
    }

    /// Replace the universe seed and re-derive all domain seeds.
    pub fn set_universe_seed(&mut self, seed: u64) {
        self.universe_seed = seed;
        self.derive_all_domain_seeds();
    }

    /// Current universe seed.
    pub fn universe_seed(&self) -> u64 {
        self.universe_seed
    }

    /// Version tag; bump to invalidate all cached PCG output.
    pub fn version(&self) -> u32 { self.version }
    pub fn set_version(&mut self, v: u32) { self.version = v; }

    /// Per-domain root seed (before hierarchy scoping).
    pub fn domain_seed(&self, domain: PcgDomain) -> u64 {
        self.domain_seeds[domain as usize]
    }

    /// Create a scoped PCG context for the given domain and hierarchy level.
    ///
    /// The `location_salt` further differentiates contexts within the same
    /// level (e.g. a planet index or entity ID).
    ///
    /// Each level is mixed with a unique level-specific prime so that
    /// `(location_salt=10, level=0)` and `(location_salt=9, level=1)` always
    /// produce different seeds.
    pub fn create_context(
        &self,
        domain: PcgDomain,
        level:  SeedLevel,
        location_salt: u64,
    ) -> PcgContext {
        // Level-specific mixing primes (one per SeedLevel variant).
        const LEVEL_PRIMES: [u64; 5] = [
            0x9e37_79b9_7f4a_7c15, // Universe
            0x6c62_272e_07bb_0142, // Galaxy
            0x94d0_49bb_1331_11eb, // System
            0xbf58_476d_1ce4_e5b9, // Sector
            0xe655_01ae_d3c3_3c41, // Object
        ];

        let base_seed = self.domain_seed(domain);
        let mut ctx = PcgContext::new(domain, SeedLevel::Universe, base_seed);
        let target = level as usize;
        for l in 0..target {
            // Mix the location salt with a level-unique prime to prevent
            // seed collisions across different hierarchy depths.
            let level_salt = location_salt
                .wrapping_mul(LEVEL_PRIMES[l])
                .wrapping_add(l as u64 + 1);
            ctx = ctx.child(level_salt);
        }
        ctx
    }

    // ── Private ──────────────────────────────────────────────────────────

    fn derive_all_domain_seeds(&mut self) {
        let root = DeterministicRng::new(self.universe_seed);
        for i in 0..PcgDomain::COUNT {
            let mut domain_rng = root.fork(i as u64 + 1);
            self.domain_seeds[i] = domain_rng.next();
        }
    }
}

impl Default for PcgManager {
    fn default() -> Self {
        Self::new(42)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn same_seed_same_domains() {
        let a = PcgManager::new(12345);
        let b = PcgManager::new(12345);
        for d in PcgDomain::all() {
            assert_eq!(a.domain_seed(d), b.domain_seed(d));
        }
    }

    #[test]
    fn different_seed_different_domains() {
        let a = PcgManager::new(1);
        let b = PcgManager::new(2);
        // At least one domain seed must differ.
        let any_diff = PcgDomain::all().any(|d| a.domain_seed(d) != b.domain_seed(d));
        assert!(any_diff);
    }

    #[test]
    fn domains_isolated() {
        let mgr = PcgManager::new(99);
        let s_ship    = mgr.domain_seed(PcgDomain::Ship);
        let s_terrain = mgr.domain_seed(PcgDomain::Terrain);
        assert_ne!(s_ship, s_terrain);
    }

    #[test]
    fn context_deterministic() {
        let mgr = PcgManager::new(42);
        let mut ctx_a = mgr.create_context(PcgDomain::Planet, SeedLevel::Object, 7);
        let mut ctx_b = mgr.create_context(PcgDomain::Planet, SeedLevel::Object, 7);
        for _ in 0..100 {
            assert_eq!(ctx_a.rng.next(), ctx_b.rng.next());
        }
    }

    #[test]
    fn set_universe_seed_rederives() {
        let mut mgr = PcgManager::new(1);
        let old = mgr.domain_seed(PcgDomain::Galaxy);
        mgr.set_universe_seed(2);
        let new = mgr.domain_seed(PcgDomain::Galaxy);
        assert_ne!(old, new);
    }
}
