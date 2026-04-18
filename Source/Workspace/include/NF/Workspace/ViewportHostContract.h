#pragma once
// NF::ViewportHostContract — Formal contract for 3D viewport surface hosting.
//
// A viewport host is the region of the workspace that owns a 3D rendering
// surface. Tools that need a 3D view (SceneEditor, AnimationEditor, etc.)
// request a viewport slot from the workspace shell via this contract.
//
// Architecture:
//   - WorkspaceShell owns a ViewportHostRegistry
//   - Tools acquire a ViewportSlot via requestSlot()
//   - Each slot has a unique handle, bounds, and render surface descriptor
//   - The render backend renders into the slot's surface on each frame
//   - Slots are released when tools are deactivated
//
// Render surface model:
//   - The workspace does NOT own the scene camera or scene graph
//   - Tools inject a ViewportCameraDescriptor and ViewportSceneDescriptor
//   - The render backend resolves these into actual GPU resources
//   - This keeps NF::Workspace tool-agnostic

#include "NF/Core/Core.h"
#include "NF/UI/ViewportRenderMode.h"
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

// ── Viewport Handle ───────────────────────────────────────────────
// Opaque handle identifying a viewport slot.

using ViewportHandle = uint32_t;
static constexpr ViewportHandle kInvalidViewportHandle = 0;

// ── Viewport Bounds ───────────────────────────────────────────────
// Screen-space pixel bounds for the viewport surface.

struct ViewportBounds {
    float x = 0.f, y = 0.f;
    float width = 0.f, height = 0.f;

    [[nodiscard]] bool isValid() const { return width > 0.f && height > 0.f; }
    [[nodiscard]] float aspectRatio() const {
        return height > 0.f ? width / height : 1.f;
    }
    [[nodiscard]] bool contains(float px, float py) const {
        return px >= x && px <= x + width && py >= y && py <= y + height;
    }
};

// ── Viewport State ────────────────────────────────────────────────

enum class ViewportState : uint8_t {
    Idle,       // allocated, not rendering
    Active,     // rendering every frame
    Paused,     // rendering paused (minimized/hidden)
    Released,   // slot returned to pool
};

inline const char* viewportStateName(ViewportState s) {
    switch (s) {
    case ViewportState::Idle:     return "Idle";
    case ViewportState::Active:   return "Active";
    case ViewportState::Paused:   return "Paused";
    case ViewportState::Released: return "Released";
    }
    return "Unknown";
}

// ── Viewport Camera Descriptor ────────────────────────────────────
// Tool-supplied camera parameters injected into the viewport each frame.

struct ViewportCameraDescriptor {
    float fovDegrees  = 60.f;
    float nearPlane   = 0.1f;
    float farPlane    = 10000.f;
    bool  orthographic = false;
    float orthoSize   = 10.f; // half-height in world units (orthographic mode)

    // Camera world-space transform — unified from ViewportCameraController.
    // yaw/pitch are in degrees; yaw=-90 looks down -Z (same as EditorViewportPanel).
    Vec3  position{0.f, 0.f, 5.f};
    float yaw   = -90.f;
    float pitch =   0.f;

    [[nodiscard]] bool isValid() const {
        return fovDegrees > 0.f && fovDegrees < 180.f
            && nearPlane > 0.f && farPlane > nearPlane;
    }
};

// ── Viewport Grid Descriptor ──────────────────────────────────────

struct ViewportGridDescriptor {
    bool    visible   = true;
    float   spacing   = 1.f;   // world units per grid line
    uint32_t color    = 0x333333FFu; // RRGGBBAA
    uint32_t majorColor = 0x555555FFu;
    int     majorEvery = 10; // every N lines is a "major" line
};

// ── Viewport Render Mode ──────────────────────────────────────────
// ViewportRenderMode and viewportRenderModeName() are defined in
// NF/UI/ViewportRenderMode.h (included above).

// ── Viewport Slot ─────────────────────────────────────────────────
// Represents a live viewport slot owned by a tool.

struct ViewportSlot {
    ViewportHandle        handle     = kInvalidViewportHandle;
    std::string           toolId;         // owning tool
    ViewportBounds        bounds;
    ViewportState         state      = ViewportState::Idle;
    ViewportRenderMode    renderMode = ViewportRenderMode::Lit;
    ViewportCameraDescriptor camera;
    ViewportGridDescriptor   grid;
    uint32_t              frameCount = 0; // frames rendered since activation

