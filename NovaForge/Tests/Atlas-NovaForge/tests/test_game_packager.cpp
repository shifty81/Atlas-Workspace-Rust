#include "../editor/tools/GamePackagerPanel.h"
#include "../engine/production/GamePackager.h"
#include <iostream>
#include <cassert>
#include <string>

using namespace atlas::editor;

void test_game_packager_defaults() {
    GamePackagerPanel panel;

    assert(std::string(panel.Name()) == "Game Packager");

    auto& s = panel.Settings();
    assert(s.target == BuildTarget::Client);
    assert(s.mode == BuildMode::Debug);
    assert(s.singleExe == false);
    assert(s.includeMods == false);
    assert(s.stripEditorData == true);
    assert(s.outputPath == "./build/output");

    std::cout << "[PASS] test_game_packager_defaults" << std::endl;
}

void test_game_packager_summary() {
    GamePackagerPanel panel;

    auto summary = panel.SettingsSummary();
    assert(summary.find("Client") != std::string::npos);
    assert(summary.find("Debug") != std::string::npos);
    assert(summary.find("./build/output") != std::string::npos);

    std::cout << "[PASS] test_game_packager_summary" << std::endl;
}

void test_game_packager_modify_settings() {
    GamePackagerPanel panel;

    panel.Settings().target = BuildTarget::Server;
    panel.Settings().mode = BuildMode::Release;
    panel.Settings().outputPath = "/tmp/dist";

    auto summary = panel.SettingsSummary();
    assert(summary.find("Server") != std::string::npos);
    assert(summary.find("Release") != std::string::npos);
    assert(summary.find("/tmp/dist") != std::string::npos);

    std::cout << "[PASS] test_game_packager_modify_settings" << std::endl;
}

void test_package_config_strip_tools_default() {
    atlas::production::PackageConfig config;
    assert(config.stripTools == true);
    assert(config.stripEditorData == true);

    // Verify stripTools can be toggled independently
    config.stripTools = false;
    assert(config.stripTools == false);
    assert(config.stripEditorData == true);

    std::cout << "[PASS] test_package_config_strip_tools_default" << std::endl;
}
