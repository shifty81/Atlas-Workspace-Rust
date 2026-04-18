#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/FoliagePainter.h"
#include "NF/Editor/WaterEditor.h"
#include "NF/Editor/TerrainEditor.h"

using namespace NF;

// ── S91: TerrainEditor + WaterEditor + FoliagePainter ────────────

// ── TerrainEditor ────────────────────────────────────────────────

TEST_CASE("TerrainBrushMode names are correct", "[Editor][S91]") {
    REQUIRE(std::string(terrainBrushModeName(TerrainBrushMode::Raise))   == "Raise");
    REQUIRE(std::string(terrainBrushModeName(TerrainBrushMode::Lower))   == "Lower");
    REQUIRE(std::string(terrainBrushModeName(TerrainBrushMode::Flatten)) == "Flatten");
    REQUIRE(std::string(terrainBrushModeName(TerrainBrushMode::Smooth))  == "Smooth");
    REQUIRE(std::string(terrainBrushModeName(TerrainBrushMode::Noise))   == "Noise");
    REQUIRE(std::string(terrainBrushModeName(TerrainBrushMode::Paint))   == "Paint");
}

TEST_CASE("TerrainLayerBlend names are correct", "[Editor][S91]") {
    REQUIRE(std::string(terrainLayerBlendName(TerrainLayerBlend::Normal))   == "Normal");
    REQUIRE(std::string(terrainLayerBlendName(TerrainLayerBlend::Height))   == "Height");
    REQUIRE(std::string(terrainLayerBlendName(TerrainLayerBlend::Slope))    == "Slope");
    REQUIRE(std::string(terrainLayerBlendName(TerrainLayerBlend::Distance)) == "Distance");
    REQUIRE(std::string(terrainLayerBlendName(TerrainLayerBlend::Custom))   == "Custom");
}

TEST_CASE("TerrainResolution names are correct", "[Editor][S91]") {
    REQUIRE(std::string(terrainResolutionName(TerrainResolution::R64))   == "R64");
    REQUIRE(std::string(terrainResolutionName(TerrainResolution::R128))  == "R128");
    REQUIRE(std::string(terrainResolutionName(TerrainResolution::R256))  == "R256");
    REQUIRE(std::string(terrainResolutionName(TerrainResolution::R512))  == "R512");
    REQUIRE(std::string(terrainResolutionName(TerrainResolution::R1024)) == "R1024");
    REQUIRE(std::string(terrainResolutionName(TerrainResolution::R2048)) == "R2048");
}

TEST_CASE("TerrainLayer stores properties", "[Editor][S91]") {
    TerrainLayer layer("Grass", TerrainLayerBlend::Slope);
    layer.setWeight(0.8f);
    layer.setTileScale(2.0f);
    REQUIRE(layer.name()      == "Grass");
    REQUIRE(layer.blend()     == TerrainLayerBlend::Slope);
    REQUIRE(layer.weight()    == 0.8f);
    REQUIRE(layer.tileScale() == 2.0f);
    REQUIRE(layer.isEnabled());
}

TEST_CASE("TerrainEditor add remove findLayer", "[Editor][S91]") {
    TerrainEditor editor;
    TerrainLayer l1("Grass", TerrainLayerBlend::Normal);
    TerrainLayer l2("Rock",  TerrainLayerBlend::Slope);
    REQUIRE(editor.addLayer(l1));
    REQUIRE(editor.addLayer(l2));
    REQUIRE(editor.layerCount() == 2);
    REQUIRE(editor.findLayer("Rock") != nullptr);
    editor.removeLayer("Rock");
    REQUIRE(editor.layerCount() == 1);
}

TEST_CASE("TerrainEditor rejects duplicate layer name", "[Editor][S91]") {
    TerrainEditor editor;
    TerrainLayer layer("Sand", TerrainLayerBlend::Height);
    editor.addLayer(layer);
    REQUIRE_FALSE(editor.addLayer(layer));
}

TEST_CASE("TerrainEditor brush settings", "[Editor][S91]") {
    TerrainEditor editor;
    editor.setBrushMode(TerrainBrushMode::Flatten);
    editor.setBrushRadius(20.0f);
    editor.setBrushStrength(0.7f);
    editor.setResolution(TerrainResolution::R1024);
    REQUIRE(editor.brushMode()     == TerrainBrushMode::Flatten);
    REQUIRE(editor.brushRadius()   == 20.0f);
    REQUIRE(editor.brushStrength() == 0.7f);
    REQUIRE(editor.resolution()    == TerrainResolution::R1024);
}

