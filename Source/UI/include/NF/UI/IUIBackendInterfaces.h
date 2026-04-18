#pragma once
// NF::IUIBackendInterfaces — Formal split of the UI backend contract.
//
// The monolithic UIBackend combines several concerns.  These interfaces
// separate them so that composited backends can mix implementations:
//
//   IFrameBackend       — window / device lifecycle and frame boundaries
//   IGeometryBackend    — batched vertex/index geometry submission
//   ITextRenderBackend  — font loading, text drawing, and measurement
//   ITextureBackend     — texture upload, binding, and destruction
//
// Usage:
//   A platform backend (e.g. D3D11Backend) implements all four.
//   A text-only backend (e.g. DirectWriteTextBackend) implements ITextRenderBackend.
//   Tests can implement individual interfaces in isolation.
//
// The legacy UIBackend base class composes all four through virtual dispatch.
// New code should program against these granular interfaces where possible.

#include "NF/UI/UI.h"
#include <string>
#include <cstdint>
#include <cstddef>

namespace NF {

// ── IFrameBackend ─────────────────────────────────────────────────
// Owns device / window lifecycle and frame boundaries.
// Implemented by platform backends (D3D11, GDI, OpenGL, Null).

class IFrameBackend {
public:
    virtual ~IFrameBackend() = default;

    // Initialize the device and allocate render resources.
    // Returns true on success.
    virtual bool init(int width, int height) = 0;

    // Release all render resources.
    virtual void shutdown() = 0;

    // Signal the start of a new frame. Implementations typically clear
    // the back buffer and update the viewport / projection matrix here.
    virtual void beginFrame(int width, int height) = 0;

    // Signal the end of a frame. Implementations present / swap buffers here.
    virtual void endFrame() = 0;

    // Human-readable name reported in diagnostics and logs.
    [[nodiscard]] virtual const char* backendName() const = 0;

    // Returns true when the backend uses GPU-accelerated rendering.
    [[nodiscard]] virtual bool isGPUAccelerated() const { return false; }
};

// ── IGeometryBackend ─────────────────────────────────────────────
// Accepts batched vertex/index geometry from the UIRenderer and submits
// it to the underlying graphics API.

class IGeometryBackend {
public:
    virtual ~IGeometryBackend() = default;

    // Submit one frame's worth of UI geometry.
    // vertices  — array of UIVertex (pos, uv, color)
    // indices   — index list forming GL_TRIANGLES / D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST
    virtual void flush(const UIVertex* vertices, size_t vertexCount,
                       const uint32_t* indices,  size_t indexCount) = 0;

    // Clip future draws to a screen-space rect.
    // Implementations map to scissor rect / clip rect APIs.
    virtual void setClipRect(float x, float y, float w, float h) {
        (void)x; (void)y; (void)w; (void)h;
    }

    // Remove the active clip rect.
    virtual void clearClipRect() {}
};

// ── ITextRenderBackend ───────────────────────────────────────────
// Handles font loading and native text rasterisation.
// Separated from geometry so a DirectWrite backend can be composed
// with a D3D11 geometry backend without coupling.

class ITextRenderBackend {
public:
    virtual ~ITextRenderBackend() = default;

    // Load and cache a named font at the given pixel size.
    virtual bool loadFont(const std::string& fontName, float fontSize) {
        (void)fontName; (void)fontSize;
        return false;
    }

    // Draw a string at pixel coordinates (x, y).
    // color is RRGGBBAA.
    virtual void drawText(const std::string& text, float x, float y,
                          float size, uint32_t color) = 0;

    // Measure the pixel width of a string at a given size.
    [[nodiscard]] virtual float measureTextWidth(const std::string& text,
                                                  float size) const = 0;

    // Return the line height for a given font size.
    [[nodiscard]] virtual float lineHeight(float size) const = 0;
};

// ── ITextureBackend ──────────────────────────────────────────────
// Manages GPU texture objects for the UI system (icons, font atlases, thumbnails).

class ITextureBackend {
public:
    virtual ~ITextureBackend() = default;

    // Upload RGBA8 pixel data and return a backend-defined texture handle.
    // Returns 0 on failure.
    virtual uint32_t uploadTexture(const uint8_t* pixels,
                                   int width, int height) = 0;

    // Free a previously uploaded texture.
    virtual void destroyTexture(uint32_t handle) = 0;

    // Bind a texture for the next draw call.
    // handle == 0 means "no texture" (solid color mode).
    virtual void bindTexture(uint32_t handle) = 0;
};

// ── NullTextRenderBackend ────────────────────────────────────────
// No-op implementation used for headless tests.

class NullTextRenderBackend final : public ITextRenderBackend {
public:
    void drawText(const std::string&, float, float, float, uint32_t) override {}

    [[nodiscard]] float measureTextWidth(const std::string& text,
                                          float /*size*/) const override {
        // 8 px per character approximation
        return static_cast<float>(text.size()) * 8.f;
    }

    [[nodiscard]] float lineHeight(float size) const override {
        return size * 1.2f;
    }
};

// ── NullTextureBackend ───────────────────────────────────────────
// No-op texture backend for headless tests.

class NullTextureBackend final : public ITextureBackend {
public:
    uint32_t uploadTexture(const uint8_t*, int, int) override { return 1u; }
    void destroyTexture(uint32_t) override {}
    void bindTexture(uint32_t) override {}
};

} // namespace NF
