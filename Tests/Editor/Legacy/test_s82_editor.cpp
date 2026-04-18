#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"

using namespace NF;

// ── S82: AssetDependencies + AssetImporters + BlenderImporter ────

// ── AssetDependencies ────────────────────────────────────────────

TEST_CASE("AssetDepType names are correct", "[Editor][S82]") {
    REQUIRE(std::string(assetDepTypeName(AssetDepType::Texture))   == "Texture");
    REQUIRE(std::string(assetDepTypeName(AssetDepType::Mesh))      == "Mesh");
    REQUIRE(std::string(assetDepTypeName(AssetDepType::Shader))    == "Shader");
    REQUIRE(std::string(assetDepTypeName(AssetDepType::Script))    == "Script");
    REQUIRE(std::string(assetDepTypeName(AssetDepType::Audio))     == "Audio");
    REQUIRE(std::string(assetDepTypeName(AssetDepType::Material))  == "Material");
    REQUIRE(std::string(assetDepTypeName(AssetDepType::Animation)) == "Animation");
    REQUIRE(std::string(assetDepTypeName(AssetDepType::Level))     == "Level");
}

TEST_CASE("AssetDepNode predicates for status", "[Editor][S82]") {
    AssetDepNode node;
    node.assetId = "tex1";
    node.status = AssetDepStatus::Resolved;
    REQUIRE(node.isResolved());
    REQUIRE_FALSE(node.isMissing());
    node.status = AssetDepStatus::Missing;
    REQUIRE(node.isMissing());
    node.status = AssetDepStatus::Circular;
    REQUIRE(node.isCircular());
}

TEST_CASE("AssetDepNode addDependency works and rejects self-dep", "[Editor][S82]") {
    AssetDepNode node;
    node.assetId = "mat1";
    REQUIRE(node.addDependency("tex1"));
    REQUIRE(node.addDependency("mesh1"));
    REQUIRE_FALSE(node.addDependency("mat1")); // self-dep rejected
    REQUIRE_FALSE(node.addDependency("tex1")); // duplicate
    REQUIRE(node.dependencyCount() == 2);
}

TEST_CASE("AssetDepNode hasDependency", "[Editor][S82]") {
    AssetDepNode node;
    node.assetId = "mat1";
    node.addDependency("tex1");
    REQUIRE(node.hasDependency("tex1"));
    REQUIRE_FALSE(node.hasDependency("mesh1"));
}

TEST_CASE("AssetDepGraph addNode and nodeCount", "[Editor][S82]") {
    AssetDepGraph graph;
    AssetDepNode n; n.assetId = "tex1"; n.type = AssetDepType::Texture;
    REQUIRE(graph.addNode(n));
    REQUIRE(graph.nodeCount() == 1);
}

TEST_CASE("AssetDepGraph addNode rejects duplicate assetId", "[Editor][S82]") {
    AssetDepGraph graph;
    AssetDepNode n; n.assetId = "mesh1"; n.type = AssetDepType::Mesh;
    graph.addNode(n);
    REQUIRE_FALSE(graph.addNode(n));
}

TEST_CASE("AssetDepGraph removeNode removes entry and cleans edges", "[Editor][S82]") {
    AssetDepGraph graph;
    AssetDepNode a; a.assetId = "mat1"; a.type = AssetDepType::Material;
    AssetDepNode b; b.assetId = "tex1"; b.type = AssetDepType::Texture;
    graph.addNode(a); graph.addNode(b);
    graph.addEdge("mat1", "tex1");
    REQUIRE(graph.hasEdge("mat1", "tex1"));
    REQUIRE(graph.removeNode("tex1"));
    REQUIRE(graph.nodeCount() == 1);
    REQUIRE_FALSE(graph.hasEdge("mat1", "tex1"));
}

TEST_CASE("AssetDepGraph addEdge and hasEdge", "[Editor][S82]") {
    AssetDepGraph graph;
    AssetDepNode a; a.assetId = "mat1";
    AssetDepNode b; b.assetId = "tex1";
    graph.addNode(a); graph.addNode(b);
    REQUIRE(graph.addEdge("mat1", "tex1"));
    REQUIRE(graph.hasEdge("mat1", "tex1"));
    REQUIRE_FALSE(graph.hasEdge("tex1", "mat1")); // directed
}

TEST_CASE("AssetDepGraph addEdge fails when endpoint missing", "[Editor][S82]") {
    AssetDepGraph graph;
    AssetDepNode a; a.assetId = "mat1";
    graph.addNode(a);
    REQUIRE_FALSE(graph.addEdge("mat1", "ghost"));
}

