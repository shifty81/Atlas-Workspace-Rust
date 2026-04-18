#pragma once
// NF::Editor — spectator editor
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

enum class SpectatorViewMode : uint8_t {
    FirstPerson, ThirdPerson, Overhead, Director, Broadcast, Free
};

inline const char* spectatorViewModeName(SpectatorViewMode v) {
    switch (v) {
        case SpectatorViewMode::FirstPerson:  return "FirstPerson";
        case SpectatorViewMode::ThirdPerson:  return "ThirdPerson";
        case SpectatorViewMode::Overhead:     return "Overhead";
        case SpectatorViewMode::Director:     return "Director";
        case SpectatorViewMode::Broadcast:    return "Broadcast";
        case SpectatorViewMode::Free:         return "Free";
    }
    return "Unknown";
}

enum class SpectatorTarget : uint8_t {
    Player, Team, Ball, POI, Auto
};

inline const char* spectatorTargetName(SpectatorTarget t) {
    switch (t) {
        case SpectatorTarget::Player: return "Player";
        case SpectatorTarget::Team:   return "Team";
        case SpectatorTarget::Ball:   return "Ball";
        case SpectatorTarget::POI:    return "POI";
        case SpectatorTarget::Auto:   return "Auto";
    }
    return "Unknown";
}

enum class SpectatorHUDLayout : uint8_t {
    Minimal, Standard, Broadcast, Custom
};

inline const char* spectatorHUDLayoutName(SpectatorHUDLayout l) {
    switch (l) {
        case SpectatorHUDLayout::Minimal:   return "Minimal";
        case SpectatorHUDLayout::Standard:  return "Standard";
        case SpectatorHUDLayout::Broadcast: return "Broadcast";
        case SpectatorHUDLayout::Custom:    return "Custom";
    }
    return "Unknown";
}

class SpectatorCamera {
public:
    explicit SpectatorCamera(uint32_t id, const std::string& name, SpectatorViewMode viewMode)
        : m_id(id), m_name(name), m_viewMode(viewMode) {}

    void setTarget(SpectatorTarget v)       { m_target         = v; }
    void setHUDLayout(SpectatorHUDLayout v) { m_hudLayout      = v; }
    void setFOV(float v)                    { m_fov            = v; }
    void setAutoSwitch(bool v)              { m_isAutoSwitch   = v; }
    void setSwitchInterval(float v)         { m_switchInterval = v; }

    [[nodiscard]] uint32_t            id()             const { return m_id;            }
    [[nodiscard]] const std::string&  name()           const { return m_name;          }
    [[nodiscard]] SpectatorViewMode   viewMode()       const { return m_viewMode;      }
    [[nodiscard]] SpectatorTarget     target()         const { return m_target;        }
    [[nodiscard]] SpectatorHUDLayout  hudLayout()      const { return m_hudLayout;     }
    [[nodiscard]] float               fov()            const { return m_fov;           }
    [[nodiscard]] bool                isAutoSwitch()   const { return m_isAutoSwitch;  }
    [[nodiscard]] float               switchInterval() const { return m_switchInterval;}

private:
    uint32_t           m_id;
    std::string        m_name;
    SpectatorViewMode  m_viewMode;
    SpectatorTarget    m_target         = SpectatorTarget::Auto;
    SpectatorHUDLayout m_hudLayout      = SpectatorHUDLayout::Standard;
    float              m_fov            = 90.0f;
    bool               m_isAutoSwitch   = true;
    float              m_switchInterval = 5.0f;
};

class SpectatorEditor {
public:
    void setActiveLayoutId(uint32_t v)         { m_activeLayoutId      = v; }
    void setShowPlayerNames(bool v)            { m_showPlayerNames     = v; }
    void setShowScoreboard(bool v)             { m_showScoreboard      = v; }
    void setTransitionDuration(float v)        { m_transitionDuration  = v; }

    bool addCamera(const SpectatorCamera& c) {
        for (auto& e : m_cameras) if (e.id() == c.id()) return false;
        m_cameras.push_back(c); return true;
    }
    bool removeCamera(uint32_t id) {
        auto it = std::find_if(m_cameras.begin(), m_cameras.end(),
            [&](const SpectatorCamera& e){ return e.id() == id; });
        if (it == m_cameras.end()) return false;
        m_cameras.erase(it); return true;
    }
    [[nodiscard]] SpectatorCamera* findCamera(uint32_t id) {
        for (auto& e : m_cameras) if (e.id() == id) return &e;
        return nullptr;
    }

    [[nodiscard]] uint32_t activeLayoutId()       const { return m_activeLayoutId;     }
    [[nodiscard]] bool     isShowPlayerNames()    const { return m_showPlayerNames;    }
    [[nodiscard]] bool     isShowScoreboard()     const { return m_showScoreboard;     }
    [[nodiscard]] float    transitionDuration()   const { return m_transitionDuration; }
    [[nodiscard]] size_t   cameraCount()          const { return m_cameras.size();     }

    [[nodiscard]] size_t countByViewMode(SpectatorViewMode v) const {
        size_t c = 0; for (auto& e : m_cameras) if (e.viewMode() == v) ++c; return c;
    }
    [[nodiscard]] size_t countByTarget(SpectatorTarget t) const {
        size_t c = 0; for (auto& e : m_cameras) if (e.target() == t) ++c; return c;
    }
    [[nodiscard]] size_t countAutoSwitch() const {
        size_t c = 0; for (auto& e : m_cameras) if (e.isAutoSwitch()) ++c; return c;
    }

private:
    std::vector<SpectatorCamera> m_cameras;
    uint32_t m_activeLayoutId     = 0u;
    bool     m_showPlayerNames    = true;
    bool     m_showScoreboard     = true;
    float    m_transitionDuration = 1.0f;
};

} // namespace NF
