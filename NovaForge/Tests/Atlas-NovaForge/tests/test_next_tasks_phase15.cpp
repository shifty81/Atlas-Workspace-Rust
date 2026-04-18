#include "../editor/panels/MeshViewerPanel.h"
#include "../editor/panels/PrefabEditorPanel.h"
#include "../engine/procedural/ProceduralMeshNodes.h"
#include <iostream>
#include <cassert>

using namespace atlas::editor;
using namespace atlas::procedural;
using namespace atlas::ui;

// ---------------------------------------------------------------
// MeshViewerPanel Draw tests
// ---------------------------------------------------------------

void test_mesh_viewer_draw_no_mesh() {
    MeshViewerPanel panel;
    panel.Draw();
    assert(panel.GetDrawList().CommandCount() > 0);

    // Should contain "No mesh loaded" text
    bool found = false;
    for (const auto& cmd : panel.GetDrawList().Commands()) {
        if (cmd.kind == UIDrawCmd::Kind::Text &&
            cmd.text.find("No mesh loaded") != std::string::npos) {
            found = true;
            break;
        }
    }
    assert(found);
    std::cout << "[PASS] test_mesh_viewer_draw_no_mesh" << std::endl;
}

void test_mesh_viewer_draw_with_mesh() {
    MeshViewerPanel panel;
    MeshData mesh = GenerateCube(1.0f);
    panel.LoadMesh(mesh);
    panel.Draw();

    // Should have background + title + stats + bounds + viewport + border
    assert(panel.GetDrawList().CommandCount() >= 6);

    // Should contain vertex/triangle count
    bool foundStats = false;
    for (const auto& cmd : panel.GetDrawList().Commands()) {
        if (cmd.kind == UIDrawCmd::Kind::Text &&
            cmd.text.find("Vertices:") != std::string::npos) {
            foundStats = true;
            break;
        }
    }
    assert(foundStats);
    std::cout << "[PASS] test_mesh_viewer_draw_with_mesh" << std::endl;
}

void test_mesh_viewer_draw_shows_bounds() {
    MeshViewerPanel panel;
    MeshData mesh = GenerateCube(2.0f);
    panel.LoadMesh(mesh);
    panel.Draw();

    bool foundBounds = false;
    for (const auto& cmd : panel.GetDrawList().Commands()) {
        if (cmd.kind == UIDrawCmd::Kind::Text &&
            cmd.text.find("Bounds:") != std::string::npos) {
            foundBounds = true;
            break;
        }
    }
    assert(foundBounds);
    std::cout << "[PASS] test_mesh_viewer_draw_shows_bounds" << std::endl;
}

void test_mesh_viewer_draw_view_mode_label() {
    MeshViewerPanel panel;
    MeshData mesh = GenerateCube(1.0f);
    panel.LoadMesh(mesh);

    panel.SetViewMode(MeshViewMode::Wireframe);
    panel.Draw();

    bool foundWireframe = false;
    for (const auto& cmd : panel.GetDrawList().Commands()) {
        if (cmd.kind == UIDrawCmd::Kind::Text &&
            cmd.text.find("Wireframe") != std::string::npos) {
            foundWireframe = true;
            break;
        }
    }
    assert(foundWireframe);
    std::cout << "[PASS] test_mesh_viewer_draw_view_mode_label" << std::endl;
}

void test_mesh_viewer_draw_normals_indicator() {
    MeshViewerPanel panel;
    MeshData mesh = GenerateCube(1.0f);
    panel.LoadMesh(mesh);

    panel.SetShowNormals(true);
    panel.Draw();

    bool foundNormals = false;
    for (const auto& cmd : panel.GetDrawList().Commands()) {
        if (cmd.kind == UIDrawCmd::Kind::Text &&
            cmd.text.find("Normals ON") != std::string::npos) {
            foundNormals = true;
            break;
        }
    }
    assert(foundNormals);
    std::cout << "[PASS] test_mesh_viewer_draw_normals_indicator" << std::endl;
}

