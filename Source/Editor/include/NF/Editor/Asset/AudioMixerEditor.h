#pragma once
// NF::Editor — Audio mixer editor
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

enum class AudioBusType : uint8_t {
    Master, Music, SFX, Voice, Ambient, UI, Reverb, Submix
};

inline const char* audioBusTypeName(AudioBusType t) {
    switch (t) {
        case AudioBusType::Master:  return "Master";
        case AudioBusType::Music:   return "Music";
        case AudioBusType::SFX:     return "SFX";
        case AudioBusType::Voice:   return "Voice";
        case AudioBusType::Ambient: return "Ambient";
        case AudioBusType::UI:      return "UI";
        case AudioBusType::Reverb:  return "Reverb";
        case AudioBusType::Submix:  return "Submix";
    }
    return "Unknown";
}

enum class AudioEffectType : uint8_t {
    EQ, Compressor, Reverb, Delay, Chorus, Distortion, Limiter, Filter
};

inline const char* audioEffectTypeName(AudioEffectType t) {
    switch (t) {
        case AudioEffectType::EQ:          return "EQ";
        case AudioEffectType::Compressor:  return "Compressor";
        case AudioEffectType::Reverb:      return "Reverb";
        case AudioEffectType::Delay:       return "Delay";
        case AudioEffectType::Chorus:      return "Chorus";
        case AudioEffectType::Distortion:  return "Distortion";
        case AudioEffectType::Limiter:     return "Limiter";
        case AudioEffectType::Filter:      return "Filter";
    }
    return "Unknown";
}

enum class AudioMixerView : uint8_t {
    Channels, Routing, Effects, Sends
};

inline const char* audioMixerViewName(AudioMixerView v) {
    switch (v) {
        case AudioMixerView::Channels: return "Channels";
        case AudioMixerView::Routing:  return "Routing";
        case AudioMixerView::Effects:  return "Effects";
        case AudioMixerView::Sends:    return "Sends";
    }
    return "Unknown";
}

class AudioBus {
public:
    explicit AudioBus(const std::string& name, AudioBusType type)
        : m_name(name), m_type(type) {}

    void setVolume(float v)   { m_volume = v; }
    void setPitch(float v)    { m_pitch  = v; }
    void setMute(bool v)      { m_muted  = v; }
    void setSolo(bool v)      { m_solo   = v; }
    void setBypass(bool v)    { m_bypass = v; }

    [[nodiscard]] const std::string& name()    const { return m_name;   }
    [[nodiscard]] AudioBusType       type()    const { return m_type;   }
    [[nodiscard]] float              volume()  const { return m_volume; }
    [[nodiscard]] float              pitch()   const { return m_pitch;  }
    [[nodiscard]] bool               isMuted() const { return m_muted;  }
    [[nodiscard]] bool               isSolo()  const { return m_solo;   }
    [[nodiscard]] bool               isBypassed() const { return m_bypass; }

    void addEffect(AudioEffectType e)    { m_effects.push_back(e); }
    bool removeEffect(AudioEffectType e) {
        auto it = std::find(m_effects.begin(), m_effects.end(), e);
        if (it == m_effects.end()) return false;
        m_effects.erase(it); return true;
    }
    [[nodiscard]] size_t effectCount() const { return m_effects.size(); }

private:
    std::string                   m_name;
    AudioBusType                  m_type;
    float                         m_volume  = 1.0f;
    float                         m_pitch   = 1.0f;
    bool                          m_muted   = false;
    bool                          m_solo    = false;
    bool                          m_bypass  = false;
    std::vector<AudioEffectType>  m_effects;
};

class AudioMixerEditor {
public:
    void setView(AudioMixerView v) { m_view = v; }

    bool addBus(const AudioBus& bus) {
        for (auto& b : m_buses) if (b.name() == bus.name()) return false;
        m_buses.push_back(bus); return true;
    }
    bool removeBus(const std::string& name) {
        auto it = std::find_if(m_buses.begin(), m_buses.end(),
            [&](const AudioBus& b){ return b.name() == name; });
        if (it == m_buses.end()) return false;
        m_buses.erase(it); return true;
    }
    [[nodiscard]] AudioBus* findBus(const std::string& name) {
        for (auto& b : m_buses) if (b.name() == name) return &b;
        return nullptr;
    }

    [[nodiscard]] AudioMixerView view()     const { return m_view; }
    [[nodiscard]] size_t         busCount() const { return m_buses.size(); }

    [[nodiscard]] size_t mutedCount()  const {
        size_t c = 0; for (auto& b : m_buses) if (b.isMuted()) ++c; return c;
    }
    [[nodiscard]] size_t soloCount()   const {
        size_t c = 0; for (auto& b : m_buses) if (b.isSolo()) ++c; return c;
    }
    [[nodiscard]] size_t countByType(AudioBusType t) const {
        size_t c = 0; for (auto& b : m_buses) if (b.type() == t) ++c; return c;
    }

private:
    std::vector<AudioBus> m_buses;
    AudioMixerView        m_view = AudioMixerView::Channels;
};

} // namespace NF
