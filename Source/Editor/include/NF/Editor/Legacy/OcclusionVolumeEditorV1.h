#pragma once
// NF::Editor — Occlusion volume editor v1: static, dynamic, portal and blocker volumes
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Ovv1OccType : uint8_t { Static, Dynamic, Portal, Blocker };
enum class Ovv1OccState: uint8_t { Active, Disabled, Baked };

inline const char* ovv1OccTypeName(Ovv1OccType t) {
    switch (t) {
        case Ovv1OccType::Static:  return "Static";
        case Ovv1OccType::Dynamic: return "Dynamic";
        case Ovv1OccType::Portal:  return "Portal";
        case Ovv1OccType::Blocker: return "Blocker";
    }
    return "Unknown";
}

inline const char* ovv1OccStateName(Ovv1OccState s) {
    switch (s) {
        case Ovv1OccState::Active:   return "Active";
        case Ovv1OccState::Disabled: return "Disabled";
        case Ovv1OccState::Baked:    return "Baked";
    }
    return "Unknown";
}

struct Ovv1Volume {
    uint64_t    id         = 0;
    std::string name;
    Ovv1OccType type       = Ovv1OccType::Static;
    Ovv1OccState state     = Ovv1OccState::Active;
    float       boundX     = 1.f;
    float       boundY     = 1.f;
    float       boundZ     = 1.f;
    bool        isTwoSided = false;

    [[nodiscard]] bool isValid()  const { return id != 0 && !name.empty(); }
    [[nodiscard]] bool isActive() const { return state == Ovv1OccState::Active; }
};

using Ovv1ChangeCallback = std::function<void(uint64_t)>;

class OcclusionVolumeEditorV1 {
public:
    static constexpr size_t MAX_VOLUMES = 256;

    bool addVolume(const Ovv1Volume& vol) {
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

    [[nodiscard]] Ovv1Volume* findVolume(uint64_t id) {
        for (auto& v : m_volumes) if (v.id == id) return &v;
        return nullptr;
    }

    bool setState(uint64_t id, Ovv1OccState state) {
        auto* v = findVolume(id);
        if (!v) return false;
        v->state = state;
        if (m_onChange) m_onChange(id);
        return true;
    }

    bool setBounds(uint64_t id, float bx, float by, float bz) {
        auto* v = findVolume(id);
        if (!v) return false;
        v->boundX = bx;
        v->boundY = by;
        v->boundZ = bz;
        if (m_onChange) m_onChange(id);
        return true;
    }

    bool toggleTwoSided(uint64_t id) {
        auto* v = findVolume(id);
        if (!v) return false;
        v->isTwoSided = !v->isTwoSided;
        if (m_onChange) m_onChange(id);
        return true;
    }

    [[nodiscard]] size_t volumeCount() const { return m_volumes.size(); }

    [[nodiscard]] size_t activeCount() const {
        size_t c = 0;
        for (const auto& v : m_volumes) if (v.isActive()) ++c;
        return c;
    }

    [[nodiscard]] size_t countByType(Ovv1OccType type) const {
        size_t c = 0;
        for (const auto& v : m_volumes) if (v.type == type) ++c;
        return c;
    }

    [[nodiscard]] float totalBoundingVolume() const {
        float total = 0.f;
        for (const auto& v : m_volumes) total += v.boundX * v.boundY * v.boundZ;
        return total;
    }

    void setOnChange(Ovv1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Ovv1Volume> m_volumes;
    Ovv1ChangeCallback      m_onChange;
};

} // namespace NF
