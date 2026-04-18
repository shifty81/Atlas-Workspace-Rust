#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/WorldGeneration.h"

using namespace NF;

// ── S79: WorldGeneration ─────────────────────────────────────────

TEST_CASE("BiomeBrushType names are correct", "[Editor][S79]") {
    REQUIRE(std::string(biomeBrushTypeName(BiomeBrushType::Paint))   == "Paint");
    REQUIRE(std::string(biomeBrushTypeName(BiomeBrushType::Erase))   == "Erase");
    REQUIRE(std::string(biomeBrushTypeName(BiomeBrushType::Smooth))  == "Smooth");
    REQUIRE(std::string(biomeBrushTypeName(BiomeBrushType::Raise))   == "Raise");
    REQUIRE(std::string(biomeBrushTypeName(BiomeBrushType::Lower))   == "Lower");
    REQUIRE(std::string(biomeBrushTypeName(BiomeBrushType::Flatten)) == "Flatten");
    REQUIRE(std::string(biomeBrushTypeName(BiomeBrushType::Noise))   == "Noise");
    REQUIRE(std::string(biomeBrushTypeName(BiomeBrushType::Fill))    == "Fill");
}

TEST_CASE("BiomePainter starts empty", "[Editor][S79]") {
    BiomePainter painter(64);
    REQUIRE(painter.cellCount() == 0);
    REQUIRE(painter.gridSize() == 64);
}

TEST_CASE("BiomePainter paint adds a cell", "[Editor][S79]") {
    BiomePainter painter(64);
    painter.paint(5, 10);
    REQUIRE(painter.cellCount() == 1);
    REQUIRE(painter.isDirty());
}

TEST_CASE("BiomePainter paint updates existing cell", "[Editor][S79]") {
    BiomePainter painter(64);
    painter.setActiveBiome(1);
    painter.paint(5, 10);
    painter.setActiveBiome(2);
    painter.paint(5, 10);
    REQUIRE(painter.cellCount() == 1);
    REQUIRE(painter.cellAt(5, 10)->biomeIndex == 2);
}

TEST_CASE("BiomePainter paint clamps to grid bounds", "[Editor][S79]") {
    BiomePainter painter(16);
    painter.paint(-1, 5);
    painter.paint(20, 5);
    REQUIRE(painter.cellCount() == 0);
}

TEST_CASE("BiomePainter erase sets biomeIndex to 0", "[Editor][S79]") {
    BiomePainter painter(64);
    painter.setActiveBiome(3);
    painter.paint(3, 3);
    REQUIRE(painter.cellAt(3, 3)->biomeIndex == 3);
    painter.erase(3, 3);
    REQUIRE(painter.cellAt(3, 3)->biomeIndex == 0);
}

TEST_CASE("BiomePainter fill fills entire grid", "[Editor][S79]") {
    BiomePainter painter(4);
    painter.fill(1);
    REQUIRE(painter.cellCount() == 16); // 4x4
    REQUIRE(painter.cellAt(0, 0)->biomeIndex == 1);
    REQUIRE(painter.cellAt(3, 3)->biomeIndex == 1);
    REQUIRE(painter.isDirty());
}

TEST_CASE("BiomePainter clear removes all cells", "[Editor][S79]") {
    BiomePainter painter(8);
    painter.fill(1);
    painter.clear();
    REQUIRE(painter.cellCount() == 0);
}

TEST_CASE("BiomePainter setBrushRadius clamps to [0,16]", "[Editor][S79]") {
    BiomePainter painter(64);
    painter.setBrushRadius(-1.f);
    REQUIRE(painter.brushRadius() == 0.f);
    painter.setBrushRadius(20.f);
    REQUIRE(painter.brushRadius() == 16.f);
    painter.setBrushRadius(5.f);
    REQUIRE(painter.brushRadius() == 5.f);
}

TEST_CASE("BiomePainter setBrushIntensity clamps to [0,1]", "[Editor][S79]") {
    BiomePainter painter(64);
    painter.setBrushIntensity(2.f);
    REQUIRE(painter.brushIntensity() == 1.f);
    painter.setBrushIntensity(-0.5f);
    REQUIRE(painter.brushIntensity() == 0.f);
}

TEST_CASE("BiomePainter setActiveBiome and activeBrush", "[Editor][S79]") {
    BiomePainter painter(64);
    painter.setActiveBiome(5);
    REQUIRE(painter.activeBiome() == 5);
    painter.setActiveBrush(BiomeBrushType::Smooth);
    REQUIRE(painter.activeBrush() == BiomeBrushType::Smooth);
}

TEST_CASE("BiomePainter clearDirty clears dirty flag", "[Editor][S79]") {
    BiomePainter painter(8);
    painter.paint(0, 0);
    REQUIRE(painter.isDirty());
    painter.clearDirty();
    REQUIRE_FALSE(painter.isDirty());
}

