#pragma once
// NF::Editor — query filter management editor
#include "NF/Core/Core.h"
#include "NF/Engine/Engine.h"
#include "NF/Renderer/Renderer.h"
#include "NF/UI/UI.h"
#include "NF/GraphVM/GraphVM.h"
#include "NF/Input/Input.h"
#include "NF/UI/UIWidgets.h"
#include <filesystem>
#include <fstream>
#include <set>
#include <map>
#include <deque>
#include <unordered_map>

namespace NF {

enum class FilterOperator : uint8_t { Equals, NotEquals, Contains, StartsWith, EndsWith, Regex, Range };
inline const char* filterOperatorName(FilterOperator v) {
    switch (v) {
        case FilterOperator::Equals:     return "Equals";
        case FilterOperator::NotEquals:  return "NotEquals";
        case FilterOperator::Contains:   return "Contains";
        case FilterOperator::StartsWith: return "StartsWith";
        case FilterOperator::EndsWith:   return "EndsWith";
        case FilterOperator::Regex:      return "Regex";
        case FilterOperator::Range:      return "Range";
    }
    return "Unknown";
}

enum class FilterLogic : uint8_t { And, Or, Not, Xor };
inline const char* filterLogicName(FilterLogic v) {
    switch (v) {
        case FilterLogic::And: return "And";
        case FilterLogic::Or:  return "Or";
        case FilterLogic::Not: return "Not";
        case FilterLogic::Xor: return "Xor";
    }
    return "Unknown";
}

class FilterRule {
public:
    explicit FilterRule(uint32_t id, const std::string& name, FilterOperator op)
        : m_id(id), m_name(name), m_op(op) {}

    void setLogic(FilterLogic v)    { m_logic      = v; }
    void setIsNegated(bool v)       { m_isNegated  = v; }
    void setIsEnabled(bool v)       { m_isEnabled  = v; }

    [[nodiscard]] uint32_t           id()        const { return m_id;        }
    [[nodiscard]] const std::string& name()      const { return m_name;      }
    [[nodiscard]] FilterOperator     op()        const { return m_op;        }
    [[nodiscard]] FilterLogic        logic()     const { return m_logic;     }
    [[nodiscard]] bool               isNegated() const { return m_isNegated; }
    [[nodiscard]] bool               isEnabled() const { return m_isEnabled; }

private:
    uint32_t       m_id;
    std::string    m_name;
    FilterOperator m_op;
    FilterLogic    m_logic     = FilterLogic::And;
    bool           m_isNegated = false;
    bool           m_isEnabled = true;
};

class FilterEditor {
public:
    void setIsShowDisabled(bool v)      { m_isShowDisabled     = v; }
    void setIsGroupByLogic(bool v)      { m_isGroupByLogic     = v; }
    void setMaxFiltersPerQuery(uint32_t v) { m_maxFiltersPerQuery = v; }

    bool addFilterRule(const FilterRule& r) {
        for (auto& x : m_rules) if (x.id() == r.id()) return false;
        m_rules.push_back(r); return true;
    }
    bool removeFilterRule(uint32_t id) {
        auto it = std::find_if(m_rules.begin(), m_rules.end(),
            [&](const FilterRule& r){ return r.id() == id; });
        if (it == m_rules.end()) return false;
        m_rules.erase(it); return true;
    }
    [[nodiscard]] FilterRule* findFilterRule(uint32_t id) {
        for (auto& r : m_rules) if (r.id() == id) return &r;
        return nullptr;
    }

    [[nodiscard]] bool     isShowDisabled()     const { return m_isShowDisabled;     }
    [[nodiscard]] bool     isGroupByLogic()     const { return m_isGroupByLogic;     }
    [[nodiscard]] uint32_t maxFiltersPerQuery() const { return m_maxFiltersPerQuery; }
    [[nodiscard]] size_t   filterRuleCount()    const { return m_rules.size();       }

    [[nodiscard]] size_t countByOperator(FilterOperator op) const {
        size_t n = 0; for (auto& r : m_rules) if (r.op() == op) ++n; return n;
    }
    [[nodiscard]] size_t countByLogic(FilterLogic l) const {
        size_t n = 0; for (auto& r : m_rules) if (r.logic() == l) ++n; return n;
    }
    [[nodiscard]] size_t countEnabled() const {
        size_t n = 0; for (auto& r : m_rules) if (r.isEnabled()) ++n; return n;
    }

private:
    std::vector<FilterRule> m_rules;
    bool     m_isShowDisabled     = false;
    bool     m_isGroupByLogic     = false;
    uint32_t m_maxFiltersPerQuery = 20u;
};

} // namespace NF
