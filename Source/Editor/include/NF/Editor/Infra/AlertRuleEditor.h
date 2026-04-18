#pragma once
// NF::Editor — alert rule configuration editor
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

enum class AlertCondition : uint8_t { ThresholdExceeded, PatternMatch, RateChange, Absence, Compound };
inline const char* alertConditionName(AlertCondition v) {
    switch (v) {
        case AlertCondition::ThresholdExceeded: return "ThresholdExceeded";
        case AlertCondition::PatternMatch:      return "PatternMatch";
        case AlertCondition::RateChange:        return "RateChange";
        case AlertCondition::Absence:           return "Absence";
        case AlertCondition::Compound:          return "Compound";
    }
    return "Unknown";
}

enum class AlertSeverity : uint8_t { Info, Warning, Error, Critical };
inline const char* alertSeverityName(AlertSeverity v) {
    switch (v) {
        case AlertSeverity::Info:     return "Info";
        case AlertSeverity::Warning:  return "Warning";
        case AlertSeverity::Error:    return "Error";
        case AlertSeverity::Critical: return "Critical";
    }
    return "Unknown";
}

class AlertRule {
public:
    explicit AlertRule(uint32_t id, const std::string& name, AlertCondition condition, AlertSeverity severity)
        : m_id(id), m_name(name), m_condition(condition), m_severity(severity) {}

    void setIsActive(bool v)       { m_isActive    = v; }
    void setCooldownSec(float v)   { m_cooldownSec = v; }
    void setIsEnabled(bool v)      { m_isEnabled   = v; }

    [[nodiscard]] uint32_t           id()          const { return m_id;          }
    [[nodiscard]] const std::string& name()        const { return m_name;        }
    [[nodiscard]] AlertCondition     condition()   const { return m_condition;   }
    [[nodiscard]] AlertSeverity      severity()    const { return m_severity;    }
    [[nodiscard]] bool               isActive()    const { return m_isActive;    }
    [[nodiscard]] float              cooldownSec() const { return m_cooldownSec; }
    [[nodiscard]] bool               isEnabled()   const { return m_isEnabled;   }

private:
    uint32_t       m_id;
    std::string    m_name;
    AlertCondition m_condition;
    AlertSeverity  m_severity;
    bool  m_isActive    = true;
    float m_cooldownSec = 60.0f;
    bool  m_isEnabled   = true;
};

class AlertRuleEditor {
public:
    void setIsShowInactive(bool v)       { m_isShowInactive    = v; }
    void setIsGroupBySeverity(bool v)    { m_isGroupBySeverity = v; }
    void setGlobalCooldownSec(float v)   { m_globalCooldownSec = v; }

    bool addRule(const AlertRule& r) {
        for (auto& x : m_rules) if (x.id() == r.id()) return false;
        m_rules.push_back(r); return true;
    }
    bool removeRule(uint32_t id) {
        auto it = std::find_if(m_rules.begin(), m_rules.end(),
            [&](const AlertRule& r){ return r.id() == id; });
        if (it == m_rules.end()) return false;
        m_rules.erase(it); return true;
    }
    [[nodiscard]] AlertRule* findRule(uint32_t id) {
        for (auto& r : m_rules) if (r.id() == id) return &r;
        return nullptr;
    }

    [[nodiscard]] bool   isShowInactive()    const { return m_isShowInactive;    }
    [[nodiscard]] bool   isGroupBySeverity() const { return m_isGroupBySeverity; }
    [[nodiscard]] float  globalCooldownSec() const { return m_globalCooldownSec; }
    [[nodiscard]] size_t ruleCount()         const { return m_rules.size();      }

    [[nodiscard]] size_t countByCondition(AlertCondition c) const {
        size_t n = 0; for (auto& r : m_rules) if (r.condition() == c) ++n; return n;
    }
    [[nodiscard]] size_t countBySeverity(AlertSeverity s) const {
        size_t n = 0; for (auto& r : m_rules) if (r.severity() == s) ++n; return n;
    }
    [[nodiscard]] size_t countActive() const {
        size_t n = 0; for (auto& r : m_rules) if (r.isActive()) ++n; return n;
    }

private:
    std::vector<AlertRule> m_rules;
    bool  m_isShowInactive    = true;
    bool  m_isGroupBySeverity = false;
    float m_globalCooldownSec = 30.0f;
};

} // namespace NF
