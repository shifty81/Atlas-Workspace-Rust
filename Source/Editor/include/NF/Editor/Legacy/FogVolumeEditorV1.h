#pragma once
// NF::Editor — Fog volume editor v1: box, sphere, height and global fog authoring
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Fvv1FogType    : uint8_t { Box, Sphere, Height, Global };
enum class Fvv1VolumeState: uint8_t { Active, Disabled, Preview };
enum class Fvv1Priority   : uint8_t { High, Normal, Low };

inline const char* fvv1FogTypeName(Fvv1FogType t) {
    switch (t) {
        case Fvv1FogType::Box:    return "Box";
        case Fvv1FogType::Sphere: return "Sphere";
        case Fvv1FogType::Height: return "Height";
        case Fvv1FogType::Global: return "Global";
    }
    return "Unknown";
}

inline const char* fvv1VolumeStateName(Fvv1VolumeState s) {
    switch (s) {
        case Fvv1VolumeState::Active:   return "Active";
        case Fvv1VolumeState::Disabled: return "Disabled";
        case Fvv1VolumeState::Preview:  return "Preview";
    }
    return "Unknown";
}

inline const char* fvv1PriorityName(Fvv1Priority p) {
    switch (p) {
        case Fvv1Priority::High:   return "High";
        case Fvv1Priority::Normal: return "Normal";
        case Fvv1Priority::Low:    return "Low";
    }
    return "Unknown";
}

struct Fvv1Volume {
    uint64_t        id       = 0;
    std::string     name;
    Fvv1FogType     type     = Fvv1FogType::Box;
    Fvv1VolumeState state    = Fvv1VolumeState::Active;
    float           density  = 0.f;
    float           falloff  = 0.f;
    float           colorR   = 1.f;
    float           colorG   = 1.f;
    float           colorB   = 1.f;
    float           colorA   = 1.f;
    Fvv1Priority    priority = Fvv1Priority::Normal;

    [[nodiscard]] bool isValid()  const { return id != 0 && !name.empty(); }
    [[nodiscard]] bool isActive() const { return state == Fvv1VolumeState::Active; }
};

using Fvv1ChangeCallback = std::function<void(uint64_t)>;

class FogVolumeEditorV1 {
public:
    static constexpr size_t MAX_VOLUMES = 128;

    bool addVolume(const Fvv1Volume& vol) {
        if (!vol.isValid()) return false;
        for (const auto& v : m_volumes) if (v.id == vol.id) return false;
        if (m_volumes.size() >= MAX_VOLUMES) return false;
        m_volumes.push_back(vol);
        if (m_onChange) m_onChange(vol.id);
        return true;
    }

    bool removeVolume(uint64_t id) {
        for (auto it = m_volumes.begin(); it != m_volumes.end(); ++it) {
            if (it->id == id) { m_volumes.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] Fvv1Volume* findVolume(uint64_t id) {
        for (auto& v : m_volumes) if (v.id == id) return &v;
        return nullptr;
    }

    bool setState(uint64_t id, Fvv1VolumeState state) {
        auto* v = findVolume(id);
        if (!v) return false;
        v->state = state;
        if (m_onChange) m_onChange(id);
        return true;
    }

    bool setDensity(uint64_t id, float density) {
        auto* v = findVolume(id);
        if (!v) return false;
        v->density = std::clamp(density, 0.f, 1.f);
        if (m_onChange) m_onChange(id);
        return true;
    }

    bool setPriority(uint64_t id, Fvv1Priority priority) {
        auto* v = findVolume(id);
        if (!v) return false;
        v->priority = priority;
        if (m_onChange) m_onChange(id);
        return true;
    }

    [[nodiscard]] size_t volumeCount() const { return m_volumes.size(); }

    [[nodiscard]] size_t activeCount() const {
        size_t c = 0;
        for (const auto& v : m_volumes) if (v.isActive()) ++c;
        return c;
    }

    [[nodiscard]] size_t countByType(Fvv1FogType type) const {
        size_t c = 0;
        for (const auto& v : m_volumes) if (v.type == type) ++c;
        return c;
    }

    void setOnChange(Fvv1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Fvv1Volume> m_volumes;
    Fvv1ChangeCallback      m_onChange;
};

} // namespace NF
