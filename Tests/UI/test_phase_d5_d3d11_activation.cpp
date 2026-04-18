// Phase D.5 — D3D11 Backend Activation
//
// D.5.1 — D3D11Backend contract (stub-level): backendName, isGPUAccelerated,
//         init stub returns false, shutdown safe, beginFrame/endFrame no-crash,
//         flush no-crash, setClipRect/clearClipRect no-crash.
//
// D.5.2 — DirectWriteTextBackend contract: measureTextWidth approximation,
//         lineHeight proportional, loadFont stub, drawText no-crash,
//         init stub, shutdown safe, isInitialized default false.
//
// D.5.3 — UIBackendSelector: queryCapabilities for all types,
//         uiBackendTypeName for all types, selectUIBackend(Null) returns non-null,
//         platformSupportsGDI / platformSupportsD3D11 compile + run without crash.
//
// D.5.4 — createD3D11WithDirectWrite: returns empty pair on non-Windows,
//         or isCreated()==true on Windows; geom->backendName()=="D3D11";
//         text->lineHeight(>0); text backend registered on geom.

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/UI/D3D11Backend.h"
#include "NF/UI/DirectWriteTextBackend.h"
#include "NF/UI/UIBackendSelector.h"
#include "NF/UI/IUIBackendInterfaces.h"

using Catch::Approx;

// ── D.5.1 — D3D11Backend contract ────────────────────────────────────────────

#ifdef _WIN32

TEST_CASE("D3D11Backend backendName returns D3D11", "[UI][D3D11][d5]") {
    NF::D3D11Backend b;
    REQUIRE(std::string(b.backendName()) == "D3D11");
}

TEST_CASE("D3D11Backend isGPUAccelerated returns true", "[UI][D3D11][d5]") {
    NF::D3D11Backend b;
    REQUIRE(b.isGPUAccelerated());
}

TEST_CASE("D3D11Backend init stub returns false (no real GPU)", "[UI][D3D11][d5]") {
    NF::D3D11Backend b;
    // Stub implementation returns false — real GPU init is deferred.
    REQUIRE_FALSE(b.init(800, 600));
}

TEST_CASE("D3D11Backend shutdown is safe before and after init", "[UI][D3D11][d5]") {
    NF::D3D11Backend b;
    b.shutdown(); // safe before init
    b.init(800, 600);
    b.shutdown(); // safe after stub init
}

TEST_CASE("D3D11Backend beginFrame and endFrame do not crash", "[UI][D3D11][d5]") {
    NF::D3D11Backend b;
    b.init(800, 600);
    b.beginFrame(800, 600);
    b.endFrame();
    b.shutdown();
}

TEST_CASE("D3D11Backend flush with zero geometry is safe", "[UI][D3D11][d5]") {
    NF::D3D11Backend b;
    b.init(800, 600);
    b.flush(nullptr, 0, nullptr, 0);
}

TEST_CASE("D3D11Backend flush with valid geometry does not crash", "[UI][D3D11][d5]") {
    NF::D3D11Backend b;
    b.init(800, 600);
    NF::UIVertex verts[4]{};
    uint32_t     idxs[6]{0, 1, 2, 2, 3, 0};
    b.flush(verts, 4, idxs, 6);
}

TEST_CASE("D3D11Backend setClipRect and clearClipRect do not crash", "[UI][D3D11][d5]") {
    NF::D3D11Backend b;
    b.init(800, 600);
    b.setClipRect(10.f, 10.f, 200.f, 100.f);
    b.clearClipRect();
}

TEST_CASE("D3D11Backend setTextBackend accepts nullptr safely", "[UI][D3D11][d5]") {
    NF::D3D11Backend b;
    b.setTextBackend(nullptr);
    b.init(800, 600);
    b.beginFrame(800, 600);
    b.endFrame();
}

