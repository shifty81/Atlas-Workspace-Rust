// S181 editor tests: WaterSimEditorV1, WeatherSystemEditorV1, VoxelPaintEditorV1
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/WaterSimEditorV1.h"
#include "NF/Editor/WeatherSystemEditorV1.h"
#include "NF/Editor/VoxelPaintEditorV1.h"

using namespace NF;
using Catch::Approx;

// ── WaterSimEditorV1 ─────────────────────────────────────────────────────────

TEST_CASE("Wsv1WaterBody validity", "[Editor][S181]") {
    Wsv1WaterBody b;
    REQUIRE(!b.isValid());
    b.id = 1; b.name = "MainOcean";
    REQUIRE(b.isValid());
}

TEST_CASE("WaterSimEditorV1 addBody and bodyCount", "[Editor][S181]") {
    WaterSimEditorV1 ws;
    REQUIRE(ws.bodyCount() == 0);
    Wsv1WaterBody b; b.id = 1; b.name = "Ocean";
    REQUIRE(ws.addBody(b));
    REQUIRE(ws.bodyCount() == 1);
}

TEST_CASE("WaterSimEditorV1 addBody invalid fails", "[Editor][S181]") {
    WaterSimEditorV1 ws;
    REQUIRE(!ws.addBody(Wsv1WaterBody{}));
}

TEST_CASE("WaterSimEditorV1 addBody duplicate fails", "[Editor][S181]") {
    WaterSimEditorV1 ws;
    Wsv1WaterBody b; b.id = 1; b.name = "A";
    ws.addBody(b);
    REQUIRE(!ws.addBody(b));
}

TEST_CASE("WaterSimEditorV1 removeBody", "[Editor][S181]") {
    WaterSimEditorV1 ws;
    Wsv1WaterBody b; b.id = 2; b.name = "B";
    ws.addBody(b);
    REQUIRE(ws.removeBody(2));
    REQUIRE(ws.bodyCount() == 0);
    REQUIRE(!ws.removeBody(2));
}

TEST_CASE("WaterSimEditorV1 addWave and waveCount", "[Editor][S181]") {
    WaterSimEditorV1 ws;
    Wsv1Wave w; w.id = 1; w.bodyId = 10; w.amplitude = 2.f; w.frequency = 0.5f;
    REQUIRE(ws.addWave(w));
    REQUIRE(ws.waveCount() == 1);
}

TEST_CASE("WaterSimEditorV1 addWave invalid fails", "[Editor][S181]") {
    WaterSimEditorV1 ws;
    REQUIRE(!ws.addWave(Wsv1Wave{}));
}

TEST_CASE("WaterSimEditorV1 removeWave", "[Editor][S181]") {
    WaterSimEditorV1 ws;
    Wsv1Wave w; w.id = 1; w.bodyId = 5;
    ws.addWave(w);
    REQUIRE(ws.removeWave(1));
    REQUIRE(ws.waveCount() == 0);
    REQUIRE(!ws.removeWave(1));
}

TEST_CASE("WaterSimEditorV1 activeCount and pausedCount", "[Editor][S181]") {
    WaterSimEditorV1 ws;
    Wsv1WaterBody b1; b1.id = 1; b1.name = "A"; b1.state = Wsv1SimState::Active;
    Wsv1WaterBody b2; b2.id = 2; b2.name = "B"; b2.state = Wsv1SimState::Paused;
    ws.addBody(b1); ws.addBody(b2);
    REQUIRE(ws.activeCount() == 1);
    REQUIRE(ws.pausedCount() == 1);
}

TEST_CASE("WaterSimEditorV1 countByBodyType", "[Editor][S181]") {
    WaterSimEditorV1 ws;
    Wsv1WaterBody b1; b1.id = 1; b1.name = "A"; b1.type = Wsv1BodyType::Lake;
    Wsv1WaterBody b2; b2.id = 2; b2.name = "B"; b2.type = Wsv1BodyType::River;
    ws.addBody(b1); ws.addBody(b2);
    REQUIRE(ws.countByBodyType(Wsv1BodyType::Lake)  == 1);
    REQUIRE(ws.countByBodyType(Wsv1BodyType::River) == 1);
}

TEST_CASE("WaterSimEditorV1 findBody returns ptr", "[Editor][S181]") {
    WaterSimEditorV1 ws;
    Wsv1WaterBody b; b.id = 5; b.name = "Pool";
    ws.addBody(b);
    REQUIRE(ws.findBody(5) != nullptr);
    REQUIRE(ws.findBody(5)->name == "Pool");
    REQUIRE(ws.findBody(99) == nullptr);
}

