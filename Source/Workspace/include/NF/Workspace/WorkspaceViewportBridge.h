#pragma once
// NF::WorkspaceViewportBridge — Wires AtlasUI::ViewportPanel ↔ WorkspaceViewportManager.
//
// After a tool acquires a viewport handle via WorkspaceViewportManager::requestViewport()
// the UI panel needs to know:
//   1. Which slot handle it belongs to (for routing frame results).
//   2. When to propagate resize events so the GPU surface is reallocated.
//
// WorkspaceViewportBridge::connect() performs both wires in one call:
//   - Sets ViewportPanel::setViewportHandle(handle)
//   - Installs a ResizeCallback that calls viewportMgr.updateBounds(handle, bounds)
//   - Optionally registers a NullViewportSurface so the frame loop has something
//     to bind/unbind immediately (useful until a real backend surface is provided).
//
// Disconnect() tears down the bindings when the tool suspends/shuts down.
//
// This component is header-only and has no runtime state; it is a pure
// wiring utility called from WorkspaceShell post-init or from the tool itself.

#include "NF/Workspace/WorkspaceViewportManager.h"
#include "NF/Workspace/IViewportSurface.h"
#include "NF/UI/AtlasUI/Panels/ViewportPanel.h"

namespace NF {

class WorkspaceViewportBridge {
public:
    // ── connect ──────────────────────────────────────────────────────────────
    // Wire the panel to the given handle inside viewportMgr.
    //
    // If nullSurface is non-null it is registered with the manager so the frame
    // loop has a valid (but headless) surface to bind until a real backend is
    // attached.  The caller retains ownership of the surface.
    //
    // Lifetime contract: the stored resize callback references `viewportMgr` by
    // reference.  The caller MUST call disconnect() before either the panel or
    // the viewportMgr is destroyed, to clear the callback and prevent a dangling
    // reference.  In practice, tools call connect() in activate() and
    // disconnect() in suspend(), which is always before manager teardown.
    //
    // Returns true if the handle is valid and the panel is non-null.
    static bool connect(UI::AtlasUI::ViewportPanel* panel,
                        WorkspaceViewportManager&  viewportMgr,
                        ViewportHandle             handle,
                        IViewportSurface*          nullSurface = nullptr) {
        if (!panel || handle == kInvalidViewportHandle) return false;

        panel->setViewportHandle(handle);

        panel->setResizeCallback([&viewportMgr, handle](float x, float y, float w, float h) {
            viewportMgr.updateBounds(handle, {x, y, w, h});
            // Also resize the surface if one is registered.
            viewportMgr.surfaceRegistry().resize(handle,
                static_cast<uint32_t>(w > 0.f ? w : 0.f),
                static_cast<uint32_t>(h > 0.f ? h : 0.f));
        });

        if (nullSurface) {
            viewportMgr.registerSurface(handle, nullSurface);
        }

        return true;
    }

    // ── disconnect ────────────────────────────────────────────────────────────
    // Remove the resize callback and viewport handle from the panel, and
    // unregister the surface from the manager.
    static void disconnect(UI::AtlasUI::ViewportPanel* panel,
                           WorkspaceViewportManager&   viewportMgr,
                           ViewportHandle              handle) {
        if (!panel) return;
        panel->setViewportHandle(kInvalidViewportHandle);
        panel->setResizeCallback(nullptr);
        if (handle != kInvalidViewportHandle)
            viewportMgr.unregisterSurface(handle);
    }

    // ── forwardFrameResults ───────────────────────────────────────────────────
    // Called each frame after WorkspaceViewportManager::renderFrame().
    // For each result, if the panel's viewportHandle matches, setColorAttachment
    // is called so the panel blits the scene texture.
    static void forwardFrameResults(
        UI::AtlasUI::ViewportPanel*               panel,
        const std::vector<ViewportFrameResult>&   results) {
        if (!panel) return;
        uint32_t panelHandle = panel->viewportHandle();
        if (panelHandle == kInvalidViewportHandle) return;
        for (const auto& result : results) {
            if (result.handle == panelHandle) {
                panel->setColorAttachment(result.colorAttachmentId);
                return;
            }
        }
    }
};

} // namespace NF
