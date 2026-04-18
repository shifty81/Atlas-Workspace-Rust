#pragma once
// NF::Editor — Tilemap editor v1: 2D tile grid authoring and layer management
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Tev1LayerBlend : uint8_t { Normal, Additive, Multiply, Screen };

inline const char* tev1LayerBlendName(Tev1LayerBlend b) {
    switch (b) {
        case Tev1LayerBlend::Normal:   return "Normal";
        case Tev1LayerBlend::Additive: return "Additive";
        case Tev1LayerBlend::Multiply: return "Multiply";
        case Tev1LayerBlend::Screen:   return "Screen";
    }
    return "Unknown";
}

struct Tev1Tile {
    uint64_t    id        = 0;
    std::string name;
    uint32_t    tilesetX  = 0;  // source tile column in tileset
    uint32_t    tilesetY  = 0;  // source tile row in tileset
    bool        passable  = true;
    bool        animated  = false;

    [[nodiscard]] bool isValid() const { return id != 0 && !name.empty(); }
};

struct Tev1Layer {
    uint64_t       id       = 0;
    std::string    name;
    float          opacity  = 1.f;
    bool           visible  = true;
    Tev1LayerBlend blend    = Tev1LayerBlend::Normal;
    std::vector<Tev1Tile> tiles;

    [[nodiscard]] bool isValid() const { return id != 0 && !name.empty(); }
};

using Tev1ChangeCallback = std::function<void(uint64_t)>;

class TilemapEditorV1 {
public:
    static constexpr size_t MAX_LAYERS = 64;

    bool addLayer(const Tev1Layer& layer) {
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

    [[nodiscard]] Tev1Layer* findLayer(uint64_t id) {
        for (auto& l : m_layers) if (l.id == id) return &l;
        return nullptr;
    }

    bool addTileToLayer(uint64_t layerId, const Tev1Tile& tile) {
        auto* l = findLayer(layerId);
        if (!l || !tile.isValid()) return false;
        for (const auto& t : l->tiles) if (t.id == tile.id) return false;
        l->tiles.push_back(tile);
        if (m_onChange) m_onChange(layerId);
        return true;
    }

    bool setLayerVisibility(uint64_t id, bool visible) {
        auto* l = findLayer(id);
        if (!l) return false;
        l->visible = visible;
        if (m_onChange) m_onChange(id);
        return true;
    }

    bool setLayerOpacity(uint64_t id, float opacity) {
        auto* l = findLayer(id);
        if (!l) return false;
        l->opacity = std::clamp(opacity, 0.f, 1.f);
        if (m_onChange) m_onChange(id);
        return true;
    }

    [[nodiscard]] size_t layerCount() const { return m_layers.size(); }

    [[nodiscard]] size_t visibleLayerCount() const {
        size_t c = 0;
        for (const auto& l : m_layers) if (l.visible) ++c;
        return c;
    }

    [[nodiscard]] size_t totalTileCount() const {
        size_t c = 0;
        for (const auto& l : m_layers) c += l.tiles.size();
        return c;
    }

    void setOnChange(Tev1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Tev1Layer> m_layers;
    Tev1ChangeCallback     m_onChange;
};

} // namespace NF
