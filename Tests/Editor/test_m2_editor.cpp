#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/EditorApp.h"
#include "NF/Editor/Legacy/PCGTuning.h"
#include "NF/Editor/Scene/EntityPlacement.h"
#include "NF/Editor/Scene/VoxelPaint.h"
#include "NF/Workspace/EditorUndoSystem.h"
#include "NF/Editor/Scene/WorldPreview.h"

// ── PCGTuningPanel Tests ────────────────────────────────────────

TEST_CASE("PCGTuningPanel default state", "[Editor][M2]") {
    NF::PCGTuningPanel panel;

    REQUIRE(panel.name() == "PCGTuning");
    REQUIRE(panel.slot() == NF::DockSlot::Right);
    REQUIRE(panel.presetCount() == 0);
    REQUIRE_FALSE(panel.isDirty());

    auto& np = panel.noiseParams();
    REQUIRE(np.frequency == 1.0f);
    REQUIRE(np.octaves == 4);
    REQUIRE(np.seed == 42);
}

TEST_CASE("PCGTuningPanel set noise params marks dirty", "[Editor][M2]") {
    NF::PCGTuningPanel panel;
    NF::NoiseParams p;
    p.frequency = 2.5f;
    p.amplitude = 0.8f;
    p.seed = 99;

    panel.setNoiseParams(p);
    REQUIRE(panel.noiseParams().frequency == 2.5f);
    REQUIRE(panel.noiseParams().seed == 99);
    REQUIRE(panel.isDirty());

    panel.clearDirty();
    REQUIRE_FALSE(panel.isDirty());
}

TEST_CASE("PCGTuningPanel presets", "[Editor][M2]") {
    NF::PCGTuningPanel panel;
    NF::NoiseParams p;
    p.frequency = 3.0f;
    p.seed = 100;

    panel.addPreset({"Mountains", p});
    panel.addPreset({"Plains", {}});
    REQUIRE(panel.presetCount() == 2);

    REQUIRE(panel.applyPreset("Mountains"));
    REQUIRE(panel.noiseParams().frequency == 3.0f);
    REQUIRE(panel.isDirty());

    REQUIRE(panel.removePreset("Plains"));
    REQUIRE(panel.presetCount() == 1);
    REQUIRE_FALSE(panel.removePreset("NonExistent"));
    REQUIRE_FALSE(panel.applyPreset("NonExistent"));
}

TEST_CASE("PCGTuningPanel seed operations", "[Editor][M2]") {
    NF::PCGTuningPanel panel;
    panel.setSeed(123);
    REQUIRE(panel.noiseParams().seed == 123);

    int before = panel.noiseParams().seed;
    panel.randomizeSeed();
    REQUIRE(panel.noiseParams().seed != before);
}

// ── EntityPlacementTool Tests ───────────────────────────────────

TEST_CASE("EntityPlacementTool place and remove", "[Editor][M2]") {
    NF::EntityPlacementTool tool;
    tool.setActiveTemplate("Tree");
    REQUIRE(tool.activeTemplate() == "Tree");
    REQUIRE(tool.placedCount() == 0);

    NF::EntityID id = tool.placeEntity({1.f, 2.f, 3.f});
    REQUIRE(id != NF::INVALID_ENTITY);
    REQUIRE(tool.placedCount() == 1);
    REQUIRE(tool.placedEntities()[0].templateName == "Tree");
    REQUIRE(tool.placedEntities()[0].position.x == 1.f);

    REQUIRE(tool.removeEntity(id));
    REQUIRE(tool.placedCount() == 0);
    REQUIRE_FALSE(tool.removeEntity(id));
}

TEST_CASE("EntityPlacementTool auto-increment IDs", "[Editor][M2]") {
    NF::EntityPlacementTool tool;
    NF::EntityID id1 = tool.placeEntity({0.f, 0.f, 0.f});
    NF::EntityID id2 = tool.placeEntity({1.f, 0.f, 0.f});
    REQUIRE(id2 > id1);
}

TEST_CASE("EntityPlacementTool grid snap", "[Editor][M2]") {
    NF::EntityPlacementTool tool;
    REQUIRE_FALSE(tool.isGridSnapEnabled());

    tool.setGridSnap(true);
    tool.setGridSize(2.0f);
    REQUIRE(tool.isGridSnapEnabled());
    REQUIRE(tool.gridSize() == 2.0f);

    NF::Vec3 snapped = tool.snapToGrid({1.3f, 2.7f, -0.5f});
    REQUIRE(snapped.x == 2.0f);
    REQUIRE(snapped.y == 2.0f);
    REQUIRE(snapped.z == 0.0f);

    tool.setActiveTemplate("Rock");
    NF::EntityID id = tool.placeEntity({1.3f, 2.7f, -0.5f});
    REQUIRE(tool.placedEntities().back().position.x == 2.0f);
}

TEST_CASE("EntityPlacementTool clear", "[Editor][M2]") {
    NF::EntityPlacementTool tool;
    tool.placeEntity({0.f, 0.f, 0.f});
    tool.placeEntity({1.f, 0.f, 0.f});
    REQUIRE(tool.placedCount() == 2);
    tool.clear();
    REQUIRE(tool.placedCount() == 0);
}