TEST_CASE("TerrainEditor countByBlend and enabledLayerCount", "[Editor][S91]") {
    TerrainEditor editor;
    TerrainLayer l1("A", TerrainLayerBlend::Normal);
    TerrainLayer l2("B", TerrainLayerBlend::Slope);
    TerrainLayer l3("C", TerrainLayerBlend::Normal); l3.setEnabled(false);
    editor.addLayer(l1); editor.addLayer(l2); editor.addLayer(l3);
    REQUIRE(editor.countByBlend(TerrainLayerBlend::Normal) == 2);
    REQUIRE(editor.enabledLayerCount() == 2);
}

TEST_CASE("TerrainEditor MAX_LAYERS is 16", "[Editor][S91]") {
    REQUIRE(TerrainEditor::MAX_LAYERS == 16);
}

// ── WaterEditor ──────────────────────────────────────────────────

TEST_CASE("WaterBodyType names are correct", "[Editor][S91]") {
    REQUIRE(std::string(waterBodyTypeName(WaterBodyType::Ocean))     == "Ocean");
    REQUIRE(std::string(waterBodyTypeName(WaterBodyType::Lake))      == "Lake");
    REQUIRE(std::string(waterBodyTypeName(WaterBodyType::River))     == "River");
    REQUIRE(std::string(waterBodyTypeName(WaterBodyType::Pond))      == "Pond");
    REQUIRE(std::string(waterBodyTypeName(WaterBodyType::Waterfall)) == "Waterfall");
}

TEST_CASE("WaterRenderMode names are correct", "[Editor][S91]") {
    REQUIRE(std::string(waterRenderModeName(WaterRenderMode::Simple))     == "Simple");
    REQUIRE(std::string(waterRenderModeName(WaterRenderMode::FFT))        == "FFT");
    REQUIRE(std::string(waterRenderModeName(WaterRenderMode::Gerstner))   == "Gerstner");
    REQUIRE(std::string(waterRenderModeName(WaterRenderMode::Planar))     == "Planar");
    REQUIRE(std::string(waterRenderModeName(WaterRenderMode::Volumetric)) == "Volumetric");
}

TEST_CASE("WaterFlowDir names are correct", "[Editor][S91]") {
    REQUIRE(std::string(waterFlowDirName(WaterFlowDir::North))    == "North");
    REQUIRE(std::string(waterFlowDirName(WaterFlowDir::South))    == "South");
    REQUIRE(std::string(waterFlowDirName(WaterFlowDir::East))     == "East");
    REQUIRE(std::string(waterFlowDirName(WaterFlowDir::West))     == "West");
    REQUIRE(std::string(waterFlowDirName(WaterFlowDir::Circular)) == "Circular");
    REQUIRE(std::string(waterFlowDirName(WaterFlowDir::None))     == "None");
}

TEST_CASE("WaterBody stores properties", "[Editor][S91]") {
    WaterBody body("Lake1", WaterBodyType::Lake);
    body.setRenderMode(WaterRenderMode::Gerstner);
    body.setFlowDir(WaterFlowDir::Circular);
    body.setDepth(20.0f);
    body.setWaveHeight(1.5f);
    body.setTransparency(0.5f);
    REQUIRE(body.name()         == "Lake1");
    REQUIRE(body.type()         == WaterBodyType::Lake);
    REQUIRE(body.renderMode()   == WaterRenderMode::Gerstner);
    REQUIRE(body.depth()        == 20.0f);
    REQUIRE(body.transparency() == 0.5f);
    REQUIRE(body.isEnabled());
}

TEST_CASE("WaterEditor addBody setActive removeBody", "[Editor][S91]") {
    WaterEditor editor;
    WaterBody b1("River1", WaterBodyType::River);
    WaterBody b2("Pond1",  WaterBodyType::Pond);
    REQUIRE(editor.addBody(b1));
    REQUIRE(editor.addBody(b2));
    REQUIRE(editor.bodyCount() == 2);
    REQUIRE(editor.setActiveBody("River1"));
    REQUIRE(editor.activeBody() == "River1");
    editor.removeBody("River1");
    REQUIRE(editor.activeBody().empty());
}

TEST_CASE("WaterEditor rejects duplicate body name", "[Editor][S91]") {
    WaterEditor editor;
    WaterBody b("Ocean", WaterBodyType::Ocean);
    editor.addBody(b);
    REQUIRE_FALSE(editor.addBody(b));
}

TEST_CASE("WaterEditor countByType and countByRenderMode", "[Editor][S91]") {
    WaterEditor editor;
    WaterBody b1("L1", WaterBodyType::Lake);  b1.setRenderMode(WaterRenderMode::FFT);
    WaterBody b2("L2", WaterBodyType::Lake);  b2.setRenderMode(WaterRenderMode::Gerstner);
    WaterBody b3("R1", WaterBodyType::River); b3.setRenderMode(WaterRenderMode::FFT);
    editor.addBody(b1); editor.addBody(b2); editor.addBody(b3);
    REQUIRE(editor.countByType(WaterBodyType::Lake)           == 2);
    REQUIRE(editor.countByRenderMode(WaterRenderMode::FFT)    == 2);
}

