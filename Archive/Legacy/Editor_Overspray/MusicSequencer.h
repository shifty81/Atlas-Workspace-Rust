#pragma once
// NF::Editor — Music sequencer
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

enum class MusicLayerState : uint8_t {
    Inactive, FadingIn, Active, FadingOut, Looping
};

inline const char* musicLayerStateName(MusicLayerState s) {
    switch (s) {
        case MusicLayerState::Inactive:  return "Inactive";
        case MusicLayerState::FadingIn:  return "FadingIn";
        case MusicLayerState::Active:    return "Active";
        case MusicLayerState::FadingOut: return "FadingOut";
        case MusicLayerState::Looping:   return "Looping";
    }
    return "Unknown";
}

enum class MusicSyncMode : uint8_t {
    Immediate, OnBeat, OnBar, OnSection, OnEnd
};

inline const char* musicSyncModeName(MusicSyncMode m) {
    switch (m) {
        case MusicSyncMode::Immediate: return "Immediate";
        case MusicSyncMode::OnBeat:    return "OnBeat";
        case MusicSyncMode::OnBar:     return "OnBar";
        case MusicSyncMode::OnSection: return "OnSection";
        case MusicSyncMode::OnEnd:     return "OnEnd";
    }
    return "Unknown";
}

enum class MusicTransitionType : uint8_t {
    Cut, CrossFade, Stinger, Segue
};

inline const char* musicTransitionTypeName(MusicTransitionType t) {
    switch (t) {
        case MusicTransitionType::Cut:       return "Cut";
        case MusicTransitionType::CrossFade: return "CrossFade";
        case MusicTransitionType::Stinger:   return "Stinger";
        case MusicTransitionType::Segue:     return "Segue";
    }
    return "Unknown";
}

class MusicLayer {
public:
    explicit MusicLayer(uint32_t id, const std::string& name)
        : m_id(id), m_name(name) {}

    void setState(MusicLayerState s) { m_state  = s; }
    void setVolume(float v)          { m_volume = v; }
    void setMute(bool v)             { m_muted  = v; }
    void setSolo(bool v)             { m_solo   = v; }
    void setFadeTime(float v)        { m_fadeTime = v; }

    [[nodiscard]] uint32_t         id()       const { return m_id;      }
    [[nodiscard]] const std::string& name()   const { return m_name;    }
    [[nodiscard]] MusicLayerState  state()    const { return m_state;   }
    [[nodiscard]] float            volume()   const { return m_volume;  }
    [[nodiscard]] bool             isMuted()  const { return m_muted;   }
    [[nodiscard]] bool             isSolo()   const { return m_solo;    }
    [[nodiscard]] float            fadeTime() const { return m_fadeTime;}

private:
    uint32_t       m_id;
    std::string    m_name;
    MusicLayerState m_state    = MusicLayerState::Inactive;
    float          m_volume   = 1.0f;
    bool           m_muted    = false;
    bool           m_solo     = false;
    float          m_fadeTime = 1.0f;
};

class MusicSequencer {
public:
    void setSyncMode(MusicSyncMode m)        { m_syncMode  = m; }
    void setTransitionType(MusicTransitionType t){ m_transition = t; }
    void setTempo(float bpm)                 { m_tempo     = bpm; }
    void setPlaying(bool v)                  { m_playing   = v; }
    void setLoopEnabled(bool v)              { m_looping   = v; }

    bool addLayer(const MusicLayer& layer) {
        for (auto& l : m_layers) if (l.id() == layer.id()) return false;
        m_layers.push_back(layer); return true;
    }
    bool removeLayer(uint32_t id) {
        auto it = std::find_if(m_layers.begin(), m_layers.end(),
            [&](const MusicLayer& l){ return l.id() == id; });
        if (it == m_layers.end()) return false;
        m_layers.erase(it); return true;
    }
    [[nodiscard]] MusicLayer* findLayer(uint32_t id) {
        for (auto& l : m_layers) if (l.id() == id) return &l;
        return nullptr;
    }

    [[nodiscard]] MusicSyncMode       syncMode()       const { return m_syncMode;   }
    [[nodiscard]] MusicTransitionType transitionType() const { return m_transition; }
    [[nodiscard]] float               tempo()          const { return m_tempo;      }
    [[nodiscard]] bool                isPlaying()      const { return m_playing;    }
    [[nodiscard]] bool                isLooping()      const { return m_looping;    }
    [[nodiscard]] size_t              layerCount()     const { return m_layers.size(); }

    [[nodiscard]] size_t activeLayerCount() const {
        size_t c = 0;
        for (auto& l : m_layers)
            if (l.state() == MusicLayerState::Active || l.state() == MusicLayerState::Looping) ++c;
        return c;
    }
    [[nodiscard]] size_t mutedLayerCount() const {
        size_t c = 0; for (auto& l : m_layers) if (l.isMuted()) ++c; return c;
    }

private:
    std::vector<MusicLayer> m_layers;
    MusicSyncMode           m_syncMode   = MusicSyncMode::OnBar;
    MusicTransitionType     m_transition = MusicTransitionType::CrossFade;
    float                   m_tempo      = 120.0f;
    bool                    m_playing    = false;
    bool                    m_looping    = true;
};

} // namespace NF
