#pragma once
// NF::Editor — Lightmap baker and UV unwrap editor
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

enum class LightmapQuality : uint8_t {
    Draft, Low, Medium, High, Production
};

inline const char* lightmapQualityName(LightmapQuality q) {
    switch (q) {
        case LightmapQuality::Draft:      return "Draft";
        case LightmapQuality::Low:        return "Low";
        case LightmapQuality::Medium:     return "Medium";
        case LightmapQuality::High:       return "High";
        case LightmapQuality::Production: return "Production";
    }
    return "Unknown";
}

enum class LightmapBakeMode : uint8_t {
    Baked, Mixed, Realtime, ShadowMaskOnly
};

inline const char* lightmapBakeModeName(LightmapBakeMode m) {
    switch (m) {
        case LightmapBakeMode::Baked:          return "Baked";
        case LightmapBakeMode::Mixed:          return "Mixed";
        case LightmapBakeMode::Realtime:       return "Realtime";
        case LightmapBakeMode::ShadowMaskOnly: return "ShadowMaskOnly";
    }
    return "Unknown";
}

enum class LightmapBakeStatus : uint8_t {
    Idle, Unwrapping, Sampling, Denoising, Done, Failed, Cancelled
};

inline const char* lightmapBakeStatusName(LightmapBakeStatus s) {
    switch (s) {
        case LightmapBakeStatus::Idle:       return "Idle";
        case LightmapBakeStatus::Unwrapping: return "Unwrapping";
        case LightmapBakeStatus::Sampling:   return "Sampling";
        case LightmapBakeStatus::Denoising:  return "Denoising";
        case LightmapBakeStatus::Done:       return "Done";
        case LightmapBakeStatus::Failed:     return "Failed";
        case LightmapBakeStatus::Cancelled:  return "Cancelled";
    }
    return "Unknown";
}

class LightmapBakeConfig {
public:
    explicit LightmapBakeConfig(const std::string& meshName)
        : m_meshName(meshName) {}

    void setQuality(LightmapQuality q)  { m_quality  = q; }
    void setBakeMode(LightmapBakeMode m){ m_bakeMode  = m; }
    void setTexelDensity(float d)       { m_texelDensity = d; }
    void setTextureSizePx(uint16_t s)   { m_textureSizePx = s; }
    void setStatus(LightmapBakeStatus s){ m_status    = s; }
    void setProgress(float p)           { m_progress  = p; }

    [[nodiscard]] const std::string& meshName()      const { return m_meshName;      }
    [[nodiscard]] LightmapQuality    quality()       const { return m_quality;       }
    [[nodiscard]] LightmapBakeMode   bakeMode()      const { return m_bakeMode;      }
    [[nodiscard]] float              texelDensity()  const { return m_texelDensity;  }
    [[nodiscard]] uint16_t           textureSizePx() const { return m_textureSizePx; }
    [[nodiscard]] LightmapBakeStatus status()        const { return m_status;        }
    [[nodiscard]] float              progress()      const { return m_progress;      }

    [[nodiscard]] bool isDone()    const { return m_status == LightmapBakeStatus::Done;    }
    [[nodiscard]] bool isBaking()  const {
        return m_status == LightmapBakeStatus::Sampling ||
               m_status == LightmapBakeStatus::Unwrapping ||
               m_status == LightmapBakeStatus::Denoising;
    }

private:
    std::string         m_meshName;
    LightmapQuality     m_quality        = LightmapQuality::Medium;
    LightmapBakeMode    m_bakeMode       = LightmapBakeMode::Baked;
    float               m_texelDensity   = 1.0f;
    uint16_t            m_textureSizePx  = 512;
    LightmapBakeStatus  m_status         = LightmapBakeStatus::Idle;
    float               m_progress       = 0.0f;
};

class LightmapEditorPanel {
public:
    static constexpr size_t MAX_CONFIGS = 256;

    [[nodiscard]] bool addConfig(const LightmapBakeConfig& cfg) {
        for (auto& c : m_configs) if (c.meshName() == cfg.meshName()) return false;
        if (m_configs.size() >= MAX_CONFIGS) return false;
        m_configs.push_back(cfg);
        return true;
    }

    [[nodiscard]] bool removeConfig(const std::string& meshName) {
        for (auto it = m_configs.begin(); it != m_configs.end(); ++it) {
            if (it->meshName() == meshName) { m_configs.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] LightmapBakeConfig* findConfig(const std::string& meshName) {
        for (auto& c : m_configs) if (c.meshName() == meshName) return &c;
        return nullptr;
    }

    [[nodiscard]] size_t configCount() const { return m_configs.size(); }
    [[nodiscard]] size_t doneCount()   const {
        size_t c = 0; for (auto& cfg : m_configs) if (cfg.isDone()) ++c; return c;
    }
    [[nodiscard]] size_t bakingCount() const {
        size_t c = 0; for (auto& cfg : m_configs) if (cfg.isBaking()) ++c; return c;
    }
    [[nodiscard]] size_t countByQuality(LightmapQuality q) const {
        size_t c = 0; for (auto& cfg : m_configs) if (cfg.quality() == q) ++c; return c;
    }
    [[nodiscard]] size_t countByMode(LightmapBakeMode m) const {
        size_t c = 0; for (auto& cfg : m_configs) if (cfg.bakeMode() == m) ++c; return c;
    }

private:
    std::vector<LightmapBakeConfig> m_configs;
};

} // namespace NF
