#pragma once
// NF::Editor — Physics joint editor v1: constraint and joint authoring
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Pjv1JointType : uint8_t { Hinge, Ball, Slider, Fixed, Spring, Distance };
enum class Pjv1JointState : uint8_t { Active, Disabled, Broken, Locked };

inline const char* pjv1JointTypeName(Pjv1JointType t) {
    switch (t) {
        case Pjv1JointType::Hinge:    return "Hinge";
        case Pjv1JointType::Ball:     return "Ball";
        case Pjv1JointType::Slider:   return "Slider";
        case Pjv1JointType::Fixed:    return "Fixed";
        case Pjv1JointType::Spring:   return "Spring";
        case Pjv1JointType::Distance: return "Distance";
    }
    return "Unknown";
}

inline const char* pjv1JointStateName(Pjv1JointState s) {
    switch (s) {
        case Pjv1JointState::Active:   return "Active";
        case Pjv1JointState::Disabled: return "Disabled";
        case Pjv1JointState::Broken:   return "Broken";
        case Pjv1JointState::Locked:   return "Locked";
    }
    return "Unknown";
}

struct Pjv1Joint {
    uint64_t       id          = 0;
    std::string    name;
    Pjv1JointType  type        = Pjv1JointType::Hinge;
    Pjv1JointState state       = Pjv1JointState::Active;
    uint64_t       bodyA       = 0;
    uint64_t       bodyB       = 0;
    float          breakForce  = -1.f; // negative = unbreakable
    float          damping     = 0.5f;
    float          stiffness   = 1.f;

    [[nodiscard]] bool isValid()       const { return id != 0 && !name.empty() && bodyA != 0 && bodyB != 0; }
    [[nodiscard]] bool isBreakable()   const { return breakForce >= 0.f; }
    [[nodiscard]] bool isActive()      const { return state == Pjv1JointState::Active; }
};

using Pjv1ChangeCallback = std::function<void(uint64_t)>;

class PhysicsJointEditorV1 {
public:
    static constexpr size_t MAX_JOINTS = 256;

    bool addJoint(const Pjv1Joint& joint) {
        if (!joint.isValid()) return false;
        for (const auto& j : m_joints) if (j.id == joint.id) return false;
        if (m_joints.size() >= MAX_JOINTS) return false;
        m_joints.push_back(joint);
        if (m_onChange) m_onChange(joint.id);
        return true;
    }

    bool removeJoint(uint64_t id) {
        for (auto it = m_joints.begin(); it != m_joints.end(); ++it) {
            if (it->id == id) { m_joints.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] Pjv1Joint* findJoint(uint64_t id) {
        for (auto& j : m_joints) if (j.id == id) return &j;
        return nullptr;
    }

    bool setState(uint64_t id, Pjv1JointState state) {
        auto* j = findJoint(id);
        if (!j) return false;
        j->state = state;
        if (m_onChange) m_onChange(id);
        return true;
    }

    bool setBreakForce(uint64_t id, float force) {
        auto* j = findJoint(id);
        if (!j) return false;
        j->breakForce = force;
        if (m_onChange) m_onChange(id);
        return true;
    }

    [[nodiscard]] size_t jointCount() const { return m_joints.size(); }

    [[nodiscard]] size_t activeCount() const {
        size_t c = 0;
        for (const auto& j : m_joints) if (j.isActive()) ++c;
        return c;
    }

    [[nodiscard]] size_t countByType(Pjv1JointType type) const {
        size_t c = 0;
        for (const auto& j : m_joints) if (j.type == type) ++c;
        return c;
    }

    [[nodiscard]] size_t breakableCount() const {
        size_t c = 0;
        for (const auto& j : m_joints) if (j.isBreakable()) ++c;
        return c;
    }

    void setOnChange(Pjv1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Pjv1Joint> m_joints;
    Pjv1ChangeCallback     m_onChange;
};

} // namespace NF