TEST_CASE("WaterEditor MAX_BODIES is 64", "[Editor][S91]") {
    REQUIRE(WaterEditor::MAX_BODIES == 64);
}

// ── FoliagePainter ───────────────────────────────────────────────

TEST_CASE("FoliagePlacementMode names are correct", "[Editor][S91]") {
    REQUIRE(std::string(foliagePlacementModeName(FoliagePlacementMode::Scatter))    == "Scatter");
    REQUIRE(std::string(foliagePlacementModeName(FoliagePlacementMode::Paint))      == "Paint");
    REQUIRE(std::string(foliagePlacementModeName(FoliagePlacementMode::Erase))      == "Erase");
    REQUIRE(std::string(foliagePlacementModeName(FoliagePlacementMode::Select))     == "Select");
    REQUIRE(std::string(foliagePlacementModeName(FoliagePlacementMode::Procedural)) == "Procedural");
}

TEST_CASE("FoliageCullMode names are correct", "[Editor][S91]") {
    REQUIRE(std::string(foliageCullModeName(FoliageCullMode::None))               == "None");
    REQUIRE(std::string(foliageCullModeName(FoliageCullMode::Distance))           == "Distance");
    REQUIRE(std::string(foliageCullModeName(FoliageCullMode::Frustum))            == "Frustum");
    REQUIRE(std::string(foliageCullModeName(FoliageCullMode::DistanceAndFrustum)) == "DistanceAndFrustum");
    REQUIRE(std::string(foliageCullModeName(FoliageCullMode::Impostor))           == "Impostor");
}

TEST_CASE("FoliageAlignMode names are correct", "[Editor][S91]") {
    REQUIRE(std::string(foliageAlignModeName(FoliageAlignMode::WorldUp))       == "WorldUp");
    REQUIRE(std::string(foliageAlignModeName(FoliageAlignMode::SurfaceNormal)) == "SurfaceNormal");
    REQUIRE(std::string(foliageAlignModeName(FoliageAlignMode::Random))        == "Random");
    REQUIRE(std::string(foliageAlignModeName(FoliageAlignMode::Custom))        == "Custom");
}

TEST_CASE("FoliageType stores properties", "[Editor][S91]") {
    FoliageType type("Fern");
    type.setDensity(3.0f);
    type.setScaleMin(0.5f);
    type.setScaleMax(1.5f);
    type.setCullMode(FoliageCullMode::DistanceAndFrustum);
    type.setCullDistance(150.0f);
    REQUIRE(type.name()         == "Fern");
    REQUIRE(type.density()      == 3.0f);
    REQUIRE(type.scaleMin()     == 0.5f);
    REQUIRE(type.cullMode()     == FoliageCullMode::DistanceAndFrustum);
    REQUIRE(type.cullDistance() == 150.0f);
    REQUIRE(type.isEnabled());
}

TEST_CASE("FoliagePainter addType setActive remove", "[Editor][S91]") {
    FoliagePainter painter;
    FoliageType t1("Grass");
    FoliageType t2("Bush");
    REQUIRE(painter.addType(t1));
    REQUIRE(painter.addType(t2));
    REQUIRE(painter.typeCount() == 2);
    REQUIRE(painter.setActiveType("Grass"));
    REQUIRE(painter.activeType() == "Grass");
    painter.removeType("Grass");
    REQUIRE(painter.activeType().empty());
}

TEST_CASE("FoliagePainter rejects duplicate type name", "[Editor][S91]") {
    FoliagePainter painter;
    FoliageType t("Tree");
    painter.addType(t);
    REQUIRE_FALSE(painter.addType(t));
}

TEST_CASE("FoliagePainter countByCullMode and shadowCastingCount", "[Editor][S91]") {
    FoliagePainter painter;
    FoliageType t1("A"); t1.setCullMode(FoliageCullMode::Distance); t1.setCastsShadow(true);
    FoliageType t2("B"); t2.setCullMode(FoliageCullMode::Frustum);  t2.setCastsShadow(false);
    FoliageType t3("C"); t3.setCullMode(FoliageCullMode::Distance); t3.setCastsShadow(true);
    painter.addType(t1); painter.addType(t2); painter.addType(t3);
    REQUIRE(painter.countByCullMode(FoliageCullMode::Distance) == 2);
    REQUIRE(painter.shadowCastingCount() == 2);
}

TEST_CASE("FoliagePainter MAX_TYPES is 64", "[Editor][S91]") {
    REQUIRE(FoliagePainter::MAX_TYPES == 64);
}
