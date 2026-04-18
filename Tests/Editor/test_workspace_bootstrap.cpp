// Tests/Editor/test_workspace_bootstrap.cpp
// Phase 3 bootstrap integration tests: verify that WorkspaceShell::initialize()
// registers and initializes the full primary tool roster when wired through
// CoreToolRoster.
//
// These tests are the integration gate for Phase 3 "Wire all primary tools into
// WorkspaceShell at bootstrap" (see Docs/Roadmap/00_MASTER_ROADMAP.md).

#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/CoreToolRoster.h"
#include "NF/Editor/IDETool.h"

// CoreToolRoster.h includes WorkspaceShell.h and all 9 primary tool headers.

using namespace NF;

// ─────────────────────────────────────────────────────────────────
// Helpers
// ─────────────────────────────────────────────────────────────────

static bool hasToolId(const WorkspaceShell& shell, const char* id) {
    return shell.toolRegistry().isRegistered(std::string(id));
}



// ─────────────────────────────────────────────────────────────────
// Full roster presence
// ─────────────────────────────────────────────────────────────────

TEST_CASE("Bootstrap: WorkspaceShell registers 9 core tools on initialize", "[bootstrap][phase3]") {
    WorkspaceShell shell;
    registerCoreTools(shell);
    REQUIRE(shell.initialize());
    CHECK(shell.toolRegistry().count() == 9);
    shell.shutdown();
}

TEST_CASE("Bootstrap: SceneEditorTool is registered at bootstrap", "[bootstrap][phase3]") {
    WorkspaceShell shell;
    registerCoreTools(shell);
    shell.initialize();
    CHECK(hasToolId(shell, SceneEditorTool::kToolId));
    shell.shutdown();
}

TEST_CASE("Bootstrap: AssetEditorTool is registered at bootstrap", "[bootstrap][phase3]") {
    WorkspaceShell shell;
    registerCoreTools(shell);
    shell.initialize();
    CHECK(hasToolId(shell, AssetEditorTool::kToolId));
    shell.shutdown();
}

TEST_CASE("Bootstrap: MaterialEditorTool is registered at bootstrap", "[bootstrap][phase3]") {
    WorkspaceShell shell;
    registerCoreTools(shell);
    shell.initialize();
    CHECK(hasToolId(shell, MaterialEditorTool::kToolId));
    shell.shutdown();
}

TEST_CASE("Bootstrap: AnimationEditorTool is registered at bootstrap", "[bootstrap][phase3]") {
    WorkspaceShell shell;
    registerCoreTools(shell);
    shell.initialize();
    CHECK(hasToolId(shell, AnimationEditorTool::kToolId));
    shell.shutdown();
}

TEST_CASE("Bootstrap: DataEditorTool is registered at bootstrap", "[bootstrap][phase3]") {
    WorkspaceShell shell;
    registerCoreTools(shell);
    shell.initialize();
    CHECK(hasToolId(shell, DataEditorTool::kToolId));
    shell.shutdown();
}

TEST_CASE("Bootstrap: VisualLogicEditorTool is registered at bootstrap", "[bootstrap][phase3]") {
    WorkspaceShell shell;
    registerCoreTools(shell);
    shell.initialize();
    CHECK(hasToolId(shell, VisualLogicEditorTool::kToolId));
    shell.shutdown();
}

TEST_CASE("Bootstrap: BuildTool is registered at bootstrap", "[bootstrap][phase3]") {
    WorkspaceShell shell;
    registerCoreTools(shell);
    shell.initialize();
    CHECK(hasToolId(shell, BuildTool::kToolId));
    shell.shutdown();
}

TEST_CASE("Bootstrap: AtlasAITool is registered at bootstrap", "[bootstrap][phase3]") {
    WorkspaceShell shell;
    registerCoreTools(shell);
    shell.initialize();
    CHECK(hasToolId(shell, AtlasAITool::kToolId));
    shell.shutdown();
}