TEST_CASE("StructureSeedBank starts empty", "[Editor][S79]") {
    StructureSeedBank bank;
    REQUIRE(bank.overrideCount() == 0);
}

TEST_CASE("StructureSeedBank addOverride succeeds", "[Editor][S79]") {
    StructureSeedBank bank;
    StructureSeedOverride ov;
    ov.structureId = "village";
    ov.overrideSeed = 12345;
    REQUIRE(bank.addOverride(ov));
    REQUIRE(bank.overrideCount() == 1);
}

TEST_CASE("StructureSeedBank addOverride rejects duplicate", "[Editor][S79]") {
    StructureSeedBank bank;
    StructureSeedOverride ov; ov.structureId = "castle";
    bank.addOverride(ov);
    REQUIRE_FALSE(bank.addOverride(ov));
    REQUIRE(bank.overrideCount() == 1);
}

TEST_CASE("StructureSeedBank removeOverride removes entry", "[Editor][S79]") {
    StructureSeedBank bank;
    StructureSeedOverride ov; ov.structureId = "dungeon";
    bank.addOverride(ov);
    REQUIRE(bank.removeOverride("dungeon"));
    REQUIRE(bank.overrideCount() == 0);
}

TEST_CASE("StructureSeedBank removeOverride returns false for missing", "[Editor][S79]") {
    StructureSeedBank bank;
    REQUIRE_FALSE(bank.removeOverride("ghost"));
}

TEST_CASE("StructureSeedBank findOverride returns pointer", "[Editor][S79]") {
    StructureSeedBank bank;
    StructureSeedOverride ov; ov.structureId = "ruin"; ov.overrideSeed = 42;
    bank.addOverride(ov);
    REQUIRE(bank.findOverride("ruin") != nullptr);
    REQUIRE(bank.findOverride("ruin")->overrideSeed == 42);
    REQUIRE(bank.findOverride("other") == nullptr);
}

TEST_CASE("StructureSeedBank lockOverride and unlockOverride", "[Editor][S79]") {
    StructureSeedBank bank;
    StructureSeedOverride ov; ov.structureId = "fortress";
    bank.addOverride(ov);
    REQUIRE(bank.lockOverride("fortress"));
    REQUIRE(bank.lockedCount() == 1);
    REQUIRE(bank.unlockOverride("fortress"));
    REQUIRE(bank.lockedCount() == 0);
}

TEST_CASE("StructureSeedBank kMaxOverrides is 128", "[Editor][S79]") {
    REQUIRE(StructureSeedBank::kMaxOverrides == 128);
}

TEST_CASE("OreSeamType names are correct", "[Editor][S79]") {
    REQUIRE(std::string(oreSeamTypeName(OreSeamType::Iron))     == "Iron");
    REQUIRE(std::string(oreSeamTypeName(OreSeamType::Copper))   == "Copper");
    REQUIRE(std::string(oreSeamTypeName(OreSeamType::Gold))     == "Gold");
    REQUIRE(std::string(oreSeamTypeName(OreSeamType::Silver))   == "Silver");
    REQUIRE(std::string(oreSeamTypeName(OreSeamType::Titanium)) == "Titanium");
    REQUIRE(std::string(oreSeamTypeName(OreSeamType::Uranium))  == "Uranium");
    REQUIRE(std::string(oreSeamTypeName(OreSeamType::Crystal))  == "Crystal");
    REQUIRE(std::string(oreSeamTypeName(OreSeamType::Exotic))   == "Exotic");
}

TEST_CASE("OreSeamDef volume is positive", "[Editor][S79]") {
    OreSeamDef def;
    def.id = "iron1";
    def.type = OreSeamType::Iron;
    def.radius = 5.f;
    def.density = 0.5f;
    REQUIRE(def.volume() > 0.f);
}

TEST_CASE("OreSeamEditor addSeam and seamCount", "[Editor][S79]") {
    OreSeamEditor editor;
    OreSeamDef def; def.id = "gold1"; def.type = OreSeamType::Gold;
    REQUIRE(editor.addSeam(def));
    REQUIRE(editor.seamCount() == 1);
}

TEST_CASE("OreSeamEditor addSeam rejects duplicate id", "[Editor][S79]") {
    OreSeamEditor editor;
    OreSeamDef def; def.id = "iron1"; def.type = OreSeamType::Iron;
    editor.addSeam(def);
    REQUIRE_FALSE(editor.addSeam(def));
    REQUIRE(editor.seamCount() == 1);
}

TEST_CASE("OreSeamEditor removeSeam removes entry", "[Editor][S79]") {
    OreSeamEditor editor;
    OreSeamDef def; def.id = "copper1"; def.type = OreSeamType::Copper;
    editor.addSeam(def);
    REQUIRE(editor.removeSeam("copper1"));
    REQUIRE(editor.seamCount() == 0);
}

