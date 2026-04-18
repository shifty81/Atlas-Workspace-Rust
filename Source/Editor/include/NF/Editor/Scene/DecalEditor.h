#pragma once
// NF::Editor — Decal asset and placement editor
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

enum class DecalBlendMode : uint8_t {
    Translucent, Additive, Multiply, Normal, Emissive
};

inline const char* decalBlendModeName(DecalBlendMode m) {
    switch (m) {
        case DecalBlendMode::Translucent: return "Translucent";
        case DecalBlendMode::Additive:    return "Additive";
        case DecalBlendMode::Multiply:    return "Multiply";
        case DecalBlendMode::Normal:      return "Normal";
        case DecalBlendMode::Emissive:    return "Emissive";
    }
    return "Unknown";
}

enum class DecalProjection : uint8_t {
    Box, Sphere, Cylinder, Mesh
};

inline const char* decalProjectionName(DecalProjection p) {
    switch (p) {
        case DecalProjection::Box:      return "Box";
        case DecalProjection::Sphere:   return "Sphere";
        case DecalProjection::Cylinder: return "Cylinder";
        case DecalProjection::Mesh:     return "Mesh";
    }
    return "Unknown";
}

enum class DecalSortOrder : uint8_t {
    Default, Front, Back, Manual
};

inline const char* decalSortOrderName(DecalSortOrder o) {
    switch (o) {
        case DecalSortOrder::Default: return "Default";
        case DecalSortOrder::Front:   return "Front";
        case DecalSortOrder::Back:    return "Back";
        case DecalSortOrder::Manual:  return "Manual";
    }
    return "Unknown";
}

class DecalAsset {
public:
    explicit DecalAsset(const std::string& name)
        : m_name(name) {}

    void setBlendMode(DecalBlendMode m)  { m_blendMode  = m; }
    void setProjection(DecalProjection p){ m_projection = p; }
    void setSortOrder(DecalSortOrder o)  { m_sortOrder  = o; }
    void setOpacity(float o)             { m_opacity    = o; }
    void setDepthFade(float f)           { m_depthFade  = f; }
    void setReceiveDecals(bool v)        { m_receiveDecals = v; }
    void setEnabled(bool v)              { m_enabled    = v; }
    void setDirty(bool v)                { m_dirty      = v; }

    [[nodiscard]] const std::string& name()           const { return m_name;          }
    [[nodiscard]] DecalBlendMode     blendMode()      const { return m_blendMode;     }
    [[nodiscard]] DecalProjection    projection()     const { return m_projection;    }
    [[nodiscard]] DecalSortOrder     sortOrder()      const { return m_sortOrder;     }
    [[nodiscard]] float              opacity()        const { return m_opacity;       }
    [[nodiscard]] float              depthFade()      const { return m_depthFade;     }
    [[nodiscard]] bool               receiveDecals()  const { return m_receiveDecals; }
    [[nodiscard]] bool               isEnabled()      const { return m_enabled;       }
    [[nodiscard]] bool               isDirty()        const { return m_dirty;         }

private:
    std::string       m_name;
    DecalBlendMode    m_blendMode    = DecalBlendMode::Translucent;
    DecalProjection   m_projection   = DecalProjection::Box;
    DecalSortOrder    m_sortOrder    = DecalSortOrder::Default;
    float             m_opacity      = 1.0f;
    float             m_depthFade    = 0.0f;
    bool              m_receiveDecals = false;
    bool              m_enabled      = true;
    bool              m_dirty        = false;
};

class DecalEditor {
public:
    static constexpr size_t MAX_DECALS = 256;

    [[nodiscard]] bool addDecal(const DecalAsset& decal) {
        for (auto& d : m_decals) if (d.name() == decal.name()) return false;
        if (m_decals.size() >= MAX_DECALS) return false;
        m_decals.push_back(decal);
        return true;
    }

    [[nodiscard]] bool removeDecal(const std::string& name) {
        for (auto it = m_decals.begin(); it != m_decals.end(); ++it) {
            if (it->name() == name) {
                if (m_activeDecal == name) m_activeDecal.clear();
                m_decals.erase(it);
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] DecalAsset* findDecal(const std::string& name) {
        for (auto& d : m_decals) if (d.name() == name) return &d;
        return nullptr;
    }

    [[nodiscard]] bool setActiveDecal(const std::string& name) {
        for (auto& d : m_decals) if (d.name() == name) { m_activeDecal = name; return true; }
        return false;
    }

    [[nodiscard]] const std::string& activeDecal() const { return m_activeDecal; }
    [[nodiscard]] size_t decalCount()  const { return m_decals.size(); }
    [[nodiscard]] size_t dirtyCount()  const {
        size_t c = 0; for (auto& d : m_decals) if (d.isDirty()) ++c; return c;
    }
    [[nodiscard]] size_t countByBlendMode(DecalBlendMode m) const {
        size_t c = 0; for (auto& d : m_decals) if (d.blendMode() == m) ++c; return c;
    }
    [[nodiscard]] size_t countByProjection(DecalProjection p) const {
        size_t c = 0; for (auto& d : m_decals) if (d.projection() == p) ++c; return c;
    }

private:
    std::vector<DecalAsset> m_decals;
    std::string             m_activeDecal;
};

} // namespace NF
