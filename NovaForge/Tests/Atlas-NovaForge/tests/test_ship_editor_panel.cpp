#include "../editor/panels/ShipEditorPanel.h"
#include <iostream>
#include <cassert>

using namespace atlas::editor;
using namespace atlas::procedural;

void test_ship_editor_name() {
    ShipEditorPanel panel;
    assert(std::string(panel.Name()) == "Ship Editor");
    std::cout << "[PASS] test_ship_editor_name" << std::endl;
}

void test_ship_editor_no_ship() {
    ShipEditorPanel panel;
    assert(!panel.HasShip());
    assert(panel.Summary() == "No ship loaded");
    assert(panel.SelectionType() == ShipEditorSelection::None);
    assert(panel.SelectionId() == 0);
    std::cout << "[PASS] test_ship_editor_no_ship" << std::endl;
}

void test_ship_editor_new_ship() {
    ShipEditorPanel panel;
    panel.NewShip("Frigate", 42);

    assert(panel.HasShip());
    assert(panel.ShipClass() == "Frigate");
    assert(panel.ControlPointCount() > 0);
    assert(panel.TurretHardpointCount() > 0);
    assert(panel.InteriorCount() > 0);
    assert(panel.LightCount() > 0);
    assert(panel.VisualFeatureCount() > 0);
    assert(panel.IsDirty());
    assert(panel.HasCockpit());

    std::cout << "[PASS] test_ship_editor_new_ship" << std::endl;
}

void test_ship_editor_new_ship_classes() {
    ShipEditorPanel panel;

    panel.NewShip("Destroyer", 100);
    assert(panel.ControlPointCount() > 0);
    size_t destroyerPts = panel.ControlPointCount();

    panel.NewShip("Capital", 100);
    assert(panel.ControlPointCount() > destroyerPts);

    std::cout << "[PASS] test_ship_editor_new_ship_classes" << std::endl;
}

void test_ship_editor_compile_and_generate() {
    ShipEditorPanel panel;
    panel.NewShip("Frigate", 42);

    bool ok = panel.CompileAndGenerate();
    assert(ok);
    assert(panel.IsCompiled());

    auto* mesh = panel.GetGeneratedMesh();
    assert(mesh != nullptr);
    assert(mesh->IsValid());
    assert(mesh->VertexCount() > 0);
    assert(mesh->TriangleCount() > 0);

    std::cout << "[PASS] test_ship_editor_compile_and_generate" << std::endl;
}

void test_ship_editor_selection() {
    ShipEditorPanel panel;
    panel.NewShip("Frigate", 42);

    auto& cps = panel.Graph().ControlPoints();
    assert(!cps.empty());
    uint32_t cpId = cps[0].id;

    panel.Select(ShipEditorSelection::ControlPoint, cpId);
    assert(panel.SelectionType() == ShipEditorSelection::ControlPoint);
    assert(panel.SelectionId() == cpId);

    panel.ClearSelection();
    assert(panel.SelectionType() == ShipEditorSelection::None);
    assert(panel.SelectionId() == 0);

    std::cout << "[PASS] test_ship_editor_selection" << std::endl;
}

void test_ship_editor_tools() {
    ShipEditorPanel panel;
    assert(panel.CurrentTool() == ShipEditorTool::Select);

    panel.SetTool(ShipEditorTool::MovePoint);
    assert(panel.CurrentTool() == ShipEditorTool::MovePoint);

    panel.SetTool(ShipEditorTool::AddTurretHardpoint);
    assert(panel.CurrentTool() == ShipEditorTool::AddTurretHardpoint);

    std::cout << "[PASS] test_ship_editor_tools" << std::endl;
}

void test_ship_editor_view_modes() {
    ShipEditorPanel panel;
    assert(panel.ViewMode() == ShipViewMode::Exterior);

    panel.SetViewMode(ShipViewMode::Interior);
    assert(panel.ViewMode() == ShipViewMode::Interior);

    panel.SetViewMode(ShipViewMode::Cockpit);
    assert(panel.ViewMode() == ShipViewMode::Cockpit);

    panel.SetViewMode(ShipViewMode::Wireframe);
    assert(panel.ViewMode() == ShipViewMode::Wireframe);

    panel.SetViewMode(ShipViewMode::Systems);
    assert(panel.ViewMode() == ShipViewMode::Systems);

    std::cout << "[PASS] test_ship_editor_view_modes" << std::endl;
}

void test_ship_editor_move_control_point() {
    ShipEditorPanel panel;
    panel.NewShip("Frigate", 42);

    auto& cps = panel.Graph().ControlPoints();
    uint32_t cpId = cps[0].id;

    panel.MoveControlPoint(cpId, 10.0f, 0.0f, 0.0f);

    auto* cp = panel.Graph().GetControlPoint(cpId);
    assert(cp != nullptr);
    assert(std::abs(cp->position.x - 10.0f) < 0.01f);
    assert(panel.IsDirty());

    // Should have recorded edit
    assert(!panel.EditHistory().empty());
    assert(panel.EditHistory().back().editType == "move_control_point");

    std::cout << "[PASS] test_ship_editor_move_control_point" << std::endl;
}

