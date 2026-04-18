#pragma once
// NF::Editor — Graph editor host contract for custom graph integration
#include "NF/Editor/EditorPanel.h"
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace NF {

// ── Graph Host Capabilities ──────────────────────────────────────
// Flags describing what a graph host supports.

enum class GraphHostCapability : uint32_t {
    None       = 0,
    Undo       = 1 << 0,
    Redo       = 1 << 1,
    Zoom       = 1 << 2,
    Pan        = 1 << 3,
    Selection  = 1 << 4,
    Minimap    = 1 << 5,
    Search     = 1 << 6,
    Validation = 1 << 7,
};

inline GraphHostCapability operator|(GraphHostCapability a, GraphHostCapability b) {
    return static_cast<GraphHostCapability>(
        static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}

inline bool hasCapability(GraphHostCapability caps, GraphHostCapability flag) {
    return (static_cast<uint32_t>(caps) & static_cast<uint32_t>(flag)) != 0;
}

// ── Graph View State ─────────────────────────────────────────────
// Describes the current view transform for a graph editor.

struct GraphViewState {
    float scrollX     = 0.f;
    float scrollY     = 0.f;
    float zoom        = 1.f;
    bool  showGrid    = true;
    bool  showMinimap = false;

    static constexpr float MIN_ZOOM = 0.1f;
    static constexpr float MAX_ZOOM = 4.0f;

    void setZoom(float z) {
        zoom = z < MIN_ZOOM ? MIN_ZOOM : (z > MAX_ZOOM ? MAX_ZOOM : z);
    }
    [[nodiscard]] bool isDefaultView() const {
        return scrollX == 0.f && scrollY == 0.f && zoom == 1.f;
    }
    void reset() { scrollX = 0.f; scrollY = 0.f; zoom = 1.f; }
};

// ── IGraphHost ───────────────────────────────────────────────────
// Interface that projects/plugins implement to provide custom
// graph rendering content inside editor graph panels.

class IGraphHost {
public:
    virtual ~IGraphHost() = default;

    // Called when the graph host is attached to a panel
    virtual void onAttach() = 0;

    // Called when the graph host is detached
    virtual void onDetach() = 0;

    // Called each frame to update graph state
    virtual void onUpdate(float dt) = 0;

    // Called to render graph contents into the given bounds
    virtual void onRender(const Rect& bounds) = 0;

    // Called when a node is selected by id
    virtual void onNodeSelected(uint32_t nodeId) = 0;

    // Called when a connection is made between two pins
    virtual void onConnected(uint32_t fromPin, uint32_t toPin) = 0;

    // Returns a human-readable name for this host
    [[nodiscard]] virtual const char* hostName() const = 0;

    // Returns the capabilities of this host
    [[nodiscard]] virtual GraphHostCapability capabilities() const = 0;

    // Returns the number of nodes managed by this host
    [[nodiscard]] virtual size_t nodeCount() const = 0;

    // Returns true if the graph has unsaved changes
    [[nodiscard]] virtual bool isDirty() const = 0;
};

// ── Graph Host Registry ──────────────────────────────────────────
// Manages available graph host implementations.

class GraphHostRegistry {
public:
    static constexpr size_t MAX_HOSTS = 16;

    bool registerHost(const std::string& id, std::shared_ptr<IGraphHost> host) {
        if (!host || id.empty()) return false;
        if (m_hosts.size() >= MAX_HOSTS) return false;
        for (const auto& entry : m_hosts) if (entry.id == id) return false;
        m_hosts.push_back({id, std::move(host)});
        return true;
    }

    bool unregisterHost(const std::string& id) {
        for (auto it = m_hosts.begin(); it != m_hosts.end(); ++it) {
            if (it->id == id) { m_hosts.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] std::shared_ptr<IGraphHost> findHost(const std::string& id) const {
        for (const auto& entry : m_hosts) if (entry.id == id) return entry.host;
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
    [[nodiscard]] std::shared_ptr<IGraphHost> activeHost() const { return findHost(m_activeId); }

private:
    struct HostEntry { std::string id; std::shared_ptr<IGraphHost> host; };
    std::vector<HostEntry> m_hosts;
    std::string m_activeId;
};

// ── Graph Host Session ───────────────────────────────────────────
// Manages the lifecycle of a graph panel with an attached host.

class GraphHostSession {
public:
    void attach(std::shared_ptr<IGraphHost> host) {
        if (m_host) detach();
        m_host = std::move(host);
        if (m_host) m_host->onAttach();
        m_attached = true;
    }

    void detach() {
        if (m_host && m_attached) m_host->onDetach();
        m_host = nullptr;
        m_attached = false;
    }

    void update(float dt) {
        if (m_host && m_attached) m_host->onUpdate(dt);
    }

    void render(const Rect& bounds) {
        if (m_host && m_attached) m_host->onRender(bounds);
    }

    [[nodiscard]] bool isAttached() const { return m_attached; }
    [[nodiscard]] std::shared_ptr<IGraphHost> host() const { return m_host; }

    void setViewState(const GraphViewState& state) { m_viewState = state; }
    [[nodiscard]] GraphViewState& viewState() { return m_viewState; }
    [[nodiscard]] const GraphViewState& viewState() const { return m_viewState; }

    void selectNode(uint32_t nodeId) {
        if (m_host && m_attached) m_host->onNodeSelected(nodeId);
        m_selectedNodeId = nodeId;
    }

    [[nodiscard]] uint32_t selectedNodeId() const { return m_selectedNodeId; }

private:
    std::shared_ptr<IGraphHost> m_host;
    GraphViewState m_viewState;
    bool m_attached = false;
    uint32_t m_selectedNodeId = 0;
};


} // namespace NF
