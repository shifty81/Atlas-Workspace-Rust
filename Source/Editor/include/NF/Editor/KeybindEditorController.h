#pragma once
// NF::Editor — Phase H.2: Keybind Editor Controller
//
// Provides an interactive keybinding editor panel: browse all shortcuts by
// context/category, capture new key combos, detect conflicts, and reset bindings.
//
//   KeybindConflict       — two binding ids that share the same key+modifier
//   KeybindCaptureState   — Idle / Capturing / Captured / Cancelled
//   KeybindEditorEntry    — display-ready wrapper around a ShortcutBinding with
//                           edited key + modifier, conflict flag, dirty flag
//   KeybindEditorController —
//       loadFrom(ShortcutContext&)     — snapshot context into editor entries
//       startCapture(bindingId)        — begin listening for a key combo
//       captureKey(key, modifier)      — deliver the captured key to the pending entry
//       cancelCapture()               — abort capture, restore previous binding
//       confirmCapture()              — confirm the pending capture
//       resetBinding(id)              — reset one binding to its default
//       resetAll()                    — reset all bindings to defaults
//       detectConflicts()             — returns list of KeybindConflict pairs
//       isDirty() / dirtyCount()
//       applyAll()                    — commit edits
//       revertAll()                   — discard edits

#include "NF/Editor/ShortcutManager.h"
#include <algorithm>
#include <functional>
#include <string>
#include <vector>

namespace NF {

// ═════════════════════════════════════════════════════════════════
// KeybindCaptureState
// ═════════════════════════════════════════════════════════════════

enum class KeybindCaptureState : uint8_t {
    Idle      = 0,
    Capturing = 1,
    Captured  = 2,
    Cancelled = 3,
};

inline const char* keybindCaptureStateName(KeybindCaptureState s) {
    switch (s) {
        case KeybindCaptureState::Idle:      return "Idle";
        case KeybindCaptureState::Capturing: return "Capturing";
        case KeybindCaptureState::Captured:  return "Captured";
        case KeybindCaptureState::Cancelled: return "Cancelled";
        default:                             return "Unknown";
    }
}

// ═════════════════════════════════════════════════════════════════
// KeybindConflict
// ═════════════════════════════════════════════════════════════════

struct KeybindConflict {
    std::string bindingIdA;
    std::string bindingIdB;
    std::string key;
    uint8_t     modifiers = 0;

    [[nodiscard]] bool isValid() const {
        return !bindingIdA.empty() && !bindingIdB.empty() && !key.empty();
    }
};

// ═════════════════════════════════════════════════════════════════
// KeybindEditorEntry
// ═════════════════════════════════════════════════════════════════

struct KeybindEditorEntry {
    std::string      id;
    std::string      name;
    ShortcutCategory category     = ShortcutCategory::File;
    std::string      currentKey;
    uint8_t          currentMods  = 0;
    std::string      defaultKey;
    uint8_t          defaultMods  = 0;
    bool             hasConflict  = false;
    bool             enabled      = true;

    [[nodiscard]] bool isDirty()   const {
        return currentKey != defaultKey || currentMods != defaultMods;
    }
    [[nodiscard]] bool isDefault() const {
        return currentKey == defaultKey && currentMods == defaultMods;
    }

    void resetToDefault() {
        currentKey  = defaultKey;
        currentMods = defaultMods;
    }

    [[nodiscard]] std::string displayString() const {
        if (currentKey.empty()) return "(unbound)";
        if (currentMods == 0)   return currentKey;
        return "Mod+" + currentKey;
    }
};

// ═════════════════════════════════════════════════════════════════
// KeybindEditorController
// ═════════════════════════════════════════════════════════════════

class KeybindEditorController {
public:
    // ── Load from context ─────────────────────────────────────────

    void loadFrom(const ShortcutContext& ctx) {
        m_entries.clear();
        m_captureState  = KeybindCaptureState::Idle;
        m_captureTarget.clear();
        m_contextName   = ctx.name();

        for (size_t i = 0; i < ctx.bindingCount(); ++i) {
            // ShortcutContext exposes bindings via bindingAt() iterator-style
            // We iterate by index through the public const view
            const auto& bindings = ctx.bindings();
            if (i >= bindings.size()) break;
            const auto& b = bindings[i];
            KeybindEditorEntry e;
            e.id          = b.id;
            e.name        = b.name;
            e.category    = b.category;
            e.currentKey  = b.key;
            e.currentMods = b.modifiers;
            e.defaultKey  = b.key;
            e.defaultMods = b.modifiers;
            e.enabled     = b.enabled;
            m_entries.push_back(e);
        }
    }

    [[nodiscard]] const std::string& contextName() const { return m_contextName; }

    // ── Capture flow ──────────────────────────────────────────────

    bool startCapture(const std::string& bindingId) {
        if (m_captureState == KeybindCaptureState::Capturing) return false;
        auto* e = findEntry(bindingId);
        if (!e) return false;
        m_captureTarget      = bindingId;
        m_pendingKey.clear();
        m_pendingMods        = 0;
        m_captureState       = KeybindCaptureState::Capturing;
        return true;
    }

