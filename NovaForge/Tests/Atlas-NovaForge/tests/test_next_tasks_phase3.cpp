#include <iostream>
#include <cassert>
#include <cstdio>
#include <vector>
#include <string>
#include <fstream>
#include <filesystem>

#include "../editor/ui/LauncherScreen.h"
#include "../engine/production/CertifiedBuild.h"
#include "../engine/ui/GUIDSLParser.h"
#include "../editor/ui/DefaultEditorLayout.h"

// ============================================================
// Task 1: LauncherScreen .atlas Descriptor Parsing
// ============================================================

void test_launcher_parses_project_name_from_descriptor() {
    const auto dirPath = std::filesystem::temp_directory_path() / "atlas_test_parse_name";
    std::filesystem::remove_all(dirPath);
    struct CleanupGuard {
        std::filesystem::path path;
        ~CleanupGuard() { std::filesystem::remove_all(path); }
    } cleanup{dirPath};

    std::filesystem::create_directories(dirPath / "myproject");

    {
        std::ofstream out((dirPath / "myproject" / "myproject.atlas").string());
        out << R"({
  "schema": "atlas.project.v1",
  "name": "My Cool Project",
  "version": "2.3.4",
  "description": "A cool project"
})";
    }

    atlas::editor::LauncherScreen launcher;
    launcher.ScanProjects(dirPath.string());

    assert(launcher.Projects().size() == 1);
    assert(launcher.Projects()[0].name == "My Cool Project");
    assert(launcher.Projects()[0].engineVersion == "2.3.4");
    assert(launcher.Projects()[0].description == "A cool project");

    std::cout << "[PASS] test_launcher_parses_project_name_from_descriptor" << std::endl;
}

void test_launcher_parses_last_opened() {
    const auto dirPath = std::filesystem::temp_directory_path() / "atlas_test_parse_lastopened";
    std::filesystem::remove_all(dirPath);
    struct CleanupGuard {
        std::filesystem::path path;
        ~CleanupGuard() { std::filesystem::remove_all(path); }
    } cleanup{dirPath};

    std::filesystem::create_directories(dirPath / "proj");

    {
        std::ofstream out((dirPath / "proj" / "proj.atlas").string());
        out << R"({
  "schema": "atlas.project.v1",
  "name": "TestProj",
  "version": "1.0.0",
  "lastOpened": "2026-02-15T12:00:00Z"
})";
    }

    atlas::editor::LauncherScreen launcher;
    launcher.ScanProjects(dirPath.string());

    assert(launcher.Projects().size() == 1);
    assert(launcher.Projects()[0].lastOpened == "2026-02-15T12:00:00Z");

    std::cout << "[PASS] test_launcher_parses_last_opened" << std::endl;
}

void test_launcher_fallback_on_minimal_descriptor() {
    const auto dirPath = std::filesystem::temp_directory_path() / "atlas_test_parse_minimal";
    std::filesystem::remove_all(dirPath);
    struct CleanupGuard {
        std::filesystem::path path;
        ~CleanupGuard() { std::filesystem::remove_all(path); }
    } cleanup{dirPath};

    std::filesystem::create_directories(dirPath / "bare");

    {
        // Descriptor with no name/version fields
        std::ofstream out((dirPath / "bare" / "bare.atlas").string());
        out << "{}";
    }

    atlas::editor::LauncherScreen launcher;
    launcher.ScanProjects(dirPath.string());

    assert(launcher.Projects().size() == 1);
    // Should fall back to directory name and default version
    assert(launcher.Projects()[0].name == "bare");
    assert(launcher.Projects()[0].engineVersion == "0.1.0");

    std::cout << "[PASS] test_launcher_fallback_on_minimal_descriptor" << std::endl;
}

void test_launcher_project_description_field() {
    const auto dirPath = std::filesystem::temp_directory_path() / "atlas_test_parse_desc";
    std::filesystem::remove_all(dirPath);
    struct CleanupGuard {
        std::filesystem::path path;
        ~CleanupGuard() { std::filesystem::remove_all(path); }
    } cleanup{dirPath};

    std::filesystem::create_directories(dirPath / "game");

    {
        std::ofstream out((dirPath / "game" / "game.atlas").string());
        out << R"({
  "name": "MyGame",
  "version": "0.5.0",
  "description": "An awesome RPG"
})";
    }

    atlas::editor::LauncherScreen launcher;
    launcher.ScanProjects(dirPath.string());

    assert(launcher.Projects().size() == 1);
    assert(launcher.Projects()[0].description == "An awesome RPG");

    std::cout << "[PASS] test_launcher_project_description_field" << std::endl;
}