void test_ship_editor_add_control_point() {
    ShipEditorPanel panel;
    panel.NewShip("Frigate", 42);

    size_t before = panel.ControlPointCount();
    uint32_t id = panel.AddControlPoint(5.0f, 0.0f, 0.0f, 1.0f);
    assert(panel.ControlPointCount() == before + 1);
    assert(id > 0);

    std::cout << "[PASS] test_ship_editor_add_control_point" << std::endl;
}

void test_ship_editor_remove_control_point() {
    ShipEditorPanel panel;
    panel.NewShip("Frigate", 42);

    auto& cps = panel.Graph().ControlPoints();
    uint32_t cpId = cps[0].id;
    size_t before = panel.ControlPointCount();

    panel.Select(ShipEditorSelection::ControlPoint, cpId);
    panel.RemoveControlPoint(cpId);

    assert(panel.ControlPointCount() == before - 1);
    // Selection should be cleared
    assert(panel.SelectionType() == ShipEditorSelection::None);

    std::cout << "[PASS] test_ship_editor_remove_control_point" << std::endl;
}

void test_ship_editor_add_turret() {
    ShipEditorPanel panel;
    panel.NewShip("Frigate", 42);

    size_t before = panel.TurretHardpointCount();
    uint32_t id = panel.AddTurretHardpoint(0.0f, 2.0f, 0.0f, "medium", "turret");
    assert(panel.TurretHardpointCount() == before + 1);
    assert(id > 0);

    std::cout << "[PASS] test_ship_editor_add_turret" << std::endl;
}

void test_ship_editor_remove_turret() {
    ShipEditorPanel panel;
    panel.NewShip("Frigate", 42);

    uint32_t id = panel.AddTurretHardpoint(0.0f, 2.0f, 0.0f, "small", "turret");
    size_t after = panel.TurretHardpointCount();

    panel.Select(ShipEditorSelection::TurretHardpoint, id);
    panel.RemoveTurretHardpoint(id);

    assert(panel.TurretHardpointCount() == after - 1);
    assert(panel.SelectionType() == ShipEditorSelection::None);

    std::cout << "[PASS] test_ship_editor_remove_turret" << std::endl;
}

void test_ship_editor_add_light() {
    ShipEditorPanel panel;
    panel.NewShip("Frigate", 42);

    size_t before = panel.LightCount();
    uint32_t id = panel.AddLight(0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.8f, 5.0f, true);
    assert(panel.LightCount() == before + 1);
    assert(id > 0);

    std::cout << "[PASS] test_ship_editor_add_light" << std::endl;
}

void test_ship_editor_remove_light() {
    ShipEditorPanel panel;
    panel.NewShip("Frigate", 42);

    uint32_t id = panel.AddLight(0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 10.0f, false);
    size_t after = panel.LightCount();

    panel.Select(ShipEditorSelection::Light, id);
    panel.RemoveLight(id);

    assert(panel.LightCount() == after - 1);
    assert(panel.SelectionType() == ShipEditorSelection::None);

    std::cout << "[PASS] test_ship_editor_remove_light" << std::endl;
}

void test_ship_editor_add_interior() {
    ShipEditorPanel panel;
    panel.NewShip("Frigate", 42);

    size_t before = panel.InteriorCount();
    uint32_t id = panel.AddInterior("cargo_bay", -1, -1, -2, 1, 0.5, 2, false);
    assert(panel.InteriorCount() == before + 1);
    assert(id > 0);

    std::cout << "[PASS] test_ship_editor_add_interior" << std::endl;
}

void test_ship_editor_remove_interior() {
    ShipEditorPanel panel;
    panel.NewShip("Frigate", 42);

    uint32_t id = panel.AddInterior("test_room", -1, -1, -1, 1, 1, 1, false);
    size_t after = panel.InteriorCount();

    panel.Select(ShipEditorSelection::Interior, id);
    panel.RemoveInterior(id);

    assert(panel.InteriorCount() == after - 1);
    assert(panel.SelectionType() == ShipEditorSelection::None);

    std::cout << "[PASS] test_ship_editor_remove_interior" << std::endl;
}

void test_ship_editor_add_visual_feature() {
    ShipEditorPanel panel;
    panel.NewShip("Frigate", 42);

    size_t before = panel.VisualFeatureCount();
    uint32_t id = panel.AddVisualFeature("antenna", 0, 2, 0, 0.1f, 1.0f, 0.1f);
    assert(panel.VisualFeatureCount() == before + 1);
    assert(id > 0);

    std::cout << "[PASS] test_ship_editor_add_visual_feature" << std::endl;
}

