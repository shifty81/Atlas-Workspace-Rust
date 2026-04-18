#pragma once
// NovaForge::NovaForgeMaterialPreviewBinder — document-to-material-preview translation layer.
//
// Translates material document state into NovaForgeMaterialPreview so that the
// MaterialEditorTool viewport reflects the live material graph content.
//
// The binder is a pure translation bridge — it owns NO state.
// MaterialEditorTool creates and holds both the document and the preview;
// this class wires them together.

#include "NovaForge/EditorAdapter/NovaForgeMaterialPreview.h"
#include <string>

namespace NovaForge {

class NovaForgeMaterialPreviewBinder {
public:
    NovaForgeMaterialPreviewBinder()  = default;
    ~NovaForgeMaterialPreviewBinder() = default;

    // ── Binding ───────────────────────────────────────────────────────────────

    /// Attach the material preview to push document state into.
    /// Pass nullptr to detach.
    void bindPreview(NovaForgeMaterialPreview* preview) { m_preview = preview; }

    /// Attach the material document source.
    /// Typed as void* until the material document class hierarchy is stable.
    /// Replace with the concrete document type when available.
    void bindDocument(void* doc) { m_document = doc; }

    // ── Rebuild ───────────────────────────────────────────────────────────────

    /// Rebuild the preview from scratch using the bound document's current state.
    /// Should be called after bindDocument() or after a major document change
    /// (e.g., shader recompile, topology change in the material graph).
    void fullRebuild() {
        if (!m_preview || !m_document) return;
        // Stub: actual implementation translates the material graph nodes/pins
        // into material parameter key/value pairs and calls m_preview->setParameter().
    }

    // ── Incremental updates ───────────────────────────────────────────────────

    /// Called when a single parameter in the material document changes.
    /// @param paramName  The parameter key that was edited (e.g., "BaseColor.R").
    void onParameterChanged(const std::string& /*paramName*/) {
        if (!m_preview || !m_document) return;
        // Stub: actual implementation pushes the changed parameter to the preview.
    }

private:
    NovaForgeMaterialPreview* m_preview  = nullptr;
    void*                     m_document = nullptr;
};

} // namespace NovaForge
