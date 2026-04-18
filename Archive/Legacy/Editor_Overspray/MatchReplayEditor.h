#pragma once
// NF::Editor — match replay editor
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

enum class MatchPhase : uint8_t {
    PreGame, Opening, MidGame, LateGame, Overtime, PostGame
};

inline const char* matchPhaseName(MatchPhase p) {
    switch (p) {
        case MatchPhase::PreGame:  return "PreGame";
        case MatchPhase::Opening:  return "Opening";
        case MatchPhase::MidGame:  return "MidGame";
        case MatchPhase::LateGame: return "LateGame";
        case MatchPhase::Overtime: return "Overtime";
        case MatchPhase::PostGame: return "PostGame";
    }
    return "Unknown";
}

enum class MatchEventType : uint8_t {
    Kill, Death, Assist, Capture, Score, PowerUp, TeamAction, Custom
};

inline const char* matchEventTypeName(MatchEventType t) {
    switch (t) {
        case MatchEventType::Kill:       return "Kill";
        case MatchEventType::Death:      return "Death";
        case MatchEventType::Assist:     return "Assist";
        case MatchEventType::Capture:    return "Capture";
        case MatchEventType::Score:      return "Score";
        case MatchEventType::PowerUp:    return "PowerUp";
        case MatchEventType::TeamAction: return "TeamAction";
        case MatchEventType::Custom:     return "Custom";
    }
    return "Unknown";
}

enum class ReplayCameraMode : uint8_t {
    Free, Follow, Overview, Fixed, Dynamic
};

inline const char* replayCameraModeName(ReplayCameraMode m) {
    switch (m) {
        case ReplayCameraMode::Free:     return "Free";
        case ReplayCameraMode::Follow:   return "Follow";
        case ReplayCameraMode::Overview: return "Overview";
        case ReplayCameraMode::Fixed:    return "Fixed";
        case ReplayCameraMode::Dynamic:  return "Dynamic";
    }
    return "Unknown";
}

class MatchEvent {
public:
    explicit MatchEvent(uint32_t id, const std::string& name, MatchEventType eventType)
        : m_id(id), m_name(name), m_eventType(eventType) {}

    void setPhase(MatchPhase v)          { m_phase        = v; }
    void setTimestampSec(float v)        { m_timestampSec = v; }
    void setTeamId(uint32_t v)           { m_teamId       = v; }
    void setPlayerId(uint32_t v)         { m_playerId     = v; }
    void setHighlight(bool v)            { m_isHighlight  = v; }

    [[nodiscard]] uint32_t           id()            const { return m_id;           }
    [[nodiscard]] const std::string& name()          const { return m_name;         }
    [[nodiscard]] MatchEventType     eventType()     const { return m_eventType;    }
    [[nodiscard]] MatchPhase         phase()         const { return m_phase;        }
    [[nodiscard]] float              timestampSec()  const { return m_timestampSec; }
    [[nodiscard]] uint32_t           teamId()        const { return m_teamId;       }
    [[nodiscard]] uint32_t           playerId()      const { return m_playerId;     }
    [[nodiscard]] bool               isHighlight()   const { return m_isHighlight;  }

private:
    uint32_t      m_id;
    std::string   m_name;
    MatchEventType m_eventType;
    MatchPhase    m_phase        = MatchPhase::MidGame;
    float         m_timestampSec = 0.0f;
    uint32_t      m_teamId       = 0u;
    uint32_t      m_playerId     = 0u;
    bool          m_isHighlight  = false;
};

class MatchReplayEditor {
public:
    void setCameraMode(ReplayCameraMode v) { m_cameraMode   = v; }
    void setShowMinimap(bool v)            { m_showMinimap  = v; }
    void setShowStats(bool v)              { m_showStats    = v; }
    void setReplaySpeed(float v)           { m_replaySpeed  = v; }

    bool addEvent(const MatchEvent& e) {
        for (auto& ev : m_events) if (ev.id() == e.id()) return false;
        m_events.push_back(e); return true;
    }
    bool removeEvent(uint32_t id) {
        auto it = std::find_if(m_events.begin(), m_events.end(),
            [&](const MatchEvent& e){ return e.id() == id; });
        if (it == m_events.end()) return false;
        m_events.erase(it); return true;
    }
    [[nodiscard]] MatchEvent* findEvent(uint32_t id) {
        for (auto& e : m_events) if (e.id() == id) return &e;
        return nullptr;
    }

    [[nodiscard]] ReplayCameraMode cameraMode()    const { return m_cameraMode;    }
    [[nodiscard]] bool             isShowMinimap() const { return m_showMinimap;   }
    [[nodiscard]] bool             isShowStats()   const { return m_showStats;     }
    [[nodiscard]] float            replaySpeed()   const { return m_replaySpeed;   }
    [[nodiscard]] size_t           eventCount()    const { return m_events.size(); }

    [[nodiscard]] size_t countByPhase(MatchPhase p) const {
        size_t c = 0; for (auto& e : m_events) if (e.phase() == p) ++c; return c;
    }
    [[nodiscard]] size_t countByEventType(MatchEventType t) const {
        size_t c = 0; for (auto& e : m_events) if (e.eventType() == t) ++c; return c;
    }
    [[nodiscard]] size_t countHighlights() const {
        size_t c = 0; for (auto& e : m_events) if (e.isHighlight()) ++c; return c;
    }

private:
    std::vector<MatchEvent> m_events;
    ReplayCameraMode m_cameraMode  = ReplayCameraMode::Dynamic;
    bool             m_showMinimap = true;
    bool             m_showStats   = true;
    float            m_replaySpeed = 1.0f;
};

} // namespace NF
