#include "../editor/ui/LauncherScreen.h"
#include <iostream>
#include <cassert>
#include <filesystem>
#include <fstream>

using namespace atlas::editor;

void test_launcher_initial_state() {
    LauncherScreen launcher;
    assert(launcher.Projects().empty());
    assert(launcher.SelectedIndex() == SIZE_MAX);
    assert(launcher.SelectedProject() == nullptr);
    assert(!launcher.IsProjectChosen());
    assert(!launcher.IsNewProjectRequested());
    assert(!launcher.IsQuitRequested());
    std::cout << "[PASS] test_launcher_initial_state" << std::endl;
}

void test_launcher_scan_nonexistent() {
    LauncherScreen launcher;
    launcher.ScanProjects("/nonexistent_directory_12345");
    assert(launcher.Projects().empty());
    std::cout << "[PASS] test_launcher_scan_nonexistent" << std::endl;
}

void test_launcher_scan_atlas_descriptors() {
    const auto dirPath = std::filesystem::temp_directory_path() / "atlas_test_launcher_projects";
    const std::string dir = dirPath.string();
    std::filesystem::remove_all(dirPath);
    struct CleanupGuard {
        std::filesystem::path path;
        ~CleanupGuard() { std::filesystem::remove_all(path); }
    } cleanup{dirPath};
    std::filesystem::create_directories(dir + "/alpha");
    std::filesystem::create_directories(dir + "/beta");
    std::filesystem::create_directories(dir + "/ignored");

    std::ofstream(dir + "/alpha/alpha.atlas") << "{}";
    std::ofstream(dir + "/beta/project.atlas") << "{}";
    std::ofstream(dir + "/ignored/readme.txt") << "not a project";

    LauncherScreen launcher;
    launcher.ScanProjects(dir);
    assert(launcher.Projects().size() == 2);

    bool foundAlpha = false;
    bool foundBeta = false;
    for (const auto& project : launcher.Projects()) {
        if (project.name == "alpha") foundAlpha = true;
        if (project.name == "beta") foundBeta = true;
    }
    assert(foundAlpha && foundBeta);

    std::cout << "[PASS] test_launcher_scan_atlas_descriptors" << std::endl;
}

void test_launcher_select_invalid() {
    LauncherScreen launcher;
    launcher.SelectProject(0);
    assert(launcher.SelectedIndex() == SIZE_MAX);
    assert(launcher.SelectedProject() == nullptr);
    std::cout << "[PASS] test_launcher_select_invalid" << std::endl;
}

void test_launcher_confirm_no_selection() {
    LauncherScreen launcher;
    launcher.ConfirmSelection();
    assert(!launcher.IsProjectChosen());
    std::cout << "[PASS] test_launcher_confirm_no_selection" << std::endl;
}

void test_launcher_request_new_project() {
    LauncherScreen launcher;
    assert(!launcher.IsNewProjectRequested());
    launcher.RequestNewProject();
    assert(launcher.IsNewProjectRequested());
    std::cout << "[PASS] test_launcher_request_new_project" << std::endl;
}

void test_launcher_request_quit() {
    LauncherScreen launcher;
    assert(!launcher.IsQuitRequested());
    launcher.RequestQuit();
    assert(launcher.IsQuitRequested());
    std::cout << "[PASS] test_launcher_request_quit" << std::endl;
}
