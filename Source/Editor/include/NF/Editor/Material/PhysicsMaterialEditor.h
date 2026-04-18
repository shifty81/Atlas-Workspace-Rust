#pragma once
// NF::Editor — physics surface material property management
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
#include <string>
#include <vector>
#include <cstdint>

namespace NF {

enum class PhysMatSurface : uint8_t { Default, Metal, Wood, Concrete, Ice, Rubber };
inline const char* physMatSurfaceName(PhysMatSurface v) {
    switch (v) {
        case PhysMatSurface::Default:  return "Default";
        case PhysMatSurface::Metal:    return "Metal";
        case PhysMatSurface::Wood:     return "Wood";
        case PhysMatSurface::Concrete: return "Concrete";
        case PhysMatSurface::Ice:      return "Ice";
        case PhysMatSurface::Rubber:   return "Rubber";
    }
    return "Unknown";
}

enum class PhysMatCombine : uint8_t { Average, Minimum, Maximum, Multiply };
inline const char* physMatCombineName(PhysMatCombine v) {
    switch (v) {
        case PhysMatCombine::Average:  return "Average";
        case PhysMatCombine::Minimum:  return "Minimum";
        case PhysMatCombine::Maximum:  return "Maximum";
        case PhysMatCombine::Multiply: return "Multiply";
    }
    return "Unknown";
}

class PhysicsMaterial {
public:
    explicit PhysicsMaterial(uint32_t id, const std::string& name,
                              PhysMatSurface surface, PhysMatCombine combine)
        : m_id(id), m_name(name), m_surface(surface), m_combine(combine) {}

    void setFriction(float v)    { m_friction    = v; }
    void setRestitution(float v) { m_restitution = v; }
    void setDensity(float v)     { m_density     = v; }
    void setIsEnabled(bool v)    { m_isEnabled   = v; }

    [[nodiscard]] uint32_t           id()          const { return m_id;          }
    [[nodiscard]] const std::string& name()        const { return m_name;        }
    [[nodiscard]] PhysMatSurface     surface()     const { return m_surface;     }
    [[nodiscard]] PhysMatCombine     combine()     const { return m_combine;     }
    [[nodiscard]] float              friction()    const { return m_friction;    }
    [[nodiscard]] float              restitution() const { return m_restitution; }
    [[nodiscard]] float              density()     const { return m_density;     }
    [[nodiscard]] bool               isEnabled()   const { return m_isEnabled;   }

private:
    uint32_t      m_id;
    std::string   m_name;
    PhysMatSurface m_surface;
    PhysMatCombine m_combine;
    float         m_friction    = 0.5f;
    float         m_restitution = 0.3f;
    float         m_density     = 1.0f;
    bool          m_isEnabled   = true;
};

class PhysicsMaterialEditor {
public:
    void setIsShowDisabled(bool v)    { m_isShowDisabled    = v; }
    void setIsGroupBySurface(bool v)  { m_isGroupBySurface  = v; }
    void setDefaultFriction(float v)  { m_defaultFriction   = v; }

    bool addMaterial(const PhysicsMaterial& m) {
        for (auto& x : m_materials) if (x.id() == m.id()) return false;
        m_materials.push_back(m); return true;
    }
    bool removeMaterial(uint32_t id) {
        auto it = std::find_if(m_materials.begin(), m_materials.end(),
            [&](const PhysicsMaterial& m){ return m.id() == id; });
        if (it == m_materials.end()) return false;
        m_materials.erase(it); return true;
    }
    [[nodiscard]] PhysicsMaterial* findMaterial(uint32_t id) {
        for (auto& m : m_materials) if (m.id() == id) return &m;
        return nullptr;
    }

    [[nodiscard]] bool   isShowDisabled()   const { return m_isShowDisabled;   }
    [[nodiscard]] bool   isGroupBySurface() const { return m_isGroupBySurface; }
    [[nodiscard]] float  defaultFriction()  const { return m_defaultFriction;  }
    [[nodiscard]] size_t materialCount()    const { return m_materials.size(); }

    [[nodiscard]] size_t countBySurface(PhysMatSurface s) const {
        size_t n = 0; for (auto& x : m_materials) if (x.surface() == s) ++n; return n;
    }
    [[nodiscard]] size_t countByCombine(PhysMatCombine c) const {
        size_t n = 0; for (auto& x : m_materials) if (x.combine() == c) ++n; return n;
    }
    [[nodiscard]] size_t countEnabled() const {
        size_t n = 0; for (auto& x : m_materials) if (x.isEnabled()) ++n; return n;
    }

private:
    std::vector<PhysicsMaterial> m_materials;
    bool  m_isShowDisabled   = false;
    bool  m_isGroupBySurface = true;
    float m_defaultFriction  = 0.6f;
};

} // namespace NF
