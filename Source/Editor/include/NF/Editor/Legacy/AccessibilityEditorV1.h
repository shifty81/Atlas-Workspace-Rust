#pragma once
// NF::Editor — Accessibility editor v1: accessibility rule management, contrast checking, screen reader support
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Accv1RuleCategory : uint8_t { Visual, Auditory, Motor, Cognitive, General };
enum class Accv1CheckState   : uint8_t { Pass, Fail, Warning, Skip, NotApplicable };

inline const char* accv1RuleCategoryName(Accv1RuleCategory c) {
    switch (c) {
        case Accv1RuleCategory::Visual:        return "Visual";
        case Accv1RuleCategory::Auditory:      return "Auditory";
        case Accv1RuleCategory::Motor:         return "Motor";
        case Accv1RuleCategory::Cognitive:     return "Cognitive";
        case Accv1RuleCategory::General:       return "General";
    }
    return "Unknown";
}

inline const char* accv1CheckStateName(Accv1CheckState s) {
    switch (s) {
        case Accv1CheckState::Pass:          return "Pass";
        case Accv1CheckState::Fail:          return "Fail";
        case Accv1CheckState::Warning:       return "Warning";
        case Accv1CheckState::Skip:          return "Skip";
        case Accv1CheckState::NotApplicable: return "NotApplicable";
    }
    return "Unknown";
}

struct Accv1CheckResult {
    uint64_t        id       = 0;
    uint64_t        ruleId   = 0;
    Accv1CheckState state    = Accv1CheckState::Skip;
    std::string     message;

    [[nodiscard]] bool isValid()         const { return id != 0 && ruleId != 0; }
    [[nodiscard]] bool isPassed()        const { return state == Accv1CheckState::Pass; }
    [[nodiscard]] bool isFailed()        const { return state == Accv1CheckState::Fail; }
    [[nodiscard]] bool isWarning()       const { return state == Accv1CheckState::Warning; }
};

struct Accv1Rule {
    uint64_t           id       = 0;
    std::string        name;
    Accv1RuleCategory  category = Accv1RuleCategory::General;
    bool               enabled  = true;

    [[nodiscard]] bool isValid()   const { return id != 0 && !name.empty(); }
    [[nodiscard]] bool isEnabled() const { return enabled; }
};

using Accv1CheckCallback = std::function<void(uint64_t)>;

class AccessibilityEditorV1 {
public:
    static constexpr size_t MAX_RULES  = 512;
    static constexpr size_t MAX_CHECKS = 4096;

    bool addRule(const Accv1Rule& rule) {
        if (!rule.isValid()) return false;
        for (const auto& r : m_rules) if (r.id == rule.id) return false;
        if (m_rules.size() >= MAX_RULES) return false;
        m_rules.push_back(rule);
        return true;
    }

    bool removeRule(uint64_t id) {
        for (auto it = m_rules.begin(); it != m_rules.end(); ++it) {
            if (it->id == id) { m_rules.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] Accv1Rule* findRule(uint64_t id) {
        for (auto& r : m_rules) if (r.id == id) return &r;
        return nullptr;
    }

    bool runCheck(const Accv1CheckResult& check) {
        if (!check.isValid()) return false;
        for (const auto& c : m_checks) if (c.id == check.id) return false;
        if (m_checks.size() >= MAX_CHECKS) return false;
        m_checks.push_back(check);
        if (m_onCheck) m_onCheck(check.id);
        return true;
    }

    [[nodiscard]] Accv1CheckResult* findCheck(uint64_t id) {
        for (auto& c : m_checks) if (c.id == id) return &c;
        return nullptr;
    }

    [[nodiscard]] size_t ruleCount()  const { return m_rules.size(); }
    [[nodiscard]] size_t checkCount() const { return m_checks.size(); }

    [[nodiscard]] size_t passCount() const {
        size_t c = 0; for (const auto& ch : m_checks) if (ch.isPassed()) ++c; return c;
    }
    [[nodiscard]] size_t failCount() const {
        size_t c = 0; for (const auto& ch : m_checks) if (ch.isFailed()) ++c; return c;
    }
    [[nodiscard]] size_t countByCategory(Accv1RuleCategory category) const {
        size_t c = 0; for (const auto& r : m_rules) if (r.category == category) ++c; return c;
    }

    void setOnCheck(Accv1CheckCallback cb) { m_onCheck = std::move(cb); }

private:
    std::vector<Accv1Rule>        m_rules;
    std::vector<Accv1CheckResult> m_checks;
    Accv1CheckCallback            m_onCheck;
};

} // namespace NF
