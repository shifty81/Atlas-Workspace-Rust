#pragma once
// NF::Editor — Fluid simulation editor
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

enum class FluidSimType : uint8_t {
    SPH, FLIP, APIC, MPM, Eulerian
};

inline const char* fluidSimTypeName(FluidSimType t) {
    switch (t) {
        case FluidSimType::SPH:      return "SPH";
        case FluidSimType::FLIP:     return "FLIP";
        case FluidSimType::APIC:     return "APIC";
        case FluidSimType::MPM:      return "MPM";
        case FluidSimType::Eulerian: return "Eulerian";
    }
    return "Unknown";
}

enum class FluidRenderMode : uint8_t {
    Particles, SurfaceMesh, Volume, Hybrid
};

inline const char* fluidRenderModeName(FluidRenderMode m) {
    switch (m) {
        case FluidRenderMode::Particles:   return "Particles";
        case FluidRenderMode::SurfaceMesh: return "SurfaceMesh";
        case FluidRenderMode::Volume:      return "Volume";
        case FluidRenderMode::Hybrid:      return "Hybrid";
    }
    return "Unknown";
}

enum class FluidBoundaryMode : uint8_t {
    Box, Sphere, Mesh, Infinite
};

inline const char* fluidBoundaryModeName(FluidBoundaryMode m) {
    switch (m) {
        case FluidBoundaryMode::Box:      return "Box";
        case FluidBoundaryMode::Sphere:   return "Sphere";
        case FluidBoundaryMode::Mesh:     return "Mesh";
        case FluidBoundaryMode::Infinite: return "Infinite";
    }
    return "Unknown";
}

class FluidSimConfig {
public:
    explicit FluidSimConfig(const std::string& name, FluidSimType type)
        : m_name(name), m_simType(type) {}

    void setRenderMode(FluidRenderMode m)    { m_renderMode  = m; }
    void setBoundaryMode(FluidBoundaryMode m){ m_boundaryMode = m; }
    void setParticleRadius(float r)          { m_particleRadius = r; }
    void setViscosity(float v)               { m_viscosity   = v; }
    void setSurfaceTension(float v)          { m_surfTension = v; }
    void setMaxParticles(uint32_t n)         { m_maxParticles = n; }
    void setEnabled(bool v)                  { m_enabled     = v; }
    void setGpuAccelerated(bool v)           { m_gpuAccel    = v; }

    [[nodiscard]] const std::string& name()           const { return m_name;          }
    [[nodiscard]] FluidSimType       simType()         const { return m_simType;       }
    [[nodiscard]] FluidRenderMode    renderMode()      const { return m_renderMode;    }
    [[nodiscard]] FluidBoundaryMode  boundaryMode()    const { return m_boundaryMode;  }
    [[nodiscard]] float              particleRadius()  const { return m_particleRadius;}
    [[nodiscard]] float              viscosity()       const { return m_viscosity;     }
    [[nodiscard]] float              surfaceTension()  const { return m_surfTension;   }
    [[nodiscard]] uint32_t           maxParticles()    const { return m_maxParticles;  }
    [[nodiscard]] bool               isEnabled()       const { return m_enabled;       }
    [[nodiscard]] bool               isGpuAccelerated()const { return m_gpuAccel;      }

private:
    std::string         m_name;
    FluidSimType        m_simType       = FluidSimType::FLIP;
    FluidRenderMode     m_renderMode    = FluidRenderMode::SurfaceMesh;
    FluidBoundaryMode   m_boundaryMode  = FluidBoundaryMode::Box;
    float               m_particleRadius= 0.05f;
    float               m_viscosity     = 0.01f;
    float               m_surfTension   = 0.0f;
    uint32_t            m_maxParticles  = 100000;
    bool                m_enabled       = true;
    bool                m_gpuAccel      = false;
};

class FluidSimEditor {
public:
    static constexpr size_t MAX_SIMS = 32;

    [[nodiscard]] bool addSim(const FluidSimConfig& sim) {
        for (auto& s : m_sims) if (s.name() == sim.name()) return false;
        if (m_sims.size() >= MAX_SIMS) return false;
        m_sims.push_back(sim);
        return true;
    }

    [[nodiscard]] bool removeSim(const std::string& name) {
        for (auto it = m_sims.begin(); it != m_sims.end(); ++it) {
            if (it->name() == name) { m_sims.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] FluidSimConfig* findSim(const std::string& name) {
        for (auto& s : m_sims) if (s.name() == name) return &s;
        return nullptr;
    }

    [[nodiscard]] size_t simCount()        const { return m_sims.size(); }
    [[nodiscard]] size_t enabledCount()    const {
        size_t c = 0; for (auto& s : m_sims) if (s.isEnabled()) ++c; return c;
    }
    [[nodiscard]] size_t gpuAccelCount()   const {
        size_t c = 0; for (auto& s : m_sims) if (s.isGpuAccelerated()) ++c; return c;
    }
    [[nodiscard]] size_t countByType(FluidSimType t) const {
        size_t c = 0; for (auto& s : m_sims) if (s.simType() == t) ++c; return c;
    }
    [[nodiscard]] size_t countByRenderMode(FluidRenderMode m) const {
        size_t c = 0; for (auto& s : m_sims) if (s.renderMode() == m) ++c; return c;
    }

private:
    std::vector<FluidSimConfig> m_sims;
};

} // namespace NF
