#pragma once
// NF::Editor — Particle system editor
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

enum class PsEmitterShape : uint8_t {
    Point, Sphere, Box, Cone, Cylinder, Edge
};

inline const char* psEmitterShapeName(PsEmitterShape s) {
    switch (s) {
        case PsEmitterShape::Point:    return "Point";
        case PsEmitterShape::Sphere:   return "Sphere";
        case PsEmitterShape::Box:      return "Box";
        case PsEmitterShape::Cone:     return "Cone";
        case PsEmitterShape::Cylinder: return "Cylinder";
        case PsEmitterShape::Edge:     return "Edge";
    }
    return "Unknown";
}

enum class ParticleRenderMode : uint8_t {
    Billboard, Mesh, Trail, Ribbon, Beam
};

inline const char* particleRenderModeName(ParticleRenderMode m) {
    switch (m) {
        case ParticleRenderMode::Billboard: return "Billboard";
        case ParticleRenderMode::Mesh:      return "Mesh";
        case ParticleRenderMode::Trail:     return "Trail";
        case ParticleRenderMode::Ribbon:    return "Ribbon";
        case ParticleRenderMode::Beam:      return "Beam";
    }
    return "Unknown";
}

enum class ParticleSimSpace : uint8_t {
    Local, World
};

inline const char* particleSimSpaceName(ParticleSimSpace s) {
    switch (s) {
        case ParticleSimSpace::Local: return "Local";
        case ParticleSimSpace::World: return "World";
    }
    return "Unknown";
}

class ParticleSystemConfig {
public:
    explicit ParticleSystemConfig(const std::string& name) : m_name(name) {}

    void setEmitterShape(PsEmitterShape s) { m_emitterShape = s; }
    void setRenderMode(ParticleRenderMode m)     { m_renderMode  = m; }
    void setSimSpace(ParticleSimSpace s)         { m_simSpace    = s; }
    void setMaxParticles(uint32_t n)             { m_maxParticles= n; }
    void setEmitRate(float r)                    { m_emitRate    = r; }
    void setLifetime(float l)                    { m_lifetime    = l; }
    void setStartSpeed(float s)                  { m_startSpeed  = s; }
    void setGravityModifier(float g)             { m_gravity     = g; }
    void setLooping(bool v)                      { m_looping     = v; }
    void setEnabled(bool v)                      { m_enabled     = v; }

    [[nodiscard]] const std::string&   name()            const { return m_name;         }
    [[nodiscard]] PsEmitterShape emitterShape()    const { return m_emitterShape; }
    [[nodiscard]] ParticleRenderMode   renderMode()      const { return m_renderMode;   }
    [[nodiscard]] ParticleSimSpace     simSpace()        const { return m_simSpace;     }
    [[nodiscard]] uint32_t             maxParticles()    const { return m_maxParticles; }
    [[nodiscard]] float                emitRate()        const { return m_emitRate;     }
    [[nodiscard]] float                lifetime()        const { return m_lifetime;     }
    [[nodiscard]] float                startSpeed()      const { return m_startSpeed;   }
    [[nodiscard]] float                gravityModifier() const { return m_gravity;      }
    [[nodiscard]] bool                 isLooping()       const { return m_looping;      }
    [[nodiscard]] bool                 isEnabled()       const { return m_enabled;      }

private:
    std::string           m_name;
    PsEmitterShape  m_emitterShape = PsEmitterShape::Cone;
    ParticleRenderMode    m_renderMode   = ParticleRenderMode::Billboard;
    ParticleSimSpace      m_simSpace     = ParticleSimSpace::World;
    uint32_t              m_maxParticles = 1000;
    float                 m_emitRate     = 10.0f;
    float                 m_lifetime     = 5.0f;
    float                 m_startSpeed   = 1.0f;
    float                 m_gravity      = 1.0f;
    bool                  m_looping      = true;
    bool                  m_enabled      = true;
};

class ParticleSystemEditor {
public:
    static constexpr size_t MAX_SYSTEMS = 256;

    [[nodiscard]] bool addSystem(const ParticleSystemConfig& sys) {
        for (auto& s : m_systems) if (s.name() == sys.name()) return false;
        if (m_systems.size() >= MAX_SYSTEMS) return false;
        m_systems.push_back(sys);
        return true;
    }

    [[nodiscard]] bool removeSystem(const std::string& name) {
        for (auto it = m_systems.begin(); it != m_systems.end(); ++it) {
            if (it->name() == name) { m_systems.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] ParticleSystemConfig* findSystem(const std::string& name) {
        for (auto& s : m_systems) if (s.name() == name) return &s;
        return nullptr;
    }

    [[nodiscard]] size_t systemCount()    const { return m_systems.size(); }
    [[nodiscard]] size_t enabledCount()   const {
        size_t c = 0; for (auto& s : m_systems) if (s.isEnabled()) ++c; return c;
    }
    [[nodiscard]] size_t loopingCount()   const {
        size_t c = 0; for (auto& s : m_systems) if (s.isLooping()) ++c; return c;
    }
    [[nodiscard]] size_t countByShape(PsEmitterShape sh) const {
        size_t c = 0; for (auto& s : m_systems) if (s.emitterShape() == sh) ++c; return c;
    }
    [[nodiscard]] size_t countByRenderMode(ParticleRenderMode m) const {
        size_t c = 0; for (auto& s : m_systems) if (s.renderMode() == m) ++c; return c;
    }

private:
    std::vector<ParticleSystemConfig> m_systems;
};

} // namespace NF
