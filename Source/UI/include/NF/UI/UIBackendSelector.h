#pragma once
// NF::UIBackendSelector — Centralized backend selection for the UI system.
//
// Selection priority (highest to lowest):
//   1. D3D11 + DirectWrite  — primary (Windows, GPU-first)
//   2. OpenGL               — compatibility (cross-platform, not production)
//   3. GDI                  — fallback / bootstrap (Windows only)
//   4. Null                 — headless / testing
//
// Usage:
//   auto backend = NF::selectUIBackend(NF::UIBackendType::Auto);
//   if (!backend) { /* null = headless mode */ }
//
// Composited backend creation (primary path):
//   auto pair = NF::createD3D11WithDirectWrite(width, height);
//   // pair.geom is the D3D11Backend (IFrameBackend + IGeometryBackend + ITextureBackend)
//   // pair.text is the DirectWriteTextBackend (ITextRenderBackend)
//
// Backend type query:
//   NF::UIBackendType preferred = NF::UIBackendType::D3D11;
//   const char* name = NF::uiBackendTypeName(preferred);

#include "NF/UI/UIBackend.h"
#include "NF/UI/IUIBackendInterfaces.h"
#include "NF/UI/D3D11Backend.h"
#include "NF/UI/DirectWriteTextBackend.h"
#include "NF/UI/GDIBackend.h"
#include <memory>
#include <string>

namespace NF {

// ── UIBackendType ─────────────────────────────────────────────────

enum class UIBackendType {
    Auto,    // Let selector pick best available
    D3D11,   // Primary target (Windows + D3D11, GPU-first)
    OpenGL,  // Compatibility backend (not for production)
    GDI,     // Fallback / bootstrap (Windows only)
    Null     // Headless / testing
};

inline const char* uiBackendTypeName(UIBackendType t) {
    switch (t) {
    case UIBackendType::Auto:   return "Auto";
    case UIBackendType::D3D11:  return "D3D11";
    case UIBackendType::OpenGL: return "OpenGL (compat)";
    case UIBackendType::GDI:    return "GDI (fallback)";
    case UIBackendType::Null:   return "Null";
    }
    return "Unknown";
}

// ── Platform capability probes ────────────────────────────────────

inline bool platformSupportsD3D11() {
#ifdef _WIN32
    // Real implementation: attempt D3D11CreateDevice with D3D_DRIVER_TYPE_HARDWARE.
    // Return true if SUCCEEDED(hr).
    return false; // stub: always false until D3D11 init is compiled in
#else
    return false;
#endif
}

inline bool platformSupportsGDI() {
#ifdef _WIN32
    return true; // GDI is always available on Win32
#else
    return false;
#endif
}

// ── selectUIBackend ───────────────────────────────────────────────
// Selects and creates the best available UIBackend.
// Returns nullptr only when type == Null (headless / testing).
//
// When 'preferred' is Auto, the highest-priority available backend wins.
// When 'preferred' is a specific type, the selector falls back if unavailable.

inline std::unique_ptr<UIBackend> selectUIBackend(
    UIBackendType preferred = UIBackendType::Auto)
{
    // Determine effective backend type
    UIBackendType effective = preferred;
    if (effective == UIBackendType::Auto) {
        if      (platformSupportsD3D11()) effective = UIBackendType::D3D11;
        else if (platformSupportsGDI())   effective = UIBackendType::GDI;
        else                              effective = UIBackendType::Null;
    }

    // Fall back through the priority chain when a backend is unavailable
    if (effective == UIBackendType::D3D11 && !platformSupportsD3D11()) {
        NF_LOG_WARN("UI", "D3D11 not available — falling back to GDI");
        effective = platformSupportsGDI() ? UIBackendType::GDI : UIBackendType::Null;
    }
    if (effective == UIBackendType::GDI && !platformSupportsGDI()) {
        NF_LOG_WARN("UI", "GDI not available — falling back to Null");
        effective = UIBackendType::Null;
    }

    NF_LOG_INFO("UI", std::string("UIBackendSelector: selected ")
                      + uiBackendTypeName(effective));

    switch (effective) {
    case UIBackendType::D3D11:
        // Real implementation: return std::make_unique<D3D11Backend>();
        // (include D3D11Backend.h in the .cpp, not here, to avoid Win32 header leakage)
        NF_LOG_WARN("UI", "D3D11Backend creation deferred to platform init");
        return nullptr;

    case UIBackendType::OpenGL:
        // Real implementation: return std::make_unique<OpenGLBackend>();
        // (compat path — not for production)
        NF_LOG_WARN("UI", "OpenGLBackend (compat) — not created by selector");
        return nullptr;

    case UIBackendType::GDI:
        // Instantiate GDIBackend — the Win32 fallback/bootstrap render path.
#if defined(_WIN32)
        return std::make_unique<GDIBackend>();
#else
        NF_LOG_WARN("UI", "GDIBackend requested on non-Win32 platform — falling back to Null");
        return std::make_unique<NullBackend>();
#endif

    case UIBackendType::Null:
    default:
        return std::make_unique<NullBackend>();
    }
}

// ── BackendCapabilities ───────────────────────────────────────────
// Describes what a selected backend type supports.

struct BackendCapabilities {
    bool gpuAccelerated   = false;
    bool nativeTextLayout = false; // DirectWrite / Core Text
    bool textureAtlas     = false;
    bool scissorClip      = false;
    bool vsync            = false;
};

inline BackendCapabilities queryCapabilities(UIBackendType t) {
    switch (t) {
    case UIBackendType::D3D11:
        return { true,  true,  true,  true,  true  };
    case UIBackendType::OpenGL:
        return { true,  false, true,  true,  true  };
    case UIBackendType::GDI:
        return { false, false, false, false, false };
    case UIBackendType::Null:
    default:
        return { false, false, false, false, false };
    }
}


#ifdef _WIN32

// ── D3D11WithDirectWritePair ──────────────────────────────────────
// Composited primary backend pair: D3D11 geometry + DirectWrite text.
// Both objects share ownership and should be kept alive together.
//
// The D3D11Backend's text delegate is set to the DirectWriteTextBackend
// automatically by createD3D11WithDirectWrite().
//
// Usage (happy path):
//   auto pair = NF::createD3D11WithDirectWrite(1280, 720);
//   if (pair.geom && pair.geom->init(1280, 720)) { /* render loop */ }

struct D3D11WithDirectWritePair {
    std::unique_ptr<D3D11Backend>           geom; ///< Geometry + frame backend
    std::unique_ptr<DirectWriteTextBackend> text; ///< Text rendering backend

    /// Returns true if both backends were successfully created (not necessarily
    /// initialised — call geom->init() to complete device initialisation).
    [[nodiscard]] bool isCreated() const { return geom && text; }
};

/// Create a composited D3D11 + DirectWrite backend pair.
///
/// This is the primary creation path for Atlas Workspace on Windows.
///
/// Caller must invoke pair.geom->init(width, height) before rendering.
/// On a stub build (no real D3D11), init() returns false and logging
/// records the deferred-init warning — the pair is still usable as a
/// contract-level stub for testing.
inline D3D11WithDirectWritePair createD3D11WithDirectWrite(int width, int height)
{
    (void)width; (void)height;
    auto geom = std::make_unique<D3D11Backend>();
    auto text = std::make_unique<DirectWriteTextBackend>();
    geom->setTextBackend(text.get());
    return { std::move(geom), std::move(text) };
}

#endif // _WIN32

} // namespace NF

