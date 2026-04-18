// Tests/Workspace/test_phase_i.cpp
//
// Phase I — NovaForge End-to-End Project Load Integration
//
// Validates that the full project loading chain works with the real NovaForge
// project structure: .atlas manifest parsing → ProjectRegistry → NovaForgeAdapter
// initialization → bootstrap → panel registration.
//
// Test categories:
//   I.1 — AtlasProjectFileLoader validates the real NovaForge.atlas manifest
//   I.2 — NovaForgeProjectBootstrap validates the real project directory layout
//   I.3 — ProjectRegistry loads NovaForge via loadProjectFromAtlasFile()
//   I.4 — NovaForgeAdapter initializes and exposes all 6 gameplay panels
//   I.5 — ProjectOpenFlowController.validate() uses real manifest content

#include <catch2/catch_test_macros.hpp>

#include "NF/Workspace/AtlasProjectFileLoader.h"
#include "NF/Workspace/ProjectRegistry.h"
#include "NF/Workspace/ProjectLoadContract.h"
#include "NF/Workspace/AssetCatalog.h"
#include "NF/Workspace/AssetCatalogPopulator.h"
#include "NF/Editor/ProjectOpenFlowController.h"
#include "NovaForge/EditorAdapter/NovaForgeAdapter.h"
#include "NovaForge/EditorAdapter/NovaForgeProjectBootstrap.h"

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

// ── Path helpers ──────────────────────────────────────────────────────────
// The NovaForge project lives at NovaForge/ relative to the repository root.
// Tests are built with ${CMAKE_SOURCE_DIR} available via the
// NF_NOVAFORGE_ATLAS_PATH compile definition (set in CMakeLists.txt).
// Fall back to a compile-time default when the define is absent.

#ifndef NF_NOVAFORGE_ATLAS_PATH
    // Default: locate NovaForge.atlas relative to the expected out-of-source
    // build tree (CMakeSource/../NovaForge/NovaForge.atlas).
    #define NF_NOVAFORGE_ATLAS_PATH ""
#endif

static std::string novaForgeAtlasPath() {
    const std::string defined = NF_NOVAFORGE_ATLAS_PATH;
    if (!defined.empty() && fs::exists(defined)) return defined;
    // Fallback: walk up from current working directory to find NovaForge.atlas.
    // 8 levels covers typical out-of-source build trees (e.g. build/bin/Tests/).
    constexpr int kMaxSearchDepth = 8;
    fs::path dir = fs::current_path();
    for (int depth = 0; depth < kMaxSearchDepth; ++depth) {
        auto candidate = dir / "NovaForge" / "NovaForge.atlas";
        if (fs::exists(candidate)) return candidate.string();
        if (dir.has_parent_path()) dir = dir.parent_path();
        else break;
    }
    return {};
}

static std::string novaForgeRoot() {
    const std::string path = novaForgeAtlasPath();
    if (path.empty()) return {};
    return fs::path(path).parent_path().string();
}

// ═══════════════════════════════════════════════════════════════════════════
//  I.1 — AtlasProjectFileLoader validates the real NovaForge.atlas manifest
// ═══════════════════════════════════════════════════════════════════════════

TEST_CASE("I.1 NovaForge.atlas parses as valid atlas.project.v1",
          "[phase_i][i1][atlas_manifest]") {
    const std::string atlasPath = novaForgeAtlasPath();
    if (atlasPath.empty()) {
        WARN("NovaForge.atlas not found in search paths — skipping manifest test");
        return;
    }

    NF::AtlasProjectFileLoader loader;
    REQUIRE(loader.loadFromFile(atlasPath));

    const auto& m = loader.manifest();
    REQUIRE(m.isValid());
    REQUIRE_FALSE(m.name.empty());
    REQUIRE(m.adapterId == "novaforge");
}

TEST_CASE("I.1 NovaForge.atlas bootstrap produces no fatal errors",
          "[phase_i][i1][atlas_manifest]") {
    const std::string atlasPath = novaForgeAtlasPath();
    if (atlasPath.empty()) {
        WARN("NovaForge.atlas not found — skipping bootstrap test");
        return;
    }

    NF::AtlasProjectFileLoader loader;
    auto result = loader.bootstrap(atlasPath, /*checkPathsOnDisk=*/false);

    REQUIRE(result.success);
    bool hasFatal = false;
    for (const auto& e : result.validationEntries) {
        if (e.severity == NF::ProjectValidationSeverity::Fatal)
            hasFatal = true;
    }
    REQUIRE_FALSE(hasFatal);
}

