// SPDX-License-Identifier: GPL-3.0-only
// NovaForge scene preview binder — port of NovaForge::NovaForgeScenePreviewBinder.
// Copyright (C) NovaForge contributors. GNU General Public License v3.0.

pub struct NovaForgeScenePreviewBinder {
    runtime_bound:  bool,
    document_bound: bool,
}

impl NovaForgeScenePreviewBinder {
    pub fn new() -> Self {
        Self { runtime_bound: false, document_bound: false }
    }

    pub fn bind_runtime(&mut self, has_runtime: bool) { self.runtime_bound = has_runtime; }
    pub fn bind_document(&mut self, has_doc: bool) { self.document_bound = has_doc; }
    pub fn has_runtime(&self) -> bool { self.runtime_bound }
    pub fn has_document(&self) -> bool { self.document_bound }
    pub fn is_bound(&self) -> bool { self.runtime_bound && self.document_bound }

    pub fn full_rebuild(&self) -> bool { self.is_bound() }

    pub fn apply_entity_change(&self, _name: &str) -> bool { self.is_bound() }
    pub fn apply_selection_change(&self, _name: &str) -> bool { self.is_bound() }
    pub fn apply_component_change(&self, _entity: &str, _component: &str) -> bool { self.is_bound() }
}

impl Default for NovaForgeScenePreviewBinder {
    fn default() -> Self { Self::new() }
}

// ── Tests ─────────────────────────────────────────────────────────────────

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn not_bound_by_default() {
        let b = NovaForgeScenePreviewBinder::new();
        assert!(!b.is_bound());
    }

    #[test]
    fn bind_both_is_bound() {
        let mut b = NovaForgeScenePreviewBinder::new();
        b.bind_runtime(true);
        b.bind_document(true);
        assert!(b.is_bound());
    }

    #[test]
    fn partial_bind_not_bound() {
        let mut b = NovaForgeScenePreviewBinder::new();
        b.bind_runtime(true);
        assert!(!b.is_bound());
    }

    #[test]
    fn full_rebuild_requires_both() {
        let mut b = NovaForgeScenePreviewBinder::new();
        assert!(!b.full_rebuild());
        b.bind_runtime(true);
        b.bind_document(true);
        assert!(b.full_rebuild());
    }

    #[test]
    fn apply_entity_change_when_bound() {
        let mut b = NovaForgeScenePreviewBinder::new();
        b.bind_runtime(true);
        b.bind_document(true);
        assert!(b.apply_entity_change("player"));
    }

    #[test]
    fn apply_component_change_when_unbound() {
        let b = NovaForgeScenePreviewBinder::new();
        assert!(!b.apply_component_change("player", "Health"));
    }
}
