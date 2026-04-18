#pragma once
// NF::Editor — Collider editor v1: collision shape authoring with layer masks
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Cov1ColliderShape : uint8_t { Box, Sphere, Capsule, Cylinder, Mesh, Convex };
enum class Cov1ColliderMode  : uint8_t { Solid, Trigger, QueryOnly };

struct Cov1Extents {
    float x = 0.5f;
    float y = 0.5f;
    float z = 0.5f;
};

struct Cov1Collider {
    uint64_t         id        = 0;
    std::string      label;
    Cov1ColliderShape shape    = Cov1ColliderShape::Box;
    Cov1ColliderMode  mode     = Cov1ColliderMode::Solid;
    Cov1Extents       extents;
    uint32_t          layer    = 0;
    uint32_t          mask     = 0xFFFFFFFF;
    bool              enabled  = true;
    [[nodiscard]] bool isValid() const { return id != 0 && !label.empty(); }
};

using Cov1ChangeCallback = std::function<void(uint64_t)>;

class ColliderEditorV1 {
public:
    bool addCollider(const Cov1Collider& c) {
        if (!c.isValid()) return false;
        for (const auto& ec : m_colliders) if (ec.id == c.id) return false;
        m_colliders.push_back(c);
        return true;
    }

    bool removeCollider(uint64_t id) {
        for (auto it = m_colliders.begin(); it != m_colliders.end(); ++it) {
            if (it->id == id) { m_colliders.erase(it); return true; }
        }
        return false;
    }

    bool setShape(uint64_t id, Cov1ColliderShape shape) {
        for (auto& c : m_colliders) {
            if (c.id == id) { c.shape = shape; if (m_onChange) m_onChange(id); return true; }
        }
        return false;
    }

    bool setMode(uint64_t id, Cov1ColliderMode mode) {
        for (auto& c : m_colliders) {
            if (c.id == id) { c.mode = mode; if (m_onChange) m_onChange(id); return true; }
        }
        return false;
    }

    bool setLayer(uint64_t id, uint32_t layer, uint32_t mask) {
        for (auto& c : m_colliders) {
            if (c.id == id) { c.layer = layer; c.mask = mask; if (m_onChange) m_onChange(id); return true; }
        }
        return false;
    }

    bool enableCollider(uint64_t id, bool enable) {
        for (auto& c : m_colliders) {
            if (c.id == id) { c.enabled = enable; if (m_onChange) m_onChange(id); return true; }
        }
        return false;
    }

    [[nodiscard]] size_t colliderCount() const { return m_colliders.size(); }

    [[nodiscard]] const Cov1Collider* findCollider(uint64_t id) const {
        for (const auto& c : m_colliders) if (c.id == id) return &c;
        return nullptr;
    }

    void setOnChange(Cov1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Cov1Collider> m_colliders;
    Cov1ChangeCallback        m_onChange;
};

} // namespace NF
