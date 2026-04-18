#pragma once
// NovaForge::NovaForgeAssetPreviewBinder — document-to-asset-preview translation layer.
//
// Translates asset document state into NovaForgeAssetPreview so that the
// AssetEditorTool viewport reflects the live document content.
//
// The binder is a pure translation bridge — it owns NO state.
// AssetEditorTool creates and holds both the document and the preview;
// this class wires them together.

#include "NovaForge/EditorAdapter/NovaForgeAssetPreview.h"
#include <string>

namespace NovaForge {

class NovaForgeAssetPreviewBinder {
public:
    NovaForgeAssetPreviewBinder()  = default;
    ~NovaForgeAssetPreviewBinder() = default;

    // ── Binding ───────────────────────────────────────────────────────────────

    /// Attach the asset preview to push document state into.
    /// Pass nullptr to detach.
    void bindPreview(NovaForgeAssetPreview* preview) { m_preview = preview; }

    /// Attach the asset document source.
    /// Typed as void* until the asset document class hierarchy is stable.
    /// Replace with the concrete document type when available.
    void bindDocument(void* doc) { m_document = doc; }

    // ── Rebuild ───────────────────────────────────────────────────────────────

    /// Rebuild the preview from scratch using the bound document's current state.
    /// Should be called after bindDocument() or after a major document change
    /// (e.g., full asset reimport, undo/redo of a significant operation).
    void fullRebuild() {
        if (!m_preview || !m_document) return;
        // Stub: actual implementation translates document asset descriptor
        // into an AssetPreviewDescriptor and calls m_preview->bindAsset().
    }

    // ── Incremental updates ───────────────────────────────────────────────────

    /// Called when any property of the bound asset document changes.
    /// Pushes the updated field values into the preview without a full rebuild.
    void onAssetChanged() {
        if (!m_preview || !m_document) return;
        // Stub: actual implementation updates the affected preview field.
    }

private:
    NovaForgeAssetPreview* m_preview  = nullptr;
    void*                  m_document = nullptr;
};

} // namespace NovaForge
