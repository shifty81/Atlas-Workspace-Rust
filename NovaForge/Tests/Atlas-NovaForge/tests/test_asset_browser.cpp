#include "../editor/panels/AssetBrowserPanel.h"
#include <iostream>
#include <cassert>
#include <filesystem>
#include <fstream>

using namespace atlas::editor;
using namespace atlas::asset;

void test_asset_browser_empty() {
    AssetRegistry registry;
    AssetBrowserPanel panel(registry);

    panel.Draw();

    assert(panel.Entries().empty());
    assert(panel.Summary() == "Assets: 0");

    std::cout << "[PASS] test_asset_browser_empty" << std::endl;
}

void test_asset_browser_with_assets() {
    // Create temp directory with test assets
    std::string tmpDir = (std::filesystem::temp_directory_path() / "atlas_test_browser").string();
    std::filesystem::create_directories(tmpDir);

    // Create dummy .atlas files
    {
        std::ofstream f(tmpDir + "/weapon.atlas");
        f << "test";
    }
    {
        std::ofstream f(tmpDir + "/terrain.atlas");
        f << "test";
    }
    {
        std::ofstream f(tmpDir + "/character.atlasb");
        f << "test";
    }

    AssetRegistry registry;
    registry.Scan(tmpDir);

    AssetBrowserPanel panel(registry);
    panel.Draw();

    assert(panel.Entries().size() == 3);
    assert(panel.Summary() == "Assets: 3");

    // Cleanup
    std::filesystem::remove_all(tmpDir);

    std::cout << "[PASS] test_asset_browser_with_assets" << std::endl;
}

void test_asset_browser_filter() {
    std::string tmpDir = (std::filesystem::temp_directory_path() / "atlas_test_browser_filter").string();
    std::filesystem::create_directories(tmpDir);

    {
        std::ofstream f(tmpDir + "/weapon.atlas");
        f << "test";
    }
    {
        std::ofstream f(tmpDir + "/terrain.atlas");
        f << "test";
    }

    AssetRegistry registry;
    registry.Scan(tmpDir);

    AssetBrowserPanel panel(registry);

    panel.SetFilter("weapon");
    panel.Draw();

    assert(panel.Entries().size() == 1);
    assert(panel.Entries()[0].id == "weapon");
    assert(panel.Summary() == "Assets: 1 (filter: weapon)");

    // Cleanup
    std::filesystem::remove_all(tmpDir);

    std::cout << "[PASS] test_asset_browser_filter" << std::endl;
}

void test_asset_browser_sort_modes() {
    std::string tmpDir = (std::filesystem::temp_directory_path() / "atlas_test_browser_sort").string();
    std::filesystem::create_directories(tmpDir);

    {
        std::ofstream f(tmpDir + "/beta.atlas");
        f << "test";
    }
    {
        std::ofstream f(tmpDir + "/alpha.atlas");
        f << "test";
    }

    AssetRegistry registry;
    registry.Scan(tmpDir);

    AssetBrowserPanel panel(registry);
    panel.SetSortMode(AssetSortMode::ByName);
    panel.Draw();

    assert(panel.Entries().size() == 2);
    assert(panel.Entries()[0].id == "alpha");
    assert(panel.Entries()[1].id == "beta");

    // Cleanup
    std::filesystem::remove_all(tmpDir);

    std::cout << "[PASS] test_asset_browser_sort_modes" << std::endl;
}

void test_asset_browser_select_asset() {
    AssetRegistry registry;
    AssetBrowserPanel panel(registry);

    assert(panel.SelectedAsset().empty());

    panel.SelectAsset("terrain");
    assert(panel.SelectedAsset() == "terrain");

    std::cout << "[PASS] test_asset_browser_select_asset" << std::endl;
}
