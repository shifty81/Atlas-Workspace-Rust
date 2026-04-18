#pragma once
// NF::Editor — Morph target editor v1: morph target and blend shape management
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Mtev1TargetCategory : uint8_t { Facial, Body, Corrective, Custom };
enum class Mtev1TargetState    : uint8_t { Inactive, Active, Preview, Locked };

inline const char* mtev1TargetCategoryName(Mtev1TargetCategory c) {
    switch (c) {
        case Mtev1TargetCategory::Facial:     return "Facial";
        case Mtev1TargetCategory::Body:       return "Body";
        case Mtev1TargetCategory::Corrective: return "Corrective";
        case Mtev1TargetCategory::Custom:     return "Custom";
    }
    return "Unknown";
}

inline const char* mtev1TargetStateName(Mtev1TargetState s) {
    switch (s) {
        case Mtev1TargetState::Inactive: return "Inactive";
        case Mtev1TargetState::Active:   return "Active";
        case Mtev1TargetState::Preview:  return "Preview";
        case Mtev1TargetState::Locked:   return "Locked";
    }
    return "Unknown";
}

struct Mtev1Target {
    uint64_t              id       = 0;
    std::string           name;
    Mtev1TargetCategory   category = Mtev1TargetCategory::Custom;
    Mtev1TargetState      state    = Mtev1TargetState::Inactive;
    float                 weight   = 0.0f;

    [[nodiscard]] bool isValid()   const { return id != 0 && !name.empty(); }
    [[nodiscard]] bool isActive()  const { return state == Mtev1TargetState::Active; }
    [[nodiscard]] bool isPreview() const { return state == Mtev1TargetState::Preview; }
    [[nodiscard]] bool isLocked()  const { return state == Mtev1TargetState::Locked; }
};

struct Mtev1BlendShape {
    uint64_t    id   = 0;
    std::string name;
    float       value = 0.0f;

    [[nodiscard]] bool isValid() const { return id != 0 && !name.empty(); }
};

using Mtev1ChangeCallback = std::function<void(uint64_t)>;

class MorphTargetEditorV1 {
public:
    static constexpr size_t MAX_TARGETS     = 512;
    static constexpr size_t MAX_BLENDSHAPES = 256;

    bool addTarget(const Mtev1Target& target) {
        if (!target.isValid()) return false;
        for (const auto& t : m_targets) if (t.id == target.id) return false;
        if (m_targets.size() >= MAX_TARGETS) return false;
        m_targets.push_back(target);
        if (m_onChange) m_onChange(target.id);
        return true;
    }

    bool removeTarget(uint64_t id) {
        for (auto it = m_targets.begin(); it != m_targets.end(); ++it) {
            if (it->id == id) { m_targets.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] Mtev1Target* findTarget(uint64_t id) {
        for (auto& t : m_targets) if (t.id == id) return &t;
        return nullptr;
    }

    bool addBlendShape(const Mtev1BlendShape& shape) {
        if (!shape.isValid()) return false;
        for (const auto& s : m_blendShapes) if (s.id == shape.id) return false;
        if (m_blendShapes.size() >= MAX_BLENDSHAPES) return false;
        m_blendShapes.push_back(shape);
        if (m_onChange) m_onChange(shape.id);
        return true;
    }

    bool removeBlendShape(uint64_t id) {
        for (auto it = m_blendShapes.begin(); it != m_blendShapes.end(); ++it) {
            if (it->id == id) { m_blendShapes.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] size_t targetCount()     const { return m_targets.size(); }
    [[nodiscard]] size_t blendShapeCount() const { return m_blendShapes.size(); }

    [[nodiscard]] size_t activeCount() const {
        size_t c = 0; for (const auto& t : m_targets) if (t.isActive()) ++c; return c;
    }
    [[nodiscard]] size_t lockedCount() const {
        size_t c = 0; for (const auto& t : m_targets) if (t.isLocked()) ++c; return c;
    }
    [[nodiscard]] size_t countByCategory(Mtev1TargetCategory cat) const {
        size_t c = 0; for (const auto& t : m_targets) if (t.category == cat) ++c; return c;
    }

    void setOnChange(Mtev1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Mtev1Target>     m_targets;
    std::vector<Mtev1BlendShape> m_blendShapes;
    Mtev1ChangeCallback          m_onChange;
};

} // namespace NF
