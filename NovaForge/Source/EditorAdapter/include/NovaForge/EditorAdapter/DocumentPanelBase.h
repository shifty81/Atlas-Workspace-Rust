#pragma once
// NovaForge::DocumentPanelBase — CRTP-free base class for IDocumentPanel implementations.
//
// Provides the common per-panel undo stack, dirty tracking, document binding,
// and save/revert scaffolding so concrete panels only override data-specific hooks.
//
// Phase C.1 — Schema-Backed Panel Framework

#include "NovaForge/EditorAdapter/IDocumentPanel.h"
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace NovaForge {

class DocumentPanelBase : public IDocumentPanel {
public:
    // ── IEditorPanel identity (concrete panels supply these) ─────────────

    // ── Document binding ──────────────────────────────────────────────────
    void bindDocument(NovaForgeDocument* doc) override {
        m_document = doc;
        if (doc) {
            loadFromDocument(*doc);
            m_undoStack.clear();
        }
        onDocumentChanged();
    }

    [[nodiscard]] NovaForgeDocument* boundDocument() const override { return m_document; }

    // ── Dirty tracking ────────────────────────────────────────────────────
    [[nodiscard]] bool isDirty() const override {
        return m_document ? m_document->isDirty() : false;
    }

    // ── Save / Revert ─────────────────────────────────────────────────────
    bool save() override {
        if (!m_document) return false;
        auto msgs = validate();
        for (const auto& m : msgs) {
            if (m.severity == DocumentPanelValidationSeverity::Error) return false;
        }
        applyToDocument(*m_document);
        bool ok = m_document->save();
        if (ok) {
            m_undoStack.clear();
            if (m_onDirty) m_onDirty(*this);
        }
        return ok;
    }

    bool revert() override {
        if (!m_document) return false;
        bool ok = m_document->revert();
        if (ok) {
            loadFromDocument(*m_document);
            m_undoStack.clear();
            onDocumentChanged();
        }
        return ok;
    }

    // ── Validation — override in subclass ─────────────────────────────────
    [[nodiscard]] std::vector<DocumentPanelValidationMessage> validate() const override {
        return {};
    }

    // ── Document change notification ──────────────────────────────────────
    void onDocumentChanged() override {}

    // ── Per-panel undo stack ──────────────────────────────────────────────
    [[nodiscard]] PanelUndoStack& undoStack() override { return m_undoStack; }
    [[nodiscard]] const PanelUndoStack& undoStack() const override { return m_undoStack; }

    // ── Dirty callback ────────────────────────────────────────────────────
    void setOnDirtyCallback(DirtyCallback cb) override { m_onDirty = std::move(cb); }

    // ── IEditorPanel lifecycle ────────────────────────────────────────────
    void onProjectLoaded(const std::string& projectRoot) override {
        m_projectRoot = projectRoot;
        m_ready = true;
    }
    void onProjectUnloaded() override {
        m_projectRoot.clear();
        m_ready = false;
        bindDocument(nullptr);
    }
    void update(float /*dt*/) override {}
    [[nodiscard]] bool isReady() const override { return m_ready; }

    [[nodiscard]] const std::string& projectRoot() const { return m_projectRoot; }

protected:
    // ── Hooks for subclasses ──────────────────────────────────────────────

    // Called when a document is bound — subclass populates local fields from doc.
    virtual void loadFromDocument(const NovaForgeDocument& /*doc*/) {}

    // Called by save() before writing — subclass writes local fields into doc.
    virtual void applyToDocument(NovaForgeDocument& /*doc*/) {}

    // ── Helpers ───────────────────────────────────────────────────────────

    // Mark the bound document dirty and fire the callback.
    void markFieldChanged() {
        if (m_document) {
            m_document->markDirty();
            if (m_onDirty) m_onDirty(*this);
        }
    }

    // Push a property-change undo action.
    template<typename T>
    void pushPropertyEdit(const std::string& label, T& field, T oldVal, T newVal) {
        field = newVal;
        markFieldChanged();
        m_undoStack.push(PanelUndoEntry{
            label,
            [this, &field, newVal]() { field = newVal; markFieldChanged(); return true; },
            [this, &field, oldVal]() { field = oldVal; markFieldChanged(); return true; }
        });
    }

    NovaForgeDocument* m_document   = nullptr;
    PanelUndoStack     m_undoStack;
    DirtyCallback      m_onDirty;
    std::string        m_projectRoot;
    bool               m_ready      = false;
};

} // namespace NovaForge