TEST_CASE("I.1 NovaForge.atlas manifest has expected fields",
          "[phase_i][i1][atlas_manifest]") {
    const std::string atlasPath = novaForgeAtlasPath();
    if (atlasPath.empty()) {
        WARN("NovaForge.atlas not found — skipping field check test");
        return;
    }

    NF::AtlasProjectFileLoader loader;
    REQUIRE(loader.loadFromFile(atlasPath));
    const auto& m = loader.manifest();

    CHECK(m.name       == "NovaForge");
    CHECK(m.adapterId  == "novaforge");
    CHECK_FALSE(m.version.empty());
    CHECK_FALSE(m.contentRoot.empty());
    CHECK_FALSE(m.capabilities.empty());
}

// ═══════════════════════════════════════════════════════════════════════════
//  I.2 — NovaForgeProjectBootstrap validates the real project directory layout
// ═══════════════════════════════════════════════════════════════════════════

TEST_CASE("I.2 NovaForgeProjectBootstrap: real project root is valid",
          "[phase_i][i2][bootstrap]") {
    const std::string root = novaForgeRoot();
    if (root.empty()) {
        WARN("NovaForge project root not found — skipping bootstrap layout test");
        return;
    }

    NF::AssetCatalog catalog;
    NovaForge::NovaForgeProjectBootstrap bs(root, &catalog);
    NF::ProjectLoadContract contract;
    const bool ok = bs.run(contract);

    // The real project should have Content/, Data/, Config/, Schemas/.
    // Bootstrap must not produce fatal entries.
    bool hasFatal = false;
    for (const auto& e : contract.validationEntries) {
        if (e.severity == NF::ProjectValidationSeverity::Fatal)
            hasFatal = true;
    }
    REQUIRE_FALSE(hasFatal);

    // Content/ and Data/ must exist — the bootstrap returns false if either is
    // absent with a Fatal/Error entry, but the directories are in the repo.
    CHECK(fs::is_directory(fs::path(root) / "Content"));
    CHECK(fs::is_directory(fs::path(root) / "Data"));
    CHECK(fs::is_directory(fs::path(root) / "Config"));
    CHECK(fs::is_directory(fs::path(root) / "Schemas"));
}

TEST_CASE("I.2 NovaForgeProjectBootstrap: asset catalog populated from Content/",
          "[phase_i][i2][bootstrap]") {
    const std::string root = novaForgeRoot();
    if (root.empty()) {
        WARN("NovaForge project root not found — skipping catalog test");
        return;
    }

    NF::AssetCatalog catalog;
    NovaForge::NovaForgeProjectBootstrap bs(root, &catalog);
    NF::ProjectLoadContract contract;
    bs.run(contract);

    // The catalog should have at least the built-in assets from Content/.
    // NovaForge/Content/ contains Fonts/builtin_fallback.json and Definitions/DevWorld.json.
    REQUIRE(catalog.count() >= 1u);
}

// ═══════════════════════════════════════════════════════════════════════════
//  I.3 — ProjectRegistry loads NovaForge via loadProjectFromAtlasFile()
// ═══════════════════════════════════════════════════════════════════════════

TEST_CASE("I.3 ProjectRegistry can register and load the novaforge adapter",
          "[phase_i][i3][registry]") {
    const std::string root = novaForgeRoot();
    if (root.empty()) {
        WARN("NovaForge project root not found — skipping registry load test");
        return;
    }

    NF::ProjectRegistry registry;
    registry.initialize();

    // Register the NovaForge factory with the project root captured in the closure.
    // This mirrors the pattern used in NovaForgeEditor/main.cpp.
    registry.registerProject("novaforge", [root]() {
        return std::make_unique<NovaForge::NovaForgeAdapter>(root);
    });

    REQUIRE(registry.isRegistered("novaforge"));
    auto contract = registry.loadProject("novaforge");

    REQUIRE(contract.state == NF::ProjectLoadState::Ready);
    REQUIRE(contract.projectId == "novaforge");
    REQUIRE_FALSE(contract.projectDisplayName.empty());
}

TEST_CASE("I.3 ProjectRegistry.loadProjectFromAtlasFile() succeeds for NovaForge.atlas",
          "[phase_i][i3][registry]") {
    const std::string atlasPath = novaForgeAtlasPath();
    const std::string root      = novaForgeRoot();
    if (atlasPath.empty() || root.empty()) {
        WARN("NovaForge.atlas not found — skipping atlas-file load test");
        return;
    }

    NF::ProjectRegistry registry;
    registry.initialize();

    // Register the factory with the project root derived from the atlas path.
    registry.registerProject("novaforge", [root]() {
        return std::make_unique<NovaForge::NovaForgeAdapter>(root);
    });

    auto contract = registry.loadProjectFromAtlasFile(atlasPath);

    REQUIRE(contract.state == NF::ProjectLoadState::Ready);
    REQUIRE(contract.projectId == "novaforge");
    REQUIRE(contract.isLoaded());
}

