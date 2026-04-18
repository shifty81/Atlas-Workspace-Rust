// S196 editor tests: SpriteEditorV1, TilemapEditorV1, TexturePainterV1
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/SpriteEditorV1.h"
#include "NF/Editor/TilemapEditorV1.h"
#include "NF/Editor/TexturePainterV1.h"

using namespace NF;
using Catch::Approx;

// ── SpriteEditorV1 ───────────────────────────────────────────────────────────

TEST_CASE("Sev1Frame validity", "[Editor][S196]") {
    Sev1Frame f;
    REQUIRE(!f.isValid());
    f.id = 1; f.name = "Idle0"; f.width = 32.f; f.height = 32.f;
    REQUIRE(f.isValid());
}

TEST_CASE("Sev1Frame area", "[Editor][S196]") {
    Sev1Frame f; f.id = 1; f.name = "A"; f.width = 64.f; f.height = 32.f;
    REQUIRE(f.area() == Approx(2048.f));
}

TEST_CASE("SpriteEditorV1 addFrame and frameCount", "[Editor][S196]") {
    SpriteEditorV1 se;
    REQUIRE(se.frameCount() == 0);
    Sev1Frame f; f.id = 1; f.name = "F1"; f.width = 16; f.height = 16;
    REQUIRE(se.addFrame(f));
    REQUIRE(se.frameCount() == 1);
}

TEST_CASE("SpriteEditorV1 addFrame invalid fails", "[Editor][S196]") {
    SpriteEditorV1 se;
    REQUIRE(!se.addFrame(Sev1Frame{}));
}

TEST_CASE("SpriteEditorV1 addFrame duplicate fails", "[Editor][S196]") {
    SpriteEditorV1 se;
    Sev1Frame f; f.id = 1; f.name = "A"; f.width = 8; f.height = 8;
    se.addFrame(f);
    REQUIRE(!se.addFrame(f));
}

TEST_CASE("SpriteEditorV1 removeFrame", "[Editor][S196]") {
    SpriteEditorV1 se;
    Sev1Frame f; f.id = 2; f.name = "B"; f.width = 8; f.height = 8;
    se.addFrame(f);
    REQUIRE(se.removeFrame(2));
    REQUIRE(se.frameCount() == 0);
    REQUIRE(!se.removeFrame(2));
}

TEST_CASE("SpriteEditorV1 findFrame", "[Editor][S196]") {
    SpriteEditorV1 se;
    Sev1Frame f; f.id = 3; f.name = "C"; f.width = 16; f.height = 16;
    se.addFrame(f);
    REQUIRE(se.findFrame(3) != nullptr);
    REQUIRE(se.findFrame(99) == nullptr);
}

TEST_CASE("SpriteEditorV1 setState", "[Editor][S196]") {
    SpriteEditorV1 se;
    Sev1Frame f; f.id = 1; f.name = "F"; f.width = 8; f.height = 8;
    se.addFrame(f);
    REQUIRE(se.setState(1, Sev1FrameState::Hidden));
    REQUIRE(se.findFrame(1)->state == Sev1FrameState::Hidden);
}

TEST_CASE("SpriteEditorV1 setPivot", "[Editor][S196]") {
    SpriteEditorV1 se;
    Sev1Frame f; f.id = 1; f.name = "F"; f.width = 8; f.height = 8;
    se.addFrame(f);
    REQUIRE(se.setPivot(1, 0.25f, 0.75f));
    auto* found = se.findFrame(1);
    REQUIRE(found->pivotX == Approx(0.25f));
    REQUIRE(found->pivotY == Approx(0.75f));
}

TEST_CASE("SpriteEditorV1 activeCount", "[Editor][S196]") {
    SpriteEditorV1 se;
    Sev1Frame f1; f1.id = 1; f1.name = "A"; f1.width = 8; f1.height = 8;
    Sev1Frame f2; f2.id = 2; f2.name = "B"; f2.width = 8; f2.height = 8;
    se.addFrame(f1); se.addFrame(f2);
    se.setState(2, Sev1FrameState::Hidden);
    REQUIRE(se.activeCount() == 1);
}

