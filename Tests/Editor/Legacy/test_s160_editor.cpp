// S160 editor tests: TerrainBrushV1, FoliagePainterV1, RoadToolV1
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/RoadToolV1.h"
#include "NF/Editor/FoliagePainterV1.h"
#include "NF/Editor/TerrainBrushV1.h"

using namespace NF;
using Catch::Approx;

// ── TerrainBrushV1 ────────────────────────────────────────────────────────────

TEST_CASE("Tbv1BrushSettings validity", "[Editor][S160]") {
    Tbv1BrushSettings s;
    REQUIRE(s.isValid());
    s.radius = 0.f;
    REQUIRE(!s.isValid());
}

TEST_CASE("Tbv1Stroke validity", "[Editor][S160]") {
    Tbv1Stroke st;
    REQUIRE(!st.isValid());
    st.id = 1; st.settings.radius = 5.f; st.settings.strength = 0.5f;
    REQUIRE(st.isValid());
}

TEST_CASE("TerrainBrushV1 setBrushMode and getBrushMode", "[Editor][S160]") {
    TerrainBrushV1 tb;
    tb.setBrushMode(Tbv1BrushMode::Smooth);
    REQUIRE(tb.brushMode() == Tbv1BrushMode::Smooth);
}

TEST_CASE("TerrainBrushV1 setBrushShape and getBrushShape", "[Editor][S160]") {
    TerrainBrushV1 tb;
    tb.setBrushShape(Tbv1BrushShape::Square);
    REQUIRE(tb.brushShape() == Tbv1BrushShape::Square);
}

TEST_CASE("TerrainBrushV1 setBrushSettings and getBrushSettings", "[Editor][S160]") {
    TerrainBrushV1 tb;
    Tbv1BrushSettings s; s.radius = 20.f; s.strength = 0.8f;
    tb.setBrushSettings(s);
    REQUIRE(tb.brushSettings().radius == Approx(20.f));
    REQUIRE(tb.brushSettings().strength == Approx(0.8f));
}

TEST_CASE("TerrainBrushV1 applyStroke increments strokeCount", "[Editor][S160]") {
    TerrainBrushV1 tb;
    REQUIRE(tb.strokeCount() == 0);
    Tbv1Stroke st; st.id = 1; st.settings.radius = 5.f; st.settings.strength = 0.5f;
    REQUIRE(tb.applyStroke(st));
    REQUIRE(tb.strokeCount() == 1);
}

TEST_CASE("TerrainBrushV1 applyStroke invalid fails", "[Editor][S160]") {
    TerrainBrushV1 tb;
    Tbv1Stroke st;
    REQUIRE(!tb.applyStroke(st));
}

TEST_CASE("TerrainBrushV1 applyStroke fires callback", "[Editor][S160]") {
    TerrainBrushV1 tb;
    uint64_t firedId = 0;
    tb.setOnStroke([&](const Tbv1Stroke& s){ firedId = s.id; });
    Tbv1Stroke st; st.id = 7; st.settings.radius = 3.f; st.settings.strength = 0.3f;
    tb.applyStroke(st);
    REQUIRE(firedId == 7);
}

TEST_CASE("TerrainBrushV1 undoStroke", "[Editor][S160]") {
    TerrainBrushV1 tb;
    Tbv1Stroke st; st.id = 1; st.settings.radius = 5.f; st.settings.strength = 0.5f;
    tb.applyStroke(st);
    REQUIRE(tb.undoStroke());
    REQUIRE(tb.strokeCount() == 0);
    REQUIRE(!tb.undoStroke());
}

TEST_CASE("TerrainBrushV1 clearStrokes", "[Editor][S160]") {
    TerrainBrushV1 tb;
    Tbv1Stroke s1; s1.id = 1; s1.settings.radius = 5.f; s1.settings.strength = 0.5f;
    Tbv1Stroke s2; s2.id = 2; s2.settings.radius = 3.f; s2.settings.strength = 0.3f;
    tb.applyStroke(s1); tb.applyStroke(s2);
    REQUIRE(tb.clearStrokes());
    REQUIRE(tb.strokeCount() == 0);
    REQUIRE(!tb.clearStrokes());
}

// ── FoliagePainterV1 ──────────────────────────────────────────────────────────

TEST_CASE("Fpv1FoliageType validity", "[Editor][S160]") {
    Fpv1FoliageType ft;
    REQUIRE(!ft.isValid());
    ft.id = 1; ft.meshName = "Tree.fbx";
    REQUIRE(ft.isValid());
}