TEST_CASE("I.3 Loaded NovaForge contract has content roots and commands",
          "[phase_i][i3][registry]") {
    const std::string root = novaForgeRoot();
    if (root.empty()) {
        WARN("NovaForge project root not found — skipping contract detail test");
        return;
    }

    NF::ProjectRegistry registry;
    registry.initialize();
    registry.registerProject("novaforge", [root]() {
        return std::make_unique<NovaForge::NovaForgeAdapter>(root);
    });

    auto contract = registry.loadProject("novaforge");

    REQUIRE(contract.state == NF::ProjectLoadState::Ready);
    CHECK_FALSE(contract.contentRoots.empty());
    CHECK_FALSE(contract.customCommands.empty());
}

// ═══════════════════════════════════════════════════════════════════════════
//  I.4 — NovaForgeAdapter exposes all 6 gameplay panels after initialization
// ═══════════════════════════════════════════════════════════════════════════

TEST_CASE("I.4 NovaForgeAdapter initializes without errors",
          "[phase_i][i4][adapter]") {
    const std::string root = novaForgeRoot();
    if (root.empty()) {
        WARN("NovaForge project root not found — skipping adapter init test");
        return;
    }

    NovaForge::NovaForgeAdapter adapter(root);
    REQUIRE(adapter.projectId()          == "novaforge");
    REQUIRE(adapter.projectDisplayName() == "NovaForge");
    REQUIRE(adapter.initialize());
}

TEST_CASE("I.4 NovaForgeAdapter provides exactly 6 gameplay panel descriptors",
          "[phase_i][i4][adapter]") {
    const std::string root = novaForgeRoot();
    if (root.empty()) {
        WARN("NovaForge project root not found — skipping panel count test");
        return;
    }

    NovaForge::NovaForgeAdapter adapter(root);
    adapter.initialize();

    const auto panels = adapter.panelDescriptors();
    REQUIRE(panels.size() == 6u);
}

TEST_CASE("I.4 NovaForgeAdapter panel IDs are all present",
          "[phase_i][i4][adapter]") {
    const std::string root = novaForgeRoot();
    if (root.empty()) {
        WARN("NovaForge project root not found — skipping panel ID test");
        return;
    }

    NovaForge::NovaForgeAdapter adapter(root);
    adapter.initialize();

    const auto panels = adapter.panelDescriptors();

    auto hasPanel = [&](const std::string& id) {
        for (const auto& p : panels)
            if (p.id == id) return true;
        return false;
    };

    CHECK(hasPanel("novaforge.economy"));
    CHECK(hasPanel("novaforge.inventory_rules"));
    CHECK(hasPanel("novaforge.shop"));
    CHECK(hasPanel("novaforge.mission_rules"));
    CHECK(hasPanel("novaforge.progression"));
    CHECK(hasPanel("novaforge.character_rules"));
}

TEST_CASE("I.4 NovaForgeAdapter panels have createPanel factories",
          "[phase_i][i4][adapter]") {
    const std::string root = novaForgeRoot();
    if (root.empty()) {
        WARN("NovaForge project root not found — skipping panel factory test");
        return;
    }

    NovaForge::NovaForgeAdapter adapter(root);
    adapter.initialize();

    for (const auto& desc : adapter.panelDescriptors()) {
        INFO("Panel: " << desc.id);
        REQUIRE(desc.createPanel != nullptr);
        auto panel = desc.createPanel();
        REQUIRE(panel != nullptr);
    }
}

TEST_CASE("I.4 NovaForgeAdapter all panels hosted under workspace.project_systems",
          "[phase_i][i4][adapter]") {
    const std::string root = novaForgeRoot();
    if (root.empty()) {
        WARN("NovaForge project root not found — skipping host tool test");
        return;
    }

    NovaForge::NovaForgeAdapter adapter(root);
    adapter.initialize();

    for (const auto& desc : adapter.panelDescriptors()) {
        INFO("Panel: " << desc.id << " hostToolId: " << desc.hostToolId);
        CHECK(desc.hostToolId == "workspace.project_systems");
        CHECK(desc.projectId  == "novaforge");
        CHECK_FALSE(desc.displayName.empty());
        CHECK(desc.enabled);
    }
}

