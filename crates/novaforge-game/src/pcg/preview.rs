// SPDX-License-Identifier: GPL-3.0-only
// NovaForge PCG preview service — port of NovaForge::PCGPreviewService.
// Copyright (C) NovaForge contributors. GNU General Public License v3.0.

use super::generator::{PcgGenerationResult, PcgGeneratorService};
use super::ruleset::PcgRuleSet;
use super::seed_context::{PcgDeterministicSeedContext, DEFAULT_UNIVERSE_SEED};

// ── PcgPreviewStats ───────────────────────────────────────────────────────

#[derive(Debug, Clone, Default)]
pub struct PcgPreviewStats {
    pub generation_count:    u32,
    pub last_placement_count: u32,
    pub last_generation_ms:  f32,
    pub has_result:          bool,
}

// ── PcgPreviewService ─────────────────────────────────────────────────────

pub struct PcgPreviewService {
    ruleset:         Option<Box<PcgRuleSet>>,
    seed_context:    PcgDeterministicSeedContext,
    auto_regenerate: bool,
    result_valid:    bool,
    last_result:     Option<PcgGenerationResult>,
    stats:           PcgPreviewStats,
}

impl PcgPreviewService {
    pub fn new() -> Self {
        Self {
            ruleset:         None,
            seed_context:    PcgDeterministicSeedContext::new(DEFAULT_UNIVERSE_SEED),
            auto_regenerate: true,
            result_valid:    false,
            last_result:     None,
            stats:           PcgPreviewStats::default(),
        }
    }

    // ── RuleSet binding ───────────────────────────────────────────────────

    pub fn bind_ruleset(&mut self, ruleset: PcgRuleSet) {
        self.ruleset = Some(Box::new(ruleset));
        self.result_valid = false;
        if self.auto_regenerate {
            self.regenerate();
        }
    }

    pub fn clear_ruleset(&mut self) {
        self.ruleset = None;
        self.result_valid = false;
        self.last_result = None;
    }

    pub fn has_ruleset(&self) -> bool { self.ruleset.is_some() }

    pub fn ruleset(&self) -> Option<&PcgRuleSet> {
        self.ruleset.as_deref()
    }

    // ── Auto-regeneration ─────────────────────────────────────────────────

    pub fn set_auto_regenerate(&mut self, enabled: bool) { self.auto_regenerate = enabled; }
    pub fn auto_regenerate(&self) -> bool                { self.auto_regenerate }

    // ── Seed management ───────────────────────────────────────────────────

    pub fn set_universe_seed(&mut self, seed: u64) {
        self.seed_context.set_universe_seed(seed);
        self.result_valid = false;
    }

    pub fn universe_seed(&self) -> u64 { self.seed_context.universe_seed() }

    // ── Regeneration ──────────────────────────────────────────────────────

    /// Regenerate if result is invalid; return last result reference.
    pub fn regenerate(&mut self) -> Option<&PcgGenerationResult> {
        if !self.result_valid {
            self.do_regenerate();
        }
        if self.result_valid { self.last_result.as_ref() } else { None }
    }

    /// Always regenerate.
    pub fn force_regenerate(&mut self) -> Option<&PcgGenerationResult> {
        self.result_valid = false;
        self.do_regenerate();
        if self.result_valid { self.last_result.as_ref() } else { None }
    }

    /// Forward rule value edit to the bound ruleset, and auto-regen if enabled.
    pub fn set_rule_value(&mut self, key: &str, val: &str) -> bool {
        if let Some(rs) = self.ruleset.as_mut() {
            if rs.set_value(key, val) {
                if self.auto_regenerate {
                    self.result_valid = false;
                    self.do_regenerate();
                }
                return true;
            }
        }
        false
    }

    pub fn reset_rules(&mut self) {
        if let Some(rs) = self.ruleset.as_mut() {
            rs.reset_to_defaults();
        }
        if self.auto_regenerate {
            self.result_valid = false;
            self.do_regenerate();
        }
    }