TEST_CASE("wsv1BodyTypeName covers all values", "[Editor][S181]") {
    REQUIRE(std::string(wsv1BodyTypeName(Wsv1BodyType::Ocean))     == "Ocean");
    REQUIRE(std::string(wsv1BodyTypeName(Wsv1BodyType::Pool))      == "Pool");
    REQUIRE(std::string(wsv1BodyTypeName(Wsv1BodyType::Waterfall)) == "Waterfall");
}

TEST_CASE("wsv1SimStateName covers all values", "[Editor][S181]") {
    REQUIRE(std::string(wsv1SimStateName(Wsv1SimState::Idle))     == "Idle");
    REQUIRE(std::string(wsv1SimStateName(Wsv1SimState::Error))    == "Error");
    REQUIRE(std::string(wsv1SimStateName(Wsv1SimState::Disabled)) == "Disabled");
}

TEST_CASE("Wsv1WaterBody state helpers", "[Editor][S181]") {
    Wsv1WaterBody b; b.id = 1; b.name = "X";
    b.state = Wsv1SimState::Active;
    REQUIRE(b.isActive());
    b.state = Wsv1SimState::Paused;
    REQUIRE(b.isPaused());
}

TEST_CASE("WaterSimEditorV1 onChange callback", "[Editor][S181]") {
    WaterSimEditorV1 ws;
    uint64_t notified = 0;
    ws.setOnChange([&](uint64_t id) { notified = id; });
    Wsv1WaterBody b; b.id = 7; b.name = "Lake";
    ws.addBody(b);
    REQUIRE(notified == 7);
}

// ── WeatherSystemEditorV1 ────────────────────────────────────────────────────

TEST_CASE("Wthv1Zone validity", "[Editor][S181]") {
    Wthv1Zone z;
    REQUIRE(!z.isValid());
    z.id = 1; z.name = "Arctic";
    REQUIRE(z.isValid());
}

TEST_CASE("WeatherSystemEditorV1 addZone and zoneCount", "[Editor][S181]") {
    WeatherSystemEditorV1 wth;
    REQUIRE(wth.zoneCount() == 0);
    Wthv1Zone z; z.id = 1; z.name = "Z1";
    REQUIRE(wth.addZone(z));
    REQUIRE(wth.zoneCount() == 1);
}

TEST_CASE("WeatherSystemEditorV1 addZone invalid fails", "[Editor][S181]") {
    WeatherSystemEditorV1 wth;
    REQUIRE(!wth.addZone(Wthv1Zone{}));
}

TEST_CASE("WeatherSystemEditorV1 addZone duplicate fails", "[Editor][S181]") {
    WeatherSystemEditorV1 wth;
    Wthv1Zone z; z.id = 1; z.name = "A";
    wth.addZone(z);
    REQUIRE(!wth.addZone(z));
}

TEST_CASE("WeatherSystemEditorV1 removeZone", "[Editor][S181]") {
    WeatherSystemEditorV1 wth;
    Wthv1Zone z; z.id = 2; z.name = "B";
    wth.addZone(z);
    REQUIRE(wth.removeZone(2));
    REQUIRE(wth.zoneCount() == 0);
    REQUIRE(!wth.removeZone(2));
}

TEST_CASE("WeatherSystemEditorV1 addTransition and transitionCount", "[Editor][S181]") {
    WeatherSystemEditorV1 wth;
    Wthv1Transition tr; tr.id = 1; tr.zoneId = 10;
    tr.fromCondition = Wthv1Condition::Clear; tr.toCondition = Wthv1Condition::Rain;
    REQUIRE(wth.addTransition(tr));
    REQUIRE(wth.transitionCount() == 1);
}

TEST_CASE("WeatherSystemEditorV1 addTransition invalid fails", "[Editor][S181]") {
    WeatherSystemEditorV1 wth;
    REQUIRE(!wth.addTransition(Wthv1Transition{}));
}

TEST_CASE("WeatherSystemEditorV1 activeCount and lockedCount", "[Editor][S181]") {
    WeatherSystemEditorV1 wth;
    Wthv1Zone z1; z1.id = 1; z1.name = "A"; z1.state = Wthv1ZoneState::Active;
    Wthv1Zone z2; z2.id = 2; z2.name = "B"; z2.state = Wthv1ZoneState::Locked;
    wth.addZone(z1); wth.addZone(z2);
    REQUIRE(wth.activeCount() == 1);
    REQUIRE(wth.lockedCount() == 1);
}

