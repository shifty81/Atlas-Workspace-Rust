#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/EditorApp.h"
#include "NF/Editor/AssetDatabase.h"
#include "NF/Editor/AssetImporters.h"

// ── AssetGuid ────────────────────────────────────────────────────

TEST_CASE("AssetGuid null check", "[Editor][M4]") {
    NF::AssetGuid g;
    REQUIRE(g.isNull());
    REQUIRE(g.hi == 0);
    REQUIRE(g.lo == 0);
}

TEST_CASE("AssetGuid fromPath is deterministic", "[Editor][M4]") {
    auto a = NF::AssetGuid::fromPath("meshes/cube.obj");
    auto b = NF::AssetGuid::fromPath("meshes/cube.obj");
    REQUIRE(a == b);
    REQUIRE_FALSE(a.isNull());
}

TEST_CASE("AssetGuid fromPath different paths differ", "[Editor][M4]") {
    auto a = NF::AssetGuid::fromPath("meshes/cube.obj");
    auto b = NF::AssetGuid::fromPath("meshes/sphere.obj");
    REQUIRE(a != b);
}

TEST_CASE("AssetGuid generate creates unique values", "[Editor][M4]") {
    auto a = NF::AssetGuid::generate(1);
    auto b = NF::AssetGuid::generate(2);
    REQUIRE_FALSE(a.isNull());
    REQUIRE(a != b);
}

TEST_CASE("AssetGuid toString format", "[Editor][M4]") {
    auto g = NF::AssetGuid::fromPath("test.obj");
    auto str = g.toString();
    REQUIRE(str.size() == 33);  // 16 + dash + 16
    REQUIRE(str[16] == '-');
}

TEST_CASE("AssetGuid ordering", "[Editor][M4]") {
    auto a = NF::AssetGuid::fromPath("aaa");
    auto b = NF::AssetGuid::fromPath("zzz");
    // Just verify < is a strict weak ordering (one must be less)
    REQUIRE((a < b || b < a));
    REQUIRE_FALSE(a < a);
}

// ── AssetType ────────────────────────────────────────────────────

TEST_CASE("AssetType names", "[Editor][M4]") {
    REQUIRE(std::string(NF::assetTypeName(NF::AssetType::Mesh))     == "Mesh");
    REQUIRE(std::string(NF::assetTypeName(NF::AssetType::Texture))  == "Texture");
    REQUIRE(std::string(NF::assetTypeName(NF::AssetType::Material)) == "Material");
    REQUIRE(std::string(NF::assetTypeName(NF::AssetType::Sound))    == "Sound");
    REQUIRE(std::string(NF::assetTypeName(NF::AssetType::Script))   == "Script");
    REQUIRE(std::string(NF::assetTypeName(NF::AssetType::Graph))    == "Graph");
    REQUIRE(std::string(NF::assetTypeName(NF::AssetType::World))    == "World");
    REQUIRE(std::string(NF::assetTypeName(NF::AssetType::Unknown))  == "Unknown");
}

TEST_CASE("classifyAssetExtension", "[Editor][M4]") {
    REQUIRE(NF::classifyAssetExtension(".obj")  == NF::AssetType::Mesh);
    REQUIRE(NF::classifyAssetExtension(".fbx")  == NF::AssetType::Mesh);
    REQUIRE(NF::classifyAssetExtension(".gltf") == NF::AssetType::Mesh);
    REQUIRE(NF::classifyAssetExtension(".png")  == NF::AssetType::Texture);
    REQUIRE(NF::classifyAssetExtension(".jpg")  == NF::AssetType::Texture);
    REQUIRE(NF::classifyAssetExtension(".wav")  == NF::AssetType::Sound);
    REQUIRE(NF::classifyAssetExtension(".lua")  == NF::AssetType::Script);
    REQUIRE(NF::classifyAssetExtension(".nfg")  == NF::AssetType::Graph);
    REQUIRE(NF::classifyAssetExtension(".nfw")  == NF::AssetType::World);
    REQUIRE(NF::classifyAssetExtension(".xyz")  == NF::AssetType::Unknown);
}

// ── AssetDatabase ────────────────────────────────────────────────

