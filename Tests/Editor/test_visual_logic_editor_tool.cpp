// Tests/Editor/test_visual_logic_editor_tool.cpp
// Tests for Phase 3 NF::VisualLogicEditorTool — sixth real IHostedTool.
#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/VisualLogicEditorTool.h"

using namespace NF;

// ─────────────────────────────────────────────────────────────────
// VisualLogicMode helpers
// ─────────────────────────────────────────────────────────────────

TEST_CASE("visualLogicModeName returns correct strings", "[VisualLogicMode]") {
    CHECK(std::string(visualLogicModeName(VisualLogicMode::Graph)) == "Graph");
    CHECK(std::string(visualLogicModeName(VisualLogicMode::Debug)) == "Debug");
    CHECK(std::string(visualLogicModeName(VisualLogicMode::Diff))  == "Diff");
}

// ─────────────────────────────────────────────────────────────────
// VisualLogicEditorTool — descriptor
// ─────────────────────────────────────────────────────────────────

TEST_CASE("VisualLogicEditorTool descriptor is valid at construction", "[VisualLogicEditorTool][descriptor]") {
    VisualLogicEditorTool tool;
    const auto& d = tool.descriptor();
    CHECK(d.isValid());
    CHECK(d.toolId      == VisualLogicEditorTool::kToolId);
    CHECK(d.displayName == "Visual Logic Editor");
    CHECK(d.category    == HostedToolCategory::LogicAuthoring);
    CHECK(d.isPrimary   == true);
    CHECK(d.acceptsProjectExtensions == false);
}

TEST_CASE("VisualLogicEditorTool toolId matches kToolId", "[VisualLogicEditorTool][descriptor]") {
    VisualLogicEditorTool tool;
    CHECK(tool.toolId() == std::string(VisualLogicEditorTool::kToolId));
    CHECK(tool.toolId() == "workspace.visual_logic_editor");
}

TEST_CASE("VisualLogicEditorTool declares expected shared panels", "[VisualLogicEditorTool][descriptor]") {
    VisualLogicEditorTool tool;
    const auto& panels = tool.descriptor().supportedPanels;
    REQUIRE(panels.size() >= 3);
    auto has = [&](const std::string& id) {
        for (const auto& p : panels) if (p == id) return true;
        return false;
    };
    CHECK(has("panel.node_graph"));
    CHECK(has("panel.inspector"));
    CHECK(has("panel.console"));
}

TEST_CASE("VisualLogicEditorTool declares expected commands", "[VisualLogicEditorTool][descriptor]") {
    VisualLogicEditorTool tool;
    const auto& cmds = tool.descriptor().commands;
    REQUIRE(cmds.size() >= 6);
    auto has = [&](const std::string& id) {
        for (const auto& c : cmds) if (c == id) return true;
        return false;
    };
    CHECK(has("vl.add_node"));
    CHECK(has("vl.delete_node"));
    CHECK(has("vl.connect_pins"));
    CHECK(has("vl.disconnect_pins"));
    CHECK(has("vl.compile"));
    CHECK(has("vl.save"));
}

// ─────────────────────────────────────────────────────────────────
// VisualLogicEditorTool — lifecycle
// ─────────────────────────────────────────────────────────────────

TEST_CASE("VisualLogicEditorTool initial state is Unloaded", "[VisualLogicEditorTool][lifecycle]") {
    VisualLogicEditorTool tool;
    CHECK(tool.state() == HostedToolState::Unloaded);
}

TEST_CASE("VisualLogicEditorTool initialize transitions to Ready", "[VisualLogicEditorTool][lifecycle]") {
    VisualLogicEditorTool tool;
    CHECK(tool.initialize());
    CHECK(tool.state() == HostedToolState::Ready);
}

TEST_CASE("VisualLogicEditorTool double initialize returns false", "[VisualLogicEditorTool][lifecycle]") {
    VisualLogicEditorTool tool;
    CHECK(tool.initialize());
    CHECK_FALSE(tool.initialize());
    CHECK(tool.state() == HostedToolState::Ready);
}

TEST_CASE("VisualLogicEditorTool activate from Ready transitions to Active", "[VisualLogicEditorTool][lifecycle]") {
    VisualLogicEditorTool tool;
    tool.initialize();
    tool.activate();
    CHECK(tool.state() == HostedToolState::Active);
}

TEST_CASE("VisualLogicEditorTool suspend from Active transitions to Suspended", "[VisualLogicEditorTool][lifecycle]") {
    VisualLogicEditorTool tool;
    tool.initialize();
    tool.activate();
    tool.suspend();
    CHECK(tool.state() == HostedToolState::Suspended);
}

TEST_CASE("VisualLogicEditorTool suspend clears isCompiling flag", "[VisualLogicEditorTool][lifecycle]") {
    VisualLogicEditorTool tool;
    tool.initialize();
    tool.activate();
    tool.setCompiling(true);
    tool.suspend();
    CHECK_FALSE(tool.isCompiling());
}