TEST_CASE("WeatherSystemEditorV1 countByCondition", "[Editor][S181]") {
    WeatherSystemEditorV1 wth;
    Wthv1Zone z1; z1.id = 1; z1.name = "A"; z1.condition = Wthv1Condition::Snow;
    Wthv1Zone z2; z2.id = 2; z2.name = "B"; z2.condition = Wthv1Condition::Fog;
    wth.addZone(z1); wth.addZone(z2);
    REQUIRE(wth.countByCondition(Wthv1Condition::Snow) == 1);
    REQUIRE(wth.countByCondition(Wthv1Condition::Fog)  == 1);
}

TEST_CASE("WeatherSystemEditorV1 findZone returns ptr", "[Editor][S181]") {
    WeatherSystemEditorV1 wth;
    Wthv1Zone z; z.id = 6; z.name = "Desert";
    wth.addZone(z);
    REQUIRE(wth.findZone(6) != nullptr);
    REQUIRE(wth.findZone(6)->name == "Desert");
    REQUIRE(wth.findZone(99) == nullptr);
}

TEST_CASE("wthv1ConditionName covers all values", "[Editor][S181]") {
    REQUIRE(std::string(wthv1ConditionName(Wthv1Condition::Clear))        == "Clear");
    REQUIRE(std::string(wthv1ConditionName(Wthv1Condition::Thunderstorm)) == "Thunderstorm");
    REQUIRE(std::string(wthv1ConditionName(Wthv1Condition::Fog))          == "Fog");
}

TEST_CASE("wthv1ZoneStateName covers all values", "[Editor][S181]") {
    REQUIRE(std::string(wthv1ZoneStateName(Wthv1ZoneState::Inactive))     == "Inactive");
    REQUIRE(std::string(wthv1ZoneStateName(Wthv1ZoneState::Transitioning))== "Transitioning");
    REQUIRE(std::string(wthv1ZoneStateName(Wthv1ZoneState::Disabled))     == "Disabled");
}

TEST_CASE("Wthv1Zone state helpers", "[Editor][S181]") {
    Wthv1Zone z; z.id = 1; z.name = "X";
    z.state = Wthv1ZoneState::Active;
    REQUIRE(z.isActive());
    z.state = Wthv1ZoneState::Transitioning;
    REQUIRE(z.isTransitioning());
    z.state = Wthv1ZoneState::Locked;
    REQUIRE(z.isLocked());
}

TEST_CASE("WeatherSystemEditorV1 onChange callback", "[Editor][S181]") {
    WeatherSystemEditorV1 wth;
    uint64_t notified = 0;
    wth.setOnChange([&](uint64_t id) { notified = id; });
    Wthv1Zone z; z.id = 3; z.name = "C";
    wth.addZone(z);
    REQUIRE(notified == 3);
}

// ── VoxelPaintEditorV1 ───────────────────────────────────────────────────────

TEST_CASE("Vpv1Layer validity", "[Editor][S181]") {
    Vpv1Layer l;
    REQUIRE(!l.isValid());
    l.id = 1; l.name = "Base";
    REQUIRE(l.isValid());
}

TEST_CASE("VoxelPaintEditorV1 addLayer and layerCount", "[Editor][S181]") {
    VoxelPaintEditorV1 vp;
    REQUIRE(vp.layerCount() == 0);
    Vpv1Layer l; l.id = 1; l.name = "Ground";
    REQUIRE(vp.addLayer(l));
    REQUIRE(vp.layerCount() == 1);
}

TEST_CASE("VoxelPaintEditorV1 addLayer invalid fails", "[Editor][S181]") {
    VoxelPaintEditorV1 vp;
    REQUIRE(!vp.addLayer(Vpv1Layer{}));
}

TEST_CASE("VoxelPaintEditorV1 addLayer duplicate fails", "[Editor][S181]") {
    VoxelPaintEditorV1 vp;
    Vpv1Layer l; l.id = 1; l.name = "A";
    vp.addLayer(l);
    REQUIRE(!vp.addLayer(l));
}

TEST_CASE("VoxelPaintEditorV1 removeLayer", "[Editor][S181]") {
    VoxelPaintEditorV1 vp;
    Vpv1Layer l; l.id = 2; l.name = "B";
    vp.addLayer(l);
    REQUIRE(vp.removeLayer(2));
    REQUIRE(vp.layerCount() == 0);
    REQUIRE(!vp.removeLayer(2));
}

