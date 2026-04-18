#pragma once
// NF::Editor — Rigid body editor v1: mass, drag and physics material authoring
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Rbv1BodyType    : uint8_t { Dynamic, Kinematic, Static };
enum class Rbv1LockAxis    : uint8_t { None, X, Y, Z, XY, XZ, YZ, XYZ };

struct Rbv1PhysicsMat {
    float friction    = 0.5f;
    float restitution = 0.3f;
    float density     = 1.f;
};

struct Rbv1Body {
    uint64_t        id            = 0;
    std::string     label;
    Rbv1BodyType    type          = Rbv1BodyType::Dynamic;
    float           mass          = 1.f;
    float           linearDrag    = 0.f;
    float           angularDrag   = 0.05f;
    bool            useGravity    = true;
    Rbv1LockAxis    positionLock  = Rbv1LockAxis::None;
    Rbv1LockAxis    rotationLock  = Rbv1LockAxis::None;
    Rbv1PhysicsMat  material;
    [[nodiscard]] bool isValid() const { return id != 0 && !label.empty() && mass > 0.f; }
};

using Rbv1ChangeCallback = std::function<void(uint64_t)>;

class RigidBodyEditorV1 {
public:
    bool addBody(const Rbv1Body& b) {
        if (!b.isValid()) return false;
        for (const auto& eb : m_bodies) if (eb.id == b.id) return false;
        m_bodies.push_back(b);
        return true;
    }

    bool removeBody(uint64_t id) {
        for (auto it = m_bodies.begin(); it != m_bodies.end(); ++it) {
            if (it->id == id) { m_bodies.erase(it); return true; }
        }
        return false;
    }

    bool setMass(uint64_t id, float mass) {
        if (mass <= 0.f) return false;
        for (auto& b : m_bodies) {
            if (b.id == id) { b.mass = mass; if (m_onChange) m_onChange(id); return true; }
        }
        return false;
    }

    bool setBodyType(uint64_t id, Rbv1BodyType type) {
        for (auto& b : m_bodies) {
            if (b.id == id) { b.type = type; if (m_onChange) m_onChange(id); return true; }
        }
        return false;
    }

    bool setMaterial(uint64_t id, const Rbv1PhysicsMat& mat) {
        for (auto& b : m_bodies) {
            if (b.id == id) { b.material = mat; if (m_onChange) m_onChange(id); return true; }
        }
        return false;
    }

    [[nodiscard]] size_t bodyCount() const { return m_bodies.size(); }

    [[nodiscard]] const Rbv1Body* findBody(uint64_t id) const {
        for (const auto& b : m_bodies) if (b.id == id) return &b;
        return nullptr;
    }

    void setOnChange(Rbv1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Rbv1Body>  m_bodies;
    Rbv1ChangeCallback     m_onChange;
};

} // namespace NF
