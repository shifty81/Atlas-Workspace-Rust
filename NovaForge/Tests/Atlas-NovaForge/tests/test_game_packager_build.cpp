#include <iostream>
#include <cassert>
#include <filesystem>
#include <fstream>

#include "../engine/production/GamePackager.h"

using namespace atlas::production;

void test_game_packager_validate_empty_source() {
    GamePackager packager;
    PackageConfig config;
    config.sourceDir = "";
    std::string error;
    assert(!packager.Validate(config, error));
    assert(!error.empty());
    std::cout << "[PASS] test_game_packager_validate_empty_source" << std::endl;
}

void test_game_packager_validate_nonexistent_source() {
    GamePackager packager;
    PackageConfig config;
    config.sourceDir = "/tmp/atlas_nonexistent_pkg_dir_xyz";
    std::string error;
    assert(!packager.Validate(config, error));
    assert(error.find("does not exist") != std::string::npos);
    std::cout << "[PASS] test_game_packager_validate_nonexistent_source" << std::endl;
}

void test_game_packager_validate_valid() {
    const std::string dir = "/tmp/atlas_pkg_validate_test";
    std::filesystem::create_directories(dir);

    GamePackager packager;
    PackageConfig config;
    config.sourceDir = dir;
    config.outputDir = dir + "/output";
    std::string error;
    assert(packager.Validate(config, error));
    assert(error.empty());

    std::filesystem::remove_all(dir);
    std::cout << "[PASS] test_game_packager_validate_valid" << std::endl;
}

void test_game_packager_package_empty_dir() {
    const std::string dir = "/tmp/atlas_pkg_empty_test";
    std::filesystem::create_directories(dir);

    GamePackager packager;
    PackageConfig config;
    config.sourceDir = dir;
    config.outputDir = dir + "/output";

    auto report = packager.Package(config);
    assert(report.result == PackageResult::Success);
    assert(report.cookStats.totalAssets == 0);
    assert(!report.manifestPath.empty());

    std::filesystem::remove_all(dir);
    std::cout << "[PASS] test_game_packager_package_empty_dir" << std::endl;
}

void test_game_packager_package_with_assets() {
    const std::string dir = "/tmp/atlas_pkg_assets_test";
    std::filesystem::create_directories(dir + "/src");

    // Create some fake atlas asset files
    {
        std::ofstream out(dir + "/src/mesh.atlas", std::ios::binary);
        out << "test mesh data";
    }
    {
        std::ofstream out(dir + "/src/texture.atlas", std::ios::binary);
        out << "test texture data";
    }

    GamePackager packager;
    PackageConfig config;
    config.sourceDir = dir + "/src";
    config.outputDir = dir + "/output";
    config.target = PackageTarget::Client;
    config.profile = ProfileType::Release;
    config.stripEditorData = true;

    auto report = packager.Package(config);
    assert(report.result == PackageResult::Success);
    assert(report.cookStats.totalAssets == 2);
    assert(report.cookStats.cookedAssets == 2);
    assert(!report.manifestPath.empty());
    assert(std::filesystem::exists(report.manifestPath));

    std::filesystem::remove_all(dir);
    std::cout << "[PASS] test_game_packager_package_with_assets" << std::endl;
}

void test_game_packager_progress_callback() {
    const std::string dir = "/tmp/atlas_pkg_progress_test";
    std::filesystem::create_directories(dir);

    GamePackager packager;
    PackageConfig config;
    config.sourceDir = dir;
    config.outputDir = dir + "/output";

    std::vector<std::string> stages;
    packager.SetProgressCallback(
        [&stages](const std::string& stage, float progress) {
            stages.push_back(stage);
        });

    packager.Package(config);

    // Should have at least Validate, Cook, Bundle, Emit stages
    assert(stages.size() >= 4);
    bool hasValidate = false, hasCook = false, hasBundle = false, hasEmit = false;
    for (const auto& s : stages) {
        if (s == "Validate") hasValidate = true;
        if (s == "Cook") hasCook = true;
        if (s == "Bundle") hasBundle = true;
        if (s == "Emit") hasEmit = true;
    }
    assert(hasValidate);
    assert(hasCook);
    assert(hasBundle);
    assert(hasEmit);

    std::filesystem::remove_all(dir);
    std::cout << "[PASS] test_game_packager_progress_callback" << std::endl;
}

void test_game_packager_last_report() {
    const std::string dir = "/tmp/atlas_pkg_lastreport_test";
    std::filesystem::create_directories(dir);

    GamePackager packager;
    PackageConfig config;
    config.sourceDir = dir;
    config.outputDir = dir + "/output";

    packager.Package(config);

    const auto& report = packager.LastReport();
    assert(report.result == PackageResult::Success);
    assert(report.outputDir == config.outputDir);

    std::filesystem::remove_all(dir);
    std::cout << "[PASS] test_game_packager_last_report" << std::endl;
}

void test_game_packager_validation_failed() {
    GamePackager packager;
    PackageConfig config;
    config.sourceDir = "/nonexistent_xyz_abc";
    config.outputDir = "/tmp/atlas_pkg_valfail";

    auto report = packager.Package(config);
    assert(report.result == PackageResult::ValidationFailed);
    assert(!report.errorMessage.empty());

    std::cout << "[PASS] test_game_packager_validation_failed" << std::endl;
}