TEST_CASE("VoxelPaintEditorV1 addStroke and strokeCount", "[Editor][S181]") {
    VoxelPaintEditorV1 vp;
    Vpv1Stroke s; s.id = 1; s.layerId = 10; s.materialId = 5;
    REQUIRE(vp.addStroke(s));
    REQUIRE(vp.strokeCount() == 1);
}

TEST_CASE("VoxelPaintEditorV1 addStroke invalid fails", "[Editor][S181]") {
    VoxelPaintEditorV1 vp;
    REQUIRE(!vp.addStroke(Vpv1Stroke{}));
}

TEST_CASE("VoxelPaintEditorV1 removeStroke", "[Editor][S181]") {
    VoxelPaintEditorV1 vp;
    Vpv1Stroke s; s.id = 1; s.layerId = 5;
    vp.addStroke(s);
    REQUIRE(vp.removeStroke(1));
    REQUIRE(vp.strokeCount() == 0);
    REQUIRE(!vp.removeStroke(1));
}

TEST_CASE("VoxelPaintEditorV1 visibleCount and lockedCount", "[Editor][S181]") {
    VoxelPaintEditorV1 vp;
    Vpv1Layer l1; l1.id = 1; l1.name = "A"; l1.state = Vpv1LayerState::Visible;
    Vpv1Layer l2; l2.id = 2; l2.name = "B"; l2.state = Vpv1LayerState::Locked;
    vp.addLayer(l1); vp.addLayer(l2);
    REQUIRE(vp.visibleCount() == 1);
    REQUIRE(vp.lockedCount()  == 1);
}

TEST_CASE("VoxelPaintEditorV1 countByBrushShape", "[Editor][S181]") {
    VoxelPaintEditorV1 vp;
    Vpv1Stroke s1; s1.id = 1; s1.layerId = 1; s1.brushShape = Vpv1BrushShape::Cube;
    Vpv1Stroke s2; s2.id = 2; s2.layerId = 1; s2.brushShape = Vpv1BrushShape::Cylinder;
    vp.addStroke(s1); vp.addStroke(s2);
    REQUIRE(vp.countByBrushShape(Vpv1BrushShape::Cube)     == 1);
    REQUIRE(vp.countByBrushShape(Vpv1BrushShape::Cylinder) == 1);
}

TEST_CASE("VoxelPaintEditorV1 findLayer returns ptr", "[Editor][S181]") {
    VoxelPaintEditorV1 vp;
    Vpv1Layer l; l.id = 4; l.name = "Detail";
    vp.addLayer(l);
    REQUIRE(vp.findLayer(4) != nullptr);
    REQUIRE(vp.findLayer(4)->name == "Detail");
    REQUIRE(vp.findLayer(99) == nullptr);
}

TEST_CASE("vpv1BrushShapeName covers all values", "[Editor][S181]") {
    REQUIRE(std::string(vpv1BrushShapeName(Vpv1BrushShape::Sphere))   == "Sphere");
    REQUIRE(std::string(vpv1BrushShapeName(Vpv1BrushShape::Flat))     == "Flat");
    REQUIRE(std::string(vpv1BrushShapeName(Vpv1BrushShape::Custom))   == "Custom");
}

TEST_CASE("vpv1LayerStateName covers all values", "[Editor][S181]") {
    REQUIRE(std::string(vpv1LayerStateName(Vpv1LayerState::Hidden))  == "Hidden");
    REQUIRE(std::string(vpv1LayerStateName(Vpv1LayerState::Active))  == "Active");
}

TEST_CASE("Vpv1Layer state helpers", "[Editor][S181]") {
    Vpv1Layer l; l.id = 1; l.name = "X";
    l.state = Vpv1LayerState::Visible;
    REQUIRE(l.isVisible());
    l.state = Vpv1LayerState::Locked;
    REQUIRE(l.isLocked());
    l.state = Vpv1LayerState::Active;
    REQUIRE(l.isActive());
}

TEST_CASE("VoxelPaintEditorV1 onChange callback", "[Editor][S181]") {
    VoxelPaintEditorV1 vp;
    uint64_t notified = 0;
    vp.setOnChange([&](uint64_t id) { notified = id; });
    Vpv1Layer l; l.id = 9; l.name = "Top";
    vp.addLayer(l);
    REQUIRE(notified == 9);
}