TEST_CASE("AssetDatabase register and find", "[Editor][M4]") {
    NF::AssetDatabase db;
    REQUIRE(db.assetCount() == 0);

    auto guid = db.registerAsset("meshes/cube.obj", NF::AssetType::Mesh, 1024);
    REQUIRE_FALSE(guid.isNull());
    REQUIRE(db.assetCount() == 1);

    auto* entry = db.findByGuid(guid);
    REQUIRE(entry != nullptr);
    REQUIRE(entry->path == "meshes/cube.obj");
    REQUIRE(entry->name == "cube");
    REQUIRE(entry->type == NF::AssetType::Mesh);
    REQUIRE(entry->sizeBytes == 1024);
    REQUIRE_FALSE(entry->imported);
}

TEST_CASE("AssetDatabase findByPath", "[Editor][M4]") {
    NF::AssetDatabase db;
    db.registerAsset("textures/wall.png", NF::AssetType::Texture, 2048);

    auto* entry = db.findByPath("textures/wall.png");
    REQUIRE(entry != nullptr);
    REQUIRE(entry->name == "wall");
}

TEST_CASE("AssetDatabase duplicate path updates existing", "[Editor][M4]") {
    NF::AssetDatabase db;
    auto g1 = db.registerAsset("test.obj", NF::AssetType::Mesh, 100);
    auto g2 = db.registerAsset("test.obj", NF::AssetType::Mesh, 200);
    REQUIRE(g1 == g2);
    REQUIRE(db.assetCount() == 1);
    REQUIRE(db.findByGuid(g1)->sizeBytes == 200);
}

TEST_CASE("AssetDatabase remove", "[Editor][M4]") {
    NF::AssetDatabase db;
    auto guid = db.registerAsset("test.obj", NF::AssetType::Mesh);
    REQUIRE(db.assetCount() == 1);
    REQUIRE(db.removeAsset(guid));
    REQUIRE(db.assetCount() == 0);
    REQUIRE_FALSE(db.removeAsset(guid));  // already removed
}

TEST_CASE("AssetDatabase markImported", "[Editor][M4]") {
    NF::AssetDatabase db;
    auto guid = db.registerAsset("test.png", NF::AssetType::Texture);
    REQUIRE(db.importedCount() == 0);
    REQUIRE(db.markImported(guid));
    REQUIRE(db.importedCount() == 1);
    REQUIRE(db.findByGuid(guid)->imported);
}

TEST_CASE("AssetDatabase assetsOfType", "[Editor][M4]") {
    NF::AssetDatabase db;
    db.registerAsset("a.obj", NF::AssetType::Mesh);
    db.registerAsset("b.png", NF::AssetType::Texture);
    db.registerAsset("c.fbx", NF::AssetType::Mesh);

    auto meshes = db.assetsOfType(NF::AssetType::Mesh);
    REQUIRE(meshes.size() == 2);
    auto textures = db.assetsOfType(NF::AssetType::Texture);
    REQUIRE(textures.size() == 1);
}

TEST_CASE("AssetDatabase clear", "[Editor][M4]") {
    NF::AssetDatabase db;
    db.registerAsset("a.obj", NF::AssetType::Mesh);
    db.registerAsset("b.png", NF::AssetType::Texture);
    db.clear();
    REQUIRE(db.assetCount() == 0);
}

// ── MeshImporter ─────────────────────────────────────────────────

TEST_CASE("MeshImporter canImport", "[Editor][M4]") {
    NF::MeshImporter importer;
    REQUIRE(importer.canImport("test.obj"));
    REQUIRE(importer.canImport("models/ship.fbx"));
    REQUIRE(importer.canImport("assets/scene.gltf"));
    REQUIRE(importer.canImport("packed.glb"));
    REQUIRE_FALSE(importer.canImport("texture.png"));
    REQUIRE_FALSE(importer.canImport("script.lua"));
}

TEST_CASE("MeshImporter import registers and marks imported", "[Editor][M4]") {
    NF::AssetDatabase db;
    NF::MeshImporter importer;
    auto guid = importer.import(db, "models/cube.obj");
    REQUIRE_FALSE(guid.isNull());
    REQUIRE(db.assetCount() == 1);
    REQUIRE(db.findByGuid(guid)->imported);
    REQUIRE(importer.importCount() == 1);
}