TEST_CASE("Bootstrap: IDETool is registered at bootstrap", "[bootstrap][phase3]") {
    WorkspaceShell shell;
    registerCoreTools(shell);
    shell.initialize();
    CHECK(hasToolId(shell, IDETool::kToolId));
    shell.shutdown();
}

// ─────────────────────────────────────────────────────────────────
// All core tools are initialized (Ready state) after shell.initialize()
// ─────────────────────────────────────────────────────────────────

TEST_CASE("Bootstrap: all 9 core tools are in Ready state after initialize", "[bootstrap][phase3]") {
    WorkspaceShell shell;
    registerCoreTools(shell);
    shell.initialize();

    auto check = [&](const char* id) {
        const auto* tool = shell.toolRegistry().find(std::string(id));
        REQUIRE(tool != nullptr);
        CHECK(tool->state() == HostedToolState::Ready);
    };

    check(SceneEditorTool::kToolId);
    check(AssetEditorTool::kToolId);
    check(MaterialEditorTool::kToolId);
    check(AnimationEditorTool::kToolId);
    check(DataEditorTool::kToolId);
    check(VisualLogicEditorTool::kToolId);
    check(BuildTool::kToolId);
    check(AtlasAITool::kToolId);
    check(IDETool::kToolId);

    shell.shutdown();
}

// ─────────────────────────────────────────────────────────────────
// All core tools are Unloaded after shell.shutdown()
// ─────────────────────────────────────────────────────────────────

TEST_CASE("Bootstrap: all 9 core tools are Unloaded after shutdown", "[bootstrap][phase3]") {
    WorkspaceShell shell;
    registerCoreTools(shell);
    shell.initialize();
    shell.shutdown();

    // After shutdown the tools are still in the registry but Unloaded.
    auto check = [&](const char* id) {
        const auto* tool = shell.toolRegistry().find(std::string(id));
        REQUIRE(tool != nullptr);
        CHECK(tool->state() == HostedToolState::Unloaded);
    };

    check(SceneEditorTool::kToolId);
    check(AssetEditorTool::kToolId);
    check(MaterialEditorTool::kToolId);
    check(AnimationEditorTool::kToolId);
    check(DataEditorTool::kToolId);
    check(VisualLogicEditorTool::kToolId);
    check(BuildTool::kToolId);
    check(AtlasAITool::kToolId);
    check(IDETool::kToolId);
}

// ─────────────────────────────────────────────────────────────────
// Primary tool filter
// ─────────────────────────────────────────────────────────────────

TEST_CASE("Bootstrap: all 9 core tools are marked primary", "[bootstrap][phase3]") {
    WorkspaceShell shell;
    registerCoreTools(shell);
    shell.initialize();
    auto primaries = shell.toolRegistry().primaryTools();
    CHECK(primaries.size() == 9);
    for (const auto* t : primaries)
        CHECK(t->descriptor().isPrimary);
    shell.shutdown();
}

// ─────────────────────────────────────────────────────────────────
// Category coverage
// ─────────────────────────────────────────────────────────────────

TEST_CASE("Bootstrap: each primary category has at least one tool", "[bootstrap][phase3]") {
    WorkspaceShell shell;
    registerCoreTools(shell);
    shell.initialize();

    auto has = [&](HostedToolCategory cat) {
        return !shell.toolRegistry().byCategory(cat).empty();
    };

    CHECK(has(HostedToolCategory::SceneEditing));
    CHECK(has(HostedToolCategory::ProjectBrowser));
    CHECK(has(HostedToolCategory::AssetAuthoring));
    CHECK(has(HostedToolCategory::AnimationAuthoring));
    CHECK(has(HostedToolCategory::DataEditing));
    CHECK(has(HostedToolCategory::LogicAuthoring));
    CHECK(has(HostedToolCategory::BuildPackaging));
    CHECK(has(HostedToolCategory::AIAssistant));

    shell.shutdown();
}

