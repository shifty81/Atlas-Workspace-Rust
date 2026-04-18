#include <iostream>
#include <cassert>
#include <cstdio>
#include <vector>
#include <string>
#include <fstream>
#include <filesystem>

#include "../engine/sim/FPDriftDetector.h"
#include "../engine/sim/StateHasher.h"
#include "../engine/assets/AssetImporter.h"
#include "../engine/assets/AssetValidator.h"
#include "../engine/assets/AssetFormat.h"

// ============================================================
// FP Drift Detector Tests
// ============================================================

void test_fp_drift_no_drift() {
    atlas::sim::StateHasher a, b;
    a.Reset(0);
    b.Reset(0);

    std::vector<uint8_t> state = {1, 2, 3};
    std::vector<uint8_t> inputs = {0};

    a.AdvanceTick(0, state, inputs);
    a.AdvanceTick(1, state, inputs);
    b.AdvanceTick(0, state, inputs);
    b.AdvanceTick(1, state, inputs);

    atlas::sim::FPDriftDetector detector;
    detector.SetPlatformNames("Linux", "Windows");
    auto report = detector.RunComparison(a, b);

    assert(!report.hasDrift);
    assert(report.firstDivergenceTick == -1);
    assert(report.totalTicksCompared == 2);
    assert(report.matchingTicks == 2);
    assert(report.platformAName == "Linux");
    assert(report.platformBName == "Windows");

    std::cout << "[PASS] test_fp_drift_no_drift" << std::endl;
}

void test_fp_drift_detected() {
    atlas::sim::StateHasher a, b;
    a.Reset(0);
    b.Reset(0);

    std::vector<uint8_t> state1 = {1, 2, 3};
    std::vector<uint8_t> state2 = {4, 5, 6};
    std::vector<uint8_t> inputs = {0};

    a.AdvanceTick(0, state1, inputs);
    a.AdvanceTick(1, state1, inputs);
    a.AdvanceTick(2, state2, inputs);  // diverges here

    b.AdvanceTick(0, state1, inputs);
    b.AdvanceTick(1, state1, inputs);
    b.AdvanceTick(2, state1, inputs);  // different state

    atlas::sim::FPDriftDetector detector;
    auto report = detector.RunComparison(a, b);

    assert(report.hasDrift);
    assert(report.firstDivergenceTick == 2);
    assert(report.totalTicksCompared == 3);
    assert(report.matchingTicks == 2);
    assert(report.platformAHash != report.platformBHash);

    std::cout << "[PASS] test_fp_drift_detected" << std::endl;
}

void test_fp_drift_system_comparison() {
    std::vector<std::pair<std::string, uint64_t>> sysA = {
        {"Physics", 100}, {"AI", 200}, {"Animation", 300}
    };
    std::vector<std::pair<std::string, uint64_t>> sysB = {
        {"Physics", 100}, {"AI", 999}, {"Animation", 300}
    };

    auto drifts = atlas::sim::FPDriftDetector::CompareSystemHashes(sysA, sysB);

    assert(drifts.size() == 3);
    assert(drifts[0].systemName == "Physics");
    assert(drifts[0].matches == true);
    assert(drifts[1].systemName == "AI");
    assert(drifts[1].matches == false);
    assert(drifts[1].hashA == 200);
    assert(drifts[1].hashB == 999);
    assert(drifts[2].systemName == "Animation");
    assert(drifts[2].matches == true);

    std::cout << "[PASS] test_fp_drift_system_comparison" << std::endl;
}

void test_fp_drift_format_report() {
    atlas::sim::FPDriftReport report;
    report.hasDrift = true;
    report.firstDivergenceTick = 42;
    report.platformAName = "Linux";
    report.platformBName = "macOS";
    report.platformAHash = 0xDEAD;
    report.platformBHash = 0xBEEF;
    report.totalTicksCompared = 100;
    report.matchingTicks = 42;

    auto str = atlas::sim::FPDriftDetector::FormatReport(report);

    assert(!str.empty());
    assert(str.find("Linux") != std::string::npos);
    assert(str.find("macOS") != std::string::npos);
    assert(str.find("DRIFT DETECTED") != std::string::npos);

    std::cout << "[PASS] test_fp_drift_format_report" << std::endl;
}

