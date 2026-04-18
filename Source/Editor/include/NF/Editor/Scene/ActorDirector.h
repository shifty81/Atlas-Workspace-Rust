#pragma once
// NF::Editor — Actor director: per-actor sequencer bindings
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

enum class ActorBindingType : uint8_t {
    Transform, Animation, Visibility, Material, Physics, Script
};

inline const char* actorBindingTypeName(ActorBindingType t) {
    switch (t) {
        case ActorBindingType::Transform:  return "Transform";
        case ActorBindingType::Animation:  return "Animation";
        case ActorBindingType::Visibility: return "Visibility";
        case ActorBindingType::Material:   return "Material";
        case ActorBindingType::Physics:    return "Physics";
        case ActorBindingType::Script:     return "Script";
    }
    return "Unknown";
}

enum class ActorDirectorMode : uint8_t {
    Edit, Preview, Record, Bake
};

inline const char* actorDirectorModeName(ActorDirectorMode m) {
    switch (m) {
        case ActorDirectorMode::Edit:    return "Edit";
        case ActorDirectorMode::Preview: return "Preview";
        case ActorDirectorMode::Record:  return "Record";
        case ActorDirectorMode::Bake:    return "Bake";
    }
    return "Unknown";
}

enum class ActorDirectorPoseMode : uint8_t {
    Authored, Override, Additive, Reset
};

inline const char* actorDirectorPoseModeName(ActorDirectorPoseMode m) {
    switch (m) {
        case ActorDirectorPoseMode::Authored:  return "Authored";
        case ActorDirectorPoseMode::Override:  return "Override";
        case ActorDirectorPoseMode::Additive:  return "Additive";
        case ActorDirectorPoseMode::Reset:     return "Reset";
    }
    return "Unknown";
}

class ActorBinding {
public:
    explicit ActorBinding(const std::string& actorName, ActorBindingType type)
        : m_actorName(actorName), m_type(type) {}

    void setEnabled(bool v)   { m_enabled  = v; }
    void setWeight(float w)   { m_weight   = w; }
    void setKeyCount(uint32_t n) { m_keyCount = n; }

    [[nodiscard]] const std::string& actorName() const { return m_actorName; }
    [[nodiscard]] ActorBindingType   type()      const { return m_type;      }
    [[nodiscard]] bool               isEnabled() const { return m_enabled;   }
    [[nodiscard]] float              weight()    const { return m_weight;    }
    [[nodiscard]] uint32_t           keyCount()  const { return m_keyCount;  }

private:
    std::string      m_actorName;
    ActorBindingType m_type;
    float            m_weight   = 1.0f;
    uint32_t         m_keyCount = 0;
    bool             m_enabled  = true;
};

class ActorDirector {
public:
    static constexpr size_t MAX_BINDINGS = 256;

    [[nodiscard]] bool addBinding(const ActorBinding& binding) {
        for (auto& b : m_bindings)
            if (b.actorName() == binding.actorName() && b.type() == binding.type())
                return false;
        if (m_bindings.size() >= MAX_BINDINGS) return false;
        m_bindings.push_back(binding);
        return true;
    }

    [[nodiscard]] bool removeBinding(const std::string& actorName, ActorBindingType type) {
        for (auto it = m_bindings.begin(); it != m_bindings.end(); ++it) {
            if (it->actorName() == actorName && it->type() == type) {
                m_bindings.erase(it); return true;
            }
        }
        return false;
    }

    [[nodiscard]] ActorBinding* findBinding(const std::string& actorName, ActorBindingType type) {
        for (auto& b : m_bindings)
            if (b.actorName() == actorName && b.type() == type) return &b;
        return nullptr;
    }

    void setMode(ActorDirectorMode m)           { m_mode     = m; }
    void setPoseMode(ActorDirectorPoseMode m)   { m_poseMode = m; }

    [[nodiscard]] ActorDirectorMode     mode()         const { return m_mode;     }
    [[nodiscard]] ActorDirectorPoseMode poseMode()     const { return m_poseMode; }
    [[nodiscard]] size_t                bindingCount() const { return m_bindings.size(); }

    [[nodiscard]] size_t enabledBindingCount() const {
        size_t c = 0; for (auto& b : m_bindings) if (b.isEnabled()) ++c; return c;
    }
    [[nodiscard]] size_t countByBindingType(ActorBindingType t) const {
        size_t c = 0; for (auto& b : m_bindings) if (b.type() == t) ++c; return c;
    }
    [[nodiscard]] size_t uniqueActorCount() const {
        std::set<std::string> actors;
        for (auto& b : m_bindings) actors.insert(b.actorName());
        return actors.size();
    }

private:
    std::vector<ActorBinding>  m_bindings;
    ActorDirectorMode          m_mode     = ActorDirectorMode::Edit;
    ActorDirectorPoseMode      m_poseMode = ActorDirectorPoseMode::Authored;
};

} // namespace NF