// ─────────────────────────────────────────────────────────────────
// Tool IDs are unique in the registry
// ─────────────────────────────────────────────────────────────────

TEST_CASE("Bootstrap: no duplicate tool IDs in bootstrap roster", "[bootstrap][phase3]") {
    WorkspaceShell shell;
    registerCoreTools(shell);
    shell.initialize();

    auto descriptors = shell.toolRegistry().allDescriptors();
    std::vector<std::string> ids;
    ids.reserve(descriptors.size());
    for (const auto* d : descriptors) ids.push_back(d->toolId);

    for (size_t i = 0; i < ids.size(); ++i)
        for (size_t j = i + 1; j < ids.size(); ++j)
            CHECK(ids[i] != ids[j]);

    shell.shutdown();
}

// ─────────────────────────────────────────────────────────────────
// double initialize is idempotent
// ─────────────────────────────────────────────────────────────────

TEST_CASE("Bootstrap: double initialize does not add extra tools", "[bootstrap][phase3]") {
    WorkspaceShell shell;
    registerCoreTools(shell);
    REQUIRE(shell.initialize());
    REQUIRE_FALSE(shell.initialize()); // second call rejected
    CHECK(shell.toolRegistry().count() == 9);
    shell.shutdown();
}

// ─────────────────────────────────────────────────────────────────
// Project events propagate to all 8 core tools
// ─────────────────────────────────────────────────────────────────

TEST_CASE("Bootstrap: project events propagate to all bootstrapped tools", "[bootstrap][phase3]") {
    WorkspaceShell shell;
    registerCoreTools(shell);
    shell.initialize();

    // This must not crash and should reach all 8 tools.
    shell.toolRegistry().notifyProjectLoaded("nova_forge");
    shell.toolRegistry().notifyProjectUnloaded();

    shell.shutdown();
}

// ─────────────────────────────────────────────────────────────────
// Activation of any core tool through the shell works
// ─────────────────────────────────────────────────────────────────

TEST_CASE("Bootstrap: SceneEditorTool can be activated through shell", "[bootstrap][phase3]") {
    WorkspaceShell shell;
    registerCoreTools(shell);
    shell.initialize();

    CHECK(shell.toolRegistry().activateTool(std::string(SceneEditorTool::kToolId)));
    CHECK(shell.toolRegistry().activeToolId() == SceneEditorTool::kToolId);
    auto* t = shell.toolRegistry().find(std::string(SceneEditorTool::kToolId));
    REQUIRE(t != nullptr);
    CHECK(t->state() == HostedToolState::Active);

    shell.shutdown();
}

TEST_CASE("Bootstrap: switching active tool suspends the previous one", "[bootstrap][phase3]") {
    WorkspaceShell shell;
    registerCoreTools(shell);
    shell.initialize();

    shell.toolRegistry().activateTool(std::string(SceneEditorTool::kToolId));
    shell.toolRegistry().activateTool(std::string(AssetEditorTool::kToolId));

    auto* scene = shell.toolRegistry().find(std::string(SceneEditorTool::kToolId));
    auto* asset = shell.toolRegistry().find(std::string(AssetEditorTool::kToolId));

    REQUIRE(scene != nullptr);
    REQUIRE(asset != nullptr);
    CHECK(scene->state() == HostedToolState::Suspended);
    CHECK(asset->state()  == HostedToolState::Active);

    shell.shutdown();
}

// ─────────────────────────────────────────────────────────────────
// update() with active tool does not crash
// ─────────────────────────────────────────────────────────────────

TEST_CASE("Bootstrap: update dispatches to active core tool", "[bootstrap][phase3]") {
    WorkspaceShell shell;
    registerCoreTools(shell);
    shell.initialize();

    shell.toolRegistry().activateTool(std::string(SceneEditorTool::kToolId));
    shell.update(0.016f); // must not crash

    shell.shutdown();
}