    bool captureKey(const std::string& key, uint8_t modifiers) {
        if (m_captureState != KeybindCaptureState::Capturing) return false;
        if (key.empty()) return false;
        m_pendingKey  = key;
        m_pendingMods = modifiers;
        m_captureState = KeybindCaptureState::Captured;
        return true;
    }

    bool confirmCapture() {
        if (m_captureState != KeybindCaptureState::Captured) return false;
        auto* e = findEntry(m_captureTarget);
        if (!e) { m_captureState = KeybindCaptureState::Idle; return false; }
        e->currentKey  = m_pendingKey;
        e->currentMods = m_pendingMods;
        m_captureState  = KeybindCaptureState::Idle;
        m_captureTarget.clear();
        return true;
    }

    void cancelCapture() {
        m_captureState = KeybindCaptureState::Cancelled;
        m_captureTarget.clear();
        m_pendingKey.clear();
        m_pendingMods  = 0;
        m_captureState = KeybindCaptureState::Idle;
    }

    [[nodiscard]] KeybindCaptureState captureState() const { return m_captureState; }
    [[nodiscard]] const std::string&  captureTarget() const { return m_captureTarget; }
    [[nodiscard]] const std::string&  pendingKey()    const { return m_pendingKey; }
    [[nodiscard]] uint8_t             pendingMods()   const { return m_pendingMods; }

    // ── Reset ─────────────────────────────────────────────────────

    bool resetBinding(const std::string& id) {
        auto* e = findEntry(id);
        if (!e) return false;
        e->resetToDefault();
        return true;
    }

    void resetAll() {
        for (auto& e : m_entries) e.resetToDefault();
    }

    // ── Conflict detection ────────────────────────────────────────

    [[nodiscard]] std::vector<KeybindConflict> detectConflicts() const {
        std::vector<KeybindConflict> conflicts;
        for (size_t i = 0; i < m_entries.size(); ++i) {
            const auto& a = m_entries[i];
            if (a.currentKey.empty()) continue;
            for (size_t j = i + 1; j < m_entries.size(); ++j) {
                const auto& b = m_entries[j];
                if (a.currentKey == b.currentKey && a.currentMods == b.currentMods) {
                    KeybindConflict c;
                    c.bindingIdA = a.id;
                    c.bindingIdB = b.id;
                    c.key        = a.currentKey;
                    c.modifiers  = a.currentMods;
                    conflicts.push_back(c);
                }
            }
        }
        // Tag entries with conflict flag
        auto* self = const_cast<KeybindEditorController*>(this);
        for (auto& e : self->m_entries) e.hasConflict = false;
        for (const auto& conflict : conflicts) {
            if (auto* ea = self->findEntry(conflict.bindingIdA)) ea->hasConflict = true;
            if (auto* eb = self->findEntry(conflict.bindingIdB)) eb->hasConflict = true;
        }
        return conflicts;
    }

    [[nodiscard]] bool hasConflicts() const {
        return !detectConflicts().empty();
    }

    // ── Dirty tracking ────────────────────────────────────────────

    [[nodiscard]] bool isDirty() const {
        for (const auto& e : m_entries) if (e.isDirty()) return true;
        return false;
    }

    [[nodiscard]] size_t dirtyCount() const {
        size_t c = 0;
        for (const auto& e : m_entries) if (e.isDirty()) ++c;
        return c;
    }

    // ── Apply / revert ────────────────────────────────────────────

    void applyAll() {
        for (auto& e : m_entries) {
            e.defaultKey  = e.currentKey;
            e.defaultMods = e.currentMods;
        }
        if (m_onApply) m_onApply();
    }

    void revertAll() {
        for (auto& e : m_entries) e.resetToDefault();
    }

    void setOnApply(std::function<void()> cb) { m_onApply = std::move(cb); }

    // ── Category filter ───────────────────────────────────────────

    [[nodiscard]] std::vector<const KeybindEditorEntry*>
    entriesForCategory(ShortcutCategory cat) const {
        std::vector<const KeybindEditorEntry*> results;
        for (const auto& e : m_entries)
            if (e.category == cat) results.push_back(&e);
        return results;
    }

    // ── All entries ────────────────────────────────────────────────

    [[nodiscard]] const std::vector<KeybindEditorEntry>& entries() const { return m_entries; }
    [[nodiscard]] size_t entryCount() const { return m_entries.size(); }

private:
    std::string                    m_contextName;
    std::vector<KeybindEditorEntry> m_entries;
    KeybindCaptureState            m_captureState  = KeybindCaptureState::Idle;
    std::string                    m_captureTarget;
    std::string                    m_pendingKey;
    uint8_t                        m_pendingMods   = 0;
    std::function<void()>          m_onApply;

    KeybindEditorEntry* findEntry(const std::string& id) {
        for (auto& e : m_entries) if (e.id == id) return &e;
        return nullptr;
    }
};

} // namespace NF
