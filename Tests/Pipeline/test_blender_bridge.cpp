#include <catch2/catch_test_macros.hpp>
#include "NF/Pipeline/Pipeline.h"

#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

namespace fs = std::filesystem;

// Helper: create a unique temp directory for each test invocation.
static fs::path makeTempDir(std::string_view name) {
    static std::atomic<uint64_t> s_seq{0};
    auto ts  = static_cast<uint64_t>(
        std::chrono::steady_clock::now().time_since_epoch().count());
    auto idx = s_seq.fetch_add(1);
    std::string unique = std::string(name) + "_" +
                         std::to_string(ts)  + "_" +
                         std::to_string(idx);
    auto dir = fs::temp_directory_path() / "nf_blenderbridge_tests" / unique;
    std::error_code ec;
    fs::create_directories(dir, ec);
    return dir;
}

// Helper: create PipelineDirectories rooted at a temp dir.
static NF::PipelineDirectories makeTempPipeline(std::string_view name) {
    auto root = makeTempDir(name);
    auto dirs = NF::PipelineDirectories::fromRoot(root);
    dirs.ensureCreated();
    return dirs;
}

// Helper: create a dummy .glb file (non-empty) at a path under the pipeline.
static void createDummyAsset(const fs::path& fullPath) {
    fs::create_directories(fullPath.parent_path());
    std::ofstream ofs(fullPath, std::ios::binary);
    // Write a minimal binary payload (not a real glTF, but non-empty).
    const char data[] = "glTF\x02\x00\x00\x00";
    ofs.write(data, sizeof(data));
}

// ── AssetImportStatus / AssetImportResult ─────────────────────────────────

TEST_CASE("AssetImportStatus name strings", "[Pipeline][BlenderBridge]") {
    REQUIRE(std::string(NF::assetImportStatusName(NF::AssetImportStatus::Pending))    == "Pending");
    REQUIRE(std::string(NF::assetImportStatusName(NF::AssetImportStatus::Validated))  == "Validated");
    REQUIRE(std::string(NF::assetImportStatusName(NF::AssetImportStatus::Registered)) == "Registered");
    REQUIRE(std::string(NF::assetImportStatusName(NF::AssetImportStatus::Failed))     == "Failed");
}

TEST_CASE("AssetImportResult default-constructs with Pending status", "[Pipeline][BlenderBridge]") {
    NF::AssetImportResult result;
    REQUIRE(result.status == NF::AssetImportStatus::Pending);
    REQUIRE(result.sourcePath.empty());
    REQUIRE(result.guid.empty());
    REQUIRE(result.assetType.empty());
    REQUIRE(result.errorMessage.empty());
    REQUIRE(result.importTimestamp == 0);
}

// ── BlenderBridge construction ────────────────────────────────────────────

TEST_CASE("BlenderBridge constructs with empty state", "[Pipeline][BlenderBridge]") {
    NF::Manifest manifest;
    NF::BlenderBridge bridge(manifest);
    REQUIRE(bridge.importCount() == 0);
    REQUIRE(bridge.failedCount() == 0);
    REQUIRE(bridge.importHistory().empty());
}

// ── importAsset: mesh ─────────────────────────────────────────────────────

TEST_CASE("BlenderBridge imports a mesh asset successfully", "[Pipeline][BlenderBridge]") {
    auto dirs = makeTempPipeline("bridge_mesh");
    NF::Manifest manifest;
    NF::BlenderBridge bridge(manifest);

    // Create a dummy .glb asset file.
    fs::path assetFile = dirs.assets / "ship.glb";
    createDummyAsset(assetFile);

    // Build an AssetImported event as the Blender bridge would emit.
    NF::ChangeEvent ev;
    ev.tool      = "BlenderGenerator";
    ev.eventType = NF::ChangeEventType::AssetImported;
    ev.path      = "pipeline/assets/ship.glb";
    ev.timestamp = 1000LL;
    ev.metadata  = "type=mesh";

    auto result = bridge.importAsset(ev, dirs);

    REQUIRE(result.status == NF::AssetImportStatus::Registered);
    REQUIRE(result.assetType == "mesh");
    REQUIRE_FALSE(result.guid.empty());
    REQUIRE(result.errorMessage.empty());
    REQUIRE(result.sourcePath == "pipeline/assets/ship.glb");
    REQUIRE(result.importTimestamp > 0);

    REQUIRE(bridge.importCount() == 1);
    REQUIRE(bridge.failedCount() == 0);
    REQUIRE(bridge.isImported("pipeline/assets/ship.glb"));
    REQUIRE(bridge.guidForPath("pipeline/assets/ship.glb") == result.guid);

    // Verify Manifest registration.
    REQUIRE(manifest.recordCount() == 1);
    auto* rec = manifest.findByGuid(result.guid);
    REQUIRE(rec != nullptr);
    REQUIRE(rec->type == "mesh");
    REQUIRE(rec->path == "pipeline/assets/ship.glb");
}

