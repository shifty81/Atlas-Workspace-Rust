#pragma once
// NF::Editor — Level-of-detail editor
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

enum class LODTransitionMode : uint8_t {
    Discrete, Blend, Dithered, CrossFade, Instant
};

inline const char* lodTransitionModeName(LODTransitionMode m) {
    switch (m) {
        case LODTransitionMode::Discrete:  return "Discrete";
        case LODTransitionMode::Blend:     return "Blend";
        case LODTransitionMode::Dithered:  return "Dithered";
        case LODTransitionMode::CrossFade: return "CrossFade";
        case LODTransitionMode::Instant:   return "Instant";
    }
    return "Unknown";
}

enum class LODReductionMethod : uint8_t {
    None, Quadric, Progressive, Impostor, Billboard
};

inline const char* lodReductionMethodName(LODReductionMethod m) {
    switch (m) {
        case LODReductionMethod::None:        return "None";
        case LODReductionMethod::Quadric:     return "Quadric";
        case LODReductionMethod::Progressive: return "Progressive";
        case LODReductionMethod::Impostor:    return "Impostor";
        case LODReductionMethod::Billboard:   return "Billboard";
    }
    return "Unknown";
}

enum class LODBias : uint8_t {
    VeryHigh, High, Medium, Low, VeryLow
};

inline const char* lodBiasName(LODBias b) {
    switch (b) {
        case LODBias::VeryHigh: return "VeryHigh";
        case LODBias::High:     return "High";
        case LODBias::Medium:   return "Medium";
        case LODBias::Low:      return "Low";
        case LODBias::VeryLow:  return "VeryLow";
    }
    return "Unknown";
}

class LODLevel {
public:
    explicit LODLevel(uint8_t level, float screenSizePct)
        : m_level(level), m_screenSizePct(screenSizePct) {}

    void setTriangleCount(uint32_t n)           { m_triangleCount = n;         }
    void setReductionPct(float pct)             { m_reductionPct  = pct;       }
    void setMethod(LODReductionMethod m)        { m_method        = m;         }
    void setTransitionMode(LODTransitionMode t) { m_transitionMode = t;        }
    void setEnabled(bool v)                     { m_enabled       = v;         }

    [[nodiscard]] uint8_t             level()          const { return m_level;          }
    [[nodiscard]] float               screenSizePct()  const { return m_screenSizePct;  }
    [[nodiscard]] uint32_t            triangleCount()  const { return m_triangleCount;  }
    [[nodiscard]] float               reductionPct()   const { return m_reductionPct;   }
    [[nodiscard]] LODReductionMethod  method()         const { return m_method;         }
    [[nodiscard]] LODTransitionMode   transitionMode() const { return m_transitionMode; }
    [[nodiscard]] bool                isEnabled()      const { return m_enabled;        }

private:
    uint8_t             m_level;
    float               m_screenSizePct;
    uint32_t            m_triangleCount  = 0;
    float               m_reductionPct   = 0.0f;
    LODReductionMethod  m_method         = LODReductionMethod::Quadric;
    LODTransitionMode   m_transitionMode = LODTransitionMode::Discrete;
    bool                m_enabled        = true;
};

class LODEditor {
public:
    static constexpr size_t MAX_LEVELS = 8;

    [[nodiscard]] bool addLevel(const LODLevel& lod) {
        if (m_levels.size() >= MAX_LEVELS) return false;
        for (auto& l : m_levels) if (l.level() == lod.level()) return false;
        m_levels.push_back(lod);
        return true;
    }

    [[nodiscard]] bool removeLevel(uint8_t level) {
        for (auto it = m_levels.begin(); it != m_levels.end(); ++it) {
            if (it->level() == level) { m_levels.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] LODLevel* findLevel(uint8_t level) {
        for (auto& l : m_levels) if (l.level() == level) return &l;
        return nullptr;
    }

    void setBias(LODBias b)          { m_bias = b; }
    void setAutoGenerate(bool v)     { m_autoGenerate = v; }

    [[nodiscard]] LODBias bias()         const { return m_bias;         }
    [[nodiscard]] bool    autoGenerate() const { return m_autoGenerate; }
    [[nodiscard]] size_t  levelCount()   const { return m_levels.size(); }

    [[nodiscard]] size_t enabledCount() const {
        size_t c = 0; for (auto& l : m_levels) if (l.isEnabled()) ++c; return c;
    }
    [[nodiscard]] size_t countByMethod(LODReductionMethod m) const {
        size_t c = 0; for (auto& l : m_levels) if (l.method() == m) ++c; return c;
    }

private:
    std::vector<LODLevel> m_levels;
    LODBias               m_bias        = LODBias::Medium;
    bool                  m_autoGenerate = false;
};

} // namespace NF
