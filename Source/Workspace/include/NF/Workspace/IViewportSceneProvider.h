#pragma once
// NF::IViewportSceneProvider — Scene submission path (Item 2 of 10).
//
// The ViewportHostContract deliberately states that the workspace does NOT own
// the scene or camera.  This interface lets tools inject scene content into a
// viewport slot's render pass each frame without the workspace needing to know
// anything about 3D geometry or the renderer.
//
// Tools register themselves with ViewportSceneProviderRegistry when they
// acquire a viewport slot.  The ViewportFrameLoop dispatches to the provider
// registered for each slot's toolId before executing the render pass.

#include "NF/Workspace/ViewportHostContract.h"
#include <string>
#include <vector>

namespace NF {

// ── ViewportEntityProxy ───────────────────────────────────────────────────────
// Minimal per-entity data passed from a scene provider to the render pass.
// Carries only what the software renderer needs to project entities to screen:
// world-space position, half-extents for a bounding box, whether the entity
// is selected, and an optional display label.
//
// This is NOT a full scene graph node — it is a lightweight draw-list entry.

struct ViewportEntityProxy {
    float       x = 0.f, y = 0.f, z = 0.f;   ///< world-space centre position
    float       halfW = 0.5f;                  ///< half-width  (X extent)
    float       halfH = 0.5f;                  ///< half-height (Y extent)
    float       halfD = 0.5f;                  ///< half-depth  (Z extent)
    bool        selected = false;              ///< draw with selection colour
    const char* label    = nullptr;            ///< optional entity name (may be null)
};

// ── ViewportSceneState ────────────────────────────────────────────────────────
// Lightweight, renderer-agnostic snapshot of what a tool wants to render.
// The frame loop uses this to decide how to drive the render backend.

struct ViewportSceneState {
    bool     hasContent     = false; ///< true when the provider has scene data
    uint32_t entityCount    = 0;     ///< number of entities / draw calls in scene
    bool     overrideCamera = false; ///< provider has updated slot.camera already
    uint32_t clearColor     = 0x1E1E1EFFu; ///< suggested clear color (RRGGBBAA)

    /// Per-entity proxy list for the software renderer.
    /// Populated by IViewportSceneProvider::provideScene() when hasContent=true.
    /// GPU backends ignore this and use their own scene graph access instead.
    std::vector<ViewportEntityProxy> entities;
};

// ── IViewportSceneProvider ────────────────────────────────────────────────────
// Implement this in each tool (SceneEditorTool, MaterialEditorTool, etc.)
// to feed scene data into a viewport slot.

class IViewportSceneProvider {
public:
    virtual ~IViewportSceneProvider() = default;

    /// Called by the frame loop for each active slot whose toolId matches
    /// the registered toolId.  The provider may update slot.camera via
    /// ViewportHostRegistry::setCamera() and returns its scene state.
    virtual ViewportSceneState provideScene(ViewportHandle handle,
                                            const ViewportSlot& slot) = 0;
};

// ── ViewportSceneProviderRegistry ─────────────────────────────────────────────
// Workspace-owned registry mapping tool IDs to scene providers.
// Providers are NOT owned by the registry (lifetime managed by the tool).

class ViewportSceneProviderRegistry {
public:
    static constexpr size_t kMaxProviders = 16;

    /// Register a provider for the given toolId.
    /// Returns false if toolId already registered or capacity reached.
    bool registerProvider(const std::string& toolId, IViewportSceneProvider* provider) {
        if (!provider || toolId.empty()) return false;
        if (m_entries.size() >= kMaxProviders) return false;
        for (const auto& e : m_entries)
            if (e.toolId == toolId) return false;
        m_entries.push_back({toolId, provider});
        return true;
    }

    /// Unregister and return true, or false if not found.
    bool unregisterProvider(const std::string& toolId) {
        for (auto it = m_entries.begin(); it != m_entries.end(); ++it) {
            if (it->toolId == toolId) {
                m_entries.erase(it);
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] IViewportSceneProvider* findProvider(const std::string& toolId) const {
        for (const auto& e : m_entries)
            if (e.toolId == toolId) return e.provider;
        return nullptr;
    }

    /// Dispatch to the provider registered for slot.toolId.
    /// Returns a default (empty) state if no provider is registered.
    ViewportSceneState dispatchProvide(const ViewportSlot& slot) {
        if (auto* p = findProvider(slot.toolId))
            return p->provideScene(slot.handle, slot);
        return {};
    }

    [[nodiscard]] size_t providerCount() const { return m_entries.size(); }
    void clear() { m_entries.clear(); }

private:
    struct Entry {
        std::string toolId;
        IViewportSceneProvider* provider;
    };
    std::vector<Entry> m_entries;
};

} // namespace NF