// ============================================================
// Task 2: CertifiedBuild Verification Steps
// ============================================================

void test_cert_register_step() {
    atlas::production::CertifiedBuildSystem sys;
    assert(sys.StepCount() == 0);

    atlas::production::VerificationStep step;
    step.name = "hash_check";
    step.type = "hash";
    step.execute = []() {
        atlas::production::VerificationArtifact a;
        a.name = "hash_check";
        a.type = "hash";
        a.passed = true;
        return a;
    };

    sys.RegisterStep(step);
    assert(sys.StepCount() == 1);

    std::cout << "[PASS] test_cert_register_step" << std::endl;
}

void test_cert_run_all_steps() {
    atlas::production::CertifiedBuildSystem sys;
    sys.SetLevel(atlas::production::CertificationLevel::Standard);

    atlas::production::VerificationStep step1;
    step1.name = "step_a";
    step1.type = "test";
    step1.execute = []() {
        atlas::production::VerificationArtifact a;
        a.name = "step_a";
        a.type = "test";
        a.passed = true;
        a.details = "All good";
        return a;
    };

    atlas::production::VerificationStep step2;
    step2.name = "step_b";
    step2.type = "hash";
    step2.execute = []() {
        atlas::production::VerificationArtifact a;
        a.name = "step_b";
        a.type = "hash";
        a.passed = true;
        return a;
    };

    sys.RegisterStep(step1);
    sys.RegisterStep(step2);

    auto result = sys.RunAllSteps();
    assert(result.passed);
    assert(result.artifacts.size() == 2);
    assert(sys.ArtifactCount() == 2);

    std::cout << "[PASS] test_cert_run_all_steps" << std::endl;
}

void test_cert_run_all_steps_with_failure() {
    atlas::production::CertifiedBuildSystem sys;
    sys.SetLevel(atlas::production::CertificationLevel::Full);

    atlas::production::VerificationStep pass_step;
    pass_step.name = "good";
    pass_step.type = "test";
    pass_step.execute = []() {
        atlas::production::VerificationArtifact a;
        a.passed = true;
        return a;
    };

    atlas::production::VerificationStep fail_step;
    fail_step.name = "bad";
    fail_step.type = "hash";
    fail_step.execute = []() {
        atlas::production::VerificationArtifact a;
        a.passed = false;
        a.details = "Hash mismatch";
        return a;
    };

    sys.RegisterStep(pass_step);
    sys.RegisterStep(fail_step);

    auto result = sys.RunAllSteps();
    assert(!result.passed);
    assert(result.artifacts.size() == 2);

    std::cout << "[PASS] test_cert_run_all_steps_with_failure" << std::endl;
}

void test_cert_verify_file_hash() {
    const std::string path = "/tmp/atlas_test_cert_hash_file.bin";
    {
        std::ofstream out(path, std::ios::binary);
        std::vector<uint8_t> data = {0x41, 0x54, 0x4C, 0x41, 0x53};
        out.write(reinterpret_cast<const char*>(data.data()),
                  static_cast<std::streamsize>(data.size()));
    }

    // First, get the hash
    auto artifact1 = atlas::production::CertifiedBuildSystem::VerifyFileHash(path, 0);
    assert(!artifact1.passed); // 0 won't match
    assert(artifact1.hash != 0);

    // Now verify with correct hash
    auto artifact2 = atlas::production::CertifiedBuildSystem::VerifyFileHash(
        path, artifact1.hash);
    assert(artifact2.passed);
    assert(artifact2.hash == artifact1.hash);
    assert(artifact2.details.find("matches") != std::string::npos);

    std::remove(path.c_str());
    std::cout << "[PASS] test_cert_verify_file_hash" << std::endl;
}

void test_cert_verify_file_hash_not_found() {
    auto artifact = atlas::production::CertifiedBuildSystem::VerifyFileHash(
        "/nonexistent/file.bin", 12345);
    assert(!artifact.passed);
    assert(artifact.hash == 0);

    std::cout << "[PASS] test_cert_verify_file_hash_not_found" << std::endl;
}

