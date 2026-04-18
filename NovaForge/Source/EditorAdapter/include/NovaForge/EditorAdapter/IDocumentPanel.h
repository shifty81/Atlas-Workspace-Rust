#pragma once
// NovaForge::IDocumentPanel — interface for panels that edit a NovaForgeDocument.
//
// Extends IEditorPanel with document lifecycle: bind, dirty tracking, save/revert,
// validation, and a per-panel undo stack.
//
// Phase C.1 — Schema-Backed Panel Framework

#include "NF/Workspace/IGameProjectAdapter.h"
#include "NovaForge/EditorAdapter/NovaForgeDocument.h"
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace NovaForge {

// ── DocumentPanelValidationMessage ──────────────────────────────────────────

enum class DocumentPanelValidationSeverity : uint8_t {
    Info    = 0,
    Warning = 1,
    Error   = 2,
};

struct DocumentPanelValidationMessage {
    std::string                        field;
    std::string                        message;
    DocumentPanelValidationSeverity    severity = DocumentPanelValidationSeverity::Info;
};

// ── PanelUndoEntry ────────────────────────────────────────────────────────
// Lightweight per-panel undo/redo entry. Self-contained — no dependency on
// NF::UndoStack / NF::UndoActionType, which have conflicting definitions in
// UndoRedoSystem.h vs WorkspaceUndoRedo.h.

struct PanelUndoEntry {
    using Handler = std::function<bool()>;
    std::string label;
    Handler     doFn;
    Handler     undoFn;
};

// ── PanelUndoStack ────────────────────────────────────────────────────────

class PanelUndoStack {
public:
    void push(PanelUndoEntry entry) {
        // Trim redo future
        if (m_cursor < m_stack.size())
            m_stack.erase(m_stack.begin() + static_cast<ptrdiff_t>(m_cursor), m_stack.end());
        m_stack.push_back(std::move(entry));
        m_cursor = m_stack.size();
    }

    bool undo() {
        if (m_cursor == 0) return false;
        --m_cursor;
        return m_stack[m_cursor].undoFn ? m_stack[m_cursor].undoFn() : false;
    }

    bool redo() {
        if (m_cursor >= m_stack.size()) return false;
        bool ok = m_stack[m_cursor].doFn ? m_stack[m_cursor].doFn() : false;
        if (ok) ++m_cursor;
        return ok;
    }

    [[nodiscard]] bool canUndo() const { return m_cursor > 0; }
    [[nodiscard]] bool canRedo() const { return m_cursor < m_stack.size(); }
    [[nodiscard]] size_t undoDepth() const { return m_cursor; }
    [[nodiscard]] size_t redoDepth() const { return m_stack.size() - m_cursor; }

    void clear() { m_stack.clear(); m_cursor = 0; }

private:
    std::vector<PanelUndoEntry> m_stack;
    size_t                      m_cursor = 0;
};

// ── IDocumentPanel ───────────────────────────────────────────────────────────
// Base interface for all NovaForge panels that bind to a NovaForgeDocument.
// Panels implementing this interface support edit/save/revert lifecycle,
// per-panel undo stacks, and schema validation feedback.

class IDocumentPanel : public NF::IEditorPanel {
public:
    ~IDocumentPanel() override = default;

    // ── Document binding ──────────────────────────────────────────────────
    // Bind a document to this panel. The panel does NOT take ownership.
    // Passing nullptr unbinds the current document.
    virtual void bindDocument(NovaForgeDocument* doc) = 0;

    [[nodiscard]] virtual NovaForgeDocument* boundDocument() const = 0;
    [[nodiscard]] virtual bool hasDocument() const { return boundDocument() != nullptr; }

    // ── Dirty tracking ────────────────────────────────────────────────────
    // Returns true if the panel has unsaved edits (bound document is dirty).
    [[nodiscard]] virtual bool isDirty() const = 0;

    // ── Save / Revert ─────────────────────────────────────────────────────
    // save()   — validates and writes changes back to the document file path.
    //            Returns false if validation fails or write fails.
    // revert() — discards local edits and reloads from the document.
    //            Returns false if reload fails.
    virtual bool save()   = 0;
    virtual bool revert() = 0;

    // ── Validation ────────────────────────────────────────────────────────
    // Returns per-field validation messages. Empty list = no issues.
    [[nodiscard]] virtual std::vector<DocumentPanelValidationMessage> validate() const = 0;

    // ── Document change notification ──────────────────────────────────────
    // Called whenever the bound document's data changes externally
    // (e.g., reverted from disk, applied from another panel).
    virtual void onDocumentChanged() = 0;

    // ── Per-panel undo stack ──────────────────────────────────────────────
    [[nodiscard]] virtual PanelUndoStack& undoStack() = 0;
    [[nodiscard]] virtual const PanelUndoStack& undoStack() const = 0;

    // ── Dirty label ───────────────────────────────────────────────────────
    // Returns a display title with dirty indicator ("*" suffix if dirty).
    [[nodiscard]] virtual std::string dirtyTitle() const {
        return isDirty() ? panelTitle() + "*" : panelTitle();
    }

    // ── Change observer ───────────────────────────────────────────────────
    // Register a callback to be called whenever the panel marks itself dirty.
    using DirtyCallback = std::function<void(IDocumentPanel&)>;
    virtual void setOnDirtyCallback(DirtyCallback cb) = 0;
};

} // namespace NovaForge
