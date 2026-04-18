#pragma once
// NF::Editor — Rope simulation editor
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

enum class RopeSimSolver : uint8_t {
    Verlet, PBD, XPBD, PositionBased, RigidBody
};

inline const char* ropeSimSolverName(RopeSimSolver s) {
    switch (s) {
        case RopeSimSolver::Verlet:        return "Verlet";
        case RopeSimSolver::PBD:           return "PBD";
        case RopeSimSolver::XPBD:          return "XPBD";
        case RopeSimSolver::PositionBased: return "PositionBased";
        case RopeSimSolver::RigidBody:     return "RigidBody";
    }
    return "Unknown";
}

enum class RopeAttachMode : uint8_t {
    Both, StartOnly, EndOnly, None
};

inline const char* ropeAttachModeName(RopeAttachMode m) {
    switch (m) {
        case RopeAttachMode::Both:      return "Both";
        case RopeAttachMode::StartOnly: return "StartOnly";
        case RopeAttachMode::EndOnly:   return "EndOnly";
        case RopeAttachMode::None:      return "None";
    }
    return "Unknown";
}

enum class RopeCollisionMode : uint8_t {
    None, Sphere, Capsule, Full
};

inline const char* ropeCollisionModeName(RopeCollisionMode m) {
    switch (m) {
        case RopeCollisionMode::None:    return "None";
        case RopeCollisionMode::Sphere:  return "Sphere";
        case RopeCollisionMode::Capsule: return "Capsule";
        case RopeCollisionMode::Full:    return "Full";
    }
    return "Unknown";
}

class RopeSimConfig {
public:
    explicit RopeSimConfig(const std::string& name) : m_name(name) {}

    void setSolver(RopeSimSolver s)         { m_solver    = s; }
    void setAttachMode(RopeAttachMode m)    { m_attachMode= m; }
    void setCollisionMode(RopeCollisionMode m){ m_collision= m; }
    void setSegments(uint32_t n)            { m_segments  = n; }
    void setStiffness(float v)              { m_stiffness = v; }
    void setDamping(float v)               { m_damping   = v; }
    void setGravityScale(float v)          { m_gravScale = v; }
    void setEnabled(bool v)                { m_enabled   = v; }

    [[nodiscard]] const std::string& name()          const { return m_name;      }
    [[nodiscard]] RopeSimSolver      solver()        const { return m_solver;    }
    [[nodiscard]] RopeAttachMode     attachMode()    const { return m_attachMode;}
    [[nodiscard]] RopeCollisionMode  collisionMode() const { return m_collision; }
    [[nodiscard]] uint32_t           segments()      const { return m_segments;  }
    [[nodiscard]] float              stiffness()     const { return m_stiffness; }
    [[nodiscard]] float              damping()       const { return m_damping;   }
    [[nodiscard]] float              gravityScale()  const { return m_gravScale; }
    [[nodiscard]] bool               isEnabled()     const { return m_enabled;   }

private:
    std::string       m_name;
    RopeSimSolver     m_solver     = RopeSimSolver::XPBD;
    RopeAttachMode    m_attachMode = RopeAttachMode::Both;
    RopeCollisionMode m_collision  = RopeCollisionMode::Capsule;
    uint32_t          m_segments   = 16;
    float             m_stiffness  = 1.0f;
    float             m_damping    = 0.1f;
    float             m_gravScale  = 1.0f;
    bool              m_enabled    = true;
};

class RopeSimEditor {
public:
    static constexpr size_t MAX_ROPES = 512;

    [[nodiscard]] bool addRope(const RopeSimConfig& rope) {
        for (auto& r : m_ropes) if (r.name() == rope.name()) return false;
        if (m_ropes.size() >= MAX_ROPES) return false;
        m_ropes.push_back(rope);
        return true;
    }

    [[nodiscard]] bool removeRope(const std::string& name) {
        for (auto it = m_ropes.begin(); it != m_ropes.end(); ++it) {
            if (it->name() == name) { m_ropes.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] RopeSimConfig* findRope(const std::string& name) {
        for (auto& r : m_ropes) if (r.name() == name) return &r;
        return nullptr;
    }

    [[nodiscard]] size_t ropeCount()   const { return m_ropes.size(); }
    [[nodiscard]] size_t enabledCount() const {
        size_t c = 0; for (auto& r : m_ropes) if (r.isEnabled()) ++c; return c;
    }
    [[nodiscard]] size_t countBySolver(RopeSimSolver s) const {
        size_t c = 0; for (auto& r : m_ropes) if (r.solver() == s) ++c; return c;
    }
    [[nodiscard]] size_t countByAttach(RopeAttachMode m) const {
        size_t c = 0; for (auto& r : m_ropes) if (r.attachMode() == m) ++c; return c;
    }

private:
    std::vector<RopeSimConfig> m_ropes;
};

} // namespace NF
