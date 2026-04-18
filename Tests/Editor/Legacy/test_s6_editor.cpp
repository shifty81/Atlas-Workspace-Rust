#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/WorldGeneration.h"

using Catch::Approx;

// ── BiomeBrushType ──────────────────────────────────────────────

TEST_CASE("BiomeBrushType names", "[Editor][S6]") {
    REQUIRE(std::string(NF::biomeBrushTypeName(NF::BiomeBrushType::Paint))   == "Paint");
    REQUIRE(std::string(NF::biomeBrushTypeName(NF::BiomeBrushType::Erase))   == "Erase");
    REQUIRE(std::string(NF::biomeBrushTypeName(NF::BiomeBrushType::Smooth))  == "Smooth");
    REQUIRE(std::string(NF::biomeBrushTypeName(NF::BiomeBrushType::Raise))   == "Raise");
    REQUIRE(std::string(NF::biomeBrushTypeName(NF::BiomeBrushType::Lower))   == "Lower");
    REQUIRE(std::string(NF::biomeBrushTypeName(NF::BiomeBrushType::Flatten)) == "Flatten");
    REQUIRE(std::string(NF::biomeBrushTypeName(NF::BiomeBrushType::Noise))   == "Noise");
    REQUIRE(std::string(NF::biomeBrushTypeName(NF::BiomeBrushType::Fill))    == "Fill");
}

// ── BiomePaintCell ──────────────────────────────────────────────

TEST_CASE("BiomePaintCell defaults", "[Editor][S6]") {
    NF::BiomePaintCell c;
    REQUIRE(c.x == 0);
    REQUIRE(c.y == 0);
    REQUIRE(c.biomeIndex == 0);
    REQUIRE(c.intensity == 1.f);
}

// ── BiomePainter ────────────────────────────────────────────────

TEST_CASE("BiomePainter paint and cellAt", "[Editor][S6]") {
    NF::BiomePainter painter(16);
    REQUIRE(painter.gridSize() == 16);
    REQUIRE(painter.cellCount() == 0);

    painter.setActiveBiome(3);
    painter.paint(5, 7);
    REQUIRE(painter.cellCount() == 1);
    auto* cell = painter.cellAt(5, 7);
    REQUIRE(cell != nullptr);
    REQUIRE(cell->biomeIndex == 3);
}

TEST_CASE("BiomePainter erase", "[Editor][S6]") {
    NF::BiomePainter painter(16);
    painter.setActiveBiome(2);
    painter.paint(3, 3);
    REQUIRE(painter.cellAt(3, 3)->biomeIndex == 2);
    painter.erase(3, 3);
    REQUIRE(painter.cellAt(3, 3)->biomeIndex == 0);
}

TEST_CASE("BiomePainter fill", "[Editor][S6]") {
    NF::BiomePainter painter(4);
    painter.fill(5);
    REQUIRE(painter.cellCount() == 16);
    auto* cell = painter.cellAt(3, 3);
    REQUIRE(cell != nullptr);
    REQUIRE(cell->biomeIndex == 5);
}

TEST_CASE("BiomePainter bounds checking", "[Editor][S6]") {
    NF::BiomePainter painter(8);
    painter.paint(-1, 0);
    painter.paint(0, -1);
    painter.paint(8, 0);
    painter.paint(0, 8);
    REQUIRE(painter.cellCount() == 0);
}

TEST_CASE("BiomePainter dirty flag", "[Editor][S6]") {
    NF::BiomePainter painter(8);
    REQUIRE_FALSE(painter.isDirty());
    painter.paint(0, 0);
    REQUIRE(painter.isDirty());
    painter.clearDirty();
    REQUIRE_FALSE(painter.isDirty());
    painter.markDirty();
    REQUIRE(painter.isDirty());
}

TEST_CASE("BiomePainter grid size clamped", "[Editor][S6]") {
    NF::BiomePainter big(999);
    REQUIRE(big.gridSize() == 256);
}

// ── StructureSeedOverride ───────────────────────────────────────

TEST_CASE("StructureSeedOverride defaults", "[Editor][S6]") {
    NF::StructureSeedOverride ov;
    REQUIRE(ov.structureId.empty());
    REQUIRE(ov.overrideSeed == 0);
    REQUIRE_FALSE(ov.locked);
    REQUIRE(ov.notes.empty());
}

// ── StructureSeedBank ───────────────────────────────────────────

