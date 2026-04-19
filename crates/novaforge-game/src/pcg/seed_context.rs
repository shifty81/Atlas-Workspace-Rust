// SPDX-License-Identifier: GPL-3.0-only
// NovaForge PCG deterministic seed context — port of NovaForge::PCGDeterministicSeedContext.
// Copyright (C) NovaForge contributors. GNU General Public License v3.0.

use std::collections::HashMap;

pub const DEFAULT_UNIVERSE_SEED: u64 = 42_424_242;

pub struct PcgDeterministicSeedContext {
    universe_seed:       u64,
    domain_seeds:        HashMap<String, u64>,
    registered_domains:  Vec<String>,
}

impl PcgDeterministicSeedContext {
    pub fn new(universe_seed: u64) -> Self {
        let seed = if universe_seed == 0 { 1 } else { universe_seed };
        Self {
            universe_seed: seed,
            domain_seeds: HashMap::new(),
            registered_domains: Vec::new(),
        }
    }

    pub fn universe_seed(&self) -> u64 { self.universe_seed }

    /// Replace universe seed; clears all cached domain seeds. 0 → 1.
    pub fn set_universe_seed(&mut self, seed: u64) {
        self.universe_seed = if seed == 0 { 1 } else { seed };
        self.domain_seeds.clear();
    }

    /// Derive and cache the seed for a given domain.
    pub fn seed_for_domain(&mut self, domain: &str) -> u64 {
        if let Some(&s) = self.domain_seeds.get(domain) {
            return s;
        }
        let s = derive_seed(self.universe_seed, domain);
        self.domain_seeds.insert(domain.to_string(), s);
        s
    }

    /// Derive a child context for a named sub-domain.
    pub fn child_context(&self, name: &str) -> Self {
        Self {
            universe_seed:      derive_seed(self.universe_seed, name),
            domain_seeds:       HashMap::new(),
            registered_domains: Vec::new(),
        }
    }

    /// Pin a specific domain seed for debugging / reproduction.
    pub fn pin_domain_seed(&mut self, domain: impl Into<String>, seed: u64) {
        self.domain_seeds.insert(domain.into(), seed);
    }

    pub fn has_pinned_seed(&self, domain: &str) -> bool {
        self.domain_seeds.contains_key(domain)
    }

    pub fn clear_pinned_seeds(&mut self) {
        self.domain_seeds.clear();
    }

    pub fn register_domain(&mut self, domain: impl Into<String>) {
        let d = domain.into();
        if !self.has_domain(&d) {
            self.registered_domains.push(d);
        }
    }

    pub fn has_domain(&self, domain: &str) -> bool {
        self.registered_domains.iter().any(|d| d == domain)
    }

    pub fn registered_domains(&self) -> &[String] { &self.registered_domains }
}

impl Default for PcgDeterministicSeedContext {
    fn default() -> Self {
        Self::new(DEFAULT_UNIVERSE_SEED)
    }
}

/// FNV-1a + xorshift64* — matches C++ deriveSeed exactly.
pub fn derive_seed(base: u64, name: &str) -> u64 {
    let mut salt: u64 = 14_695_981_039_346_656_037;
    for c in name.bytes() {
        salt ^= c as u64;
        salt = salt.wrapping_mul(1_099_511_628_211);
    }
    let mut s = base ^ salt;
    if s == 0 { s = 1; }
    s ^= s >> 12;
    s ^= s << 25;
    s ^= s >> 27;
    s = s.wrapping_mul(2_685_821_657_736_338_717);
    if s == 0 { s = 1; }
    s
}

// ── Tests ─────────────────────────────────────────────────────────────────

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn new_with_default_seed() {
        let ctx = PcgDeterministicSeedContext::new(DEFAULT_UNIVERSE_SEED);
        assert_eq!(ctx.universe_seed(), DEFAULT_UNIVERSE_SEED);
    }

    #[test]
    fn zero_seed_becomes_one() {
        let ctx = PcgDeterministicSeedContext::new(0);
        assert_eq!(ctx.universe_seed(), 1);
    }

    #[test]
    fn set_universe_seed_clears_cache() {
        let mut ctx = PcgDeterministicSeedContext::new(DEFAULT_UNIVERSE_SEED);
        let s1 = ctx.seed_for_domain("forest");
        ctx.set_universe_seed(999);
        let s2 = ctx.seed_for_domain("forest");
        assert_ne!(s1, s2);
    }

    #[test]
    fn seed_for_domain_deterministic() {
        let mut ctx = PcgDeterministicSeedContext::new(DEFAULT_UNIVERSE_SEED);
        let a = ctx.seed_for_domain("biome");
        let b = ctx.seed_for_domain("biome");
        assert_eq!(a, b);
        assert_ne!(a, 0);
    }

    #[test]
    fn different_domains_produce_different_seeds() {
        let mut ctx = PcgDeterministicSeedContext::new(DEFAULT_UNIVERSE_SEED);
        let a = ctx.seed_for_domain("forest");
        let b = ctx.seed_for_domain("desert");
        assert_ne!(a, b);
    }

    #[test]
    fn child_context_has_different_seed() {
        let ctx = PcgDeterministicSeedContext::new(DEFAULT_UNIVERSE_SEED);
        let child = ctx.child_context("zone-1");
        assert_ne!(child.universe_seed(), ctx.universe_seed());
    }

    #[test]
    fn pin_seed_is_returned() {
        let mut ctx = PcgDeterministicSeedContext::new(DEFAULT_UNIVERSE_SEED);
        ctx.pin_domain_seed("pinned", 12345);
        assert!(ctx.has_pinned_seed("pinned"));
        let s = ctx.seed_for_domain("pinned");
        assert_eq!(s, 12345);
    }

    #[test]
    fn has_domain_after_register() {
        let mut ctx = PcgDeterministicSeedContext::new(DEFAULT_UNIVERSE_SEED);
        ctx.register_domain("biome");
        assert!(ctx.has_domain("biome"));
        assert!(!ctx.has_domain("unknown"));
    }

    #[test]
    fn clear_pinned_seeds() {
        let mut ctx = PcgDeterministicSeedContext::new(DEFAULT_UNIVERSE_SEED);
        ctx.pin_domain_seed("x", 99);
        ctx.clear_pinned_seeds();
        assert!(!ctx.has_pinned_seed("x"));
    }
}
