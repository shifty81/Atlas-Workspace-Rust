#pragma once
// NF::Editor — Component inspector v1: property-based component editing for entities
#include "NF/Core/Core.h"
#include <cstdint>
#include <string>
#include <vector>

namespace NF {

enum class CiPropType : uint8_t { Int, Float, Bool, String, Vec3, Color, Asset, Enum };

struct CiProperty {
    uint32_t    id           = 0;
    std::string name;
    CiPropType  type         = CiPropType::String;
    std::string value;
    std::string defaultValue;
    bool        readOnly     = false;
    [[nodiscard]] bool isValid()     const { return id != 0 && !name.empty(); }
    [[nodiscard]] bool isAtDefault() const { return value == defaultValue; }
};

struct CiComponent {
    uint32_t               id       = 0;
    std::string            typeName;
    std::vector<CiProperty> properties;
    bool                   enabled  = true;
    [[nodiscard]] bool isValid() const { return id != 0 && !typeName.empty(); }

    CiProperty* findProp(const std::string& name) {
        for (auto& p : properties) if (p.name == name) return &p;
        return nullptr;
    }
    size_t dirtyCount() const {
        size_t n = 0;
        for (const auto& p : properties) if (!p.isAtDefault()) ++n;
        return n;
    }
};

class ComponentInspectorV1 {
public:
    void setTarget(uint32_t entityId) { m_targetEntity = entityId; m_components.clear(); }

    bool addComponent(const CiComponent& comp) {
        if (!comp.isValid()) return false;
        for (const auto& c : m_components) if (c.id == comp.id) return false;
        m_components.push_back(comp);
        return true;
    }

    bool removeComponent(uint32_t id) {
        for (auto it = m_components.begin(); it != m_components.end(); ++it) {
            if (it->id == id) { m_components.erase(it); return true; }
        }
        return false;
    }

    bool setPropertyValue(uint32_t compId, const std::string& propName, const std::string& val) {
        for (auto& c : m_components) {
            if (c.id == compId) {
                CiProperty* p = c.findProp(propName);
                if (!p || p->readOnly) return false;
                p->value = val;
                return true;
            }
        }
        return false;
    }

    const CiProperty* getProperty(uint32_t compId, const std::string& propName) const {
        for (const auto& c : m_components) {
            if (c.id == compId) {
                for (const auto& p : c.properties)
                    if (p.name == propName) return &p;
            }
        }
        return nullptr;
    }

    [[nodiscard]] size_t   componentCount()  const { return m_components.size(); }
    [[nodiscard]] uint32_t targetEntity()    const { return m_targetEntity; }

private:
    std::vector<CiComponent> m_components;
    uint32_t                 m_targetEntity = 0;
};

} // namespace NF