TEST_CASE("Fpv1PaintStroke validity", "[Editor][S160]") {
    Fpv1PaintStroke ps;
    REQUIRE(!ps.isValid());
    ps.id = 1; ps.typeId = 2;
    REQUIRE(ps.isValid());
}

TEST_CASE("FoliagePainterV1 addFoliageType and typeCount", "[Editor][S160]") {
    FoliagePainterV1 fp;
    REQUIRE(fp.typeCount() == 0);
    Fpv1FoliageType ft; ft.id = 1; ft.meshName = "Grass.fbx";
    REQUIRE(fp.addFoliageType(ft));
    REQUIRE(fp.typeCount() == 1);
}

TEST_CASE("FoliagePainterV1 addFoliageType invalid fails", "[Editor][S160]") {
    FoliagePainterV1 fp;
    Fpv1FoliageType ft;
    REQUIRE(!fp.addFoliageType(ft));
}

TEST_CASE("FoliagePainterV1 addFoliageType duplicate id fails", "[Editor][S160]") {
    FoliagePainterV1 fp;
    Fpv1FoliageType ft; ft.id = 1; ft.meshName = "Bush.fbx";
    fp.addFoliageType(ft);
    REQUIRE(!fp.addFoliageType(ft));
}

TEST_CASE("FoliagePainterV1 removeFoliageType", "[Editor][S160]") {
    FoliagePainterV1 fp;
    Fpv1FoliageType ft; ft.id = 2; ft.meshName = "Rock.fbx";
    fp.addFoliageType(ft);
    REQUIRE(fp.removeFoliageType(2));
    REQUIRE(fp.typeCount() == 0);
    REQUIRE(!fp.removeFoliageType(2));
}

TEST_CASE("FoliagePainterV1 paint and strokeCount", "[Editor][S160]") {
    FoliagePainterV1 fp;
    Fpv1PaintStroke ps; ps.id = 1; ps.typeId = 1;
    REQUIRE(fp.paint(ps));
    REQUIRE(fp.strokeCount() == 1);
}

TEST_CASE("FoliagePainterV1 paint invalid fails", "[Editor][S160]") {
    FoliagePainterV1 fp;
    Fpv1PaintStroke ps;
    REQUIRE(!fp.paint(ps));
}

TEST_CASE("FoliagePainterV1 paint fires callback", "[Editor][S160]") {
    FoliagePainterV1 fp;
    uint64_t paintedId = 0;
    fp.setOnPaint([&](const Fpv1PaintStroke& s){ paintedId = s.id; });
    Fpv1PaintStroke ps; ps.id = 5; ps.typeId = 1;
    fp.paint(ps);
    REQUIRE(paintedId == 5);
}

TEST_CASE("FoliagePainterV1 undoStroke", "[Editor][S160]") {
    FoliagePainterV1 fp;
    Fpv1PaintStroke ps; ps.id = 1; ps.typeId = 1;
    fp.paint(ps);
    REQUIRE(fp.undoStroke());
    REQUIRE(fp.strokeCount() == 0);
    REQUIRE(!fp.undoStroke());
}

TEST_CASE("FoliagePainterV1 clearStrokes", "[Editor][S160]") {
    FoliagePainterV1 fp;
    Fpv1PaintStroke ps; ps.id = 1; ps.typeId = 1;
    fp.paint(ps);
    REQUIRE(fp.clearStrokes());
    REQUIRE(fp.strokeCount() == 0);
    REQUIRE(!fp.clearStrokes());
}

TEST_CASE("FoliagePainterV1 setPlacementMode and getCullMode", "[Editor][S160]") {
    FoliagePainterV1 fp;
    fp.setPlacementMode(Fpv1PlacementMode::Aligned);
    REQUIRE(fp.placementMode() == Fpv1PlacementMode::Aligned);
    fp.setCullMode(Fpv1CullMode::Distance);
    REQUIRE(fp.cullMode() == Fpv1CullMode::Distance);
}

// ── RoadToolV1 ────────────────────────────────────────────────────────────────

TEST_CASE("Rtv1RoadNode validity", "[Editor][S160]") {
    Rtv1RoadNode n;
    REQUIRE(!n.isValid());
    n.id = 1;
    REQUIRE(n.isValid());
    n.width = 0.f;
    REQUIRE(!n.isValid());
}

TEST_CASE("Rtv1Road validity", "[Editor][S160]") {
    Rtv1Road r;
    REQUIRE(!r.isValid());
    r.id = 1; r.name = "Main St";
    REQUIRE(r.isValid());
}

