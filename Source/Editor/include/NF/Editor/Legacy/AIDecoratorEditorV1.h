#pragma once
// NF::Editor — AI decorator editor v1: AI decorator rule and condition management
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Adcv1DecoratorType : uint8_t { Cooldown, Loop, Inverter, Retry, TimeLimit, Blackboard };
enum class Adcv1ConditionOp   : uint8_t { Equal, NotEqual, Less, Greater, LessEq, GreaterEq, IsSet, IsNotSet };

inline const char* adcv1DecoratorTypeName(Adcv1DecoratorType t) {
    switch (t) {
        case Adcv1DecoratorType::Cooldown:   return "Cooldown";
        case Adcv1DecoratorType::Loop:       return "Loop";
        case Adcv1DecoratorType::Inverter:   return "Inverter";
        case Adcv1DecoratorType::Retry:      return "Retry";
        case Adcv1DecoratorType::TimeLimit:  return "TimeLimit";
        case Adcv1DecoratorType::Blackboard: return "Blackboard";
    }
    return "Unknown";
}

inline const char* adcv1ConditionOpName(Adcv1ConditionOp op) {
    switch (op) {
        case Adcv1ConditionOp::Equal:      return "Equal";
        case Adcv1ConditionOp::NotEqual:   return "NotEqual";
        case Adcv1ConditionOp::Less:       return "Less";
        case Adcv1ConditionOp::Greater:    return "Greater";
        case Adcv1ConditionOp::LessEq:     return "LessEq";
        case Adcv1ConditionOp::GreaterEq:  return "GreaterEq";
        case Adcv1ConditionOp::IsSet:      return "IsSet";
        case Adcv1ConditionOp::IsNotSet:   return "IsNotSet";
    }
    return "Unknown";
}

struct Adcv1Decorator {
    uint64_t            id   = 0;
    std::string         name;
    Adcv1DecoratorType  type = Adcv1DecoratorType::Cooldown;

    [[nodiscard]] bool isValid() const { return id != 0 && !name.empty(); }
};

struct Adcv1Condition {
    uint64_t          id          = 0;
    uint64_t          decoratorId = 0;
    std::string       key;
    Adcv1ConditionOp  op          = Adcv1ConditionOp::Equal;

    [[nodiscard]] bool isValid() const { return id != 0 && decoratorId != 0 && !key.empty(); }
};

using Adcv1ChangeCallback = std::function<void(uint64_t)>;

class AIDecoratorEditorV1 {
public:
    static constexpr size_t MAX_DECORATORS = 1024;
    static constexpr size_t MAX_CONDITIONS = 4096;

    bool addDecorator(const Adcv1Decorator& decorator) {
        if (!decorator.isValid()) return false;
        for (const auto& d : m_decorators) if (d.id == decorator.id) return false;
        if (m_decorators.size() >= MAX_DECORATORS) return false;
        m_decorators.push_back(decorator);
        if (m_onChange) m_onChange(decorator.id);
        return true;
    }

    bool removeDecorator(uint64_t id) {
        for (auto it = m_decorators.begin(); it != m_decorators.end(); ++it) {
            if (it->id == id) { m_decorators.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] Adcv1Decorator* findDecorator(uint64_t id) {
        for (auto& d : m_decorators) if (d.id == id) return &d;
        return nullptr;
    }

    bool addCondition(const Adcv1Condition& condition) {
        if (!condition.isValid()) return false;
        for (const auto& c : m_conditions) if (c.id == condition.id) return false;
        if (m_conditions.size() >= MAX_CONDITIONS) return false;
        m_conditions.push_back(condition);
        if (m_onChange) m_onChange(condition.decoratorId);
        return true;
    }

    bool removeCondition(uint64_t id) {
        for (auto it = m_conditions.begin(); it != m_conditions.end(); ++it) {
            if (it->id == id) { m_conditions.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] size_t decoratorCount() const { return m_decorators.size(); }
    [[nodiscard]] size_t conditionCount() const { return m_conditions.size(); }

    [[nodiscard]] size_t countByDecoratorType(Adcv1DecoratorType type) const {
        size_t c = 0; for (const auto& d : m_decorators) if (d.type == type) ++c; return c;
    }
    [[nodiscard]] size_t countByConditionOp(Adcv1ConditionOp op) const {
        size_t c = 0; for (const auto& c2 : m_conditions) if (c2.op == op) ++c; return c;
    }

    void setOnChange(Adcv1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Adcv1Decorator> m_decorators;
    std::vector<Adcv1Condition> m_conditions;
    Adcv1ChangeCallback         m_onChange;
};

} // namespace NF