TEST_CASE("VisualLogicEditorTool activate from Suspended transitions to Active", "[VisualLogicEditorTool][lifecycle]") {
    VisualLogicEditorTool tool;
    tool.initialize();
    tool.activate();
    tool.suspend();
    tool.activate();
    CHECK(tool.state() == HostedToolState::Active);
}

TEST_CASE("VisualLogicEditorTool shutdown from Active transitions to Unloaded", "[VisualLogicEditorTool][lifecycle]") {
    VisualLogicEditorTool tool;
    tool.initialize();
    tool.activate();
    tool.shutdown();
    CHECK(tool.state() == HostedToolState::Unloaded);
}

// ─────────────────────────────────────────────────────────────────
// VisualLogicEditorTool — edit mode
// ─────────────────────────────────────────────────────────────────

TEST_CASE("VisualLogicEditorTool default edit mode is Graph", "[VisualLogicEditorTool][mode]") {
    VisualLogicEditorTool tool;
    tool.initialize();
    CHECK(tool.editMode() == VisualLogicMode::Graph);
}

TEST_CASE("VisualLogicEditorTool setEditMode roundtrips all modes", "[VisualLogicEditorTool][mode]") {
    VisualLogicEditorTool tool;
    tool.initialize();
    tool.setEditMode(VisualLogicMode::Debug);
    CHECK(tool.editMode() == VisualLogicMode::Debug);
    tool.setEditMode(VisualLogicMode::Diff);
    CHECK(tool.editMode() == VisualLogicMode::Diff);
    tool.setEditMode(VisualLogicMode::Graph);
    CHECK(tool.editMode() == VisualLogicMode::Graph);
}

// ─────────────────────────────────────────────────────────────────
// VisualLogicEditorTool — dirty state
// ─────────────────────────────────────────────────────────────────

TEST_CASE("VisualLogicEditorTool initially not dirty", "[VisualLogicEditorTool][dirty]") {
    VisualLogicEditorTool tool;
    tool.initialize();
    CHECK_FALSE(tool.isDirty());
}

TEST_CASE("VisualLogicEditorTool markDirty / clearDirty", "[VisualLogicEditorTool][dirty]") {
    VisualLogicEditorTool tool;
    tool.initialize();
    tool.markDirty();
    CHECK(tool.isDirty());
    tool.clearDirty();
    CHECK_FALSE(tool.isDirty());
}

// ─────────────────────────────────────────────────────────────────
// VisualLogicEditorTool — stats
// ─────────────────────────────────────────────────────────────────

TEST_CASE("VisualLogicEditorTool stats default to zero", "[VisualLogicEditorTool][stats]") {
    VisualLogicEditorTool tool;
    tool.initialize();
    const auto& s = tool.stats();
    CHECK(s.nodeCount       == 0u);
    CHECK(s.connectionCount == 0u);
    CHECK(s.errorCount      == 0u);
    CHECK(s.isDirty         == false);
    CHECK(s.isCompiling     == false);
}

TEST_CASE("VisualLogicEditorTool setNodeCount roundtrips", "[VisualLogicEditorTool][stats]") {
    VisualLogicEditorTool tool;
    tool.initialize();
    tool.setNodeCount(42);
    CHECK(tool.stats().nodeCount == 42u);
}

TEST_CASE("VisualLogicEditorTool setConnectionCount roundtrips", "[VisualLogicEditorTool][stats]") {
    VisualLogicEditorTool tool;
    tool.initialize();
    tool.setConnectionCount(18);
    CHECK(tool.stats().connectionCount == 18u);
}

TEST_CASE("VisualLogicEditorTool setCompiling / isCompiling", "[VisualLogicEditorTool][stats]") {
    VisualLogicEditorTool tool;
    tool.initialize();
    tool.setCompiling(true);
    CHECK(tool.isCompiling());
    tool.setCompiling(false);
    CHECK_FALSE(tool.isCompiling());
}

TEST_CASE("VisualLogicEditorTool setErrorCount / errorCount", "[VisualLogicEditorTool][stats]") {
    VisualLogicEditorTool tool;
    tool.initialize();
    tool.setErrorCount(3);
    CHECK(tool.errorCount() == 3u);
}

// ─────────────────────────────────────────────────────────────────
// VisualLogicEditorTool — project hooks
// ─────────────────────────────────────────────────────────────────

TEST_CASE("VisualLogicEditorTool onProjectLoaded clears stats", "[VisualLogicEditorTool][project]") {
    VisualLogicEditorTool tool;
    tool.initialize();
    tool.setNodeCount(20);
    tool.markDirty();
    tool.onProjectLoaded("nova_forge");
    CHECK(tool.stats().nodeCount == 0u);
    CHECK_FALSE(tool.isDirty());
}

TEST_CASE("VisualLogicEditorTool onProjectUnloaded clears stats", "[VisualLogicEditorTool][project]") {
    VisualLogicEditorTool tool;
    tool.initialize();
    tool.setNodeCount(20);
    tool.onProjectLoaded("nova_forge");
    tool.onProjectUnloaded();
    CHECK(tool.stats().nodeCount == 0u);
}
