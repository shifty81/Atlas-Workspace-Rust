#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"

using namespace NF;

// ── GraphEditorPanel ─────────────────────────────────────────────

TEST_CASE("GraphEditorPanel name is GraphEditor", "[Editor][S62]") {
    GraphEditorPanel panel;
    REQUIRE(panel.name() == "GraphEditor");
}

TEST_CASE("GraphEditorPanel slot is Center", "[Editor][S62]") {
    GraphEditorPanel panel;
    REQUIRE(panel.slot() == DockSlot::Center);
}

TEST_CASE("GraphEditorPanel starts with no open graph", "[Editor][S62]") {
    GraphEditorPanel panel;
    REQUIRE_FALSE(panel.hasOpenGraph());
    REQUIRE(panel.currentGraph() == nullptr);
    REQUIRE(panel.nodeCount() == 0);
    REQUIRE(panel.linkCount() == 0);
}

TEST_CASE("GraphEditorPanel newGraph creates a graph", "[Editor][S62]") {
    GraphEditorPanel panel;
    panel.newGraph(GraphType::World, "MyGraph");
    REQUIRE(panel.hasOpenGraph());
    REQUIRE(panel.currentGraphName() == "MyGraph");
}

TEST_CASE("GraphEditorPanel addNode increases node count", "[Editor][S62]") {
    GraphEditorPanel panel;
    panel.newGraph(GraphType::World, "Test");
    int id = panel.addNode("Start", GraphType::World, 0.f, 0.f);
    REQUIRE(id >= 0);
    REQUIRE(panel.nodeCount() == 1);
}

TEST_CASE("GraphEditorPanel addNode without open graph returns -1", "[Editor][S62]") {
    GraphEditorPanel panel;
    int id = panel.addNode("Node");
    REQUIRE(id == -1);
}

TEST_CASE("GraphEditorPanel removeNode decreases count", "[Editor][S62]") {
    GraphEditorPanel panel;
    panel.newGraph(GraphType::World, "Test");
    int id = panel.addNode("A");
    panel.addNode("B");
    REQUIRE(panel.nodeCount() == 2);
    bool ok = panel.removeNode(id);
    REQUIRE(ok);
    REQUIRE(panel.nodeCount() == 1);
}

TEST_CASE("GraphEditorPanel removeNode invalid id returns false", "[Editor][S62]") {
    GraphEditorPanel panel;
    panel.newGraph(GraphType::World, "Test");
    REQUIRE_FALSE(panel.removeNode(999));
}

TEST_CASE("GraphEditorPanel addLink increases link count", "[Editor][S62]") {
    GraphEditorPanel panel;
    panel.newGraph(GraphType::World, "Test");
    int a = panel.addNode("A");
    int b = panel.addNode("B");
    bool ok = panel.addLink(a, 0, b, 0);
    REQUIRE(ok);
    REQUIRE(panel.linkCount() == 1);
}

TEST_CASE("GraphEditorPanel selectNode sets selection", "[Editor][S62]") {
    GraphEditorPanel panel;
    panel.newGraph(GraphType::World, "Test");
    int id = panel.addNode("Node");
    panel.selectNode(id);
    REQUIRE(panel.selectedNodeId() == id);
}

TEST_CASE("GraphEditorPanel clearSelection resets to -1", "[Editor][S62]") {
    GraphEditorPanel panel;
    panel.newGraph(GraphType::World, "Test");
    int id = panel.addNode("Node");
    panel.selectNode(id);
    panel.clearSelection();
    REQUIRE(panel.selectedNodeId() == -1);
}

TEST_CASE("GraphEditorPanel removeNode clears selection if selected", "[Editor][S62]") {
    GraphEditorPanel panel;
    panel.newGraph(GraphType::World, "Test");
    int id = panel.addNode("Node");
    panel.selectNode(id);
    panel.removeNode(id);
    REQUIRE(panel.selectedNodeId() == -1);
}

