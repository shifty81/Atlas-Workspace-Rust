#pragma once
// NF::Editor — Level-of-detail configuration editor
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

enum class LODTransitionType : uint8_t {
    Discrete, Dithered, ScreenSize, Distance, Custom
};

inline const char* lodTransitionTypeName(LODTransitionType t) {
    switch (t) {
        case LODTransitionType::Discrete:   return "Discrete";
        case LODTransitionType::Dithered:   return "Dithered";
        case LODTransitionType::ScreenSize: return "ScreenSize";
        case LODTransitionType::Distance:   return "Distance";
        case LODTransitionType::Custom:     return "Custom";
    }
    return "Unknown";
}

enum class LODGenerationMode : uint8_t {
    Manual, Auto, Nanite, Impostor
};

inline const char* lodGenerationModeName(LODGenerationMode m) {
    switch (m) {
        case LODGenerationMode::Manual:   return "Manual";
        case LODGenerationMode::Auto:     return "Auto";
        case LODGenerationMode::Nanite:   return "Nanite";
        case LODGenerationMode::Impostor: return "Impostor";
    }
    return "Unknown";
}

enum class LODGroupPreset : uint8_t {
    None, SmallProp, MediumProp, LargeProp, Character, Vehicle, Building, Landscape
};

inline const char* lodGroupPresetName(LODGroupPreset p) {
    switch (p) {
        case LODGroupPreset::None:        return "None";
        case LODGroupPreset::SmallProp:   return "SmallProp";
        case LODGroupPreset::MediumProp:  return "MediumProp";
        case LODGroupPreset::LargeProp:   return "LargeProp";
        case LODGroupPreset::Character:   return "Character";
        case LODGroupPreset::Vehicle:     return "Vehicle";
        case LODGroupPreset::Building:    return "Building";
        case LODGroupPreset::Landscape:   return "Landscape";
    }
    return "Unknown";
}

class LODPanelLevel {
public:
    explicit LODPanelLevel(uint8_t idx, float screenSizeThreshold)
        : m_index(idx), m_screenSize(screenSizeThreshold) {}

    void setTriangleReduction(float r) { m_triReduction = r; }
    void setEnabled(bool v)            { m_enabled       = v; }

    [[nodiscard]] uint8_t levelIndex()        const { return m_index;        }
    [[nodiscard]] float   screenSize()        const { return m_screenSize;   }
    [[nodiscard]] float   triangleReduction() const { return m_triReduction; }
    [[nodiscard]] bool    isEnabled()         const { return m_enabled;      }

private:
    uint8_t m_index;
    float   m_screenSize;
    float   m_triReduction = 0.5f;
    bool    m_enabled      = true;
};

class LODPanelConfig {
public:
    explicit LODPanelConfig(const std::string& assetName)
        : m_assetName(assetName) {}

    void setTransitionType(LODTransitionType t)   { m_transitionType   = t; }
    void setGenerationMode(LODGenerationMode m)   { m_generationMode   = m; }
    void setGroupPreset(LODGroupPreset p)          { m_groupPreset      = p; }
    void setMinDrawDistance(float d)              { m_minDrawDistance  = d; }
    void setMaxDrawDistance(float d)              { m_maxDrawDistance  = d; }

    [[nodiscard]] bool addLevel(const LODPanelLevel& level) {
        for (auto& l : m_levels) if (l.levelIndex() == level.levelIndex()) return false;
        m_levels.push_back(level);
        return true;
    }

    [[nodiscard]] const std::string& assetName()       const { return m_assetName;       }
    [[nodiscard]] LODTransitionType  transitionType()  const { return m_transitionType;  }
    [[nodiscard]] LODGenerationMode  generationMode()  const { return m_generationMode;  }
    [[nodiscard]] LODGroupPreset     groupPreset()     const { return m_groupPreset;     }
    [[nodiscard]] float              minDrawDistance() const { return m_minDrawDistance; }
    [[nodiscard]] float              maxDrawDistance() const { return m_maxDrawDistance; }
    [[nodiscard]] size_t             levelCount()      const { return m_levels.size();   }
    [[nodiscard]] size_t             enabledLevelCount() const {
        size_t c = 0; for (auto& l : m_levels) if (l.isEnabled()) ++c; return c;
    }

private:
    std::string            m_assetName;
    std::vector<LODPanelLevel> m_levels;
    LODTransitionType      m_transitionType = LODTransitionType::ScreenSize;
    LODGenerationMode      m_generationMode = LODGenerationMode::Auto;
    LODGroupPreset         m_groupPreset    = LODGroupPreset::None;
    float                  m_minDrawDistance = 0.0f;
    float                  m_maxDrawDistance = 50000.0f;
};

class LODEditorPanel {
public:
    static constexpr size_t MAX_CONFIGS = 512;

    [[nodiscard]] bool addConfig(const LODPanelConfig& config) {
        for (auto& c : m_configs) if (c.assetName() == config.assetName()) return false;
        if (m_configs.size() >= MAX_CONFIGS) return false;
        m_configs.push_back(config);
        return true;
    }

    [[nodiscard]] bool removeConfig(const std::string& assetName) {
        for (auto it = m_configs.begin(); it != m_configs.end(); ++it) {
            if (it->assetName() == assetName) { m_configs.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] LODPanelConfig* findConfig(const std::string& assetName) {
        for (auto& c : m_configs) if (c.assetName() == assetName) return &c;
        return nullptr;
    }

    [[nodiscard]] size_t configCount() const { return m_configs.size(); }
    [[nodiscard]] size_t countByPreset(LODGroupPreset p) const {
        size_t c = 0; for (auto& cfg : m_configs) if (cfg.groupPreset() == p) ++c; return c;
    }
    [[nodiscard]] size_t countByGenerationMode(LODGenerationMode m) const {
        size_t c = 0; for (auto& cfg : m_configs) if (cfg.generationMode() == m) ++c; return c;
    }

private:
    std::vector<LODPanelConfig> m_configs;
};

} // namespace NF
