#pragma once
// NF::Editor — Voxel paint editor v1: voxel layer and stroke management
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Vpv1BrushShape : uint8_t { Sphere, Cube, Cylinder, Flat, Custom };
enum class Vpv1LayerState : uint8_t { Hidden, Visible, Locked, Active };

inline const char* vpv1BrushShapeName(Vpv1BrushShape s) {
    switch (s) {
        case Vpv1BrushShape::Sphere:   return "Sphere";
        case Vpv1BrushShape::Cube:     return "Cube";
        case Vpv1BrushShape::Cylinder: return "Cylinder";
        case Vpv1BrushShape::Flat:     return "Flat";
        case Vpv1BrushShape::Custom:   return "Custom";
    }
    return "Unknown";
}

inline const char* vpv1LayerStateName(Vpv1LayerState s) {
    switch (s) {
        case Vpv1LayerState::Hidden:  return "Hidden";
        case Vpv1LayerState::Visible: return "Visible";
        case Vpv1LayerState::Locked:  return "Locked";
        case Vpv1LayerState::Active:  return "Active";
    }
    return "Unknown";
}

struct Vpv1Layer {
    uint64_t        id    = 0;
    std::string     name;
    Vpv1LayerState  state = Vpv1LayerState::Hidden;

    [[nodiscard]] bool isValid()   const { return id != 0 && !name.empty(); }
    [[nodiscard]] bool isVisible() const { return state == Vpv1LayerState::Visible; }
    [[nodiscard]] bool isLocked()  const { return state == Vpv1LayerState::Locked; }
    [[nodiscard]] bool isActive()  const { return state == Vpv1LayerState::Active; }
};

struct Vpv1Stroke {
    uint64_t      id         = 0;
    uint64_t      layerId    = 0;
    uint32_t      materialId = 0;
    Vpv1BrushShape brushShape = Vpv1BrushShape::Sphere;

    [[nodiscard]] bool isValid() const { return id != 0 && layerId != 0; }
};

using Vpv1ChangeCallback = std::function<void(uint64_t)>;

class VoxelPaintEditorV1 {
public:
    static constexpr size_t MAX_LAYERS  = 128;
    static constexpr size_t MAX_STROKES = 65536;

    bool addLayer(const Vpv1Layer& layer) {
        if (!layer.isValid()) return false;
        for (const auto& l : m_layers) if (l.id == layer.id) return false;
        if (m_layers.size() >= MAX_LAYERS) return false;
        m_layers.push_back(layer);
        if (m_onChange) m_onChange(layer.id);
        return true;
    }

    bool removeLayer(uint64_t id) {
        for (auto it = m_layers.begin(); it != m_layers.end(); ++it) {
            if (it->id == id) { m_layers.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] Vpv1Layer* findLayer(uint64_t id) {
        for (auto& l : m_layers) if (l.id == id) return &l;
        return nullptr;
    }

    bool addStroke(const Vpv1Stroke& stroke) {
        if (!stroke.isValid()) return false;
        for (const auto& s : m_strokes) if (s.id == stroke.id) return false;
        if (m_strokes.size() >= MAX_STROKES) return false;
        m_strokes.push_back(stroke);
        if (m_onChange) m_onChange(stroke.layerId);
        return true;
    }

    bool removeStroke(uint64_t id) {
        for (auto it = m_strokes.begin(); it != m_strokes.end(); ++it) {
            if (it->id == id) { m_strokes.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] size_t layerCount()  const { return m_layers.size(); }
    [[nodiscard]] size_t strokeCount() const { return m_strokes.size(); }

    [[nodiscard]] size_t visibleCount() const {
        size_t c = 0; for (const auto& l : m_layers) if (l.isVisible()) ++c; return c;
    }
    [[nodiscard]] size_t lockedCount() const {
        size_t c = 0; for (const auto& l : m_layers) if (l.isLocked()) ++c; return c;
    }
    [[nodiscard]] size_t countByBrushShape(Vpv1BrushShape shape) const {
        size_t c = 0; for (const auto& s : m_strokes) if (s.brushShape == shape) ++c; return c;
    }

    void setOnChange(Vpv1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Vpv1Layer>  m_layers;
    std::vector<Vpv1Stroke> m_strokes;
    Vpv1ChangeCallback      m_onChange;
};

} // namespace NF
