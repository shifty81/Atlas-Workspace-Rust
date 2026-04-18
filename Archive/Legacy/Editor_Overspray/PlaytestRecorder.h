#pragma once
// NF::Editor — playtest recorder
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

enum class PlaytestSessionState : uint8_t {
    Idle, Recording, Paused, Reviewing, Exported, Archived
};

inline const char* playtestSessionStateName(PlaytestSessionState s) {
    switch (s) {
        case PlaytestSessionState::Idle:       return "Idle";
        case PlaytestSessionState::Recording:  return "Recording";
        case PlaytestSessionState::Paused:     return "Paused";
        case PlaytestSessionState::Reviewing:  return "Reviewing";
        case PlaytestSessionState::Exported:   return "Exported";
        case PlaytestSessionState::Archived:   return "Archived";
    }
    return "Unknown";
}

enum class PlaytestCaptureMode : uint8_t {
    InputOnly, InputAndScreen, FullSession, EventStream, Annotated
};

inline const char* playtestCaptureModeName(PlaytestCaptureMode m) {
    switch (m) {
        case PlaytestCaptureMode::InputOnly:      return "InputOnly";
        case PlaytestCaptureMode::InputAndScreen: return "InputAndScreen";
        case PlaytestCaptureMode::FullSession:    return "FullSession";
        case PlaytestCaptureMode::EventStream:    return "EventStream";
        case PlaytestCaptureMode::Annotated:      return "Annotated";
    }
    return "Unknown";
}

enum class PlaytestAnnotationType : uint8_t {
    Bug, Suggestion, Highlight, Question, Blocker
};

inline const char* playtestAnnotationTypeName(PlaytestAnnotationType a) {
    switch (a) {
        case PlaytestAnnotationType::Bug:         return "Bug";
        case PlaytestAnnotationType::Suggestion:  return "Suggestion";
        case PlaytestAnnotationType::Highlight:   return "Highlight";
        case PlaytestAnnotationType::Question:    return "Question";
        case PlaytestAnnotationType::Blocker:     return "Blocker";
    }
    return "Unknown";
}

class PlaytestSession {
public:
    explicit PlaytestSession(uint32_t id, const std::string& name, PlaytestCaptureMode captureMode)
        : m_id(id), m_name(name), m_captureMode(captureMode) {}

    void setState(PlaytestSessionState v)  { m_state         = v; }
    void setDurationSeconds(float v)       { m_durationSec   = v; }
    void setAnnotationCount(uint32_t v)    { m_annotationCnt = v; }
    void setIsFlagged(bool v)              { m_isFlagged     = v; }
    void setTesterName(const std::string& v){ m_testerName   = v; }

    [[nodiscard]] uint32_t              id()              const { return m_id;            }
    [[nodiscard]] const std::string&    name()            const { return m_name;          }
    [[nodiscard]] PlaytestCaptureMode   captureMode()     const { return m_captureMode;   }
    [[nodiscard]] PlaytestSessionState  state()           const { return m_state;         }
    [[nodiscard]] float                 durationSeconds() const { return m_durationSec;   }
    [[nodiscard]] uint32_t              annotationCount() const { return m_annotationCnt; }
    [[nodiscard]] bool                  isFlagged()       const { return m_isFlagged;     }
    [[nodiscard]] const std::string&    testerName()      const { return m_testerName;    }

private:
    uint32_t             m_id;
    std::string          m_name;
    PlaytestCaptureMode  m_captureMode;
    PlaytestSessionState m_state         = PlaytestSessionState::Idle;
    float                m_durationSec   = 0.0f;
    uint32_t             m_annotationCnt = 0u;
    bool                 m_isFlagged     = false;
    std::string          m_testerName;
};

class PlaytestRecorder {
public:
    void setDefaultCaptureMode(PlaytestCaptureMode v) { m_defaultCaptureMode = v; }
    void setShowTimeline(bool v)                      { m_showTimeline       = v; }
    void setShowAnnotations(bool v)                   { m_showAnnotations    = v; }
    void setMaxSessionDurationSec(float v)            { m_maxSessionDurationSec = v; }

    bool addSession(const PlaytestSession& s) {
        for (auto& e : m_sessions) if (e.id() == s.id()) return false;
        m_sessions.push_back(s); return true;
    }
    bool removeSession(uint32_t id) {
        auto it = std::find_if(m_sessions.begin(), m_sessions.end(),
            [&](const PlaytestSession& e){ return e.id() == id; });
        if (it == m_sessions.end()) return false;
        m_sessions.erase(it); return true;
    }
    [[nodiscard]] PlaytestSession* findSession(uint32_t id) {
        for (auto& e : m_sessions) if (e.id() == id) return &e;
        return nullptr;
    }

    [[nodiscard]] PlaytestCaptureMode defaultCaptureMode()    const { return m_defaultCaptureMode;    }
    [[nodiscard]] bool                isShowTimeline()        const { return m_showTimeline;          }
    [[nodiscard]] bool                isShowAnnotations()     const { return m_showAnnotations;       }
    [[nodiscard]] float               maxSessionDurationSec() const { return m_maxSessionDurationSec; }
    [[nodiscard]] size_t              sessionCount()          const { return m_sessions.size();       }

    [[nodiscard]] size_t countByState(PlaytestSessionState s) const {
        size_t n = 0; for (auto& e : m_sessions) if (e.state() == s) ++n; return n;
    }
    [[nodiscard]] size_t countByCaptureMode(PlaytestCaptureMode m) const {
        size_t n = 0; for (auto& e : m_sessions) if (e.captureMode() == m) ++n; return n;
    }
    [[nodiscard]] size_t countFlagged() const {
        size_t n = 0; for (auto& e : m_sessions) if (e.isFlagged()) ++n; return n;
    }

private:
    std::vector<PlaytestSession> m_sessions;
    PlaytestCaptureMode m_defaultCaptureMode    = PlaytestCaptureMode::InputAndScreen;
    bool                m_showTimeline         = true;
    bool                m_showAnnotations      = true;
    float               m_maxSessionDurationSec = 3600.0f;
};

} // namespace NF
