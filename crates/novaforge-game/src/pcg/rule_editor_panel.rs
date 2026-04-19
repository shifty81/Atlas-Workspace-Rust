// SPDX-License-Identifier: GPL-3.0-only
// NovaForge ProcGen rule editor panel — port of NovaForge::ProcGenRuleEditorPanel.
// Copyright (C) NovaForge contributors. GNU General Public License v3.0.

use super::preview::PcgPreviewService;
use super::ruleset::PcgRuleSet;

// ── ProcGenSaveResult ─────────────────────────────────────────────────────

#[derive(Debug, Clone)]
pub struct ProcGenSaveResult {
    pub success:   bool,
    pub error_msg: String,
}

impl ProcGenSaveResult {
    pub fn ok(&self)     -> bool { self.success }
    pub fn failed(&self) -> bool { !self.success }
}

// ── ProcGenRuleEditorPanel ────────────────────────────────────────────────

pub struct ProcGenRuleEditorPanel {
    document:   Option<PcgRuleSet>,
    snapshot:   PcgRuleSet,
    preview:    Option<PcgPreviewService>,
    dirty:      bool,
    active:     bool,
    edit_count: u32,
    save_count: u32,
}

impl ProcGenRuleEditorPanel {
    pub fn new() -> Self {
        Self {
            document:   None,
            snapshot:   PcgRuleSet::default(),
            preview:    None,
            dirty:      false,
            active:     false,
            edit_count: 0,
            save_count: 0,
        }
    }

    // ── Document binding ──────────────────────────────────────────────────

    pub fn bind_document(&mut self, ruleset: PcgRuleSet) {
        self.snapshot = ruleset.clone();
        if let Some(preview) = &mut self.preview {
            preview.bind_ruleset(ruleset.clone());
        }
        self.document = Some(ruleset);
        self.dirty = false;
        self.edit_count = 0;
    }

    pub fn clear_document(&mut self) {
        if let Some(preview) = &mut self.preview {
            preview.clear_ruleset();
        }
        self.document = None;
        self.snapshot = PcgRuleSet::default();
        self.dirty = false;
        self.edit_count = 0;
    }

    pub fn has_document(&self) -> bool { self.document.is_some() }

    pub fn document(&self) -> Option<&PcgRuleSet> { self.document.as_ref() }

    // ── Preview service wiring ────────────────────────────────────────────

    pub fn attach_preview_service(&mut self, mut preview: PcgPreviewService) {
        if let Some(rs) = &self.document {
            preview.bind_ruleset(rs.clone());
        }
        self.preview = Some(preview);
    }

    pub fn detach_preview_service(&mut self) {
        if let Some(preview) = &mut self.preview {
            preview.clear_ruleset();
        }
        self.preview = None;
    }

    pub fn has_preview_service(&self) -> bool { self.preview.is_some() }

    // ── Rule editing ──────────────────────────────────────────────────────

    pub fn edit_rule(&mut self, key: &str, val: &str) -> bool {
        let Some(doc) = &mut self.document else { return false };
        if !doc.set_value(key, val) {
            return false;
        }
        self.dirty = true;
        self.edit_count += 1;
        if let Some(preview) = &mut self.preview {
            preview.set_rule_value(key, val);
        }
        true
    }

    pub fn reset_rule(&mut self, key: &str) -> bool {
        let Some(doc) = &mut self.document else { return false };
        if !doc.reset_rule(key) {
            return false;
        }
        self.dirty = true;
        self.edit_count += 1;
        let val = doc.get_value(key, "").to_string();
        if let Some(preview) = &mut self.preview {
            preview.set_rule_value(key, &val);
        }
        true
    }

    pub fn reset_all(&mut self) -> bool {
        let Some(doc) = &mut self.document else { return false };
        doc.reset_to_defaults();
        self.dirty = true;
        self.edit_count += 1;
        if let Some(preview) = &mut self.preview {
            preview.reset_rules();
        }
        true
    }

    // ── Save / Revert ─────────────────────────────────────────────────────

    pub fn save(&mut self) -> ProcGenSaveResult {
        let Some(doc) = &self.document else {
            return ProcGenSaveResult { success: false, error_msg: "No document bound".into() };
        };
        self.snapshot = doc.clone();
        self.dirty = false;
        self.save_count += 1;
        ProcGenSaveResult { success: true, error_msg: String::new() }
    }

