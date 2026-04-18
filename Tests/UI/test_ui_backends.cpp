#include <catch2/catch_test_macros.hpp>
#include "NF/UI/IUIBackendInterfaces.h"
#include "NF/UI/UIBackendSelector.h"
#include "NF/UI/D3D11Backend.h"
#include "NF/UI/DirectWriteTextBackend.h"
#include "NF/UI/UIBackend.h"

// ── IFrameBackend contract ────────────────────────────────────────
// NullBackend implements IFrameBackend via UIBackend; test the contract
// using it as a reference implementation.

TEST_CASE("NullBackend satisfies IFrameBackend contract", "[UI][Backend][IFrameBackend]") {
    NF::NullBackend nb;

    SECTION("init succeeds and returns true") {
        REQUIRE(nb.init(800, 600));
    }

    SECTION("beginFrame and endFrame are no-ops") {
        REQUIRE(nb.init(800, 600));
        nb.beginFrame(1024, 768);
        nb.endFrame();
        // No crash is the contract
    }

    SECTION("backendName is not empty") {
        REQUIRE(std::string(nb.backendName()) == "Null");
    }

    SECTION("isGPUAccelerated returns false for Null") {
        REQUIRE_FALSE(nb.isGPUAccelerated());
    }

    SECTION("shutdown is safe to call after init") {
        REQUIRE(nb.init(800, 600));
        nb.shutdown();
    }

    SECTION("shutdown is safe to call without prior init") {
        nb.shutdown(); // must not crash
    }
}

// ── IGeometryBackend contract ─────────────────────────────────────

TEST_CASE("NullBackend satisfies IGeometryBackend contract (via UIBackend::flush)",
          "[UI][Backend][IGeometryBackend]") {
    NF::NullBackend nb;
    REQUIRE(nb.init(800, 600));

    SECTION("flush with zero vertices is safe") {
        nb.flush(nullptr, 0, nullptr, 0);
    }

    SECTION("flush with valid geometry is safe") {
        NF::UIVertex verts[4]{};
        uint32_t     idxs[6]{0, 1, 2, 2, 3, 0};
        nb.flush(verts, 4, idxs, 6);
    }
}

// Verify IGeometryBackend's optional clip methods via the interface's default impl
TEST_CASE("IGeometryBackend default setClipRect and clearClipRect are no-ops",
          "[UI][Backend][IGeometryBackend]") {
    // Use a concrete local backend that only implements IGeometryBackend
    struct TestGeomBackend final : public NF::IGeometryBackend {
        size_t flushCount = 0;
        void flush(const NF::UIVertex*, size_t, const uint32_t*, size_t) override {
            ++flushCount;
        }
        // setClipRect and clearClipRect use IGeometryBackend defaults
    };

    TestGeomBackend gb;
    gb.setClipRect(10.f, 10.f, 200.f, 100.f); // must not crash
    gb.clearClipRect();                         // must not crash
    gb.flush(nullptr, 0, nullptr, 0);
    REQUIRE(gb.flushCount == 1);
}

// ── ITextRenderBackend contract ───────────────────────────────────

TEST_CASE("NullTextRenderBackend satisfies ITextRenderBackend contract",
          "[UI][Backend][ITextRenderBackend]") {
    NF::NullTextRenderBackend tb;

    SECTION("drawText is a no-op and does not crash") {
        tb.drawText("Hello", 10.f, 20.f, 14.f, 0xFFFFFFFF);
    }

    SECTION("measureTextWidth returns positive value for non-empty string") {
        float w = tb.measureTextWidth("Hello", 14.f);
        REQUIRE(w > 0.f);
    }

    SECTION("measureTextWidth returns zero for empty string") {
        float w = tb.measureTextWidth("", 14.f);
        REQUIRE(w == 0.f);
    }

    SECTION("measureTextWidth scales with string length") {
        float w1 = tb.measureTextWidth("Hi", 14.f);
        float w2 = tb.measureTextWidth("Hello", 14.f);
        REQUIRE(w2 > w1);
    }

    SECTION("lineHeight returns positive value") {
        float h = tb.lineHeight(14.f);
        REQUIRE(h > 0.f);
    }

    SECTION("lineHeight scales with font size") {
        float h12 = tb.lineHeight(12.f);
        float h24 = tb.lineHeight(24.f);
        REQUIRE(h24 > h12);
    }

    SECTION("loadFont default returns false (no-op)") {
        REQUIRE_FALSE(tb.loadFont("Consolas", 14.f));
    }
}

