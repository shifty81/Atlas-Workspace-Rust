#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"
#include <filesystem>

// ── BlenderExportFormat ──────────────────────────────────────────

TEST_CASE("BlenderExportFormat names", "[Editor][S4]") {
    REQUIRE(std::string(NF::blenderExportFormatName(NF::BlenderExportFormat::FBX))  == "FBX");
    REQUIRE(std::string(NF::blenderExportFormatName(NF::BlenderExportFormat::GLTF)) == "GLTF");
    REQUIRE(std::string(NF::blenderExportFormatName(NF::BlenderExportFormat::OBJ))  == "OBJ");
    REQUIRE(std::string(NF::blenderExportFormatName(NF::BlenderExportFormat::GLB))  == "GLB");
}

TEST_CASE("BlenderExportFormat extensions", "[Editor][S4]") {
    REQUIRE(std::string(NF::blenderExportFormatExtension(NF::BlenderExportFormat::FBX))  == ".fbx");
    REQUIRE(std::string(NF::blenderExportFormatExtension(NF::BlenderExportFormat::GLTF)) == ".gltf");
    REQUIRE(std::string(NF::blenderExportFormatExtension(NF::BlenderExportFormat::OBJ))  == ".obj");
    REQUIRE(std::string(NF::blenderExportFormatExtension(NF::BlenderExportFormat::GLB))  == ".glb");
}

// ── BlenderExportEntry ───────────────────────────────────────────

TEST_CASE("BlenderExportEntry defaults", "[Editor][S4]") {
    NF::BlenderExportEntry entry;
    REQUIRE(entry.sourcePath.empty());
    REQUIRE(entry.format == NF::BlenderExportFormat::FBX);
    REQUIRE(entry.exportedAt == 0);
    REQUIRE_FALSE(entry.autoImported);
    REQUIRE(entry.importedGuid.isNull());
}

// ── BlenderAutoImporter ──────────────────────────────────────────

TEST_CASE("BlenderAutoImporter default state", "[Editor][S4]") {
    NF::BlenderAutoImporter importer;
    REQUIRE(importer.exportDirectory().empty());
    REQUIRE(importer.isAutoImportEnabled());
    REQUIRE(importer.exportCount() == 0);
    REQUIRE(importer.importedCount() == 0);
    REQUIRE(importer.pendingCount() == 0);
}

TEST_CASE("BlenderAutoImporter set export dir", "[Editor][S4]") {
    NF::BlenderAutoImporter importer;
    importer.setExportDirectory("/tmp/blender_exports");
    REQUIRE(importer.exportDirectory() == "/tmp/blender_exports");
}

TEST_CASE("BlenderAutoImporter toggle auto-import", "[Editor][S4]") {
    NF::BlenderAutoImporter importer;
    REQUIRE(importer.isAutoImportEnabled());
    importer.setAutoImportEnabled(false);
    REQUIRE_FALSE(importer.isAutoImportEnabled());
    importer.setAutoImportEnabled(true);
    REQUIRE(importer.isAutoImportEnabled());
}

TEST_CASE("BlenderAutoImporter scan empty dir returns 0", "[Editor][S4]") {
    NF::BlenderAutoImporter importer;
    // No dir set
    REQUIRE(importer.scanExports() == 0);
    // Non-existent dir
    importer.setExportDirectory("/tmp/nonexistent_blender_dir_12345");
    REQUIRE(importer.scanExports() == 0);
}

TEST_CASE("BlenderAutoImporter scan with real files", "[Editor][S4]") {
    // Create a temp directory with mock export files
    std::string dir = "/tmp/nf_s4_test_exports";
    std::filesystem::create_directories(dir);
    // Create mock files
    { std::ofstream f(dir + "/model.fbx"); f << "mock"; }
    { std::ofstream f(dir + "/scene.gltf"); f << "mock"; }
    { std::ofstream f(dir + "/readme.txt"); f << "skip"; }  // not an export format

    NF::BlenderAutoImporter importer;
    importer.setExportDirectory(dir);

    size_t found = importer.scanExports();
    REQUIRE(found == 2);
    REQUIRE(importer.exportCount() == 2);

    // Scanning again should not re-add
    found = importer.scanExports();
    REQUIRE(found == 0);
    REQUIRE(importer.exportCount() == 2);

    // Cleanup
    std::filesystem::remove_all(dir);
}