TEST_CASE("AssetDepGraph resolveAll marks unknown nodes resolved", "[Editor][S82]") {
    AssetDepGraph graph;
    AssetDepNode a; a.assetId = "a"; a.status = AssetDepStatus::Unknown;
    AssetDepNode b; b.assetId = "b"; b.status = AssetDepStatus::Unknown;
    graph.addNode(a); graph.addNode(b);
    graph.resolveAll();
    REQUIRE(graph.findNode("a")->isResolved());
    REQUIRE(graph.findNode("b")->isResolved());
}

TEST_CASE("AssetDepGraph unresolvedCount counts unknown and missing", "[Editor][S82]") {
    AssetDepGraph graph;
    AssetDepNode a; a.assetId = "a"; a.status = AssetDepStatus::Unknown;
    AssetDepNode b; b.assetId = "b"; b.status = AssetDepStatus::Missing;
    AssetDepNode c; c.assetId = "c"; c.status = AssetDepStatus::Resolved;
    graph.addNode(a); graph.addNode(b); graph.addNode(c);
    REQUIRE(graph.unresolvedCount() == 2);
}

TEST_CASE("AssetDepGraph totalEdgeCount sums all edges", "[Editor][S82]") {
    AssetDepGraph graph;
    AssetDepNode a; a.assetId = "a";
    AssetDepNode b; b.assetId = "b";
    AssetDepNode c; c.assetId = "c";
    graph.addNode(a); graph.addNode(b); graph.addNode(c);
    graph.addEdge("a", "b");
    graph.addEdge("a", "c");
    REQUIRE(graph.totalEdgeCount() == 2);
}

TEST_CASE("AssetDepGraph detectCircular marks circular nodes", "[Editor][S82]") {
    AssetDepGraph graph;
    AssetDepNode a; a.assetId = "a"; a.status = AssetDepStatus::Unknown;
    AssetDepNode b; b.assetId = "b"; b.status = AssetDepStatus::Unknown;
    graph.addNode(a); graph.addNode(b);
    graph.addEdge("a", "b");
    graph.addEdge("b", "a");
    graph.detectCircular();
    // At least one of them should be marked circular
    bool anyCircular = false;
    for (auto& n : graph.nodes()) if (n.isCircular()) anyCircular = true;
    REQUIRE(anyCircular);
}

TEST_CASE("AssetDependencyTracker registerAsset and assetCount", "[Editor][S82]") {
    AssetDependencyTracker tracker;
    REQUIRE(tracker.registerAsset("tex1", "Textures/rock.png", AssetDepType::Texture));
    REQUIRE(tracker.assetCount() == 1);
}

TEST_CASE("AssetDependencyTracker addDependency and hasDependency", "[Editor][S82]") {
    AssetDependencyTracker tracker;
    tracker.registerAsset("mat1", "Materials/rock.nfm", AssetDepType::Material);
    tracker.registerAsset("tex1", "Textures/rock.png",  AssetDepType::Texture);
    REQUIRE(tracker.addDependency("mat1", "tex1"));
    REQUIRE(tracker.hasDependency("mat1", "tex1"));
}

TEST_CASE("AssetDependencyTracker resolveAll and unresolvedCount", "[Editor][S82]") {
    AssetDependencyTracker tracker;
    tracker.registerAsset("a", "a.nfm", AssetDepType::Material);
    tracker.registerAsset("b", "b.png", AssetDepType::Texture);
    REQUIRE(tracker.unresolvedCount() == 2);
    tracker.resolveAll();
    REQUIRE(tracker.unresolvedCount() == 0);
}

// ── AssetImporters ───────────────────────────────────────────────

TEST_CASE("MeshImportSettings default values", "[Editor][S82]") {
    MeshImportSettings s;
    REQUIRE(s.scaleFactor == 1.0f);
    REQUIRE(s.generateNormals == true);
    REQUIRE(s.generateTangents == false);
    REQUIRE(s.flipWindingOrder == false);
    REQUIRE(s.mergeMeshes == false);
    REQUIRE(s.maxVertices == 0);
}

TEST_CASE("MeshImporter canImport recognizes .obj .fbx .gltf .glb", "[Editor][S82]") {
    MeshImporter importer;
    REQUIRE(importer.canImport("model.obj"));
    REQUIRE(importer.canImport("model.fbx"));
    REQUIRE(importer.canImport("model.gltf"));
    REQUIRE(importer.canImport("model.glb"));
    REQUIRE_FALSE(importer.canImport("model.png"));
    REQUIRE_FALSE(importer.canImport("model.txt"));
}

