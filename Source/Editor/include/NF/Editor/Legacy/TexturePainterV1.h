#pragma once
// NF::Editor — Texture painter v1: per-pixel painting and brush authoring
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Tpv1BrushShape : uint8_t { Round, Square, Diamond, Custom };
enum class Tpv1BlendMode  : uint8_t { Normal, Add, Subtract, Multiply };

inline const char* tpv1BrushShapeName(Tpv1BrushShape s) {
    switch (s) {
        case Tpv1BrushShape::Round:   return "Round";
        case Tpv1BrushShape::Square:  return "Square";
        case Tpv1BrushShape::Diamond: return "Diamond";
        case Tpv1BrushShape::Custom:  return "Custom";
    }
    return "Unknown";
}

inline const char* tpv1BlendModeName(Tpv1BlendMode m) {
    switch (m) {
        case Tpv1BlendMode::Normal:   return "Normal";
        case Tpv1BlendMode::Add:      return "Add";
        case Tpv1BlendMode::Subtract: return "Subtract";
        case Tpv1BlendMode::Multiply: return "Multiply";
    }
    return "Unknown";
}

struct Tpv1Brush {
    uint64_t       id       = 0;
    std::string    name;
    Tpv1BrushShape shape    = Tpv1BrushShape::Round;
    float          size     = 16.f;
    float          hardness = 0.8f;
    float          opacity  = 1.f;
    float          flow     = 1.f;
    Tpv1BlendMode  blend    = Tpv1BlendMode::Normal;

    [[nodiscard]] bool isValid() const { return id != 0 && !name.empty() && size > 0.f; }
};

struct Tpv1Stroke {
    uint64_t id       = 0;
    uint64_t brushId  = 0;
    uint32_t color    = 0xFFFFFFFF;
    size_t   pointCount = 0;

    [[nodiscard]] bool isValid() const { return id != 0 && brushId != 0; }
};

using Tpv1ChangeCallback = std::function<void(uint64_t)>;

class TexturePainterV1 {
public:
    static constexpr size_t MAX_BRUSHES = 128;
    static constexpr size_t MAX_STROKES = 4096;

    bool addBrush(const Tpv1Brush& brush) {
        if (!brush.isValid()) return false;
        for (const auto& b : m_brushes) if (b.id == brush.id) return false;
        if (m_brushes.size() >= MAX_BRUSHES) return false;
        m_brushes.push_back(brush);
        return true;
    }

    bool removeBrush(uint64_t id) {
        for (auto it = m_brushes.begin(); it != m_brushes.end(); ++it) {
            if (it->id == id) { m_brushes.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] Tpv1Brush* findBrush(uint64_t id) {
        for (auto& b : m_brushes) if (b.id == id) return &b;
        return nullptr;
    }

    bool selectBrush(uint64_t id) {
        if (!findBrush(id)) return false;
        m_activeBrushId = id;
        return true;
    }

    [[nodiscard]] uint64_t activeBrushId() const { return m_activeBrushId; }

    bool recordStroke(const Tpv1Stroke& stroke) {
        if (!stroke.isValid()) return false;
        if (m_strokes.size() >= MAX_STROKES) return false;
        m_strokes.push_back(stroke);
        if (m_onChange) m_onChange(stroke.id);
        return true;
    }

    bool undoLastStroke() {
        if (m_strokes.empty()) return false;
        m_strokes.pop_back();
        return true;
    }

    [[nodiscard]] size_t brushCount()  const { return m_brushes.size(); }
    [[nodiscard]] size_t strokeCount() const { return m_strokes.size(); }

    [[nodiscard]] size_t countByShape(Tpv1BrushShape shape) const {
        size_t c = 0;
        for (const auto& b : m_brushes) if (b.shape == shape) ++c;
        return c;
    }

    void setOnChange(Tpv1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Tpv1Brush>  m_brushes;
    std::vector<Tpv1Stroke> m_strokes;
    uint64_t                m_activeBrushId = 0;
    Tpv1ChangeCallback      m_onChange;
};

} // namespace NF
