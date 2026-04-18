#pragma once
// NF::Editor — Settings control panel v1: sectioned settings with search, reset, dirty tracking
#include "NF/Core/Core.h"
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

// ── CpSettingType ─────────────────────────────────────────────────
// Prefixed Cp (ControlPanel) to avoid collision with SettingEntry in SettingsPanel.h

enum class CpSettingType : uint8_t {
    Bool,
    Int,
    Float,
    String,
    Enum,
    Color,   // hex string "#RRGGBBAA"
    Path,    // file/folder path string
};

inline const char* cpSettingTypeName(CpSettingType t) {
    switch (t) {
        case CpSettingType::Bool:   return "Bool";
        case CpSettingType::Int:    return "Int";
        case CpSettingType::Float:  return "Float";
        case CpSettingType::String: return "String";
        case CpSettingType::Enum:   return "Enum";
        case CpSettingType::Color:  return "Color";
        case CpSettingType::Path:   return "Path";
    }
    return "Unknown";
}

// ── CpSettingEntry ────────────────────────────────────────────────

struct CpSettingEntry {
    uint32_t         id              = 0;
    std::string      key;
    std::string      label;
    std::string      description;
    CpSettingType    valueType       = CpSettingType::String;
    std::string      value;
    std::string      defaultValue;
    bool             requiresRestart = false;
    bool             readOnly        = false;
    bool             dirty           = false;

    // Enum support
    std::vector<std::string> enumOptions;

    [[nodiscard]] bool isValid()     const { return id != 0 && !key.empty(); }
    [[nodiscard]] bool isAtDefault() const { return value == defaultValue;   }

    void resetToDefault() {
        if (!readOnly) { value = defaultValue; dirty = false; }
    }
};

// ── CpSection ─────────────────────────────────────────────────────

struct CpSection {
    uint32_t                  id       = 0;
    std::string               title;
    std::string               iconId;
    bool                      expanded = true;
    std::vector<CpSettingEntry> entries;

    [[nodiscard]] bool isValid() const { return id != 0 && !title.empty(); }

    CpSettingEntry* findEntry(const std::string& key) {
        for (auto& e : entries) if (e.key == key) return &e;
        return nullptr;
    }
    const CpSettingEntry* findEntry(const std::string& key) const {
        for (const auto& e : entries) if (e.key == key) return &e;
        return nullptr;
    }
    size_t dirtyCount() const {
        size_t n = 0; for (const auto& e : entries) if (e.dirty) ++n; return n;
    }
};

// ── SettingsControlPanelV1 ────────────────────────────────────────

using CpChangeCallback = std::function<void(const CpSettingEntry&, const std::string& oldVal)>;

class SettingsControlPanelV1 {
public:
    static constexpr size_t MAX_SECTIONS = 32;
    static constexpr size_t MAX_ENTRIES  = 512;

    bool addSection(const CpSection& section) {
        if (!section.isValid()) return false;
        if (m_sections.size() >= MAX_SECTIONS) return false;
        for (const auto& s : m_sections) if (s.id == section.id) return false;
        m_sections.push_back(section);
        return true;
    }

    bool removeSection(uint32_t id) {
        for (auto it = m_sections.begin(); it != m_sections.end(); ++it) {
            if (it->id == id) { m_sections.erase(it); return true; }
        }
        return false;
    }

    bool addEntry(uint32_t sectionId, const CpSettingEntry& entry) {
        auto* sec = findSection(sectionId);
        if (!sec || !entry.isValid()) return false;
        if (totalEntryCount() >= MAX_ENTRIES) return false;
        if (sec->findEntry(entry.key)) return false;
        sec->entries.push_back(entry);
        return true;
    }

    bool setValue(const std::string& key, const std::string& newValue) {
        for (auto& sec : m_sections) {
            auto* e = sec.findEntry(key);
            if (e) {
                if (e->readOnly) return false;
                std::string old = e->value;
                e->value = newValue;
                e->dirty = (newValue != e->defaultValue);
                ++m_changeCount;
                if (m_onChange) m_onChange(*e, old);
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] std::string getValue(const std::string& key,
                                       const std::string& def = "") const {
        for (const auto& sec : m_sections) {
            const auto* e = sec.findEntry(key);
            if (e) return e->value;
        }
        return def;
    }

    bool resetToDefault(const std::string& key) {
        for (auto& sec : m_sections) {
            auto* e = sec.findEntry(key);
            if (e) {
                if (e->readOnly) return false;
                std::string old = e->value;
                e->resetToDefault();
                ++m_changeCount;
                if (m_onChange) m_onChange(*e, old);
                return true;
            }
        }
        return false;
    }

    void resetAllToDefault() {
        for (auto& sec : m_sections)
            for (auto& e : sec.entries)
                if (!e.readOnly) e.resetToDefault();
    }

    [[nodiscard]] std::vector<std::string> search(const std::string& query) const {
        std::vector<std::string> keys;
        for (const auto& sec : m_sections) {
            for (const auto& e : sec.entries) {
                if (e.key.find(query)         != std::string::npos ||
                    e.label.find(query)       != std::string::npos ||
                    e.description.find(query) != std::string::npos) {
                    keys.push_back(e.key);
                }
            }
        }
        return keys;
    }

    bool setSectionExpanded(uint32_t id, bool expanded) {
        auto* s = findSection(id);
        if (!s) return false;
        s->expanded = expanded;
        return true;
    }

    CpSection* findSection(uint32_t id) {
        for (auto& s : m_sections) if (s.id == id) return &s;
        return nullptr;
    }
    const CpSection* findSection(uint32_t id) const {
        for (const auto& s : m_sections) if (s.id == id) return &s;
        return nullptr;
    }

    void setOnChange(CpChangeCallback cb) { m_onChange = std::move(cb); }

    [[nodiscard]] size_t sectionCount()    const { return m_sections.size(); }
    [[nodiscard]] size_t changeCount()     const { return m_changeCount;     }
    [[nodiscard]] const std::vector<CpSection>& sections() const { return m_sections; }

    [[nodiscard]] size_t totalEntryCount() const {
        size_t n = 0;
        for (const auto& s : m_sections) n += s.entries.size();
        return n;
    }
    [[nodiscard]] size_t totalDirtyCount() const {
        size_t n = 0;
        for (const auto& s : m_sections) n += s.dirtyCount();
        return n;
    }
    [[nodiscard]] bool hasRestartRequired() const {
        for (const auto& s : m_sections)
            for (const auto& e : s.entries)
                if (e.dirty && e.requiresRestart) return true;
        return false;
    }

private:
    std::vector<CpSection> m_sections;
    CpChangeCallback       m_onChange;
    size_t                 m_changeCount = 0;
};

} // namespace NF
