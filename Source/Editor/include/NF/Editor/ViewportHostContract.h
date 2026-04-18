#pragma once
// NF::Editor — Viewport host contract for custom rendering integration
#include "NF/Core/Core.h"
#include "NF/Editor/EditorCamera.h"
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace NF {

// ── Viewport Render Target ───────────────────────────────────────
// Describes the render target for a hosted viewport.

struct ViewportRenderTarget {
    uint32_t width  = 0;
    uint32_t height = 0;
    uint32_t framebufferHandle = 0;
    float    dpiScale = 1.f;
    bool     valid = false;

    [[nodiscard]] bool isValid() const { return valid && width > 0 && height > 0; }
    void invalidate() { valid = false; }
    void resize(uint32_t w, uint32_t h) {
        width = w;
        height = h;
    }
    [[nodiscard]] float aspectRatio() const {
        return height > 0 ? static_cast<float>(width) / static_cast<float>(height) : 1.f;
    }
};

// ── Viewport Input State ─────────────────────────────────────────
// Captures input state relevant to the viewport area.

struct ViewportInputState {
    float mouseX = 0.f;
    float mouseY = 0.f;
    float deltaX = 0.f;
    float deltaY = 0.f;
    float scrollDelta = 0.f;
    bool  leftButton   = false;
    bool  rightButton  = false;
    bool  middleButton = false;
    bool  hasFocus     = false;
    bool  hovered      = false;

    [[nodiscard]] bool anyButtonDown() const {
        return leftButton || rightButton || middleButton;
    }
};

// ── Gizmo Space ──────────────────────────────────────────────────

enum class GizmoSpace : uint8_t {
    Local = 0,
    World = 1,
};

inline const char* gizmoSpaceName(GizmoSpace s) {
    switch (s) {
        case GizmoSpace::Local: return "Local";
        case GizmoSpace::World: return "World";
        default:                return "Unknown";
    }
}

// ── IViewportHost ────────────────────────────────────────────────
// Interface that projects/plugins implement to provide custom
// rendering content inside editor viewports.

class IViewportHost {
public:
    virtual ~IViewportHost() = default;

    // Called when the viewport is first created or re-attached
    virtual void onAttach(const ViewportRenderTarget& target) = 0;

    // Called when the viewport is destroyed or detached
    virtual void onDetach() = 0;

    // Called each frame to update the scene
    virtual void onUpdate(float dt) = 0;

    // Called each frame to render into the target
    virtual void onRender(const ViewportRenderTarget& target) = 0;

    // Called when viewport is resized
    virtual void onResize(uint32_t width, uint32_t height) = 0;

    // Called to deliver input events
    virtual void onInput(const ViewportInputState& input) = 0;

    // Returns a human-readable name for this host
    [[nodiscard]] virtual const char* hostName() const = 0;

    // Returns true if the host wants exclusive input capture
    [[nodiscard]] virtual bool wantsExclusiveInput() const { return false; }
};

// ── Viewport Host Registry ───────────────────────────────────────
// Manages available viewport host implementations.

class ViewportRenderHostRegistry {
public:
    static constexpr size_t MAX_HOSTS = 16;

    bool registerHost(const std::string& id, std::shared_ptr<IViewportHost> host) {
        if (!host || id.empty()) return false;
        if (m_hosts.size() >= MAX_HOSTS) return false;
        for (const auto& entry : m_hosts) if (entry.id == id) return false;
        m_hosts.push_back({id, std::move(host)});
        return true;
    }

    bool unregisterHost(const std::string& id) {
        for (auto it = m_hosts.begin(); it != m_hosts.end(); ++it) {
            if (it->id == id) {
                m_hosts.erase(it);
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] std::shared_ptr<IViewportHost> findHost(const std::string& id) const {
        for (const auto& entry : m_hosts) {
            if (entry.id == id) return entry.host;
        }
        return nullptr;
    }

    [[nodiscard]] size_t hostCount() const { return m_hosts.size(); }

    [[nodiscard]] std::vector<std::string> hostIds() const {
        std::vector<std::string> ids;
        ids.reserve(m_hosts.size());
        for (const auto& entry : m_hosts) ids.push_back(entry.id);
        return ids;
    }

    void setActiveHost(const std::string& id) { m_activeId = id; }
    [[nodiscard]] const std::string& activeHostId() const { return m_activeId; }

    [[nodiscard]] std::shared_ptr<IViewportHost> activeHost() const {
        return findHost(m_activeId);
    }

private:
    struct HostEntry {
        std::string id;
        std::shared_ptr<IViewportHost> host;
    };
    std::vector<HostEntry> m_hosts;
    std::string m_activeId;
};

// ── Viewport Host Session ────────────────────────────────────────
// Manages the lifecycle of a viewport with an attached host.

class ViewportHostSession {
public:
    void attach(std::shared_ptr<IViewportHost> host, const ViewportRenderTarget& target) {
        if (m_host) detach();
        m_host = std::move(host);
        m_target = target;
        if (m_host) m_host->onAttach(m_target);
        m_attached = true;
    }

    void detach() {
        if (m_host && m_attached) {
            m_host->onDetach();
        }
        m_host = nullptr;
        m_attached = false;
    }

    void update(float dt) {
        if (m_host && m_attached) m_host->onUpdate(dt);
    }

    void render() {
        if (m_host && m_attached) m_host->onRender(m_target);
    }

    void resize(uint32_t width, uint32_t height) {
        m_target.resize(width, height);
        if (m_host && m_attached) m_host->onResize(width, height);
    }

    void deliverInput(const ViewportInputState& input) {
        if (m_host && m_attached) m_host->onInput(input);
    }

    [[nodiscard]] bool isAttached() const { return m_attached; }
    [[nodiscard]] std::shared_ptr<IViewportHost> host() const { return m_host; }
    [[nodiscard]] const ViewportRenderTarget& target() const { return m_target; }

    void setGizmoMode(GizmoMode mode) { m_gizmoMode = mode; }
    [[nodiscard]] GizmoMode gizmoMode() const { return m_gizmoMode; }

    void setGizmoSpace(GizmoSpace space) { m_gizmoSpace = space; }
    [[nodiscard]] GizmoSpace gizmoSpace() const { return m_gizmoSpace; }

private:
    std::shared_ptr<IViewportHost> m_host;
    ViewportRenderTarget m_target;
    bool m_attached = false;
    GizmoMode m_gizmoMode = GizmoMode::Translate;
    GizmoSpace m_gizmoSpace = GizmoSpace::World;
};


} // namespace NF