    pub fn last_result(&self) -> Option<&PcgGenerationResult> { self.last_result.as_ref() }
    pub fn stats(&self)       -> &PcgPreviewStats              { &self.stats }
    pub fn is_result_valid(&self) -> bool                      { self.result_valid }

    // ── Private ───────────────────────────────────────────────────────────

    fn do_regenerate(&mut self) {
        let Some(rs) = &self.ruleset else { return };
        let svc = PcgGeneratorService::new();
        // Clone the ruleset ref to avoid borrow conflicts with seed_context
        let rs_clone = rs.as_ref().clone();
        let result = svc.generate(&rs_clone, &mut self.seed_context, "");
        self.result_valid = result.success;
        self.stats.generation_count += 1;
        self.stats.last_placement_count = result.placements.len() as u32;
        self.stats.has_result = result.success;
        self.last_result = Some(result);
    }
}

impl Default for PcgPreviewService {
    fn default() -> Self { Self::new() }
}

// ── Tests ─────────────────────────────────────────────────────────────────

#[cfg(test)]
mod tests {
    use super::*;
    use super::super::ruleset::{PcgRule, PcgRuleValueType};

    fn make_ruleset_with_rules() -> PcgRuleSet {
        let mut rs = PcgRuleSet::new("preview-test", "zone");
        let mut r1 = PcgRule::new("density", PcgRuleValueType::Float);
        r1.value = "1.0".into();
        r1.default_value = "1.0".into();
        rs.add_rule(r1);
        let mut r2 = PcgRule::new("count", PcgRuleValueType::Int);
        r2.value = "4".into();
        r2.default_value = "4".into();
        rs.add_rule(r2);
        rs
    }

    #[test]
    fn new_has_no_ruleset() {
        let svc = PcgPreviewService::new();
        assert!(!svc.has_ruleset());
        assert!(!svc.is_result_valid());
    }

    #[test]
    fn bind_ruleset_auto_regenerates() {
        let mut svc = PcgPreviewService::new();
        svc.bind_ruleset(make_ruleset_with_rules());
        assert!(svc.has_ruleset());
        assert!(svc.is_result_valid());
        assert!(svc.last_result().is_some());
    }

    #[test]
    fn clear_ruleset_invalidates_result() {
        let mut svc = PcgPreviewService::new();
        svc.bind_ruleset(make_ruleset_with_rules());
        svc.clear_ruleset();
        assert!(!svc.has_ruleset());
        assert!(!svc.is_result_valid());
    }

    #[test]
    fn force_regenerate_increments_stats() {
        let mut svc = PcgPreviewService::new();
        svc.bind_ruleset(make_ruleset_with_rules());
        let before = svc.stats().generation_count;
        svc.force_regenerate();
        assert_eq!(svc.stats().generation_count, before + 1);
    }

    #[test]
    fn set_universe_seed_invalidates_result() {
        let mut svc = PcgPreviewService::new();
        svc.bind_ruleset(make_ruleset_with_rules());
        assert!(svc.is_result_valid());
        svc.set_universe_seed(999);
        assert!(!svc.is_result_valid());
    }

    #[test]
    fn set_rule_value_and_auto_regen() {
        let mut rs = make_ruleset_with_rules();
        let mut r = PcgRule::new("placementTag", PcgRuleValueType::Tag);
        r.value = "tree".into();
        rs.add_rule(r);

        let mut svc = PcgPreviewService::new();
        svc.bind_ruleset(rs);
        // set_rule_value returns true for existing key
        assert!(svc.set_rule_value("placementTag", "rock"));
    }

    #[test]
    fn set_rule_value_returns_false_for_unknown_key() {
        let mut svc = PcgPreviewService::new();
        svc.bind_ruleset(make_ruleset_with_rules());
        assert!(!svc.set_rule_value("nonexistent", "val"));
    }

    #[test]
    fn auto_regenerate_flag() {
        let mut svc = PcgPreviewService::new();
        svc.set_auto_regenerate(false);
        assert!(!svc.auto_regenerate());
        svc.set_auto_regenerate(true);
        assert!(svc.auto_regenerate());
    }
}
