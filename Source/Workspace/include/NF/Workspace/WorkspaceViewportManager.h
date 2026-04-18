#pragma once
// NF::WorkspaceViewportManager — Phase 65: viewport subsystem orchestrator.
//
// Assembles all Phase 64 viewport infrastructure into one manager:
//   - ViewportHostRegistry   (slot allocation / camera / renderMode)
//   - ViewportSurfaceRegistry (GPU surface per slot)
//   - ViewportSceneProviderRegistry (tool scene injection)
//   - ViewportFrameLoop       (per-frame render dispatch)
//   - ViewportCompositor      (multi-viewport layout)
//   - GizmoRenderer           (gizmo overlay accumulation)
//
// WorkspaceShell owns one WorkspaceViewportManager and exposes it via
// viewportManager().  Tools that need a 3D view call requestViewport()
// to obtain a handle, register a surface and/or scene provider, then call
// activateViewport().  The host (WorkspaceShell) calls renderFrame() once per tick.
//
// Design rules (from ViewportHostContract.h):
//   - The workspace does NOT own the scene camera or scene graph.
//   - Tools inject scene content via IViewportSceneProvider.
//   - Surfaces are NOT owned by this manager; the caller manages lifetime.
//   - Providers are NOT owned by this manager; the tool manages lifetime.

#include "NF/Workspace/ViewportHostContract.h"
#include "NF/Workspace/IViewportSurface.h"
#include "NF/Workspace/IViewportSceneProvider.h"
#include "NF/Workspace/ViewportFrameLoop.h"
#include "NF/Workspace/ViewportCompositor.h"
#include "NF/Workspace/GizmoRenderer.h"
#include "NF/Workspace/SoftwareViewportRenderer.h"
#include <memory>
#include <vector>
#include <string>

namespace NF {

// ── WorkspaceViewportManager ──────────────────────────────────────────────────

class WorkspaceViewportManager {
public:
    WorkspaceViewportManager() {
        m_frameLoop.setViewportRegistry(&m_viewportReg);
        m_frameLoop.setSceneRegistry(&m_sceneReg);
        m_frameLoop.setSurfaceRegistry(&m_surfaceReg);
        m_frameLoop.setGizmoRenderer(&m_gizmoRenderer);
    }

    // ── Slot lifecycle ────────────────────────────────────────────────────────

    /// Request a new viewport slot for the given tool.
    /// Returns kInvalidViewportHandle if no slots are available or args invalid.
    [[nodiscard]] ViewportHandle requestViewport(const std::string& toolId,
                                                  const ViewportBounds& bounds) {
        return m_viewportReg.requestSlot(toolId, bounds);
    }

    /// Release a viewport slot.  Returns false if handle is unknown.
    bool releaseViewport(ViewportHandle handle) {
        m_surfaceReg.unregisterSurface(handle);
        return m_viewportReg.releaseSlot(handle);
    }

    /// Activate a slot — begin rendering.
    bool activateViewport(ViewportHandle handle) {
        return m_viewportReg.activateSlot(handle);
    }

    /// Pause a rendering slot (e.g. panel hidden).
    bool pauseViewport(ViewportHandle handle) {
        return m_viewportReg.pauseSlot(handle);
    }

    /// Resume a paused viewport slot.
    bool resumeViewport(ViewportHandle handle) {
        return m_viewportReg.resumeSlot(handle);
    }

    /// Update the pixel bounds of a slot (e.g. window resize).
    bool updateBounds(ViewportHandle handle, const ViewportBounds& bounds) {
        return m_viewportReg.updateBounds(handle, bounds);
    }

    /// Set the render mode for a slot.
    bool setRenderMode(ViewportHandle handle, ViewportRenderMode mode) {
        return m_viewportReg.setRenderMode(handle, mode);
    }

    /// Update the camera descriptor for a slot.
    bool setCamera(ViewportHandle handle, const ViewportCameraDescriptor& camera) {
        return m_viewportReg.setCamera(handle, camera);
    }

    // ── Surface registration ──────────────────────────────────────────────────

    /// Associate a GPU surface with a viewport slot.
    /// The surface is NOT owned; the caller manages its lifetime.
    bool registerSurface(ViewportHandle handle, IViewportSurface* surface) {
        return m_surfaceReg.registerSurface(handle, surface);
    }

    /// Remove the surface association for a slot.
    bool unregisterSurface(ViewportHandle handle) {
        return m_surfaceReg.unregisterSurface(handle);
    }

    // ── Scene provider registration ───────────────────────────────────────────

    /// Register a scene provider for the given tool ID.
    /// The provider is NOT owned; the tool manages its lifetime.
    bool registerSceneProvider(const std::string& toolId, IViewportSceneProvider* provider) {
        return m_sceneReg.registerProvider(toolId, provider);
    }

    /// Unregister the scene provider for a tool ID.
    bool unregisterSceneProvider(const std::string& toolId) {
        return m_sceneReg.unregisterProvider(toolId);
    }

    // ── Layout ────────────────────────────────────────────────────────────────

    /// Change the multi-viewport layout mode.
    void setLayoutMode(ViewportLayoutMode mode) { m_compositor.setLayoutMode(mode); }
    [[nodiscard]] ViewportLayoutMode layoutMode() const { return m_compositor.layoutMode(); }
    [[nodiscard]] const char* layoutName()        const { return m_compositor.layoutName(); }

