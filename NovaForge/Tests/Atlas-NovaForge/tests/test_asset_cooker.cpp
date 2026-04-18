#include "../engine/production/AssetCooker.h"
#include <iostream>
#include <cassert>
#include <filesystem>
#include <fstream>

using namespace atlas::production;

void test_cooker_default_settings() {
    AssetCooker cooker;

    assert(cooker.OutputDir() == "./build/cooked");
    assert(cooker.StripEditorData() == true);
    assert(cooker.CookLog().empty());

    std::cout << "[PASS] test_cooker_default_settings" << std::endl;
}

void test_cooker_set_output_dir() {
    AssetCooker cooker;

    cooker.SetOutputDir("/tmp/cooker_output");
    assert(cooker.OutputDir() == "/tmp/cooker_output");

    std::cout << "[PASS] test_cooker_set_output_dir" << std::endl;
}

void test_cooker_cook_single_asset() {
    std::string tmpDir = (std::filesystem::temp_directory_path() / "atlas_cooker_test").string();
    std::string outDir = (std::filesystem::temp_directory_path() / "atlas_cooker_output").string();
    std::filesystem::create_directories(tmpDir);
    std::filesystem::remove_all(outDir);

    // Create a test asset
    {
        std::ofstream f(tmpDir + "/test_weapon.atlas");
        f << "weapon_data_content";
    }

    AssetCooker cooker;
    cooker.SetOutputDir(outDir);

    CookResult result = cooker.CookAsset("test_weapon", tmpDir + "/test_weapon.atlas");
    assert(result == CookResult::Success);

    // Verify output file was created
    assert(std::filesystem::exists(outDir + "/test_weapon.atlasb"));

    // Verify cook log
    assert(cooker.CookLog().size() == 1);
    assert(cooker.CookLog()[0].sourceId == "test_weapon");
    assert(cooker.CookLog()[0].result == CookResult::Success);

    // Cleanup
    std::filesystem::remove_all(tmpDir);
    std::filesystem::remove_all(outDir);

    std::cout << "[PASS] test_cooker_cook_single_asset" << std::endl;
}

void test_cooker_source_not_found() {
    AssetCooker cooker;

    CookResult result = cooker.CookAsset("missing", "/nonexistent/path.atlas");
    assert(result == CookResult::SourceNotFound);
    assert(cooker.CookLog().size() == 1);
    assert(cooker.CookLog()[0].result == CookResult::SourceNotFound);

    std::cout << "[PASS] test_cooker_source_not_found" << std::endl;
}

void test_cooker_cook_all() {
    std::string tmpDir = (std::filesystem::temp_directory_path() / "atlas_cooker_all_test").string();
    std::string outDir = (std::filesystem::temp_directory_path() / "atlas_cooker_all_output").string();
    std::filesystem::create_directories(tmpDir);
    std::filesystem::remove_all(outDir);

    {
        std::ofstream f(tmpDir + "/asset1.atlas");
        f << "data1";
    }
    {
        std::ofstream f(tmpDir + "/asset2.atlas");
        f << "data2";
    }
    {
        std::ofstream f(tmpDir + "/readme.txt");
        f << "not an asset";
    }

    AssetCooker cooker;
    cooker.SetOutputDir(outDir);

    CookStats stats = cooker.CookAll(tmpDir);
    assert(stats.totalAssets == 2);
    assert(stats.cookedAssets == 2);
    assert(stats.failedAssets == 0);

    // Cleanup
    std::filesystem::remove_all(tmpDir);
    std::filesystem::remove_all(outDir);

    std::cout << "[PASS] test_cooker_cook_all" << std::endl;
}

void test_cooker_strip_editor_data() {
    AssetCooker cooker;

    assert(cooker.StripEditorData() == true);
    cooker.SetStripEditorData(false);
    assert(cooker.StripEditorData() == false);

    std::cout << "[PASS] test_cooker_strip_editor_data" << std::endl;
}

void test_cooker_clear_log() {
    AssetCooker cooker;

    cooker.CookAsset("missing", "/nonexistent.atlas");
    assert(cooker.CookLog().size() == 1);

    cooker.ClearLog();
    assert(cooker.CookLog().empty());

    std::cout << "[PASS] test_cooker_clear_log" << std::endl;
}

void test_cooker_progress_callback() {
    std::string tmpDir = (std::filesystem::temp_directory_path() / "atlas_cooker_progress_test").string();
    std::string outDir = (std::filesystem::temp_directory_path() / "atlas_cooker_progress_output").string();
    std::filesystem::create_directories(tmpDir);
    std::filesystem::remove_all(outDir);

    {
        std::ofstream f(tmpDir + "/item.atlas");
        f << "data";
    }

    AssetCooker cooker;
    cooker.SetOutputDir(outDir);

    int callbackCount = 0;
    cooker.SetProgressCallback([&](const std::string&, uint32_t, uint32_t) {
        callbackCount++;
    });

    cooker.CookAll(tmpDir);
    assert(callbackCount == 1);

    // Cleanup
    std::filesystem::remove_all(tmpDir);
    std::filesystem::remove_all(outDir);

    std::cout << "[PASS] test_cooker_progress_callback" << std::endl;
}
