#include "../editor/panels/MeshViewerPanel.h"
#include "../engine/procedural/ProceduralMeshNodes.h"
#include <iostream>
#include <cassert>

using namespace atlas::editor;
using namespace atlas::procedural;

void test_mesh_viewer_name() {
    MeshViewerPanel panel;
    assert(std::string(panel.Name()) == "Mesh Viewer");
    std::cout << "[PASS] test_mesh_viewer_name" << std::endl;
}

void test_mesh_viewer_no_mesh() {
    MeshViewerPanel panel;
    assert(!panel.HasMesh());
    assert(panel.Summary() == "No mesh loaded");
    assert(panel.SelectedVertex() == -1);
    std::cout << "[PASS] test_mesh_viewer_no_mesh" << std::endl;
}

void test_mesh_viewer_load_mesh() {
    MeshViewerPanel panel;
    MeshData mesh = GenerateCube(1.0f);
    panel.LoadMesh(mesh);

    assert(panel.HasMesh());
    assert(panel.Stats().vertexCount == mesh.VertexCount());
    assert(panel.Stats().triangleCount == mesh.TriangleCount());
    assert(panel.Stats().vertexCount > 0);
    assert(panel.Stats().triangleCount > 0);

    std::cout << "[PASS] test_mesh_viewer_load_mesh" << std::endl;
}

void test_mesh_viewer_bounds() {
    MeshViewerPanel panel;
    MeshData mesh = GenerateCube(2.0f);
    panel.LoadMesh(mesh);

    assert(panel.Stats().boundsMinX <= panel.Stats().boundsMaxX);
    assert(panel.Stats().boundsMinY <= panel.Stats().boundsMaxY);
    assert(panel.Stats().boundsMinZ <= panel.Stats().boundsMaxZ);

    std::cout << "[PASS] test_mesh_viewer_bounds" << std::endl;
}

void test_mesh_viewer_view_modes() {
    MeshViewerPanel panel;

    assert(panel.ViewMode() == MeshViewMode::Solid);

    panel.SetViewMode(MeshViewMode::Wireframe);
    assert(panel.ViewMode() == MeshViewMode::Wireframe);

    panel.SetViewMode(MeshViewMode::SolidWireframe);
    assert(panel.ViewMode() == MeshViewMode::SolidWireframe);

    panel.SetViewMode(MeshViewMode::Normals);
    assert(panel.ViewMode() == MeshViewMode::Normals);

    std::cout << "[PASS] test_mesh_viewer_view_modes" << std::endl;
}

void test_mesh_viewer_show_normals() {
    MeshViewerPanel panel;
    assert(!panel.ShowNormals());

    panel.SetShowNormals(true);
    assert(panel.ShowNormals());

    panel.SetShowNormals(false);
    assert(!panel.ShowNormals());

    std::cout << "[PASS] test_mesh_viewer_show_normals" << std::endl;
}

void test_mesh_viewer_show_grid() {
    MeshViewerPanel panel;
    assert(panel.ShowGrid());  // default on

    panel.SetShowGrid(false);
    assert(!panel.ShowGrid());

    std::cout << "[PASS] test_mesh_viewer_show_grid" << std::endl;
}

void test_mesh_viewer_select_vertex() {
    MeshViewerPanel panel;
    MeshData mesh = GenerateCube(1.0f);
    panel.LoadMesh(mesh);

    assert(panel.SelectedVertex() == -1);

    panel.SelectVertex(0);
    assert(panel.SelectedVertex() == 0);

    panel.SelectVertex(static_cast<int>(mesh.VertexCount()) - 1);
    assert(panel.SelectedVertex() == static_cast<int>(mesh.VertexCount()) - 1);

    // Out of range should deselect
    panel.SelectVertex(static_cast<int>(mesh.VertexCount()));
    assert(panel.SelectedVertex() == -1);

    panel.SelectVertex(-1);
    assert(panel.SelectedVertex() == -1);

    std::cout << "[PASS] test_mesh_viewer_select_vertex" << std::endl;
}

void test_mesh_viewer_summary() {
    MeshViewerPanel panel;
    MeshData mesh = GenerateCube(1.0f);
    panel.LoadMesh(mesh);

    std::string summary = panel.Summary();
    assert(summary.find("Vertices:") != std::string::npos);
    assert(summary.find("Triangles:") != std::string::npos);

    std::cout << "[PASS] test_mesh_viewer_summary" << std::endl;
}

void test_mesh_viewer_clear() {
    MeshViewerPanel panel;
    MeshData mesh = GenerateCube(1.0f);
    panel.LoadMesh(mesh);
    panel.SelectVertex(0);

    assert(panel.HasMesh());

    panel.Clear();
    assert(!panel.HasMesh());
    assert(panel.SelectedVertex() == -1);
    assert(panel.Summary() == "No mesh loaded");

    std::cout << "[PASS] test_mesh_viewer_clear" << std::endl;
}