TEST_CASE("BlenderAutoImporter import pending", "[Editor][S4]") {
    std::string dir = "/tmp/nf_s4_test_import";
    std::filesystem::create_directories(dir);
    { std::ofstream f(dir + "/ship.obj"); f << "v 0 0 0"; }

    NF::BlenderAutoImporter importer;
    importer.setExportDirectory(dir);
    importer.scanExports();
    REQUIRE(importer.pendingCount() == 1);

    NF::AssetDatabase db;
    NF::MeshImporter meshImporter;
    size_t imported = importer.importPending(db, meshImporter);
    REQUIRE(imported == 1);
    REQUIRE(importer.importedCount() == 1);
    REQUIRE(importer.pendingCount() == 0);
    REQUIRE(db.assetCount() == 1);

    // Re-import should do nothing
    imported = importer.importPending(db, meshImporter);
    REQUIRE(imported == 0);

    std::filesystem::remove_all(dir);
}

TEST_CASE("BlenderAutoImporter poll convenience", "[Editor][S4]") {
    std::string dir = "/tmp/nf_s4_test_poll";
    std::filesystem::create_directories(dir);
    { std::ofstream f(dir + "/cube.glb"); f << "mock"; }

    NF::BlenderAutoImporter importer;
    importer.setExportDirectory(dir);

    NF::AssetDatabase db;
    NF::MeshImporter meshImporter;

    // Poll = scan + import
    size_t imported = importer.poll(db, meshImporter);
    REQUIRE(imported == 1);
    REQUIRE(importer.exportCount() == 1);
    REQUIRE(importer.importedCount() == 1);

    std::filesystem::remove_all(dir);
}

TEST_CASE("BlenderAutoImporter poll with auto-import disabled", "[Editor][S4]") {
    std::string dir = "/tmp/nf_s4_test_no_auto";
    std::filesystem::create_directories(dir);
    { std::ofstream f(dir + "/tree.fbx"); f << "mock"; }

    NF::BlenderAutoImporter importer;
    importer.setExportDirectory(dir);
    importer.setAutoImportEnabled(false);

    NF::AssetDatabase db;
    NF::MeshImporter meshImporter;

    // Poll should scan but not import
    size_t imported = importer.poll(db, meshImporter);
    REQUIRE(imported == 0);
    REQUIRE(importer.exportCount() == 1);
    REQUIRE(importer.pendingCount() == 1);

    std::filesystem::remove_all(dir);
}

TEST_CASE("BlenderAutoImporter clearHistory", "[Editor][S4]") {
    NF::BlenderAutoImporter importer;
    // Manually test clear
    std::string dir = "/tmp/nf_s4_test_clear";
    std::filesystem::create_directories(dir);
    { std::ofstream f(dir + "/a.obj"); f << "v"; }

    importer.setExportDirectory(dir);
    importer.scanExports();
    REQUIRE(importer.exportCount() == 1);

    importer.clearHistory();
    REQUIRE(importer.exportCount() == 0);

    std::filesystem::remove_all(dir);
}

// ── EditorApp S4 Integration ─────────────────────────────────────

TEST_CASE("EditorApp S4 Blender bridge accessible", "[Editor][S4]") {
    NF::EditorApp app;
    app.init(800, 600);

    // BlenderAutoImporter starts with defaults
    REQUIRE(app.blenderAutoImporter().exportDirectory().empty());
    REQUIRE(app.blenderAutoImporter().isAutoImportEnabled());

    // S4 commands registered
    REQUIRE(app.commands().findCommand("blender.set_export_dir") != nullptr);
    REQUIRE(app.commands().findCommand("blender.scan_exports") != nullptr);
    REQUIRE(app.commands().findCommand("blender.import_pending") != nullptr);
    REQUIRE(app.commands().findCommand("blender.toggle_auto_import") != nullptr);

    app.shutdown();
}
