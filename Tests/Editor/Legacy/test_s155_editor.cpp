// S155 editor tests: MeshInspectorV1, FontEditorV1, LocalizationEditorV1
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/LocalizationEditorV1.h"
#include "NF/Editor/FontEditorV1.h"
#include "NF/Editor/MeshInspectorV1.h"

using namespace NF;
using Catch::Approx;

// ── MeshInspectorV1 ───────────────────────────────────────────────────────────

TEST_CASE("MivMeshStats validity", "[Editor][S155]") {
    MivMeshStats s;
    REQUIRE(!s.isValid());
    s.vertexCount = 100;
    REQUIRE(s.isValid());
}

TEST_CASE("MivLod validity", "[Editor][S155]") {
    MivLod lod;
    REQUIRE(!lod.isValid());
    lod.id = 1;
    REQUIRE(lod.isValid());
}

TEST_CASE("MeshInspectorV1 loadMesh", "[Editor][S155]") {
    MeshInspectorV1 mi;
    REQUIRE(!mi.isLoaded());
    REQUIRE(mi.loadMesh("Cube.fbx"));
    REQUIRE(mi.isLoaded());
    REQUIRE(mi.meshName() == "Cube.fbx");
}

TEST_CASE("MeshInspectorV1 loadMesh fires callback", "[Editor][S155]") {
    MeshInspectorV1 mi;
    std::string inspected;
    mi.setOnInspect([&](const std::string& n){ inspected = n; });
    mi.loadMesh("Sphere.obj");
    REQUIRE(inspected == "Sphere.obj");
}

TEST_CASE("MeshInspectorV1 loadMesh empty name fails", "[Editor][S155]") {
    MeshInspectorV1 mi;
    REQUIRE(!mi.loadMesh(""));
}

TEST_CASE("MeshInspectorV1 unloadMesh", "[Editor][S155]") {
    MeshInspectorV1 mi;
    mi.loadMesh("Mesh.fbx");
    REQUIRE(mi.unloadMesh());
    REQUIRE(!mi.isLoaded());
    REQUIRE(!mi.unloadMesh());
}

TEST_CASE("MeshInspectorV1 addLod and lodCount", "[Editor][S155]") {
    MeshInspectorV1 mi;
    mi.loadMesh("Car.fbx");
    MivLod lod; lod.id = 1; lod.screenSizePct = 1.f;
    REQUIRE(mi.addLod(lod));
    REQUIRE(mi.lodCount() == 1);
}

TEST_CASE("MeshInspectorV1 removeLod", "[Editor][S155]") {
    MeshInspectorV1 mi;
    mi.loadMesh("Tree.fbx");
    MivLod lod; lod.id = 2;
    mi.addLod(lod);
    REQUIRE(mi.removeLod(2));
    REQUIRE(mi.lodCount() == 0);
    REQUIRE(!mi.removeLod(2));
}

TEST_CASE("MeshInspectorV1 setTopology and getTopology", "[Editor][S155]") {
    MeshInspectorV1 mi;
    mi.setTopology(MivTopology::LineList);
    REQUIRE(mi.getTopology() == MivTopology::LineList);
}

TEST_CASE("MeshInspectorV1 setStats and getStats", "[Editor][S155]") {
    MeshInspectorV1 mi;
    MivMeshStats s; s.vertexCount = 500; s.triangleCount = 200;
    mi.setStats(s);
    REQUIRE(mi.getStats().vertexCount == 500);
    REQUIRE(mi.getStats().triangleCount == 200);
}

TEST_CASE("MeshInspectorV1 unload clears lods", "[Editor][S155]") {
    MeshInspectorV1 mi;
    mi.loadMesh("Boat.fbx");
    MivLod lod; lod.id = 1;
    mi.addLod(lod);
    mi.unloadMesh();
    REQUIRE(mi.lodCount() == 0);
}

// ── FontEditorV1 ──────────────────────────────────────────────────────────────

TEST_CASE("Fev1RenderMode names", "[Editor][S155]") {
    REQUIRE(std::string(fev1RenderModeName(Fev1RenderMode::Bitmap)) == "Bitmap");
    REQUIRE(std::string(fev1RenderModeName(Fev1RenderMode::SDF))    == "SDF");
    REQUIRE(std::string(fev1RenderModeName(Fev1RenderMode::MSDF))   == "MSDF");
}

TEST_CASE("Fev1GlyphInfo validity", "[Editor][S155]") {
    Fev1GlyphInfo g;
    REQUIRE(!g.isValid());
    g.codepoint = 65; // 'A'
    REQUIRE(g.isValid());
}

TEST_CASE("FontEditorV1 addConfig and configCount", "[Editor][S155]") {
    FontEditorV1 fe;
    Fev1FontConfig cfg; cfg.id = 1; cfg.familyName = "Roboto";
    REQUIRE(fe.addConfig(cfg));
    REQUIRE(fe.configCount() == 1);
}

TEST_CASE("FontEditorV1 reject duplicate config", "[Editor][S155]") {
    FontEditorV1 fe;
    Fev1FontConfig cfg; cfg.id = 2; cfg.familyName = "Arial";
    REQUIRE(fe.addConfig(cfg));
    REQUIRE(!fe.addConfig(cfg));
}

TEST_CASE("FontEditorV1 removeConfig", "[Editor][S155]") {
    FontEditorV1 fe;
    Fev1FontConfig cfg; cfg.id = 3; cfg.familyName = "Ubuntu";
    fe.addConfig(cfg);
    REQUIRE(fe.removeConfig(3));
    REQUIRE(fe.configCount() == 0);
    REQUIRE(!fe.removeConfig(3));
}

