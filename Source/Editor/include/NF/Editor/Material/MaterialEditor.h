#pragma once
// NF::Editor — Material asset + editor
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

enum class MaterialShadingModel : uint8_t {
    Unlit, Lambert, Phong, BlinnPhong, PBR, Toon, Subsurface, Custom
};

inline const char* materialShadingModelName(MaterialShadingModel m) {
    switch (m) {
        case MaterialShadingModel::Unlit:      return "Unlit";
        case MaterialShadingModel::Lambert:    return "Lambert";
        case MaterialShadingModel::Phong:      return "Phong";
        case MaterialShadingModel::BlinnPhong: return "BlinnPhong";
        case MaterialShadingModel::PBR:        return "PBR";
        case MaterialShadingModel::Toon:       return "Toon";
        case MaterialShadingModel::Subsurface: return "Subsurface";
        case MaterialShadingModel::Custom:     return "Custom";
    }
    return "Unknown";
}

enum class MaterialBlendMode : uint8_t {
    Opaque, Masked, Translucent, Additive
};

inline const char* materialBlendModeName(MaterialBlendMode b) {
    switch (b) {
        case MaterialBlendMode::Opaque:       return "Opaque";
        case MaterialBlendMode::Masked:       return "Masked";
        case MaterialBlendMode::Translucent:  return "Translucent";
        case MaterialBlendMode::Additive:     return "Additive";
    }
    return "Unknown";
}

struct MaterialParameter {
    std::string name;
    float       value = 0.0f;
    bool        exposed = false;

    void expose()  { exposed = true;  }
    void hide()    { exposed = false; }
};

class MaterialAsset {
public:
    explicit MaterialAsset(const std::string& name) : m_name(name) {}

    [[nodiscard]] bool addParameter(const MaterialParameter& p) {
        for (auto& existing : m_params) if (existing.name == p.name) return false;
        m_params.push_back(p);
        return true;
    }

    [[nodiscard]] bool removeParameter(const std::string& name) {
        for (auto it = m_params.begin(); it != m_params.end(); ++it) {
            if (it->name == name) { m_params.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] MaterialParameter* findParameter(const std::string& name) {
        for (auto& p : m_params) if (p.name == name) return &p;
        return nullptr;
    }

    void setShadingModel(MaterialShadingModel m) { m_shadingModel = m; }
    void setBlendMode(MaterialBlendMode b)        { m_blendMode = b; }
    void setDirty(bool d)                         { m_dirty = d; }

    [[nodiscard]] MaterialShadingModel shadingModel() const { return m_shadingModel; }
    [[nodiscard]] MaterialBlendMode    blendMode()    const { return m_blendMode; }
    [[nodiscard]] bool                 isDirty()      const { return m_dirty; }
    [[nodiscard]] size_t               paramCount()   const { return m_params.size(); }
    [[nodiscard]] size_t               exposedParamCount() const {
        size_t c = 0; for (auto& p : m_params) if (p.exposed) ++c; return c;
    }
    [[nodiscard]] const std::string&   name()         const { return m_name; }

private:
    std::string                   m_name;
    MaterialShadingModel          m_shadingModel = MaterialShadingModel::PBR;
    MaterialBlendMode             m_blendMode    = MaterialBlendMode::Opaque;
    bool                          m_dirty        = false;
    std::vector<MaterialParameter> m_params;
};

class MaterialEditor {
public:
    static constexpr size_t MAX_ASSETS = 128;

    [[nodiscard]] bool addAsset(const MaterialAsset& asset) {
        for (auto& a : m_assets) if (a.name() == asset.name()) return false;
        if (m_assets.size() >= MAX_ASSETS) return false;
        m_assets.push_back(asset);
        return true;
    }

    [[nodiscard]] bool removeAsset(const std::string& name) {
        for (auto it = m_assets.begin(); it != m_assets.end(); ++it) {
            if (it->name() == name) {
                if (m_activeAsset == name) m_activeAsset.clear();
                m_assets.erase(it);
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] MaterialAsset* findAsset(const std::string& name) {
        for (auto& a : m_assets) if (a.name() == name) return &a;
        return nullptr;
    }

    [[nodiscard]] bool setActiveAsset(const std::string& name) {
        for (auto& a : m_assets) {
            if (a.name() == name) { m_activeAsset = name; return true; }
        }
        return false;
    }

    [[nodiscard]] size_t            assetCount()   const { return m_assets.size(); }
    [[nodiscard]] const std::string& activeAsset() const { return m_activeAsset; }
    [[nodiscard]] size_t dirtyCount() const {
        size_t c = 0; for (auto& a : m_assets) if (a.isDirty()) ++c; return c;
    }

private:
    std::vector<MaterialAsset> m_assets;
    std::string                m_activeAsset;
};

// ── S35 — Texture Editor ──────────────────────────────────────────


} // namespace NF