TEST_CASE("I.4 NovaForgeAdapter shutdown cleans up panels",
          "[phase_i][i4][adapter]") {
    const std::string root = novaForgeRoot();
    if (root.empty()) {
        WARN("NovaForge project root not found — skipping shutdown test");
        return;
    }

    NovaForge::NovaForgeAdapter adapter(root);
    adapter.initialize();
    REQUIRE_FALSE(adapter.panelDescriptors().empty());
    adapter.shutdown();
    REQUIRE(adapter.panelDescriptors().empty());
}

// ═══════════════════════════════════════════════════════════════════════════
//  I.5 — ProjectOpenFlowController.validate() with a real .atlas file
// ═══════════════════════════════════════════════════════════════════════════

TEST_CASE("I.5 ProjectOpenFlowController.validate() succeeds for real NovaForge.atlas",
          "[phase_i][i5][flow]") {
    const std::string atlasPath = novaForgeAtlasPath();
    if (atlasPath.empty()) {
        WARN("NovaForge.atlas not found — skipping flow validation test");
        return;
    }

    NF::ProjectOpenFlowController ctrl;
    ctrl.beginFileOpen();
    ctrl.selectFile(atlasPath);
    auto result = ctrl.validate();

    REQUIRE(result.success);
    REQUIRE_FALSE(result.hasErrors());
    REQUIRE_FALSE(result.projectName.empty());
}

TEST_CASE("I.5 ProjectOpenFlowController.validate() extracts project name from manifest",
          "[phase_i][i5][flow]") {
    const std::string atlasPath = novaForgeAtlasPath();
    if (atlasPath.empty()) {
        WARN("NovaForge.atlas not found — skipping name extraction test");
        return;
    }

    NF::ProjectOpenFlowController ctrl;
    ctrl.beginFileOpen();
    ctrl.selectFile(atlasPath);
    auto result = ctrl.validate();

    // The manifest has name = "NovaForge"; the controller should use that.
    REQUIRE(result.success);
    CHECK(result.projectName == "NovaForge");
}

TEST_CASE("I.5 ProjectOpenFlowController.validate() with real file reaches Open state via confirmOpen()",
          "[phase_i][i5][flow]") {
    const std::string atlasPath = novaForgeAtlasPath();
    if (atlasPath.empty()) {
        WARN("NovaForge.atlas not found — skipping confirmOpen test");
        return;
    }

    NF::ProjectOpenFlowController ctrl;
    bool opened = false;
    ctrl.setOnProjectOpened([&](const std::string&) { opened = true; });
    ctrl.beginFileOpen();
    ctrl.selectFile(atlasPath);
    auto result = ctrl.validate();
    REQUIRE(result.success);

    REQUIRE(ctrl.confirmOpen());
    REQUIRE(ctrl.state() == NF::ProjectOpenFlowState::Open);
    REQUIRE(opened);
}

// ── Regression: non-existent paths still succeed (extension-only validation)

TEST_CASE("I.5 ProjectOpenFlowController.validate() still succeeds for non-existent .atlas path",
          "[phase_i][i5][flow][regression]") {
    NF::ProjectOpenFlowController ctrl;
    ctrl.beginFileOpen();
    ctrl.selectFile("/nonexistent/path/MyGame.atlas");
    auto result = ctrl.validate();

    // Non-existent file → extension OK, file-not-found warning → success = true.
    REQUIRE(result.success);
    REQUIRE_FALSE(result.hasErrors());
    REQUIRE(result.projectName == "MyGame");
    REQUIRE(result.hasWarnings()); // file-not-found warning
}

TEST_CASE("I.5 ProjectOpenFlowController.validate() fails for malformed .atlas file",
          "[phase_i][i5][flow]") {
    // Write a malformed atlas file to a temp path
    const fs::path tmpDir   = fs::temp_directory_path() / "test_phase_i_malformed";
    const fs::path tmpAtlas = tmpDir / "Malformed.atlas";
    fs::create_directories(tmpDir);
    {
        std::ofstream f(tmpAtlas);
        f << "{ this is not valid json }";
    }

    NF::ProjectOpenFlowController ctrl;
    ctrl.beginFileOpen();
    ctrl.selectFile(tmpAtlas.string());
    auto result = ctrl.validate();

    // File exists but cannot be parsed → error
    REQUIRE_FALSE(result.success);
    REQUIRE(result.hasErrors());

    fs::remove_all(tmpDir);
}
