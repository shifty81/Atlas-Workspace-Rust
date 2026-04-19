// SPDX-License-Identifier: GPL-3.0-only
// NovaForge PCG generator service — port of NovaForge::PCGGeneratorService.
// Copyright (C) NovaForge contributors. GNU General Public License v3.0.

use super::ruleset::PcgRuleSet;
use super::seed_context::PcgDeterministicSeedContext;

// ── PcgPlacement ──────────────────────────────────────────────────────────

#[derive(Debug, Clone)]
pub struct PcgPlacement {
    pub asset_tag:    String,
    pub x:            f32,
    pub y:            f32,
    pub z:            f32,
    pub yaw:          f32,
    pub scale:        f32,
    pub material_tag: String,
    pub pcg_tag:      String,
}

// ── PcgGenerationResult ───────────────────────────────────────────────────

#[derive(Debug, Clone)]
pub struct PcgGenerationResult {
    pub success:         bool,
    pub seed:            u32,
    pub ruleset_id:      String,
    pub domain:          String,
    pub placements:      Vec<PcgPlacement>,
    pub generated_count: u32,
    pub culled_count:    u32,
    pub error_message:   String,
}

impl Default for PcgGenerationResult {
    fn default() -> Self {
        Self {
            success:         false,
            seed:            0,
            ruleset_id:      String::new(),
            domain:          String::new(),
            placements:      Vec::new(),
            generated_count: 0,
            culled_count:    0,
            error_message:   String::new(),
        }
    }
}

// ── PcgValidationResult ───────────────────────────────────────────────────

#[derive(Debug, Clone)]
pub struct PcgValidationResult {
    pub valid:         bool,
    pub missing_rules: Vec<String>,
    pub warnings:      Vec<String>,
}

// ── PcgGeneratorService ───────────────────────────────────────────────────

pub struct PcgGeneratorService;

impl PcgGeneratorService {
    pub fn new() -> Self { Self }

    pub fn generate(
        &self,
        ruleset:        &PcgRuleSet,
        seed_ctx:       &mut PcgDeterministicSeedContext,
        domain_override: &str,
    ) -> PcgGenerationResult {
        let mut result = PcgGenerationResult::default();
        result.ruleset_id = ruleset.id().to_string();
        result.domain = if domain_override.is_empty() {
            ruleset.domain().to_string()
        } else {
            domain_override.to_string()
        };
        result.seed = (seed_ctx.seed_for_domain(&result.domain) & 0xFFFF_FFFF) as u32;

        if ruleset.rule_count() == 0 {
            result.success = true;
            return result;
        }

        let density_str   = ruleset.get_value("density", "1.0");
        let count_str     = ruleset.get_value("count", "10");
        let placement_tag = ruleset.get_value("placementTag", "default").to_string();
        let material_tag  = ruleset.get_value("materialTag", "mat/default").to_string();

        let density: f32 = density_str.parse().unwrap_or(1.0);
        let count: i32   = count_str.parse().unwrap_or(10);
        let actual = ((count as f32) * density) as i32;

        let min_scale_str = ruleset.get_value("minScale", "0.8");
        let max_scale_str = ruleset.get_value("maxScale", "1.2");
        let min_s: f32 = min_scale_str.parse().unwrap_or(0.8);
        let max_s: f32 = max_scale_str.parse().unwrap_or(1.2);

        let mut rng: u64 = result.seed as u64 | 1;

        let mut xnext = || -> u32 {
            rng ^= rng << 13;
            rng ^= rng >> 7;
            rng ^= rng << 17;
            (rng & 0xFFFF_FFFF) as u32
        };

        result.generated_count = actual.max(0) as u32;

        for _ in 0..actual.max(0) {
            let to_f = |v: u32, scale: f32| -> f32 {
                ((v % 10_000) as f32 / 10_000.0 - 0.5) * scale
            };
            let x = to_f(xnext(), 100.0);
            let z = to_f(xnext(), 100.0);
            let yaw = (xnext() % 360) as f32;
            let t = (xnext() % 1000) as f32 / 1000.0;
            let scale = min_s + t * (max_s - min_s);

            result.placements.push(PcgPlacement {
                asset_tag:    placement_tag.clone(),
                x,
                y:            0.0,
                z,
                yaw,
                scale,
                material_tag: material_tag.clone(),
                pcg_tag:      placement_tag.clone(),
            });
        }

        result.success = true;
        result
    }

