#pragma once
// NF::Editor — Procedural mesh editor v1: mesh rule and variant management
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Pmev1RuleType  : uint8_t { Extrude, Lathe, Sweep, Subdivide, Weld, Bevel };
enum class Pmev1RuleState : uint8_t { Inactive, Active, Preview, Baked };

inline const char* pmev1RuleTypeName(Pmev1RuleType t) {
    switch (t) {
        case Pmev1RuleType::Extrude:   return "Extrude";
        case Pmev1RuleType::Lathe:     return "Lathe";
        case Pmev1RuleType::Sweep:     return "Sweep";
        case Pmev1RuleType::Subdivide: return "Subdivide";
        case Pmev1RuleType::Weld:      return "Weld";
        case Pmev1RuleType::Bevel:     return "Bevel";
    }
    return "Unknown";
}

inline const char* pmev1RuleStateName(Pmev1RuleState s) {
    switch (s) {
        case Pmev1RuleState::Inactive: return "Inactive";
        case Pmev1RuleState::Active:   return "Active";
        case Pmev1RuleState::Preview:  return "Preview";
        case Pmev1RuleState::Baked:    return "Baked";
    }
    return "Unknown";
}

struct Pmev1Rule {
    uint64_t        id        = 0;
    std::string     name;
    Pmev1RuleType   ruleType  = Pmev1RuleType::Extrude;
    Pmev1RuleState  state     = Pmev1RuleState::Inactive;
    float           strength  = 1.0f;
    uint32_t        iterations= 1;

    [[nodiscard]] bool isValid()   const { return id != 0 && !name.empty(); }
    [[nodiscard]] bool isActive()  const { return state == Pmev1RuleState::Active; }
    [[nodiscard]] bool isPreview() const { return state == Pmev1RuleState::Preview; }
    [[nodiscard]] bool isBaked()   const { return state == Pmev1RuleState::Baked; }
};

struct Pmev1Variant {
    uint64_t    id   = 0;
    std::string name;

    [[nodiscard]] bool isValid() const { return id != 0 && !name.empty(); }
};

using Pmev1ChangeCallback = std::function<void(uint64_t)>;

class ProceduralMeshEditorV1 {
public:
    static constexpr size_t MAX_RULES    = 256;
    static constexpr size_t MAX_VARIANTS = 128;

    bool addRule(const Pmev1Rule& rule) {
        if (!rule.isValid()) return false;
        for (const auto& r : m_rules) if (r.id == rule.id) return false;
        if (m_rules.size() >= MAX_RULES) return false;
        m_rules.push_back(rule);
        if (m_onChange) m_onChange(rule.id);
        return true;
    }

    bool removeRule(uint64_t id) {
        for (auto it = m_rules.begin(); it != m_rules.end(); ++it) {
            if (it->id == id) { m_rules.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] Pmev1Rule* findRule(uint64_t id) {
        for (auto& r : m_rules) if (r.id == id) return &r;
        return nullptr;
    }

    bool addVariant(const Pmev1Variant& variant) {
        if (!variant.isValid()) return false;
        for (const auto& v : m_variants) if (v.id == variant.id) return false;
        if (m_variants.size() >= MAX_VARIANTS) return false;
        m_variants.push_back(variant);
        return true;
    }

    bool removeVariant(uint64_t id) {
        for (auto it = m_variants.begin(); it != m_variants.end(); ++it) {
            if (it->id == id) { m_variants.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] size_t ruleCount()    const { return m_rules.size(); }
    [[nodiscard]] size_t variantCount() const { return m_variants.size(); }

    [[nodiscard]] size_t activeCount() const {
        size_t c = 0; for (const auto& r : m_rules) if (r.isActive()) ++c; return c;
    }
    [[nodiscard]] size_t bakedCount() const {
        size_t c = 0; for (const auto& r : m_rules) if (r.isBaked()) ++c; return c;
    }
    [[nodiscard]] size_t countByType(Pmev1RuleType type) const {
        size_t c = 0; for (const auto& r : m_rules) if (r.ruleType == type) ++c; return c;
    }

    void setOnChange(Pmev1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Pmev1Rule>    m_rules;
    std::vector<Pmev1Variant> m_variants;
    Pmev1ChangeCallback       m_onChange;
};

} // namespace NF