TEST_CASE("FontEditorV1 generateAtlas fires callback", "[Editor][S155]") {
    FontEditorV1 fe;
    Fev1FontConfig cfg; cfg.id = 1; cfg.familyName = "Noto";
    fe.addConfig(cfg);
    uint32_t cbId = 0; bool cbOk = false;
    fe.setOnGenerate([&](uint32_t id, bool ok){ cbId = id; cbOk = ok; });
    REQUIRE(fe.generateAtlas(1));
    REQUIRE(cbId == 1);
    REQUIRE(cbOk);
}

TEST_CASE("FontEditorV1 addGlyph and totalGlyphCount", "[Editor][S155]") {
    FontEditorV1 fe;
    Fev1FontConfig cfg; cfg.id = 1; cfg.familyName = "Mono";
    fe.addConfig(cfg);
    Fev1GlyphInfo g1; g1.codepoint = 65; g1.advance = 8.f;
    Fev1GlyphInfo g2; g2.codepoint = 66; g2.advance = 8.f;
    REQUIRE(fe.addGlyph(1, g1));
    REQUIRE(fe.addGlyph(1, g2));
    REQUIRE(fe.totalGlyphCount() == 2);
}

TEST_CASE("FontEditorV1 addGlyph rejects duplicate codepoint", "[Editor][S155]") {
    FontEditorV1 fe;
    Fev1FontConfig cfg; cfg.id = 1; cfg.familyName = "Sans";
    fe.addConfig(cfg);
    Fev1GlyphInfo g; g.codepoint = 97;
    REQUIRE(fe.addGlyph(1, g));
    REQUIRE(!fe.addGlyph(1, g));
}

TEST_CASE("FontEditorV1 findConfig by name", "[Editor][S155]") {
    FontEditorV1 fe;
    Fev1FontConfig cfg; cfg.id = 1; cfg.familyName = "Serif";
    fe.addConfig(cfg);
    REQUIRE(fe.findConfig("Serif") != nullptr);
    REQUIRE(fe.findConfig("Missing") == nullptr);
}

// ── LocalizationEditorV1 ──────────────────────────────────────────────────────

TEST_CASE("LevString validity", "[Editor][S155]") {
    LevString s;
    REQUIRE(!s.isValid());
    s.id = 1; s.key = "hello"; s.locale = "en";
    REQUIRE(s.isValid());
}

TEST_CASE("LocalizationEditorV1 addLocale and localeCount", "[Editor][S155]") {
    LocalizationEditorV1 le;
    LevLocale loc; loc.code = "en"; loc.name = "English";
    REQUIRE(le.addLocale(loc));
    REQUIRE(le.localeCount() == 1);
}

TEST_CASE("LocalizationEditorV1 reject duplicate locale", "[Editor][S155]") {
    LocalizationEditorV1 le;
    LevLocale loc; loc.code = "fr"; loc.name = "French";
    REQUIRE(le.addLocale(loc));
    REQUIRE(!le.addLocale(loc));
}

TEST_CASE("LocalizationEditorV1 removeLocale", "[Editor][S155]") {
    LocalizationEditorV1 le;
    LevLocale loc; loc.code = "de";
    le.addLocale(loc);
    REQUIRE(le.removeLocale("de"));
    REQUIRE(le.localeCount() == 0);
}

TEST_CASE("LocalizationEditorV1 addString and getString", "[Editor][S155]") {
    LocalizationEditorV1 le;
    LevString s; s.id = 1; s.key = "greet"; s.locale = "en"; s.value = "Hello";
    REQUIRE(le.addString(s));
    REQUIRE(le.getString("greet", "en") == "Hello");
}

TEST_CASE("LocalizationEditorV1 setString updates value", "[Editor][S155]") {
    LocalizationEditorV1 le;
    LevString s; s.id = 1; s.key = "btn.ok"; s.locale = "en"; s.value = "OK";
    le.addString(s);
    REQUIRE(le.setString("btn.ok", "en", "Okay"));
    REQUIRE(le.getString("btn.ok", "en") == "Okay");
}

TEST_CASE("LocalizationEditorV1 missingCount", "[Editor][S155]") {
    LocalizationEditorV1 le;
    LevString s; s.id = 1; s.key = "title"; s.locale = "ja"; s.status = LevLocaleStatus::Missing;
    le.addString(s);
    REQUIRE(le.missingCount("ja") == 1);
    REQUIRE(le.missingCount("en") == 0);
}

TEST_CASE("LocalizationEditorV1 stringCount per locale", "[Editor][S155]") {
    LocalizationEditorV1 le;
    LevString s1; s1.id = 1; s1.key = "a"; s1.locale = "en";
    LevString s2; s2.id = 2; s2.key = "b"; s2.locale = "en";
    LevString s3; s3.id = 3; s3.key = "a"; s3.locale = "fr";
    le.addString(s1); le.addString(s2); le.addString(s3);
    REQUIRE(le.stringCount("en") == 2);
    REQUIRE(le.stringCount("fr") == 1);
}

TEST_CASE("LocalizationEditorV1 importFrom and exportTo", "[Editor][S155]") {
    LocalizationEditorV1 le;
    le.importFrom("{\"key\":\"val\"}");
    REQUIRE(le.exportTo() == "{\"key\":\"val\"}");
}
