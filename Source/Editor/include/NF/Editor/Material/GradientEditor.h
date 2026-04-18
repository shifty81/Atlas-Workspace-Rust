#pragma once
// NF::Editor — Gradient ramp + editor panel
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

enum class GradientType : uint8_t {
    Linear, Radial, Angular, Diamond, Square, Reflected, Conical, Custom
};

inline const char* gradientTypeName(GradientType t) {
    switch (t) {
        case GradientType::Linear:    return "Linear";
        case GradientType::Radial:    return "Radial";
        case GradientType::Angular:   return "Angular";
        case GradientType::Diamond:   return "Diamond";
        case GradientType::Square:    return "Square";
        case GradientType::Reflected: return "Reflected";
        case GradientType::Conical:   return "Conical";
        case GradientType::Custom:    return "Custom";
    }
    return "Unknown";
}

enum class GradientInterpolation : uint8_t {
    Linear, Step, Spline, Constant
};

inline const char* gradientInterpolationName(GradientInterpolation i) {
    switch (i) {
        case GradientInterpolation::Linear:   return "Linear";
        case GradientInterpolation::Step:     return "Step";
        case GradientInterpolation::Spline:   return "Spline";
        case GradientInterpolation::Constant: return "Constant";
    }
    return "Unknown";
}

struct GradientColorStop {
    float                 position      = 0.0f; // [0,1]
    float                 r = 1.0f, g = 1.0f, b = 1.0f, a = 1.0f;
    GradientInterpolation interpolation = GradientInterpolation::Linear;
    bool                  selected      = false;

    void select()   { selected = true;  }
    void deselect() { selected = false; }
    void setPosition(float p) { position = p; }
};

class GradientRamp {
public:
    static constexpr size_t MAX_STOPS = 64;

    explicit GradientRamp(const std::string& name, GradientType type = GradientType::Linear)
        : m_name(name), m_type(type) {}

    [[nodiscard]] bool addStop(const GradientColorStop& s) {
        for (auto& e : m_stops) {
            if (e.position == s.position) return false;
        }
        if (m_stops.size() >= MAX_STOPS) return false;
        m_stops.push_back(s);
        return true;
    }

    [[nodiscard]] bool removeStop(float position) {
        for (auto it = m_stops.begin(); it != m_stops.end(); ++it) {
            if (it->position == position) { m_stops.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] GradientColorStop* findStop(float position) {
        for (auto& s : m_stops)
            if (s.position == position) return &s;
        return nullptr;
    }

    void selectAll()   { for (auto& s : m_stops) s.select();   }
    void deselectAll() { for (auto& s : m_stops) s.deselect(); }

    [[nodiscard]] size_t stopCount()     const { return m_stops.size(); }
    [[nodiscard]] size_t selectedCount() const {
        size_t c = 0; for (auto& s : m_stops) if (s.selected) ++c; return c;
    }
    [[nodiscard]] const std::string& name() const { return m_name; }
    [[nodiscard]] GradientType       type() const { return m_type; }

private:
    std::string               m_name;
    GradientType              m_type;
    std::vector<GradientColorStop> m_stops;
};

class GradientEditorPanel {
public:
    static constexpr size_t MAX_RAMPS = 32;

    [[nodiscard]] bool addRamp(const GradientRamp& ramp) {
        for (auto& r : m_ramps)
            if (r.name() == ramp.name()) return false;
        if (m_ramps.size() >= MAX_RAMPS) return false;
        m_ramps.push_back(ramp);
        return true;
    }

    [[nodiscard]] bool removeRamp(const std::string& name) {
        for (auto it = m_ramps.begin(); it != m_ramps.end(); ++it) {
            if (it->name() == name) {
                if (m_activeRamp == name) m_activeRamp.clear();
                m_ramps.erase(it);
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] GradientRamp* findRamp(const std::string& name) {
        for (auto& r : m_ramps)
            if (r.name() == name) return &r;
        return nullptr;
    }

    [[nodiscard]] bool setActiveRamp(const std::string& name) {
        for (auto& r : m_ramps) {
            if (r.name() == name) { m_activeRamp = name; return true; }
        }
        return false;
    }

    [[nodiscard]] const std::string& activeRamp()  const { return m_activeRamp; }
    [[nodiscard]] size_t             rampCount()   const { return m_ramps.size(); }
    [[nodiscard]] bool               isSymmetric() const { return m_symmetric; }
    void                             setSymmetric(bool v) { m_symmetric = v; }

    void selectAllStops()   { for (auto& r : m_ramps) r.selectAll();   }
    void deselectAllStops() { for (auto& r : m_ramps) r.deselectAll(); }

private:
    std::vector<GradientRamp> m_ramps;
    std::string               m_activeRamp;
    bool                      m_symmetric = false;
};

// ── S31 — Timeline Editor ────────────────────────────────────────


} // namespace NF
