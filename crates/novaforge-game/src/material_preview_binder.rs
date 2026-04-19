// SPDX-License-Identifier: GPL-3.0-only
// NovaForge material preview binder — port of NovaForge::NovaForgeMaterialPreviewBinder.
// Copyright (C) NovaForge contributors. GNU General Public License v3.0.

pub struct NovaForgeMaterialPreviewBinder {
    preview_bound:  bool,
    document_bound: bool,
}

impl NovaForgeMaterialPreviewBinder {
    pub fn new() -> Self {
        Self { preview_bound: false, document_bound: false }
    }

    pub fn bind_preview(&mut self, has_preview: bool) { self.preview_bound = has_preview; }
    pub fn bind_document(&mut self, has_doc: bool) { self.document_bound = has_doc; }
    pub fn has_preview(&self) -> bool { self.preview_bound }
    pub fn has_document(&self) -> bool { self.document_bound }
    pub fn is_bound(&self) -> bool { self.preview_bound && self.document_bound }

    pub fn full_rebuild(&self) -> bool { self.is_bound() }
    pub fn apply_parameter_change(&self, _name: &str) -> bool { self.is_bound() }
    pub fn apply_shader_change(&self) -> bool { self.is_bound() }
    pub fn apply_mesh_change(&self) -> bool { self.is_bound() }
}

impl Default for NovaForgeMaterialPreviewBinder {
    fn default() -> Self { Self::new() }
}

// ── Tests ─────────────────────────────────────────────────────────────────

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn not_bound_by_default() {
        let b = NovaForgeMaterialPreviewBinder::new();
        assert!(!b.is_bound());
    }

    #[test]
    fn bind_both_is_bound() {
        let mut b = NovaForgeMaterialPreviewBinder::new();
        b.bind_preview(true);
        b.bind_document(true);
        assert!(b.is_bound());
    }

    #[test]
    fn full_rebuild_requires_both() {
        let mut b = NovaForgeMaterialPreviewBinder::new();
        assert!(!b.full_rebuild());
        b.bind_preview(true);
        b.bind_document(true);
        assert!(b.full_rebuild());
    }

    #[test]
    fn apply_parameter_change_when_bound() {
        let mut b = NovaForgeMaterialPreviewBinder::new();
        b.bind_preview(true);
        b.bind_document(true);
        assert!(b.apply_parameter_change("roughness"));
    }

    #[test]
    fn apply_shader_change_when_unbound() {
        let b = NovaForgeMaterialPreviewBinder::new();
        assert!(!b.apply_shader_change());
    }
}
