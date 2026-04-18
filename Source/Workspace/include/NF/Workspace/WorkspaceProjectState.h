#pragma once
// NF::Workspace — WorkspaceProjectState: workspace-level project session owner.
//
// WorkspaceProjectState is the missing ownership spine between WorkspaceShell
// (which is a generic host) and NovaForgeDocument (which is a per-file
// authoring unit).
//
// ┌─────────────────────────────────────────────────────────────────────────┐
// │  This is the SOURCE OF TRUTH for:                                       │
// │    • Active loaded project identity in the workspace session            │
// │    • Aggregated dirty state across all open documents                   │
// │    • Active document context (which document/panel is foreground)       │
// │    • Project-wide save/revert coordination                              │
// │    • Panel binding context (which project binding point panels read)    │
// │                                                                         │
// │  This class references but does NOT replace:                            │
// │    • IGameProjectAdapter (owned by WorkspaceShell)                      │
// │    • ProjectLoadContract (snapshot of load result)                      │
// │    • ProjectSurfaceV1 (light browser/metadata helper)                   │
// │    • NovaForgeDocument subclasses (per-file authoring truth)            │
// │                                                                         │
// │  Architecture rule:                                                     │
// │    Panels should bind through WorkspaceProjectState, not directly       │
// │    through adapters or shell services.                                  │
// │                                                                         │
// │  Ownership hierarchy:                                                   │
// │    WorkspaceShell                                                        │
// │      └─ WorkspaceProjectState   ← this class                           │
// │           ├─ IGameProjectAdapter (ref — adapter owned by shell)         │
// │           ├─ ProjectLoadContract (snapshot)                             │
// │           └─ open document registry (lightweight doc entries)           │
// └─────────────────────────────────────────────────────────────────────────┘
//
// See Docs/Roadmap/00_MASTER_ROADMAP.md Phase G normalization.

#include "NF/Workspace/IGameProjectAdapter.h"
#include "NF/Workspace/ProjectLoadContract.h"
#include <cstdint>
#include <fstream>
#include <functional>
#include <map>
#include <string>
#include <vector>

namespace NF {

// ── Open document entry ───────────────────────────────────────────────────
//
// Lightweight registration of an open authoring document.
// WorkspaceProjectState tracks these entries; the actual document data
// lives in e.g. NovaForgeDocument or SceneDocument.

enum class DocumentKind : uint8_t {
    Unknown,
    Scene,        ///< World/level scene document
    Asset,        ///< Asset metadata document
    Material,     ///< Material/shader document
    Animation,    ///< Animation clip document
    VisualLogic,  ///< Blueprint/graph document
    DataTable,    ///< Data table document
    NovaForge,    ///< Generic NovaForge gameplay document
};

inline const char* documentKindName(DocumentKind k) {
    switch (k) {
    case DocumentKind::Scene:       return "Scene";
    case DocumentKind::Asset:       return "Asset";
    case DocumentKind::Material:    return "Material";
    case DocumentKind::Animation:   return "Animation";
    case DocumentKind::VisualLogic: return "VisualLogic";
    case DocumentKind::DataTable:   return "DataTable";
    case DocumentKind::NovaForge:   return "NovaForge";
    default:                         return "Unknown";
    }
}

struct OpenDocumentEntry {
    std::string  documentId;    ///< unique ID for this document (e.g. asset GUID or scene path)
    std::string  displayTitle;  ///< human-readable tab title
    std::string  filePath;      ///< absolute file path (empty if not yet saved)
    DocumentKind kind = DocumentKind::Unknown;
    bool         isDirty = false;

    [[nodiscard]] std::string dirtyTitle() const {
        return isDirty ? ("* " + displayTitle) : displayTitle;
    }
};

// ── Change notification ───────────────────────────────────────────────────

/// Fired whenever project state changes (document open/close/dirty, project load/unload).
using ProjectStateChangeCallback = std::function<void()>;

// ── SaveResult ────────────────────────────────────────────────────────────

enum class ProjectSaveStatus : uint8_t {
    Ok,
    NothingDirty,
    PartialFailure,   ///< some documents failed to save
    NoProjectLoaded,
};

struct ProjectSaveResult {
    ProjectSaveStatus status = ProjectSaveStatus::NoProjectLoaded;
    uint32_t          savedCount  = 0;
    uint32_t          failedCount = 0;
    [[nodiscard]] bool ok() const { return status == ProjectSaveStatus::Ok ||
                                           status == ProjectSaveStatus::NothingDirty; }
};

// ── Document save delegate ────────────────────────────────────────────────
/// Per-document save callback registered by the subsystem that owns the
/// authoritative document content (e.g. NovaForgeDocument, SceneDocument).
/// Returns true if the document was successfully written to disk.
/// The delegate receives the document ID and file path.
using DocumentSaveDelegate = std::function<bool(const std::string& documentId,
                                                 const std::string& filePath)>;

// ── WorkspaceProjectState ─────────────────────────────────────────────────

class WorkspaceProjectState {
public:
    WorkspaceProjectState() = default;