    pub fn validate(ruleset: &PcgRuleSet) -> PcgValidationResult {
        let mut r = PcgValidationResult {
            valid:         true,
            missing_rules: Vec::new(),
            warnings:      Vec::new(),
        };

        for key in &["density", "count", "placementTag"] {
            if !ruleset.has_rule(key) {
                r.warnings.push(format!("Missing recommended rule: {}", key));
            }
        }

        if ruleset.id().is_empty() {
            r.valid = false;
            r.missing_rules.push("id".to_string());
        }

        r
    }
}

impl Default for PcgGeneratorService {
    fn default() -> Self { Self::new() }
}

// ── Tests ─────────────────────────────────────────────────────────────────

#[cfg(test)]
mod tests {
    use super::*;
    use super::super::ruleset::{PcgRule, PcgRuleValueType};

    fn make_seed_ctx() -> PcgDeterministicSeedContext {
        PcgDeterministicSeedContext::new(12345)
    }

    fn rule(key: &str, val: &str) -> PcgRule {
        let mut r = PcgRule::new(key, PcgRuleValueType::Float);
        r.value = val.into();
        r.default_value = val.into();
        r
    }

    #[test]
    fn empty_ruleset_succeeds_with_zero_placements() {
        let rs = PcgRuleSet::new("id", "domain");
        let mut ctx = make_seed_ctx();
        let svc = PcgGeneratorService::new();
        let result = svc.generate(&rs, &mut ctx, "");
        assert!(result.success);
        assert_eq!(result.placements.len(), 0);
    }

    #[test]
    fn with_rules_generates_placements() {
        let mut rs = PcgRuleSet::new("test", "forest");
        rs.add_rule(rule("density", "1.0"));
        rs.add_rule(rule("count", "5"));
        rs.add_rule({ let mut r = PcgRule::new("placementTag", PcgRuleValueType::Tag); r.value = "tree".into(); r });
        rs.add_rule({ let mut r = PcgRule::new("materialTag", PcgRuleValueType::Tag); r.value = "mat/bark".into(); r });

        let mut ctx = make_seed_ctx();
        let svc = PcgGeneratorService::new();
        let result = svc.generate(&rs, &mut ctx, "");
        assert!(result.success);
        assert_eq!(result.placements.len(), 5);
    }

    #[test]
    fn deterministic_same_seed_same_result() {
        let mut rs = PcgRuleSet::new("det", "zone");
        rs.add_rule(rule("density", "1.0"));
        rs.add_rule(rule("count", "3"));

        let svc = PcgGeneratorService::new();
        let mut ctx1 = PcgDeterministicSeedContext::new(99999);
        let mut ctx2 = PcgDeterministicSeedContext::new(99999);
        let r1 = svc.generate(&rs, &mut ctx1, "");
        let r2 = svc.generate(&rs, &mut ctx2, "");
        assert_eq!(r1.placements.len(), r2.placements.len());
        for (a, b) in r1.placements.iter().zip(r2.placements.iter()) {
            assert_eq!(a.x, b.x);
            assert_eq!(a.z, b.z);
        }
    }

    #[test]
    fn validate_warns_on_missing_recommended_rules() {
        let rs = PcgRuleSet::new("my-id", "zone");
        let v = PcgGeneratorService::validate(&rs);
        assert!(v.valid);
        assert!(!v.warnings.is_empty());
    }

    #[test]
    fn validate_errors_on_empty_id() {
        let rs = PcgRuleSet::new("", "zone");
        let v = PcgGeneratorService::validate(&rs);
        assert!(!v.valid);
        assert!(v.missing_rules.contains(&"id".to_string()));
    }

    #[test]
    fn domain_override_used() {
        let rs = PcgRuleSet::new("id", "default-domain");
        let mut ctx = make_seed_ctx();
        let svc = PcgGeneratorService::new();
        let result = svc.generate(&rs, &mut ctx, "override-domain");
        assert_eq!(result.domain, "override-domain");
    }

    #[test]
    fn density_zero_produces_no_placements() {
        let mut rs = PcgRuleSet::new("id", "z");
        rs.add_rule(rule("density", "0.0"));
        rs.add_rule(rule("count", "10"));
        let mut ctx = make_seed_ctx();
        let svc = PcgGeneratorService::new();
        let result = svc.generate(&rs, &mut ctx, "");
        assert_eq!(result.placements.len(), 0);
    }
}
