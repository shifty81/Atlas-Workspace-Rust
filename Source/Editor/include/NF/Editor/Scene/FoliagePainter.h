#pragma once
// NF::Editor — Foliage scatter and paint editor
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

enum class FoliagePlacementMode : uint8_t {
    Scatter, Paint, Erase, Select, Procedural
};

inline const char* foliagePlacementModeName(FoliagePlacementMode m) {
    switch (m) {
        case FoliagePlacementMode::Scatter:    return "Scatter";
        case FoliagePlacementMode::Paint:      return "Paint";
        case FoliagePlacementMode::Erase:      return "Erase";
        case FoliagePlacementMode::Select:     return "Select";
        case FoliagePlacementMode::Procedural: return "Procedural";
    }
    return "Unknown";
}

enum class FoliageCullMode : uint8_t {
    None, Distance, Frustum, DistanceAndFrustum, Impostor
};

inline const char* foliageCullModeName(FoliageCullMode c) {
    switch (c) {
        case FoliageCullMode::None:                return "None";
        case FoliageCullMode::Distance:            return "Distance";
        case FoliageCullMode::Frustum:             return "Frustum";
        case FoliageCullMode::DistanceAndFrustum:  return "DistanceAndFrustum";
        case FoliageCullMode::Impostor:            return "Impostor";
    }
    return "Unknown";
}

enum class FoliageAlignMode : uint8_t {
    WorldUp, SurfaceNormal, Random, Custom
};

inline const char* foliageAlignModeName(FoliageAlignMode a) {
    switch (a) {
        case FoliageAlignMode::WorldUp:       return "WorldUp";
        case FoliageAlignMode::SurfaceNormal: return "SurfaceNormal";
        case FoliageAlignMode::Random:        return "Random";
        case FoliageAlignMode::Custom:        return "Custom";
    }
    return "Unknown";
}

class FoliageType {
public:
    explicit FoliageType(const std::string& name)
        : m_name(name) {}

    void setDensity(float d)              { m_density    = d; }
    void setScaleMin(float s)             { m_scaleMin   = s; }
    void setScaleMax(float s)             { m_scaleMax   = s; }
    void setCullMode(FoliageCullMode c)   { m_cullMode   = c; }
    void setAlignMode(FoliageAlignMode a) { m_alignMode  = a; }
    void setCullDistance(float d)         { m_cullDist   = d; }
    void setEnabled(bool v)               { m_enabled    = v; }
    void setCastsShadow(bool v)           { m_castsShadow = v; }

    [[nodiscard]] const std::string& name()         const { return m_name;        }
    [[nodiscard]] float              density()      const { return m_density;     }
    [[nodiscard]] float              scaleMin()     const { return m_scaleMin;    }
    [[nodiscard]] float              scaleMax()     const { return m_scaleMax;    }
    [[nodiscard]] FoliageCullMode    cullMode()     const { return m_cullMode;    }
    [[nodiscard]] FoliageAlignMode   alignMode()    const { return m_alignMode;   }
    [[nodiscard]] float              cullDistance() const { return m_cullDist;    }
    [[nodiscard]] bool               isEnabled()    const { return m_enabled;     }
    [[nodiscard]] bool               castsShadow()  const { return m_castsShadow; }

private:
    std::string       m_name;
    FoliageCullMode   m_cullMode    = FoliageCullMode::DistanceAndFrustum;
    FoliageAlignMode  m_alignMode   = FoliageAlignMode::SurfaceNormal;
    float             m_density     = 1.0f;
    float             m_scaleMin    = 0.8f;
    float             m_scaleMax    = 1.2f;
    float             m_cullDist    = 200.0f;
    bool              m_enabled     = true;
    bool              m_castsShadow = true;
};

class FoliagePainter {
public:
    static constexpr size_t MAX_TYPES = 64;

    [[nodiscard]] bool addType(const FoliageType& type) {
        for (auto& t : m_types) if (t.name() == type.name()) return false;
        if (m_types.size() >= MAX_TYPES) return false;
        m_types.push_back(type);
        return true;
    }

    [[nodiscard]] bool removeType(const std::string& name) {
        for (auto it = m_types.begin(); it != m_types.end(); ++it) {
            if (it->name() == name) {
                if (m_activeType == name) m_activeType.clear();
                m_types.erase(it);
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] FoliageType* findType(const std::string& name) {
        for (auto& t : m_types) if (t.name() == name) return &t;
        return nullptr;
    }

    [[nodiscard]] bool setActiveType(const std::string& name) {
        for (auto& t : m_types) if (t.name() == name) { m_activeType = name; return true; }
        return false;
    }

    void setPlacementMode(FoliagePlacementMode m) { m_placementMode = m; }
    void setBrushRadius(float r)                  { m_brushRadius   = r; }

    [[nodiscard]] FoliagePlacementMode placementMode() const { return m_placementMode; }
    [[nodiscard]] float                brushRadius()   const { return m_brushRadius;   }
    [[nodiscard]] const std::string&   activeType()    const { return m_activeType;    }
    [[nodiscard]] size_t               typeCount()     const { return m_types.size();  }

    [[nodiscard]] size_t enabledCount() const {
        size_t c = 0; for (auto& t : m_types) if (t.isEnabled()) ++c; return c;
    }
    [[nodiscard]] size_t shadowCastingCount() const {
        size_t c = 0; for (auto& t : m_types) if (t.castsShadow()) ++c; return c;
    }
    [[nodiscard]] size_t countByCullMode(FoliageCullMode c) const {
        size_t n = 0; for (auto& t : m_types) if (t.cullMode() == c) ++n; return n;
    }

private:
    std::vector<FoliageType>  m_types;
    std::string               m_activeType;
    FoliagePlacementMode      m_placementMode = FoliagePlacementMode::Scatter;
    float                     m_brushRadius   = 50.0f;
};

} // namespace NF
