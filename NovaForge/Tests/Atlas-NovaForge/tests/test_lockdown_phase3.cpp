#include <iostream>
#include <cassert>
#include <cstdio>
#include <vector>
#include <string>
#include <fstream>
#include <filesystem>

#include "../engine/sim/SimulationStateAuditor.h"
#include "../engine/sim/DesyncReproducer.h"
#include "../engine/assets/ServerAssetValidator.h"
#include "../engine/assets/AssetImporter.h"
#include "../engine/core/Engine.h"

// ============================================================
// SimulationStateAuditor Tests
// ============================================================

void test_auditor_empty_passes() {
    atlas::sim::SimulationStateAuditor auditor;
    auto result = auditor.Audit();
    // No systems registered → nothing to fail
    assert(result.passed);
    assert(result.systemsAudited == 0);
    assert(result.errorsFound == 0);
    std::cout << "[PASS] test_auditor_empty_passes" << std::endl;
}

void test_auditor_system_no_state_fails() {
    atlas::sim::SimulationStateAuditor auditor;
    auditor.RegisterSystem("Physics");
    // Don't declare any state blocks

    auto result = auditor.Audit();
    assert(!result.passed);
    assert(result.systemsAudited == 1);
    assert(result.errorsFound == 1);
    assert(result.findings.size() == 1);
    assert(result.findings[0].severity == atlas::sim::AuditFinding::Severity::Error);
    assert(result.findings[0].systemName == "Physics");
    std::cout << "[PASS] test_auditor_system_no_state_fails" << std::endl;
}

void test_auditor_system_with_state_passes() {
    atlas::sim::SimulationStateAuditor auditor;
    auditor.RegisterSystem("Physics");
    auditor.DeclareState("Physics", "RigidBodies",
                         atlas::sim::StateCategory::Simulated);

    auto result = auditor.Audit();
    assert(result.passed);
    assert(result.systemsAudited == 1);
    assert(result.stateBlocksVerified == 1);
    assert(result.errorsFound == 0);
    std::cout << "[PASS] test_auditor_system_with_state_passes" << std::endl;
}

void test_auditor_multiple_systems() {
    atlas::sim::SimulationStateAuditor auditor;
    auditor.RegisterSystem("Physics");
    auditor.RegisterSystem("AI");
    auditor.RegisterSystem("Gameplay");

    auditor.DeclareState("Physics", "RigidBodies");
    auditor.DeclareState("AI", "AIMemory");
    // Gameplay has no declared state → should fail

    auto result = auditor.Audit();
    assert(!result.passed);
    assert(result.systemsAudited == 3);
    assert(result.errorsFound == 1);

    bool foundGameplay = false;
    for (const auto& f : result.findings) {
        if (f.systemName == "Gameplay") foundGameplay = true;
    }
    assert(foundGameplay);
    std::cout << "[PASS] test_auditor_multiple_systems" << std::endl;
}

void test_auditor_duplicate_register() {
    atlas::sim::SimulationStateAuditor auditor;
    auditor.RegisterSystem("Physics");
    auditor.RegisterSystem("Physics");  // duplicate

    assert(auditor.RegisteredSystems().size() == 1);
    std::cout << "[PASS] test_auditor_duplicate_register" << std::endl;
}

void test_auditor_declared_blocks() {
    atlas::sim::SimulationStateAuditor auditor;
    auditor.RegisterSystem("Physics");
    auditor.DeclareState("Physics", "RigidBodies");
    auditor.DeclareState("Physics", "Constraints");

    assert(auditor.DeclaredStateCount("Physics") == 2);
    auto blocks = auditor.DeclaredBlocks("Physics");
    assert(blocks.size() == 2);
    assert(blocks[0] == "RigidBodies");
    assert(blocks[1] == "Constraints");
    std::cout << "[PASS] test_auditor_declared_blocks" << std::endl;
}

void test_auditor_unregistered_declaration_warning() {
    atlas::sim::SimulationStateAuditor auditor;
    // Declare state for a system that hasn't been registered
    auditor.DeclareState("Orphan", "Data");

    auto result = auditor.Audit();
    assert(result.passed);  // No registered systems → passes
    assert(result.warningsFound == 1);
    assert(result.findings.size() == 1);
    assert(result.findings[0].severity == atlas::sim::AuditFinding::Severity::Warning);
    std::cout << "[PASS] test_auditor_unregistered_declaration_warning" << std::endl;
}