    /// Compute per-slot pixel bounds given the full workspace viewport region
    /// and the ordered list of handles to lay out.
    [[nodiscard]] std::vector<CompositorSlotBounds> computeLayout(
        const ViewportBounds& fullBounds,
        const std::vector<ViewportHandle>& handles) const
    {
        return m_compositor.computeSlotBounds(fullBounds, handles);
    }

    // ── Gizmo overlay ─────────────────────────────────────────────────────────

    /// Submit a gizmo draw command for this frame.
    void addGizmo(const GizmoDrawCommand& cmd) { m_gizmoRenderer.addGizmo(cmd); }

    /// Flush the gizmo overlay onto a surface.
    /// Returns the number of gizmos composited; 0 for invalid surface.
    uint32_t renderGizmos(IViewportSurface& surface, ViewportHandle handle) {
        return m_gizmoRenderer.renderToSurface(surface, handle);
    }

    /// Clear accumulated gizmo commands (call at the start of each frame).
    void clearGizmos() { m_gizmoRenderer.clear(); }

    [[nodiscard]] const GizmoRenderer& gizmoRenderer() const { return m_gizmoRenderer; }
    [[nodiscard]] GizmoRenderer&       gizmoRenderer()       { return m_gizmoRenderer; }

    // ── Frame loop ────────────────────────────────────────────────────────────

    /// Execute one render frame across all active slots.
    /// Returns one ViewportFrameResult per active slot.
    std::vector<ViewportFrameResult> renderFrame() {
        return m_frameLoop.renderFrame();
    }

    [[nodiscard]] const ViewportFrameStats& frameStats() const {
        return m_frameLoop.stats();
    }

    // ── Queries ───────────────────────────────────────────────────────────────

    [[nodiscard]] size_t slotCount()    const { return m_viewportReg.slotCount(); }
    [[nodiscard]] size_t activeCount()  const { return m_viewportReg.activeCount(); }
    [[nodiscard]] size_t surfaceCount() const { return m_surfaceReg.surfaceCount(); }
    [[nodiscard]] size_t providerCount() const { return m_sceneReg.providerCount(); }

    [[nodiscard]] const ViewportSlot* findSlot(ViewportHandle handle) const {
        return m_viewportReg.findSlot(handle);
    }

    [[nodiscard]] const std::vector<ViewportSlot>& slots() const {
        return m_viewportReg.slots();
    }

    /// Collect handles of all active slots (useful for computeLayout).
    [[nodiscard]] std::vector<ViewportHandle> activeHandles() const {
        std::vector<ViewportHandle> handles;
        for (const auto& slot : m_viewportReg.slots())
            if (slot.isActive()) handles.push_back(slot.handle);
        return handles;
    }

    // ── Sub-registry accessors (advanced use) ─────────────────────────────────

    [[nodiscard]] ViewportHostRegistry&            viewportRegistry()  { return m_viewportReg; }
    [[nodiscard]] const ViewportHostRegistry&      viewportRegistry()  const { return m_viewportReg; }
    [[nodiscard]] ViewportSurfaceRegistry&         surfaceRegistry()   { return m_surfaceReg; }
    [[nodiscard]] const ViewportSurfaceRegistry&   surfaceRegistry()   const { return m_surfaceReg; }
    [[nodiscard]] ViewportSceneProviderRegistry&   sceneRegistry()     { return m_sceneReg; }
    [[nodiscard]] const ViewportSceneProviderRegistry& sceneRegistry() const { return m_sceneReg; }
    [[nodiscard]] ViewportCompositor&              compositor()        { return m_compositor; }
    [[nodiscard]] const ViewportCompositor&        compositor()        const { return m_compositor; }
    [[nodiscard]] ViewportFrameLoop&               frameLoop()         { return m_frameLoop; }
    [[nodiscard]] const ViewportFrameLoop&         frameLoop()         const { return m_frameLoop; }

    // ── Software renderer ─────────────────────────────────────────────────────

    /// Install the built-in SoftwareViewportRenderer as the frame loop's
    /// render callback.  This draws a visible placeholder grid into each
    /// viewport so the pipeline is proven end-to-end even without a GPU.
    /// Call this after construction to enable visible viewport content.
    void installSoftwareRenderer() {
        if (!m_softRenderer)
            m_softRenderer = std::make_unique<SoftwareViewportRenderer>();
        m_frameLoop.setRenderCallback(
            [this](IViewportSurface& surface,
                   const ViewportSlot& slot,
                   const ViewportSceneState& scene) {
                m_softRenderer->renderGrid(surface, slot, scene);
            });
    }

    /// Returns the software renderer (nullptr if not installed).
    [[nodiscard]] SoftwareViewportRenderer* softwareRenderer() const {
        return m_softRenderer.get();
    }

private:
    ViewportHostRegistry          m_viewportReg;
    ViewportSurfaceRegistry       m_surfaceReg;
    ViewportSceneProviderRegistry m_sceneReg;
    ViewportFrameLoop             m_frameLoop;
    ViewportCompositor            m_compositor;
    GizmoRenderer                 m_gizmoRenderer;
    std::unique_ptr<SoftwareViewportRenderer> m_softRenderer;
};

} // namespace NF