void test_fp_drift_export_report() {
    const std::string path = "/tmp/atlas_test_fp_drift_report.txt";

    atlas::sim::FPDriftReport report;
    report.platformAName = "Win64";
    report.platformBName = "Linux64";
    report.totalTicksCompared = 50;
    report.matchingTicks = 50;

    assert(atlas::sim::FPDriftDetector::ExportReport(report, path));

    std::ifstream in(path);
    assert(in.is_open());
    std::string content((std::istreambuf_iterator<char>(in)),
                         std::istreambuf_iterator<char>());
    assert(content.find("Win64") != std::string::npos);
    assert(content.find("Linux64") != std::string::npos);

    std::remove(path.c_str());
    std::cout << "[PASS] test_fp_drift_export_report" << std::endl;
}

void test_fp_drift_add_system_hashes() {
    atlas::sim::StateHasher a, b;
    a.Reset(0);
    b.Reset(0);

    std::vector<uint8_t> state1 = {1};
    std::vector<uint8_t> state2 = {2};
    std::vector<uint8_t> inputs = {0};

    a.AdvanceTick(0, state1, inputs);
    b.AdvanceTick(0, state2, inputs);

    atlas::sim::FPDriftDetector detector;
    detector.SetPlatformNames("ARM", "x86");

    std::vector<std::pair<std::string, uint64_t>> sysA = {{"Render", 10}};
    std::vector<std::pair<std::string, uint64_t>> sysB = {{"Render", 20}};
    detector.AddSystemHashes("Render", sysA, sysB);

    auto report = detector.RunComparison(a, b);
    assert(report.hasDrift);
    assert(report.systemDrifts.size() == 1);
    assert(report.systemDrifts[0].systemName == "Render");
    assert(!report.systemDrifts[0].matches);

    std::cout << "[PASS] test_fp_drift_add_system_hashes" << std::endl;
}

// ============================================================
// Asset Import Pipeline Tests
// ============================================================

void test_asset_import_registry_empty() {
    atlas::asset::AssetImportRegistry registry;
    assert(registry.ImporterCount() == 0);
    assert(registry.SupportedFormats().empty());
    assert(registry.FindImporter("test.png") == nullptr);

    std::cout << "[PASS] test_asset_import_registry_empty" << std::endl;
}

void test_asset_import_registry_register() {
    atlas::asset::AssetImportRegistry registry;
    registry.RegisterImporter(
        std::make_unique<atlas::asset::MeshImporter>(atlas::asset::ImportFormat::FBX));
    registry.RegisterImporter(
        std::make_unique<atlas::asset::TextureImporter>(atlas::asset::ImportFormat::PNG));

    assert(registry.ImporterCount() == 2);
    assert(registry.SupportedFormats().size() == 2);

    std::cout << "[PASS] test_asset_import_registry_register" << std::endl;
}

void test_asset_import_mesh_can_import() {
    atlas::asset::MeshImporter imp(atlas::asset::ImportFormat::FBX);
    assert(imp.CanImport("model.fbx"));
    assert(imp.CanImport("model.obj"));
    assert(imp.CanImport("model.gltf"));
    assert(imp.CanImport("model.glb"));
    assert(!imp.CanImport("image.png"));
    assert(!imp.CanImport("audio.wav"));
    assert(imp.OutputType() == atlas::asset::AssetType::Mesh);

    std::cout << "[PASS] test_asset_import_mesh_can_import" << std::endl;
}

void test_asset_import_texture_can_import() {
    atlas::asset::TextureImporter imp(atlas::asset::ImportFormat::PNG);
    assert(imp.CanImport("image.png"));
    assert(imp.CanImport("image.dds"));
    assert(imp.CanImport("image.tga"));
    assert(!imp.CanImport("model.fbx"));
    assert(imp.OutputType() == atlas::asset::AssetType::Texture);

    std::cout << "[PASS] test_asset_import_texture_can_import" << std::endl;
}

void test_asset_import_audio_can_import() {
    atlas::asset::AudioImporter imp(atlas::asset::ImportFormat::WAV);
    assert(imp.CanImport("sound.wav"));
    assert(imp.CanImport("sound.ogg"));
    assert(imp.CanImport("sound.flac"));
    assert(!imp.CanImport("model.obj"));
    assert(imp.OutputType() == atlas::asset::AssetType::Audio);

    std::cout << "[PASS] test_asset_import_audio_can_import" << std::endl;
}