void test_mesh_viewer_draw_selected_vertex() {
    MeshViewerPanel panel;
    MeshData mesh = GenerateCube(1.0f);
    panel.LoadMesh(mesh);
    panel.SelectVertex(0);
    panel.Draw();

    bool foundSelected = false;
    for (const auto& cmd : panel.GetDrawList().Commands()) {
        if (cmd.kind == UIDrawCmd::Kind::Text &&
            cmd.text.find("Selected vertex: 0") != std::string::npos) {
            foundSelected = true;
            break;
        }
    }
    assert(foundSelected);
    std::cout << "[PASS] test_mesh_viewer_draw_selected_vertex" << std::endl;
}

void test_mesh_viewer_draw_clears_between_frames() {
    MeshViewerPanel panel;
    MeshData mesh = GenerateCube(1.0f);
    panel.LoadMesh(mesh);

    panel.Draw();
    size_t firstCount = panel.GetDrawList().CommandCount();
    assert(firstCount > 0);

    panel.Draw();
    size_t secondCount = panel.GetDrawList().CommandCount();
    assert(secondCount == firstCount);
    std::cout << "[PASS] test_mesh_viewer_draw_clears_between_frames" << std::endl;
}

void test_mesh_viewer_draw_grid_commands() {
    MeshViewerPanel panel;
    MeshData mesh = GenerateCube(1.0f);
    panel.LoadMesh(mesh);

    panel.SetShowGrid(true);
    panel.Draw();
    size_t withGrid = panel.GetDrawList().CommandCount();

    panel.SetShowGrid(false);
    panel.Draw();
    size_t withoutGrid = panel.GetDrawList().CommandCount();

    // Grid adds 2 draw commands for crosshair lines (horizontal + vertical)
    assert(withGrid > withoutGrid);
    std::cout << "[PASS] test_mesh_viewer_draw_grid_commands" << std::endl;
}

// ---------------------------------------------------------------
// PrefabEditorPanel Draw tests
// ---------------------------------------------------------------

void test_prefab_editor_draw_empty() {
    PrefabEditorPanel panel;
    panel.Draw();
    assert(panel.GetDrawList().CommandCount() > 0);

    // Should contain "Empty prefab" text
    bool found = false;
    for (const auto& cmd : panel.GetDrawList().Commands()) {
        if (cmd.kind == UIDrawCmd::Kind::Text &&
            cmd.text.find("Empty prefab") != std::string::npos) {
            found = true;
            break;
        }
    }
    assert(found);
    std::cout << "[PASS] test_prefab_editor_draw_empty" << std::endl;
}

void test_prefab_editor_draw_with_entities() {
    PrefabEditorPanel panel;
    panel.AddEntity("Ship");
    panel.AddEntity("Turret");
    panel.Draw();

    // Should have background + title + hierarchy header + 2 entity rows
    assert(panel.GetDrawList().CommandCount() >= 5);

    // Should contain entity names
    bool foundShip = false, foundTurret = false;
    for (const auto& cmd : panel.GetDrawList().Commands()) {
        if (cmd.kind == UIDrawCmd::Kind::Text) {
            if (cmd.text.find("Ship") != std::string::npos) foundShip = true;
            if (cmd.text.find("Turret") != std::string::npos) foundTurret = true;
        }
    }
    assert(foundShip);
    assert(foundTurret);
    std::cout << "[PASS] test_prefab_editor_draw_with_entities" << std::endl;
}

void test_prefab_editor_draw_dirty_indicator() {
    PrefabEditorPanel panel;
    panel.AddEntity("Player");
    panel.Draw();

    bool foundDirty = false;
    for (const auto& cmd : panel.GetDrawList().Commands()) {
        if (cmd.kind == UIDrawCmd::Kind::Text &&
            cmd.text.find("*") != std::string::npos &&
            cmd.text.find("Prefab Editor") != std::string::npos) {
            foundDirty = true;
            break;
        }
    }
    assert(foundDirty);

    // Clean and check no dirty indicator
    panel.MarkClean();
    panel.Draw();
    bool foundClean = false;
    for (const auto& cmd : panel.GetDrawList().Commands()) {
        if (cmd.kind == UIDrawCmd::Kind::Text &&
            cmd.text == "Prefab Editor") {
            foundClean = true;
            break;
        }
    }
    assert(foundClean);
    std::cout << "[PASS] test_prefab_editor_draw_dirty_indicator" << std::endl;
}

