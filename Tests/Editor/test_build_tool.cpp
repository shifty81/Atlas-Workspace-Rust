// Tests/Editor/test_build_tool.cpp
// Tests for Phase 3 NF::BuildTool — seventh real IHostedTool.
#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/BuildTool.h"

using namespace NF;

// ─────────────────────────────────────────────────────────────────
// BuildMode helpers
// ─────────────────────────────────────────────────────────────────

TEST_CASE("buildModeName returns correct strings", "[BuildMode]") {
    CHECK(std::string(buildModeName(BuildMode::Full))        == "Full");
    CHECK(std::string(buildModeName(BuildMode::Incremental)) == "Incremental");
    CHECK(std::string(buildModeName(BuildMode::ShaderOnly))  == "ShaderOnly");
}

// ─────────────────────────────────────────────────────────────────
// BuildTool — descriptor
// ─────────────────────────────────────────────────────────────────

TEST_CASE("BuildTool descriptor is valid at construction", "[BuildTool][descriptor]") {
    BuildTool tool;
    const auto& d = tool.descriptor();
    CHECK(d.isValid());
    CHECK(d.toolId      == BuildTool::kToolId);
    CHECK(d.displayName == "Build Tool");
    CHECK(d.category    == HostedToolCategory::BuildPackaging);
    CHECK(d.isPrimary   == true);
    CHECK(d.acceptsProjectExtensions == true);
}

TEST_CASE("BuildTool toolId matches kToolId", "[BuildTool][descriptor]") {
    BuildTool tool;
    CHECK(tool.toolId() == std::string(BuildTool::kToolId));
    CHECK(tool.toolId() == "workspace.build_tool");
}

TEST_CASE("BuildTool declares expected shared panels", "[BuildTool][descriptor]") {
    BuildTool tool;
    const auto& panels = tool.descriptor().supportedPanels;
    REQUIRE(panels.size() >= 3);
    auto has = [&](const std::string& id) {
        for (const auto& p : panels) if (p == id) return true;
        return false;
    };
    CHECK(has("panel.build_log"));
    CHECK(has("panel.inspector"));
    CHECK(has("panel.console"));
}

TEST_CASE("BuildTool declares expected commands", "[BuildTool][descriptor]") {
    BuildTool tool;
    const auto& cmds = tool.descriptor().commands;
    REQUIRE(cmds.size() >= 6);
    auto has = [&](const std::string& id) {
        for (const auto& c : cmds) if (c == id) return true;
        return false;
    };
    CHECK(has("build.run"));
    CHECK(has("build.clean"));
    CHECK(has("build.cancel"));
    CHECK(has("build.open_output"));
    CHECK(has("build.set_target"));
    CHECK(has("build.run_tests"));
}

// ─────────────────────────────────────────────────────────────────
// BuildTool — lifecycle
// ─────────────────────────────────────────────────────────────────

TEST_CASE("BuildTool initial state is Unloaded", "[BuildTool][lifecycle]") {
    BuildTool tool;
    CHECK(tool.state() == HostedToolState::Unloaded);
}

TEST_CASE("BuildTool initialize transitions to Ready", "[BuildTool][lifecycle]") {
    BuildTool tool;
    CHECK(tool.initialize());
    CHECK(tool.state() == HostedToolState::Ready);
}

TEST_CASE("BuildTool double initialize returns false", "[BuildTool][lifecycle]") {
    BuildTool tool;
    CHECK(tool.initialize());
    CHECK_FALSE(tool.initialize());
    CHECK(tool.state() == HostedToolState::Ready);
}

TEST_CASE("BuildTool activate from Ready transitions to Active", "[BuildTool][lifecycle]") {
    BuildTool tool;
    tool.initialize();
    tool.activate();
    CHECK(tool.state() == HostedToolState::Active);
}

TEST_CASE("BuildTool suspend from Active transitions to Suspended", "[BuildTool][lifecycle]") {
    BuildTool tool;
    tool.initialize();
    tool.activate();
    tool.suspend();
    CHECK(tool.state() == HostedToolState::Suspended);
}

TEST_CASE("BuildTool activate from Suspended transitions to Active", "[BuildTool][lifecycle]") {
    BuildTool tool;
    tool.initialize();
    tool.activate();
    tool.suspend();
    tool.activate();
    CHECK(tool.state() == HostedToolState::Active);
}

TEST_CASE("BuildTool shutdown from Active transitions to Unloaded", "[BuildTool][lifecycle]") {
    BuildTool tool;
    tool.initialize();
    tool.activate();
    tool.shutdown();
    CHECK(tool.state() == HostedToolState::Unloaded);
}

TEST_CASE("BuildTool update does not change state", "[BuildTool][lifecycle]") {
    BuildTool tool;
    tool.initialize();
    tool.activate();
    tool.update(0.016f);
    CHECK(tool.state() == HostedToolState::Active);
}