void test_auditor_clear() {
    atlas::sim::SimulationStateAuditor auditor;
    auditor.RegisterSystem("Physics");
    auditor.DeclareState("Physics", "Bodies");

    auditor.Clear();
    assert(auditor.RegisteredSystems().empty());
    assert(auditor.DeclaredStateCount("Physics") == 0);

    auto result = auditor.Audit();
    assert(result.passed);
    assert(result.systemsAudited == 0);
    std::cout << "[PASS] test_auditor_clear" << std::endl;
}

// ============================================================
// ServerAssetValidator Tests
// ============================================================

void test_server_validator_empty_manifest() {
    atlas::asset::ServerAssetValidator validator;
    assert(validator.ManifestSize() == 0);
    assert(!validator.HasAsset("anything"));
    std::cout << "[PASS] test_server_validator_empty_manifest" << std::endl;
}

void test_server_validator_register_and_check() {
    atlas::asset::ServerAssetValidator validator;
    validator.RegisterAsset("texture_stone", 0xABCD, 1);

    assert(validator.ManifestSize() == 1);
    assert(validator.HasAsset("texture_stone"));
    assert(!validator.HasAsset("texture_wood"));
    std::cout << "[PASS] test_server_validator_register_and_check" << std::endl;
}

void test_server_validator_hash_match() {
    atlas::asset::ServerAssetValidator validator;
    validator.RegisterAsset("mesh_wall", 12345, 1);

    auto result = validator.ValidateHash("mesh_wall", 12345);
    assert(result.accepted);
    assert(result.assetId == "mesh_wall");
    assert(result.reason.empty());
    std::cout << "[PASS] test_server_validator_hash_match" << std::endl;
}

void test_server_validator_hash_mismatch() {
    atlas::asset::ServerAssetValidator validator;
    validator.RegisterAsset("mesh_wall", 12345, 1);

    auto result = validator.ValidateHash("mesh_wall", 99999);
    assert(!result.accepted);
    assert(!result.reason.empty());
    std::cout << "[PASS] test_server_validator_hash_mismatch" << std::endl;
}

void test_server_validator_unknown_asset() {
    atlas::asset::ServerAssetValidator validator;

    auto result = validator.ValidateHash("nonexistent", 0);
    assert(!result.accepted);
    assert(result.reason.find("not in server manifest") != std::string::npos);
    std::cout << "[PASS] test_server_validator_unknown_asset" << std::endl;
}

void test_server_validator_file_validation() {
    const std::string dir = "/tmp/atlas_test_server_val";
    std::filesystem::create_directories(dir);

    // Create a valid .atlasb file via import
    std::string srcPath = dir + "/test.png";
    {
        std::ofstream out(srcPath, std::ios::binary);
        std::vector<uint8_t> data = {0x89, 0x50, 0x4E, 0x47};
        out.write(reinterpret_cast<const char*>(data.data()),
                  static_cast<std::streamsize>(data.size()));
    }

    atlas::asset::TextureImporter imp(atlas::asset::ImportFormat::PNG);
    atlas::asset::ImportOptions opts;
    opts.outputDirectory = dir;
    auto importResult = imp.Import(srcPath, opts);
    assert(importResult.success);

    // Register with the correct hash
    atlas::asset::ServerAssetValidator validator;
    validator.RegisterAsset("test_tex", importResult.outputHash, 1);

    auto result = validator.ValidateAsset("test_tex", importResult.outputPath);
    assert(result.accepted);

    // Register with wrong hash
    atlas::asset::ServerAssetValidator validator2;
    validator2.RegisterAsset("test_tex", 0xDEAD, 1);
    auto result2 = validator2.ValidateAsset("test_tex", importResult.outputPath);
    assert(!result2.accepted);

    std::filesystem::remove_all(dir);
    std::cout << "[PASS] test_server_validator_file_validation" << std::endl;
}

void test_server_validator_file_not_found() {
    atlas::asset::ServerAssetValidator validator;
    validator.RegisterAsset("missing", 123, 1);

    auto result = validator.ValidateAsset("missing", "/nonexistent/path.atlasb");
    assert(!result.accepted);
    assert(result.reason.find("File not found") != std::string::npos);
    std::cout << "[PASS] test_server_validator_file_not_found" << std::endl;
}

void test_server_validator_manifest() {
    atlas::asset::ServerAssetValidator validator;
    validator.RegisterAsset("a", 100, 1);
    validator.RegisterAsset("b", 200, 2);

    auto manifest = validator.Manifest();
    assert(manifest.size() == 2);
    std::cout << "[PASS] test_server_validator_manifest" << std::endl;
}

