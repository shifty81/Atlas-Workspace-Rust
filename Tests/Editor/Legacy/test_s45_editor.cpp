#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"

using namespace NF;

// ── PlaceEntityCommand ──────────────────────────────────────────

TEST_CASE("PlaceEntityCommand description includes template name", "[Editor][S45]") {
    EntityPlacementTool tool;
    PlacedEntity entity;
    entity.templateName = "Barrel";
    entity.entityId = 1;

    PlaceEntityCommand cmd(tool, entity);
    REQUIRE(cmd.description() == "Place Entity Barrel");
}

TEST_CASE("PlaceEntityCommand execute adds entity to tool", "[Editor][S45]") {
    EntityPlacementTool tool;
    PlacedEntity entity;
    entity.templateName = "Crate";
    entity.entityId = 42;

    PlaceEntityCommand cmd(tool, entity);
    cmd.execute();
    REQUIRE(tool.placedCount() == 1);
}

TEST_CASE("PlaceEntityCommand undo removes entity from tool", "[Editor][S45]") {
    EntityPlacementTool tool;
    PlacedEntity entity;
    entity.templateName = "Tree";
    entity.entityId = 7;

    PlaceEntityCommand cmd(tool, entity);
    cmd.execute();
    REQUIRE(tool.placedCount() == 1);

    cmd.undo();
    REQUIRE(tool.placedCount() == 0);
}

// ── RemoveEntityCommand ─────────────────────────────────────────

TEST_CASE("RemoveEntityCommand description includes entity id", "[Editor][S45]") {
    EntityPlacementTool tool;
    PlacedEntity entity;
    entity.templateName = "Rock";
    entity.entityId = 99;
    tool.addEntity(entity);

    RemoveEntityCommand cmd(tool, 99);
    REQUIRE(cmd.description() == "Remove Entity 99");
}

TEST_CASE("RemoveEntityCommand execute removes entity", "[Editor][S45]") {
    EntityPlacementTool tool;
    PlacedEntity entity;
    entity.templateName = "Wall";
    entity.entityId = 10;
    tool.addEntity(entity);
    REQUIRE(tool.placedCount() == 1);

    RemoveEntityCommand cmd(tool, 10);
    cmd.execute();
    REQUIRE(tool.placedCount() == 0);
}

TEST_CASE("RemoveEntityCommand undo restores entity", "[Editor][S45]") {
    EntityPlacementTool tool;
    PlacedEntity entity;
    entity.templateName = "Fence";
    entity.entityId = 33;
    tool.addEntity(entity);

    RemoveEntityCommand cmd(tool, 33);
    cmd.execute();
    REQUIRE(tool.placedCount() == 0);

    cmd.undo();
    REQUIRE(tool.placedCount() == 1);
}

// ── PaintStrokeCommand ──────────────────────────────────────────

TEST_CASE("PaintStrokeCommand description has voxel count", "[Editor][S45]") {
    VoxelPaintTool paintTool;
    PaintStroke stroke;
    stroke.positions.push_back({1, 2, 3});
    stroke.positions.push_back({4, 5, 6});
    stroke.positions.push_back({7, 8, 9});

    PaintStrokeCommand cmd(paintTool, stroke);
    REQUIRE(cmd.description() == "Paint Stroke (3 voxels)");
}

TEST_CASE("PaintStrokeCommand execute adds stroke", "[Editor][S45]") {
    VoxelPaintTool paintTool;
    PaintStroke stroke;
    stroke.positions.push_back({0, 0, 0});

    PaintStrokeCommand cmd(paintTool, stroke);
    cmd.execute();
    REQUIRE(paintTool.strokeCount() == 1);
}

// ── PCGParamChangeCommand ───────────────────────────────────────

TEST_CASE("PCGParamChangeCommand description is fixed", "[Editor][S45]") {
    PCGTuningPanel pcgPanel;
    NoiseParams oldP, newP;
    newP.frequency = 2.0f;

    PCGParamChangeCommand cmd(pcgPanel, oldP, newP);
    REQUIRE(cmd.description() == "Change PCG Parameters");
}

