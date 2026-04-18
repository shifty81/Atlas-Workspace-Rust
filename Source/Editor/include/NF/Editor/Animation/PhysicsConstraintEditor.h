#pragma once
// NF::Editor — physics joint/constraint management
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

enum class PhysConstraintType : uint8_t { Fixed, Hinge, Slider, Ball, Spring };
inline const char* physConstraintTypeName(PhysConstraintType v) {
    switch (v) {
        case PhysConstraintType::Fixed:  return "Fixed";
        case PhysConstraintType::Hinge:  return "Hinge";
        case PhysConstraintType::Slider: return "Slider";
        case PhysConstraintType::Ball:   return "Ball";
        case PhysConstraintType::Spring: return "Spring";
    }
    return "Unknown";
}

enum class PhysConstraintAxis : uint8_t { X, Y, Z, XY, XYZ };
inline const char* physConstraintAxisName(PhysConstraintAxis v) {
    switch (v) {
        case PhysConstraintAxis::X:   return "X";
        case PhysConstraintAxis::Y:   return "Y";
        case PhysConstraintAxis::Z:   return "Z";
        case PhysConstraintAxis::XY:  return "XY";
        case PhysConstraintAxis::XYZ: return "XYZ";
    }
    return "Unknown";
}

class PhysicsConstraint {
public:
    explicit PhysicsConstraint(uint32_t id, const std::string& name,
                                PhysConstraintType type, PhysConstraintAxis axis)
        : m_id(id), m_name(name), m_type(type), m_axis(axis) {}

    void setStiffness(float v) { m_stiffness = v; }
    void setDamping(float v)   { m_damping   = v; }
    void setIsEnabled(bool v)  { m_isEnabled  = v; }

    [[nodiscard]] uint32_t             id()         const { return m_id;         }
    [[nodiscard]] const std::string&   name()       const { return m_name;       }
    [[nodiscard]] PhysConstraintType   type()       const { return m_type;       }
    [[nodiscard]] PhysConstraintAxis   axis()       const { return m_axis;       }
    [[nodiscard]] float                stiffness()  const { return m_stiffness;  }
    [[nodiscard]] float                damping()    const { return m_damping;    }
    [[nodiscard]] bool                 isEnabled()  const { return m_isEnabled;  }

private:
    uint32_t           m_id;
    std::string        m_name;
    PhysConstraintType m_type;
    PhysConstraintAxis m_axis;
    float              m_stiffness = 100.0f;
    float              m_damping   = 10.0f;
    bool               m_isEnabled = true;
};

class PhysicsConstraintEditor {
public:
    void setIsShowDisabled(bool v)      { m_isShowDisabled    = v; }
    void setIsGroupByType(bool v)       { m_isGroupByType     = v; }
    void setDefaultStiffness(float v)   { m_defaultStiffness  = v; }

    bool addConstraint(const PhysicsConstraint& c) {
        for (auto& x : m_constraints) if (x.id() == c.id()) return false;
        m_constraints.push_back(c); return true;
    }
    bool removeConstraint(uint32_t id) {
        auto it = std::find_if(m_constraints.begin(), m_constraints.end(),
            [&](const PhysicsConstraint& c){ return c.id() == id; });
        if (it == m_constraints.end()) return false;
        m_constraints.erase(it); return true;
    }
    [[nodiscard]] PhysicsConstraint* findConstraint(uint32_t id) {
        for (auto& c : m_constraints) if (c.id() == id) return &c;
        return nullptr;
    }

    [[nodiscard]] bool   isShowDisabled()   const { return m_isShowDisabled;    }
    [[nodiscard]] bool   isGroupByType()    const { return m_isGroupByType;     }
    [[nodiscard]] float  defaultStiffness() const { return m_defaultStiffness;  }
    [[nodiscard]] size_t constraintCount()  const { return m_constraints.size(); }

    [[nodiscard]] size_t countByType(PhysConstraintType t) const {
        size_t n = 0; for (auto& x : m_constraints) if (x.type() == t) ++n; return n;
    }
    [[nodiscard]] size_t countByAxis(PhysConstraintAxis a) const {
        size_t n = 0; for (auto& x : m_constraints) if (x.axis() == a) ++n; return n;
    }
    [[nodiscard]] size_t countEnabled() const {
        size_t n = 0; for (auto& x : m_constraints) if (x.isEnabled()) ++n; return n;
    }

private:
    std::vector<PhysicsConstraint> m_constraints;
    bool  m_isShowDisabled   = false;
    bool  m_isGroupByType    = true;
    float m_defaultStiffness = 200.0f;
};

} // namespace NF