// ── importAsset: rig ──────────────────────────────────────────────────────

TEST_CASE("BlenderBridge imports a rig asset successfully", "[Pipeline][BlenderBridge]") {
    auto dirs = makeTempPipeline("bridge_rig");
    NF::Manifest manifest;
    NF::BlenderBridge bridge(manifest);

    fs::path assetFile = dirs.animations / "char_rig.glb";
    createDummyAsset(assetFile);

    NF::ChangeEvent ev;
    ev.tool      = "BlenderGenerator";
    ev.eventType = NF::ChangeEventType::AnimationExported;
    ev.path      = "pipeline/animations/char_rig.glb";
    ev.timestamp = 2000LL;
    ev.metadata  = "type=rig";

    auto result = bridge.importAsset(ev, dirs);

    REQUIRE(result.status == NF::AssetImportStatus::Registered);
    REQUIRE(result.assetType == "rig");
    REQUIRE_FALSE(result.guid.empty());
    REQUIRE(bridge.importCount() == 1);
}

// ── importAsset: animation clip ───────────────────────────────────────────

TEST_CASE("BlenderBridge imports an animation clip successfully", "[Pipeline][BlenderBridge]") {
    auto dirs = makeTempPipeline("bridge_clip");
    NF::Manifest manifest;
    NF::BlenderBridge bridge(manifest);

    fs::path assetFile = dirs.animations / "walk_anim.glb";
    createDummyAsset(assetFile);

    NF::ChangeEvent ev;
    ev.tool      = "BlenderGenerator";
    ev.eventType = NF::ChangeEventType::AnimationExported;
    ev.path      = "pipeline/animations/walk_anim.glb";
    ev.timestamp = 3000LL;
    ev.metadata  = "type=clip";

    auto result = bridge.importAsset(ev, dirs);

    REQUIRE(result.status == NF::AssetImportStatus::Registered);
    REQUIRE(result.assetType == "clip");
    REQUIRE_FALSE(result.guid.empty());
}

// ── importAsset: missing file ─────────────────────────────────────────────

TEST_CASE("BlenderBridge fails when asset file is missing", "[Pipeline][BlenderBridge]") {
    auto dirs = makeTempPipeline("bridge_missing");
    NF::Manifest manifest;
    NF::BlenderBridge bridge(manifest);

    NF::ChangeEvent ev;
    ev.tool      = "BlenderGenerator";
    ev.eventType = NF::ChangeEventType::AssetImported;
    ev.path      = "pipeline/assets/nonexistent.glb";
    ev.timestamp = 4000LL;
    ev.metadata  = "type=mesh";

    auto result = bridge.importAsset(ev, dirs);

    REQUIRE(result.status == NF::AssetImportStatus::Failed);
    REQUIRE_FALSE(result.errorMessage.empty());
    REQUIRE(result.guid.empty());
    REQUIRE(bridge.importCount() == 0);
    REQUIRE(bridge.failedCount() == 1);
    REQUIRE_FALSE(bridge.isImported("pipeline/assets/nonexistent.glb"));
}

// ── importAsset: empty file ───────────────────────────────────────────────

TEST_CASE("BlenderBridge fails when asset file is empty", "[Pipeline][BlenderBridge]") {
    auto dirs = makeTempPipeline("bridge_empty");
    NF::Manifest manifest;
    NF::BlenderBridge bridge(manifest);

    // Create an empty file.
    fs::path assetFile = dirs.assets / "empty.glb";
    fs::create_directories(assetFile.parent_path());
    { std::ofstream ofs(assetFile); }  // empty

    NF::ChangeEvent ev;
    ev.tool      = "BlenderGenerator";
    ev.eventType = NF::ChangeEventType::AssetImported;
    ev.path      = "pipeline/assets/empty.glb";
    ev.timestamp = 5000LL;
    ev.metadata  = "type=mesh";

    auto result = bridge.importAsset(ev, dirs);

    REQUIRE(result.status == NF::AssetImportStatus::Failed);
    REQUIRE_FALSE(result.errorMessage.empty());
    REQUIRE(bridge.failedCount() == 1);
}

// ── importAsset: type detection fallback ──────────────────────────────────

