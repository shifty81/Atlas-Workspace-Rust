// SPDX-License-Identifier: GPL-3.0-only
// NovaForge PCG rule set — port of NovaForge::PCGRuleSet.
// Copyright (C) NovaForge contributors. GNU General Public License v3.0.

// ── PcgRuleValueType ──────────────────────────────────────────────────────

#[derive(Debug, Clone, Copy, PartialEq, Eq, Default)]
pub enum PcgRuleValueType {
    #[default]
    Float,
    Int,
    Bool,
    String,
    Vec2,
    Vec3,
    Range,
    Tag,
}

pub fn pcg_rule_value_type_name(t: PcgRuleValueType) -> &'static str {
    match t {
        PcgRuleValueType::Float  => "Float",
        PcgRuleValueType::Int    => "Int",
        PcgRuleValueType::Bool   => "Bool",
        PcgRuleValueType::String => "String",
        PcgRuleValueType::Vec2   => "Vec2",
        PcgRuleValueType::Vec3   => "Vec3",
        PcgRuleValueType::Range  => "Range",
        PcgRuleValueType::Tag    => "Tag",
    }
}

// ── PcgRule ───────────────────────────────────────────────────────────────

#[derive(Debug, Clone)]
pub struct PcgRule {
    pub key:           String,
    pub value_type:    PcgRuleValueType,
    pub value:         String,
    pub default_value: String,
    pub category:      String,
    pub description:   String,
    pub read_only:     bool,
}

impl PcgRule {
    pub fn new(key: impl Into<String>, value_type: PcgRuleValueType) -> Self {
        Self {
            key: key.into(),
            value_type,
            value: String::new(),
            default_value: String::new(),
            category: String::new(),
            description: String::new(),
            read_only: false,
        }
    }
}

// ── PcgRuleSet ────────────────────────────────────────────────────────────

pub struct PcgRuleSet {
    id:      String,
    domain:  String,
    name:    String,
    version: String,
    rules:   Vec<PcgRule>,
    dirty:   bool,
}

impl PcgRuleSet {
    pub const MAX_RULES: usize = 512;

    pub fn new(id: impl Into<String>, domain: impl Into<String>) -> Self {
        Self {
            id:      id.into(),
            domain:  domain.into(),
            name:    String::new(),
            version: "1.0".into(),
            rules:   Vec::new(),
            dirty:   false,
        }
    }

    // ── Identity ──────────────────────────────────────────────────────────

    pub fn id(&self)      -> &str { &self.id }
    pub fn domain(&self)  -> &str { &self.domain }
    pub fn name(&self)    -> &str { &self.name }
    pub fn version(&self) -> &str { &self.version }

    pub fn set_id(&mut self, id: impl Into<String>)           { self.id      = id.into(); }
    pub fn set_domain(&mut self, domain: impl Into<String>)   { self.domain  = domain.into(); }
    pub fn set_name(&mut self, name: impl Into<String>)       { self.name    = name.into(); }
    pub fn set_version(&mut self, version: impl Into<String>) { self.version = version.into(); }

    // ── Rule management ───────────────────────────────────────────────────

    /// Returns false if key is duplicate or capacity reached.
    pub fn add_rule(&mut self, rule: PcgRule) -> bool {
        if rule.key.is_empty() || self.rules.len() >= Self::MAX_RULES {
            return false;
        }
        if self.rules.iter().any(|r| r.key == rule.key) {
            return false;
        }
        self.rules.push(rule);
        self.dirty = true;
        true
    }

    /// Returns false if key not found or rule is read-only.
    pub fn set_value(&mut self, key: &str, value: impl Into<String>) -> bool {
        if let Some(r) = self.rules.iter_mut().find(|r| r.key == key) {
            if r.read_only {
                return false;
            }
            r.value = value.into();
            self.dirty = true;
            true
        } else {
            false
        }
    }

    pub fn remove_rule(&mut self, key: &str) -> bool {
        if let Some(pos) = self.rules.iter().position(|r| r.key == key) {
            self.rules.remove(pos);
            self.dirty = true;
            true
        } else {
            false
        }
    }

    pub fn find_rule(&self, key: &str) -> Option<&PcgRule> {
        self.rules.iter().find(|r| r.key == key)
    }

