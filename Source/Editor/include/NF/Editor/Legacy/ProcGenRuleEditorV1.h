#pragma once
// NF::Editor — Procedural generation rule editor v1: rule and constraint management
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Pgrv1RuleType  : uint8_t { Placement, Replacement, Decoration, Constraint, Filter, Post };
enum class Pgrv1RuleState : uint8_t { Disabled, Enabled, Testing, Locked };

inline const char* pgrv1RuleTypeName(Pgrv1RuleType t) {
    switch (t) {
        case Pgrv1RuleType::Placement:    return "Placement";
        case Pgrv1RuleType::Replacement:  return "Replacement";
        case Pgrv1RuleType::Decoration:   return "Decoration";
        case Pgrv1RuleType::Constraint:   return "Constraint";
        case Pgrv1RuleType::Filter:       return "Filter";
        case Pgrv1RuleType::Post:         return "Post";
    }
    return "Unknown";
}

inline const char* pgrv1RuleStateName(Pgrv1RuleState s) {
    switch (s) {
        case Pgrv1RuleState::Disabled: return "Disabled";
        case Pgrv1RuleState::Enabled:  return "Enabled";
        case Pgrv1RuleState::Testing:  return "Testing";
        case Pgrv1RuleState::Locked:   return "Locked";
    }
    return "Unknown";
}

struct Pgrv1Rule {
    uint64_t        id    = 0;
    std::string     name;
    Pgrv1RuleType   type  = Pgrv1RuleType::Placement;
    Pgrv1RuleState  state = Pgrv1RuleState::Disabled;

    [[nodiscard]] bool isValid()   const { return id != 0 && !name.empty(); }
    [[nodiscard]] bool isEnabled() const { return state == Pgrv1RuleState::Enabled; }
    [[nodiscard]] bool isLocked()  const { return state == Pgrv1RuleState::Locked; }
};

struct Pgrv1Constraint {
    uint64_t    id     = 0;
    uint64_t    ruleId = 0;
    std::string description;

    [[nodiscard]] bool isValid() const { return id != 0 && ruleId != 0 && !description.empty(); }
};

using Pgrv1ChangeCallback = std::function<void(uint64_t)>;

class ProcGenRuleEditorV1 {
public:
    static constexpr size_t MAX_RULES       = 1024;
    static constexpr size_t MAX_CONSTRAINTS = 4096;

    bool addRule(const Pgrv1Rule& rule) {
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

    [[nodiscard]] Pgrv1Rule* findRule(uint64_t id) {
        for (auto& r : m_rules) if (r.id == id) return &r;
        return nullptr;
    }

    bool addConstraint(const Pgrv1Constraint& constraint) {
        if (!constraint.isValid()) return false;
        for (const auto& c : m_constraints) if (c.id == constraint.id) return false;
        if (m_constraints.size() >= MAX_CONSTRAINTS) return false;
        m_constraints.push_back(constraint);
        if (m_onChange) m_onChange(constraint.ruleId);
        return true;
    }

    bool removeConstraint(uint64_t id) {
        for (auto it = m_constraints.begin(); it != m_constraints.end(); ++it) {
            if (it->id == id) { m_constraints.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] size_t ruleCount()       const { return m_rules.size(); }
    [[nodiscard]] size_t constraintCount() const { return m_constraints.size(); }

    [[nodiscard]] size_t enabledCount() const {
        size_t c = 0; for (const auto& r : m_rules) if (r.isEnabled()) ++c; return c;
    }
    [[nodiscard]] size_t lockedCount() const {
        size_t c = 0; for (const auto& r : m_rules) if (r.isLocked()) ++c; return c;
    }
    [[nodiscard]] size_t countByRuleType(Pgrv1RuleType type) const {
        size_t c = 0; for (const auto& r : m_rules) if (r.type == type) ++c; return c;
    }

    void setOnChange(Pgrv1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Pgrv1Rule>       m_rules;
    std::vector<Pgrv1Constraint> m_constraints;
    Pgrv1ChangeCallback          m_onChange;
};

} // namespace NF