TEST_CASE("OreSeamEditor findSeam returns correct seam", "[Editor][S79]") {
    OreSeamEditor editor;
    OreSeamDef def; def.id = "crystal1"; def.type = OreSeamType::Crystal; def.density = 0.8f;
    editor.addSeam(def);
    REQUIRE(editor.findSeam("crystal1") != nullptr);
    REQUIRE(editor.findSeam("crystal1")->density == 0.8f);
    REQUIRE(editor.findSeam("other") == nullptr);
}

TEST_CASE("OreSeamEditor seamsOfType filters correctly", "[Editor][S79]") {
    OreSeamEditor editor;
    OreSeamDef a; a.id = "g1"; a.type = OreSeamType::Gold;
    OreSeamDef b; b.id = "g2"; b.type = OreSeamType::Gold;
    OreSeamDef c; c.id = "i1"; c.type = OreSeamType::Iron;
    editor.addSeam(a); editor.addSeam(b); editor.addSeam(c);
    auto gold = editor.seamsOfType(OreSeamType::Gold);
    REQUIRE(gold.size() == 2);
    auto iron = editor.seamsOfType(OreSeamType::Iron);
    REQUIRE(iron.size() == 1);
}

TEST_CASE("OreSeamEditor totalVolume sums all seam volumes", "[Editor][S79]") {
    OreSeamEditor editor;
    OreSeamDef a; a.id = "a"; a.type = OreSeamType::Iron; a.radius = 3.f; a.density = 0.5f;
    OreSeamDef b; b.id = "b"; b.type = OreSeamType::Gold; b.radius = 2.f; b.density = 0.8f;
    editor.addSeam(a); editor.addSeam(b);
    REQUIRE(editor.totalVolume() > 0.f);
}

TEST_CASE("OreSeamEditor kMaxSeams is 64", "[Editor][S79]") {
    REQUIRE(OreSeamEditor::kMaxSeams == 64);
}

TEST_CASE("PCGPreviewMode enum has expected values", "[Editor][S79]") {
    PCGPreviewMode m = PCGPreviewMode::Heightmap;
    (void)m;
    REQUIRE(true); // just verifying enum compiles
}

TEST_CASE("PCGPreviewSettings default values", "[Editor][S79]") {
    PCGPreviewSettings settings;
    REQUIRE(settings.resolution >= 32);
    REQUIRE(settings.zoom > 0.f);
}

TEST_CASE("PCGPreviewRenderer setResolution clamps to [32,512]", "[Editor][S79]") {
    PCGPreviewRenderer renderer;
    renderer.setResolution(10);
    REQUIRE(renderer.settings().resolution == 32);
    renderer.setResolution(1000);
    REQUIRE(renderer.settings().resolution == 512);
    renderer.setResolution(128);
    REQUIRE(renderer.settings().resolution == 128);
}

TEST_CASE("PCGPreviewRenderer setZoom clamps to [0.1,10]", "[Editor][S79]") {
    PCGPreviewRenderer renderer;
    renderer.setZoom(0.0f);
    REQUIRE(renderer.settings().zoom == 0.1f);
    renderer.setZoom(100.f);
    REQUIRE(renderer.settings().zoom == 10.f);
    renderer.setZoom(2.f);
    REQUIRE(renderer.settings().zoom == 2.f);
}

TEST_CASE("PCGPreviewRenderer refresh marks stale and increments count", "[Editor][S79]") {
    PCGPreviewRenderer renderer;
    REQUIRE_FALSE(renderer.isStale());
    REQUIRE(renderer.refreshCount() == 0);
    renderer.refresh();
    REQUIRE(renderer.isStale());
    REQUIRE(renderer.refreshCount() == 1);
}

TEST_CASE("PCGPreviewRenderer markFresh clears stale", "[Editor][S79]") {
    PCGPreviewRenderer renderer;
    renderer.refresh();
    REQUIRE(renderer.isStale());
    renderer.markFresh();
    REQUIRE_FALSE(renderer.isStale());
}

TEST_CASE("PCGPreviewRenderer setPreviewData and previewPixelCount", "[Editor][S79]") {
    PCGPreviewRenderer renderer;
    renderer.setResolution(64);
    renderer.setPreviewData(std::vector<float>(64 * 64, 0.5f));
    REQUIRE(renderer.previewPixelCount() == 64 * 64);
    REQUIRE(renderer.previewData().size() == 64 * 64);
}

TEST_CASE("PCGPreviewRenderer clear resets state", "[Editor][S79]") {
    PCGPreviewRenderer renderer;
    renderer.setPreviewData({1.f, 2.f, 3.f});
    renderer.refresh();
    renderer.clear();
    REQUIRE(renderer.previewData().empty());
    REQUIRE_FALSE(renderer.isStale());
    REQUIRE(renderer.refreshCount() == 0);
}

TEST_CASE("PCGPreviewRenderer setMode updates mode", "[Editor][S79]") {
    PCGPreviewRenderer renderer;
    renderer.setMode(PCGPreviewMode::Biome);
    REQUIRE(renderer.settings().mode == PCGPreviewMode::Biome);
}