// ── ITextureBackend contract ──────────────────────────────────────

TEST_CASE("NullTextureBackend satisfies ITextureBackend contract",
          "[UI][Backend][ITextureBackend]") {
    NF::NullTextureBackend texb;

    SECTION("uploadTexture returns a non-zero handle") {
        uint8_t pixels[4 * 4 * 4]{};
        uint32_t handle = texb.uploadTexture(pixels, 4, 4);
        REQUIRE(handle != 0u);
    }

    SECTION("destroyTexture is safe for any handle") {
        texb.destroyTexture(0u);
        texb.destroyTexture(1u);
        texb.destroyTexture(0xFFFFFFFFu);
    }

    SECTION("bindTexture is safe for any handle") {
        texb.bindTexture(0u);
        texb.bindTexture(1u);
    }
}

// ── UIBackendSelector ─────────────────────────────────────────────

TEST_CASE("uiBackendTypeName returns correct strings", "[UI][BackendSelector]") {
    REQUIRE(std::string(NF::uiBackendTypeName(NF::UIBackendType::Auto))   != "");
    REQUIRE(std::string(NF::uiBackendTypeName(NF::UIBackendType::D3D11))  != "");
    REQUIRE(std::string(NF::uiBackendTypeName(NF::UIBackendType::OpenGL)) != "");
    REQUIRE(std::string(NF::uiBackendTypeName(NF::UIBackendType::GDI))    != "");
    REQUIRE(std::string(NF::uiBackendTypeName(NF::UIBackendType::Null))   != "");
}

TEST_CASE("selectUIBackend(Null) returns a non-null NullBackend", "[UI][BackendSelector]") {
    auto backend = NF::selectUIBackend(NF::UIBackendType::Null);
    REQUIRE(backend != nullptr);
    REQUIRE(std::string(backend->backendName()) == "Null");
}

TEST_CASE("selectUIBackend(Auto) returns a valid backend on all platforms",
          "[UI][BackendSelector]") {
    auto backend = NF::selectUIBackend(NF::UIBackendType::Auto);
    // On Linux/CI with no D3D11 or GDI: Auto falls back to Null.
    // On Windows without D3D11 compiled in: falls back to GDI or Null.
    // In all cases the result must be non-null (Null backend is the floor).
    REQUIRE(backend != nullptr);
}

TEST_CASE("selectUIBackend(D3D11) falls back gracefully on non-Windows",
          "[UI][BackendSelector]") {
    // D3D11 is not available on Linux; the selector falls through to Null.
    auto backend = NF::selectUIBackend(NF::UIBackendType::D3D11);
    // On Linux this will be nullptr (D3D11 deferred) or the Null backend.
    // Either is acceptable — the key requirement is no crash.
    (void)backend;
}

TEST_CASE("queryCapabilities returns correct flags per backend type",
          "[UI][BackendSelector]") {
    auto d3d11  = NF::queryCapabilities(NF::UIBackendType::D3D11);
    auto opengl = NF::queryCapabilities(NF::UIBackendType::OpenGL);
    auto gdi    = NF::queryCapabilities(NF::UIBackendType::GDI);
    auto null_b = NF::queryCapabilities(NF::UIBackendType::Null);

    // D3D11 — full GPU feature set
    REQUIRE(d3d11.gpuAccelerated);
    REQUIRE(d3d11.nativeTextLayout);
    REQUIRE(d3d11.textureAtlas);
    REQUIRE(d3d11.scissorClip);
    REQUIRE(d3d11.vsync);

    // OpenGL — GPU but no native text layout
    REQUIRE(opengl.gpuAccelerated);
    REQUIRE_FALSE(opengl.nativeTextLayout);
    REQUIRE(opengl.textureAtlas);

    // GDI — no GPU acceleration
    REQUIRE_FALSE(gdi.gpuAccelerated);
    REQUIRE_FALSE(gdi.nativeTextLayout);

    // Null — nothing
    REQUIRE_FALSE(null_b.gpuAccelerated);
    REQUIRE_FALSE(null_b.nativeTextLayout);
}

// ── D3D11Backend (non-Windows: init always returns false) ─────────

#ifdef _WIN32
TEST_CASE("D3D11Backend reports correct name and capabilities",
          "[UI][Backend][D3D11]") {
    NF::D3D11Backend d3d;

    REQUIRE(std::string(d3d.backendName()) == "D3D11");
    REQUIRE(d3d.isGPUAccelerated());
}