TEST_CASE("GraphEditorPanel is an EditorPanel", "[Editor][S62]") {
    GraphEditorPanel panel;
    EditorPanel* base = &panel;
    REQUIRE(base->name() == "GraphEditor");
}

// ── MeshImportSettings ───────────────────────────────────────────

TEST_CASE("MeshImportSettings defaults", "[Editor][S62]") {
    MeshImportSettings s;
    REQUIRE(s.scaleFactor     == 1.0f);
    REQUIRE(s.generateNormals);
    REQUIRE_FALSE(s.generateTangents);
    REQUIRE_FALSE(s.flipWindingOrder);
    REQUIRE_FALSE(s.mergeMeshes);
    REQUIRE(s.maxVertices     == 0);
}

// ── MeshImporter ─────────────────────────────────────────────────

TEST_CASE("MeshImporter starts with zero imports", "[Editor][S62]") {
    MeshImporter mi;
    REQUIRE(mi.importCount() == 0);
}

TEST_CASE("MeshImporter canImport recognises supported formats", "[Editor][S62]") {
    MeshImporter mi;
    REQUIRE(mi.canImport("model.obj"));
    REQUIRE(mi.canImport("character.fbx"));
    REQUIRE(mi.canImport("scene.gltf"));
    REQUIRE(mi.canImport("prop.glb"));
}

TEST_CASE("MeshImporter canImport rejects unsupported formats", "[Editor][S62]") {
    MeshImporter mi;
    REQUIRE_FALSE(mi.canImport("image.png"));
    REQUIRE_FALSE(mi.canImport("data.json"));
    REQUIRE_FALSE(mi.canImport("noextension"));
}

TEST_CASE("MeshImporter import returns valid guid for supported file", "[Editor][S62]") {
    MeshImporter mi;
    AssetDatabase db;
    AssetGuid guid = mi.import(db, "mesh/cube.obj");
    REQUIRE_FALSE(guid.isNull());
    REQUIRE(mi.importCount() == 1);
}

TEST_CASE("MeshImporter import returns null guid for unsupported file", "[Editor][S62]") {
    MeshImporter mi;
    AssetDatabase db;
    AssetGuid guid = mi.import(db, "image.png");
    REQUIRE(guid.isNull());
    REQUIRE(mi.importCount() == 0);
}

TEST_CASE("MeshImporter setSettings stores settings", "[Editor][S62]") {
    MeshImporter mi;
    MeshImportSettings s;
    s.scaleFactor = 0.01f;
    s.generateTangents = true;
    mi.setSettings(s);
    REQUIRE(mi.settings().scaleFactor == 0.01f);
    REQUIRE(mi.settings().generateTangents);
}

// ── TextureImportSettings ────────────────────────────────────────

TEST_CASE("TextureImportSettings defaults", "[Editor][S62]") {
    TextureImportSettings s;
    REQUIRE(s.generateMipmaps);
    REQUIRE(s.sRGB);
    REQUIRE(s.maxResolution == 0);
    REQUIRE_FALSE(s.premultiplyAlpha);
    REQUIRE(s.flipVertically);
}

// ── TextureImporter ───────────────────────────────────────────────

TEST_CASE("TextureImporter canImport recognises supported formats", "[Editor][S62]") {
    TextureImporter ti;
    REQUIRE(ti.canImport("icon.png"));
    REQUIRE(ti.canImport("photo.jpg"));
    REQUIRE(ti.canImport("skin.tga"));
    REQUIRE(ti.canImport("sprite.bmp"));
}

TEST_CASE("TextureImporter canImport rejects mesh files", "[Editor][S62]") {
    TextureImporter ti;
    REQUIRE_FALSE(ti.canImport("model.obj"));
    REQUIRE_FALSE(ti.canImport("scene.gltf"));
}

TEST_CASE("TextureImporter import increments count", "[Editor][S62]") {
    TextureImporter ti;
    AssetDatabase db;
    AssetGuid guid = ti.import(db, "ui/button.png");
    REQUIRE_FALSE(guid.isNull());
    REQUIRE(ti.importCount() == 1);
}
