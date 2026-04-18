#pragma once
// NF::Editor — Cinematics editor and sequence player
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

enum class CinematicShotType : uint8_t {
    Cut, Dissolve, FadeIn, FadeOut, Wipe, CrossFade
};

inline const char* cinematicShotTypeName(CinematicShotType t) {
    switch (t) {
        case CinematicShotType::Cut:       return "Cut";
        case CinematicShotType::Dissolve:  return "Dissolve";
        case CinematicShotType::FadeIn:    return "FadeIn";
        case CinematicShotType::FadeOut:   return "FadeOut";
        case CinematicShotType::Wipe:      return "Wipe";
        case CinematicShotType::CrossFade: return "CrossFade";
    }
    return "Unknown";
}

enum class CinematicPlayMode : uint8_t {
    Once, Loop, PingPong, Manual
};

inline const char* cinematicPlayModeName(CinematicPlayMode m) {
    switch (m) {
        case CinematicPlayMode::Once:     return "Once";
        case CinematicPlayMode::Loop:     return "Loop";
        case CinematicPlayMode::PingPong: return "PingPong";
        case CinematicPlayMode::Manual:   return "Manual";
    }
    return "Unknown";
}

enum class CinematicTrackBind : uint8_t {
    Camera, Actor, Light, Audio, Subtitle, PostProcess
};

inline const char* cinematicTrackBindName(CinematicTrackBind b) {
    switch (b) {
        case CinematicTrackBind::Camera:      return "Camera";
        case CinematicTrackBind::Actor:       return "Actor";
        case CinematicTrackBind::Light:       return "Light";
        case CinematicTrackBind::Audio:       return "Audio";
        case CinematicTrackBind::Subtitle:    return "Subtitle";
        case CinematicTrackBind::PostProcess: return "PostProcess";
    }
    return "Unknown";
}

class CinematicShot {
public:
    explicit CinematicShot(const std::string& name, CinematicShotType type, float duration)
        : m_name(name), m_type(type), m_duration(duration) {}

    void setStartTime(float t)          { m_startTime  = t; }
    void setBoundCamera(const std::string& cam) { m_boundCamera = cam; }
    void setEnabled(bool v)             { m_enabled    = v; }

    [[nodiscard]] const std::string&  name()        const { return m_name;        }
    [[nodiscard]] CinematicShotType   type()        const { return m_type;        }
    [[nodiscard]] float               duration()    const { return m_duration;    }
    [[nodiscard]] float               startTime()   const { return m_startTime;   }
    [[nodiscard]] const std::string&  boundCamera() const { return m_boundCamera; }
    [[nodiscard]] bool                isEnabled()   const { return m_enabled;     }

private:
    std::string        m_name;
    CinematicShotType  m_type;
    float              m_duration;
    float              m_startTime  = 0.0f;
    std::string        m_boundCamera;
    bool               m_enabled   = true;
};

class CinematicsEditor {
public:
    static constexpr size_t MAX_SHOTS = 256;

    [[nodiscard]] bool addShot(const CinematicShot& shot) {
        for (auto& s : m_shots) if (s.name() == shot.name()) return false;
        if (m_shots.size() >= MAX_SHOTS) return false;
        m_shots.push_back(shot);
        return true;
    }

    [[nodiscard]] bool removeShot(const std::string& name) {
        for (auto it = m_shots.begin(); it != m_shots.end(); ++it) {
            if (it->name() == name) {
                if (m_activeShot == name) m_activeShot.clear();
                m_shots.erase(it);
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] CinematicShot* findShot(const std::string& name) {
        for (auto& s : m_shots) if (s.name() == name) return &s;
        return nullptr;
    }

    [[nodiscard]] bool setActiveShot(const std::string& name) {
        for (auto& s : m_shots) if (s.name() == name) { m_activeShot = name; return true; }
        return false;
    }

    void setPlayMode(CinematicPlayMode m) { m_playMode = m; }
    void setPlaying(bool v)               { m_playing  = v; }
    void setPlayhead(float t)             { m_playhead = t; }

    [[nodiscard]] const std::string& activeShot() const { return m_activeShot; }
    [[nodiscard]] CinematicPlayMode  playMode()   const { return m_playMode;   }
    [[nodiscard]] bool               isPlaying()  const { return m_playing;    }
    [[nodiscard]] float              playhead()   const { return m_playhead;   }
    [[nodiscard]] size_t             shotCount()  const { return m_shots.size(); }

    [[nodiscard]] size_t enabledCount() const {
        size_t c = 0; for (auto& s : m_shots) if (s.isEnabled()) ++c; return c;
    }
    [[nodiscard]] size_t countByType(CinematicShotType t) const {
        size_t c = 0; for (auto& s : m_shots) if (s.type() == t) ++c; return c;
    }
    [[nodiscard]] float totalDuration() const {
        float d = 0.0f; for (auto& s : m_shots) d += s.duration(); return d;
    }

private:
    std::vector<CinematicShot> m_shots;
    std::string                m_activeShot;
    CinematicPlayMode          m_playMode = CinematicPlayMode::Once;
    float                      m_playhead = 0.0f;
    bool                       m_playing  = false;
};

} // namespace NF
