#pragma once
// NF::Editor — Cutscene director panel
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

enum class CutsceneShotCut : uint8_t {
    Cut, Dissolve, Fade, Wipe, ZoomIn, ZoomOut
};

inline const char* cutsceneShotCutName(CutsceneShotCut c) {
    switch (c) {
        case CutsceneShotCut::Cut:     return "Cut";
        case CutsceneShotCut::Dissolve:return "Dissolve";
        case CutsceneShotCut::Fade:    return "Fade";
        case CutsceneShotCut::Wipe:    return "Wipe";
        case CutsceneShotCut::ZoomIn:  return "ZoomIn";
        case CutsceneShotCut::ZoomOut: return "ZoomOut";
    }
    return "Unknown";
}

enum class CutsceneDirectorState : uint8_t {
    Idle, Previewing, Recording, Exporting, Done
};

inline const char* cutsceneDirectorStateName(CutsceneDirectorState s) {
    switch (s) {
        case CutsceneDirectorState::Idle:       return "Idle";
        case CutsceneDirectorState::Previewing: return "Previewing";
        case CutsceneDirectorState::Recording:  return "Recording";
        case CutsceneDirectorState::Exporting:  return "Exporting";
        case CutsceneDirectorState::Done:       return "Done";
    }
    return "Unknown";
}

enum class CutsceneOutputFormat : uint8_t {
    InGame, MP4, AVI, MOV, ImageSequence
};

inline const char* cutsceneOutputFormatName(CutsceneOutputFormat f) {
    switch (f) {
        case CutsceneOutputFormat::InGame:        return "InGame";
        case CutsceneOutputFormat::MP4:           return "MP4";
        case CutsceneOutputFormat::AVI:           return "AVI";
        case CutsceneOutputFormat::MOV:           return "MOV";
        case CutsceneOutputFormat::ImageSequence: return "ImageSequence";
    }
    return "Unknown";
}

class CutsceneShot {
public:
    explicit CutsceneShot(const std::string& name, CutsceneShotCut transition)
        : m_name(name), m_transition(transition) {}

    void setDuration(float d)      { m_duration   = d; }
    void setEnabled(bool v)        { m_enabled    = v; }
    void setCameraName(const std::string& cam) { m_camera = cam; }

    [[nodiscard]] const std::string& name()       const { return m_name;       }
    [[nodiscard]] CutsceneShotCut    transition()  const { return m_transition; }
    [[nodiscard]] float              duration()   const { return m_duration;   }
    [[nodiscard]] bool               isEnabled()  const { return m_enabled;    }
    [[nodiscard]] const std::string& cameraName() const { return m_camera;     }

private:
    std::string      m_name;
    CutsceneShotCut  m_transition;
    std::string      m_camera;
    float            m_duration = 5.0f;
    bool             m_enabled  = true;
};

class CutsceneDirector {
public:
    static constexpr size_t MAX_SHOTS = 256;

    [[nodiscard]] bool addShot(const CutsceneShot& shot) {
        for (auto& s : m_shots) if (s.name() == shot.name()) return false;
        if (m_shots.size() >= MAX_SHOTS) return false;
        m_shots.push_back(shot);
        return true;
    }

    [[nodiscard]] bool removeShot(const std::string& name) {
        for (auto it = m_shots.begin(); it != m_shots.end(); ++it) {
            if (it->name() == name) { m_shots.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] CutsceneShot* findShot(const std::string& name) {
        for (auto& s : m_shots) if (s.name() == name) return &s;
        return nullptr;
    }

    void setState(CutsceneDirectorState s)     { m_state  = s; }
    void setOutputFormat(CutsceneOutputFormat f){ m_format = f; }
    void setFPS(float fps)                     { m_fps    = fps; }

    [[nodiscard]] CutsceneDirectorState state()        const { return m_state;  }
    [[nodiscard]] CutsceneOutputFormat  outputFormat() const { return m_format; }
    [[nodiscard]] float                 fps()          const { return m_fps;    }
    [[nodiscard]] size_t                shotCount()    const { return m_shots.size(); }

    [[nodiscard]] bool isRecording() const { return m_state == CutsceneDirectorState::Recording; }
    [[nodiscard]] size_t enabledShotCount() const {
        size_t c = 0; for (auto& s : m_shots) if (s.isEnabled()) ++c; return c;
    }
    [[nodiscard]] size_t countByTransition(CutsceneShotCut t) const {
        size_t c = 0; for (auto& s : m_shots) if (s.transition() == t) ++c; return c;
    }
    [[nodiscard]] float totalDuration() const {
        float d = 0.0f; for (auto& s : m_shots) d += s.duration(); return d;
    }

private:
    std::vector<CutsceneShot>   m_shots;
    CutsceneDirectorState       m_state  = CutsceneDirectorState::Idle;
    CutsceneOutputFormat        m_format = CutsceneOutputFormat::InGame;
    float                       m_fps    = 30.0f;
};

} // namespace NF
