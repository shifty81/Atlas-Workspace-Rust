#pragma once
// NF::Editor — Spline editor
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

enum class SplineType : uint8_t {
    Linear, CatmullRom, Bezier, Hermite, NURBS
};

inline const char* splineTypeName(SplineType t) {
    switch (t) {
        case SplineType::Linear:    return "Linear";
        case SplineType::CatmullRom:return "CatmullRom";
        case SplineType::Bezier:    return "Bezier";
        case SplineType::Hermite:   return "Hermite";
        case SplineType::NURBS:     return "NURBS";
    }
    return "Unknown";
}

enum class SplineTangentMode : uint8_t {
    Auto, Clamped, Linear, Constant, Broken
};

inline const char* splineTangentModeName(SplineTangentMode m) {
    switch (m) {
        case SplineTangentMode::Auto:     return "Auto";
        case SplineTangentMode::Clamped:  return "Clamped";
        case SplineTangentMode::Linear:   return "Linear";
        case SplineTangentMode::Constant: return "Constant";
        case SplineTangentMode::Broken:   return "Broken";
    }
    return "Unknown";
}

enum class SplineLoopMode : uint8_t {
    None, Loop, PingPong
};

inline const char* splineLoopModeName(SplineLoopMode m) {
    switch (m) {
        case SplineLoopMode::None:     return "None";
        case SplineLoopMode::Loop:     return "Loop";
        case SplineLoopMode::PingPong: return "PingPong";
    }
    return "Unknown";
}

class SplineControlPoint {
public:
    explicit SplineControlPoint(uint32_t id) : m_id(id) {}

    void setTangentMode(SplineTangentMode m) { m_tangentMode = m; }
    void setWeight(float w)                  { m_weight = w; }
    void setSelected(bool v)                 { m_selected = v; }

    [[nodiscard]] uint32_t          id()          const { return m_id;          }
    [[nodiscard]] SplineTangentMode tangentMode() const { return m_tangentMode; }
    [[nodiscard]] float             weight()      const { return m_weight;      }
    [[nodiscard]] bool              isSelected()  const { return m_selected;    }

private:
    uint32_t          m_id;
    SplineTangentMode m_tangentMode = SplineTangentMode::Auto;
    float             m_weight      = 1.0f;
    bool              m_selected    = false;
};

class SplineEditor {
public:
    explicit SplineEditor(const std::string& name, SplineType type = SplineType::CatmullRom)
        : m_name(name), m_type(type) {}

    void setType(SplineType t)       { m_type = t; }
    void setLoopMode(SplineLoopMode m){ m_loopMode = m; }
    void setClosed(bool v)           { m_closed = v; }
    void setResolution(uint32_t r)   { m_resolution = r; }
    void setDirty(bool v)            { m_dirty = v; }

    [[nodiscard]] bool addPoint(const SplineControlPoint& pt) {
        for (auto& p : m_points) if (p.id() == pt.id()) return false;
        m_points.push_back(pt);
        m_dirty = true;
        return true;
    }

    [[nodiscard]] bool removePoint(uint32_t id) {
        for (auto it = m_points.begin(); it != m_points.end(); ++it) {
            if (it->id() == id) { m_points.erase(it); m_dirty = true; return true; }
        }
        return false;
    }

    [[nodiscard]] SplineControlPoint* findPoint(uint32_t id) {
        for (auto& p : m_points) if (p.id() == id) return &p;
        return nullptr;
    }

    [[nodiscard]] const std::string& name()       const { return m_name;       }
    [[nodiscard]] SplineType         type()       const { return m_type;       }
    [[nodiscard]] SplineLoopMode     loopMode()   const { return m_loopMode;   }
    [[nodiscard]] bool               isClosed()   const { return m_closed;     }
    [[nodiscard]] uint32_t           resolution() const { return m_resolution; }
    [[nodiscard]] bool               isDirty()    const { return m_dirty;      }
    [[nodiscard]] size_t             pointCount() const { return m_points.size(); }

    [[nodiscard]] size_t selectedPointCount() const {
        size_t c = 0; for (auto& p : m_points) if (p.isSelected()) ++c; return c;
    }

private:
    std::string                      m_name;
    SplineType                       m_type       = SplineType::CatmullRom;
    SplineLoopMode                   m_loopMode   = SplineLoopMode::None;
    std::vector<SplineControlPoint>  m_points;
    uint32_t                         m_resolution = 32;
    bool                             m_closed     = false;
    bool                             m_dirty      = false;
};

} // namespace NF
