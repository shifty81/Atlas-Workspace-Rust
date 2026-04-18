#pragma once
// NF::Editor — Reflection probe editor v1: probe placement, bake mode, and blending
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Rpv1ProbeType    : uint8_t { Box, Sphere, Planar, ScreenSpace };
enum class Rpv1BakeMode     : uint8_t { Realtime, Baked, Custom, Once };
enum class Rpv1BakeStatus   : uint8_t { Idle, Baking, Done, Stale, Failed };

inline const char* rpv1ProbeTypeName(Rpv1ProbeType t) {
    switch (t) {
        case Rpv1ProbeType::Box:         return "Box";
        case Rpv1ProbeType::Sphere:      return "Sphere";
        case Rpv1ProbeType::Planar:      return "Planar";
        case Rpv1ProbeType::ScreenSpace: return "ScreenSpace";
    }
    return "Unknown";
}

inline const char* rpv1BakeModeName(Rpv1BakeMode m) {
    switch (m) {
        case Rpv1BakeMode::Realtime: return "Realtime";
        case Rpv1BakeMode::Baked:    return "Baked";
        case Rpv1BakeMode::Custom:   return "Custom";
        case Rpv1BakeMode::Once:     return "Once";
    }
    return "Unknown";
}

inline const char* rpv1BakeStatusName(Rpv1BakeStatus s) {
    switch (s) {
        case Rpv1BakeStatus::Idle:   return "Idle";
        case Rpv1BakeStatus::Baking: return "Baking";
        case Rpv1BakeStatus::Done:   return "Done";
        case Rpv1BakeStatus::Stale:  return "Stale";
        case Rpv1BakeStatus::Failed: return "Failed";
    }
    return "Unknown";
}

struct Rpv1Probe {
    uint64_t        id             = 0;
    std::string     name;
    Rpv1ProbeType   type           = Rpv1ProbeType::Box;
    Rpv1BakeMode    bakeMode       = Rpv1BakeMode::Baked;
    Rpv1BakeStatus  status         = Rpv1BakeStatus::Idle;
    uint16_t        resolution     = 256;
    float           blendDistance  = 1.0f;
    float           intensity      = 1.0f;
    bool            enabled        = true;
    bool            boxProjection  = false;

    [[nodiscard]] bool isValid() const { return id != 0 && !name.empty() && resolution > 0 && intensity >= 0.f; }
    [[nodiscard]] bool isDone()  const { return status == Rpv1BakeStatus::Done; }
    [[nodiscard]] bool isStale() const { return status == Rpv1BakeStatus::Stale; }
};

using Rpv1ChangeCallback = std::function<void(uint64_t)>;

class ReflectionProbeEditorV1 {
public:
    static constexpr size_t MAX_PROBES = 128;

    bool addProbe(const Rpv1Probe& probe) {
        if (!probe.isValid()) return false;
        for (const auto& p : m_probes) if (p.id == probe.id) return false;
        if (m_probes.size() >= MAX_PROBES) return false;
        m_probes.push_back(probe);
        return true;
    }

    bool removeProbe(uint64_t id) {
        for (auto it = m_probes.begin(); it != m_probes.end(); ++it) {
            if (it->id == id) {
                if (m_activeId == id) m_activeId = 0;
                m_probes.erase(it);
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] Rpv1Probe* findProbe(uint64_t id) {
        for (auto& p : m_probes) if (p.id == id) return &p;
        return nullptr;
    }

    bool setActive(uint64_t id) {
        for (const auto& p : m_probes) if (p.id == id) { m_activeId = id; return true; }
        return false;
    }

    bool updateStatus(uint64_t id, Rpv1BakeStatus status) {
        auto* p = findProbe(id);
        if (!p) return false;
        p->status = status;
        if (m_onChange) m_onChange(id);
        return true;
    }

    [[nodiscard]] uint64_t activeId()     const { return m_activeId; }
    [[nodiscard]] size_t   probeCount()   const { return m_probes.size(); }
    [[nodiscard]] size_t   doneCount()    const {
        size_t c = 0; for (const auto& p : m_probes) if (p.isDone())  ++c; return c;
    }
    [[nodiscard]] size_t   staleCount()   const {
        size_t c = 0; for (const auto& p : m_probes) if (p.isStale()) ++c; return c;
    }
    [[nodiscard]] size_t   countByType(Rpv1ProbeType t) const {
        size_t c = 0; for (const auto& p : m_probes) if (p.type == t) ++c; return c;
    }
    [[nodiscard]] size_t   countByMode(Rpv1BakeMode m) const {
        size_t c = 0; for (const auto& p : m_probes) if (p.bakeMode == m) ++c; return c;
    }

    void setOnChange(Rpv1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Rpv1Probe> m_probes;
    uint64_t               m_activeId = 0;
    Rpv1ChangeCallback     m_onChange;
};

} // namespace NF