TEST_CASE("SpriteEditorV1 totalDuration", "[Editor][S196]") {
    SpriteEditorV1 se;
    Sev1Frame f1; f1.id = 1; f1.name = "A"; f1.width = 8; f1.height = 8; f1.duration = 0.1f;
    Sev1Frame f2; f2.id = 2; f2.name = "B"; f2.width = 8; f2.height = 8; f2.duration = 0.2f;
    se.addFrame(f1); se.addFrame(f2);
    REQUIRE(se.totalDuration() == Approx(0.3f));
}

TEST_CASE("SpriteEditorV1 onChange callback", "[Editor][S196]") {
    SpriteEditorV1 se;
    uint64_t notified = 0;
    se.setOnChange([&](uint64_t id){ notified = id; });
    Sev1Frame f; f.id = 5; f.name = "X"; f.width = 8; f.height = 8;
    se.addFrame(f);
    REQUIRE(notified == 5);
}

TEST_CASE("sev1FrameStateName all values", "[Editor][S196]") {
    REQUIRE(std::string(sev1FrameStateName(Sev1FrameState::Active))     == "Active");
    REQUIRE(std::string(sev1FrameStateName(Sev1FrameState::Hidden))     == "Hidden");
    REQUIRE(std::string(sev1FrameStateName(Sev1FrameState::Locked))     == "Locked");
    REQUIRE(std::string(sev1FrameStateName(Sev1FrameState::Deprecated)) == "Deprecated");
}

// ── TilemapEditorV1 ──────────────────────────────────────────────────────────

TEST_CASE("Tev1Layer validity", "[Editor][S196]") {
    Tev1Layer l;
    REQUIRE(!l.isValid());
    l.id = 1; l.name = "Ground";
    REQUIRE(l.isValid());
}

TEST_CASE("TilemapEditorV1 addLayer and layerCount", "[Editor][S196]") {
    TilemapEditorV1 te;
    REQUIRE(te.layerCount() == 0);
    Tev1Layer l; l.id = 1; l.name = "L1";
    REQUIRE(te.addLayer(l));
    REQUIRE(te.layerCount() == 1);
}

TEST_CASE("TilemapEditorV1 addLayer invalid fails", "[Editor][S196]") {
    TilemapEditorV1 te;
    REQUIRE(!te.addLayer(Tev1Layer{}));
}

TEST_CASE("TilemapEditorV1 addLayer duplicate fails", "[Editor][S196]") {
    TilemapEditorV1 te;
    Tev1Layer l; l.id = 1; l.name = "A";
    te.addLayer(l);
    REQUIRE(!te.addLayer(l));
}

TEST_CASE("TilemapEditorV1 removeLayer", "[Editor][S196]") {
    TilemapEditorV1 te;
    Tev1Layer l; l.id = 2; l.name = "B";
    te.addLayer(l);
    REQUIRE(te.removeLayer(2));
    REQUIRE(te.layerCount() == 0);
    REQUIRE(!te.removeLayer(2));
}

TEST_CASE("TilemapEditorV1 findLayer", "[Editor][S196]") {
    TilemapEditorV1 te;
    Tev1Layer l; l.id = 3; l.name = "C";
    te.addLayer(l);
    REQUIRE(te.findLayer(3) != nullptr);
    REQUIRE(te.findLayer(99) == nullptr);
}

TEST_CASE("TilemapEditorV1 addTileToLayer", "[Editor][S196]") {
    TilemapEditorV1 te;
    Tev1Layer l; l.id = 1; l.name = "L";
    te.addLayer(l);
    Tev1Tile t; t.id = 10; t.name = "Grass";
    REQUIRE(te.addTileToLayer(1, t));
    REQUIRE(te.findLayer(1)->tiles.size() == 1);
}

TEST_CASE("TilemapEditorV1 addTileToLayer duplicate fails", "[Editor][S196]") {
    TilemapEditorV1 te;
    Tev1Layer l; l.id = 1; l.name = "L";
    te.addLayer(l);
    Tev1Tile t; t.id = 10; t.name = "Grass";
    te.addTileToLayer(1, t);
    REQUIRE(!te.addTileToLayer(1, t));
}

TEST_CASE("TilemapEditorV1 setLayerVisibility", "[Editor][S196]") {
    TilemapEditorV1 te;
    Tev1Layer l; l.id = 1; l.name = "L";
    te.addLayer(l);
    REQUIRE(te.setLayerVisibility(1, false));
    REQUIRE(!te.findLayer(1)->visible);
}

