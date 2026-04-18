#pragma once
// NF::Editor — per-quality graphics preset management editor
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

enum class GfxQualityLevel : uint8_t { Low, Medium, High, Ultra, Custom };
inline const char* gfxQualityLevelName(GfxQualityLevel v) {
    switch (v) {
        case GfxQualityLevel::Low:    return "Low";
        case GfxQualityLevel::Medium: return "Medium";
        case GfxQualityLevel::High:   return "High";
        case GfxQualityLevel::Ultra:  return "Ultra";
        case GfxQualityLevel::Custom: return "Custom";
    }
    return "Unknown";
}

enum class GfxFeature : uint8_t { Shadows, AO, Reflections, AntiAliasing, VolumetricFog, MotionBlur };
inline const char* gfxFeatureName(GfxFeature v) {
    switch (v) {
        case GfxFeature::Shadows:       return "Shadows";
        case GfxFeature::AO:            return "AO";
        case GfxFeature::Reflections:   return "Reflections";
        case GfxFeature::AntiAliasing:  return "AntiAliasing";
        case GfxFeature::VolumetricFog: return "VolumetricFog";
        case GfxFeature::MotionBlur:    return "MotionBlur";
    }
    return "Unknown";
}

class GfxPreset {
public:
    explicit GfxPreset(uint32_t id, const std::string& name, GfxQualityLevel quality)
        : m_id(id), m_name(name), m_quality(quality) {}

    void setEnabledFeaturesMask(uint32_t v) { m_enabledFeaturesMask = v; }
    void setShadowDistance(float v)         { m_shadowDistance      = v; }
    void setIsEnabled(bool v)               { m_isEnabled           = v; }

    [[nodiscard]] uint32_t           id()                  const { return m_id;                  }
    [[nodiscard]] const std::string& name()                const { return m_name;                }
    [[nodiscard]] GfxQualityLevel    quality()             const { return m_quality;             }
    [[nodiscard]] uint32_t           enabledFeaturesMask() const { return m_enabledFeaturesMask; }
    [[nodiscard]] float              shadowDistance()      const { return m_shadowDistance;      }
    [[nodiscard]] bool               isEnabled()           const { return m_isEnabled;           }

private:
    uint32_t      m_id;
    std::string   m_name;
    GfxQualityLevel m_quality;
    uint32_t      m_enabledFeaturesMask = 0u;
    float         m_shadowDistance      = 100.0f;
    bool          m_isEnabled           = true;
};

class GraphicsSettingsEditor {
public:
    void setIsShowDisabled(bool v)         { m_isShowDisabled      = v; }
    void setIsGroupByQuality(bool v)       { m_isGroupByQuality    = v; }
    void setDefaultShadowDistance(float v) { m_defaultShadowDistance = v; }

    bool addPreset(const GfxPreset& p) {
        for (auto& x : m_presets) if (x.id() == p.id()) return false;
        m_presets.push_back(p); return true;
    }
    bool removePreset(uint32_t id) {
        auto it = std::find_if(m_presets.begin(), m_presets.end(),
            [&](const GfxPreset& p){ return p.id() == id; });
        if (it == m_presets.end()) return false;
        m_presets.erase(it); return true;
    }
    [[nodiscard]] GfxPreset* findPreset(uint32_t id) {
        for (auto& p : m_presets) if (p.id() == id) return &p;
        return nullptr;
    }

    [[nodiscard]] bool   isShowDisabled()        const { return m_isShowDisabled;       }
    [[nodiscard]] bool   isGroupByQuality()      const { return m_isGroupByQuality;     }
    [[nodiscard]] float  defaultShadowDistance() const { return m_defaultShadowDistance;}
    [[nodiscard]] size_t presetCount()           const { return m_presets.size();       }

    [[nodiscard]] size_t countByQuality(GfxQualityLevel q) const {
        size_t n = 0; for (auto& p : m_presets) if (p.quality() == q) ++n; return n;
    }
    [[nodiscard]] size_t countEnabled() const {
        size_t n = 0; for (auto& p : m_presets) if (p.isEnabled()) ++n; return n;
    }

private:
    std::vector<GfxPreset> m_presets;
    bool  m_isShowDisabled        = false;
    bool  m_isGroupByQuality      = true;
    float m_defaultShadowDistance = 50.0f;
};

} // namespace NF