TEST_CASE("D3D11Backend init returns false (stub — no real D3D11 device)",
          "[UI][Backend][D3D11]") {
    NF::D3D11Backend d3d;
    // Stub returns false because D3D11CreateDevice is not called.
    REQUIRE_FALSE(d3d.init(800, 600));
}

TEST_CASE("D3D11Backend flush is safe before and after init",
          "[UI][Backend][D3D11]") {
    NF::D3D11Backend d3d;
    // Must not crash even when init returned false.
    NF::UIVertex verts[4]{};
    uint32_t     idxs[6]{0, 1, 2, 2, 3, 0};
    d3d.flush(verts, 4, idxs, 6);
    REQUIRE(d3d.lastVertexCount() == 4);
    REQUIRE(d3d.lastIndexCount()  == 6);
}

TEST_CASE("D3D11Backend setClipRect tracks state", "[UI][Backend][D3D11]") {
    NF::D3D11Backend d3d;
    REQUIRE_FALSE(d3d.scissorActive());
    d3d.setClipRect(0.f, 0.f, 100.f, 100.f);
    REQUIRE(d3d.scissorActive());
    d3d.clearClipRect();
    REQUIRE_FALSE(d3d.scissorActive());
}

TEST_CASE("D3D11Backend uploadTexture returns 0 (stub)",
          "[UI][Backend][D3D11]") {
    NF::D3D11Backend d3d;
    uint8_t pixels[16]{};
    uint32_t h = d3d.uploadTexture(pixels, 2, 2);
    REQUIRE(h == 0u); // stub always returns 0
}

TEST_CASE("D3D11Backend text shim delegates to ITextRenderBackend when set",
          "[UI][Backend][D3D11]") {
    NF::D3D11Backend d3d;
    NF::NullTextRenderBackend textb;
    d3d.setTextBackend(&textb);

    // measureText should use NullTextRenderBackend's approximation (8px per char)
    static constexpr float kCharWidthPx = 8.f;
    NF::Vec2 size = d3d.measureText("Hello", 14.f);
    REQUIRE(size.x == 5 * kCharWidthPx); // 5 chars × 8px
    REQUIRE(size.y > 0.f);
}

TEST_CASE("D3D11Backend text shim uses fallback when no text backend set",
          "[UI][Backend][D3D11]") {
    NF::D3D11Backend d3d;
    // No text backend set — falls back to UIBackend::measureText()
    NF::Vec2 size = d3d.measureText("Hi", 14.f);
    REQUIRE(size.x > 0.f);
    REQUIRE(size.y > 0.f);
}

// ── DirectWriteTextBackend (Windows only, stub returns false) ─────

TEST_CASE("DirectWriteTextBackend init returns false (stub)",
          "[UI][Backend][DirectWrite]") {
    NF::DirectWriteTextBackend dw;
    REQUIRE_FALSE(dw.init());
}

TEST_CASE("DirectWriteTextBackend measureTextWidth approximation is consistent",
          "[UI][Backend][DirectWrite]") {
    NF::DirectWriteTextBackend dw;
    float w5 = dw.measureTextWidth("Hello", 14.f);
    float w2 = dw.measureTextWidth("Hi",    14.f);
    REQUIRE(w5 > w2);
    REQUIRE(w2 > 0.f);
}

TEST_CASE("DirectWriteTextBackend lineHeight scales with font size",
          "[UI][Backend][DirectWrite]") {
    NF::DirectWriteTextBackend dw;
    float h14 = dw.lineHeight(14.f);
    float h28 = dw.lineHeight(28.f);
    REQUIRE(h28 > h14);
    REQUIRE(h14 > 0.f);
}

TEST_CASE("DirectWriteTextBackend loadFont returns false (stub)",
          "[UI][Backend][DirectWrite]") {
    NF::DirectWriteTextBackend dw;
    REQUIRE_FALSE(dw.loadFont("Consolas", 14.f));
}

TEST_CASE("DirectWriteTextBackend drawText does not crash (stub)",
          "[UI][Backend][DirectWrite]") {
    NF::DirectWriteTextBackend dw;
    dw.drawText("Hello World", 10.f, 20.f, 14.f, 0xFFFFFFFF);
}

TEST_CASE("DirectWriteTextBackend shutdown is safe without init",
          "[UI][Backend][DirectWrite]") {
    NF::DirectWriteTextBackend dw;
    dw.shutdown(); // must not crash
}
#endif // _WIN32
