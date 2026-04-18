#pragma once
// NF::Editor — Curve + curve editor panel
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

enum class CurveType : uint8_t {
    Linear    = 0,
    Bezier    = 1,
    Hermite   = 2,
    CatmullRom= 3,
    Step      = 4,
    Sine      = 5,
    Cosine    = 6,
    Custom    = 7
};

inline const char* curveTypeName(CurveType t) {
    switch (t) {
        case CurveType::Linear:     return "Linear";
        case CurveType::Bezier:     return "Bezier";
        case CurveType::Hermite:    return "Hermite";
        case CurveType::CatmullRom: return "CatmullRom";
        case CurveType::Step:       return "Step";
        case CurveType::Sine:       return "Sine";
        case CurveType::Cosine:     return "Cosine";
        case CurveType::Custom:     return "Custom";
        default:                    return "Unknown";
    }
}

enum class CurveHandleMode : uint8_t {
    Free     = 0,
    Aligned  = 1,
    Vector   = 2,
    Auto     = 3
};

inline const char* curveHandleModeName(CurveHandleMode m) {
    switch (m) {
        case CurveHandleMode::Free:    return "Free";
        case CurveHandleMode::Aligned: return "Aligned";
        case CurveHandleMode::Vector:  return "Vector";
        case CurveHandleMode::Auto:    return "Auto";
        default:                       return "Unknown";
    }
}

struct CurveControlPoint {
    float           time     = 0.f;
    float           value    = 0.f;
    float           handleL  = 0.f;
    float           handleR  = 0.f;
    CurveHandleMode mode     = CurveHandleMode::Auto;
    bool            selected = false;

    void select()   { selected = true;  }
    void deselect() { selected = false; }
    void setTime(float t)  { time  = t; }
    void setValue(float v) { value = v; }
};

class Curve {
public:
    explicit Curve(const std::string& name, CurveType type = CurveType::Linear)
        : m_name(name), m_type(type) {}

    bool addPoint(CurveControlPoint cp) {
        for (auto& existing : m_points) if (existing.time == cp.time) return false;
        m_points.push_back(cp);
        return true;
    }

    bool removePoint(float time) {
        for (auto it = m_points.begin(); it != m_points.end(); ++it) {
            if (it->time == time) { m_points.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] CurveControlPoint* findPoint(float time) {
        for (auto& cp : m_points) if (cp.time == time) return &cp;
        return nullptr;
    }

    [[nodiscard]] size_t pointCount()    const { return m_points.size(); }

    [[nodiscard]] size_t selectedCount() const {
        size_t c = 0;
        for (auto& cp : m_points) if (cp.selected) c++;
        return c;
    }

    void selectAll()   { for (auto& cp : m_points) cp.select();   }
    void deselectAll() { for (auto& cp : m_points) cp.deselect(); }

    [[nodiscard]] const std::string& name() const { return m_name; }
    [[nodiscard]] CurveType          type() const { return m_type; }

    [[nodiscard]] float duration() const {
        float d = 0.f;
        for (auto& cp : m_points) if (cp.time > d) d = cp.time;
        return d;
    }

private:
    std::string                   m_name;
    CurveType                     m_type;
    std::vector<CurveControlPoint> m_points;
};

class CurveEditorPanel {
public:
    static constexpr size_t MAX_CURVES = 32;

    bool addCurve(Curve curve) {
        if (m_curves.size() >= MAX_CURVES) return false;
        for (auto& existing : m_curves) if (existing.name() == curve.name()) return false;
        m_curves.push_back(std::move(curve));
        return true;
    }

    bool removeCurve(const std::string& name) {
        for (auto it = m_curves.begin(); it != m_curves.end(); ++it) {
            if (it->name() == name) {
                if (m_activeCurve == name) m_activeCurve.clear();
                m_curves.erase(it);
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] Curve* findCurve(const std::string& name) {
        for (auto& c : m_curves) if (c.name() == name) return &c;
        return nullptr;
    }

    bool setActiveCurve(const std::string& name) {
        for (auto& c : m_curves) {
            if (c.name() == name) { m_activeCurve = name; return true; }
        }
        return false;
    }

    [[nodiscard]] const std::string& activeCurve() const { return m_activeCurve; }
    [[nodiscard]] size_t             curveCount()  const { return m_curves.size(); }
    [[nodiscard]] bool               isLooping()   const { return m_looping; }
    void                             setLooping(bool v) { m_looping = v; }

    void selectAllPoints()   { for (auto& c : m_curves) c.selectAll();   }
    void deselectAllPoints() { for (auto& c : m_curves) c.deselectAll(); }

private:
    std::vector<Curve> m_curves;
    std::string        m_activeCurve;
    bool               m_looping = false;
};

// ── S30 — Gradient Editor ────────────────────────────────────────


} // namespace NF