    pub fn revert(&mut self) -> bool {
        let Some(doc) = &mut self.document else { return false };
        *doc = self.snapshot.clone();
        self.dirty = false;
        self.edit_count = 0;
        if let Some(preview) = &mut self.preview {
            preview.bind_ruleset(doc.clone());
        }
        true
    }

    // ── Dirty tracking / lifecycle ────────────────────────────────────────

    pub fn is_dirty(&self)   -> bool { self.dirty }
    pub fn edit_count(&self) -> u32  { self.edit_count }
    pub fn save_count(&self) -> u32  { self.save_count }

    pub fn activate(&mut self)   { self.active = true; }
    pub fn deactivate(&mut self) { self.active = false; }
    pub fn is_active(&self) -> bool { self.active }

    // ── Rule inspection ───────────────────────────────────────────────────

    pub fn rule_count(&self) -> usize {
        self.document.as_ref().map(|d| d.rule_count()).unwrap_or(0)
    }

    pub fn rule_value(&self, key: &str) -> Option<String> {
        self.document.as_ref().map(|d| d.get_value(key, "").to_string())
    }

    pub fn has_rule(&self, key: &str) -> bool {
        self.document.as_ref().map(|d| d.has_rule(key)).unwrap_or(false)
    }
}

impl Default for ProcGenRuleEditorPanel {
    fn default() -> Self { Self::new() }
}

// ── Tests ─────────────────────────────────────────────────────────────────

#[cfg(test)]
mod tests {
    use super::*;
    use super::super::ruleset::{PcgRule, PcgRuleValueType};

    fn make_ruleset() -> PcgRuleSet {
        let mut rs = PcgRuleSet::new("panel-test", "zone");
        let mut r = PcgRule::new("density", PcgRuleValueType::Float);
        r.value = "1.0".into();
        r.default_value = "0.5".into();
        rs.add_rule(r);
        rs
    }

    #[test]
    fn bind_document() {
        let mut panel = ProcGenRuleEditorPanel::new();
        panel.bind_document(make_ruleset());
        assert!(panel.has_document());
        assert!(!panel.is_dirty());
    }

    #[test]
    fn edit_rule_marks_dirty() {
        let mut panel = ProcGenRuleEditorPanel::new();
        panel.bind_document(make_ruleset());
        assert!(panel.edit_rule("density", "2.0"));
        assert!(panel.is_dirty());
        assert_eq!(panel.edit_count(), 1);
    }

    #[test]
    fn edit_rule_unknown_key_returns_false() {
        let mut panel = ProcGenRuleEditorPanel::new();
        panel.bind_document(make_ruleset());
        assert!(!panel.edit_rule("nonexistent", "val"));
    }

    #[test]
    fn reset_rule_reverts_to_default() {
        let mut panel = ProcGenRuleEditorPanel::new();
        panel.bind_document(make_ruleset());
        panel.edit_rule("density", "9.9");
        panel.reset_rule("density");
        assert_eq!(panel.rule_value("density").unwrap(), "0.5");
    }

    #[test]
    fn reset_all() {
        let mut panel = ProcGenRuleEditorPanel::new();
        panel.bind_document(make_ruleset());
        panel.edit_rule("density", "9.9");
        assert!(panel.reset_all());
        assert_eq!(panel.rule_value("density").unwrap(), "0.5");
    }

    #[test]
    fn save_clears_dirty() {
        let mut panel = ProcGenRuleEditorPanel::new();
        panel.bind_document(make_ruleset());
        panel.edit_rule("density", "3.0");
        let result = panel.save();
        assert!(result.ok());
        assert!(!panel.is_dirty());
        assert_eq!(panel.save_count(), 1);
    }

    #[test]
    fn revert_restores_snapshot() {
        let mut panel = ProcGenRuleEditorPanel::new();
        panel.bind_document(make_ruleset());
        panel.edit_rule("density", "99.0");
        assert!(panel.is_dirty());
        panel.revert();
        assert!(!panel.is_dirty());
        assert_eq!(panel.rule_value("density").unwrap(), "1.0");
    }

    #[test]
    fn without_document_save_fails() {
        let mut panel = ProcGenRuleEditorPanel::new();
        let result = panel.save();
        assert!(result.failed());
    }

    #[test]
    fn activate_deactivate() {
        let mut panel = ProcGenRuleEditorPanel::new();
        assert!(!panel.is_active());
        panel.activate();
        assert!(panel.is_active());
        panel.deactivate();
        assert!(!panel.is_active());
    }
}