void test_ship_editor_remove_visual_feature() {
    ShipEditorPanel panel;
    panel.NewShip("Frigate", 42);

    uint32_t id = panel.AddVisualFeature("vent", 0, 0, 1, 0.2f, 0.1f, 0.3f);
    size_t after = panel.VisualFeatureCount();

    panel.Select(ShipEditorSelection::VisualFeature, id);
    panel.RemoveVisualFeature(id);

    assert(panel.VisualFeatureCount() == after - 1);
    assert(panel.SelectionType() == ShipEditorSelection::None);

    std::cout << "[PASS] test_ship_editor_remove_visual_feature" << std::endl;
}

void test_ship_editor_has_cockpit() {
    ShipEditorPanel panel;
    panel.NewShip("Frigate", 42);
    assert(panel.HasCockpit());

    std::cout << "[PASS] test_ship_editor_has_cockpit" << std::endl;
}

void test_ship_editor_export_import() {
    ShipEditorPanel panel;
    panel.NewShip("Cruiser", 99);
    panel.SetShipId("test_cruiser");
    panel.SetShipName("Test Cruiser");

    auto exported = panel.ExportShip();
    assert(exported.shipId == "test_cruiser");
    assert(exported.shipName == "Test Cruiser");
    assert(exported.shipClass == "Cruiser");
    assert(!exported.controlPoints.empty());
    assert(!exported.turretHardpoints.empty());

    // Import into new panel
    ShipEditorPanel panel2;
    panel2.LoadShipData(exported);

    assert(panel2.HasShip());
    assert(panel2.ShipId() == "test_cruiser");
    assert(panel2.ShipName() == "Test Cruiser");
    assert(panel2.ShipClass() == "Cruiser");
    assert(panel2.ControlPointCount() == panel.ControlPointCount());
    assert(!panel2.IsDirty()); // loaded, not modified

    std::cout << "[PASS] test_ship_editor_export_import" << std::endl;
}

void test_ship_editor_pcg_learning() {
    ShipEditorPanel panel;
    panel.NewShip("Frigate", 42);

    // Move a control point to record an edit
    auto& cps = panel.Graph().ControlPoints();
    panel.MoveControlPoint(cps[0].id, 10.0f, 0.0f, 0.0f);

    assert(panel.EditHistory().size() >= 1);
    assert(panel.EditHistory().back().shipClass == "Frigate");

    // Apply a learning profile
    PCGLearningProfile profile;
    profile.shipClass = "Frigate";
    profile.avgHullWidth = 3.0f;
    profile.sampleCount = 5;
    panel.ApplyLearningProfile(profile);

    assert(panel.IsDirty());

    std::cout << "[PASS] test_ship_editor_pcg_learning" << std::endl;
}

void test_ship_editor_summary() {
    ShipEditorPanel panel;
    panel.NewShip("Frigate", 42);
    panel.SetShipName("Fang");

    std::string summary = panel.Summary();
    assert(summary.find("Frigate") != std::string::npos);
    assert(summary.find("Fang") != std::string::npos);
    assert(summary.find("hull pts") != std::string::npos);
    assert(summary.find("turrets") != std::string::npos);
    assert(summary.find("lights") != std::string::npos);
    assert(summary.find("rooms") != std::string::npos);

    std::cout << "[PASS] test_ship_editor_summary" << std::endl;
}

void test_ship_editor_draw() {
    ShipEditorPanel panel;
    panel.NewShip("Frigate", 42);
    panel.CompileAndGenerate();

    // Draw should not crash
    panel.Draw();
    assert(panel.GetDrawList().Commands().size() > 0);

    std::cout << "[PASS] test_ship_editor_draw" << std::endl;
}

void test_ship_editor_draw_no_ship() {
    ShipEditorPanel panel;
    panel.Draw();
    assert(panel.GetDrawList().Commands().size() > 0);

    std::cout << "[PASS] test_ship_editor_draw_no_ship" << std::endl;
}

void test_ship_editor_set_ship_info() {
    ShipEditorPanel panel;
    panel.NewShip("Frigate", 42);

    panel.SetShipId("fang");
    panel.SetShipName("Fang");

    assert(panel.ShipId() == "fang");
    assert(panel.ShipName() == "Fang");

    std::cout << "[PASS] test_ship_editor_set_ship_info" << std::endl;
}

void test_ship_editor_mark_clean() {
    ShipEditorPanel panel;
    panel.NewShip("Frigate", 42);
    assert(panel.IsDirty());

    panel.MarkClean();
    assert(!panel.IsDirty());

    std::cout << "[PASS] test_ship_editor_mark_clean" << std::endl;
}
