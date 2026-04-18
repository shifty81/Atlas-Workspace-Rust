#pragma once
// NF::Editor — Terrain brush v1: height/paint brush authoring for terrain editing
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Tbv1BrushMode   : uint8_t { Raise, Lower, Smooth, Flatten, Paint, Stamp };
enum class Tbv1BrushShape  : uint8_t { Circle, Square, Diamond, Custom };
enum class Tbv1FalloffCurve: uint8_t { Linear, Smooth, Constant, InverseSqr };

struct Tbv1BrushSettings {
    float radius    = 10.f;
    float strength  = 0.5f;
    float opacity   = 1.f;
    float falloff   = 0.5f;
    Tbv1FalloffCurve curve = Tbv1FalloffCurve::Smooth;
    [[nodiscard]] bool isValid() const { return radius > 0.f && strength > 0.f; }
};

struct Tbv1Stroke {
    uint64_t          id = 0;
    float             x  = 0.f;
    float             y  = 0.f;
    Tbv1BrushMode     mode;
    Tbv1BrushSettings settings;
    [[nodiscard]] bool isValid() const { return id != 0 && settings.isValid(); }
};

using Tbv1StrokeCallback = std::function<void(const Tbv1Stroke&)>;

class TerrainBrushV1 {
public:
    void setBrushMode(Tbv1BrushMode mode)         { m_mode = mode;     }
    void setBrushShape(Tbv1BrushShape shape)       { m_shape = shape;   }
    void setBrushSettings(const Tbv1BrushSettings& s) { m_settings = s; }

    [[nodiscard]] Tbv1BrushMode      brushMode()     const { return m_mode;     }
    [[nodiscard]] Tbv1BrushShape     brushShape()    const { return m_shape;    }
    [[nodiscard]] Tbv1BrushSettings  brushSettings() const { return m_settings; }

    bool applyStroke(const Tbv1Stroke& s) {
        if (!s.isValid()) return false;
        m_strokes.push_back(s);
        if (m_onStroke) m_onStroke(s);
        return true;
    }

    bool undoStroke() {
        if (m_strokes.empty()) return false;
        m_strokes.pop_back();
        return true;
    }

    bool clearStrokes() {
        if (m_strokes.empty()) return false;
        m_strokes.clear();
        return true;
    }

    [[nodiscard]] size_t strokeCount() const { return m_strokes.size(); }

    void setOnStroke(Tbv1StrokeCallback cb) { m_onStroke = std::move(cb); }

private:
    std::vector<Tbv1Stroke> m_strokes;
    Tbv1BrushMode           m_mode  = Tbv1BrushMode::Raise;
    Tbv1BrushShape          m_shape = Tbv1BrushShape::Circle;
    Tbv1BrushSettings       m_settings;
    Tbv1StrokeCallback      m_onStroke;
};

} // namespace NF
