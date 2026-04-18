#pragma once
// NF::Editor — Soundscape and ambient audio zone editor
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

enum class SoundscapeZoneShape : uint8_t {
    Box, Sphere, Capsule, Convex, Global
};

inline const char* soundscapeZoneShapeName(SoundscapeZoneShape s) {
    switch (s) {
        case SoundscapeZoneShape::Box:     return "Box";
        case SoundscapeZoneShape::Sphere:  return "Sphere";
        case SoundscapeZoneShape::Capsule: return "Capsule";
        case SoundscapeZoneShape::Convex:  return "Convex";
        case SoundscapeZoneShape::Global:  return "Global";
    }
    return "Unknown";
}

enum class SoundscapeBlendMode : uint8_t {
    Additive, Override, Interpolate, Priority
};

inline const char* soundscapeBlendModeName(SoundscapeBlendMode m) {
    switch (m) {
        case SoundscapeBlendMode::Additive:    return "Additive";
        case SoundscapeBlendMode::Override:    return "Override";
        case SoundscapeBlendMode::Interpolate: return "Interpolate";
        case SoundscapeBlendMode::Priority:    return "Priority";
    }
    return "Unknown";
}

enum class SoundscapeReverbPreset : uint8_t {
    None, Room, Hall, Cave, Forest, City, Dungeon, Custom
};

inline const char* soundscapeReverbPresetName(SoundscapeReverbPreset p) {
    switch (p) {
        case SoundscapeReverbPreset::None:    return "None";
        case SoundscapeReverbPreset::Room:    return "Room";
        case SoundscapeReverbPreset::Hall:    return "Hall";
        case SoundscapeReverbPreset::Cave:    return "Cave";
        case SoundscapeReverbPreset::Forest:  return "Forest";
        case SoundscapeReverbPreset::City:    return "City";
        case SoundscapeReverbPreset::Dungeon: return "Dungeon";
        case SoundscapeReverbPreset::Custom:  return "Custom";
    }
    return "Unknown";
}

class SoundscapeZone {
public:
    explicit SoundscapeZone(const std::string& name, SoundscapeZoneShape shape)
        : m_name(name), m_shape(shape) {}

    void setBlendMode(SoundscapeBlendMode m)   { m_blendMode  = m; }
    void setReverbPreset(SoundscapeReverbPreset p){ m_reverb   = p; }
    void setVolume(float v)                    { m_volume     = v; }
    void setPriority(int p)                    { m_priority   = p; }
    void setFadeInTime(float t)                { m_fadeIn     = t; }
    void setFadeOutTime(float t)               { m_fadeOut    = t; }
    void setEnabled(bool v)                    { m_enabled    = v; }

    [[nodiscard]] const std::string&    name()         const { return m_name;        }
    [[nodiscard]] SoundscapeZoneShape   shape()        const { return m_shape;       }
    [[nodiscard]] SoundscapeBlendMode   blendMode()    const { return m_blendMode;   }
    [[nodiscard]] SoundscapeReverbPreset reverb()      const { return m_reverb;      }
    [[nodiscard]] float                 volume()       const { return m_volume;      }
    [[nodiscard]] int                   priority()     const { return m_priority;    }
    [[nodiscard]] float                 fadeInTime()   const { return m_fadeIn;      }
    [[nodiscard]] float                 fadeOutTime()  const { return m_fadeOut;     }
    [[nodiscard]] bool                  isEnabled()    const { return m_enabled;     }

private:
    std::string              m_name;
    SoundscapeZoneShape      m_shape;
    SoundscapeBlendMode      m_blendMode = SoundscapeBlendMode::Interpolate;
    SoundscapeReverbPreset   m_reverb    = SoundscapeReverbPreset::None;
    float                    m_volume    = 1.0f;
    float                    m_fadeIn    = 1.0f;
    float                    m_fadeOut   = 1.0f;
    int                      m_priority  = 0;
    bool                     m_enabled   = true;
};

class SoundscapeEditor {
public:
    static constexpr size_t MAX_ZONES = 64;

    [[nodiscard]] bool addZone(const SoundscapeZone& zone) {
        for (auto& z : m_zones) if (z.name() == zone.name()) return false;
        if (m_zones.size() >= MAX_ZONES) return false;
        m_zones.push_back(zone);
        return true;
    }

    [[nodiscard]] bool removeZone(const std::string& name) {
        for (auto it = m_zones.begin(); it != m_zones.end(); ++it) {
            if (it->name() == name) {
                if (m_activeZone == name) m_activeZone.clear();
                m_zones.erase(it);
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] SoundscapeZone* findZone(const std::string& name) {
        for (auto& z : m_zones) if (z.name() == name) return &z;
        return nullptr;
    }

    [[nodiscard]] bool setActiveZone(const std::string& name) {
        for (auto& z : m_zones) if (z.name() == name) { m_activeZone = name; return true; }
        return false;
    }

    [[nodiscard]] const std::string& activeZone() const { return m_activeZone; }
    [[nodiscard]] size_t zoneCount()  const { return m_zones.size(); }
    [[nodiscard]] size_t enabledCount() const {
        size_t c = 0; for (auto& z : m_zones) if (z.isEnabled()) ++c; return c;
    }
    [[nodiscard]] size_t countByShape(SoundscapeZoneShape s) const {
        size_t c = 0; for (auto& z : m_zones) if (z.shape() == s) ++c; return c;
    }
    [[nodiscard]] size_t countByReverb(SoundscapeReverbPreset p) const {
        size_t c = 0; for (auto& z : m_zones) if (z.reverb() == p) ++c; return c;
    }

private:
    std::vector<SoundscapeZone> m_zones;
    std::string                 m_activeZone;
};

} // namespace NF
