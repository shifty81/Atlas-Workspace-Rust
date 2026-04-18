#pragma once
// NF::Editor — profiling session management editor
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

enum class ProfSessionMode : uint8_t { Realtime, Capture, Replay, Compare, Export };
inline const char* profSessionModeName(ProfSessionMode v) {
    switch (v) {
        case ProfSessionMode::Realtime: return "Realtime";
        case ProfSessionMode::Capture:  return "Capture";
        case ProfSessionMode::Replay:   return "Replay";
        case ProfSessionMode::Compare:  return "Compare";
        case ProfSessionMode::Export:   return "Export";
    }
    return "Unknown";
}

enum class ProfSessionState : uint8_t { Idle, Running, Paused, Stopped, Analyzing };
inline const char* profSessionStateName(ProfSessionState v) {
    switch (v) {
        case ProfSessionState::Idle:      return "Idle";
        case ProfSessionState::Running:   return "Running";
        case ProfSessionState::Paused:    return "Paused";
        case ProfSessionState::Stopped:   return "Stopped";
        case ProfSessionState::Analyzing: return "Analyzing";
    }
    return "Unknown";
}

class ProfilingSession {
public:
    explicit ProfilingSession(uint32_t id, const std::string& name,
                               ProfSessionMode mode, ProfSessionState state)
        : m_id(id), m_name(name), m_mode(mode), m_state(state) {}

    void setMaxDurationSecs(float v)    { m_maxDurationSecs = v; }
    void setSampleRateHz(uint32_t v)    { m_sampleRateHz    = v; }
    void setIsEnabled(bool v)           { m_isEnabled       = v; }

    [[nodiscard]] uint32_t           id()              const { return m_id;              }
    [[nodiscard]] const std::string& name()            const { return m_name;            }
    [[nodiscard]] ProfSessionMode    mode()            const { return m_mode;            }
    [[nodiscard]] ProfSessionState   state()           const { return m_state;           }
    [[nodiscard]] float              maxDurationSecs() const { return m_maxDurationSecs; }
    [[nodiscard]] uint32_t           sampleRateHz()   const { return m_sampleRateHz;    }
    [[nodiscard]] bool               isEnabled()       const { return m_isEnabled;       }

private:
    uint32_t        m_id;
    std::string     m_name;
    ProfSessionMode  m_mode;
    ProfSessionState m_state;
    float           m_maxDurationSecs = 60.0f;
    uint32_t        m_sampleRateHz    = 60u;
    bool            m_isEnabled       = true;
};

class ProfilingSessionEditor {
public:
    void setIsShowDisabled(bool v)         { m_isShowDisabled       = v; }
    void setIsGroupByMode(bool v)          { m_isGroupByMode        = v; }
    void setDefaultSampleRateHz(uint32_t v){ m_defaultSampleRateHz  = v; }

    bool addSession(const ProfilingSession& s) {
        for (auto& x : m_sessions) if (x.id() == s.id()) return false;
        m_sessions.push_back(s); return true;
    }
    bool removeSession(uint32_t id) {
        auto it = std::find_if(m_sessions.begin(), m_sessions.end(),
            [&](const ProfilingSession& s){ return s.id() == id; });
        if (it == m_sessions.end()) return false;
        m_sessions.erase(it); return true;
    }
    [[nodiscard]] ProfilingSession* findSession(uint32_t id) {
        for (auto& s : m_sessions) if (s.id() == id) return &s;
        return nullptr;
    }

    [[nodiscard]] bool     isShowDisabled()      const { return m_isShowDisabled;      }
    [[nodiscard]] bool     isGroupByMode()       const { return m_isGroupByMode;       }
    [[nodiscard]] uint32_t defaultSampleRateHz() const { return m_defaultSampleRateHz; }
    [[nodiscard]] size_t   sessionCount()        const { return m_sessions.size();     }

    [[nodiscard]] size_t countByMode(ProfSessionMode m) const {
        size_t n = 0; for (auto& s : m_sessions) if (s.mode() == m) ++n; return n;
    }
    [[nodiscard]] size_t countByState(ProfSessionState st) const {
        size_t n = 0; for (auto& s : m_sessions) if (s.state() == st) ++n; return n;
    }
    [[nodiscard]] size_t countEnabled() const {
        size_t n = 0; for (auto& s : m_sessions) if (s.isEnabled()) ++n; return n;
    }

private:
    std::vector<ProfilingSession> m_sessions;
    bool     m_isShowDisabled      = false;
    bool     m_isGroupByMode       = false;
    uint32_t m_defaultSampleRateHz = 120u;
};

} // namespace NF
