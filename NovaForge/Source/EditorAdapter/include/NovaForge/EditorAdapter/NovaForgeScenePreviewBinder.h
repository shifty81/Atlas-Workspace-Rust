#pragma once
// NovaForge::NovaForgeScenePreviewBinder — document-to-preview translation layer.
//
// Listens to scene document changes and pushes them into NovaForgePreviewRuntime.
//
// Responsibilities:
//   - Accept a document pointer and a preview runtime pointer.
//   - Translate document entity state into preview world entities.
//   - Support full rebuild (initial load or document reset) and partial
//     field updates (transform, component edit, selection change).
//
// This class owns NO state — it is a pure translation bridge.
// The runtime and document pointers are non-owning; lifetimes are managed
// by the tool that creates and holds both objects.

#include "NovaForge/EditorAdapter/NovaForgePreviewRuntime.h"
#include <string>

namespace NovaForge {

class NovaForgeScenePreviewBinder {
public:
    NovaForgeScenePreviewBinder()  = default;
    ~NovaForgeScenePreviewBinder() = default;

    // ── Binding ───────────────────────────────────────────────────────────────

    /// Attach the preview runtime to push entity state into.
    /// Pass nullptr to detach.
    void bindRuntime(NovaForgePreviewRuntime* runtime) { m_runtime = runtime; }

    /// Attach the scene document source.
    /// Typed as void* until the document class hierarchy is stable.
    /// Replace with the concrete document type when available.
    void bindDocument(void* doc) { m_document = doc; }

    // ── Rebuild ───────────────────────────────────────────────────────────────

    /// Rebuild the entire preview world from the bound document.
    /// Should be called after bindDocument() and after significant document changes
    /// (e.g., scene load, undo/redo of a large operation).
    void fullRebuild() {
        if (!m_runtime || !m_document) return;
        m_runtime->rebuildFromDocument();
    }

    // ── Incremental updates ───────────────────────────────────────────────────

    /// Called when a single entity's transform or component data has changed.
    /// Pushes only the affected entity's state into the preview world.
    void onEntityChanged(EntityId id) {
        if (!m_runtime) return;
        NovaForgePreviewRuntime::PreviewTransform t;
        m_runtime->applyEntityChange(id, t);
    }

    /// Called when the document selection changes.
    /// Updates the preview world's selected entity so gizmos track the selection.
    void onSelectionChanged(EntityId id) {
        if (!m_runtime) return;
        m_runtime->applySelection(id);
    }

private:
    NovaForgePreviewRuntime* m_runtime  = nullptr;
    void*                    m_document = nullptr;
};

} // namespace NovaForge
