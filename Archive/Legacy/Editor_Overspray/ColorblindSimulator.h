#pragma once
// NF::Editor — colorblind simulator
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

enum class ColorblindType : uint8_t {
    None, Protanopia, Deuteranopia, Tritanopia, Protanomaly, Deuteranomaly,
    Tritanomaly, Achromatopsia, Achromatomaly
};

inline const char* colorblindTypeName(ColorblindType t) {
    switch (t) {
        case ColorblindType::None:          return "None";
        case ColorblindType::Protanopia:    return "Protanopia";
        case ColorblindType::Deuteranopia:  return "Deuteranopia";
        case ColorblindType::Tritanopia:    return "Tritanopia";
        case ColorblindType::Protanomaly:   return "Protanomaly";
        case ColorblindType::Deuteranomaly: return "Deuteranomaly";
        case ColorblindType::Tritanomaly:   return "Tritanomaly";
        case ColorblindType::Achromatopsia: return "Achromatopsia";
        case ColorblindType::Achromatomaly: return "Achromatomaly";
    }
    return "Unknown";
}

enum class ColorblindSimMode : uint8_t {
    RealTime, Screenshot, SideBySide, Overlay, Palette
};

inline const char* colorblindSimModeName(ColorblindSimMode m) {
    switch (m) {
        case ColorblindSimMode::RealTime:   return "RealTime";
        case ColorblindSimMode::Screenshot: return "Screenshot";
        case ColorblindSimMode::SideBySide: return "SideBySide";
        case ColorblindSimMode::Overlay:    return "Overlay";
        case ColorblindSimMode::Palette:    return "Palette";
    }
    return "Unknown";
}

enum class ColorblindSeverity : uint8_t {
    Mild, Moderate, Severe, Complete
};

inline const char* colorblindSeverityName(ColorblindSeverity s) {
    switch (s) {
        case ColorblindSeverity::Mild:     return "Mild";
        case ColorblindSeverity::Moderate: return "Moderate";
        case ColorblindSeverity::Severe:   return "Severe";
        case ColorblindSeverity::Complete: return "Complete";
    }
    return "Unknown";
}

class ColorblindSimProfile {
public:
    explicit ColorblindSimProfile(uint32_t id, const std::string& name, ColorblindType type)
        : m_id(id), m_name(name), m_type(type) {}

    void setSimMode(ColorblindSimMode v)   { m_simMode  = v; }
    void setSeverity(ColorblindSeverity v) { m_severity = v; }
    void setIsActive(bool v)               { m_isActive = v; }
    void setBlendFactor(float v)           { m_blendFactor = v; }

    [[nodiscard]] uint32_t            id()          const { return m_id;          }
    [[nodiscard]] const std::string&  name()        const { return m_name;        }
    [[nodiscard]] ColorblindType      type()        const { return m_type;        }
    [[nodiscard]] ColorblindSimMode   simMode()     const { return m_simMode;     }
    [[nodiscard]] ColorblindSeverity  severity()    const { return m_severity;    }
    [[nodiscard]] bool                isActive()    const { return m_isActive;    }
    [[nodiscard]] float               blendFactor() const { return m_blendFactor; }

private:
    uint32_t          m_id;
    std::string       m_name;
    ColorblindType    m_type;
    ColorblindSimMode m_simMode     = ColorblindSimMode::SideBySide;
    ColorblindSeverity m_severity   = ColorblindSeverity::Severe;
    bool              m_isActive    = false;
    float             m_blendFactor = 1.0f;
};

class ColorblindSimulator {
public:
    void setActiveType(ColorblindType v)   { m_activeType  = v; }
    void setActiveMode(ColorblindSimMode v){ m_activeMode  = v; }
    void setShowStats(bool v)              { m_showStats   = v; }

    bool addProfile(const ColorblindSimProfile& p) {
        for (auto& x : m_profiles) if (x.id() == p.id()) return false;
        m_profiles.push_back(p); return true;
    }
    bool removeProfile(uint32_t id) {
        auto it = std::find_if(m_profiles.begin(), m_profiles.end(),
            [&](const ColorblindSimProfile& p){ return p.id() == id; });
        if (it == m_profiles.end()) return false;
        m_profiles.erase(it); return true;
    }
    [[nodiscard]] ColorblindSimProfile* findProfile(uint32_t id) {
        for (auto& p : m_profiles) if (p.id() == id) return &p;
        return nullptr;
    }

    [[nodiscard]] ColorblindType    activeType()    const { return m_activeType;  }
    [[nodiscard]] ColorblindSimMode activeMode()    const { return m_activeMode;  }
    [[nodiscard]] bool              isShowStats()   const { return m_showStats;   }
    [[nodiscard]] size_t            profileCount()  const { return m_profiles.size(); }

    [[nodiscard]] size_t countByType(ColorblindType t) const {
        size_t n = 0; for (auto& p : m_profiles) if (p.type() == t) ++n; return n;
    }
    [[nodiscard]] size_t countActive() const {
        size_t n = 0; for (auto& p : m_profiles) if (p.isActive()) ++n; return n;
    }

private:
    std::vector<ColorblindSimProfile> m_profiles;
    ColorblindType                    m_activeType = ColorblindType::None;
    ColorblindSimMode                 m_activeMode = ColorblindSimMode::SideBySide;
    bool                              m_showStats  = true;
};

} // namespace NF
