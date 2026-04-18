#pragma once
// NF::Editor — Terrain sculpt and paint editor
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

enum class TerrainBrushMode : uint8_t {
    Raise, Lower, Flatten, Smooth, Noise, Paint
};

inline const char* terrainBrushModeName(TerrainBrushMode m) {
    switch (m) {
        case TerrainBrushMode::Raise:   return "Raise";
        case TerrainBrushMode::Lower:   return "Lower";
        case TerrainBrushMode::Flatten: return "Flatten";
        case TerrainBrushMode::Smooth:  return "Smooth";
        case TerrainBrushMode::Noise:   return "Noise";
        case TerrainBrushMode::Paint:   return "Paint";
    }
    return "Unknown";
}

enum class TerrainLayerBlend : uint8_t {
    Normal, Height, Slope, Distance, Custom
};

inline const char* terrainLayerBlendName(TerrainLayerBlend b) {
    switch (b) {
        case TerrainLayerBlend::Normal:   return "Normal";
        case TerrainLayerBlend::Height:   return "Height";
        case TerrainLayerBlend::Slope:    return "Slope";
        case TerrainLayerBlend::Distance: return "Distance";
        case TerrainLayerBlend::Custom:   return "Custom";
    }
    return "Unknown";
}

enum class TerrainResolution : uint8_t {
    R64, R128, R256, R512, R1024, R2048
};

inline const char* terrainResolutionName(TerrainResolution r) {
    switch (r) {
        case TerrainResolution::R64:   return "R64";
        case TerrainResolution::R128:  return "R128";
        case TerrainResolution::R256:  return "R256";
        case TerrainResolution::R512:  return "R512";
        case TerrainResolution::R1024: return "R1024";
        case TerrainResolution::R2048: return "R2048";
    }
    return "Unknown";
}

class TerrainLayer {
public:
    explicit TerrainLayer(const std::string& name, TerrainLayerBlend blend)
        : m_name(name), m_blend(blend) {}

    void setWeight(float w)       { m_weight  = w; }
    void setTileScale(float s)    { m_tileScale = s; }
    void setEnabled(bool v)       { m_enabled = v; }

    [[nodiscard]] const std::string& name()      const { return m_name;      }
    [[nodiscard]] TerrainLayerBlend  blend()     const { return m_blend;     }
    [[nodiscard]] float              weight()    const { return m_weight;    }
    [[nodiscard]] float              tileScale() const { return m_tileScale; }
    [[nodiscard]] bool               isEnabled() const { return m_enabled;   }

private:
    std::string        m_name;
    TerrainLayerBlend  m_blend;
    float              m_weight    = 1.0f;
    float              m_tileScale = 1.0f;
    bool               m_enabled   = true;
};

class TerrainEditor {
public:
    static constexpr size_t MAX_LAYERS = 16;

    [[nodiscard]] bool addLayer(const TerrainLayer& layer) {
        for (auto& l : m_layers) if (l.name() == layer.name()) return false;
        if (m_layers.size() >= MAX_LAYERS) return false;
        m_layers.push_back(layer);
        return true;
    }

    [[nodiscard]] bool removeLayer(const std::string& name) {
        for (auto it = m_layers.begin(); it != m_layers.end(); ++it) {
            if (it->name() == name) { m_layers.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] TerrainLayer* findLayer(const std::string& name) {
        for (auto& l : m_layers) if (l.name() == name) return &l;
        return nullptr;
    }

    void setBrushMode(TerrainBrushMode m) { m_brushMode  = m; }
    void setBrushRadius(float r)          { m_brushRadius = r; }
    void setBrushStrength(float s)        { m_brushStrength = s; }
    void setResolution(TerrainResolution r) { m_resolution = r; }
    void setActiveLayer(const std::string& n) { m_activeLayer = n; }

    [[nodiscard]] TerrainBrushMode   brushMode()     const { return m_brushMode;     }
    [[nodiscard]] float              brushRadius()   const { return m_brushRadius;   }
    [[nodiscard]] float              brushStrength() const { return m_brushStrength; }
    [[nodiscard]] TerrainResolution  resolution()    const { return m_resolution;    }
    [[nodiscard]] const std::string& activeLayer()   const { return m_activeLayer;   }
    [[nodiscard]] size_t             layerCount()    const { return m_layers.size(); }

    [[nodiscard]] size_t enabledLayerCount() const {
        size_t c = 0; for (auto& l : m_layers) if (l.isEnabled()) ++c; return c;
    }
    [[nodiscard]] size_t countByBlend(TerrainLayerBlend b) const {
        size_t c = 0; for (auto& l : m_layers) if (l.blend() == b) ++c; return c;
    }

private:
    std::vector<TerrainLayer> m_layers;
    std::string               m_activeLayer;
    TerrainBrushMode          m_brushMode     = TerrainBrushMode::Raise;
    TerrainResolution         m_resolution    = TerrainResolution::R512;
    float                     m_brushRadius   = 10.0f;
    float                     m_brushStrength = 0.5f;
};

} // namespace NF