TEST_CASE("PCGParamChangeCommand execute applies new params", "[Editor][S45]") {
    PCGTuningPanel pcgPanel;
    NoiseParams oldP;
    NoiseParams newP;
    newP.frequency = 5.0f;

    PCGParamChangeCommand cmd(pcgPanel, oldP, newP);
    cmd.execute();
    REQUIRE(pcgPanel.noiseParams().frequency == 5.0f);
}

// ── EditorUndoSystem ────────────────────────────────────────────

TEST_CASE("EditorUndoSystem initial state has no undo/redo", "[Editor][S45]") {
    CommandStack stack;
    EditorUndoSystem sys(stack);

    REQUIRE_FALSE(sys.canUndo());
    REQUIRE_FALSE(sys.canRedo());
    REQUIRE(sys.undoCount() == 0);
    REQUIRE(sys.redoCount() == 0);
}

TEST_CASE("EditorUndoSystem executePlaceEntity enables undo", "[Editor][S45]") {
    CommandStack stack;
    EditorUndoSystem sys(stack);
    EntityPlacementTool tool;

    PlacedEntity entity;
    entity.templateName = "Lamp";
    entity.entityId = 1;

    sys.executePlaceEntity(tool, entity);
    REQUIRE(sys.canUndo());
    REQUIRE(sys.undoCount() == 1);
    REQUIRE(tool.placedCount() == 1);
}

TEST_CASE("EditorUndoSystem undo and redo round-trip", "[Editor][S45]") {
    CommandStack stack;
    EditorUndoSystem sys(stack);
    EntityPlacementTool tool;

    PlacedEntity entity;
    entity.templateName = "Chair";
    entity.entityId = 2;

    sys.executePlaceEntity(tool, entity);
    REQUIRE(tool.placedCount() == 1);

    sys.undo();
    REQUIRE(tool.placedCount() == 0);
    REQUIRE(sys.canRedo());

    sys.redo();
    REQUIRE(tool.placedCount() == 1);
}

TEST_CASE("EditorUndoSystem multiple commands stack correctly", "[Editor][S45]") {
    CommandStack stack;
    EditorUndoSystem sys(stack);
    EntityPlacementTool tool;

    PlacedEntity e1; e1.templateName = "A"; e1.entityId = 1;
    PlacedEntity e2; e2.templateName = "B"; e2.entityId = 2;
    PlacedEntity e3; e3.templateName = "C"; e3.entityId = 3;

    sys.executePlaceEntity(tool, e1);
    sys.executePlaceEntity(tool, e2);
    sys.executePlaceEntity(tool, e3);

    REQUIRE(sys.undoCount() == 3);
    REQUIRE(tool.placedCount() == 3);

    sys.undo();
    sys.undo();
    REQUIRE(sys.undoCount() == 1);
    REQUIRE(sys.redoCount() == 2);
    REQUIRE(tool.placedCount() == 1);
}

TEST_CASE("EditorUndoSystem executePaintStroke integrates with stack", "[Editor][S45]") {
    CommandStack stack;
    EditorUndoSystem sys(stack);
    VoxelPaintTool paintTool;

    PaintStroke stroke;
    stroke.positions.push_back({0, 0, 0});

    sys.executePaintStroke(paintTool, stroke);
    REQUIRE(sys.canUndo());
    REQUIRE(paintTool.strokeCount() == 1);

    sys.undo();
    REQUIRE(paintTool.strokeCount() == 0);
}

TEST_CASE("EditorUndoSystem executePCGChange integrates with stack", "[Editor][S45]") {
    CommandStack stack;
    EditorUndoSystem sys(stack);
    PCGTuningPanel panel;

    NoiseParams oldP = panel.noiseParams();
    NoiseParams newP = oldP;
    newP.frequency = 8.0f;

    sys.executePCGChange(panel, oldP, newP);
    REQUIRE(panel.noiseParams().frequency == 8.0f);

    sys.undo();
    REQUIRE(panel.noiseParams().frequency == oldP.frequency);
}