void test_asset_import_font_can_import() {
    atlas::asset::FontImporter imp(atlas::asset::ImportFormat::TTF);
    assert(imp.CanImport("font.ttf"));
    assert(imp.CanImport("font.otf"));
    assert(!imp.CanImport("model.fbx"));
    assert(imp.OutputType() == atlas::asset::AssetType::Font);

    std::cout << "[PASS] test_asset_import_font_can_import" << std::endl;
}

void test_asset_import_sprite_can_import() {
    atlas::asset::SpriteSheetImporter imp;
    assert(imp.CanImport("sprites.sprite"));
    assert(imp.CanImport("sprites.spritesheet"));
    assert(!imp.CanImport("model.fbx"));
    assert(imp.OutputType() == atlas::asset::AssetType::SpriteSheet);

    std::cout << "[PASS] test_asset_import_sprite_can_import" << std::endl;
}

void test_asset_import_find_importer() {
    atlas::asset::AssetImportRegistry registry;
    registry.RegisterImporter(
        std::make_unique<atlas::asset::MeshImporter>(atlas::asset::ImportFormat::FBX));
    registry.RegisterImporter(
        std::make_unique<atlas::asset::TextureImporter>(atlas::asset::ImportFormat::PNG));
    registry.RegisterImporter(
        std::make_unique<atlas::asset::AudioImporter>(atlas::asset::ImportFormat::WAV));
    registry.RegisterImporter(
        std::make_unique<atlas::asset::FontImporter>(atlas::asset::ImportFormat::TTF));
    registry.RegisterImporter(
        std::make_unique<atlas::asset::SpriteSheetImporter>());

    assert(registry.FindImporter("model.fbx") != nullptr);
    assert(registry.FindImporter("image.png") != nullptr);
    assert(registry.FindImporter("sound.wav") != nullptr);
    assert(registry.FindImporter("font.ttf") != nullptr);
    assert(registry.FindImporter("sprites.sprite") != nullptr);
    assert(registry.FindImporter("unknown.xyz") == nullptr);

    assert(registry.FindImporterByFormat(atlas::asset::ImportFormat::FBX) != nullptr);
    assert(registry.FindImporterByFormat(atlas::asset::ImportFormat::PNG) != nullptr);

    std::cout << "[PASS] test_asset_import_find_importer" << std::endl;
}

void test_asset_import_file_roundtrip() {
    const std::string dir = "/tmp/atlas_test_import";
    std::filesystem::create_directories(dir);

    // Create a fake PNG file
    std::string srcPath = dir + "/test.png";
    {
        std::ofstream out(srcPath, std::ios::binary);
        std::vector<uint8_t> data = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};
        out.write(reinterpret_cast<const char*>(data.data()),
                  static_cast<std::streamsize>(data.size()));
    }

    atlas::asset::TextureImporter imp(atlas::asset::ImportFormat::PNG);
    atlas::asset::ImportOptions opts;
    opts.outputDirectory = dir;

    auto result = imp.Import(srcPath, opts);
    assert(result.success);
    assert(!result.outputPath.empty());
    assert(result.outputType == atlas::asset::AssetType::Texture);
    assert(result.outputHash != 0);
    assert(result.errorMessage.empty());

    // Verify the .atlasb file was created
    assert(std::filesystem::exists(result.outputPath));

    // Read back and verify header
    std::ifstream in(result.outputPath, std::ios::binary);
    atlas::asset::AssetHeader hdr;
    in.read(reinterpret_cast<char*>(&hdr), sizeof(hdr));
    assert(hdr.magic == atlas::asset::ASSET_MAGIC);
    assert(hdr.type == atlas::asset::AssetType::Texture);
    assert(hdr.size == 8);
    assert(hdr.hash == result.outputHash);

    std::filesystem::remove_all(dir);
    std::cout << "[PASS] test_asset_import_file_roundtrip" << std::endl;
}

void test_asset_import_file_not_found() {
    atlas::asset::MeshImporter imp(atlas::asset::ImportFormat::FBX);
    atlas::asset::ImportOptions opts;

    auto result = imp.Import("/nonexistent/path/model.fbx", opts);
    assert(!result.success);
    assert(!result.errorMessage.empty());

    std::cout << "[PASS] test_asset_import_file_not_found" << std::endl;
}

