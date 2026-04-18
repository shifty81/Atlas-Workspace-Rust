#pragma once
// NF::Editor — Sound effect editor
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

enum class SfxTriggerMode : uint8_t {
    OnEnter, OnExit, OnLoop, OnDistance, OnEvent, OnTimer, Manual
};

inline const char* sfxTriggerModeName(SfxTriggerMode m) {
    switch (m) {
        case SfxTriggerMode::OnEnter:    return "OnEnter";
        case SfxTriggerMode::OnExit:     return "OnExit";
        case SfxTriggerMode::OnLoop:     return "OnLoop";
        case SfxTriggerMode::OnDistance: return "OnDistance";
        case SfxTriggerMode::OnEvent:    return "OnEvent";
        case SfxTriggerMode::OnTimer:    return "OnTimer";
        case SfxTriggerMode::Manual:     return "Manual";
    }
    return "Unknown";
}

enum class SfxAttenuationType : uint8_t {
    None, Linear, Logarithmic, InverseSquare, Custom
};

inline const char* sfxAttenuationTypeName(SfxAttenuationType t) {
    switch (t) {
        case SfxAttenuationType::None:          return "None";
        case SfxAttenuationType::Linear:        return "Linear";
        case SfxAttenuationType::Logarithmic:   return "Logarithmic";
        case SfxAttenuationType::InverseSquare: return "InverseSquare";
        case SfxAttenuationType::Custom:        return "Custom";
    }
    return "Unknown";
}

enum class SfxPlaybackState : uint8_t {
    Stopped, Playing, Paused, FadingIn, FadingOut
};

inline const char* sfxPlaybackStateName(SfxPlaybackState s) {
    switch (s) {
        case SfxPlaybackState::Stopped:   return "Stopped";
        case SfxPlaybackState::Playing:   return "Playing";
        case SfxPlaybackState::Paused:    return "Paused";
        case SfxPlaybackState::FadingIn:  return "FadingIn";
        case SfxPlaybackState::FadingOut: return "FadingOut";
    }
    return "Unknown";
}

class SoundVariant {
public:
    explicit SoundVariant(const std::string& assetPath, float weight = 1.0f)
        : m_assetPath(assetPath), m_weight(weight) {}

    void setWeight(float w)    { m_weight  = w; }
    void setPitch(float p)     { m_pitch   = p; }
    void setVolume(float v)    { m_volume  = v; }
    void setEnabled(bool v)    { m_enabled = v; }

    [[nodiscard]] const std::string& assetPath() const { return m_assetPath; }
    [[nodiscard]] float              weight()    const { return m_weight;    }
    [[nodiscard]] float              pitch()     const { return m_pitch;     }
    [[nodiscard]] float              volume()    const { return m_volume;    }
    [[nodiscard]] bool               isEnabled() const { return m_enabled;   }

private:
    std::string m_assetPath;
    float       m_weight  = 1.0f;
    float       m_pitch   = 1.0f;
    float       m_volume  = 1.0f;
    bool        m_enabled = true;
};

class SoundEffectEditor {
public:
    void setTriggerMode(SfxTriggerMode m)     { m_triggerMode     = m; }
    void setAttenuationType(SfxAttenuationType t){ m_attenuationType = t; }
    void setPlaybackState(SfxPlaybackState s) { m_playbackState    = s; }
    void setLoop(bool v)                      { m_loop             = v; }
    void setVolume(float v)                   { m_volume           = v; }
    void setPitchVariance(float v)            { m_pitchVariance    = v; }
    void setMinDistance(float v)              { m_minDist          = v; }
    void setMaxDistance(float v)              { m_maxDist          = v; }

    void addVariant(const SoundVariant& sv)   { m_variants.push_back(sv); }
    void clearVariants()                      { m_variants.clear(); }

    [[nodiscard]] SfxTriggerMode     triggerMode()      const { return m_triggerMode;     }
    [[nodiscard]] SfxAttenuationType attenuationType()  const { return m_attenuationType; }
    [[nodiscard]] SfxPlaybackState   playbackState()    const { return m_playbackState;   }
    [[nodiscard]] bool               isLoop()           const { return m_loop;            }
    [[nodiscard]] float              volume()           const { return m_volume;          }
    [[nodiscard]] float              pitchVariance()    const { return m_pitchVariance;   }
    [[nodiscard]] float              minDistance()      const { return m_minDist;         }
    [[nodiscard]] float              maxDistance()      const { return m_maxDist;         }
    [[nodiscard]] size_t             variantCount()     const { return m_variants.size(); }

    [[nodiscard]] size_t enabledVariantCount() const {
        size_t c = 0; for (auto& v : m_variants) if (v.isEnabled()) ++c; return c;
    }
    [[nodiscard]] float totalWeight() const {
        float t = 0.0f; for (auto& v : m_variants) if (v.isEnabled()) t += v.weight(); return t;
    }

private:
    std::vector<SoundVariant> m_variants;
    SfxTriggerMode     m_triggerMode     = SfxTriggerMode::OnEvent;
    SfxAttenuationType m_attenuationType = SfxAttenuationType::Logarithmic;
    SfxPlaybackState   m_playbackState   = SfxPlaybackState::Stopped;
    bool               m_loop           = false;
    float              m_volume         = 1.0f;
    float              m_pitchVariance  = 0.0f;
    float              m_minDist        = 1.0f;
    float              m_maxDist        = 50.0f;
};

} // namespace NF