// ── VoxelPaintTool Tests ────────────────────────────────────────

TEST_CASE("VoxelPaintTool default brush", "[Editor][M2]") {
    NF::VoxelPaintTool tool;
    REQUIRE(tool.brush().shape == NF::VoxelBrushShape::Sphere);
    REQUIRE(tool.brush().radius == 1);
    REQUIRE(tool.brush().strength == 1.0f);
    REQUIRE_FALSE(tool.isStroking());
    REQUIRE(tool.strokeCount() == 0);
}

TEST_CASE("VoxelPaintTool stroke workflow", "[Editor][M2]") {
    NF::VoxelPaintTool tool;
    NF::VoxelBrushSettings b;
    b.shape = NF::VoxelBrushShape::Cube;
    b.radius = 3;
    b.materialId = 42;
    b.strength = 0.8f;
    tool.setBrush(b);

    tool.beginStroke();
    REQUIRE(tool.isStroking());

    tool.addToStroke({0, 0, 0});
    tool.addToStroke({1, 0, 0});
    tool.addToStroke({2, 0, 0});

    tool.endStroke();
    REQUIRE_FALSE(tool.isStroking());
    REQUIRE(tool.strokeCount() == 1);
    REQUIRE(tool.strokes()[0].positions.size() == 3);
    REQUIRE(tool.strokes()[0].materialId == 42);
    REQUIRE(tool.strokes()[0].brush.shape == NF::VoxelBrushShape::Cube);
}

TEST_CASE("VoxelPaintTool palette", "[Editor][M2]") {
    NF::VoxelPaintTool tool;
    tool.setPaletteSlot(0, 100);
    tool.setPaletteSlot(1, 200);
    tool.setPaletteSlot(3, 400);

    REQUIRE(tool.paletteSize() == 4);
    REQUIRE(tool.getPaletteSlot(0) == 100);
    REQUIRE(tool.getPaletteSlot(1) == 200);
    REQUIRE(tool.getPaletteSlot(2) == 0);
    REQUIRE(tool.getPaletteSlot(3) == 400);
    REQUIRE(tool.getPaletteSlot(99) == 0);
}

TEST_CASE("VoxelPaintTool active palette slot", "[Editor][M2]") {
    NF::VoxelPaintTool tool;
    tool.setPaletteSlot(0, 100);
    tool.setPaletteSlot(1, 200);

    tool.setActivePaletteSlot(1);
    REQUIRE(tool.activePaletteSlot() == 1);
    REQUIRE(tool.brush().materialId == 200);
}

TEST_CASE("VoxelPaintTool clear", "[Editor][M2]") {
    NF::VoxelPaintTool tool;
    tool.beginStroke();
    tool.addToStroke({0, 0, 0});
    tool.endStroke();
    REQUIRE(tool.strokeCount() == 1);

    tool.clear();
    REQUIRE(tool.strokeCount() == 0);
    REQUIRE_FALSE(tool.isStroking());
}

// ── EditorUndoSystem Tests ──────────────────────────────────────

TEST_CASE("EditorUndoSystem place entity undo/redo", "[Editor][M2]") {
    NF::CommandStack stack;
    NF::EditorUndoSystem undo(stack);
    NF::EntityPlacementTool tool;

    NF::PlacedEntity e;
    e.entityId = 1;
    e.templateName = "Barrel";
    e.position = {5.f, 0.f, 5.f};

    undo.executePlaceEntity(tool, e);
    REQUIRE(tool.placedCount() == 1);
    REQUIRE(undo.canUndo());
    REQUIRE(undo.undoCount() == 1);

    undo.undo();
    REQUIRE(tool.placedCount() == 0);
    REQUIRE(undo.canRedo());
    REQUIRE(undo.redoCount() == 1);

    undo.redo();
    REQUIRE(tool.placedCount() == 1);
}

TEST_CASE("EditorUndoSystem remove entity undo", "[Editor][M2]") {
    NF::CommandStack stack;
    NF::EditorUndoSystem undo(stack);
    NF::EntityPlacementTool tool;

    NF::PlacedEntity e;
    e.entityId = 1;
    e.templateName = "Crate";
    e.position = {10.f, 0.f, 10.f};
    tool.addEntity(e);
    REQUIRE(tool.placedCount() == 1);

    undo.executeRemoveEntity(tool, 1);
    REQUIRE(tool.placedCount() == 0);

    undo.undo();
    REQUIRE(tool.placedCount() == 1);
    REQUIRE(tool.placedEntities()[0].templateName == "Crate");
}

TEST_CASE("EditorUndoSystem paint stroke undo", "[Editor][M2]") {
    NF::CommandStack stack;
    NF::EditorUndoSystem undo(stack);
    NF::VoxelPaintTool tool;

    NF::PaintStroke stroke;
    stroke.positions = {{0, 0, 0}, {1, 1, 1}};
    stroke.materialId = 5;

    undo.executePaintStroke(tool, stroke);
    REQUIRE(tool.strokeCount() == 1);

    undo.undo();
    REQUIRE(tool.strokeCount() == 0);

    undo.redo();
    REQUIRE(tool.strokeCount() == 1);
}