TEST_CASE("TilemapEditorV1 setLayerOpacity clamped", "[Editor][S196]") {
    TilemapEditorV1 te;
    Tev1Layer l; l.id = 1; l.name = "L";
    te.addLayer(l);
    te.setLayerOpacity(1, 5.f);
    REQUIRE(te.findLayer(1)->opacity == Approx(1.f));
    te.setLayerOpacity(1, -1.f);
    REQUIRE(te.findLayer(1)->opacity == Approx(0.f));
}

TEST_CASE("TilemapEditorV1 visibleLayerCount", "[Editor][S196]") {
    TilemapEditorV1 te;
    Tev1Layer l1; l1.id = 1; l1.name = "A";
    Tev1Layer l2; l2.id = 2; l2.name = "B";
    te.addLayer(l1); te.addLayer(l2);
    te.setLayerVisibility(2, false);
    REQUIRE(te.visibleLayerCount() == 1);
}

TEST_CASE("TilemapEditorV1 totalTileCount", "[Editor][S196]") {
    TilemapEditorV1 te;
    Tev1Layer l1; l1.id = 1; l1.name = "A";
    Tev1Layer l2; l2.id = 2; l2.name = "B";
    te.addLayer(l1); te.addLayer(l2);
    Tev1Tile t1; t1.id = 1; t1.name = "T1";
    Tev1Tile t2; t2.id = 2; t2.name = "T2";
    Tev1Tile t3; t3.id = 3; t3.name = "T3";
    te.addTileToLayer(1, t1);
    te.addTileToLayer(2, t2);
    te.addTileToLayer(2, t3);
    REQUIRE(te.totalTileCount() == 3);
}

TEST_CASE("TilemapEditorV1 onChange callback", "[Editor][S196]") {
    TilemapEditorV1 te;
    uint64_t notified = 0;
    te.setOnChange([&](uint64_t id){ notified = id; });
    Tev1Layer l; l.id = 7; l.name = "X";
    te.addLayer(l);
    REQUIRE(notified == 7);
}

TEST_CASE("tev1LayerBlendName all values", "[Editor][S196]") {
    REQUIRE(std::string(tev1LayerBlendName(Tev1LayerBlend::Normal))   == "Normal");
    REQUIRE(std::string(tev1LayerBlendName(Tev1LayerBlend::Additive)) == "Additive");
    REQUIRE(std::string(tev1LayerBlendName(Tev1LayerBlend::Multiply)) == "Multiply");
    REQUIRE(std::string(tev1LayerBlendName(Tev1LayerBlend::Screen))   == "Screen");
}

// ── TexturePainterV1 ─────────────────────────────────────────────────────────

TEST_CASE("Tpv1Brush validity", "[Editor][S196]") {
    Tpv1Brush b;
    REQUIRE(!b.isValid());
    b.id = 1; b.name = "Soft"; b.size = 16.f;
    REQUIRE(b.isValid());
}

TEST_CASE("TexturePainterV1 addBrush and brushCount", "[Editor][S196]") {
    TexturePainterV1 tp;
    REQUIRE(tp.brushCount() == 0);
    Tpv1Brush b; b.id = 1; b.name = "B1"; b.size = 8.f;
    REQUIRE(tp.addBrush(b));
    REQUIRE(tp.brushCount() == 1);
}

TEST_CASE("TexturePainterV1 addBrush invalid fails", "[Editor][S196]") {
    TexturePainterV1 tp;
    REQUIRE(!tp.addBrush(Tpv1Brush{}));
}

TEST_CASE("TexturePainterV1 addBrush duplicate fails", "[Editor][S196]") {
    TexturePainterV1 tp;
    Tpv1Brush b; b.id = 1; b.name = "A"; b.size = 8.f;
    tp.addBrush(b);
    REQUIRE(!tp.addBrush(b));
}

TEST_CASE("TexturePainterV1 removeBrush", "[Editor][S196]") {
    TexturePainterV1 tp;
    Tpv1Brush b; b.id = 2; b.name = "B"; b.size = 8.f;
    tp.addBrush(b);
    REQUIRE(tp.removeBrush(2));
    REQUIRE(tp.brushCount() == 0);
    REQUIRE(!tp.removeBrush(2));
}