TEST_CASE("StructureSeedBank add remove find", "[Editor][S6]") {
    NF::StructureSeedBank bank;
    NF::StructureSeedOverride ov;
    ov.structureId = "tower_01";
    ov.overrideSeed = 42;
    REQUIRE(bank.addOverride(ov));
    REQUIRE(bank.overrideCount() == 1);
    REQUIRE(bank.findOverride("tower_01") != nullptr);
    REQUIRE(bank.findOverride("tower_01")->overrideSeed == 42);
    REQUIRE(bank.removeOverride("tower_01"));
    REQUIRE(bank.overrideCount() == 0);
}

TEST_CASE("StructureSeedBank rejects duplicates", "[Editor][S6]") {
    NF::StructureSeedBank bank;
    NF::StructureSeedOverride ov;
    ov.structureId = "wall_01";
    REQUIRE(bank.addOverride(ov));
    REQUIRE_FALSE(bank.addOverride(ov));
    REQUIRE(bank.overrideCount() == 1);
}

TEST_CASE("StructureSeedBank lock unlock and lockedCount", "[Editor][S6]") {
    NF::StructureSeedBank bank;
    NF::StructureSeedOverride ov;
    ov.structureId = "bridge_01";
    bank.addOverride(ov);
    REQUIRE(bank.lockedCount() == 0);
    REQUIRE(bank.lockOverride("bridge_01"));
    REQUIRE(bank.findOverride("bridge_01")->locked);
    REQUIRE(bank.lockedCount() == 1);
    REQUIRE(bank.unlockOverride("bridge_01"));
    REQUIRE_FALSE(bank.findOverride("bridge_01")->locked);
    REQUIRE(bank.lockedCount() == 0);
    REQUIRE_FALSE(bank.lockOverride("nonexistent"));
}

TEST_CASE("StructureSeedBank max overrides", "[Editor][S6]") {
    NF::StructureSeedBank bank;
    for (size_t i = 0; i < NF::StructureSeedBank::kMaxOverrides; ++i) {
        NF::StructureSeedOverride ov;
        ov.structureId = "s_" + std::to_string(i);
        REQUIRE(bank.addOverride(ov));
    }
    NF::StructureSeedOverride extra;
    extra.structureId = "overflow";
    REQUIRE_FALSE(bank.addOverride(extra));
}

// ── OreSeamType ─────────────────────────────────────────────────

TEST_CASE("OreSeamType names", "[Editor][S6]") {
    REQUIRE(std::string(NF::oreSeamTypeName(NF::OreSeamType::Iron))     == "Iron");
    REQUIRE(std::string(NF::oreSeamTypeName(NF::OreSeamType::Copper))   == "Copper");
    REQUIRE(std::string(NF::oreSeamTypeName(NF::OreSeamType::Gold))     == "Gold");
    REQUIRE(std::string(NF::oreSeamTypeName(NF::OreSeamType::Silver))   == "Silver");
    REQUIRE(std::string(NF::oreSeamTypeName(NF::OreSeamType::Titanium)) == "Titanium");
    REQUIRE(std::string(NF::oreSeamTypeName(NF::OreSeamType::Uranium))  == "Uranium");
    REQUIRE(std::string(NF::oreSeamTypeName(NF::OreSeamType::Crystal))  == "Crystal");
    REQUIRE(std::string(NF::oreSeamTypeName(NF::OreSeamType::Exotic))   == "Exotic");
}

// ── OreSeamDef ──────────────────────────────────────────────────

TEST_CASE("OreSeamDef volume calculation", "[Editor][S6]") {
    NF::OreSeamDef seam;
    seam.radius = 5.f;
    seam.density = 0.5f;
    float expected = 4.f / 3.f * 3.14159265f * (5.f * 5.f * 5.f) * 0.5f;
    REQUIRE(seam.volume() == Approx(expected).margin(0.01f));
}

// ── OreSeamEditor ───────────────────────────────────────────────

TEST_CASE("OreSeamEditor add remove find", "[Editor][S6]") {
    NF::OreSeamEditor editor;
    NF::OreSeamDef seam;
    seam.id = "iron_01";
    seam.type = NF::OreSeamType::Iron;
    REQUIRE(editor.addSeam(seam));
    REQUIRE(editor.seamCount() == 1);
    REQUIRE(editor.findSeam("iron_01") != nullptr);
    REQUIRE(editor.removeSeam("iron_01"));
    REQUIRE(editor.seamCount() == 0);
}

