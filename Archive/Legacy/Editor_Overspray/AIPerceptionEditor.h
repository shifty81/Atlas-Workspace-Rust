#pragma once
// NF::Editor — AI perception sensor configuration management
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

enum class AiSensorType : uint8_t { Sight, Hearing, Touch, Smell, Custom };
inline const char* aiSensorTypeName(AiSensorType v) {
    switch (v) {
        case AiSensorType::Sight:   return "Sight";
        case AiSensorType::Hearing: return "Hearing";
        case AiSensorType::Touch:   return "Touch";
        case AiSensorType::Smell:   return "Smell";
        case AiSensorType::Custom:  return "Custom";
    }
    return "Unknown";
}

enum class AiSensorShape : uint8_t { Cone, Sphere, Box, Capsule, Frustum };
inline const char* aiSensorShapeName(AiSensorShape v) {
    switch (v) {
        case AiSensorShape::Cone:    return "Cone";
        case AiSensorShape::Sphere:  return "Sphere";
        case AiSensorShape::Box:     return "Box";
        case AiSensorShape::Capsule: return "Capsule";
        case AiSensorShape::Frustum: return "Frustum";
    }
    return "Unknown";
}

class AiPerceptionConfig {
public:
    explicit AiPerceptionConfig(uint32_t id, const std::string& name,
                                 AiSensorType sensorType, AiSensorShape sensorShape)
        : m_id(id), m_name(name), m_sensorType(sensorType), m_sensorShape(sensorShape) {}

    void setRange(float v)        { m_range       = v; }
    void setFieldOfView(float v)  { m_fieldOfView  = v; }
    void setIsEnabled(bool v)     { m_isEnabled    = v; }

    [[nodiscard]] uint32_t           id()          const { return m_id;          }
    [[nodiscard]] const std::string& name()        const { return m_name;        }
    [[nodiscard]] AiSensorType       sensorType()  const { return m_sensorType;  }
    [[nodiscard]] AiSensorShape      sensorShape() const { return m_sensorShape; }
    [[nodiscard]] float              range()       const { return m_range;       }
    [[nodiscard]] float              fieldOfView() const { return m_fieldOfView; }
    [[nodiscard]] bool               isEnabled()   const { return m_isEnabled;   }

private:
    uint32_t      m_id;
    std::string   m_name;
    AiSensorType  m_sensorType;
    AiSensorShape m_sensorShape;
    float         m_range       = 10.0f;
    float         m_fieldOfView = 90.0f;
    bool          m_isEnabled   = true;
};

class AIPerceptionEditor {
public:
    void setIsShowDisabled(bool v)    { m_isShowDisabled      = v; }
    void setIsGroupBySensorType(bool v) { m_isGroupBySensorType = v; }
    void setDefaultRange(float v)     { m_defaultRange        = v; }

    bool addConfig(const AiPerceptionConfig& c) {
        for (auto& x : m_configs) if (x.id() == c.id()) return false;
        m_configs.push_back(c); return true;
    }
    bool removeConfig(uint32_t id) {
        auto it = std::find_if(m_configs.begin(), m_configs.end(),
            [&](const AiPerceptionConfig& c){ return c.id() == id; });
        if (it == m_configs.end()) return false;
        m_configs.erase(it); return true;
    }
    [[nodiscard]] AiPerceptionConfig* findConfig(uint32_t id) {
        for (auto& c : m_configs) if (c.id() == id) return &c;
        return nullptr;
    }

    [[nodiscard]] bool   isShowDisabled()      const { return m_isShowDisabled;      }
    [[nodiscard]] bool   isGroupBySensorType() const { return m_isGroupBySensorType; }
    [[nodiscard]] float  defaultRange()        const { return m_defaultRange;        }
    [[nodiscard]] size_t configCount()         const { return m_configs.size();      }

    [[nodiscard]] size_t countBySensorType(AiSensorType t) const {
        size_t n = 0; for (auto& x : m_configs) if (x.sensorType() == t) ++n; return n;
    }
    [[nodiscard]] size_t countBySensorShape(AiSensorShape s) const {
        size_t n = 0; for (auto& x : m_configs) if (x.sensorShape() == s) ++n; return n;
    }
    [[nodiscard]] size_t countEnabled() const {
        size_t n = 0; for (auto& x : m_configs) if (x.isEnabled()) ++n; return n;
    }

private:
    std::vector<AiPerceptionConfig> m_configs;
    bool  m_isShowDisabled      = false;
    bool  m_isGroupBySensorType = true;
    float m_defaultRange        = 15.0f;
};

} // namespace NF
