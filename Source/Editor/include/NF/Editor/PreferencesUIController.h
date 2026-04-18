#pragma once
// NF::Editor — Phase H.1: Preferences UI Controller
//
// Provides a category-navigable preferences window that surfaces all registered
// preferences for interactive editing.  Supports project vs. user scoping,
// live validation, import/export of preference sets, and full reset.
//
//   PreferenceScopeType   — User (per-machine) or Project (per-project override)
//   PreferenceUIEntry     — display-ready wrapper around a PreferenceEntry with
//                           current edited value, scope, and dirty flag
//   PreferencesUIState    — open/closed + active category
//   PreferencesUIController — owns the editable preference list:
//       open() / close() / isOpen()
//       setActiveCategory() / activeCategory()
//       loadFrom(PreferenceRegistry&)  — snapshot registry into editable entries
//       setValue(key, value)           — edit a preference value (marks dirty)
//       getValue(key)                  — current edited value
//       resetEntry(key)                — reset one entry to its default
//       resetAll()                     — reset all entries to defaults
//       isDirty() / dirtyCount()       — unsaved changes tracking
//       applyAll()                     — commit edits (clears dirty)
//       revertAll()                    — discard edits (restore from last load)
//       exportSettings()               — serialize current values to a string
//       importSettings(data)           — deserialize and apply values
//       entriesForCategory(cat)        — entries visible under a category
//       setScope(key, scope)           — assign user vs project scope to an entry
//       scopeOf(key)                   — query scope of an entry

#include "NF/Workspace/WorkspacePreferences.h"
#include <algorithm>
#include <functional>
#include <sstream>
#include <string>
#include <vector>

namespace NF {

// ═════════════════════════════════════════════════════════════════
// PreferenceScopeType
// ═════════════════════════════════════════════════════════════════

enum class PreferenceScopeType : uint8_t {
    User    = 0,   // machine-level; persists across projects
    Project = 1,   // project override; stored alongside project file
};

inline const char* preferenceScopeTypeName(PreferenceScopeType s) {
    switch (s) {
        case PreferenceScopeType::User:    return "User";
        case PreferenceScopeType::Project: return "Project";
        default:                           return "Unknown";
    }
}

// ═════════════════════════════════════════════════════════════════
// PreferenceUIEntry
// ═════════════════════════════════════════════════════════════════

struct PreferenceUIEntry {
    std::string         key;
    std::string         displayName;
    std::string         description;
    std::string         currentValue;   // edited (may differ from committed)
    std::string         committedValue; // last applied value
    std::string         defaultValue;
    PreferenceCategory  category    = PreferenceCategory::General;
    PreferenceType      type        = PreferenceType::String;
    PreferenceScopeType scope       = PreferenceScopeType::User;
    bool                readOnly    = false;
    float               minValue    = 0.f;
    float               maxValue    = 0.f;
    bool                hasRange    = false;

    [[nodiscard]] bool isDirty()   const { return currentValue != committedValue; }
    [[nodiscard]] bool isDefault() const { return currentValue == defaultValue; }

    void resetToDefault() { currentValue = defaultValue; }
    void revert()         { currentValue = committedValue; }
    void commit()         { committedValue = currentValue; }
};

// ═════════════════════════════════════════════════════════════════
// PreferencesUIController
// ═════════════════════════════════════════════════════════════════

class PreferencesUIController {
public:
    // ── Open / close ──────────────────────────────────────────────

    void open()  { m_isOpen = true; }
    void close() { m_isOpen = false; }

    [[nodiscard]] bool isOpen() const { return m_isOpen; }

    // ── Category navigation ───────────────────────────────────────

    void setActiveCategory(PreferenceCategory cat) { m_activeCategory = cat; }

    [[nodiscard]] PreferenceCategory activeCategory() const { return m_activeCategory; }

    // ── Load from registry ────────────────────────────────────────
    // Snapshot all entries from the registry into editable UIEntries.
    // Existing edits are cleared.

    void loadFrom(const PreferenceRegistry& registry,
                  const std::vector<std::string>& orderedKeys = {}) {
        m_entries.clear();
        if (!orderedKeys.empty()) {
            for (const auto& key : orderedKeys) {
                const PreferenceEntry* src = registry.find(key);
                if (src) m_entries.push_back(makeUIEntry(*src));
            }
        } else {
            for (const auto& src : registry.all()) {
                m_entries.push_back(makeUIEntry(src));
            }
        }
    }

