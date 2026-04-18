#pragma once
// NF::Editor — Workspace viewport manager v1: viewport slot registration and camera binding authoring
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Wvmv1ViewportType  : uint8_t { Scene3D, Scene2D, UV, Preview, Game, Custom };
enum class Wvmv1ShadingMode   : uint8_t { Wireframe, Solid, Material, Rendered, Unlit };
enum class Wvmv1ViewportState : uint8_t { Idle, Focused, Locked, Maximized, Minimized };

inline const char* wvmv1ViewportTypeName(Wvmv1ViewportType t) {
    switch (t) {
        case Wvmv1ViewportType::Scene3D:  return "Scene3D";
        case Wvmv1ViewportType::Scene2D:  return "Scene2D";
        case Wvmv1ViewportType::UV:       return "UV";
        case Wvmv1ViewportType::Preview:  return "Preview";
        case Wvmv1ViewportType::Game:     return "Game";
        case Wvmv1ViewportType::Custom:   return "Custom";
    }
    return "Unknown";
}

inline const char* wvmv1ShadingModeName(Wvmv1ShadingMode m) {
    switch (m) {
        case Wvmv1ShadingMode::Wireframe: return "Wireframe";
        case Wvmv1ShadingMode::Solid:     return "Solid";
        case Wvmv1ShadingMode::Material:  return "Material";
        case Wvmv1ShadingMode::Rendered:  return "Rendered";
        case Wvmv1ShadingMode::Unlit:     return "Unlit";
    }
    return "Unknown";
}

inline const char* wvmv1ViewportStateName(Wvmv1ViewportState s) {
    switch (s) {
        case Wvmv1ViewportState::Idle:      return "Idle";
        case Wvmv1ViewportState::Focused:   return "Focused";
        case Wvmv1ViewportState::Locked:    return "Locked";
        case Wvmv1ViewportState::Maximized: return "Maximized";
        case Wvmv1ViewportState::Minimized: return "Minimized";
    }
    return "Unknown";
}

struct Wvmv1Viewport {
    uint64_t            id            = 0;
    std::string         name;
    std::string         boundCamera;
    Wvmv1ViewportType   viewportType  = Wvmv1ViewportType::Scene3D;
    Wvmv1ShadingMode    shadingMode   = Wvmv1ShadingMode::Solid;
    Wvmv1ViewportState  state         = Wvmv1ViewportState::Idle;
    float               fov           = 60.f;
    bool                showGrid      = true;
    bool                showGizmos    = true;
    bool                showStats     = false;

    [[nodiscard]] bool isValid()      const { return id != 0 && !name.empty() && fov > 0.f; }
    [[nodiscard]] bool isFocused()    const { return state == Wvmv1ViewportState::Focused; }
    [[nodiscard]] bool isMaximized()  const { return state == Wvmv1ViewportState::Maximized; }
    [[nodiscard]] bool isLocked()     const { return state == Wvmv1ViewportState::Locked; }
};

using Wvmv1ChangeCallback = std::function<void(uint64_t)>;

class WorkspaceViewportManagerV1 {
public:
    static constexpr size_t MAX_VIEWPORTS = 16;

    bool addViewport(const Wvmv1Viewport& vp) {
        if (!vp.isValid()) return false;
        for (const auto& v : m_viewports) if (v.id == vp.id) return false;
        if (m_viewports.size() >= MAX_VIEWPORTS) return false;
        m_viewports.push_back(vp);
        return true;
    }

    bool removeViewport(uint64_t id) {
        for (auto it = m_viewports.begin(); it != m_viewports.end(); ++it) {
            if (it->id == id) {
                if (m_focusedId == id) m_focusedId = 0;
                m_viewports.erase(it);
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] Wvmv1Viewport* findViewport(uint64_t id) {
        for (auto& v : m_viewports) if (v.id == id) return &v;
        return nullptr;
    }

    bool setFocus(uint64_t id) {
        auto* v = findViewport(id);
        if (!v || v->isLocked()) return false;
        for (auto& vp : m_viewports)
            if (vp.state == Wvmv1ViewportState::Focused) vp.state = Wvmv1ViewportState::Idle;
        v->state    = Wvmv1ViewportState::Focused;
        m_focusedId = id;
        if (m_onChange) m_onChange(id);
        return true;
    }

    bool setState(uint64_t id, Wvmv1ViewportState state) {
        auto* v = findViewport(id);
        if (!v) return false;
        v->state = state;
        if (m_onChange) m_onChange(id);
        return true;
    }

    bool setShadingMode(uint64_t id, Wvmv1ShadingMode mode) {
        auto* v = findViewport(id);
        if (!v) return false;
        v->shadingMode = mode;
        if (m_onChange) m_onChange(id);
        return true;
    }

    bool bindCamera(uint64_t id, const std::string& camera) {
        auto* v = findViewport(id);
        if (!v) return false;
        v->boundCamera = camera;
        if (m_onChange) m_onChange(id);
        return true;
    }

    bool setFOV(uint64_t id, float fov) {
        if (fov <= 0.f) return false;
        auto* v = findViewport(id);
        if (!v) return false;
        v->fov = fov;
        if (m_onChange) m_onChange(id);
        return true;
    }

    [[nodiscard]] uint64_t focusedId()        const { return m_focusedId; }
    [[nodiscard]] size_t   viewportCount()    const { return m_viewports.size(); }
    [[nodiscard]] size_t   maximizedCount()   const {
        size_t c = 0; for (const auto& v : m_viewports) if (v.isMaximized()) ++c; return c;
    }
    [[nodiscard]] size_t   countByType(Wvmv1ViewportType type) const {
        size_t c = 0; for (const auto& v : m_viewports) if (v.viewportType == type) ++c; return c;
    }
    [[nodiscard]] size_t   countByShading(Wvmv1ShadingMode mode) const {
        size_t c = 0; for (const auto& v : m_viewports) if (v.shadingMode == mode) ++c; return c;
    }

    void setOnChange(Wvmv1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Wvmv1Viewport> m_viewports;
    uint64_t                   m_focusedId = 0;
    Wvmv1ChangeCallback        m_onChange;
};

} // namespace NF