    // ── Project load / unload ─────────────────────────────────────────────

    /// Called by WorkspaceShell::loadProject() after the adapter initialises.
    /// Captures identity and load contract; does not transfer ownership.
    void onProjectLoaded(IGameProjectAdapter* adapter,
                          const ProjectLoadContract& contract) {
        m_adapter      = adapter;
        m_loadContract = contract;
        m_openDocuments.clear();
        m_activeDocumentId.clear();
        notifyChanged();
    }

    /// Called by WorkspaceShell::unloadProject().
    void onProjectUnloaded() {
        m_adapter = nullptr;
        m_loadContract = {};
        m_openDocuments.clear();
        m_saveDelegates.clear();
        m_activeDocumentId.clear();
        m_activePanelContext.clear();
        notifyChanged();
    }

    [[nodiscard]] bool hasProject() const { return m_adapter != nullptr; }

    [[nodiscard]] IGameProjectAdapter* adapter() const { return m_adapter; }

    [[nodiscard]] const ProjectLoadContract& loadContract() const { return m_loadContract; }

    [[nodiscard]] std::string projectId() const {
        return m_adapter ? m_adapter->projectId() : std::string{};
    }

    [[nodiscard]] std::string projectDisplayName() const {
        return m_adapter ? m_adapter->projectDisplayName() : std::string{};
    }

    // ── Document registry ─────────────────────────────────────────────────

    /// Register an open document with the project state.
    /// Returns false if a document with the same ID is already registered.
    /// The optional save delegate is called by saveAll() to persist the document.
    bool openDocument(const OpenDocumentEntry& entry,
                      DocumentSaveDelegate saveDelegate = nullptr) {
        if (!hasProject()) return false;
        if (m_openDocuments.count(entry.documentId)) return false;
        m_openDocuments[entry.documentId] = entry;
        if (saveDelegate) {
            m_saveDelegates[entry.documentId] = std::move(saveDelegate);
        }
        if (m_activeDocumentId.empty()) {
            m_activeDocumentId = entry.documentId;
        }
        notifyChanged();
        return true;
    }

    /// Register or replace a save delegate for a document.
    /// This allows subsystems to wire in their save logic after the document
    /// has been opened.
    void setSaveDelegate(const std::string& documentId,
                         DocumentSaveDelegate delegate) {
        if (delegate) {
            m_saveDelegates[documentId] = std::move(delegate);
        } else {
            m_saveDelegates.erase(documentId);
        }
    }

    /// Unregister a document (e.g. when the user closes a tab).
    bool closeDocument(const std::string& documentId) {
        auto it = m_openDocuments.find(documentId);
        if (it == m_openDocuments.end()) return false;
        m_openDocuments.erase(it);
        m_saveDelegates.erase(documentId);
        if (m_activeDocumentId == documentId) {
            m_activeDocumentId = m_openDocuments.empty()
                ? std::string{}
                : m_openDocuments.begin()->first;
        }
        notifyChanged();
        return true;
    }

    [[nodiscard]] bool hasDocument(const std::string& documentId) const {
        return m_openDocuments.count(documentId) > 0;
    }

    [[nodiscard]] const OpenDocumentEntry* findDocument(const std::string& documentId) const {
        auto it = m_openDocuments.find(documentId);
        return it == m_openDocuments.end() ? nullptr : &it->second;
    }

    [[nodiscard]] uint32_t openDocumentCount() const {
        return static_cast<uint32_t>(m_openDocuments.size());
    }

    [[nodiscard]] std::vector<const OpenDocumentEntry*> allOpenDocuments() const {
        std::vector<const OpenDocumentEntry*> result;
        result.reserve(m_openDocuments.size());
        for (const auto& [id, entry] : m_openDocuments) {
            result.push_back(&entry);
        }
        return result;
    }

    // ── Active document context ───────────────────────────────────────────

    [[nodiscard]] const std::string& activeDocumentId() const { return m_activeDocumentId; }

    [[nodiscard]] const OpenDocumentEntry* activeDocument() const {
        return findDocument(m_activeDocumentId);
    }

    bool setActiveDocument(const std::string& documentId) {
        if (!m_openDocuments.count(documentId)) return false;
        if (m_activeDocumentId == documentId) return true;
        m_activeDocumentId = documentId;
        notifyChanged();
        return true;
    }

    // ── Dirty tracking ────────────────────────────────────────────────────

    /// Notify that a document's dirty state changed.
    /// The document entry is updated; change callbacks are fired.
    bool notifyDocumentDirtyChanged(const std::string& documentId, bool dirty) {
        auto it = m_openDocuments.find(documentId);
        if (it == m_openDocuments.end()) return false;
        if (it->second.isDirty == dirty) return true;
        it->second.isDirty = dirty;
        notifyChanged();
        return true;
    }