// ─────────────────────────────────────────────────────────────────
// BuildTool — build mode
// ─────────────────────────────────────────────────────────────────

TEST_CASE("BuildTool default build mode is Incremental", "[BuildTool][mode]") {
    BuildTool tool;
    tool.initialize();
    CHECK(tool.buildMode() == BuildMode::Incremental);
}

TEST_CASE("BuildTool setBuildMode roundtrips all modes", "[BuildTool][mode]") {
    BuildTool tool;
    tool.initialize();
    tool.setBuildMode(BuildMode::Full);
    CHECK(tool.buildMode() == BuildMode::Full);
    tool.setBuildMode(BuildMode::ShaderOnly);
    CHECK(tool.buildMode() == BuildMode::ShaderOnly);
    tool.setBuildMode(BuildMode::Incremental);
    CHECK(tool.buildMode() == BuildMode::Incremental);
}

// ─────────────────────────────────────────────────────────────────
// BuildTool — stats
// ─────────────────────────────────────────────────────────────────

TEST_CASE("BuildTool stats default to zero", "[BuildTool][stats]") {
    BuildTool tool;
    tool.initialize();
    const auto& s = tool.stats();
    CHECK(s.warningCount == 0u);
    CHECK(s.errorCount   == 0u);
    CHECK(s.lastBuildMs  == 0.0f);
    CHECK(s.isBuilding   == false);
}

TEST_CASE("BuildTool setBuilding / isBuilding", "[BuildTool][stats]") {
    BuildTool tool;
    tool.initialize();
    tool.setBuilding(true);
    CHECK(tool.isBuilding());
    tool.setBuilding(false);
    CHECK_FALSE(tool.isBuilding());
}

TEST_CASE("BuildTool setLastBuildMs roundtrips", "[BuildTool][stats]") {
    BuildTool tool;
    tool.initialize();
    tool.setLastBuildMs(4200.0f);
    CHECK(tool.stats().lastBuildMs == 4200.0f);
}

TEST_CASE("BuildTool setWarningCount / warningCount", "[BuildTool][stats]") {
    BuildTool tool;
    tool.initialize();
    tool.setWarningCount(7);
    CHECK(tool.warningCount() == 7u);
}

TEST_CASE("BuildTool setErrorCount / errorCount", "[BuildTool][stats]") {
    BuildTool tool;
    tool.initialize();
    tool.setErrorCount(2);
    CHECK(tool.errorCount() == 2u);
}

TEST_CASE("BuildTool clearStats resets all counters", "[BuildTool][stats]") {
    BuildTool tool;
    tool.initialize();
    tool.setWarningCount(5);
    tool.setErrorCount(2);
    tool.setLastBuildMs(1000.0f);
    tool.setBuilding(true);
    tool.clearStats();
    CHECK(tool.stats().warningCount == 0u);
    CHECK(tool.stats().errorCount   == 0u);
    CHECK(tool.stats().lastBuildMs  == 0.0f);
    CHECK_FALSE(tool.isBuilding());
}

// ─────────────────────────────────────────────────────────────────
// BuildTool — active target
// ─────────────────────────────────────────────────────────────────

TEST_CASE("BuildTool activeTarget starts empty", "[BuildTool][target]") {
    BuildTool tool;
    tool.initialize();
    CHECK(tool.activeTarget().empty());
}

TEST_CASE("BuildTool setActiveTarget roundtrips", "[BuildTool][target]") {
    BuildTool tool;
    tool.initialize();
    tool.setActiveTarget("AtlasWorkspace_Debug");
    CHECK(tool.activeTarget() == "AtlasWorkspace_Debug");
}

TEST_CASE("BuildTool shutdown clears activeTarget", "[BuildTool][target]") {
    BuildTool tool;
    tool.initialize();
    tool.setActiveTarget("AtlasWorkspace_Debug");
    tool.shutdown();
    CHECK(tool.activeTarget().empty());
}

// ─────────────────────────────────────────────────────────────────
// BuildTool — project hooks
// ─────────────────────────────────────────────────────────────────

TEST_CASE("BuildTool onProjectLoaded clears stats and target", "[BuildTool][project]") {
    BuildTool tool;
    tool.initialize();
    tool.setErrorCount(5);
    tool.setActiveTarget("SomeTarget");
    tool.onProjectLoaded("nova_forge");
    CHECK(tool.stats().errorCount == 0u);
    CHECK(tool.activeTarget().empty());
}

TEST_CASE("BuildTool onProjectUnloaded clears stats and target", "[BuildTool][project]") {
    BuildTool tool;
    tool.initialize();
    tool.setErrorCount(5);
    tool.onProjectLoaded("nova_forge");
    tool.onProjectUnloaded();
    CHECK(tool.stats().errorCount == 0u);
    CHECK(tool.activeTarget().empty());
}
