// SPDX-License-Identifier: GPL-3.0-only
// NovaForge asset preview binder — port of NovaForge::NovaForgeAssetPreviewBinder.
// Copyright (C) NovaForge contributors. GNU General Public License v3.0.

pub struct NovaForgeAssetPreviewBinder {
    preview_bound:  bool,
    document_bound: bool,
}

impl NovaForgeAssetPreviewBinder {
    pub fn new() -> Self {
        Self { preview_bound: false, document_bound: false }
    }

    pub fn bind_preview(&mut self, has_preview: bool) { self.preview_bound = has_preview; }
    pub fn bind_document(&mut self, has_doc: bool) { self.document_bound = has_doc; }
    pub fn has_preview(&self) -> bool { self.preview_bound }
    pub fn has_document(&self) -> bool { self.document_bound }
    pub fn is_bound(&self) -> bool { self.preview_bound && self.document_bound }

    pub fn full_rebuild(&self) -> bool { self.is_bound() }
    pub fn apply_transform_change(&self) -> bool { self.is_bound() }
    pub fn apply_mesh_change(&self) -> bool { self.is_bound() }
    pub fn apply_material_change(&self) -> bool { self.is_bound() }
}

impl Default for NovaForgeAssetPreviewBinder {
    fn default() -> Self { Self::new() }
}

// ── Tests ─────────────────────────────────────────────────────────────────

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn not_bound_by_default() {
        let b = NovaForgeAssetPreviewBinder::new();
        assert!(!b.is_bound());
    }

    #[test]
    fn bind_both_is_bound() {
        let mut b = NovaForgeAssetPreviewBinder::new();
        b.bind_preview(true);
        b.bind_document(true);
        assert!(b.is_bound());
    }

    #[test]
    fn full_rebuild_requires_both() {
        let mut b = NovaForgeAssetPreviewBinder::new();
        assert!(!b.full_rebuild());
        b.bind_preview(true);
        b.bind_document(true);
        assert!(b.full_rebuild());
    }

    #[test]
    fn apply_mesh_change_when_bound() {
        let mut b = NovaForgeAssetPreviewBinder::new();
        b.bind_preview(true);
        b.bind_document(true);
        assert!(b.apply_mesh_change());
    }

    #[test]
    fn apply_material_change_when_unbound() {
        let b = NovaForgeAssetPreviewBinder::new();
        assert!(!b.apply_material_change());
    }
}