TEST_CASE("RoadToolV1 addRoad and roadCount", "[Editor][S160]") {
    RoadToolV1 rt;
    REQUIRE(rt.roadCount() == 0);
    Rtv1Road r; r.id = 1; r.name = "Highway";
    REQUIRE(rt.addRoad(r));
    REQUIRE(rt.roadCount() == 1);
}

TEST_CASE("RoadToolV1 addRoad invalid fails", "[Editor][S160]") {
    RoadToolV1 rt;
    Rtv1Road r;
    REQUIRE(!rt.addRoad(r));
}

TEST_CASE("RoadToolV1 addRoad duplicate id fails", "[Editor][S160]") {
    RoadToolV1 rt;
    Rtv1Road r; r.id = 1; r.name = "A";
    rt.addRoad(r);
    REQUIRE(!rt.addRoad(r));
}

TEST_CASE("RoadToolV1 removeRoad", "[Editor][S160]") {
    RoadToolV1 rt;
    Rtv1Road r; r.id = 2; r.name = "Side St";
    rt.addRoad(r);
    REQUIRE(rt.removeRoad(2));
    REQUIRE(rt.roadCount() == 0);
    REQUIRE(!rt.removeRoad(2));
}

TEST_CASE("RoadToolV1 addNode to road", "[Editor][S160]") {
    RoadToolV1 rt;
    Rtv1Road r; r.id = 1; r.name = "Road1";
    rt.addRoad(r);
    Rtv1RoadNode n; n.id = 1; n.width = 4.f;
    REQUIRE(rt.addNode(1, n));
    auto* found = rt.findRoad(1);
    REQUIRE(found != nullptr);
    REQUIRE(found->nodes.size() == 1);
}

TEST_CASE("RoadToolV1 addNode invalid fails", "[Editor][S160]") {
    RoadToolV1 rt;
    Rtv1Road r; r.id = 1; r.name = "R";
    rt.addRoad(r);
    Rtv1RoadNode n; n.id = 1; n.width = 0.f;
    REQUIRE(!rt.addNode(1, n));
}

TEST_CASE("RoadToolV1 addNode unknown road fails", "[Editor][S160]") {
    RoadToolV1 rt;
    Rtv1RoadNode n; n.id = 1; n.width = 4.f;
    REQUIRE(!rt.addNode(99, n));
}

TEST_CASE("RoadToolV1 removeNode", "[Editor][S160]") {
    RoadToolV1 rt;
    Rtv1Road r; r.id = 1; r.name = "R";
    rt.addRoad(r);
    Rtv1RoadNode n; n.id = 3; n.width = 4.f;
    rt.addNode(1, n);
    REQUIRE(rt.removeNode(1, 3));
    auto* found = rt.findRoad(1);
    REQUIRE(found != nullptr);
    REQUIRE(found->nodes.empty());
    REQUIRE(!rt.removeNode(1, 3));
}

TEST_CASE("RoadToolV1 setRoadType", "[Editor][S160]") {
    RoadToolV1 rt;
    Rtv1Road r; r.id = 1; r.name = "Dirt";
    rt.addRoad(r);
    REQUIRE(rt.setRoadType(1, Rtv1RoadType::Dirt));
    auto* found = rt.findRoad(1);
    REQUIRE(found->type == Rtv1RoadType::Dirt);
}

TEST_CASE("RoadToolV1 setRoadType unknown road fails", "[Editor][S160]") {
    RoadToolV1 rt;
    REQUIRE(!rt.setRoadType(99, Rtv1RoadType::Gravel));
}

TEST_CASE("RoadToolV1 setClosed", "[Editor][S160]") {
    RoadToolV1 rt;
    Rtv1Road r; r.id = 1; r.name = "Loop";
    rt.addRoad(r);
    REQUIRE(rt.setClosed(1, true));
    auto* found = rt.findRoad(1);
    REQUIRE(found->closed);
}

TEST_CASE("RoadToolV1 onChange fires on addNode", "[Editor][S160]") {
    RoadToolV1 rt;
    Rtv1Road r; r.id = 1; r.name = "X";
    rt.addRoad(r);
    uint64_t changed = 0;
    rt.setOnChange([&](uint64_t id){ changed = id; });
    Rtv1RoadNode n; n.id = 1; n.width = 4.f;
    rt.addNode(1, n);
    REQUIRE(changed == 1);
}

TEST_CASE("RoadToolV1 findRoad unknown returns null", "[Editor][S160]") {
    RoadToolV1 rt;
    REQUIRE(rt.findRoad(999) == nullptr);
}
