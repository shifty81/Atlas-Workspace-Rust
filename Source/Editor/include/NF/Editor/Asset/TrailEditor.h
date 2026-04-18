#pragma once
// NF::Editor — trail editor
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

enum class TrailRendererType : uint8_t {
    Line, Ribbon, Tube, Billboard, Mesh
};

inline const char* trailRendererTypeName(TrailRendererType t) {
    switch (t) {
        case TrailRendererType::Line:      return "Line";
        case TrailRendererType::Ribbon:    return "Ribbon";
        case TrailRendererType::Tube:      return "Tube";
        case TrailRendererType::Billboard: return "Billboard";
        case TrailRendererType::Mesh:      return "Mesh";
    }
    return "Unknown";
}

enum class TrailColorMode : uint8_t {
    Constant, OverLifetime, OverSpeed, OverAngle
};

inline const char* trailColorModeName(TrailColorMode m) {
    switch (m) {
        case TrailColorMode::Constant:    return "Constant";
        case TrailColorMode::OverLifetime:return "OverLifetime";
        case TrailColorMode::OverSpeed:   return "OverSpeed";
        case TrailColorMode::OverAngle:   return "OverAngle";
    }
    return "Unknown";
}

enum class TrailWidthMode : uint8_t {
    Constant, OverLifetime, OverSpeed
};

inline const char* trailWidthModeName(TrailWidthMode m) {
    switch (m) {
        case TrailWidthMode::Constant:    return "Constant";
        case TrailWidthMode::OverLifetime:return "OverLifetime";
        case TrailWidthMode::OverSpeed:   return "OverSpeed";
    }
    return "Unknown";
}

class TrailConfig {
public:
    explicit TrailConfig(uint32_t id, const std::string& name)
        : m_id(id), m_name(name) {}

    void setRendererType(TrailRendererType v)  { m_rendererType = v; }
    void setColorMode(TrailColorMode v)        { m_colorMode = v; }
    void setWidthMode(TrailWidthMode v)        { m_widthMode = v; }
    void setLifetime(float v)                  { m_lifetime = v; }
    void setMinVertexDistance(float v)         { m_minVertexDistance = v; }
    void setWidth(float v)                     { m_width = v; }
    void setCastShadows(bool v)                { m_castShadows = v; }

    [[nodiscard]] uint32_t            id()                const { return m_id; }
    [[nodiscard]] const std::string&  name()              const { return m_name; }
    [[nodiscard]] TrailRendererType   rendererType()      const { return m_rendererType; }
    [[nodiscard]] TrailColorMode      colorMode()         const { return m_colorMode; }
    [[nodiscard]] TrailWidthMode      widthMode()         const { return m_widthMode; }
    [[nodiscard]] float               lifetime()          const { return m_lifetime; }
    [[nodiscard]] float               minVertexDistance() const { return m_minVertexDistance; }
    [[nodiscard]] float               width()             const { return m_width; }
    [[nodiscard]] bool                castShadows()       const { return m_castShadows; }

private:
    uint32_t          m_id;
    std::string       m_name;
    TrailRendererType m_rendererType     = TrailRendererType::Ribbon;
    TrailColorMode    m_colorMode        = TrailColorMode::OverLifetime;
    TrailWidthMode    m_widthMode        = TrailWidthMode::OverLifetime;
    float             m_lifetime         = 1.0f;
    float             m_minVertexDistance = 0.1f;
    float             m_width            = 0.1f;
    bool              m_castShadows      = false;
};

class TrailEditor {
public:
    bool addConfig(const TrailConfig& config) {
        for (const auto& c : m_configs)
            if (c.id() == config.id()) return false;
        m_configs.push_back(config);
        return true;
    }

    bool removeConfig(uint32_t id) {
        for (auto it = m_configs.begin(); it != m_configs.end(); ++it) {
            if (it->id() == id) { m_configs.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] TrailConfig* findConfig(uint32_t id) {
        for (auto& c : m_configs)
            if (c.id() == id) return &c;
        return nullptr;
    }

    [[nodiscard]] size_t configCount() const { return m_configs.size(); }

    [[nodiscard]] size_t countByRendererType(TrailRendererType t) const {
        size_t n = 0;
        for (const auto& c : m_configs) if (c.rendererType() == t) ++n;
        return n;
    }

    [[nodiscard]] size_t countByColorMode(TrailColorMode m) const {
        size_t n = 0;
        for (const auto& c : m_configs) if (c.colorMode() == m) ++n;
        return n;
    }

    [[nodiscard]] size_t countCastingShadows() const {
        size_t n = 0;
        for (const auto& c : m_configs) if (c.castShadows()) ++n;
        return n;
    }

    void setShowGizmos(bool v)       { m_isShowGizmos = v; }
    void setLoopPreview(bool v)      { m_isLoopPreview = v; }
    void setPreviewSpeed(float v)    { m_previewSpeed = v; }

    [[nodiscard]] bool  isShowGizmos()  const { return m_isShowGizmos; }
    [[nodiscard]] bool  isLoopPreview() const { return m_isLoopPreview; }
    [[nodiscard]] float previewSpeed()  const { return m_previewSpeed; }

private:
    std::vector<TrailConfig> m_configs;
    bool  m_isShowGizmos  = true;
    bool  m_isLoopPreview = true;
    float m_previewSpeed  = 1.0f;
};

} // namespace NF
