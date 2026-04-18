#pragma once
// NF::Editor — physics trigger volume configuration management
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

enum class PhysTriggerShape : uint8_t { Box, Sphere, Capsule, Mesh, Convex };
inline const char* physTriggerShapeName(PhysTriggerShape v) {
    switch (v) {
        case PhysTriggerShape::Box:     return "Box";
        case PhysTriggerShape::Sphere:  return "Sphere";
        case PhysTriggerShape::Capsule: return "Capsule";
        case PhysTriggerShape::Mesh:    return "Mesh";
        case PhysTriggerShape::Convex:  return "Convex";
    }
    return "Unknown";
}

enum class PhysTriggerEvent : uint8_t { OnEnter, OnExit, OnStay, OnOverlap };
inline const char* physTriggerEventName(PhysTriggerEvent v) {
    switch (v) {
        case PhysTriggerEvent::OnEnter:   return "OnEnter";
        case PhysTriggerEvent::OnExit:    return "OnExit";
        case PhysTriggerEvent::OnStay:    return "OnStay";
        case PhysTriggerEvent::OnOverlap: return "OnOverlap";
    }
    return "Unknown";
}

class PhysicsTrigger {
public:
    explicit PhysicsTrigger(uint32_t id, const std::string& name,
                             PhysTriggerShape shape, PhysTriggerEvent event)
        : m_id(id), m_name(name), m_shape(shape), m_event(event) {}

    void setIsContinuous(bool v)     { m_isContinuous = v; }
    void setFilterMask(uint32_t v)   { m_filterMask   = v; }
    void setIsEnabled(bool v)        { m_isEnabled    = v; }

    [[nodiscard]] uint32_t           id()           const { return m_id;           }
    [[nodiscard]] const std::string& name()         const { return m_name;         }
    [[nodiscard]] PhysTriggerShape   shape()        const { return m_shape;        }
    [[nodiscard]] PhysTriggerEvent   event()        const { return m_event;        }
    [[nodiscard]] bool               isContinuous() const { return m_isContinuous; }
    [[nodiscard]] uint32_t           filterMask()   const { return m_filterMask;   }
    [[nodiscard]] bool               isEnabled()    const { return m_isEnabled;    }

private:
    uint32_t         m_id;
    std::string      m_name;
    PhysTriggerShape m_shape;
    PhysTriggerEvent m_event;
    bool             m_isContinuous = false;
    uint32_t         m_filterMask   = 0xFFFFFFFFu;
    bool             m_isEnabled    = true;
};

class PhysicsTriggerEditor {
public:
    void setIsShowDisabled(bool v)        { m_isShowDisabled     = v; }
    void setIsGroupByShape(bool v)        { m_isGroupByShape     = v; }
    void setDefaultFilterMask(uint32_t v) { m_defaultFilterMask  = v; }

    bool addTrigger(const PhysicsTrigger& t) {
        for (auto& x : m_triggers) if (x.id() == t.id()) return false;
        m_triggers.push_back(t); return true;
    }
    bool removeTrigger(uint32_t id) {
        auto it = std::find_if(m_triggers.begin(), m_triggers.end(),
            [&](const PhysicsTrigger& t){ return t.id() == id; });
        if (it == m_triggers.end()) return false;
        m_triggers.erase(it); return true;
    }
    [[nodiscard]] PhysicsTrigger* findTrigger(uint32_t id) {
        for (auto& t : m_triggers) if (t.id() == id) return &t;
        return nullptr;
    }

    [[nodiscard]] bool     isShowDisabled()    const { return m_isShowDisabled;    }
    [[nodiscard]] bool     isGroupByShape()    const { return m_isGroupByShape;    }
    [[nodiscard]] uint32_t defaultFilterMask() const { return m_defaultFilterMask; }
    [[nodiscard]] size_t   triggerCount()      const { return m_triggers.size();   }

    [[nodiscard]] size_t countByShape(PhysTriggerShape s) const {
        size_t n = 0; for (auto& x : m_triggers) if (x.shape() == s) ++n; return n;
    }
    [[nodiscard]] size_t countByEvent(PhysTriggerEvent e) const {
        size_t n = 0; for (auto& x : m_triggers) if (x.event() == e) ++n; return n;
    }
    [[nodiscard]] size_t countEnabled() const {
        size_t n = 0; for (auto& x : m_triggers) if (x.isEnabled()) ++n; return n;
    }

private:
    std::vector<PhysicsTrigger> m_triggers;
    bool     m_isShowDisabled    = false;
    bool     m_isGroupByShape    = false;
    uint32_t m_defaultFilterMask = 0x00000001u;
};

} // namespace NF
