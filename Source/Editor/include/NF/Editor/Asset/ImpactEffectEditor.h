#pragma once
// NF::Editor — impact effect editor
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

enum class ImpactSurface : uint8_t {
    Concrete, Metal, Wood, Dirt, Water, Glass, Flesh, Sand, Stone, Fabric
};

inline const char* impactSurfaceName(ImpactSurface s) {
    switch (s) {
        case ImpactSurface::Concrete: return "Concrete";
        case ImpactSurface::Metal:    return "Metal";
        case ImpactSurface::Wood:     return "Wood";
        case ImpactSurface::Dirt:     return "Dirt";
        case ImpactSurface::Water:    return "Water";
        case ImpactSurface::Glass:    return "Glass";
        case ImpactSurface::Flesh:    return "Flesh";
        case ImpactSurface::Sand:     return "Sand";
        case ImpactSurface::Stone:    return "Stone";
        case ImpactSurface::Fabric:   return "Fabric";
    }
    return "Unknown";
}

enum class ImpactEffectLayer : uint8_t {
    Decal, Particles, Audio, Light, Physics, Screen
};

inline const char* impactEffectLayerName(ImpactEffectLayer l) {
    switch (l) {
        case ImpactEffectLayer::Decal:    return "Decal";
        case ImpactEffectLayer::Particles:return "Particles";
        case ImpactEffectLayer::Audio:    return "Audio";
        case ImpactEffectLayer::Light:    return "Light";
        case ImpactEffectLayer::Physics:  return "Physics";
        case ImpactEffectLayer::Screen:   return "Screen";
    }
    return "Unknown";
}

enum class ImpactResponse : uint8_t {
    None, Ricochet, Penetrate, Shatter, Splash, Burn, Crumble, Bounce
};

inline const char* impactResponseName(ImpactResponse r) {
    switch (r) {
        case ImpactResponse::None:      return "None";
        case ImpactResponse::Ricochet:  return "Ricochet";
        case ImpactResponse::Penetrate: return "Penetrate";
        case ImpactResponse::Shatter:   return "Shatter";
        case ImpactResponse::Splash:    return "Splash";
        case ImpactResponse::Burn:      return "Burn";
        case ImpactResponse::Crumble:   return "Crumble";
        case ImpactResponse::Bounce:    return "Bounce";
    }
    return "Unknown";
}

class ImpactEffect {
public:
    explicit ImpactEffect(uint32_t id, const std::string& name, ImpactSurface surface)
        : m_id(id), m_name(name), m_surface(surface) {}

    void setResponse(ImpactResponse v)    { m_response = v; }
    void setDecalEnabled(bool v)          { m_isDecalEnabled = v; }
    void setParticlesEnabled(bool v)      { m_isParticlesEnabled = v; }
    void setAudioEnabled(bool v)          { m_isAudioEnabled = v; }
    void setIntensity(float v)            { m_intensity = v; }

    [[nodiscard]] uint32_t           id()                 const { return m_id; }
    [[nodiscard]] const std::string& name()               const { return m_name; }
    [[nodiscard]] ImpactSurface      surface()            const { return m_surface; }
    [[nodiscard]] ImpactResponse     response()           const { return m_response; }
    [[nodiscard]] bool               isDecalEnabled()     const { return m_isDecalEnabled; }
    [[nodiscard]] bool               isParticlesEnabled() const { return m_isParticlesEnabled; }
    [[nodiscard]] bool               isAudioEnabled()     const { return m_isAudioEnabled; }
    [[nodiscard]] float              intensity()          const { return m_intensity; }

private:
    uint32_t       m_id;
    std::string    m_name;
    ImpactSurface  m_surface            = ImpactSurface::Concrete;
    ImpactResponse m_response           = ImpactResponse::None;
    bool           m_isDecalEnabled     = true;
    bool           m_isParticlesEnabled = true;
    bool           m_isAudioEnabled     = true;
    float          m_intensity          = 1.0f;
};

class ImpactEffectEditor {
public:
    bool addEffect(const ImpactEffect& effect) {
        for (const auto& e : m_effects)
            if (e.id() == effect.id()) return false;
        m_effects.push_back(effect);
        return true;
    }

    bool removeEffect(uint32_t id) {
        for (auto it = m_effects.begin(); it != m_effects.end(); ++it) {
            if (it->id() == id) { m_effects.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] ImpactEffect* findEffect(uint32_t id) {
        for (auto& e : m_effects)
            if (e.id() == id) return &e;
        return nullptr;
    }

    [[nodiscard]] size_t effectCount() const { return m_effects.size(); }

    [[nodiscard]] size_t countBySurface(ImpactSurface s) const {
        size_t n = 0;
        for (const auto& e : m_effects) if (e.surface() == s) ++n;
        return n;
    }

    [[nodiscard]] size_t countByResponse(ImpactResponse r) const {
        size_t n = 0;
        for (const auto& e : m_effects) if (e.response() == r) ++n;
        return n;
    }

    void setPreviewEnabled(bool v)      { m_isPreviewEnabled = v; }
    void setShowDecalBounds(bool v)     { m_isShowDecalBounds = v; }
    void setImpactForce(float v)        { m_impactForce = v; }

    [[nodiscard]] bool  isPreviewEnabled()  const { return m_isPreviewEnabled; }
    [[nodiscard]] bool  isShowDecalBounds() const { return m_isShowDecalBounds; }
    [[nodiscard]] float impactForce()       const { return m_impactForce; }

private:
    std::vector<ImpactEffect> m_effects;
    bool  m_isPreviewEnabled  = false;
    bool  m_isShowDecalBounds = false;
    float m_impactForce       = 1.0f;
};

} // namespace NF
