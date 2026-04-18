#pragma once
// NF::IViewportSurface — Viewport ↔ Renderer surface bridge (Item 1 of 10).
//
// Connects ViewportHostRegistry slots to the actual render backend by mapping
// ViewportHandle → offscreen framebuffer.  When a slot is activated the frame
// loop calls bind/unbind on the associated surface.  Resize events propagate
// from ViewportPanel through ViewportHostRegistry::updateBounds() and then to
// ViewportSurfaceRegistry::resize(), which resizes the underlying FBO.
//
// The NullViewportSurface is the testable reference implementation; real
// backends (OpenGL, D3D11, Vulkan) provide concrete subclasses.

#include "NF/Workspace/ViewportHostContract.h"
#include <vector>

namespace NF {

// ── IViewportSurface ──────────────────────────────────────────────────────────
// Abstract offscreen surface that a viewport slot renders into.

class IViewportSurface {
public:
    virtual ~IViewportSurface() = default;

    /// Resize the surface.  Only call when the panel bounds actually change.
    virtual void resize(uint32_t width, uint32_t height) = 0;

    /// Bind this surface as the active render target.
    virtual void bind() = 0;

    /// Unbind and restore the default render target.
    virtual void unbind() = 0;

    /// Returns a backend-specific color attachment handle (e.g. GL texture name).
    /// Zero means the surface has not been created or is invalid.
    [[nodiscard]] virtual uint32_t colorAttachment() const = 0;

    /// Returns true if the surface is usable (has positive dimensions).
    [[nodiscard]] virtual bool isValid() const = 0;

    [[nodiscard]] virtual uint32_t width()  const = 0;
    [[nodiscard]] virtual uint32_t height() const = 0;

    /// Write a CPU-side pixel buffer (BGRA, top-down) into the surface's
    /// backing store.  The buffer must contain exactly width()*height() pixels.
    ///
    /// Used by SoftwareViewportRenderer to push its grid pixels into the
    /// GDI DIB section so they appear on screen after unbind() → BitBlt.
    /// GPU-backed surfaces may leave this as a no-op if they use shaders instead.
    ///
    /// Default: no-op.  Override in concrete surfaces that support CPU writes.
    virtual void writeScanlines(const uint32_t* /*pixels*/, size_t /*count*/) {}
};

// ── NullViewportSurface ───────────────────────────────────────────────────────
// Headless / CI / test implementation.  All GPU operations are no-ops.

class NullViewportSurface final : public IViewportSurface {
public:
    explicit NullViewportSurface(uint32_t width = 0, uint32_t height = 0)
        : m_width(width), m_height(height) {}

    void resize(uint32_t w, uint32_t h) override {
        m_width = w;
        m_height = h;
    }

    void bind() override {
        ++m_bindCount;
        m_bound = true;
    }

    void unbind() override {
        m_bound = false;
    }

    [[nodiscard]] uint32_t colorAttachment() const override { return m_colorAttachment; }
    [[nodiscard]] bool isValid() const override { return m_width > 0 && m_height > 0; }
    [[nodiscard]] uint32_t width()  const override { return m_width; }
    [[nodiscard]] uint32_t height() const override { return m_height; }

    // ── Test helpers ─────────────────────────────────────────────────────────
    [[nodiscard]] bool isBound()     const { return m_bound; }
    [[nodiscard]] uint32_t bindCount() const { return m_bindCount; }

    /// Simulate a backend assigning a texture handle after creation.
    void setColorAttachment(uint32_t id) { m_colorAttachment = id; }

private:
    uint32_t m_width  = 0;
    uint32_t m_height = 0;
    uint32_t m_colorAttachment = 0;
    bool     m_bound     = false;
    uint32_t m_bindCount = 0;
};

// ── ViewportSurfaceRegistry ───────────────────────────────────────────────────
// Workspace-owned registry mapping ViewportHandle → IViewportSurface*.
// Surfaces are NOT owned by the registry (lifetime managed by the caller).

class ViewportSurfaceRegistry {
public:
    static constexpr size_t kMaxSurfaces = 16;

    /// Register a surface for the given handle.
    /// Returns false if handle already registered or capacity reached.
    bool registerSurface(ViewportHandle handle, IViewportSurface* surface) {
        if (!surface) return false;
        if (handle == kInvalidViewportHandle) return false;
        if (m_entries.size() >= kMaxSurfaces) return false;
        for (const auto& e : m_entries)
            if (e.handle == handle) return false;
        m_entries.push_back({handle, surface});
        return true;
    }

    /// Unregister and return true, or false if not found.
    bool unregisterSurface(ViewportHandle handle) {
        for (auto it = m_entries.begin(); it != m_entries.end(); ++it) {
            if (it->handle == handle) {
                m_entries.erase(it);
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] IViewportSurface* findSurface(ViewportHandle handle) const {
        for (const auto& e : m_entries)
            if (e.handle == handle) return e.surface;
        return nullptr;
    }

    /// Resize the surface for the given handle.  Returns false if not found.
    bool resize(ViewportHandle handle, uint32_t w, uint32_t h) {
        if (auto* s = findSurface(handle)) {
            s->resize(w, h);
            return true;
        }
        return false;
    }

    [[nodiscard]] size_t surfaceCount() const { return m_entries.size(); }
    void clear() { m_entries.clear(); }

private:
    struct Entry {
        ViewportHandle   handle;
        IViewportSurface* surface;
    };
    std::vector<Entry> m_entries;
};

} // namespace NF