TEST_CASE("BlenderBridge detects asset type from extension when metadata lacks type", "[Pipeline][BlenderBridge]") {
    auto dirs = makeTempPipeline("bridge_type_fallback");
    NF::Manifest manifest;
    NF::BlenderBridge bridge(manifest);

    fs::path assetFile = dirs.assets / "cube.glb";
    createDummyAsset(assetFile);

    NF::ChangeEvent ev;
    ev.tool      = "BlenderGenerator";
    ev.eventType = NF::ChangeEventType::AssetImported;
    ev.path      = "pipeline/assets/cube.glb";
    ev.timestamp = 6000LL;
    ev.metadata  = "source=blender";  // no type= field

    auto result = bridge.importAsset(ev, dirs);

    REQUIRE(result.status == NF::AssetImportStatus::Registered);
    REQUIRE(result.assetType == "mesh");  // inferred from .glb extension
}

TEST_CASE("BlenderBridge detects animation type from event type when metadata lacks type", "[Pipeline][BlenderBridge]") {
    auto dirs = makeTempPipeline("bridge_anim_fallback");
    NF::Manifest manifest;
    NF::BlenderBridge bridge(manifest);

    fs::path assetFile = dirs.animations / "run.glb";
    createDummyAsset(assetFile);

    NF::ChangeEvent ev;
    ev.tool      = "BlenderGenerator";
    ev.eventType = NF::ChangeEventType::AnimationExported;
    ev.path      = "pipeline/animations/run.glb";
    ev.timestamp = 7000LL;
    ev.metadata  = "";  // no metadata at all

    auto result = bridge.importAsset(ev, dirs);

    REQUIRE(result.status == NF::AssetImportStatus::Registered);
    REQUIRE(result.assetType == "animation");  // inferred from AnimationExported
}

// ── importAsset: duplicate import updates ─────────────────────────────────

TEST_CASE("BlenderBridge re-import of same path updates Manifest record", "[Pipeline][BlenderBridge]") {
    auto dirs = makeTempPipeline("bridge_reimport");
    NF::Manifest manifest;
    NF::BlenderBridge bridge(manifest);

    fs::path assetFile = dirs.assets / "helm.glb";
    createDummyAsset(assetFile);

    NF::ChangeEvent ev;
    ev.tool      = "BlenderGenerator";
    ev.eventType = NF::ChangeEventType::AssetImported;
    ev.path      = "pipeline/assets/helm.glb";
    ev.timestamp = 8000LL;
    ev.metadata  = "type=mesh";

    auto r1 = bridge.importAsset(ev, dirs);
    REQUIRE(r1.status == NF::AssetImportStatus::Registered);
    REQUIRE(manifest.recordCount() == 1);

    // Re-import the same asset (simulating an update from Blender).
    ev.timestamp = 9000LL;
    auto r2 = bridge.importAsset(ev, dirs);
    REQUIRE(r2.status == NF::AssetImportStatus::Registered);

    // Manifest should still have 1 record (updated, not duplicated).
    REQUIRE(manifest.recordCount() == 1);
    REQUIRE(bridge.importCount() == 2);
    REQUIRE(bridge.importHistory().size() == 2);
}

// ── importAsset: multiple different assets ────────────────────────────────

TEST_CASE("BlenderBridge tracks multiple assets independently", "[Pipeline][BlenderBridge]") {
    auto dirs = makeTempPipeline("bridge_multi");
    NF::Manifest manifest;
    NF::BlenderBridge bridge(manifest);

    // Import 3 different assets.
    fs::path meshFile = dirs.assets / "turret.glb";
    fs::path rigFile  = dirs.animations / "pilot_rig.glb";
    fs::path clipFile = dirs.animations / "idle_clip.glb";
    createDummyAsset(meshFile);
    createDummyAsset(rigFile);
    createDummyAsset(clipFile);

    NF::ChangeEvent ev1;
    ev1.tool      = "BlenderGenerator";
    ev1.eventType = NF::ChangeEventType::AssetImported;
    ev1.path      = "pipeline/assets/turret.glb";
    ev1.timestamp = 10000LL;
    ev1.metadata  = "type=mesh";

    NF::ChangeEvent ev2;
    ev2.tool      = "BlenderGenerator";
    ev2.eventType = NF::ChangeEventType::AnimationExported;
    ev2.path      = "pipeline/animations/pilot_rig.glb";
    ev2.timestamp = 10001LL;
    ev2.metadata  = "type=rig";

    NF::ChangeEvent ev3;
    ev3.tool      = "BlenderGenerator";
    ev3.eventType = NF::ChangeEventType::AnimationExported;
    ev3.path      = "pipeline/animations/idle_clip.glb";
    ev3.timestamp = 10002LL;
    ev3.metadata  = "type=clip";

    auto r1 = bridge.importAsset(ev1, dirs);
    auto r2 = bridge.importAsset(ev2, dirs);
    auto r3 = bridge.importAsset(ev3, dirs);

    REQUIRE(r1.status == NF::AssetImportStatus::Registered);
    REQUIRE(r2.status == NF::AssetImportStatus::Registered);
    REQUIRE(r3.status == NF::AssetImportStatus::Registered);

    REQUIRE(bridge.importCount() == 3);
    REQUIRE(bridge.failedCount() == 0);
    REQUIRE(manifest.recordCount() == 3);
    REQUIRE(bridge.importHistory().size() == 3);

    // All GUIDs should be unique.
    REQUIRE(r1.guid != r2.guid);
    REQUIRE(r2.guid != r3.guid);
    REQUIRE(r1.guid != r3.guid);

    // Check each is independently queryable.
    REQUIRE(bridge.isImported("pipeline/assets/turret.glb"));
    REQUIRE(bridge.isImported("pipeline/animations/pilot_rig.glb"));
    REQUIRE(bridge.isImported("pipeline/animations/idle_clip.glb"));
    REQUIRE(bridge.guidForPath("pipeline/assets/turret.glb") == r1.guid);
}