    [[nodiscard]] bool isValid()    const { return handle != kInvalidViewportHandle; }
    [[nodiscard]] bool isActive()   const { return state == ViewportState::Active;  }
    [[nodiscard]] bool isReleased() const { return state == ViewportState::Released; }

    void activate() {
        if (state != ViewportState::Released) state = ViewportState::Active;
    }
    void pause()   { if (isActive())  state = ViewportState::Paused; }
    void resume()  { if (state == ViewportState::Paused) state = ViewportState::Active; }

    void onFrameRendered() { if (isActive()) ++frameCount; }
};

// ── ViewportHostRegistry ──────────────────────────────────────────
// Workspace-owned registry that allocates and tracks viewport slots.

class ViewportHostRegistry {
public:
    static constexpr size_t kMaxSlots = 16;

    // Request a new viewport slot for the given tool.
    // Returns kInvalidViewportHandle if no slots are available.
    [[nodiscard]] ViewportHandle requestSlot(const std::string& toolId,
                                              const ViewportBounds& bounds) {
        if (m_slots.size() >= kMaxSlots) return kInvalidViewportHandle;
        if (toolId.empty())              return kInvalidViewportHandle;
        if (!bounds.isValid())           return kInvalidViewportHandle;

        ViewportSlot slot;
        slot.handle = ++m_nextHandle;
        slot.toolId = toolId;
        slot.bounds = bounds;
        slot.state  = ViewportState::Idle;
        m_slots.push_back(std::move(slot));
        return m_slots.back().handle;
    }

    // Release a viewport slot. Returns false if not found.
    bool releaseSlot(ViewportHandle handle) {
        for (auto it = m_slots.begin(); it != m_slots.end(); ++it) {
            if (it->handle == handle) {
                it->state = ViewportState::Released;
                m_slots.erase(it);
                return true;
            }
        }
        return false;
    }

    // Activate a slot — begin rendering.
    bool activateSlot(ViewportHandle handle) {
        if (auto* s = findSlot(handle)) { s->activate(); return true; }
        return false;
    }

    // Pause rendering for a slot.
    bool pauseSlot(ViewportHandle handle) {
        if (auto* s = findSlot(handle)) { s->pause(); return true; }
        return false;
    }

    // Resume rendering for a paused slot.
    bool resumeSlot(ViewportHandle handle) {
        if (auto* s = findSlot(handle)) { s->resume(); return true; }
        return false;
    }

    // Update bounds (e.g. on window resize).
    bool updateBounds(ViewportHandle handle, const ViewportBounds& bounds) {
        if (!bounds.isValid()) return false;
        if (auto* s = findSlot(handle)) { s->bounds = bounds; return true; }
        return false;
    }

    // Set render mode for a slot.
    bool setRenderMode(ViewportHandle handle, ViewportRenderMode mode) {
        if (auto* s = findSlot(handle)) { s->renderMode = mode; return true; }
        return false;
    }

    // Update camera descriptor for a slot.
    bool setCamera(ViewportHandle handle, const ViewportCameraDescriptor& camera) {
        if (!camera.isValid()) return false;
        if (auto* s = findSlot(handle)) { s->camera = camera; return true; }
        return false;
    }

    // Notify that a frame has been rendered (called by render loop).
    void onFrameRendered(ViewportHandle handle) {
        if (auto* s = findSlot(handle)) s->onFrameRendered();
    }

    // Lookups
    [[nodiscard]] const ViewportSlot* findSlot(ViewportHandle handle) const {
        for (const auto& s : m_slots) if (s.handle == handle) return &s;
        return nullptr;
    }
    [[nodiscard]] ViewportSlot* findSlot(ViewportHandle handle) {
        for (auto& s : m_slots) if (s.handle == handle) return &s;
        return nullptr;
    }

    [[nodiscard]] size_t slotCount()   const { return m_slots.size(); }
    [[nodiscard]] size_t activeCount() const {
        size_t n = 0;
        for (const auto& s : m_slots) if (s.isActive()) ++n;
        return n;
    }

    [[nodiscard]] const std::vector<ViewportSlot>& slots() const { return m_slots; }

    void clear() { m_slots.clear(); }

private:
    std::vector<ViewportSlot> m_slots;
    ViewportHandle m_nextHandle = 0;
};

} // namespace NF