void test_asset_import_registry_import() {
    const std::string dir = "/tmp/atlas_test_import_reg";
    std::filesystem::create_directories(dir);

    std::string srcPath = dir + "/sound.wav";
    {
        std::ofstream out(srcPath, std::ios::binary);
        std::vector<uint8_t> data = {0x52, 0x49, 0x46, 0x46}; // RIFF header start
        out.write(reinterpret_cast<const char*>(data.data()),
                  static_cast<std::streamsize>(data.size()));
    }

    atlas::asset::AssetImportRegistry registry;
    registry.RegisterImporter(
        std::make_unique<atlas::asset::AudioImporter>(atlas::asset::ImportFormat::WAV));

    atlas::asset::ImportOptions opts;
    opts.outputDirectory = dir;

    auto result = registry.Import(srcPath, opts);
    assert(result.success);
    assert(result.outputType == atlas::asset::AssetType::Audio);

    // Test unsupported format
    auto result2 = registry.Import("/tmp/unknown.xyz", opts);
    assert(!result2.success);
    assert(!result2.errorMessage.empty());

    std::filesystem::remove_all(dir);
    std::cout << "[PASS] test_asset_import_registry_import" << std::endl;
}

// ============================================================
// Asset Validator Tests
// ============================================================

void test_asset_validator_validate_valid_file() {
    const std::string dir = "/tmp/atlas_test_validate";
    std::filesystem::create_directories(dir);

    // Create a valid .atlasb file via import
    std::string srcPath = dir + "/mesh.obj";
    {
        std::ofstream out(srcPath, std::ios::binary);
        std::vector<uint8_t> data = {0x01, 0x02, 0x03, 0x04, 0x05};
        out.write(reinterpret_cast<const char*>(data.data()),
                  static_cast<std::streamsize>(data.size()));
    }

    atlas::asset::MeshImporter imp(atlas::asset::ImportFormat::OBJ);
    atlas::asset::ImportOptions opts;
    opts.outputDirectory = dir;
    auto importResult = imp.Import(srcPath, opts);
    assert(importResult.success);

    atlas::asset::AssetValidator validator;
    auto result = validator.Validate(importResult.outputPath);
    assert(result.valid);
    assert(result.formatValid);
    assert(result.hashMatch);
    assert(result.errorMessage.empty());

    std::filesystem::remove_all(dir);
    std::cout << "[PASS] test_asset_validator_validate_valid_file" << std::endl;
}

void test_asset_validator_validate_corrupt_file() {
    const std::string dir = "/tmp/atlas_test_validate_corrupt";
    std::filesystem::create_directories(dir);

    // Create a valid file first
    std::string srcPath = dir + "/tex.png";
    {
        std::ofstream out(srcPath, std::ios::binary);
        std::vector<uint8_t> data = {0xAA, 0xBB, 0xCC, 0xDD};
        out.write(reinterpret_cast<const char*>(data.data()),
                  static_cast<std::streamsize>(data.size()));
    }

    atlas::asset::TextureImporter imp(atlas::asset::ImportFormat::PNG);
    atlas::asset::ImportOptions opts;
    opts.outputDirectory = dir;
    auto importResult = imp.Import(srcPath, opts);
    assert(importResult.success);

    // Corrupt the data portion
    {
        std::fstream f(importResult.outputPath,
                       std::ios::in | std::ios::out | std::ios::binary);
        assert(f.is_open());
        f.seekp(static_cast<std::streamoff>(sizeof(atlas::asset::AssetHeader)));
        uint8_t corrupt = 0xFF;
        f.write(reinterpret_cast<char*>(&corrupt), 1);
    }

    atlas::asset::AssetValidator validator;
    auto result = validator.Validate(importResult.outputPath);
    assert(!result.valid);
    assert(result.formatValid);
    assert(!result.hashMatch);
    assert(!result.errorMessage.empty());

    std::filesystem::remove_all(dir);
    std::cout << "[PASS] test_asset_validator_validate_corrupt_file" << std::endl;
}

void test_asset_validator_validate_header_bad_magic() {
    const std::string path = "/tmp/atlas_test_bad_magic.atlasb";

    {
        std::ofstream out(path, std::ios::binary);
        atlas::asset::AssetHeader hdr;
        hdr.magic = 0xDEADBEEF; // wrong magic
        out.write(reinterpret_cast<const char*>(&hdr), sizeof(hdr));
    }

    atlas::asset::AssetValidator validator;
    auto result = validator.ValidateHeader(path);
    assert(!result.valid);
    assert(!result.formatValid);
    assert(result.errorMessage.find("magic") != std::string::npos);

    std::remove(path.c_str());
    std::cout << "[PASS] test_asset_validator_validate_header_bad_magic" << std::endl;
}

