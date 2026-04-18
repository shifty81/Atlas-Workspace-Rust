#pragma once
// NF::Editor — Post-process editor v1: post-process effect and pass management
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Ppev1EffectType  : uint8_t { Bloom, DOF, SSAO, MotionBlur, ChromaticAberration, Vignette, ColorGrading, ToneMapping };
enum class Ppev1EffectState : uint8_t { Disabled, Enabled, Preview, Override };

inline const char* ppev1EffectTypeName(Ppev1EffectType t) {
    switch (t) {
        case Ppev1EffectType::Bloom:               return "Bloom";
        case Ppev1EffectType::DOF:                 return "DOF";
        case Ppev1EffectType::SSAO:                return "SSAO";
        case Ppev1EffectType::MotionBlur:          return "MotionBlur";
        case Ppev1EffectType::ChromaticAberration: return "ChromaticAberration";
        case Ppev1EffectType::Vignette:            return "Vignette";
        case Ppev1EffectType::ColorGrading:        return "ColorGrading";
        case Ppev1EffectType::ToneMapping:         return "ToneMapping";
    }
    return "Unknown";
}

inline const char* ppev1EffectStateName(Ppev1EffectState s) {
    switch (s) {
        case Ppev1EffectState::Disabled: return "Disabled";
        case Ppev1EffectState::Enabled:  return "Enabled";
        case Ppev1EffectState::Preview:  return "Preview";
        case Ppev1EffectState::Override: return "Override";
    }
    return "Unknown";
}

struct Ppev1Effect {
    uint64_t         id         = 0;
    std::string      name;
    Ppev1EffectType  effectType = Ppev1EffectType::Bloom;
    Ppev1EffectState state      = Ppev1EffectState::Disabled;
    float            intensity  = 1.0f;
    int              order      = 0;

    [[nodiscard]] bool isValid()    const { return id != 0 && !name.empty(); }
    [[nodiscard]] bool isEnabled()  const { return state == Ppev1EffectState::Enabled; }
    [[nodiscard]] bool isPreview()  const { return state == Ppev1EffectState::Preview; }
    [[nodiscard]] bool isOverride() const { return state == Ppev1EffectState::Override; }
};

struct Ppev1Profile {
    uint64_t    id   = 0;
    std::string name;

    [[nodiscard]] bool isValid() const { return id != 0 && !name.empty(); }
};

using Ppev1ChangeCallback = std::function<void(uint64_t)>;

class PostProcessEditorV1 {
public:
    static constexpr size_t MAX_EFFECTS  = 128;
    static constexpr size_t MAX_PROFILES = 32;

    bool addEffect(const Ppev1Effect& effect) {
        if (!effect.isValid()) return false;
        for (const auto& e : m_effects) if (e.id == effect.id) return false;
        if (m_effects.size() >= MAX_EFFECTS) return false;
        m_effects.push_back(effect);
        if (m_onChange) m_onChange(effect.id);
        return true;
    }

    bool removeEffect(uint64_t id) {
        for (auto it = m_effects.begin(); it != m_effects.end(); ++it) {
            if (it->id == id) { m_effects.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] Ppev1Effect* findEffect(uint64_t id) {
        for (auto& e : m_effects) if (e.id == id) return &e;
        return nullptr;
    }

    bool addProfile(const Ppev1Profile& profile) {
        if (!profile.isValid()) return false;
        for (const auto& p : m_profiles) if (p.id == profile.id) return false;
        if (m_profiles.size() >= MAX_PROFILES) return false;
        m_profiles.push_back(profile);
        return true;
    }

    bool removeProfile(uint64_t id) {
        for (auto it = m_profiles.begin(); it != m_profiles.end(); ++it) {
            if (it->id == id) { m_profiles.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] size_t effectCount()  const { return m_effects.size(); }
    [[nodiscard]] size_t profileCount() const { return m_profiles.size(); }

    [[nodiscard]] size_t enabledCount() const {
        size_t c = 0; for (const auto& e : m_effects) if (e.isEnabled()) ++c; return c;
    }
    [[nodiscard]] size_t countByType(Ppev1EffectType type) const {
        size_t c = 0; for (const auto& e : m_effects) if (e.effectType == type) ++c; return c;
    }

    void setOnChange(Ppev1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Ppev1Effect>  m_effects;
    std::vector<Ppev1Profile> m_profiles;
    Ppev1ChangeCallback       m_onChange;
};

} // namespace NF