    pub fn find_rule_mut(&mut self, key: &str) -> Option<&mut PcgRule> {
        self.rules.iter_mut().find(|r| r.key == key)
    }

    pub fn get_value<'a>(&'a self, key: &str, fallback: &'a str) -> &'a str {
        self.rules.iter().find(|r| r.key == key)
            .map(|r| r.value.as_str())
            .unwrap_or(fallback)
    }

    pub fn has_rule(&self, key: &str) -> bool {
        self.rules.iter().any(|r| r.key == key)
    }

    pub fn reset_to_defaults(&mut self) {
        for r in &mut self.rules {
            r.value = r.default_value.clone();
        }
        self.dirty = true;
    }

    pub fn reset_rule(&mut self, key: &str) -> bool {
        if let Some(r) = self.rules.iter_mut().find(|r| r.key == key) {
            r.value = r.default_value.clone();
            self.dirty = true;
            true
        } else {
            false
        }
    }

    pub fn rules(&self) -> &[PcgRule] { &self.rules }
    pub fn rule_count(&self) -> usize  { self.rules.len() }

    // ── Dirty tracking ────────────────────────────────────────────────────

    pub fn is_dirty(&self)  -> bool { self.dirty }
    pub fn clear_dirty(&mut self)   { self.dirty = false; }
}

impl Clone for PcgRuleSet {
    fn clone(&self) -> Self {
        Self {
            id:      self.id.clone(),
            domain:  self.domain.clone(),
            name:    self.name.clone(),
            version: self.version.clone(),
            rules:   self.rules.clone(),
            dirty:   self.dirty,
        }
    }
}

impl Default for PcgRuleSet {
    fn default() -> Self {
        Self::new("", "")
    }
}

// ── Tests ─────────────────────────────────────────────────────────────────

#[cfg(test)]
mod tests {
    use super::*;

    fn make_ruleset() -> PcgRuleSet {
        PcgRuleSet::new("test-id", "forest")
    }

    fn density_rule() -> PcgRule {
        let mut r = PcgRule::new("density", PcgRuleValueType::Float);
        r.value = "1.0".into();
        r.default_value = "1.0".into();
        r
    }

    #[test]
    fn identity_fields() {
        let rs = make_ruleset();
        assert_eq!(rs.id(), "test-id");
        assert_eq!(rs.domain(), "forest");
        assert_eq!(rs.version(), "1.0");
    }

    #[test]
    fn add_rule_and_find() {
        let mut rs = make_ruleset();
        assert!(rs.add_rule(density_rule()));
        assert!(rs.has_rule("density"));
        assert_eq!(rs.get_value("density", "0"), "1.0");
    }

    #[test]
    fn add_duplicate_key_fails() {
        let mut rs = make_ruleset();
        assert!(rs.add_rule(density_rule()));
        assert!(!rs.add_rule(density_rule()));
    }

    #[test]
    fn set_value_updates_rule() {
        let mut rs = make_ruleset();
        rs.add_rule(density_rule());
        assert!(rs.set_value("density", "2.5"));
        assert_eq!(rs.get_value("density", ""), "2.5");
    }

    #[test]
    fn set_value_read_only_fails() {
        let mut rs = make_ruleset();
        let mut r = density_rule();
        r.read_only = true;
        rs.add_rule(r);
        assert!(!rs.set_value("density", "99"));
    }

    #[test]
    fn remove_rule() {
        let mut rs = make_ruleset();
        rs.add_rule(density_rule());
        assert!(rs.remove_rule("density"));
        assert!(!rs.has_rule("density"));
        assert!(!rs.remove_rule("density"));
    }

    #[test]
    fn reset_to_defaults() {
        let mut rs = make_ruleset();
        rs.add_rule(density_rule());
        rs.set_value("density", "5.0");
        rs.reset_to_defaults();
        assert_eq!(rs.get_value("density", ""), "1.0");
    }

    #[test]
    fn dirty_tracking() {
        let mut rs = make_ruleset();
        assert!(!rs.is_dirty());
        rs.add_rule(density_rule());
        assert!(rs.is_dirty());
        rs.clear_dirty();
        assert!(!rs.is_dirty());
    }

    #[test]
    fn rule_count() {
        let mut rs = make_ruleset();
        assert_eq!(rs.rule_count(), 0);
        rs.add_rule(density_rule());
        assert_eq!(rs.rule_count(), 1);
    }
}
