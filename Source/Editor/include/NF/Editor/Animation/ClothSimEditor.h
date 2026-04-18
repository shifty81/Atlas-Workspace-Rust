#pragma once
// NF::Editor — Cloth simulation editor
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

enum class ClothSolverType : uint8_t {
    PBD, XPBD, Verlet, FEM, CoRotational
};

inline const char* clothSolverTypeName(ClothSolverType t) {
    switch (t) {
        case ClothSolverType::PBD:          return "PBD";
        case ClothSolverType::XPBD:         return "XPBD";
        case ClothSolverType::Verlet:       return "Verlet";
        case ClothSolverType::FEM:          return "FEM";
        case ClothSolverType::CoRotational: return "CoRotational";
    }
    return "Unknown";
}

enum class ClothCollisionResponse : uint8_t {
    None, PointCloud, Mesh, SDF
};

inline const char* clothCollisionResponseName(ClothCollisionResponse r) {
    switch (r) {
        case ClothCollisionResponse::None:       return "None";
        case ClothCollisionResponse::PointCloud: return "PointCloud";
        case ClothCollisionResponse::Mesh:       return "Mesh";
        case ClothCollisionResponse::SDF:        return "SDF";
    }
    return "Unknown";
}

enum class ClothPaintMode : uint8_t {
    MaxDistance, Stiffness, Damping, BackstopRadius, None
};

inline const char* clothPaintModeName(ClothPaintMode m) {
    switch (m) {
        case ClothPaintMode::MaxDistance:    return "MaxDistance";
        case ClothPaintMode::Stiffness:      return "Stiffness";
        case ClothPaintMode::Damping:        return "Damping";
        case ClothPaintMode::BackstopRadius: return "BackstopRadius";
        case ClothPaintMode::None:           return "None";
    }
    return "Unknown";
}

class ClothSimConfig {
public:
    explicit ClothSimConfig(const std::string& name) : m_name(name) {}

    void setSolver(ClothSolverType t)               { m_solver    = t; }
    void setCollisionResponse(ClothCollisionResponse r){ m_collision = r; }
    void setPaintMode(ClothPaintMode m)             { m_paintMode = m; }
    void setIterations(uint32_t n)                  { m_iterations = n; }
    void setStiffness(float v)                      { m_stiffness = v; }
    void setDamping(float v)                        { m_damping   = v; }
    void setGravityScale(float v)                   { m_gravScale = v; }
    void setSelfCollision(bool v)                   { m_selfCollision = v; }
    void setEnabled(bool v)                         { m_enabled   = v; }

    [[nodiscard]] const std::string&     name()             const { return m_name;         }
    [[nodiscard]] ClothSolverType        solver()           const { return m_solver;        }
    [[nodiscard]] ClothCollisionResponse collisionResponse()const { return m_collision;     }
    [[nodiscard]] ClothPaintMode         paintMode()        const { return m_paintMode;     }
    [[nodiscard]] uint32_t               iterations()       const { return m_iterations;    }
    [[nodiscard]] float                  stiffness()        const { return m_stiffness;     }
    [[nodiscard]] float                  damping()          const { return m_damping;       }
    [[nodiscard]] float                  gravityScale()     const { return m_gravScale;     }
    [[nodiscard]] bool                   hasSelfCollision() const { return m_selfCollision; }
    [[nodiscard]] bool                   isEnabled()        const { return m_enabled;       }

private:
    std::string           m_name;
    ClothSolverType        m_solver       = ClothSolverType::XPBD;
    ClothCollisionResponse m_collision    = ClothCollisionResponse::Mesh;
    ClothPaintMode         m_paintMode    = ClothPaintMode::None;
    uint32_t               m_iterations   = 8;
    float                  m_stiffness    = 0.8f;
    float                  m_damping      = 0.05f;
    float                  m_gravScale    = 1.0f;
    bool                   m_selfCollision= false;
    bool                   m_enabled      = true;
};

class ClothSimEditor {
public:
    static constexpr size_t MAX_CONFIGS = 256;

    [[nodiscard]] bool addConfig(const ClothSimConfig& cfg) {
        for (auto& c : m_configs) if (c.name() == cfg.name()) return false;
        if (m_configs.size() >= MAX_CONFIGS) return false;
        m_configs.push_back(cfg);
        return true;
    }

    [[nodiscard]] bool removeConfig(const std::string& name) {
        for (auto it = m_configs.begin(); it != m_configs.end(); ++it) {
            if (it->name() == name) { m_configs.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] ClothSimConfig* findConfig(const std::string& name) {
        for (auto& c : m_configs) if (c.name() == name) return &c;
        return nullptr;
    }

    [[nodiscard]] size_t configCount()  const { return m_configs.size(); }
    [[nodiscard]] size_t enabledCount() const {
        size_t c = 0; for (auto& cfg : m_configs) if (cfg.isEnabled()) ++c; return c;
    }
    [[nodiscard]] size_t selfCollisionCount() const {
        size_t c = 0; for (auto& cfg : m_configs) if (cfg.hasSelfCollision()) ++c; return c;
    }
    [[nodiscard]] size_t countBySolver(ClothSolverType t) const {
        size_t c = 0; for (auto& cfg : m_configs) if (cfg.solver() == t) ++c; return c;
    }

private:
    std::vector<ClothSimConfig> m_configs;
};

} // namespace NF
