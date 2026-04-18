#pragma once
// NF::Editor — Water simulation editor
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

enum class WaterSimMethod : uint8_t {
    FFT, Gerstner, SPH, HeightField, Hybrid
};

inline const char* waterSimMethodName(WaterSimMethod m) {
    switch (m) {
        case WaterSimMethod::FFT:         return "FFT";
        case WaterSimMethod::Gerstner:    return "Gerstner";
        case WaterSimMethod::SPH:         return "SPH";
        case WaterSimMethod::HeightField: return "HeightField";
        case WaterSimMethod::Hybrid:      return "Hybrid";
    }
    return "Unknown";
}

enum class WaterSimBodyType : uint8_t {
    Ocean, Lake, River, Pool, Waterfall
};

inline const char* waterSimBodyTypeName(WaterSimBodyType t) {
    switch (t) {
        case WaterSimBodyType::Ocean:     return "Ocean";
        case WaterSimBodyType::Lake:      return "Lake";
        case WaterSimBodyType::River:     return "River";
        case WaterSimBodyType::Pool:      return "Pool";
        case WaterSimBodyType::Waterfall: return "Waterfall";
    }
    return "Unknown";
}

enum class WaterFoamMode : uint8_t {
    None, Simple, Detailed, Volumetric
};

inline const char* waterFoamModeName(WaterFoamMode m) {
    switch (m) {
        case WaterFoamMode::None:       return "None";
        case WaterFoamMode::Simple:     return "Simple";
        case WaterFoamMode::Detailed:   return "Detailed";
        case WaterFoamMode::Volumetric: return "Volumetric";
    }
    return "Unknown";
}

class WaterBodyConfig {
public:
    explicit WaterBodyConfig(const std::string& name, WaterSimBodyType type)
        : m_name(name), m_bodyType(type) {}

    void setMethod(WaterSimMethod m)   { m_method    = m; }
    void setFoamMode(WaterFoamMode f)  { m_foamMode  = f; }
    void setAmplitude(float v)         { m_amplitude = v; }
    void setWindSpeed(float v)         { m_windSpeed = v; }
    void setTurbulence(float v)        { m_turbulence= v; }
    void setTessellation(uint32_t t)   { m_tessellation = t; }
    void setEnabled(bool v)            { m_enabled   = v; }
    void setInteractive(bool v)        { m_interactive = v; }

    [[nodiscard]] const std::string& name()         const { return m_name;        }
    [[nodiscard]] WaterSimBodyType      bodyType()      const { return m_bodyType;    }
    [[nodiscard]] WaterSimMethod     method()        const { return m_method;      }
    [[nodiscard]] WaterFoamMode      foamMode()      const { return m_foamMode;    }
    [[nodiscard]] float              amplitude()     const { return m_amplitude;   }
    [[nodiscard]] float              windSpeed()     const { return m_windSpeed;   }
    [[nodiscard]] float              turbulence()    const { return m_turbulence;  }
    [[nodiscard]] uint32_t           tessellation()  const { return m_tessellation;}
    [[nodiscard]] bool               isEnabled()     const { return m_enabled;     }
    [[nodiscard]] bool               isInteractive() const { return m_interactive; }

private:
    std::string      m_name;
    WaterSimBodyType    m_bodyType     = WaterSimBodyType::Ocean;
    WaterSimMethod   m_method       = WaterSimMethod::Gerstner;
    WaterFoamMode    m_foamMode     = WaterFoamMode::Simple;
    float            m_amplitude    = 1.0f;
    float            m_windSpeed    = 5.0f;
    float            m_turbulence   = 0.2f;
    uint32_t         m_tessellation = 64;
    bool             m_enabled      = true;
    bool             m_interactive  = false;
};

class WaterSimEditor {
public:
    static constexpr size_t MAX_BODIES = 64;

    [[nodiscard]] bool addBody(const WaterBodyConfig& body) {
        for (auto& b : m_bodies) if (b.name() == body.name()) return false;
        if (m_bodies.size() >= MAX_BODIES) return false;
        m_bodies.push_back(body);
        return true;
    }

    [[nodiscard]] bool removeBody(const std::string& name) {
        for (auto it = m_bodies.begin(); it != m_bodies.end(); ++it) {
            if (it->name() == name) { m_bodies.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] WaterBodyConfig* findBody(const std::string& name) {
        for (auto& b : m_bodies) if (b.name() == name) return &b;
        return nullptr;
    }

    [[nodiscard]] size_t bodyCount()       const { return m_bodies.size(); }
    [[nodiscard]] size_t enabledCount()    const {
        size_t c = 0; for (auto& b : m_bodies) if (b.isEnabled()) ++c; return c;
    }
    [[nodiscard]] size_t interactiveCount()const {
        size_t c = 0; for (auto& b : m_bodies) if (b.isInteractive()) ++c; return c;
    }
    [[nodiscard]] size_t countByType(WaterSimBodyType t) const {
        size_t c = 0; for (auto& b : m_bodies) if (b.bodyType() == t) ++c; return c;
    }
    [[nodiscard]] size_t countByMethod(WaterSimMethod m) const {
        size_t c = 0; for (auto& b : m_bodies) if (b.method() == m) ++c; return c;
    }

private:
    std::vector<WaterBodyConfig> m_bodies;
};

} // namespace NF
