#pragma once
// NF::DirectWriteTextBackend — Primary text rendering backend.
//
// This is the intended primary text rendering path for Atlas Workspace.
// Uses DirectWrite for font loading, glyph layout, shaping, and measurement.
// Integrates with the D3D11Backend via an interop surface (IDWriteBitmapRenderTarget
// or D2D1 render target backed by the D3D11 shared texture).
//
// Platform: Windows only (_WIN32).
//
// Architecture:
//   DirectWriteTextBackend implements ITextRenderBackend.
//   Attach to D3D11Backend via D3D11Backend::setTextBackend().
//
// DirectWrite object hierarchy:
//   IDWriteFactory         — root factory (singleton per process)
//   IDWriteFontCollection  — system / custom font collection
//   IDWriteTextFormat      — named font + size + alignment
//   IDWriteTextLayout      — shaped and measured text block
//   IDWriteBitmapRenderTarget — software rasterisation target
//     or
//   ID2D1RenderTarget      — hardware-accelerated target via D2D1+D3D11 interop
//   ID2D1SolidColorBrush   — per-color draw brush (cached by color)
//
// Glyph atlas strategy (planned):
//   Pre-rasterise ASCII printable glyphs into a greyscale texture atlas.
//   Upload to D3D11 via ITextureBackend::uploadTexture().
//   Text drawing then emits quads into the geometry backend (fast path).
//   Fallback to IDWriteBitmapRenderTarget for non-ASCII / large sizes.
//
// Implementation note:
//   This file is an architecturally-complete stub.  All COM types are
//   commented out so the file compiles on non-Windows platforms.
//   Replace stub bodies with real COM calls when targeting Windows.

#include "NF/Core/Core.h"
#include "NF/UI/IUIBackendInterfaces.h"
#include <string>
#include <unordered_map>
#include <cstdint>

// Forward-declare COM interfaces used in the Windows implementation.
struct IDWriteFactory;
struct IDWriteFontCollection;
struct IDWriteTextFormat;
struct IWICImagingFactory;

namespace NF {

// ── NullTextRenderBackend is already defined in IUIBackendInterfaces.h ────

#ifdef _WIN32

// ── DirectWriteTextBackend ────────────────────────────────────────

class DirectWriteTextBackend final : public ITextRenderBackend {
public:
    // ── ITextRenderBackend ────────────────────────────────────────

    bool loadFont(const std::string& fontName, float fontSize) override {
        m_fontName = fontName;
        m_fontSize = fontSize;

        // Real implementation:
        //   if (!m_factory) DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED,
        //       __uuidof(IDWriteFactory), (IUnknown**)&m_factory);
        //
        //   IDWriteTextFormat* fmt = nullptr;
        //   m_factory->CreateTextFormat(
        //       widen(fontName).c_str(),   // family name
        //       nullptr,                   // system font collection
        //       DWRITE_FONT_WEIGHT_NORMAL,
        //       DWRITE_FONT_STYLE_NORMAL,
        //       DWRITE_FONT_STRETCH_NORMAL,
        //       fontSize,
        //       L"en-us",
        //       &fmt);
        //   m_formats[key] = fmt;

        NF_LOG_INFO("UI", "DirectWriteTextBackend::loadFont stub — "
                    + fontName + " @ " + std::to_string(fontSize));
        return false; // returns true once COM initialisation is complete
    }

    void drawText(const std::string& text, float x, float y,
                  float size, uint32_t color) override {
        (void)color;

        // Real implementation — software rasterisation path:
        //   IDWriteTextLayout* layout = nullptr;
        //   auto wtext = widen(text);
        //   m_factory->CreateTextLayout(wtext.c_str(), wtext.size(),
        //       m_formats[{m_fontName, size}],
        //       FLT_MAX, FLT_MAX, &layout);
        //
        //   // Render into IDWriteBitmapRenderTarget, then blit to screen.
        //   m_renderTarget->DrawTextLayout({x, y}, layout, m_brush);
        //   layout->Release();
        //
        // Hardware path (D2D1 + D3D11 interop):
        //   m_d2dRenderTarget->BeginDraw();
        //   m_d2dRenderTarget->DrawTextLayout({x, y}, layout, m_brush);
        //   m_d2dRenderTarget->EndDraw();

        (void)text; (void)x; (void)y; (void)size;
    }

    [[nodiscard]] float measureTextWidth(const std::string& text,
                                          float size) const override {
        // Real implementation:
        //   IDWriteTextLayout* layout = nullptr;
        //   m_factory->CreateTextLayout(wtext, len, m_formats[key],
        //       FLT_MAX, FLT_MAX, &layout);
        //   DWRITE_TEXT_METRICS metrics{};
        //   layout->GetMetrics(&metrics);
        //   layout->Release();
        //   return metrics.widthIncludingTrailingWhitespace;

        // Stub approximation (8 px per character)
        (void)size;
        return static_cast<float>(text.size()) * 8.f;
    }

    [[nodiscard]] float lineHeight(float size) const override {
        // Real implementation:
        //   IDWriteTextLayout* layout = nullptr;
        //   m_factory->CreateTextLayout(L"Wg", 2, m_formats[key],
        //       FLT_MAX, FLT_MAX, &layout);
        //   DWRITE_LINE_METRICS lm{};
        //   UINT32 count = 1;
        //   layout->GetLineMetrics(&lm, 1, &count);
        //   layout->Release();
        //   return lm.height;

        return size * 1.2f; // stub approximation
    }

    // ── Lifecycle ─────────────────────────────────────────────────

    bool init() {
        // Real implementation:
        //   HRESULT hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED,
        //       __uuidof(IDWriteFactory), (IUnknown**)&m_factory);
        //   return SUCCEEDED(hr);
        NF_LOG_WARN("UI", "DirectWriteTextBackend: stub — real DirectWrite not compiled in");
        return false;
    }

    void shutdown() {
        // Real implementation: SafeRelease all m_formats, m_factory, m_brush.
        m_formats.clear();
        NF_LOG_INFO("UI", "DirectWriteTextBackend shutdown");
    }

    [[nodiscard]] bool isInitialized() const { return m_initialized; }

private:
    struct FontKey {
        std::string name;
        float       size = 0.f;
        bool operator==(const FontKey& o) const {
            return name == o.name && size == o.size;
        }
    };

    // Hash combination constant: golden-ratio mixing for better distribution.
    // This is the standard boost::hash_combine technique.
    struct FontKeyHash {
        std::size_t operator()(const FontKey& k) const {
            std::size_t h = std::hash<std::string>{}(k.name);
            h ^= std::hash<float>{}(k.size) + 0x9e3779b9u + (h << 6) + (h >> 2);
            return h;
        }
    };

    std::string m_fontName  = "Consolas";
    float       m_fontSize  = 14.f;
    bool        m_initialized = false;

    // COM handles (populated when real DirectWrite is available):
    IDWriteFactory*       m_factory = nullptr;
    // IDWriteBitmapRenderTarget* m_renderTarget = nullptr;
    // ID2D1SolidColorBrush*      m_brush = nullptr;

    // Cached text formats keyed by {family, size}
    std::unordered_map<FontKey, IDWriteTextFormat*, FontKeyHash> m_formats;
};

#endif // _WIN32

} // namespace NF

