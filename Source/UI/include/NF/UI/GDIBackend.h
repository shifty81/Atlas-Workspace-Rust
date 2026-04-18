#pragma once
// NF::GDIBackend — Win32 GDI fallback rendering backend for the UI system.
// Renders batched UI quads using double-buffered GDI (FillRect + TextOutA).
// Only compiled on Windows.
//
// NOTE: This is the FALLBACK/BOOTSTRAP backend. The intended primary backend
// is D3D11 + DirectWrite. GDI is kept for recovery and initial bring-up only.
#include "NF/UI/UIBackend.h"

#ifdef _WIN32
#include <windows.h>

namespace NF {

class GDIBackend final : public UIBackend {
public:
    bool init(int width, int height) override {
        m_width  = width;
        m_height = height;
        NF_LOG_INFO("UI", "GDIBackend initialized (" + std::to_string(width) + "x" +
                    std::to_string(height) + ")");
        return true;
    }

    void shutdown() override {
        releaseResources();
        NF_LOG_INFO("UI", "GDIBackend shutdown");
    }

    void setWindowHandle(HWND hwnd) { m_hwnd = hwnd; }

    void setTargetDC(HDC hdc) { m_targetDC = hdc; }

    void beginFrame(int width, int height) override {
        m_width  = width;
        m_height = height;

        if (m_targetDC) {
            // Create off-screen buffer for double-buffering
            if (m_memDC) { DeleteDC(m_memDC); m_memDC = nullptr; }
            if (m_bitmap) { DeleteObject(m_bitmap); m_bitmap = nullptr; }

            m_memDC  = CreateCompatibleDC(m_targetDC);
            m_bitmap = CreateCompatibleBitmap(m_targetDC, width, height);
            m_oldBitmap = SelectObject(m_memDC, m_bitmap);

            // Clear to black
            RECT full{0, 0, width, height};
            HBRUSH bg = CreateSolidBrush(RGB(30, 30, 30));
            FillRect(m_memDC, &full, bg);
            DeleteObject(bg);

            // Set up text rendering
            SetBkMode(m_memDC, TRANSPARENT);
            if (!m_font) {
                m_font = CreateFontA(
                    13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                    ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                    CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Consolas");
            }
            m_oldFont = static_cast<HFONT>(SelectObject(m_memDC, m_font));
        }
    }

    void flush(const UIVertex* vertices, size_t vertexCount,
               const uint32_t* indices, size_t indexCount) override {
        if (!m_memDC || vertexCount == 0 || indexCount == 0) return;

        // Process triangles: for each pair of triangles (quad = 6 indices),
        // extract the bounding box and fill with the first vertex colour.
        for (size_t i = 0; i + 5 < indexCount; i += 6) {
            // A quad is indices [i..i+5] representing two triangles.
            uint32_t i0 = indices[i];
            uint32_t i2 = indices[i + 2];

            if (i0 >= vertexCount || i2 >= vertexCount) continue;

            const UIVertex& v0 = vertices[i0];
            const UIVertex& v2 = vertices[i2];

            // Extract axis-aligned bounds
            float minX = std::min(v0.position.x, v2.position.x);
            float minY = std::min(v0.position.y, v2.position.y);
            float maxX = std::max(v0.position.x, v2.position.x);
            float maxY = std::max(v0.position.y, v2.position.y);

            int left   = static_cast<int>(minX);
            int top    = static_cast<int>(minY);
            int right  = static_cast<int>(maxX);
            int bottom = static_cast<int>(maxY);

            if (right <= left || bottom <= top) continue;

            // Extract colour (RGBA → RGB)
            uint32_t c = v0.color;
            uint8_t r = static_cast<uint8_t>((c >> 24) & 0xFF);
            uint8_t g = static_cast<uint8_t>((c >> 16) & 0xFF);
            uint8_t b = static_cast<uint8_t>((c >>  8) & 0xFF);

            RECT rect{left, top, right, bottom};
            HBRUSH brush = CreateSolidBrush(RGB(r, g, b));
            FillRect(m_memDC, &rect, brush);
            DeleteObject(brush);
        }
    }

    void endFrame() override {
        if (m_targetDC && m_memDC) {
            BitBlt(m_targetDC, 0, 0, m_width, m_height, m_memDC, 0, 0, SRCCOPY);

            if (m_oldFont) SelectObject(m_memDC, m_oldFont);
            if (m_oldBitmap) SelectObject(m_memDC, m_oldBitmap);
            if (m_memDC) { DeleteDC(m_memDC); m_memDC = nullptr; }
            if (m_bitmap) { DeleteObject(m_bitmap); m_bitmap = nullptr; }
            m_oldFont = nullptr;
            m_oldBitmap = nullptr;
        }
    }

    void loadFont(const std::string& fontName, float fontSize) override {
        if (m_font) { DeleteObject(m_font); m_font = nullptr; }
        m_font = CreateFontA(
            static_cast<int>(fontSize), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, fontName.c_str());
    }

    [[nodiscard]] Vec2 measureText(std::string_view text, float fontSize) const override {
        if (m_memDC) {
            SIZE sz{};
            GetTextExtentPoint32A(m_memDC, text.data(), static_cast<int>(text.size()), &sz);
            return {static_cast<float>(sz.cx), static_cast<float>(sz.cy)};
        }
        return UIBackend::measureText(text, fontSize);
    }

    // Convenience: draw native GDI text at a position (used during flush for annotated text)
    void drawTextGDI(float x, float y, std::string_view text, uint32_t color) {
        if (!m_memDC) return;
        uint8_t r = static_cast<uint8_t>((color >> 24) & 0xFF);
        uint8_t g = static_cast<uint8_t>((color >> 16) & 0xFF);
        uint8_t b = static_cast<uint8_t>((color >>  8) & 0xFF);
        SetTextColor(m_memDC, RGB(r, g, b));
        TextOutA(m_memDC, static_cast<int>(x), static_cast<int>(y),
                 text.data(), static_cast<int>(text.size()));
    }

    // UIBackend override: route AtlasUI DrawTextCmd directly to GDI TextOutA.
    void drawTextNative(float x, float y, std::string_view text, uint32_t color) override {
        drawTextGDI(x, y, text, color);
    }

    // Direct access to the memory DC for advanced rendering
    [[nodiscard]] HDC memDC() const { return m_memDC; }

    [[nodiscard]] const char* backendName() const override { return "Win32 GDI"; }
    [[nodiscard]] bool isGPUAccelerated() const override { return false; }

private:
    void releaseResources() {
        if (m_font) { DeleteObject(m_font); m_font = nullptr; }
        if (m_memDC) { DeleteDC(m_memDC); m_memDC = nullptr; }
        if (m_bitmap) { DeleteObject(m_bitmap); m_bitmap = nullptr; }
    }

    HWND    m_hwnd      = nullptr;
    HDC     m_targetDC  = nullptr;
    HDC     m_memDC     = nullptr;
    HBITMAP m_bitmap    = nullptr;
    HFONT   m_font      = nullptr;
    HFONT   m_oldFont   = nullptr;
    HGDIOBJ m_oldBitmap = nullptr;
    int     m_width     = 0;
    int     m_height    = 0;
};

} // namespace NF

#endif // _WIN32
