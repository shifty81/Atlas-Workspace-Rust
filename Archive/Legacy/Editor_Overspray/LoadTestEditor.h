#pragma once
// NF::Editor — load test editor
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

enum class LoadTestProfile : uint8_t {
    Constant, Ramp, Spike, Soak, Breakpoint, Endurance
};

inline const char* loadTestProfileName(LoadTestProfile p) {
    switch (p) {
        case LoadTestProfile::Constant:   return "Constant";
        case LoadTestProfile::Ramp:       return "Ramp";
        case LoadTestProfile::Spike:      return "Spike";
        case LoadTestProfile::Soak:       return "Soak";
        case LoadTestProfile::Breakpoint: return "Breakpoint";
        case LoadTestProfile::Endurance:  return "Endurance";
    }
    return "Unknown";
}

enum class LoadTestTarget : uint8_t {
    Matchmaking, Login, AssetStreaming, SaveLoad, Leaderboard, Chat, Custom
};

inline const char* loadTestTargetName(LoadTestTarget t) {
    switch (t) {
        case LoadTestTarget::Matchmaking:    return "Matchmaking";
        case LoadTestTarget::Login:          return "Login";
        case LoadTestTarget::AssetStreaming: return "AssetStreaming";
        case LoadTestTarget::SaveLoad:       return "SaveLoad";
        case LoadTestTarget::Leaderboard:    return "Leaderboard";
        case LoadTestTarget::Chat:           return "Chat";
        case LoadTestTarget::Custom:         return "Custom";
    }
    return "Unknown";
}

enum class LoadTestResult : uint8_t {
    NotRun, Pass, Degraded, Fail, Timeout, Aborted
};

inline const char* loadTestResultName(LoadTestResult r) {
    switch (r) {
        case LoadTestResult::NotRun:   return "NotRun";
        case LoadTestResult::Pass:     return "Pass";
        case LoadTestResult::Degraded: return "Degraded";
        case LoadTestResult::Fail:     return "Fail";
        case LoadTestResult::Timeout:  return "Timeout";
        case LoadTestResult::Aborted:  return "Aborted";
    }
    return "Unknown";
}

class LoadTestScenario {
public:
    explicit LoadTestScenario(uint32_t id, const std::string& name,
                              LoadTestProfile profile, LoadTestTarget target)
        : m_id(id), m_name(name), m_profile(profile), m_target(target) {}

    void setResult(LoadTestResult v)   { m_result     = v; }
    void setConcurrentUsers(uint32_t v){ m_concurrentUsers = v; }
    void setDurationSec(float v)       { m_durationSec = v; }
    void setIsEnabled(bool v)          { m_isEnabled  = v; }

    [[nodiscard]] uint32_t           id()              const { return m_id;              }
    [[nodiscard]] const std::string& name()            const { return m_name;            }
    [[nodiscard]] LoadTestProfile    profile()         const { return m_profile;         }
    [[nodiscard]] LoadTestTarget     target()          const { return m_target;          }
    [[nodiscard]] LoadTestResult     result()          const { return m_result;          }
    [[nodiscard]] uint32_t           concurrentUsers() const { return m_concurrentUsers; }
    [[nodiscard]] float              durationSec()     const { return m_durationSec;     }
    [[nodiscard]] bool               isEnabled()       const { return m_isEnabled;       }

private:
    uint32_t        m_id;
    std::string     m_name;
    LoadTestProfile m_profile;
    LoadTestTarget  m_target;
    LoadTestResult  m_result          = LoadTestResult::NotRun;
    uint32_t        m_concurrentUsers = 100u;
    float           m_durationSec     = 60.0f;
    bool            m_isEnabled       = true;
};

class LoadTestEditor {
public:
    void setShowPassed(bool v)     { m_showPassed   = v; }
    void setStopOnFail(bool v)     { m_stopOnFail   = v; }
    void setGlobalTimeout(float v) { m_globalTimeout = v; }

    bool addScenario(const LoadTestScenario& s) {
        for (auto& x : m_scenarios) if (x.id() == s.id()) return false;
        m_scenarios.push_back(s); return true;
    }
    bool removeScenario(uint32_t id) {
        auto it = std::find_if(m_scenarios.begin(), m_scenarios.end(),
            [&](const LoadTestScenario& s){ return s.id() == id; });
        if (it == m_scenarios.end()) return false;
        m_scenarios.erase(it); return true;
    }
    [[nodiscard]] LoadTestScenario* findScenario(uint32_t id) {
        for (auto& s : m_scenarios) if (s.id() == id) return &s;
        return nullptr;
    }

    [[nodiscard]] bool   isShowPassed()    const { return m_showPassed;    }
    [[nodiscard]] bool   isStopOnFail()    const { return m_stopOnFail;    }
    [[nodiscard]] float  globalTimeout()   const { return m_globalTimeout; }
    [[nodiscard]] size_t scenarioCount()   const { return m_scenarios.size(); }

    [[nodiscard]] size_t countByProfile(LoadTestProfile p) const {
        size_t n = 0; for (auto& s : m_scenarios) if (s.profile() == p) ++n; return n;
    }
    [[nodiscard]] size_t countByResult(LoadTestResult r) const {
        size_t n = 0; for (auto& s : m_scenarios) if (s.result() == r) ++n; return n;
    }
    [[nodiscard]] size_t countEnabled() const {
        size_t n = 0; for (auto& s : m_scenarios) if (s.isEnabled()) ++n; return n;
    }

private:
    std::vector<LoadTestScenario> m_scenarios;
    bool  m_showPassed    = true;
    bool  m_stopOnFail    = false;
    float m_globalTimeout = 300.0f;
};

} // namespace NF
