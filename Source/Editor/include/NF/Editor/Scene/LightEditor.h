#pragma once
// NF::Editor — Light placement and property editor
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

enum class EditorLightType : uint8_t {
    Directional, Point, Spot, Rect, Sky
};

inline const char* editorLightTypeName(EditorLightType t) {
    switch (t) {
        case EditorLightType::Directional: return "Directional";
        case EditorLightType::Point:       return "Point";
        case EditorLightType::Spot:        return "Spot";
        case EditorLightType::Rect:        return "Rect";
        case EditorLightType::Sky:         return "Sky";
    }
    return "Unknown";
}

enum class EditorLightMobility : uint8_t {
    Static, Stationary, Dynamic
};

inline const char* editorLightMobilityName(EditorLightMobility m) {
    switch (m) {
        case EditorLightMobility::Static:      return "Static";
        case EditorLightMobility::Stationary:  return "Stationary";
        case EditorLightMobility::Dynamic:     return "Dynamic";
    }
    return "Unknown";
}

enum class EditorLightShadowMode : uint8_t {
    None, Hard, Soft, RayTraced, Contact
};

inline const char* editorLightShadowModeName(EditorLightShadowMode m) {
    switch (m) {
        case EditorLightShadowMode::None:      return "None";
        case EditorLightShadowMode::Hard:      return "Hard";
        case EditorLightShadowMode::Soft:      return "Soft";
        case EditorLightShadowMode::RayTraced: return "RayTraced";
        case EditorLightShadowMode::Contact:   return "Contact";
    }
    return "Unknown";
}

class LightInstance {
public:
    explicit LightInstance(const std::string& name, EditorLightType type)
        : m_name(name), m_type(type) {}

    void setMobility(EditorLightMobility m)   { m_mobility   = m; }
    void setShadowMode(EditorLightShadowMode s){ m_shadowMode = s; }
    void setIntensity(float i)          { m_intensity  = i; }
    void setRange(float r)              { m_range      = r; }
    void setInnerAngle(float a)         { m_innerAngle = a; }
    void setOuterAngle(float a)         { m_outerAngle = a; }
    void setEnabled(bool v)             { m_enabled    = v; }
    void setCastsShadow(bool v)         { m_castsShadow = v; }

    [[nodiscard]] const std::string&   name()         const { return m_name;        }
    [[nodiscard]] EditorLightType      type()         const { return m_type;        }
    [[nodiscard]] EditorLightMobility  mobility()     const { return m_mobility;    }
    [[nodiscard]] EditorLightShadowMode shadowMode()  const { return m_shadowMode;  }
    [[nodiscard]] float                intensity()    const { return m_intensity;   }
    [[nodiscard]] float                range()        const { return m_range;       }
    [[nodiscard]] float                innerAngle()   const { return m_innerAngle;  }
    [[nodiscard]] float                outerAngle()   const { return m_outerAngle;  }
    [[nodiscard]] bool                 isEnabled()    const { return m_enabled;     }
    [[nodiscard]] bool                 castsShadow()  const { return m_castsShadow; }

private:
    std::string          m_name;
    EditorLightType      m_type;
    EditorLightMobility  m_mobility    = EditorLightMobility::Dynamic;
    EditorLightShadowMode m_shadowMode = EditorLightShadowMode::Soft;
    float                m_intensity   = 1.0f;
    float                m_range       = 10.0f;
    float                m_innerAngle  = 20.0f;
    float                m_outerAngle  = 45.0f;
    bool                 m_enabled     = true;
    bool                 m_castsShadow = true;
};

class LightEditor {
public:
    static constexpr size_t MAX_LIGHTS = 512;

    [[nodiscard]] bool addLight(const LightInstance& light) {
        for (auto& l : m_lights) if (l.name() == light.name()) return false;
        if (m_lights.size() >= MAX_LIGHTS) return false;
        m_lights.push_back(light);
        return true;
    }

    [[nodiscard]] bool removeLight(const std::string& name) {
        for (auto it = m_lights.begin(); it != m_lights.end(); ++it) {
            if (it->name() == name) {
                if (m_activeLight == name) m_activeLight.clear();
                m_lights.erase(it);
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] LightInstance* findLight(const std::string& name) {
        for (auto& l : m_lights) if (l.name() == name) return &l;
        return nullptr;
    }

    [[nodiscard]] bool setActiveLight(const std::string& name) {
        for (auto& l : m_lights) if (l.name() == name) { m_activeLight = name; return true; }
        return false;
    }

    [[nodiscard]] const std::string& activeLight() const { return m_activeLight; }
    [[nodiscard]] size_t lightCount()     const { return m_lights.size(); }
    [[nodiscard]] size_t shadowCastCount()const {
        size_t c = 0; for (auto& l : m_lights) if (l.castsShadow()) ++c; return c;
    }
    [[nodiscard]] size_t countByType(EditorLightType t) const {
        size_t c = 0; for (auto& l : m_lights) if (l.type() == t) ++c; return c;
    }
    [[nodiscard]] size_t countByMobility(EditorLightMobility m) const {
        size_t c = 0; for (auto& l : m_lights) if (l.mobility() == m) ++c; return c;
    }

private:
    std::vector<LightInstance> m_lights;
    std::string                m_activeLight;
};

} // namespace NF
