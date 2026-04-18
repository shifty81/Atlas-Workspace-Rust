#pragma once
// NF::Editor — Reflection probe placement and bake editor
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

enum class ReflectionProbeType : uint8_t {
    Box, Sphere, Planar, ScreenSpace
};

inline const char* reflectionProbeTypeName(ReflectionProbeType t) {
    switch (t) {
        case ReflectionProbeType::Box:         return "Box";
        case ReflectionProbeType::Sphere:      return "Sphere";
        case ReflectionProbeType::Planar:      return "Planar";
        case ReflectionProbeType::ScreenSpace: return "ScreenSpace";
    }
    return "Unknown";
}

enum class ReflectionBakeMode : uint8_t {
    Realtime, Baked, Custom, Once
};

inline const char* reflectionBakeModeName(ReflectionBakeMode m) {
    switch (m) {
        case ReflectionBakeMode::Realtime: return "Realtime";
        case ReflectionBakeMode::Baked:    return "Baked";
        case ReflectionBakeMode::Custom:   return "Custom";
        case ReflectionBakeMode::Once:     return "Once";
    }
    return "Unknown";
}

enum class ReflectionBakeStatus : uint8_t {
    Idle, Baking, Done, Stale, Failed
};

inline const char* reflectionBakeStatusName(ReflectionBakeStatus s) {
    switch (s) {
        case ReflectionBakeStatus::Idle:   return "Idle";
        case ReflectionBakeStatus::Baking: return "Baking";
        case ReflectionBakeStatus::Done:   return "Done";
        case ReflectionBakeStatus::Stale:  return "Stale";
        case ReflectionBakeStatus::Failed: return "Failed";
    }
    return "Unknown";
}

class ReflectionProbe {
public:
    explicit ReflectionProbe(const std::string& name, ReflectionProbeType type)
        : m_name(name), m_type(type) {}

    void setBakeMode(ReflectionBakeMode m)   { m_bakeMode   = m; }
    void setStatus(ReflectionBakeStatus s)   { m_status     = s; }
    void setResolution(uint16_t res)         { m_resolution = res; }
    void setBlendDistance(float d)           { m_blendDist  = d;  }
    void setIntensity(float i)               { m_intensity  = i;  }
    void setEnabled(bool v)                  { m_enabled    = v;  }
    void setBoxProjection(bool v)            { m_boxProjection = v; }

    [[nodiscard]] const std::string&  name()          const { return m_name;         }
    [[nodiscard]] ReflectionProbeType type()          const { return m_type;         }
    [[nodiscard]] ReflectionBakeMode  bakeMode()      const { return m_bakeMode;     }
    [[nodiscard]] ReflectionBakeStatus status()       const { return m_status;       }
    [[nodiscard]] uint16_t            resolution()    const { return m_resolution;   }
    [[nodiscard]] float               blendDistance() const { return m_blendDist;    }
    [[nodiscard]] float               intensity()     const { return m_intensity;    }
    [[nodiscard]] bool                isEnabled()     const { return m_enabled;      }
    [[nodiscard]] bool                boxProjection() const { return m_boxProjection;}

    [[nodiscard]] bool isDone()   const { return m_status == ReflectionBakeStatus::Done; }
    [[nodiscard]] bool isStale()  const { return m_status == ReflectionBakeStatus::Stale; }

private:
    std::string           m_name;
    ReflectionProbeType   m_type;
    ReflectionBakeMode    m_bakeMode      = ReflectionBakeMode::Baked;
    ReflectionBakeStatus  m_status        = ReflectionBakeStatus::Idle;
    uint16_t              m_resolution    = 256;
    float                 m_blendDist     = 1.0f;
    float                 m_intensity     = 1.0f;
    bool                  m_enabled       = true;
    bool                  m_boxProjection = false;
};

class ReflectionProbeEditor {
public:
    static constexpr size_t MAX_PROBES = 128;

    [[nodiscard]] bool addProbe(const ReflectionProbe& probe) {
        for (auto& p : m_probes) if (p.name() == probe.name()) return false;
        if (m_probes.size() >= MAX_PROBES) return false;
        m_probes.push_back(probe);
        return true;
    }

    [[nodiscard]] bool removeProbe(const std::string& name) {
        for (auto it = m_probes.begin(); it != m_probes.end(); ++it) {
            if (it->name() == name) {
                if (m_activeProbe == name) m_activeProbe.clear();
                m_probes.erase(it);
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] ReflectionProbe* findProbe(const std::string& name) {
        for (auto& p : m_probes) if (p.name() == name) return &p;
        return nullptr;
    }

    [[nodiscard]] bool setActiveProbe(const std::string& name) {
        for (auto& p : m_probes) if (p.name() == name) { m_activeProbe = name; return true; }
        return false;
    }

    [[nodiscard]] const std::string& activeProbe() const { return m_activeProbe; }
    [[nodiscard]] size_t probeCount()  const { return m_probes.size(); }
    [[nodiscard]] size_t doneCount()   const {
        size_t c = 0; for (auto& p : m_probes) if (p.isDone())  ++c; return c;
    }
    [[nodiscard]] size_t staleCount()  const {
        size_t c = 0; for (auto& p : m_probes) if (p.isStale()) ++c; return c;
    }
    [[nodiscard]] size_t countByType(ReflectionProbeType t) const {
        size_t c = 0; for (auto& p : m_probes) if (p.type() == t) ++c; return c;
    }
    [[nodiscard]] size_t countByBakeMode(ReflectionBakeMode m) const {
        size_t c = 0; for (auto& p : m_probes) if (p.bakeMode() == m) ++c; return c;
    }

private:
    std::vector<ReflectionProbe> m_probes;
    std::string                  m_activeProbe;
};

} // namespace NF
