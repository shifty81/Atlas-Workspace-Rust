#pragma once
// NovaForge::ProcGenRuleEditorPanel — panel-level wiring for PCG rule editing.
//
// Bridges a PCGRuleSet document (from project data) to the PCGPreviewService
// so that rule edits are reflected live in the viewport preview.
//
// Responsibilities:
//   - Bind a PCGRuleSet loaded from project data (bindDocument / clearDocument)
//   - Forward rule edits from the panel UI (editRule / resetRule / resetAll)
//   - Save-back: translate dirty rule state to project data (saveToDocument)
//   - Revert: discard edits and reload the bound document (revert)
//   - Dirty tracking: panel tracks unsaved changes
//   - Lifecycle: activate/deactivate like a hosted panel
//
// Phase E.3 — ProcGenRuleEditorPanel wiring

#include "NovaForge/EditorAdapter/PCGRuleSet.h"
#include "NovaForge/EditorAdapter/PCGPreviewService.h"
#include <functional>
#include <string>
#include <vector>

namespace NovaForge {

// ── SaveResult ────────────────────────────────────────────────────────────────

struct ProcGenSaveResult {
    bool        success   = false;
    std::string errorMsg;

    [[nodiscard]] bool ok()   const { return success; }
    [[nodiscard]] bool failed() const { return !success; }
};

// ── ProcGenRuleEditorPanel ────────────────────────────────────────────────────

class ProcGenRuleEditorPanel {
public:
    ProcGenRuleEditorPanel()  = default;
    ~ProcGenRuleEditorPanel() = default;

    // ── Document binding ──────────────────────────────────────────────────

    /// Bind a PCGRuleSet document to the panel.
    /// Takes a snapshot of the document for revert purposes and forwards the
    /// live ruleset to the attached PCGPreviewService.
    void bindDocument(PCGRuleSet* ruleSet) {
        m_document       = ruleSet;
        m_dirty          = false;
        m_editCount      = 0;
        if (ruleSet) {
            m_snapshot = *ruleSet; // capture for revert
        } else {
            m_snapshot = PCGRuleSet{};
        }
        if (m_preview) m_preview->bindRuleSet(ruleSet);
    }

    void clearDocument() { bindDocument(nullptr); }

    [[nodiscard]] bool     hasDocument() const { return m_document != nullptr; }
    [[nodiscard]] PCGRuleSet* document() const { return m_document; }

    // ── Preview service wiring ────────────────────────────────────────────

    void attachPreviewService(PCGPreviewService* preview) {
        m_preview = preview;
        if (m_preview && m_document) m_preview->bindRuleSet(m_document);
    }

    void detachPreviewService() {
        if (m_preview) m_preview->clearRuleSet();
        m_preview = nullptr;
    }

    [[nodiscard]] bool hasPreviewService() const { return m_preview != nullptr; }

    // ── Rule editing (panel UI → document → preview) ──────────────────────

    /// Edit a rule value.  Propagates to the live PCGPreviewService so the
    /// viewport regenerates immediately when autoRegenerate is enabled.
    /// Returns false if no document is bound or the key is not found.
    bool editRule(const std::string& key, const std::string& value) {
        if (!m_document) return false;
        if (!m_document->setValue(key, value)) return false;
        m_dirty = true;
        ++m_editCount;
        if (m_preview) m_preview->setRuleValue(key, value);
        if (m_onChange) m_onChange(key, value);
        return true;
    }

    /// Reset a single rule to its default value.
    bool resetRule(const std::string& key) {
        if (!m_document) return false;
        const PCGRule* r = m_document->findRule(key);
        if (!r) return false;
        if (!m_document->resetRule(key)) return false;
        m_dirty = true;
        ++m_editCount;
        if (m_preview) m_preview->setRuleValue(key, r->defaultValue);
        if (m_onChange) m_onChange(key, r->defaultValue);
        return true;
    }

    /// Reset all rules in the document to their defaults and trigger preview regen.
    bool resetAll() {
        if (!m_document) return false;
        m_document->resetToDefaults();
        m_dirty = true;
        ++m_editCount;
        if (m_preview) m_preview->resetRules();
        if (m_onResetAll) m_onResetAll();
        return true;
    }

    // ── Save-back ─────────────────────────────────────────────────────────

    /// Commit all unsaved rule edits as the new baseline.
    /// In a real editor this writes back to the project data file.
    /// Here it updates the snapshot (represents the saved state) and clears dirty.
    ProcGenSaveResult save() {
        if (!m_document) return { false, "No document bound" };
        m_snapshot = *m_document; // update saved snapshot
        m_dirty    = false;
        ++m_saveCount;
        if (m_onSave) m_onSave(*m_document);
        return { true, "" };
    }

    // ── Revert ────────────────────────────────────────────────────────────

    /// Discard unsaved edits and restore the last saved snapshot.
    bool revert() {
        if (!m_document) return false;
        *m_document = m_snapshot;
        m_dirty     = false;
        m_editCount = 0;
        if (m_preview) m_preview->bindRuleSet(m_document);
        if (m_onRevert) m_onRevert();
        return true;
    }

    // ── Dirty tracking ────────────────────────────────────────────────────

    [[nodiscard]] bool     isDirty()   const { return m_dirty; }
    [[nodiscard]] uint32_t editCount() const { return m_editCount; }
    [[nodiscard]] uint32_t saveCount() const { return m_saveCount; }

    // ── Panel lifecycle ───────────────────────────────────────────────────

    void activate()   { m_active = true; }
    void deactivate() { m_active = false; }
    [[nodiscard]] bool isActive() const { return m_active; }

    // ── Rule inspection helpers ───────────────────────────────────────────

    /// Number of rules in the bound document.
    [[nodiscard]] uint32_t ruleCount() const {
        return m_document ? m_document->ruleCount() : 0;
    }

    /// Retrieve rule value by key from the live document.
    [[nodiscard]] std::string ruleValue(const std::string& key) const {
        if (!m_document) return {};
        return m_document->getValue(key);
    }

    /// True if the document has a rule with the given key.
    [[nodiscard]] bool hasRule(const std::string& key) const {
        return m_document && m_document->hasRule(key);
    }

    // ── Change callbacks ──────────────────────────────────────────────────

    using RuleChangeCallback = std::function<void(const std::string& key,
                                                  const std::string& value)>;
    using SimpleCallback     = std::function<void()>;
    using SaveCallback       = std::function<void(const PCGRuleSet&)>;

    void setOnChange(RuleChangeCallback cb)   { m_onChange   = std::move(cb); }
    void setOnResetAll(SimpleCallback cb)     { m_onResetAll = std::move(cb); }
    void setOnSave(SaveCallback cb)           { m_onSave     = std::move(cb); }
    void setOnRevert(SimpleCallback cb)       { m_onRevert   = std::move(cb); }

private:
    PCGRuleSet*       m_document = nullptr;
    PCGRuleSet        m_snapshot;           ///< last-saved state (for revert)
    PCGPreviewService* m_preview  = nullptr;

    bool     m_dirty    = false;
    bool     m_active   = false;
    uint32_t m_editCount = 0;
    uint32_t m_saveCount = 0;

    RuleChangeCallback m_onChange;
    SimpleCallback     m_onResetAll;
    SaveCallback       m_onSave;
    SimpleCallback     m_onRevert;
};

} // namespace NovaForge