void test_prefab_editor_draw_hierarchy() {
    PrefabEditorPanel panel;
    uint32_t parent = panel.AddEntity("Ship");
    uint32_t child = panel.AddEntity("Turret");
    panel.SetParent(child, parent);
    panel.Draw();

    // Should contain parent with > prefix (has children) and child
    bool foundParent = false, foundChild = false;
    for (const auto& cmd : panel.GetDrawList().Commands()) {
        if (cmd.kind == UIDrawCmd::Kind::Text) {
            if (cmd.text.find("> Ship") != std::string::npos) foundParent = true;
            if (cmd.text.find("Turret") != std::string::npos) foundChild = true;
        }
    }
    assert(foundParent);
    assert(foundChild);
    std::cout << "[PASS] test_prefab_editor_draw_hierarchy" << std::endl;
}

void test_prefab_editor_draw_selected_entity_components() {
    PrefabEditorPanel panel;
    uint32_t id = panel.AddEntity("Player");

    PrefabComponent comp;
    comp.type = "Transform";
    comp.properties = {{"x", "10"}, {"y", "20"}};
    panel.AddComponent(id, comp);

    panel.SelectEntity(id);
    panel.Draw();

    // Should show component section with "Transform" and properties
    bool foundComp = false, foundProp = false;
    for (const auto& cmd : panel.GetDrawList().Commands()) {
        if (cmd.kind == UIDrawCmd::Kind::Text) {
            if (cmd.text == "Transform") foundComp = true;
            if (cmd.text.find("x: 10") != std::string::npos) foundProp = true;
        }
    }
    assert(foundComp);
    assert(foundProp);
    std::cout << "[PASS] test_prefab_editor_draw_selected_entity_components" << std::endl;
}

void test_prefab_editor_draw_no_components_message() {
    PrefabEditorPanel panel;
    uint32_t id = panel.AddEntity("Empty");
    panel.SelectEntity(id);
    panel.Draw();

    bool found = false;
    for (const auto& cmd : panel.GetDrawList().Commands()) {
        if (cmd.kind == UIDrawCmd::Kind::Text &&
            cmd.text.find("no components") != std::string::npos) {
            found = true;
            break;
        }
    }
    assert(found);
    std::cout << "[PASS] test_prefab_editor_draw_no_components_message" << std::endl;
}

void test_prefab_editor_draw_clears_between_frames() {
    PrefabEditorPanel panel;
    panel.AddEntity("Ship");

    panel.Draw();
    size_t firstCount = panel.GetDrawList().CommandCount();
    assert(firstCount > 0);

    panel.Draw();
    size_t secondCount = panel.GetDrawList().CommandCount();
    assert(secondCount == firstCount);
    std::cout << "[PASS] test_prefab_editor_draw_clears_between_frames" << std::endl;
}

// ---------------------------------------------------------------
// Runner
// ---------------------------------------------------------------

void register_next_tasks_phase15() {
    std::cout << "\n--- Phase 15: MeshViewerPanel Draw ---" << std::endl;
    test_mesh_viewer_draw_no_mesh();
    test_mesh_viewer_draw_with_mesh();
    test_mesh_viewer_draw_shows_bounds();
    test_mesh_viewer_draw_view_mode_label();
    test_mesh_viewer_draw_normals_indicator();
    test_mesh_viewer_draw_selected_vertex();
    test_mesh_viewer_draw_clears_between_frames();
    test_mesh_viewer_draw_grid_commands();

    std::cout << "\n--- Phase 15: PrefabEditorPanel Draw ---" << std::endl;
    test_prefab_editor_draw_empty();
    test_prefab_editor_draw_with_entities();
    test_prefab_editor_draw_dirty_indicator();
    test_prefab_editor_draw_hierarchy();
    test_prefab_editor_draw_selected_entity_components();
    test_prefab_editor_draw_no_components_message();
    test_prefab_editor_draw_clears_between_frames();
}
