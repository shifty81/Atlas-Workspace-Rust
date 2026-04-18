#pragma once
// NF::Editor — Lighting rig editor v1: light source and rig management
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Lrev1LightType  : uint8_t { Point, Spot, Directional, Area, Emissive, Ambient };
enum class Lrev1LightState : uint8_t { Off, On, Baking, Preview };

inline const char* lrev1LightTypeName(Lrev1LightType t) {
    switch (t) {
        case Lrev1LightType::Point:       return "Point";
        case Lrev1LightType::Spot:        return "Spot";
        case Lrev1LightType::Directional: return "Directional";
        case Lrev1LightType::Area:        return "Area";
        case Lrev1LightType::Emissive:    return "Emissive";
        case Lrev1LightType::Ambient:     return "Ambient";
    }
    return "Unknown";
}

inline const char* lrev1LightStateName(Lrev1LightState s) {
    switch (s) {
        case Lrev1LightState::Off:     return "Off";
        case Lrev1LightState::On:      return "On";
        case Lrev1LightState::Baking:  return "Baking";
        case Lrev1LightState::Preview: return "Preview";
    }
    return "Unknown";
}

struct Lrev1Light {
    uint64_t        id         = 0;
    std::string     name;
    Lrev1LightType  lightType  = Lrev1LightType::Point;
    Lrev1LightState state      = Lrev1LightState::Off;
    float           intensity  = 1.0f;
    float           range      = 10.0f;

    [[nodiscard]] bool isValid()   const { return id != 0 && !name.empty(); }
    [[nodiscard]] bool isOn()      const { return state == Lrev1LightState::On; }
    [[nodiscard]] bool isBaking()  const { return state == Lrev1LightState::Baking; }
    [[nodiscard]] bool isPreview() const { return state == Lrev1LightState::Preview; }
};

struct Lrev1Rig {
    uint64_t    id   = 0;
    std::string name;

    [[nodiscard]] bool isValid() const { return id != 0 && !name.empty(); }
};

using Lrev1ChangeCallback = std::function<void(uint64_t)>;

class LightingRigEditorV1 {
public:
    static constexpr size_t MAX_LIGHTS = 512;
    static constexpr size_t MAX_RIGS   = 64;

    bool addLight(const Lrev1Light& light) {
        if (!light.isValid()) return false;
        for (const auto& l : m_lights) if (l.id == light.id) return false;
        if (m_lights.size() >= MAX_LIGHTS) return false;
        m_lights.push_back(light);
        if (m_onChange) m_onChange(light.id);
        return true;
    }

    bool removeLight(uint64_t id) {
        for (auto it = m_lights.begin(); it != m_lights.end(); ++it) {
            if (it->id == id) { m_lights.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] Lrev1Light* findLight(uint64_t id) {
        for (auto& l : m_lights) if (l.id == id) return &l;
        return nullptr;
    }

    bool addRig(const Lrev1Rig& rig) {
        if (!rig.isValid()) return false;
        for (const auto& r : m_rigs) if (r.id == rig.id) return false;
        if (m_rigs.size() >= MAX_RIGS) return false;
        m_rigs.push_back(rig);
        return true;
    }

    bool removeRig(uint64_t id) {
        for (auto it = m_rigs.begin(); it != m_rigs.end(); ++it) {
            if (it->id == id) { m_rigs.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] size_t lightCount() const { return m_lights.size(); }
    [[nodiscard]] size_t rigCount()   const { return m_rigs.size(); }

    [[nodiscard]] size_t onCount() const {
        size_t c = 0; for (const auto& l : m_lights) if (l.isOn()) ++c; return c;
    }
    [[nodiscard]] size_t countByType(Lrev1LightType type) const {
        size_t c = 0; for (const auto& l : m_lights) if (l.lightType == type) ++c; return c;
    }
    [[nodiscard]] float totalIntensity() const {
        float sum = 0.0f; for (const auto& l : m_lights) sum += l.intensity; return sum;
    }

    void setOnChange(Lrev1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Lrev1Light> m_lights;
    std::vector<Lrev1Rig>   m_rigs;
    Lrev1ChangeCallback     m_onChange;
};

} // namespace NF
