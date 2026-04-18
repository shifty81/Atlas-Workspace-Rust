#pragma once
// NF::Editor — Material layer editor
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

enum class MaterialLayerBlendMode : uint8_t {
    Opaque, Translucent, Additive, Multiply, Screen, Overlay, Masked
};

inline const char* materialLayerBlendModeName(MaterialLayerBlendMode m) {
    switch (m) {
        case MaterialLayerBlendMode::Opaque:      return "Opaque";
        case MaterialLayerBlendMode::Translucent: return "Translucent";
        case MaterialLayerBlendMode::Additive:    return "Additive";
        case MaterialLayerBlendMode::Multiply:    return "Multiply";
        case MaterialLayerBlendMode::Screen:      return "Screen";
        case MaterialLayerBlendMode::Overlay:     return "Overlay";
        case MaterialLayerBlendMode::Masked:      return "Masked";
    }
    return "Unknown";
}

enum class MaterialLayerShadingModel : uint8_t {
    Lit, Unlit, SubsurfaceScattering, ClearCoat, TwoSidedFoliage, Hair
};

inline const char* materialLayerShadingModelName(MaterialLayerShadingModel m) {
    switch (m) {
        case MaterialLayerShadingModel::Lit:                  return "Lit";
        case MaterialLayerShadingModel::Unlit:                return "Unlit";
        case MaterialLayerShadingModel::SubsurfaceScattering: return "SubsurfaceScattering";
        case MaterialLayerShadingModel::ClearCoat:            return "ClearCoat";
        case MaterialLayerShadingModel::TwoSidedFoliage:      return "TwoSidedFoliage";
        case MaterialLayerShadingModel::Hair:                 return "Hair";
    }
    return "Unknown";
}

class MaterialLayer {
public:
    explicit MaterialLayer(uint32_t id, const std::string& name) : m_id(id), m_name(name) {}

    void setBlendMode(MaterialLayerBlendMode m)    { m_blendMode    = m; }
    void setShadingModel(MaterialLayerShadingModel m){ m_shadingModel = m; }
    void setOpacity(float v)                       { m_opacity      = v; }
    void setTiling(float v)                        { m_tiling       = v; }
    void setEnabled(bool v)                        { m_enabled      = v; }
    void setLocked(bool v)                         { m_locked       = v; }

    [[nodiscard]] uint32_t                   id()          const { return m_id;          }
    [[nodiscard]] const std::string&         name()        const { return m_name;        }
    [[nodiscard]] MaterialLayerBlendMode     blendMode()   const { return m_blendMode;   }
    [[nodiscard]] MaterialLayerShadingModel  shadingModel()const { return m_shadingModel;}
    [[nodiscard]] float                      opacity()     const { return m_opacity;     }
    [[nodiscard]] float                      tiling()      const { return m_tiling;      }
    [[nodiscard]] bool                       isEnabled()   const { return m_enabled;     }
    [[nodiscard]] bool                       isLocked()    const { return m_locked;      }

private:
    uint32_t                   m_id;
    std::string                m_name;
    MaterialLayerBlendMode     m_blendMode    = MaterialLayerBlendMode::Opaque;
    MaterialLayerShadingModel  m_shadingModel = MaterialLayerShadingModel::Lit;
    float                      m_opacity      = 1.0f;
    float                      m_tiling       = 1.0f;
    bool                       m_enabled      = true;
    bool                       m_locked       = false;
};

class MaterialLayerEditor {
public:
    [[nodiscard]] bool addLayer(const MaterialLayer& layer) {
        for (auto& l : m_layers) if (l.id() == layer.id()) return false;
        m_layers.push_back(layer);
        return true;
    }

    [[nodiscard]] bool removeLayer(uint32_t id) {
        for (auto it = m_layers.begin(); it != m_layers.end(); ++it) {
            if (it->id() == id) { m_layers.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] MaterialLayer* findLayer(uint32_t id) {
        for (auto& l : m_layers) if (l.id() == id) return &l;
        return nullptr;
    }

    [[nodiscard]] size_t layerCount()   const { return m_layers.size(); }
    [[nodiscard]] size_t enabledCount() const {
        size_t c = 0; for (auto& l : m_layers) if (l.isEnabled()) ++c; return c;
    }
    [[nodiscard]] size_t lockedCount()  const {
        size_t c = 0; for (auto& l : m_layers) if (l.isLocked()) ++c; return c;
    }
    [[nodiscard]] size_t countByBlendMode(MaterialLayerBlendMode m) const {
        size_t c = 0; for (auto& l : m_layers) if (l.blendMode() == m) ++c; return c;
    }
    [[nodiscard]] size_t countByShadingModel(MaterialLayerShadingModel m) const {
        size_t c = 0; for (auto& l : m_layers) if (l.shadingModel() == m) ++c; return c;
    }

private:
    std::vector<MaterialLayer> m_layers;
};

} // namespace NF