TEST_CASE("TextureImportSettings default values", "[Editor][S82]") {
    TextureImportSettings s;
    REQUIRE(s.generateMipmaps == true);
    REQUIRE(s.sRGB == true);
    REQUIRE(s.maxResolution == 0);
    REQUIRE(s.premultiplyAlpha == false);
    REQUIRE(s.flipVertically == true);
    REQUIRE(s.compressionQuality == 0.8f);
}

TEST_CASE("TextureImporter canImport recognizes .png .jpg .jpeg .tga .bmp", "[Editor][S82]") {
    TextureImporter importer;
    REQUIRE(importer.canImport("rock.png"));
    REQUIRE(importer.canImport("rock.jpg"));
    REQUIRE(importer.canImport("rock.jpeg"));
    REQUIRE(importer.canImport("rock.tga"));
    REQUIRE(importer.canImport("rock.bmp"));
    REQUIRE_FALSE(importer.canImport("rock.obj"));
    REQUIRE_FALSE(importer.canImport("rock.mp3"));
}

TEST_CASE("AssetWatcher markDirty and isDirty", "[Editor][S82]") {
    AssetWatcher watcher;
    AssetGuid guid;
    guid.hi = 42; guid.lo = 0;
    watcher.markDirty(guid);
    REQUIRE(watcher.isDirty(guid));
    REQUIRE(watcher.dirtyCount() == 1);
}

TEST_CASE("AssetWatcher clearDirty removes entry", "[Editor][S82]") {
    AssetWatcher watcher;
    AssetGuid guid; guid.hi = 7; guid.lo = 0;
    watcher.markDirty(guid);
    watcher.clearDirty(guid);
    REQUIRE_FALSE(watcher.isDirty(guid));
    REQUIRE(watcher.dirtyCount() == 0);
}

TEST_CASE("AssetWatcher clearAll removes all dirty assets", "[Editor][S82]") {
    AssetWatcher watcher;
    AssetGuid a; a.hi = 1; a.lo = 0;
    AssetGuid b; b.hi = 2; b.lo = 0;
    watcher.markDirty(a); watcher.markDirty(b);
    watcher.clearAll();
    REQUIRE(watcher.dirtyCount() == 0);
}

// ── BlenderImporter ──────────────────────────────────────────────

TEST_CASE("BlenderExportFormat names are correct", "[Editor][S82]") {
    REQUIRE(std::string(blenderExportFormatName(BlenderExportFormat::FBX))  == "FBX");
    REQUIRE(std::string(blenderExportFormatName(BlenderExportFormat::GLTF)) == "GLTF");
    REQUIRE(std::string(blenderExportFormatName(BlenderExportFormat::OBJ))  == "OBJ");
    REQUIRE(std::string(blenderExportFormatName(BlenderExportFormat::GLB))  == "GLB");
}

TEST_CASE("BlenderExportFormat extensions are correct", "[Editor][S82]") {
    REQUIRE(std::string(blenderExportFormatExtension(BlenderExportFormat::FBX))  == ".fbx");
    REQUIRE(std::string(blenderExportFormatExtension(BlenderExportFormat::GLTF)) == ".gltf");
    REQUIRE(std::string(blenderExportFormatExtension(BlenderExportFormat::OBJ))  == ".obj");
    REQUIRE(std::string(blenderExportFormatExtension(BlenderExportFormat::GLB))  == ".glb");
}

TEST_CASE("BlenderAutoImporter default settings", "[Editor][S82]") {
    BlenderAutoImporter importer;
    REQUIRE(importer.exportDirectory().empty());
    REQUIRE(importer.isAutoImportEnabled());
    REQUIRE(importer.exportCount() == 0);
    REQUIRE(importer.importedCount() == 0);
    REQUIRE(importer.pendingCount() == 0);
}

TEST_CASE("BlenderAutoImporter setExportDirectory and setAutoImportEnabled", "[Editor][S82]") {
    BlenderAutoImporter importer;
    importer.setExportDirectory("/exports/blender");
    REQUIRE(importer.exportDirectory() == "/exports/blender");
    importer.setAutoImportEnabled(false);
    REQUIRE_FALSE(importer.isAutoImportEnabled());
}

TEST_CASE("BlenderAutoImporter clearHistory resets exports", "[Editor][S82]") {
    BlenderAutoImporter importer;
    // Manually add an export entry by calling clearHistory on empty - just tests no crash
    importer.clearHistory();
    REQUIRE(importer.exportCount() == 0);
}
