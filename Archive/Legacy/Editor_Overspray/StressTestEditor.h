#pragma once
// NF::Editor — stress test editor
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

enum class StressTestScenario : uint8_t {
    MaxEntities, MaxParticles, MaxLights, MaxAudioSources,
    MaxNetworkPeers, MaxPhysicsObjects, MaxAnimatedCharacters, Custom
};

inline const char* stressTestScenarioName(StressTestScenario s) {
    switch (s) {
        case StressTestScenario::MaxEntities:          return "MaxEntities";
        case StressTestScenario::MaxParticles:         return "MaxParticles";
        case StressTestScenario::MaxLights:            return "MaxLights";
        case StressTestScenario::MaxAudioSources:      return "MaxAudioSources";
        case StressTestScenario::MaxNetworkPeers:      return "MaxNetworkPeers";
        case StressTestScenario::MaxPhysicsObjects:    return "MaxPhysicsObjects";
        case StressTestScenario::MaxAnimatedCharacters:return "MaxAnimatedCharacters";
        case StressTestScenario::Custom:               return "Custom";
    }
    return "Unknown";
}

enum class StressTestStatus : uint8_t {
    Idle, Warming, Running, Peaked, Crashed, Passed, Failed
};

inline const char* stressTestStatusName(StressTestStatus s) {
    switch (s) {
        case StressTestStatus::Idle:    return "Idle";
        case StressTestStatus::Warming: return "Warming";
        case StressTestStatus::Running: return "Running";
        case StressTestStatus::Peaked:  return "Peaked";
        case StressTestStatus::Crashed: return "Crashed";
        case StressTestStatus::Passed:  return "Passed";
        case StressTestStatus::Failed:  return "Failed";
    }
    return "Unknown";
}

enum class StressTestRampMode : uint8_t {
    Linear, Exponential, Step, Burst, Sustained
};

inline const char* stressTestRampModeName(StressTestRampMode m) {
    switch (m) {
        case StressTestRampMode::Linear:      return "Linear";
        case StressTestRampMode::Exponential: return "Exponential";
        case StressTestRampMode::Step:        return "Step";
        case StressTestRampMode::Burst:       return "Burst";
        case StressTestRampMode::Sustained:   return "Sustained";
    }
    return "Unknown";
}

class StressTestJob {
public:
    explicit StressTestJob(uint32_t id, const std::string& name, StressTestScenario scenario)
        : m_id(id), m_name(name), m_scenario(scenario) {}

    void setStatus(StressTestStatus v)    { m_status    = v; }
    void setRampMode(StressTestRampMode v){ m_rampMode  = v; }
    void setTargetLoad(uint32_t v)        { m_targetLoad = v; }
    void setIsEnabled(bool v)             { m_isEnabled = v; }

    [[nodiscard]] uint32_t            id()         const { return m_id;         }
    [[nodiscard]] const std::string&  name()       const { return m_name;       }
    [[nodiscard]] StressTestScenario  scenario()   const { return m_scenario;   }
    [[nodiscard]] StressTestStatus    status()     const { return m_status;     }
    [[nodiscard]] StressTestRampMode  rampMode()   const { return m_rampMode;   }
    [[nodiscard]] uint32_t            targetLoad() const { return m_targetLoad; }
    [[nodiscard]] bool                isEnabled()  const { return m_isEnabled;  }

private:
    uint32_t          m_id;
    std::string       m_name;
    StressTestScenario m_scenario;
    StressTestStatus  m_status     = StressTestStatus::Idle;
    StressTestRampMode m_rampMode  = StressTestRampMode::Linear;
    uint32_t          m_targetLoad = 1000u;
    bool              m_isEnabled  = true;
};

class StressTestEditor {
public:
    void setShowPassed(bool v)        { m_showPassed  = v; }
    void setAbortOnCrash(bool v)      { m_abortOnCrash = v; }
    void setTimeoutSec(float v)       { m_timeoutSec   = v; }

    bool addJob(const StressTestJob& j) {
        for (auto& x : m_jobs) if (x.id() == j.id()) return false;
        m_jobs.push_back(j); return true;
    }
    bool removeJob(uint32_t id) {
        auto it = std::find_if(m_jobs.begin(), m_jobs.end(),
            [&](const StressTestJob& j){ return j.id() == id; });
        if (it == m_jobs.end()) return false;
        m_jobs.erase(it); return true;
    }
    [[nodiscard]] StressTestJob* findJob(uint32_t id) {
        for (auto& j : m_jobs) if (j.id() == id) return &j;
        return nullptr;
    }

    [[nodiscard]] bool   isShowPassed()   const { return m_showPassed;   }
    [[nodiscard]] bool   isAbortOnCrash() const { return m_abortOnCrash; }
    [[nodiscard]] float  timeoutSec()     const { return m_timeoutSec;   }
    [[nodiscard]] size_t jobCount()       const { return m_jobs.size();  }

    [[nodiscard]] size_t countByScenario(StressTestScenario s) const {
        size_t n = 0; for (auto& j : m_jobs) if (j.scenario() == s) ++n; return n;
    }
    [[nodiscard]] size_t countByStatus(StressTestStatus s) const {
        size_t n = 0; for (auto& j : m_jobs) if (j.status() == s) ++n; return n;
    }
    [[nodiscard]] size_t countEnabled() const {
        size_t n = 0; for (auto& j : m_jobs) if (j.isEnabled()) ++n; return n;
    }

private:
    std::vector<StressTestJob> m_jobs;
    bool  m_showPassed   = true;
    bool  m_abortOnCrash = true;
    float m_timeoutSec   = 60.0f;
};

} // namespace NF