    // ── Value editing ─────────────────────────────────────────────

    bool setValue(const std::string& key, const std::string& value) {
        auto* e = findEntry(key);
        if (!e || e->readOnly) return false;
        e->currentValue = value;
        return true;
    }

    [[nodiscard]] std::string getValue(const std::string& key) const {
        const auto* e = findEntryConst(key);
        return e ? e->currentValue : std::string{};
    }

    // ── Reset ─────────────────────────────────────────────────────

    bool resetEntry(const std::string& key) {
        auto* e = findEntry(key);
        if (!e) return false;
        e->resetToDefault();
        return true;
    }

    void resetAll() {
        for (auto& e : m_entries) e.resetToDefault();
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
        for (auto& e : m_entries) e.commit();
        if (m_onApply) m_onApply();
    }

    void revertAll() {
        for (auto& e : m_entries) e.revert();
    }

    // ── Callbacks ─────────────────────────────────────────────────

    void setOnApply(std::function<void()> cb) { m_onApply = std::move(cb); }

    // ── Export / import ───────────────────────────────────────────

    [[nodiscard]] std::string exportSettings() const {
        std::ostringstream out;
        for (const auto& e : m_entries) {
            out << e.key << "=" << e.currentValue << "\n";
        }
        return out.str();
    }

    bool importSettings(const std::string& data) {
        if (data.empty()) return false;
        std::istringstream in(data);
        std::string line;
        bool any = false;
        while (std::getline(in, line)) {
            auto sep = line.find('=');
            if (sep == std::string::npos) continue;
            std::string key   = line.substr(0, sep);
            std::string value = line.substr(sep + 1);
            if (setValue(key, value)) any = true;
        }
        return any;
    }

    // ── Category filtering ────────────────────────────────────────

    [[nodiscard]] std::vector<const PreferenceUIEntry*>
    entriesForCategory(PreferenceCategory cat) const {
        std::vector<const PreferenceUIEntry*> results;
        for (const auto& e : m_entries)
            if (e.category == cat) results.push_back(&e);
        return results;
    }

    [[nodiscard]] std::vector<const PreferenceUIEntry*> activeEntries() const {
        return entriesForCategory(m_activeCategory);
    }

    // ── Scope management ─────────────────────────────────────────

    bool setScope(const std::string& key, PreferenceScopeType scope) {
        auto* e = findEntry(key);
        if (!e) return false;
        e->scope = scope;
        return true;
    }

    [[nodiscard]] PreferenceScopeType scopeOf(const std::string& key) const {
        const auto* e = findEntryConst(key);
        return e ? e->scope : PreferenceScopeType::User;
    }

    // ── All entries accessor ───────────────────────────────────────

    [[nodiscard]] const std::vector<PreferenceUIEntry>& entries() const { return m_entries; }
    [[nodiscard]] size_t entryCount() const { return m_entries.size(); }

private:
    bool                         m_isOpen        = false;
    PreferenceCategory           m_activeCategory = PreferenceCategory::General;
    std::vector<PreferenceUIEntry> m_entries;
    std::function<void()>        m_onApply;

    static PreferenceUIEntry makeUIEntry(const PreferenceEntry& src) {
        PreferenceUIEntry e;
        e.key            = src.key;
        e.displayName    = src.displayName;
        e.description    = src.description;
        e.currentValue   = src.defaultValue;
        e.committedValue = src.defaultValue;
        e.defaultValue   = src.defaultValue;
        e.category       = src.category;
        e.type           = src.type;
        e.readOnly       = src.readOnly;
        e.minValue       = src.minValue;
        e.maxValue       = src.maxValue;
        e.hasRange       = src.hasRange;
        return e;
    }

    PreferenceUIEntry* findEntry(const std::string& key) {
        for (auto& e : m_entries) if (e.key == key) return &e;
        return nullptr;
    }

    const PreferenceUIEntry* findEntryConst(const std::string& key) const {
        for (const auto& e : m_entries) if (e.key == key) return &e;
        return nullptr;
    }
};

} // namespace NF
