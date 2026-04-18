#pragma once
// NF::GDIViewportSurface — Windows GDI concrete IViewportSurface.
//
// Implements IViewportSurface for the GDI fallback render backend.
//
// Each viewport slot that needs a GDI surface owns one GDIViewportSurface.
// The surface is a Device-Independent Bitmap (DIB section) created via
// CreateDIBSection.  The scan-line buffer is accessible from the CPU, so
// placeholder "software rasterizer" draw calls can write directly into it.
//
// bind()   — selects the DIB into a memory DC for drawing.
// unbind() — BitBlts the DIB back to the target HWND/HDC at the panel rect.
// colorAttachment() — returns a uint32_t cookie (the HDC cast) that
//                     AtlasUI::ViewportPanel uses to identify the surface.
//                     In the GDI path "blitting the texture" means issuing
//                     another BitBlt from the stored HDC; the panel callback
//                     (set by WorkspaceViewportBridge) handles this.
//
// On non-Windows platforms this header is compiled but all methods are no-ops;
// the class degrades to a NullViewportSurface equivalent.
//
// NOTE: GDIViewportSurface is NOT intended for production rendering.
//       It is the reference implementation that proves the wiring end-to-end.
//       Replace with OpenGLViewportSurface / D3D11ViewportSurface when ready.

#include "NF/Workspace/IViewportSurface.h"

#if defined(_WIN32)
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#  include <windows.h>
#endif

#include <cstdint>
#include <cstring>

namespace NF {

class GDIViewportSurface final : public IViewportSurface {
public:
    GDIViewportSurface() = default;

    ~GDIViewportSurface() override {
        destroySurface();
    }

    // IViewportSurface ────────────────────────────────────────────────────────

    void resize(uint32_t w, uint32_t h) override {
        if (w == m_width && h == m_height) return;
        destroySurface();
        m_width  = w;
        m_height = h;
        createSurface();
    }

    void bind() override {
#if defined(_WIN32)
        if (m_memDC && m_dib)
            SelectObject(m_memDC, m_dib);
#endif
        m_bound = true;
        ++m_bindCount;
    }

    void unbind() override {
        m_bound = false;
#if defined(_WIN32)
        // BitBlt to the target HDC if one was set via setTargetDC().
        if (m_targetDC && m_memDC && m_dib && m_width > 0 && m_height > 0) {
            BitBlt(m_targetDC,
                   static_cast<int>(m_targetX), static_cast<int>(m_targetY),
                   static_cast<int>(m_width),   static_cast<int>(m_height),
                   m_memDC, 0, 0, SRCCOPY);
        }
#endif
    }

    [[nodiscard]] uint32_t colorAttachment() const override {
        // Return a non-zero opaque cookie when the surface is valid so
        // ViewportPanel::paint() can distinguish "has content" from "no surface".
        // The real blit is driven by unbind(); the cookie just signals readiness.
#if defined(_WIN32)
        return (m_dib && m_width > 0) ? static_cast<uint32_t>(
            reinterpret_cast<uintptr_t>(m_memDC) & 0xFFFFFFFFu) : 0u;
#else
        return (m_width > 0 && m_height > 0) ? 1u : 0u;
#endif
    }

    [[nodiscard]] bool     isValid() const override { return m_width > 0 && m_height > 0; }
    [[nodiscard]] uint32_t width()   const override { return m_width; }
    [[nodiscard]] uint32_t height()  const override { return m_height; }

    /// Copy a CPU-side BGRA pixel buffer into the DIB section scanline buffer
    /// so that unbind() → BitBlt displays the software-rendered content.
    /// `count` must equal width()*height(); excess pixels are ignored.
    /// No-op on non-Windows platforms (no DIB section exists).
    void writeScanlines(const uint32_t* pixels, size_t count) override {
#if defined(_WIN32)
        if (!m_bits || !pixels || m_width == 0 || m_height == 0) return;
        size_t expected = static_cast<size_t>(m_width) * m_height;
        if (count > expected) count = expected;
        std::memcpy(m_bits, pixels, count * sizeof(uint32_t));
#else
        (void)pixels;
        (void)count;
#endif
    }

    // ── GDI-specific helpers ──────────────────────────────────────────────────

    /// Set the target HDC and panel origin so unbind() can BitBlt the result.
    /// Call each frame before the frame loop runs (e.g. from renderAll).
#if defined(_WIN32)
    void setTargetDC(HDC dc, int x, int y) {
        m_targetDC = dc;
        m_targetX  = x;
        m_targetY  = y;
    }
    [[nodiscard]] HDC  memDC()    const { return m_memDC;    }
    [[nodiscard]] HBITMAP dib()   const { return m_dib;      }
    [[nodiscard]] void* scanlines() const { return m_bits;   }
#endif

    // Test helpers
    [[nodiscard]] bool     isBound()    const { return m_bound;     }
    [[nodiscard]] uint32_t bindCount()  const { return m_bindCount; }

private:
    void createSurface() {
#if defined(_WIN32)
        HDC screenDC = GetDC(nullptr);
        m_memDC = CreateCompatibleDC(screenDC);
        ReleaseDC(nullptr, screenDC);

        BITMAPINFO bmi = {};
        bmi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth       = static_cast<LONG>(m_width);
        bmi.bmiHeader.biHeight      = -static_cast<LONG>(m_height); // top-down
        bmi.bmiHeader.biPlanes      = 1;
        bmi.bmiHeader.biBitCount    = 32;
        bmi.bmiHeader.biCompression = BI_RGB;

        m_dib = CreateDIBSection(m_memDC, &bmi, DIB_RGB_COLORS, &m_bits, nullptr, 0);
        if (m_dib)
            SelectObject(m_memDC, m_dib);
#endif
    }

    void destroySurface() {
#if defined(_WIN32)
        if (m_dib) {
            if (m_memDC) {
                // Select a stock object before deleting the DIB to leave the DC
                // in a well-defined state (avoids selecting a deleted bitmap).
                SelectObject(m_memDC, GetStockObject(NULL_BRUSH));
            }
            DeleteObject(m_dib);
            m_dib = nullptr;
        }
        if (m_memDC) {
            DeleteDC(m_memDC);
            m_memDC = nullptr;
        }
        m_bits = nullptr;
#endif
        m_width  = 0;
        m_height = 0;
    }

    uint32_t m_width     = 0;
    uint32_t m_height    = 0;
    bool     m_bound     = false;
    uint32_t m_bindCount = 0;

#if defined(_WIN32)
    HDC     m_memDC    = nullptr;
    HBITMAP m_dib      = nullptr;
    void*   m_bits     = nullptr; // raw scan-line buffer (32-bit BGRA)
    HDC     m_targetDC = nullptr; // destination DC for BitBlt in unbind()
    int     m_targetX  = 0;
    int     m_targetY  = 0;
#endif
};

} // namespace NF