TEST_CASE("D3D11Backend setTextBackend wires a DirectWriteTextBackend", "[UI][D3D11][d5]") {
    NF::D3D11Backend b;
    NF::DirectWriteTextBackend text;
    b.setTextBackend(&text);
    REQUIRE_FALSE(b.init(800, 600)); // still stub-false, but wiring must not crash
}

// ── D.5.2 — DirectWriteTextBackend contract ───────────────────────────────────

TEST_CASE("DirectWriteTextBackend measureTextWidth proportional to length", "[UI][DirectWrite][d5]") {
    NF::DirectWriteTextBackend t;
    float w4  = t.measureTextWidth("AAAA", 14.f);
    float w8  = t.measureTextWidth("AAAAAAAA", 14.f);
    REQUIRE(w8 > w4);
    REQUIRE(w4 > 0.f);
}

TEST_CASE("DirectWriteTextBackend measureTextWidth empty string returns 0", "[UI][DirectWrite][d5]") {
    NF::DirectWriteTextBackend t;
    REQUIRE(t.measureTextWidth("", 14.f) == Approx(0.f));
}

TEST_CASE("DirectWriteTextBackend lineHeight is positive and proportional to size", "[UI][DirectWrite][d5]") {
    NF::DirectWriteTextBackend t;
    float h12 = t.lineHeight(12.f);
    float h24 = t.lineHeight(24.f);
    REQUIRE(h12 > 0.f);
    REQUIRE(h24 > h12);
}

TEST_CASE("DirectWriteTextBackend loadFont stub returns false", "[UI][DirectWrite][d5]") {
    NF::DirectWriteTextBackend t;
    REQUIRE_FALSE(t.loadFont("Arial", 14.f));
}

TEST_CASE("DirectWriteTextBackend drawText does not crash", "[UI][DirectWrite][d5]") {
    NF::DirectWriteTextBackend t;
    t.drawText("Hello Atlas", 10.f, 20.f, 14.f, 0xFFFFFFFFu);
}

TEST_CASE("DirectWriteTextBackend init stub returns false", "[UI][DirectWrite][d5]") {
    NF::DirectWriteTextBackend t;
    REQUIRE_FALSE(t.init());
}

TEST_CASE("DirectWriteTextBackend isInitialized default false", "[UI][DirectWrite][d5]") {
    NF::DirectWriteTextBackend t;
    REQUIRE_FALSE(t.isInitialized());
}

TEST_CASE("DirectWriteTextBackend shutdown before init is safe", "[UI][DirectWrite][d5]") {
    NF::DirectWriteTextBackend t;
    t.shutdown(); // must not crash
}

TEST_CASE("DirectWriteTextBackend shutdown after init stub is safe", "[UI][DirectWrite][d5]") {
    NF::DirectWriteTextBackend t;
    t.init();
    t.shutdown();
}

// ── D.5.3 — UIBackendSelector: capabilities and names ────────────────────────

TEST_CASE("uiBackendTypeName returns correct string for all values", "[UI][BackendSelector][d5]") {
    REQUIRE(std::string(NF::uiBackendTypeName(NF::UIBackendType::Auto))   == "Auto");
    REQUIRE(std::string(NF::uiBackendTypeName(NF::UIBackendType::D3D11))  == "D3D11");
    REQUIRE(std::string(NF::uiBackendTypeName(NF::UIBackendType::OpenGL)) == "OpenGL (compat)");
    REQUIRE(std::string(NF::uiBackendTypeName(NF::UIBackendType::GDI))    == "GDI (fallback)");
    REQUIRE(std::string(NF::uiBackendTypeName(NF::UIBackendType::Null))   == "Null");
}

TEST_CASE("queryCapabilities D3D11 returns full capabilities", "[UI][BackendSelector][d5]") {
    auto caps = NF::queryCapabilities(NF::UIBackendType::D3D11);
    REQUIRE(caps.gpuAccelerated);
    REQUIRE(caps.nativeTextLayout);
    REQUIRE(caps.textureAtlas);
    REQUIRE(caps.scissorClip);
    REQUIRE(caps.vsync);
}