void test_cert_verify_test_results() {
    auto pass = atlas::production::CertifiedBuildSystem::VerifyTestResults(
        "unit_tests", 100, 0);
    assert(pass.passed);
    assert(pass.details.find("100 passed") != std::string::npos);
    assert(pass.details.find("0 failed") != std::string::npos);

    auto fail = atlas::production::CertifiedBuildSystem::VerifyTestResults(
        "integration_tests", 95, 5);
    assert(!fail.passed);
    assert(fail.details.find("5 failed") != std::string::npos);

    std::cout << "[PASS] test_cert_verify_test_results" << std::endl;
}

void test_cert_verify_contract_scan() {
    auto pass = atlas::production::CertifiedBuildSystem::VerifyContractScan(true, 0);
    assert(pass.passed);
    assert(pass.name == "contract_scan");
    assert(pass.details.find("No contract violations") != std::string::npos);

    auto fail = atlas::production::CertifiedBuildSystem::VerifyContractScan(false, 3);
    assert(!fail.passed);
    assert(fail.details.find("3 violation(s)") != std::string::npos);

    std::cout << "[PASS] test_cert_verify_contract_scan" << std::endl;
}

void test_cert_clear_clears_steps() {
    atlas::production::CertifiedBuildSystem sys;

    atlas::production::VerificationStep step;
    step.name = "test";
    step.type = "test";
    step.execute = []() {
        atlas::production::VerificationArtifact a;
        a.passed = true;
        return a;
    };
    sys.RegisterStep(step);
    assert(sys.StepCount() == 1);

    sys.Clear();
    assert(sys.StepCount() == 0);
    assert(sys.ArtifactCount() == 0);

    std::cout << "[PASS] test_cert_clear_clears_steps" << std::endl;
}

// ============================================================
// Task 3: DSL Layout Parsing for Editor
// ============================================================

void test_dsl_parses_default_editor_layout() {
    atlas::ui::GUIDSLParser parser;
    auto result = parser.Parse(atlas::editor::DefaultEditorDSL());

    assert(result.success);
    assert(result.root != nullptr);
    assert(result.root->type == atlas::ui::DSLNodeType::Layout);
    assert(result.root->name == "DefaultEditor");
    assert(!result.root->children.empty());

    std::cout << "[PASS] test_dsl_parses_default_editor_layout" << std::endl;
}

void test_dsl_layout_has_panels() {
    atlas::ui::GUIDSLParser parser;
    auto result = parser.Parse(atlas::editor::DefaultEditorDSL());
    assert(result.success);

    // Recursively find all Panel nodes
    std::vector<std::string> panelNames;
    std::function<void(const atlas::ui::DSLNode&)> collectPanels =
        [&](const atlas::ui::DSLNode& node) {
            if (node.type == atlas::ui::DSLNodeType::Panel)
                panelNames.push_back(node.name);
            for (auto& child : node.children)
                collectPanels(*child);
        };
    collectPanels(*result.root);

    // Should find at least AssetBrowser, ShipEditor, Console
    bool hasAssetBrowser = false, hasShipEditor = false;
    bool hasConsole = false;
    for (auto& name : panelNames) {
        if (name == "AssetBrowser") hasAssetBrowser = true;
        if (name == "ShipEditor") hasShipEditor = true;
        if (name == "Console") hasConsole = true;
    }
    assert(hasAssetBrowser);
    assert(hasShipEditor);
    assert(hasConsole);

    std::cout << "[PASS] test_dsl_layout_has_panels" << std::endl;
}

void test_dsl_layout_has_dock_and_split() {
    atlas::ui::GUIDSLParser parser;
    auto result = parser.Parse(atlas::editor::DefaultEditorDSL());
    assert(result.success);

    bool hasSplit = false;
    std::function<void(const atlas::ui::DSLNode&)> inspect =
        [&](const atlas::ui::DSLNode& node) {
            if (node.type == atlas::ui::DSLNodeType::Split) hasSplit = true;
            for (auto& child : node.children)
                inspect(*child);
        };
    inspect(*result.root);

    assert(hasSplit);

    std::cout << "[PASS] test_dsl_layout_has_dock_and_split" << std::endl;
}
