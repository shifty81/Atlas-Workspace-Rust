#pragma once
// NF::Editor — Gradient editor v1: color stop ramp authoring
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Gev1GradientType : uint8_t { Linear, Radial, Angular, Diamond };
enum class Gev1Interp       : uint8_t { Linear, Step, Smooth, EaseIn, EaseOut };

struct Gev1ColorStop {
    uint64_t id       = 0;
    float    position = 0.f;   // 0..1
    float    r        = 0.f;
    float    g        = 0.f;
    float    b        = 0.f;
    float    a        = 1.f;
    [[nodiscard]] bool isValid() const { return id != 0 && position >= 0.f && position <= 1.f; }
};

using Gev1ChangeCallback = std::function<void()>;

class GradientEditorV1 {
public:
    bool addStop(const Gev1ColorStop& s) {
        if (!s.isValid()) return false;
        for (const auto& es : m_stops) if (es.id == s.id) return false;
        m_stops.push_back(s);
        sortStops();
        if (m_onChange) m_onChange();
        return true;
    }

    bool removeStop(uint64_t id) {
        for (auto it = m_stops.begin(); it != m_stops.end(); ++it) {
            if (it->id == id) { m_stops.erase(it); if (m_onChange) m_onChange(); return true; }
        }
        return false;
    }

    bool moveStop(uint64_t id, float pos) {
        if (pos < 0.f || pos > 1.f) return false;
        for (auto& s : m_stops) {
            if (s.id == id) { s.position = pos; sortStops(); if (m_onChange) m_onChange(); return true; }
        }
        return false;
    }

    void setGradientType(Gev1GradientType t) { m_type = t; }
    void setInterp(Gev1Interp i)             { m_interp = i; }

    [[nodiscard]] Gev1GradientType gradientType() const { return m_type;         }
    [[nodiscard]] Gev1Interp       interp()        const { return m_interp;       }
    [[nodiscard]] size_t           stopCount()     const { return m_stops.size(); }

    void setOnChange(Gev1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    void sortStops() {
        std::sort(m_stops.begin(), m_stops.end(),
            [](const Gev1ColorStop& a, const Gev1ColorStop& b){ return a.position < b.position; });
    }

    std::vector<Gev1ColorStop> m_stops;
    Gev1GradientType           m_type   = Gev1GradientType::Linear;
    Gev1Interp                 m_interp = Gev1Interp::Linear;
    Gev1ChangeCallback         m_onChange;
};

} // namespace NF
