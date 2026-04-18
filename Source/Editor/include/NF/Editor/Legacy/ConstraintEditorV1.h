#pragma once
// NF::Editor — Constraint editor v1: physics joint/constraint authoring
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Cev1ConstraintType : uint8_t { Fixed, Hinge, Slider, Ball, Spring, Distance };
enum class Cev1Axis           : uint8_t { X, Y, Z, XY, XZ, YZ, XYZ };

struct Cev1Limit {
    float lower = 0.f;
    float upper = 0.f;
    bool  enabled = false;
};

struct Cev1Constraint {
    uint64_t         id      = 0;
    std::string      label;
    Cev1ConstraintType type  = Cev1ConstraintType::Fixed;
    uint64_t         bodyA   = 0;
    uint64_t         bodyB   = 0;
    Cev1Axis         axis    = Cev1Axis::X;
    Cev1Limit        linear;
    Cev1Limit        angular;
    bool             enabled = true;
    [[nodiscard]] bool isValid() const { return id != 0 && !label.empty() && bodyA != 0; }
};

using Cev1ChangeCallback = std::function<void(uint64_t)>;

class ConstraintEditorV1 {
public:
    bool addConstraint(const Cev1Constraint& c) {
        if (!c.isValid()) return false;
        for (const auto& ec : m_constraints) if (ec.id == c.id) return false;
        m_constraints.push_back(c);
        return true;
    }

    bool removeConstraint(uint64_t id) {
        for (auto it = m_constraints.begin(); it != m_constraints.end(); ++it) {
            if (it->id == id) { m_constraints.erase(it); return true; }
        }
        return false;
    }

    bool enableConstraint(uint64_t id, bool enable) {
        for (auto& c : m_constraints) {
            if (c.id == id) { c.enabled = enable; if (m_onChange) m_onChange(id); return true; }
        }
        return false;
    }

    bool setLimit(uint64_t id, bool angular, float lower, float upper) {
        for (auto& c : m_constraints) {
            if (c.id == id) {
                auto& lim = angular ? c.angular : c.linear;
                lim.lower = lower; lim.upper = upper; lim.enabled = true;
                if (m_onChange) m_onChange(id);
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] size_t constraintCount() const { return m_constraints.size(); }

    [[nodiscard]] const Cev1Constraint* findConstraint(uint64_t id) const {
        for (const auto& c : m_constraints) if (c.id == id) return &c;
        return nullptr;
    }

    void setOnChange(Cev1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Cev1Constraint> m_constraints;
    Cev1ChangeCallback          m_onChange;
};

} // namespace NF