TEST_CASE("EditorUndoSystem PCG param change undo", "[Editor][M2]") {
    NF::PCGTuningPanel panel;
    NF::CommandStack stack;
    NF::EditorUndoSystem undo(stack);

    NF::NoiseParams oldP = panel.noiseParams();
    NF::NoiseParams newP;
    newP.frequency = 5.0f;
    newP.seed = 999;

    undo.executePCGChange(panel, oldP, newP);
    REQUIRE(panel.noiseParams().frequency == 5.0f);
    REQUIRE(panel.noiseParams().seed == 999);

    undo.undo();
    REQUIRE(panel.noiseParams().frequency == oldP.frequency);
    REQUIRE(panel.noiseParams().seed == oldP.seed);
}

TEST_CASE("EditorUndoSystem multiple operations", "[Editor][M2]") {
    NF::CommandStack stack;
    NF::EditorUndoSystem undo(stack);
    NF::EntityPlacementTool tool;

    NF::PlacedEntity e1{1, "A", {0,0,0}, {0,0,0}, {1,1,1}};
    NF::PlacedEntity e2{2, "B", {1,0,0}, {0,0,0}, {1,1,1}};

    undo.executePlaceEntity(tool, e1);
    undo.executePlaceEntity(tool, e2);
    REQUIRE(undo.undoCount() == 2);

    undo.undo();
    REQUIRE(tool.placedCount() == 1);
    undo.undo();
    REQUIRE(tool.placedCount() == 0);
    REQUIRE_FALSE(undo.canUndo());
    REQUIRE(undo.canRedo());
}

// ── WorldPreviewService Tests ───────────────────────────────────

TEST_CASE("WorldPreviewService default state", "[Editor][M2]") {
    NF::WorldPreviewService svc;

    REQUIRE(svc.state() == NF::PreviewState::Idle);
    REQUIRE(svc.worldPath().empty());
    REQUIRE_FALSE(svc.isDirty());
    REQUIRE(svc.viewRadius() == 100.0f);
}

TEST_CASE("WorldPreviewService load and unload", "[Editor][M2]") {
    NF::WorldPreviewService svc;

    svc.loadPreview("worlds/test.nfw");
    REQUIRE(svc.state() == NF::PreviewState::Ready);
    REQUIRE(svc.worldPath() == "worlds/test.nfw");

    svc.unloadPreview();
    REQUIRE(svc.state() == NF::PreviewState::Idle);
    REQUIRE(svc.worldPath().empty());
}

TEST_CASE("WorldPreviewService empty path error", "[Editor][M2]") {
    NF::WorldPreviewService svc;

    svc.loadPreview("");
    REQUIRE(svc.state() == NF::PreviewState::Error);
    REQUIRE_FALSE(svc.lastError().empty());
}

TEST_CASE("WorldPreviewService view controls", "[Editor][M2]") {
    NF::WorldPreviewService svc;

    svc.setViewCenter({10.f, 20.f, 30.f});
    REQUIRE(svc.viewCenter().x == 10.f);
    REQUIRE(svc.viewCenter().y == 20.f);
    REQUIRE(svc.viewCenter().z == 30.f);

    svc.setViewRadius(50.0f);
    REQUIRE(svc.viewRadius() == 50.0f);
}

TEST_CASE("WorldPreviewService dirty flag", "[Editor][M2]") {
    NF::WorldPreviewService svc;

    REQUIRE_FALSE(svc.isDirty());
    svc.setDirty();
    REQUIRE(svc.isDirty());
    svc.clearDirty();
    REQUIRE_FALSE(svc.isDirty());
}

// ── Vec3i Tests ─────────────────────────────────────────────────

TEST_CASE("Vec3i equality", "[Editor][M2]") {
    NF::Vec3i a{1, 2, 3};
    NF::Vec3i b{1, 2, 3};
    NF::Vec3i c{4, 5, 6};

    REQUIRE(a == b);
    REQUIRE(a != c);
}

// ── Integration: EditorApp M2 wiring ────────────────────────────

TEST_CASE("EditorApp M2 systems accessible", "[Editor][M2]") {
    NF::EditorApp app;
    app.init(800, 600);

    REQUIRE(app.pcgTuningPanel() != nullptr);
    REQUIRE(app.pcgTuningPanel()->name() == "PCGTuning");

    app.entityPlacementTool().setActiveTemplate("Pillar");
    REQUIRE(app.entityPlacementTool().activeTemplate() == "Pillar");

    app.voxelPaintTool().setPaletteSlot(0, 77);
    REQUIRE(app.voxelPaintTool().getPaletteSlot(0) == 77);

    REQUIRE_FALSE(app.editorUndoSystem().canUndo());

    app.worldPreview().loadPreview("test.nfw");
    REQUIRE(app.worldPreview().state() == NF::PreviewState::Ready);

    app.shutdown();
}