// ── isImported / guidForPath for unknown path ─────────────────────────────

TEST_CASE("BlenderBridge returns false/empty for unknown paths", "[Pipeline][BlenderBridge]") {
    NF::Manifest manifest;
    NF::BlenderBridge bridge(manifest);

    REQUIRE_FALSE(bridge.isImported("nonexistent/path.glb"));
    REQUIRE(bridge.guidForPath("nonexistent/path.glb").empty());
}

// ── Manifest persistence roundtrip ────────────────────────────────────────

TEST_CASE("BlenderBridge imports persist through Manifest save/load", "[Pipeline][BlenderBridge]") {
    auto dirs = makeTempPipeline("bridge_persist");
    NF::Manifest manifest;
    manifest.projectName = "TestProject";
    NF::BlenderBridge bridge(manifest);

    fs::path assetFile = dirs.assets / "wall.glb";
    createDummyAsset(assetFile);

    NF::ChangeEvent ev;
    ev.tool      = "BlenderGenerator";
    ev.eventType = NF::ChangeEventType::AssetImported;
    ev.path      = "pipeline/assets/wall.glb";
    ev.timestamp = 20000LL;
    ev.metadata  = "type=mesh";

    auto result = bridge.importAsset(ev, dirs);
    REQUIRE(result.status == NF::AssetImportStatus::Registered);

    // Save the manifest.
    REQUIRE(manifest.save(dirs.manifestFile));

    // Load into a fresh Manifest and verify the asset is there.
    NF::Manifest loaded;
    REQUIRE(loaded.load(dirs.manifestFile));
    REQUIRE(loaded.recordCount() == 1);
    auto* rec = loaded.findByGuid(result.guid);
    REQUIRE(rec != nullptr);
    REQUIRE(rec->type == "mesh");
    REQUIRE(rec->path == "pipeline/assets/wall.glb");
}

// ── End-to-end: PipelineWatcher → ToolRegistry → BlenderBridge ────────────

TEST_CASE("End-to-end: PipelineWatcher dispatches to BlenderBridge", "[Pipeline][BlenderBridge]") {
    auto dirs = makeTempPipeline("bridge_e2e");
    NF::Manifest manifest;
    NF::BlenderBridge bridge(manifest);

    // Set up a ToolRegistry with BlenderGenAdapter.
    NF::ToolRegistry registry;
    registry.registerTool(std::make_unique<NF::BlenderGenAdapter>());

    // Set up a PipelineWatcher and attach the registry.
    NF::PipelineWatcher watcher(dirs.changes);

    // Also subscribe the bridge to handle AssetImported events.
    watcher.subscribe([&bridge, &dirs](const NF::ChangeEvent& event) {
        if (event.eventType == NF::ChangeEventType::AssetImported) {
            bridge.importAsset(event, dirs);
        }
    });

    registry.attach(watcher, dirs);

    // Create a dummy asset and write a .change.json event.
    fs::path assetFile = dirs.assets / "crate.glb";
    createDummyAsset(assetFile);

    NF::ChangeEvent ev;
    ev.tool      = "BlenderGenerator";
    ev.eventType = NF::ChangeEventType::AssetImported;
    ev.path      = "pipeline/assets/crate.glb";
    ev.timestamp = 30000LL;
    ev.metadata  = "type=mesh";
    REQUIRE(ev.writeToFile(dirs.changes));

    // Poll the watcher — should pick up the event and route it.
    int found = watcher.poll();
    REQUIRE(found >= 1);

    // The bridge should have imported the asset.
    REQUIRE(bridge.importCount() == 1);
    REQUIRE(bridge.isImported("pipeline/assets/crate.glb"));

    // The Manifest should have a record.
    REQUIRE(manifest.recordCount() == 1);
}