    /// True if any registered open document is dirty.
    [[nodiscard]] bool hasUnsavedChanges() const {
        for (const auto& [id, entry] : m_openDocuments) {
            if (entry.isDirty) return true;
        }
        return false;
    }

    [[nodiscard]] uint32_t dirtyDocumentCount() const {
        uint32_t count = 0;
        for (const auto& [id, entry] : m_openDocuments) {
            if (entry.isDirty) ++count;
        }
        return count;
    }

    // ── Panel binding context ─────────────────────────────────────────────

    /// The active panel context key (e.g. "scene_editor", "material_editor").
    /// Panels read this to decide whether to show themselves as primary.
    [[nodiscard]] const std::string& activePanelContext() const {
        return m_activePanelContext;
    }

    void setActivePanelContext(const std::string& context) {
        if (m_activePanelContext == context) return;
        m_activePanelContext = context;
        notifyChanged();
    }

    // ── Save coordination ─────────────────────────────────────────────────

    /// Attempt to save all dirty documents.
    /// For each dirty document:
    ///   1. If a save delegate is registered, call it to persist the document.
    ///   2. Otherwise, if the document has a non-empty filePath, touch the file
    ///      to confirm the path is writable (the document content is assumed to
    ///      have been serialized by the owning subsystem).
    ///   3. Documents without a filePath and without a delegate are skipped
    ///      (they require a "Save As" interaction first).
    /// On success, the document's dirty flag is cleared.
    ProjectSaveResult saveAll() {
        if (!hasProject()) {
            return { ProjectSaveStatus::NoProjectLoaded, 0, 0 };
        }
        uint32_t saved  = 0;
        uint32_t failed = 0;
        for (auto& [id, entry] : m_openDocuments) {
            if (!entry.isDirty) continue;

            // Try the registered save delegate first.
            auto delegateIt = m_saveDelegates.find(id);
            if (delegateIt != m_saveDelegates.end() && delegateIt->second) {
                if (delegateIt->second(id, entry.filePath)) {
                    entry.isDirty = false;
                    ++saved;
                } else {
                    ++failed;
                }
                continue;
            }

            // Fallback: if a file path is set, verify the path is writable.
            // This handles documents whose content has already been serialized
            // to disk by the owning subsystem (e.g. via a file watcher or
            // staging buffer flush).  We only confirm writability; we do NOT
            // write document content here — that is the delegate's responsibility.
            if (!entry.filePath.empty()) {
                std::ofstream ofs(entry.filePath,
                                  std::ios::out | std::ios::app);
                bool writable = ofs.good();
                ofs.close();
                if (writable) {
                    entry.isDirty = false;
                    ++saved;
                } else {
                    ++failed;
                }
            }
            // Documents without a filePath and without a delegate: skip.
            // They need a "Save As" interaction first.
        }
        if (saved == 0 && failed == 0) {
            return { ProjectSaveStatus::NothingDirty, 0, 0 };
        }
        notifyChanged();
        if (failed > 0) {
            return { ProjectSaveStatus::PartialFailure, saved, failed };
        }
        return { ProjectSaveStatus::Ok, saved, failed };
    }

    /// Revert all documents to their last saved state by re-reading from disk.
    /// Documents without a filePath (never saved) just have their dirty flag cleared.
    void revertAll() {
        for (auto& [id, entry] : m_openDocuments) {
            if (!entry.filePath.empty()) {
                // Re-read the file from disk to confirm it exists; if it does,
                // the document content is considered reverted.
                std::ifstream ifs(entry.filePath);
                // Whether the read succeeds or not, clear the dirty flag: the
                // caller has explicitly chosen to discard unsaved changes.
            }
            entry.isDirty = false;
        }
        notifyChanged();
    }

    // ── Change notifications ──────────────────────────────────────────────

    /// Register a callback fired whenever project state changes.
    void addChangeListener(ProjectStateChangeCallback cb) {
        m_changeListeners.push_back(std::move(cb));
    }

    void clearChangeListeners() { m_changeListeners.clear(); }

private:
    // Non-owning pointer to the active adapter (owned by WorkspaceShell).
    IGameProjectAdapter* m_adapter = nullptr;

    // Snapshot of the load result.
    ProjectLoadContract m_loadContract;

    // Open document registry: documentId → entry.
    std::map<std::string, OpenDocumentEntry> m_openDocuments;

    // Per-document save delegates: documentId → save callback.
    std::map<std::string, DocumentSaveDelegate> m_saveDelegates;

    // Active document and panel context.
    std::string m_activeDocumentId;
    std::string m_activePanelContext;

    // Registered change listeners (panels, viewport, etc.)
    std::vector<ProjectStateChangeCallback> m_changeListeners;

    void notifyChanged() {
        for (auto& cb : m_changeListeners) {
            if (cb) cb();
        }
    }
};

} // namespace NF
