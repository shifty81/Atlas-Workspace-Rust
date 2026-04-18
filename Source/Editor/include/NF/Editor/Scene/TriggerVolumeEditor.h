#pragma once
// NF::Editor — Trigger volume editor
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

enum class TriggerVolumeShape : uint8_t {
    Box, Sphere, Capsule, Cylinder, Convex
};

inline const char* triggerVolumeShapeName(TriggerVolumeShape s) {
    switch (s) {
        case TriggerVolumeShape::Box:     return "Box";
        case TriggerVolumeShape::Sphere:  return "Sphere";
        case TriggerVolumeShape::Capsule: return "Capsule";
        case TriggerVolumeShape::Cylinder:return "Cylinder";
        case TriggerVolumeShape::Convex:  return "Convex";
    }
    return "Unknown";
}

enum class TriggerVolumeEvent : uint8_t {
    OnEnter, OnExit, OnStay, OnEnterExit
};

inline const char* triggerVolumeEventName(TriggerVolumeEvent e) {
    switch (e) {
        case TriggerVolumeEvent::OnEnter:     return "OnEnter";
        case TriggerVolumeEvent::OnExit:      return "OnExit";
        case TriggerVolumeEvent::OnStay:      return "OnStay";
        case TriggerVolumeEvent::OnEnterExit: return "OnEnterExit";
    }
    return "Unknown";
}

enum class TriggerVolumeFilterMode : uint8_t {
    All, Player, NPC, Physics, Custom
};

inline const char* triggerVolumeFilterModeName(TriggerVolumeFilterMode f) {
    switch (f) {
        case TriggerVolumeFilterMode::All:     return "All";
        case TriggerVolumeFilterMode::Player:  return "Player";
        case TriggerVolumeFilterMode::NPC:     return "NPC";
        case TriggerVolumeFilterMode::Physics: return "Physics";
        case TriggerVolumeFilterMode::Custom:  return "Custom";
    }
    return "Unknown";
}

class TriggerVolume {
public:
    explicit TriggerVolume(const std::string& name, TriggerVolumeShape shape)
        : m_name(name), m_shape(shape) {}

    void setEvent(TriggerVolumeEvent e)         { m_event      = e; }
    void setFilterMode(TriggerVolumeFilterMode f){ m_filterMode = f; }
    void setEnabled(bool v)                     { m_enabled    = v; }
    void setDebugVisible(bool v)                { m_debugVisible = v; }
    void setRepeat(bool v)                      { m_repeat     = v; }
    void setCooldown(float c)                   { m_cooldown   = c; }

    [[nodiscard]] const std::string&      name()         const { return m_name;        }
    [[nodiscard]] TriggerVolumeShape      shape()        const { return m_shape;       }
    [[nodiscard]] TriggerVolumeEvent      event()        const { return m_event;       }
    [[nodiscard]] TriggerVolumeFilterMode filterMode()   const { return m_filterMode;  }
    [[nodiscard]] bool                    isEnabled()    const { return m_enabled;     }
    [[nodiscard]] bool                    isDebugVisible()const{ return m_debugVisible;}
    [[nodiscard]] bool                    repeats()      const { return m_repeat;      }
    [[nodiscard]] float                   cooldown()     const { return m_cooldown;    }

private:
    std::string            m_name;
    TriggerVolumeShape     m_shape;
    TriggerVolumeEvent     m_event       = TriggerVolumeEvent::OnEnter;
    TriggerVolumeFilterMode m_filterMode = TriggerVolumeFilterMode::All;
    float                  m_cooldown    = 0.0f;
    bool                   m_enabled     = true;
    bool                   m_debugVisible = false;
    bool                   m_repeat      = false;
};

class TriggerVolumeEditor {
public:
    static constexpr size_t MAX_VOLUMES = 1024;

    [[nodiscard]] bool addVolume(const TriggerVolume& vol) {
        for (auto& v : m_volumes) if (v.name() == vol.name()) return false;
        if (m_volumes.size() >= MAX_VOLUMES) return false;
        m_volumes.push_back(vol);
        return true;
    }

    [[nodiscard]] bool removeVolume(const std::string& name) {
        for (auto it = m_volumes.begin(); it != m_volumes.end(); ++it) {
            if (it->name() == name) { m_volumes.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] TriggerVolume* findVolume(const std::string& name) {
        for (auto& v : m_volumes) if (v.name() == name) return &v;
        return nullptr;
    }

    [[nodiscard]] size_t volumeCount()  const { return m_volumes.size(); }
    [[nodiscard]] size_t enabledCount() const {
        size_t c = 0; for (auto& v : m_volumes) if (v.isEnabled()) ++c; return c;
    }
    [[nodiscard]] size_t countByShape(TriggerVolumeShape s) const {
        size_t c = 0; for (auto& v : m_volumes) if (v.shape() == s) ++c; return c;
    }
    [[nodiscard]] size_t countByEvent(TriggerVolumeEvent e) const {
        size_t c = 0; for (auto& v : m_volumes) if (v.event() == e) ++c; return c;
    }
    [[nodiscard]] size_t countByFilter(TriggerVolumeFilterMode f) const {
        size_t c = 0; for (auto& v : m_volumes) if (v.filterMode() == f) ++c; return c;
    }
    [[nodiscard]] size_t debugVisibleCount() const {
        size_t c = 0; for (auto& v : m_volumes) if (v.isDebugVisible()) ++c; return c;
    }

private:
    std::vector<TriggerVolume> m_volumes;
};

} // namespace NF