// ============================================================
// Save/Load Determinism Verification Tests
// ============================================================

void test_verify_save_load_determinism() {
    const std::string path = "/tmp/atlas_sld_test.asav";
    std::remove(path.c_str());

    atlas::EngineConfig cfg;
    cfg.mode = atlas::EngineMode::Server;
    cfg.tickRate = 60;
    cfg.maxTicks = 10;

    atlas::Engine engine(cfg);
    engine.InitCore();
    engine.InitECS();
    engine.InitNetworking();
    engine.GetScheduler().SetFramePacing(false);

    engine.GetWorld().CreateEntity();
    engine.Run();

    bool verified = engine.VerifySaveLoadDeterminism(path, 5);
    assert(verified);

    std::remove(path.c_str());
    std::cout << "[PASS] test_verify_save_load_determinism" << std::endl;
}

// ============================================================
// DesyncReproducer Tests
// ============================================================

void test_desync_reproducer_output_dir() {
    atlas::sim::DesyncReproducer reproducer;
    assert(reproducer.OutputDirectory() == "/tmp/atlas_repro");

    reproducer.SetOutputDirectory("/tmp/atlas_custom_repro");
    assert(reproducer.OutputDirectory() == "/tmp/atlas_custom_repro");
    std::cout << "[PASS] test_desync_reproducer_output_dir" << std::endl;
}

void test_desync_reproducer_capture() {
    const std::string dir = "/tmp/atlas_test_repro";
    std::filesystem::remove_all(dir);

    atlas::sim::DesyncReproducer reproducer;
    reproducer.SetOutputDirectory(dir);

    std::vector<uint8_t> ecsData = {1, 2, 3, 4, 5};
    std::vector<uint8_t> replayData = {10, 20, 30};

    auto capture = reproducer.CaptureDesync(42, 0x1111, 0x2222,
                                             ecsData, 60, 99, replayData);

    assert(capture.valid);
    assert(capture.tick == 42);
    assert(capture.localHash == 0x1111);
    assert(capture.remoteHash == 0x2222);
    assert(!capture.savePath.empty());
    assert(!capture.replayPath.empty());
    assert(!capture.reportPath.empty());

    // Verify files exist
    assert(std::filesystem::exists(capture.savePath));
    assert(std::filesystem::exists(capture.replayPath));
    assert(std::filesystem::exists(capture.reportPath));

    assert(reproducer.CaptureCount() == 1);

    // Verify report content
    std::ifstream report(capture.reportPath);
    std::string content((std::istreambuf_iterator<char>(report)),
                         std::istreambuf_iterator<char>());
    assert(content.find("Desync Report") != std::string::npos);
    assert(content.find("42") != std::string::npos);
    assert(content.find("--repro") != std::string::npos);

    std::filesystem::remove_all(dir);
    std::cout << "[PASS] test_desync_reproducer_capture" << std::endl;
}

void test_desync_reproducer_generate_command() {
    atlas::sim::DesyncCapture capture;
    capture.savePath = "/tmp/desync.asav";
    capture.replayPath = "/tmp/desync.rply";

    auto cmd = atlas::sim::DesyncReproducer::GenerateReproCommand(capture);
    assert(cmd.find("--repro") != std::string::npos);
    assert(cmd.find("--save") != std::string::npos);
    assert(cmd.find("--replay") != std::string::npos);
    assert(cmd.find("/tmp/desync.asav") != std::string::npos);
    assert(cmd.find("/tmp/desync.rply") != std::string::npos);
    std::cout << "[PASS] test_desync_reproducer_generate_command" << std::endl;
}

void test_desync_reproducer_multiple_captures() {
    const std::string dir = "/tmp/atlas_test_repro_multi";
    std::filesystem::remove_all(dir);

    atlas::sim::DesyncReproducer reproducer;
    reproducer.SetOutputDirectory(dir);

    std::vector<uint8_t> ecsData = {1, 2, 3};
    std::vector<uint8_t> replayData = {4, 5};

    reproducer.CaptureDesync(10, 0xA, 0xB, ecsData, 60, 1, replayData);
    reproducer.CaptureDesync(20, 0xC, 0xD, ecsData, 60, 1, replayData);

    assert(reproducer.CaptureCount() == 2);
    assert(reproducer.Captures().size() == 2);
    assert(reproducer.Captures()[0].tick == 10);
    assert(reproducer.Captures()[1].tick == 20);

    std::filesystem::remove_all(dir);
    std::cout << "[PASS] test_desync_reproducer_multiple_captures" << std::endl;
}
