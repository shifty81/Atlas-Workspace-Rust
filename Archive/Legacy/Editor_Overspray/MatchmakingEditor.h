#pragma once
// NF::Editor — online matchmaking configuration editor
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

enum class MatchmakingStrategy : uint8_t { Random, SkillBased, RegionBased, LatencyBased, Custom };
inline const char* matchmakingStrategyName(MatchmakingStrategy v) {
    switch (v) {
        case MatchmakingStrategy::Random:       return "Random";
        case MatchmakingStrategy::SkillBased:   return "SkillBased";
        case MatchmakingStrategy::RegionBased:  return "RegionBased";
        case MatchmakingStrategy::LatencyBased: return "LatencyBased";
        case MatchmakingStrategy::Custom:       return "Custom";
    }
    return "Unknown";
}

enum class MatchmakingStatus : uint8_t { Idle, Searching, Found, Connecting, Failed, Cancelled };
inline const char* matchmakingStatusName(MatchmakingStatus v) {
    switch (v) {
        case MatchmakingStatus::Idle:       return "Idle";
        case MatchmakingStatus::Searching:  return "Searching";
        case MatchmakingStatus::Found:      return "Found";
        case MatchmakingStatus::Connecting: return "Connecting";
        case MatchmakingStatus::Failed:     return "Failed";
        case MatchmakingStatus::Cancelled:  return "Cancelled";
    }
    return "Unknown";
}

class MatchmakingRule {
public:
    explicit MatchmakingRule(uint32_t id, const std::string& name, MatchmakingStrategy strategy)
        : m_id(id), m_name(name), m_strategy(strategy) {}

    void setStatus(MatchmakingStatus v)  { m_status     = v; }
    void setMaxPlayers(uint32_t v)       { m_maxPlayers  = v; }
    void setTimeoutSec(float v)          { m_timeoutSec  = v; }
    void setIsEnabled(bool v)            { m_isEnabled   = v; }

    [[nodiscard]] uint32_t              id()         const { return m_id;         }
    [[nodiscard]] const std::string&    name()       const { return m_name;       }
    [[nodiscard]] MatchmakingStrategy   strategy()   const { return m_strategy;   }
    [[nodiscard]] MatchmakingStatus     status()     const { return m_status;     }
    [[nodiscard]] uint32_t              maxPlayers() const { return m_maxPlayers;  }
    [[nodiscard]] float                 timeoutSec() const { return m_timeoutSec;  }
    [[nodiscard]] bool                  isEnabled()  const { return m_isEnabled;  }

private:
    uint32_t            m_id;
    std::string         m_name;
    MatchmakingStrategy m_strategy;
    MatchmakingStatus   m_status     = MatchmakingStatus::Idle;
    uint32_t            m_maxPlayers  = 4u;
    float               m_timeoutSec  = 60.0f;
    bool                m_isEnabled  = true;
};

class MatchmakingEditor {
public:
    void setIsShowDisabled(bool v)       { m_isShowDisabled     = v; }
    void setIsGroupByStrategy(bool v)    { m_isGroupByStrategy  = v; }
    void setGlobalTimeoutSec(float v)    { m_globalTimeoutSec   = v; }

    bool addRule(const MatchmakingRule& r) {
        for (auto& x : m_rules) if (x.id() == r.id()) return false;
        m_rules.push_back(r); return true;
    }
    bool removeRule(uint32_t id) {
        auto it = std::find_if(m_rules.begin(), m_rules.end(),
            [&](const MatchmakingRule& r){ return r.id() == id; });
        if (it == m_rules.end()) return false;
        m_rules.erase(it); return true;
    }
    [[nodiscard]] MatchmakingRule* findRule(uint32_t id) {
        for (auto& r : m_rules) if (r.id() == id) return &r;
        return nullptr;
    }

    [[nodiscard]] bool   isShowDisabled()    const { return m_isShowDisabled;    }
    [[nodiscard]] bool   isGroupByStrategy() const { return m_isGroupByStrategy; }
    [[nodiscard]] float  globalTimeoutSec()  const { return m_globalTimeoutSec;  }
    [[nodiscard]] size_t ruleCount()         const { return m_rules.size();      }

    [[nodiscard]] size_t countByStrategy(MatchmakingStrategy s) const {
        size_t n = 0; for (auto& r : m_rules) if (r.strategy() == s) ++n; return n;
    }
    [[nodiscard]] size_t countByStatus(MatchmakingStatus s) const {
        size_t n = 0; for (auto& r : m_rules) if (r.status() == s) ++n; return n;
    }
    [[nodiscard]] size_t countEnabled() const {
        size_t n = 0; for (auto& r : m_rules) if (r.isEnabled()) ++n; return n;
    }

private:
    std::vector<MatchmakingRule> m_rules;
    bool  m_isShowDisabled    = false;
    bool  m_isGroupByStrategy = false;
    float m_globalTimeoutSec  = 120.0f;
};

} // namespace NF