TEST_CASE("queryCapabilities GDI returns no capabilities", "[UI][BackendSelector][d5]") {
    auto caps = NF::queryCapabilities(NF::UIBackendType::GDI);
    REQUIRE_FALSE(caps.gpuAccelerated);
    REQUIRE_FALSE(caps.nativeTextLayout);
    REQUIRE_FALSE(caps.textureAtlas);
    REQUIRE_FALSE(caps.scissorClip);
    REQUIRE_FALSE(caps.vsync);
}

TEST_CASE("queryCapabilities Null returns no capabilities", "[UI][BackendSelector][d5]") {
    auto caps = NF::queryCapabilities(NF::UIBackendType::Null);
    REQUIRE_FALSE(caps.gpuAccelerated);
    REQUIRE_FALSE(caps.nativeTextLayout);
}

TEST_CASE("queryCapabilities OpenGL returns gpu+atlas+scissor but no nativeText", "[UI][BackendSelector][d5]") {
    auto caps = NF::queryCapabilities(NF::UIBackendType::OpenGL);
    REQUIRE(caps.gpuAccelerated);
    REQUIRE_FALSE(caps.nativeTextLayout);
    REQUIRE(caps.textureAtlas);
    REQUIRE(caps.scissorClip);
}

TEST_CASE("selectUIBackend Null returns non-null NullBackend", "[UI][BackendSelector][d5]") {
    auto b = NF::selectUIBackend(NF::UIBackendType::Null);
    REQUIRE(b != nullptr);
}

TEST_CASE("platformSupportsGDI and platformSupportsD3D11 compile and run", "[UI][BackendSelector][d5]") {
    bool gdi  = NF::platformSupportsGDI();
    bool d3d11 = NF::platformSupportsD3D11();
    // On non-Windows CI both are false; on Windows GDI is always true.
    (void)gdi; (void)d3d11; // compile-time and no-crash guarantee
}

// ── D.5.4 — createD3D11WithDirectWrite factory ────────────────────────────────

TEST_CASE("createD3D11WithDirectWrite on _WIN32 returns isCreated()==true", "[UI][D3D11Factory][d5]") {
    auto pair = NF::createD3D11WithDirectWrite(1280, 720);
    // On Windows the pair is always created (geom + text objects exist even as stubs).
    REQUIRE(pair.isCreated());
}

TEST_CASE("createD3D11WithDirectWrite geom backendName is D3D11", "[UI][D3D11Factory][d5]") {
    auto pair = NF::createD3D11WithDirectWrite(800, 600);
    REQUIRE(pair.isCreated());
    REQUIRE(std::string(pair.geom->backendName()) == "D3D11");
}

TEST_CASE("createD3D11WithDirectWrite geom isGPUAccelerated", "[UI][D3D11Factory][d5]") {
    auto pair = NF::createD3D11WithDirectWrite(800, 600);
    REQUIRE(pair.isCreated());
    REQUIRE(pair.geom->isGPUAccelerated());
}

TEST_CASE("createD3D11WithDirectWrite text lineHeight is positive", "[UI][D3D11Factory][d5]") {
    auto pair = NF::createD3D11WithDirectWrite(800, 600);
    REQUIRE(pair.isCreated());
    REQUIRE(pair.text->lineHeight(14.f) > 0.f);
}

TEST_CASE("createD3D11WithDirectWrite geom init stub returns false (no GPU in CI)", "[UI][D3D11Factory][d5]") {
    auto pair = NF::createD3D11WithDirectWrite(800, 600);
    REQUIRE(pair.isCreated());
    REQUIRE_FALSE(pair.geom->init(800, 600));
}

TEST_CASE("createD3D11WithDirectWrite shutdown after stub init is safe", "[UI][D3D11Factory][d5]") {
    auto pair = NF::createD3D11WithDirectWrite(800, 600);
    REQUIRE(pair.isCreated());
    pair.geom->init(800, 600);
    pair.geom->shutdown();
    pair.text->shutdown();
}

#endif // _WIN32
