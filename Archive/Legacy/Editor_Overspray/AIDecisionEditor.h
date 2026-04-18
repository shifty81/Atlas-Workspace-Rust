#pragma once
// NF::Editor — AI behavior decision tree/utility configuration
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
#include <string>
#include <vector>
#include <cstdint>

namespace NF {

enum class AiDecisionModel : uint8_t { BehaviorTree, UtilityAI, FSM, GOAP, Custom };
inline const char* aiDecisionModelName(AiDecisionModel v) {
    switch (v) {
        case AiDecisionModel::BehaviorTree: return "BehaviorTree";
        case AiDecisionModel::UtilityAI:    return "UtilityAI";
        case AiDecisionModel::FSM:          return "FSM";
        case AiDecisionModel::GOAP:         return "GOAP";
        case AiDecisionModel::Custom:       return "Custom";
    }
    return "Unknown";
}

enum class AiDecisionPriority : uint8_t { Low, Normal, High, Critical, Override };
inline const char* aiDecisionPriorityName(AiDecisionPriority v) {
    switch (v) {
        case AiDecisionPriority::Low:      return "Low";
        case AiDecisionPriority::Normal:   return "Normal";
        case AiDecisionPriority::High:     return "High";
        case AiDecisionPriority::Critical: return "Critical";
        case AiDecisionPriority::Override: return "Override";
    }
    return "Unknown";
}

class AiDecisionConfig {
public:
    explicit AiDecisionConfig(uint32_t id, const std::string& name,
                               AiDecisionModel model, AiDecisionPriority priority)
        : m_id(id), m_name(name), m_model(model), m_priority(priority) {}

    void setTickRateHz(uint32_t v)         { m_tickRateHz         = v; }
    void setMaxConcurrentGoals(uint32_t v) { m_maxConcurrentGoals = v; }
    void setIsEnabled(bool v)              { m_isEnabled           = v; }

    [[nodiscard]] uint32_t           id()                const { return m_id;                }
    [[nodiscard]] const std::string& name()              const { return m_name;              }
    [[nodiscard]] AiDecisionModel    model()             const { return m_model;             }
    [[nodiscard]] AiDecisionPriority priority()          const { return m_priority;          }
    [[nodiscard]] uint32_t           tickRateHz()        const { return m_tickRateHz;        }
    [[nodiscard]] uint32_t           maxConcurrentGoals() const { return m_maxConcurrentGoals; }
    [[nodiscard]] bool               isEnabled()         const { return m_isEnabled;         }

private:
    uint32_t           m_id;
    std::string        m_name;
    AiDecisionModel    m_model;
    AiDecisionPriority m_priority;
    uint32_t           m_tickRateHz         = 10u;
    uint32_t           m_maxConcurrentGoals = 3u;
    bool               m_isEnabled          = true;
};

class AIDecisionEditor {
public:
    void setIsShowDisabled(bool v)      { m_isShowDisabled    = v; }
    void setIsGroupByModel(bool v)      { m_isGroupByModel    = v; }
    void setDefaultTickRateHz(uint32_t v) { m_defaultTickRateHz = v; }

    bool addConfig(const AiDecisionConfig& c) {
        for (auto& x : m_configs) if (x.id() == c.id()) return false;
        m_configs.push_back(c); return true;
    }
    bool removeConfig(uint32_t id) {
        auto it = std::find_if(m_configs.begin(), m_configs.end(),
            [&](const AiDecisionConfig& c){ return c.id() == id; });
        if (it == m_configs.end()) return false;
        m_configs.erase(it); return true;
    }
    [[nodiscard]] AiDecisionConfig* findConfig(uint32_t id) {
        for (auto& c : m_configs) if (c.id() == id) return &c;
        return nullptr;
    }

    [[nodiscard]] bool     isShowDisabled()    const { return m_isShowDisabled;    }
    [[nodiscard]] bool     isGroupByModel()    const { return m_isGroupByModel;    }
    [[nodiscard]] uint32_t defaultTickRateHz() const { return m_defaultTickRateHz; }
    [[nodiscard]] size_t   configCount()       const { return m_configs.size();    }

    [[nodiscard]] size_t countByModel(AiDecisionModel m) const {
        size_t n = 0; for (auto& x : m_configs) if (x.model() == m) ++n; return n;
    }
    [[nodiscard]] size_t countByPriority(AiDecisionPriority p) const {
        size_t n = 0; for (auto& x : m_configs) if (x.priority() == p) ++n; return n;
    }
    [[nodiscard]] size_t countEnabled() const {
        size_t n = 0; for (auto& x : m_configs) if (x.isEnabled()) ++n; return n;
    }

private:
    std::vector<AiDecisionConfig> m_configs;
    bool     m_isShowDisabled    = false;
    bool     m_isGroupByModel    = true;
    uint32_t m_defaultTickRateHz = 20u;
};

} // namespace NF
