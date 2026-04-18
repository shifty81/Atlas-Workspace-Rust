#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"

using namespace NF;

// ── ToolLaunchMode enum ─────────────────────────────────────────

TEST_CASE("ToolLaunchMode enum values", "[Editor][S50]") {
    REQUIRE(static_cast<uint8_t>(ToolLaunchMode::Embedded) == 0);
    REQUIRE(static_cast<uint8_t>(ToolLaunchMode::External) == 1);
    REQUIRE(static_cast<uint8_t>(ToolLaunchMode::Docked) == 2);
}

// ── ToolDescriptor defaults ─────────────────────────────────────

TEST_CASE("ToolDescriptor default values", "[Editor][S50]") {
    ToolDescriptor td;
    REQUIRE(td.name.empty());
    REQUIRE(td.executable.empty());
    REQUIRE(td.icon.empty());
    REQUIRE(td.mode == ToolLaunchMode::External);
    REQUIRE(td.isRunning == false);
}

// ── ToolWindowManager ───────────────────────────────────────────

TEST_CASE("ToolWindowManager starts empty", "[Editor][S50]") {
    ToolWindowManager mgr;
    REQUIRE(mgr.toolCount() == 0);
    REQUIRE(mgr.runningCount() == 0);
    REQUIRE(mgr.tools().empty());
}

TEST_CASE("ToolWindowManager registerTool increases count", "[Editor][S50]") {
    ToolWindowManager mgr;
    ToolDescriptor td;
    td.name = "Profiler";
    td.executable = "profiler.exe";

    mgr.registerTool(td);
    REQUIRE(mgr.toolCount() == 1);
}

TEST_CASE("ToolWindowManager findTool returns pointer or nullptr", "[Editor][S50]") {
    ToolWindowManager mgr;
    ToolDescriptor td;
    td.name = "Debugger";
    mgr.registerTool(td);

    REQUIRE(mgr.findTool("Debugger") != nullptr);
    REQUIRE(mgr.findTool("Debugger")->name == "Debugger");
    REQUIRE(mgr.findTool("Missing") == nullptr);
}

TEST_CASE("ToolWindowManager launchTool returns true for known tool", "[Editor][S50]") {
    ToolWindowManager mgr;
    ToolDescriptor td;
    td.name = "AssetBrowser";
    mgr.registerTool(td);

    REQUIRE(mgr.launchTool("AssetBrowser") == true);
    REQUIRE(mgr.findTool("AssetBrowser")->isRunning == true);
    REQUIRE(mgr.runningCount() == 1);
}

TEST_CASE("ToolWindowManager launchTool returns false for unknown tool", "[Editor][S50]") {
    ToolWindowManager mgr;
    REQUIRE(mgr.launchTool("NonExistent") == false);
    REQUIRE(mgr.runningCount() == 0);
}

TEST_CASE("ToolWindowManager stopTool marks tool as not running", "[Editor][S50]") {
    ToolWindowManager mgr;
    ToolDescriptor td;
    td.name = "Console";
    mgr.registerTool(td);

    mgr.launchTool("Console");
    REQUIRE(mgr.runningCount() == 1);

    mgr.stopTool("Console");
    REQUIRE(mgr.findTool("Console")->isRunning == false);
    REQUIRE(mgr.runningCount() == 0);
}

TEST_CASE("ToolWindowManager multiple tools running", "[Editor][S50]") {
    ToolWindowManager mgr;

    ToolDescriptor t1; t1.name = "A";
    ToolDescriptor t2; t2.name = "B";
    ToolDescriptor t3; t3.name = "C";
    mgr.registerTool(t1);
    mgr.registerTool(t2);
    mgr.registerTool(t3);
    REQUIRE(mgr.toolCount() == 3);

    mgr.launchTool("A");
    mgr.launchTool("C");
    REQUIRE(mgr.runningCount() == 2);

    mgr.stopTool("A");
    REQUIRE(mgr.runningCount() == 1);
}

TEST_CASE("ToolWindowManager tools returns full list", "[Editor][S50]") {
    ToolWindowManager mgr;
    ToolDescriptor t1; t1.name = "X"; t1.mode = ToolLaunchMode::Embedded;
    ToolDescriptor t2; t2.name = "Y"; t2.mode = ToolLaunchMode::Docked;
    mgr.registerTool(t1);
    mgr.registerTool(t2);

    const auto& tools = mgr.tools();
    REQUIRE(tools.size() == 2);
    REQUIRE(tools[0].name == "X");
    REQUIRE(tools[0].mode == ToolLaunchMode::Embedded);
    REQUIRE(tools[1].name == "Y");
    REQUIRE(tools[1].mode == ToolLaunchMode::Docked);
}

// ── PipelineEventEntry defaults ─────────────────────────────────

TEST_CASE("PipelineEventEntry default values", "[Editor][S50]") {
    PipelineEventEntry entry;
    REQUIRE(entry.type.empty());
    REQUIRE(entry.source.empty());
    REQUIRE(entry.details.empty());
    REQUIRE(entry.timestamp == 0.f);
}