TEST_CASE("OreSeamEditor seamsOfType and totalVolume", "[Editor][S6]") {
    NF::OreSeamEditor editor;
    NF::OreSeamDef s1; s1.id = "iron_1"; s1.type = NF::OreSeamType::Iron; s1.radius = 2.f; s1.density = 1.f;
    NF::OreSeamDef s2; s2.id = "gold_1"; s2.type = NF::OreSeamType::Gold; s2.radius = 3.f; s2.density = 1.f;
    NF::OreSeamDef s3; s3.id = "iron_2"; s3.type = NF::OreSeamType::Iron; s3.radius = 1.f; s3.density = 1.f;
    editor.addSeam(s1);
    editor.addSeam(s2);
    editor.addSeam(s3);
    auto ironSeams = editor.seamsOfType(NF::OreSeamType::Iron);
    REQUIRE(ironSeams.size() == 2);
    REQUIRE(editor.totalVolume() == Approx(s1.volume() + s2.volume() + s3.volume()));
}

TEST_CASE("OreSeamEditor max seams", "[Editor][S6]") {
    NF::OreSeamEditor editor;
    for (size_t i = 0; i < NF::OreSeamEditor::kMaxSeams; ++i) {
        NF::OreSeamDef s;
        s.id = "ore_" + std::to_string(i);
        REQUIRE(editor.addSeam(s));
    }
    NF::OreSeamDef extra; extra.id = "overflow";
    REQUIRE_FALSE(editor.addSeam(extra));
}

// ── PCGPreviewMode ──────────────────────────────────────────────

TEST_CASE("PCGPreviewMode names", "[Editor][S6]") {
    REQUIRE(std::string(NF::pcgPreviewModeName(NF::PCGPreviewMode::Heightmap))   == "Heightmap");
    REQUIRE(std::string(NF::pcgPreviewModeName(NF::PCGPreviewMode::Biome))       == "Biome");
    REQUIRE(std::string(NF::pcgPreviewModeName(NF::PCGPreviewMode::Moisture))    == "Moisture");
    REQUIRE(std::string(NF::pcgPreviewModeName(NF::PCGPreviewMode::OreDeposits)) == "OreDeposits");
    REQUIRE(std::string(NF::pcgPreviewModeName(NF::PCGPreviewMode::Structures))  == "Structures");
    REQUIRE(std::string(NF::pcgPreviewModeName(NF::PCGPreviewMode::Combined))    == "Combined");
    REQUIRE(std::string(NF::pcgPreviewModeName(NF::PCGPreviewMode::Wireframe))   == "Wireframe");
    REQUIRE(std::string(NF::pcgPreviewModeName(NF::PCGPreviewMode::Heatmap))     == "Heatmap");
}

// ── PCGPreviewRenderer ──────────────────────────────────────────

TEST_CASE("PCGPreviewRenderer settings and resolution clamp", "[Editor][S6]") {
    NF::PCGPreviewRenderer renderer;
    REQUIRE(renderer.settings().mode == NF::PCGPreviewMode::Combined);
    renderer.setResolution(10);
    REQUIRE(renderer.settings().resolution == 32);
    renderer.setResolution(999);
    REQUIRE(renderer.settings().resolution == 512);
    renderer.setResolution(256);
    REQUIRE(renderer.settings().resolution == 256);
}

TEST_CASE("PCGPreviewRenderer zoom clamp", "[Editor][S6]") {
    NF::PCGPreviewRenderer renderer;
    renderer.setZoom(0.01f);
    REQUIRE(renderer.settings().zoom == Approx(0.1f));
    renderer.setZoom(99.f);
    REQUIRE(renderer.settings().zoom == Approx(10.f));
    renderer.setZoom(5.f);
    REQUIRE(renderer.settings().zoom == Approx(5.f));
}

TEST_CASE("PCGPreviewRenderer refresh and stale", "[Editor][S6]") {
    NF::PCGPreviewRenderer renderer;
    REQUIRE(renderer.refreshCount() == 0);
    REQUIRE_FALSE(renderer.isStale());
    renderer.refresh();
    REQUIRE(renderer.refreshCount() == 1);
    REQUIRE(renderer.isStale());
    renderer.markFresh();
    REQUIRE_FALSE(renderer.isStale());
}

TEST_CASE("PCGPreviewRenderer preview data", "[Editor][S6]") {
    NF::PCGPreviewRenderer renderer;
    renderer.setResolution(64);
    REQUIRE(renderer.previewPixelCount() == 64 * 64);
    std::vector<float> data(64 * 64, 0.5f);
    renderer.setPreviewData(data);
    REQUIRE(renderer.previewData().size() == 64 * 64);
}
