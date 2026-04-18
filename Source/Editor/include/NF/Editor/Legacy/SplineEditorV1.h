#pragma once
// NF::Editor — Spline editor v1: control-point spline authoring with tangent modes
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Sev1SplineType  : uint8_t { Linear, CatmullRom, Bezier, Hermite };
enum class Sev1TangentMode : uint8_t { Auto, Broken, Aligned, Flat };
enum class Sev1LoopMode    : uint8_t { None, Loop, PingPong };

struct Sev1ControlPoint {
    uint64_t       id       = 0;
    float          x        = 0.f;
    float          y        = 0.f;
    float          z        = 0.f;
    Sev1TangentMode tangent = Sev1TangentMode::Auto;
    [[nodiscard]] bool isValid() const { return id != 0; }
};

using Sev1SplineCallback = std::function<void()>;

class SplineEditorV1 {
public:
    bool addPoint(const Sev1ControlPoint& p) {
        if (!p.isValid()) return false;
        for (const auto& ep : m_points) if (ep.id == p.id) return false;
        m_points.push_back(p);
        if (m_onChange) m_onChange();
        return true;
    }

    bool removePoint(uint64_t id) {
        for (auto it = m_points.begin(); it != m_points.end(); ++it) {
            if (it->id == id) { m_points.erase(it); if (m_onChange) m_onChange(); return true; }
        }
        return false;
    }

    bool movePoint(uint64_t id, float x, float y, float z) {
        for (auto& p : m_points) {
            if (p.id == id) { p.x = x; p.y = y; p.z = z; if (m_onChange) m_onChange(); return true; }
        }
        return false;
    }

    void setSplineType(Sev1SplineType t) { m_type = t; }
    void setLoopMode(Sev1LoopMode m)     { m_loop = m; }

    [[nodiscard]] Sev1SplineType splineType()  const { return m_type;        }
    [[nodiscard]] Sev1LoopMode   loopMode()    const { return m_loop;        }
    [[nodiscard]] size_t         pointCount()  const { return m_points.size(); }

    void setOnChange(Sev1SplineCallback cb) { m_onChange = std::move(cb); }

    float length() const {
        if (m_points.size() < 2) return 0.f;
        float total = 0.f;
        for (size_t i = 1; i < m_points.size(); ++i) {
            float dx = m_points[i].x - m_points[i-1].x;
            float dy = m_points[i].y - m_points[i-1].y;
            float dz = m_points[i].z - m_points[i-1].z;
            total += std::sqrt(dx*dx + dy*dy + dz*dz);
        }
        return total;
    }

private:
    std::vector<Sev1ControlPoint> m_points;
    Sev1SplineType               m_type = Sev1SplineType::CatmullRom;
    Sev1LoopMode                 m_loop = Sev1LoopMode::None;
    Sev1SplineCallback           m_onChange;
};

} // namespace NF
