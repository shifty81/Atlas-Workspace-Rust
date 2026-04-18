#pragma once
// NF::Editor — playmode session editor
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

enum class PlaymodeState : uint8_t { Stopped, Entering, Running, Paused, Exiting };
inline const char* playmodeStateName(PlaymodeState v) {
    switch (v) {
        case PlaymodeState::Stopped:  return "Stopped";
        case PlaymodeState::Entering: return "Entering";
        case PlaymodeState::Running:  return "Running";
        case PlaymodeState::Paused:   return "Paused";
        case PlaymodeState::Exiting:  return "Exiting";
    }
    return "Unknown";
}

enum class PlaymodeTarget : uint8_t { Editor, StandaloneWindow, DevicePreview, Headless, Custom };
inline const char* playmodeTargetName(PlaymodeTarget v) {
    switch (v) {
        case PlaymodeTarget::Editor:           return "Editor";
        case PlaymodeTarget::StandaloneWindow: return "StandaloneWindow";
        case PlaymodeTarget::DevicePreview:    return "DevicePreview";
        case PlaymodeTarget::Headless:         return "Headless";
        case PlaymodeTarget::Custom:           return "Custom";
    }
    return "Unknown";
}

class PlaymodeSession {
public:
    explicit PlaymodeSession(uint32_t id, const std::string& name, PlaymodeTarget target)
        : m_id(id), m_name(name), m_target(target) {}

    void setState(PlaymodeState v)   { m_state       = v; }
    void setFrameCount(uint32_t v)   { m_frameCount  = v; }
    void setIsRecording(bool v)      { m_isRecording = v; }
    void setIsEnabled(bool v)        { m_isEnabled   = v; }

    [[nodiscard]] uint32_t           id()          const { return m_id;          }
    [[nodiscard]] const std::string& name()        const { return m_name;        }
    [[nodiscard]] PlaymodeTarget     target()      const { return m_target;      }
    [[nodiscard]] PlaymodeState      state()       const { return m_state;       }
    [[nodiscard]] uint32_t           frameCount()  const { return m_frameCount;  }
    [[nodiscard]] bool               isRecording() const { return m_isRecording; }
    [[nodiscard]] bool               isEnabled()   const { return m_isEnabled;   }

private:
    uint32_t        m_id;
    std::string     m_name;
    PlaymodeTarget  m_target;
    PlaymodeState   m_state       = PlaymodeState::Stopped;
    uint32_t        m_frameCount  = 0u;
    bool            m_isRecording = false;
    bool            m_isEnabled   = true;
};

class PlaymodeEditor {
public:
    void setIsAutoRecompile(bool v)  { m_isAutoRecompile  = v; }
    void setIsRestoreOnExit(bool v)  { m_isRestoreOnExit  = v; }
    void setTargetFPS(uint32_t v)    { m_targetFPS        = v; }

    bool addSession(const PlaymodeSession& s) {
        for (auto& x : m_sessions) if (x.id() == s.id()) return false;
        m_sessions.push_back(s); return true;
    }
    bool removeSession(uint32_t id) {
        auto it = std::find_if(m_sessions.begin(), m_sessions.end(),
            [&](const PlaymodeSession& s){ return s.id() == id; });
        if (it == m_sessions.end()) return false;
        m_sessions.erase(it); return true;
    }
    [[nodiscard]] PlaymodeSession* findSession(uint32_t id) {
        for (auto& s : m_sessions) if (s.id() == id) return &s;
        return nullptr;
    }

    [[nodiscard]] bool     isAutoRecompile()  const { return m_isAutoRecompile;  }
    [[nodiscard]] bool     isRestoreOnExit()  const { return m_isRestoreOnExit;  }
    [[nodiscard]] uint32_t targetFPS()        const { return m_targetFPS;        }
    [[nodiscard]] size_t   sessionCount()     const { return m_sessions.size();  }

    [[nodiscard]] size_t countByState(PlaymodeState s) const {
        size_t n = 0; for (auto& ss : m_sessions) if (ss.state() == s) ++n; return n;
    }
    [[nodiscard]] size_t countByTarget(PlaymodeTarget t) const {
        size_t n = 0; for (auto& ss : m_sessions) if (ss.target() == t) ++n; return n;
    }
    [[nodiscard]] size_t countRecording() const {
        size_t n = 0; for (auto& ss : m_sessions) if (ss.isRecording()) ++n; return n;
    }

private:
    std::vector<PlaymodeSession> m_sessions;
    bool     m_isAutoRecompile = true;
    bool     m_isRestoreOnExit = true;
    uint32_t m_targetFPS       = 60u;
};

} // namespace NF