TEST_CASE("TexturePainterV1 findBrush", "[Editor][S196]") {
    TexturePainterV1 tp;
    Tpv1Brush b; b.id = 3; b.name = "C"; b.size = 8.f;
    tp.addBrush(b);
    REQUIRE(tp.findBrush(3) != nullptr);
    REQUIRE(tp.findBrush(99) == nullptr);
}

TEST_CASE("TexturePainterV1 selectBrush", "[Editor][S196]") {
    TexturePainterV1 tp;
    Tpv1Brush b; b.id = 1; b.name = "B"; b.size = 8.f;
    tp.addBrush(b);
    REQUIRE(tp.selectBrush(1));
    REQUIRE(tp.activeBrushId() == 1);
    REQUIRE(!tp.selectBrush(99));
}

TEST_CASE("TexturePainterV1 recordStroke", "[Editor][S196]") {
    TexturePainterV1 tp;
    REQUIRE(tp.strokeCount() == 0);
    Tpv1Stroke s; s.id = 1; s.brushId = 10;
    REQUIRE(tp.recordStroke(s));
    REQUIRE(tp.strokeCount() == 1);
}

TEST_CASE("TexturePainterV1 recordStroke invalid fails", "[Editor][S196]") {
    TexturePainterV1 tp;
    REQUIRE(!tp.recordStroke(Tpv1Stroke{}));
}

TEST_CASE("TexturePainterV1 undoLastStroke", "[Editor][S196]") {
    TexturePainterV1 tp;
    Tpv1Stroke s; s.id = 1; s.brushId = 10;
    tp.recordStroke(s);
    REQUIRE(tp.undoLastStroke());
    REQUIRE(tp.strokeCount() == 0);
    REQUIRE(!tp.undoLastStroke());
}

TEST_CASE("TexturePainterV1 countByShape", "[Editor][S196]") {
    TexturePainterV1 tp;
    Tpv1Brush b1; b1.id = 1; b1.name = "A"; b1.size = 8.f; b1.shape = Tpv1BrushShape::Round;
    Tpv1Brush b2; b2.id = 2; b2.name = "B"; b2.size = 8.f; b2.shape = Tpv1BrushShape::Square;
    Tpv1Brush b3; b3.id = 3; b3.name = "C"; b3.size = 8.f; b3.shape = Tpv1BrushShape::Round;
    tp.addBrush(b1); tp.addBrush(b2); tp.addBrush(b3);
    REQUIRE(tp.countByShape(Tpv1BrushShape::Round) == 2);
    REQUIRE(tp.countByShape(Tpv1BrushShape::Square) == 1);
}

TEST_CASE("TexturePainterV1 onChange callback fires on recordStroke", "[Editor][S196]") {
    TexturePainterV1 tp;
    uint64_t notified = 0;
    tp.setOnChange([&](uint64_t id){ notified = id; });
    Tpv1Stroke s; s.id = 5; s.brushId = 10;
    tp.recordStroke(s);
    REQUIRE(notified == 5);
}

TEST_CASE("tpv1BrushShapeName all values", "[Editor][S196]") {
    REQUIRE(std::string(tpv1BrushShapeName(Tpv1BrushShape::Round))   == "Round");
    REQUIRE(std::string(tpv1BrushShapeName(Tpv1BrushShape::Square))  == "Square");
    REQUIRE(std::string(tpv1BrushShapeName(Tpv1BrushShape::Diamond)) == "Diamond");
    REQUIRE(std::string(tpv1BrushShapeName(Tpv1BrushShape::Custom))  == "Custom");
}

TEST_CASE("tpv1BlendModeName all values", "[Editor][S196]") {
    REQUIRE(std::string(tpv1BlendModeName(Tpv1BlendMode::Normal))   == "Normal");
    REQUIRE(std::string(tpv1BlendModeName(Tpv1BlendMode::Add))      == "Add");
    REQUIRE(std::string(tpv1BlendModeName(Tpv1BlendMode::Subtract)) == "Subtract");
    REQUIRE(std::string(tpv1BlendModeName(Tpv1BlendMode::Multiply)) == "Multiply");
}
