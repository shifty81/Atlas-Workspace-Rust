#pragma once
// NF::Editor — Heightmap editor v1: heightmap layer and brush management
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Hmev1LayerType  : uint8_t { Base, Erosion, Noise, Stamp, Sculpt, Import };
enum class Hmev1LayerState : uint8_t { Hidden, Visible, Locked, Solo };

inline const char* hmev1LayerTypeName(Hmev1LayerType t) {
    switch (t) {
        case Hmev1LayerType::Base:    return "Base";
        case Hmev1LayerType::Erosion: return "Erosion";
        case Hmev1LayerType::Noise:   return "Noise";
        case Hmev1LayerType::Stamp:   return "Stamp";
        case Hmev1LayerType::Sculpt:  return "Sculpt";
        case Hmev1LayerType::Import:  return "Import";
    }
    return "Unknown";
}

inline const char* hmev1LayerStateName(Hmev1LayerState s) {
    switch (s) {
        case Hmev1LayerState::Hidden:  return "Hidden";
        case Hmev1LayerState::Visible: return "Visible";
        case Hmev1LayerState::Locked:  return "Locked";
        case Hmev1LayerState::Solo:    return "Solo";
    }
    return "Unknown";
}

struct Hmev1Layer {
    uint64_t         id        = 0;
    std::string      name;
    Hmev1LayerType   layerType = Hmev1LayerType::Base;
    Hmev1LayerState  state     = Hmev1LayerState::Visible;
    float            opacity   = 1.0f;
    uint32_t         resolution = 512;

    [[nodiscard]] bool isValid()   const { return id != 0 && !name.empty(); }
    [[nodiscard]] bool isVisible() const { return state == Hmev1LayerState::Visible; }
    [[nodiscard]] bool isLocked()  const { return state == Hmev1LayerState::Locked; }
    [[nodiscard]] bool isSolo()    const { return state == Hmev1LayerState::Solo; }
};

struct Hmev1Brush {
    uint64_t    id   = 0;
    std::string name;
    float       size     = 50.0f;
    float       strength = 0.5f;

    [[nodiscard]] bool isValid() const { return id != 0 && !name.empty(); }
};

using Hmev1ChangeCallback = std::function<void(uint64_t)>;

class HeightmapEditorV1 {
public:
    static constexpr size_t MAX_LAYERS  = 64;
    static constexpr size_t MAX_BRUSHES = 128;

    bool addLayer(const Hmev1Layer& layer) {
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

    [[nodiscard]] Hmev1Layer* findLayer(uint64_t id) {
        for (auto& l : m_layers) if (l.id == id) return &l;
        return nullptr;
    }

    bool addBrush(const Hmev1Brush& brush) {
        if (!brush.isValid()) return false;
        for (const auto& b : m_brushes) if (b.id == brush.id) return false;
        if (m_brushes.size() >= MAX_BRUSHES) return false;
        m_brushes.push_back(brush);
        if (m_onChange) m_onChange(brush.id);
        return true;
    }

    bool removeBrush(uint64_t id) {
        for (auto it = m_brushes.begin(); it != m_brushes.end(); ++it) {
            if (it->id == id) { m_brushes.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] size_t layerCount() const { return m_layers.size(); }
    [[nodiscard]] size_t brushCount() const { return m_brushes.size(); }

    [[nodiscard]] size_t visibleCount() const {
        size_t c = 0; for (const auto& l : m_layers) if (l.isVisible()) ++c; return c;
    }
    [[nodiscard]] size_t lockedCount() const {
        size_t c = 0; for (const auto& l : m_layers) if (l.isLocked()) ++c; return c;
    }
    [[nodiscard]] size_t countByType(Hmev1LayerType type) const {
        size_t c = 0; for (const auto& l : m_layers) if (l.layerType == type) ++c; return c;
    }

    void setOnChange(Hmev1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Hmev1Layer> m_layers;
    std::vector<Hmev1Brush> m_brushes;
    Hmev1ChangeCallback     m_onChange;
};

} // namespace NF
