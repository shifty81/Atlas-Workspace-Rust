#pragma once
// NF::Editor — Property grid v1: typed property rows, groups, and change tracking
#include "NF/Core/Core.h"
#include <cstdint>
#include <functional>
#include <string>
#include <variant>
#include <vector>

namespace NF {

// ── Property Value Type ───────────────────────────────────────────

enum class PropValueType : uint8_t {
    Bool,
    Int,
    Float,
    String,
    Color,      // RGBA packed as uint32_t
    Vec2,       // x, y floats
    Vec3,       // x, y, z floats
    Enum,       // int index + string label list
    Asset,      // asset path string
    ReadOnly,   // display-only, not editable
};

inline const char* propValueTypeName(PropValueType t) {
    switch (t) {
        case PropValueType::Bool:     return "Bool";
        case PropValueType::Int:      return "Int";
        case PropValueType::Float:    return "Float";
        case PropValueType::String:   return "String";
        case PropValueType::Color:    return "Color";
        case PropValueType::Vec2:     return "Vec2";
        case PropValueType::Vec3:     return "Vec3";
        case PropValueType::Enum:     return "Enum";
        case PropValueType::Asset:    return "Asset";
        case PropValueType::ReadOnly: return "ReadOnly";
    }
    return "Unknown";
}

// ── Property Value ────────────────────────────────────────────────

struct PropVec2 { float x = 0.f, y = 0.f; };
struct PropVec3 { float x = 0.f, y = 0.f, z = 0.f; };

using PropVariant = std::variant<
    bool,
    int,
    float,
    std::string,
    uint32_t,   // Color as RGBA packed
    PropVec2,
    PropVec3
>;

// ── Property Row ──────────────────────────────────────────────────

struct PropertyRow {
    uint32_t      id        = 0;
    std::string   key;
    std::string   label;
    std::string   tooltip;
    PropValueType valueType = PropValueType::String;
    PropVariant   value;
    bool          readOnly  = false;
    bool          dirty     = false;  // modified but not applied
    bool          visible   = true;

    // For Enum type: list of option labels
    std::vector<std::string> enumOptions;

    [[nodiscard]] bool isValid() const { return id != 0 && !key.empty(); }

    [[nodiscard]] std::string displayValue() const {
        return std::visit([&](const auto& v) -> std::string {
            using T = std::decay_t<decltype(v)>;
            if constexpr (std::is_same_v<T, bool>)
                return v ? "true" : "false";
            else if constexpr (std::is_same_v<T, int>)
                return std::to_string(v);
            else if constexpr (std::is_same_v<T, float>)
                return std::to_string(v);
            else if constexpr (std::is_same_v<T, std::string>)
                return v;
            else if constexpr (std::is_same_v<T, uint32_t>)
                return "#" + std::to_string(v);
            else if constexpr (std::is_same_v<T, PropVec2>)
                return "(" + std::to_string(v.x) + ", " + std::to_string(v.y) + ")";
            else if constexpr (std::is_same_v<T, PropVec3>)
                return "(" + std::to_string(v.x) + ", " + std::to_string(v.y)
                     + ", " + std::to_string(v.z) + ")";
            return "";
        }, value);
    }
};

// ── Property Group ────────────────────────────────────────────────

struct PropertyGroup {
    uint32_t             id          = 0;
    std::string          label;
    bool                 expanded    = true;
    bool                 visible     = true;
    std::vector<PropertyRow> rows;

    [[nodiscard]] bool isValid() const { return id != 0 && !label.empty(); }
    [[nodiscard]] size_t rowCount() const { return rows.size(); }

    PropertyRow* findRow(const std::string& key) {
        for (auto& r : rows) if (r.key == key) return &r;
        return nullptr;
    }
    const PropertyRow* findRow(const std::string& key) const {
        for (const auto& r : rows) if (r.key == key) return &r;
        return nullptr;
    }
};

// ── Property Grid V1 ──────────────────────────────────────────────
// Manages groups and rows; tracks changes and notifies listeners.

using PropChangeCallback = std::function<void(const PropertyRow&, const PropVariant& oldVal)>;

class PropertyGridV1 {
public:
    static constexpr size_t MAX_GROUPS = 32;
    static constexpr size_t MAX_ROWS   = 256;

    bool addGroup(const PropertyGroup& g) {
        if (!g.isValid()) return false;
        if (m_groups.size() >= MAX_GROUPS) return false;
        for (const auto& eg : m_groups) if (eg.id == g.id) return false;
        m_groups.push_back(g);
        return true;
    }

    bool removeGroup(uint32_t groupId) {
        for (auto it = m_groups.begin(); it != m_groups.end(); ++it) {
            if (it->id == groupId) { m_groups.erase(it); return true; }
        }
        return false;
    }

    PropertyGroup* findGroup(uint32_t id) {
        for (auto& g : m_groups) if (g.id == id) return &g;
        return nullptr;
    }
    const PropertyGroup* findGroup(uint32_t id) const {
        for (const auto& g : m_groups) if (g.id == id) return &g;
        return nullptr;
    }

    bool addRow(uint32_t groupId, const PropertyRow& row) {
        auto* g = findGroup(groupId);
        if (!g || !row.isValid()) return false;
        if (totalRowCount() >= MAX_ROWS) return false;
        for (const auto& r : g->rows) if (r.id == row.id) return false;
        g->rows.push_back(row);
        return true;
    }

    bool setValue(uint32_t rowId, const PropVariant& newVal) {
        for (auto& g : m_groups) {
            for (auto& r : g.rows) {
                if (r.id == rowId) {
                    if (r.readOnly) return false;
                    PropVariant old = r.value;
                    r.value = newVal;
                    r.dirty = true;
                    ++m_changeCount;
                    if (m_onChange) m_onChange(r, old);
                    return true;
                }
            }
        }
        return false;
    }

    const PropertyRow* findRow(uint32_t rowId) const {
        for (const auto& g : m_groups)
            for (const auto& r : g.rows)
                if (r.id == rowId) return &r;
        return nullptr;
    }

    const PropertyRow* findRowByKey(const std::string& key) const {
        for (const auto& g : m_groups)
            for (const auto& r : g.rows)
                if (r.key == key) return &r;
        return nullptr;
    }

    void setGroupExpanded(uint32_t groupId, bool expanded) {
        auto* g = findGroup(groupId);
        if (g) g->expanded = expanded;
    }

    void clearDirty() {
        for (auto& g : m_groups)
            for (auto& r : g.rows)
                r.dirty = false;
    }

    [[nodiscard]] size_t groupCount()       const { return m_groups.size();  }
    [[nodiscard]] size_t changeCount()      const { return m_changeCount;    }
    [[nodiscard]] const std::vector<PropertyGroup>& groups() const { return m_groups; }

    [[nodiscard]] size_t totalRowCount() const {
        size_t n = 0;
        for (const auto& g : m_groups) n += g.rows.size();
        return n;
    }

    [[nodiscard]] size_t dirtyRowCount() const {
        size_t n = 0;
        for (const auto& g : m_groups)
            for (const auto& r : g.rows)
                if (r.dirty) ++n;
        return n;
    }

    void setOnChange(PropChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<PropertyGroup> m_groups;
    PropChangeCallback m_onChange;
    size_t m_changeCount = 0;
};

} // namespace NF
