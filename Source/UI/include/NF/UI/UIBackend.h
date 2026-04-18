#pragma once
// NF::UIBackend — Abstract rendering backend for the UI system.
// Concrete implementations:
//   - D3D11Backend    (primary target — not yet implemented)
//   - GDIBackend      (fallback/bootstrap — Win32 only)
//   - OpenGLBackend   (compatibility — cross-platform)
#include "NF/Core/Core.h"
#include "NF/UI/UI.h"

namespace NF {

// ── UIBackend (abstract) ─────────────────────────────────────────
// All UI drawing ultimately flows through a UIBackend.  The UIRenderer
// batches vertices/indices per frame and calls flush() once per frame,
// allowing the backend to render them in a platform-specific way.

class UIBackend {
public:
    virtual ~UIBackend() = default;

    // Lifecycle
    virtual bool init(int width, int height) = 0;
    virtual void shutdown() = 0;

    // Frame boundary
    virtual void beginFrame(int width, int height) = 0;
    virtual void endFrame() = 0;   // present / swapbuffers

    // Flush batched UI geometry to the screen.
    virtual void flush(const UIVertex* vertices, size_t vertexCount,
                       const uint32_t* indices,  size_t indexCount) = 0;

    // Text support
    virtual void loadFont(const std::string& fontName, float fontSize) {
        (void)fontName; (void)fontSize;
    }

    // Draw text natively at pixel coordinates using the platform backend.
    // Called by DrawListDispatcher for AtlasUI DrawTextCmd commands.
    // The color is in RRGGBBAA format (same as UIVertex color).
    // Override in platform backends (e.g. GDIBackend) for proper text rendering.
    virtual void drawTextNative(float x, float y, std::string_view text, uint32_t color) {
        (void)x; (void)y; (void)text; (void)color; // default: no-op
    }

    // Measure text extents (for layout calculations).
    [[nodiscard]] virtual Vec2 measureText(std::string_view text, float fontSize) const {
        (void)fontSize;
        constexpr float kCharWidth  = 8.f;
        constexpr float kCharHeight = 14.f;
        float maxW = 0.f;
        float cx = 0.f;
        float lines = 1.f;
        for (char ch : text) {
            if (ch == '\n') { maxW = std::max(maxW, cx); cx = 0.f; lines += 1.f; continue; }
            cx += kCharWidth;
        }
        maxW = std::max(maxW, cx);
        return {maxW, lines * (kCharHeight + 2.f)};
    }

    // Query
    [[nodiscard]] virtual const char* backendName() const = 0;
    [[nodiscard]] virtual bool isGPUAccelerated() const { return false; }
};

// ── NullBackend ──────────────────────────────────────────────────
// Default no-op backend used when no platform backend is configured.
// Useful for headless tests and CI.

class NullBackend final : public UIBackend {
public:
    bool init(int w, int h) override { m_w = w; m_h = h; return true; }
    void shutdown() override {}
    void beginFrame(int w, int h) override { m_w = w; m_h = h; }
    void endFrame() override {}
    void flush(const UIVertex*, size_t, const uint32_t*, size_t) override {}
    [[nodiscard]] const char* backendName() const override { return "Null"; }

private:
    int m_w = 0, m_h = 0;
};

} // namespace NF
