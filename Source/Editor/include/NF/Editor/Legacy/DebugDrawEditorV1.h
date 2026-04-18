#pragma once
// NF::Editor — Debug draw editor v1: line, box, sphere, arrow, capsule and grid shape authoring
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Ddv1ShapeType : uint8_t { Line, Box, Sphere, Arrow, Capsule, Grid };

inline const char* ddv1ShapeTypeName(Ddv1ShapeType t) {
    switch (t) {
        case Ddv1ShapeType::Line:    return "Line";
        case Ddv1ShapeType::Box:     return "Box";
        case Ddv1ShapeType::Sphere:  return "Sphere";
        case Ddv1ShapeType::Arrow:   return "Arrow";
        case Ddv1ShapeType::Capsule: return "Capsule";
        case Ddv1ShapeType::Grid:    return "Grid";
    }
    return "Unknown";
}

struct Ddv1Shape {
    uint64_t      id       = 0;
    std::string   name;
    Ddv1ShapeType type     = Ddv1ShapeType::Line;
    float         duration = 0.f;
    float         colorR   = 1.f;
    float         colorG   = 1.f;
    float         colorB   = 1.f;
    float         colorA   = 1.f;
    float         posX     = 0.f;
    float         posY     = 0.f;
    float         posZ     = 0.f;
    bool          enabled  = true;

    [[nodiscard]] bool isValid() const { return id != 0 && !name.empty(); }
};

using Ddv1ChangeCallback = std::function<void(uint64_t)>;

class DebugDrawEditorV1 {
public:
    static constexpr size_t MAX_SHAPES = 1024;

    bool addShape(const Ddv1Shape& shape) {
        if (!shape.isValid()) return false;
        for (const auto& s : m_shapes) if (s.id == shape.id) return false;
        if (m_shapes.size() >= MAX_SHAPES) return false;
        m_shapes.push_back(shape);
        if (m_onChange) m_onChange(shape.id);
        return true;
    }

    bool removeShape(uint64_t id) {
        for (auto it = m_shapes.begin(); it != m_shapes.end(); ++it) {
            if (it->id == id) { m_shapes.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] Ddv1Shape* findShape(uint64_t id) {
        for (auto& s : m_shapes) if (s.id == id) return &s;
        return nullptr;
    }

    bool setState(uint64_t id, bool enabled) {
        auto* s = findShape(id);
        if (!s) return false;
        s->enabled = enabled;
        if (m_onChange) m_onChange(id);
        return true;
    }

    bool setDuration(uint64_t id, float duration) {
        auto* s = findShape(id);
        if (!s) return false;
        s->duration = std::clamp(duration, 0.f, 3600.f);
        if (m_onChange) m_onChange(id);
        return true;
    }

    void clearAll() {
        m_shapes.clear();
        if (m_onChange) m_onChange(0);
    }

    [[nodiscard]] size_t shapeCount()   const { return m_shapes.size(); }

    [[nodiscard]] size_t enabledCount() const {
        size_t c = 0;
        for (const auto& s : m_shapes) if (s.enabled) ++c;
        return c;
    }

    [[nodiscard]] size_t countByType(Ddv1ShapeType type) const {
        size_t c = 0;
        for (const auto& s : m_shapes) if (s.type == type) ++c;
        return c;
    }

    void setOnChange(Ddv1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Ddv1Shape> m_shapes;
    Ddv1ChangeCallback     m_onChange;
};

} // namespace NF