TEST_CASE("MeshImporter rejects non-mesh files", "[Editor][M4]") {
    NF::AssetDatabase db;
    NF::MeshImporter importer;
    auto guid = importer.import(db, "texture.png");
    REQUIRE(guid.isNull());
    REQUIRE(db.assetCount() == 0);
}

TEST_CASE("MeshImporter settings", "[Editor][M4]") {
    NF::MeshImporter importer;
    NF::MeshImportSettings s;
    s.scaleFactor = 2.5f;
    s.generateTangents = true;
    importer.setSettings(s);
    REQUIRE(importer.settings().scaleFactor == 2.5f);
    REQUIRE(importer.settings().generateTangents);
}

// ── TextureImporter ──────────────────────────────────────────────

TEST_CASE("TextureImporter canImport", "[Editor][M4]") {
    NF::TextureImporter importer;
    REQUIRE(importer.canImport("wall.png"));
    REQUIRE(importer.canImport("floor.jpg"));
    REQUIRE(importer.canImport("detail.tga"));
    REQUIRE(importer.canImport("icon.bmp"));
    REQUIRE_FALSE(importer.canImport("model.obj"));
}

TEST_CASE("TextureImporter import registers and marks imported", "[Editor][M4]") {
    NF::AssetDatabase db;
    NF::TextureImporter importer;
    auto guid = importer.import(db, "textures/wall.png");
    REQUIRE_FALSE(guid.isNull());
    REQUIRE(db.assetCount() == 1);
    REQUIRE(db.findByGuid(guid)->imported);
    REQUIRE(importer.importCount() == 1);
}

TEST_CASE("TextureImporter settings", "[Editor][M4]") {
    NF::TextureImporter importer;
    NF::TextureImportSettings s;
    s.generateMipmaps = false;
    s.maxResolution = 2048;
    importer.setSettings(s);
    REQUIRE_FALSE(importer.settings().generateMipmaps);
    REQUIRE(importer.settings().maxResolution == 2048);
}

// ── AssetWatcher ─────────────────────────────────────────────────

TEST_CASE("AssetWatcher dirty tracking", "[Editor][M4]") {
    NF::AssetWatcher watcher;
    REQUIRE(watcher.dirtyCount() == 0);

    auto guid = NF::AssetGuid::fromPath("test.obj");
    watcher.markDirty(guid);
    REQUIRE(watcher.dirtyCount() == 1);
    REQUIRE(watcher.isDirty(guid));

    watcher.clearDirty(guid);
    REQUIRE(watcher.dirtyCount() == 0);
    REQUIRE_FALSE(watcher.isDirty(guid));
}

TEST_CASE("AssetWatcher clearAll", "[Editor][M4]") {
    NF::AssetWatcher watcher;
    watcher.markDirty(NF::AssetGuid::fromPath("a.obj"));
    watcher.markDirty(NF::AssetGuid::fromPath("b.png"));
    REQUIRE(watcher.dirtyCount() == 2);
    watcher.clearAll();
    REQUIRE(watcher.dirtyCount() == 0);
}

// ── EditorApp M4 Integration ─────────────────────────────────────

TEST_CASE("EditorApp M4 asset pipeline accessible", "[Editor][M4]") {
    NF::EditorApp app;
    app.init(800, 600);

    // Asset database starts empty
    REQUIRE(app.assetDatabase().assetCount() == 0);

    // Register an asset manually
    auto guid = app.assetDatabase().registerAsset("test.obj", NF::AssetType::Mesh, 512);
    REQUIRE_FALSE(guid.isNull());
    REQUIRE(app.assetDatabase().assetCount() == 1);

    // Mesh importer accessible
    REQUIRE(app.meshImporter().canImport("model.fbx"));

    // Texture importer accessible
    REQUIRE(app.textureImporter().canImport("wall.png"));

    // Asset watcher accessible
    REQUIRE(app.assetWatcher().dirtyCount() == 0);

    // Asset pipeline commands registered
    REQUIRE(app.commands().findCommand("assets.scan") != nullptr);
    REQUIRE(app.commands().findCommand("assets.reimport") != nullptr);

    app.shutdown();
}