void test_asset_validator_migration_path() {
    atlas::asset::AssetValidator validator;

    validator.RegisterMigration(1, 2, "Add texture compression field");
    validator.RegisterMigration(2, 3, "Add LOD support");
    validator.RegisterMigration(3, 4, "Add PBR material data");

    assert(validator.MigrationCount() == 3);

    assert(validator.CanMigrate(1, 2));
    assert(validator.CanMigrate(1, 3));
    assert(validator.CanMigrate(1, 4));
    assert(validator.CanMigrate(2, 4));
    assert(!validator.CanMigrate(4, 1));  // can't go backwards
    assert(!validator.CanMigrate(1, 5));  // no path

    auto path = validator.MigrationPath(1, 4);
    assert(path.size() == 3);
    assert(path[0].fromVersion == 1);
    assert(path[0].toVersion == 2);
    assert(path[1].fromVersion == 2);
    assert(path[1].toVersion == 3);
    assert(path[2].fromVersion == 3);
    assert(path[2].toVersion == 4);

    // Same version → empty path
    assert(validator.MigrationPath(2, 2).empty());

    std::cout << "[PASS] test_asset_validator_migration_path" << std::endl;
}

void test_asset_validator_dependencies() {
    atlas::asset::AssetValidator validator;

    validator.AddDependency("material_stone", "texture_stone_diffuse");
    validator.AddDependency("material_stone", "texture_stone_normal");
    validator.AddDependency("prefab_wall", "material_stone");
    validator.AddDependency("prefab_wall", "mesh_wall");

    assert(validator.DependencyCount() == 4);

    auto deps = validator.GetDependencies("material_stone");
    assert(deps.size() == 2);

    auto deps2 = validator.GetDependencies("prefab_wall");
    assert(deps2.size() == 2);

    auto dependents = validator.GetDependents("material_stone");
    assert(dependents.size() == 1);
    assert(dependents[0] == "prefab_wall");

    auto all = validator.AllDependencies();
    assert(all.size() == 4);

    std::cout << "[PASS] test_asset_validator_dependencies" << std::endl;
}

void test_asset_validator_circular_dependency() {
    atlas::asset::AssetValidator validator;

    // Create a cycle: A → B → C → A
    validator.AddDependency("A", "B");
    validator.AddDependency("B", "C");
    validator.AddDependency("C", "A");

    assert(validator.HasCircularDependency("A"));
    assert(validator.HasCircularDependency("B"));
    assert(validator.HasCircularDependency("C"));

    std::cout << "[PASS] test_asset_validator_circular_dependency" << std::endl;
}

void test_asset_validator_no_circular_dependency() {
    atlas::asset::AssetValidator validator;

    validator.AddDependency("A", "B");
    validator.AddDependency("B", "C");
    validator.AddDependency("A", "C");

    assert(!validator.HasCircularDependency("A"));
    assert(!validator.HasCircularDependency("B"));
    assert(!validator.HasCircularDependency("C"));

    std::cout << "[PASS] test_asset_validator_no_circular_dependency" << std::endl;
}

void test_asset_validator_compute_file_hash() {
    const std::string dir = "/tmp/atlas_test_hash";
    std::filesystem::create_directories(dir);

    std::string srcPath = dir + "/data.obj";
    {
        std::ofstream out(srcPath, std::ios::binary);
        std::vector<uint8_t> data = {0x10, 0x20, 0x30};
        out.write(reinterpret_cast<const char*>(data.data()),
                  static_cast<std::streamsize>(data.size()));
    }

    atlas::asset::MeshImporter imp(atlas::asset::ImportFormat::OBJ);
    atlas::asset::ImportOptions opts;
    opts.outputDirectory = dir;
    auto importResult = imp.Import(srcPath, opts);
    assert(importResult.success);

    uint64_t hash = atlas::asset::AssetValidator::ComputeFileHash(importResult.outputPath);
    assert(hash != 0);
    assert(hash == importResult.outputHash);

    std::filesystem::remove_all(dir);
    std::cout << "[PASS] test_asset_validator_compute_file_hash" << std::endl;
}
