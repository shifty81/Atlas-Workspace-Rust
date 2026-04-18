#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/SoundscapeEditor.h"
#include "NF/Editor/AIBehaviorEditor.h"
#include "NF/Editor/NavMeshEditor.h"

using namespace NF;

// ── S93: NavMeshEditor + AIBehaviorEditor + SoundscapeEditor ─────

// ── NavMeshEditor ────────────────────────────────────────────────

TEST_CASE("NavMeshBuildStatus names are correct", "[Editor][S93]") {
    REQUIRE(std::string(navMeshBuildStatusName(NavMeshBuildStatus::Idle))     == "Idle");
    REQUIRE(std::string(navMeshBuildStatusName(NavMeshBuildStatus::Building)) == "Building");
    REQUIRE(std::string(navMeshBuildStatusName(NavMeshBuildStatus::Done))     == "Done");
    REQUIRE(std::string(navMeshBuildStatusName(NavMeshBuildStatus::Failed))   == "Failed");
    REQUIRE(std::string(navMeshBuildStatusName(NavMeshBuildStatus::Outdated)) == "Outdated");
}

TEST_CASE("NavAgentCategory names are correct", "[Editor][S93]") {
    REQUIRE(std::string(navAgentCategoryName(NavAgentCategory::Humanoid)) == "Humanoid");
    REQUIRE(std::string(navAgentCategoryName(NavAgentCategory::Vehicle))  == "Vehicle");
    REQUIRE(std::string(navAgentCategoryName(NavAgentCategory::Flying))   == "Flying");
    REQUIRE(std::string(navAgentCategoryName(NavAgentCategory::Crawling)) == "Crawling");
    REQUIRE(std::string(navAgentCategoryName(NavAgentCategory::Custom))   == "Custom");
}

TEST_CASE("NavMeshDebugView names are correct", "[Editor][S93]") {
    REQUIRE(std::string(navMeshDebugViewName(NavMeshDebugView::None))        == "None");
    REQUIRE(std::string(navMeshDebugViewName(NavMeshDebugView::Polygons))    == "Polygons");
    REQUIRE(std::string(navMeshDebugViewName(NavMeshDebugView::Connections)) == "Connections");
    REQUIRE(std::string(navMeshDebugViewName(NavMeshDebugView::Areas))       == "Areas");
    REQUIRE(std::string(navMeshDebugViewName(NavMeshDebugView::Boundaries))  == "Boundaries");
    REQUIRE(std::string(navMeshDebugViewName(NavMeshDebugView::All))         == "All");
}

TEST_CASE("NavAgentConfig stores properties", "[Editor][S93]") {
    NavAgentConfig agent("Player", NavAgentCategory::Humanoid);
    agent.setRadius(0.6f);
    agent.setHeight(1.8f);
    agent.setMaxSlope(50.0f);
    agent.setStepHeight(0.4f);
    REQUIRE(agent.name()       == "Player");
    REQUIRE(agent.category()   == NavAgentCategory::Humanoid);
    REQUIRE(agent.radius()     == 0.6f);
    REQUIRE(agent.height()     == 1.8f);
    REQUIRE(agent.maxSlope()   == 50.0f);
    REQUIRE(agent.isEnabled());
}

TEST_CASE("NavMeshEditor add remove findAgent", "[Editor][S93]") {
    NavMeshEditor editor;
    NavAgentConfig a1("Human", NavAgentCategory::Humanoid);
    NavAgentConfig a2("Car",   NavAgentCategory::Vehicle);
    REQUIRE(editor.addAgent(a1));
    REQUIRE(editor.addAgent(a2));
    REQUIRE(editor.agentCount() == 2);
    REQUIRE(editor.findAgent("Car") != nullptr);
    editor.removeAgent("Car");
    REQUIRE(editor.agentCount() == 1);
}

TEST_CASE("NavMeshEditor rejects duplicate agent name", "[Editor][S93]") {
    NavMeshEditor editor;
    NavAgentConfig a("Bot", NavAgentCategory::Humanoid);
    editor.addAgent(a);
    REQUIRE_FALSE(editor.addAgent(a));
}

TEST_CASE("NavMeshEditor build settings and isBuilt", "[Editor][S93]") {
    NavMeshEditor editor;
    editor.setBuildStatus(NavMeshBuildStatus::Done);
    editor.setDebugView(NavMeshDebugView::Polygons);
    editor.setCellSize(0.2f);
    editor.setAutoRebuild(true);
    REQUIRE(editor.buildStatus() == NavMeshBuildStatus::Done);
    REQUIRE(editor.isBuilt());
    REQUIRE(editor.cellSize()    == 0.2f);
    REQUIRE(editor.autoRebuild());
}

TEST_CASE("NavMeshEditor countByCategory and enabledAgentCount", "[Editor][S93]") {
    NavMeshEditor editor;
    NavAgentConfig a1("H1", NavAgentCategory::Humanoid);
    NavAgentConfig a2("H2", NavAgentCategory::Humanoid); a2.setEnabled(false);
    NavAgentConfig a3("V1", NavAgentCategory::Vehicle);
    editor.addAgent(a1); editor.addAgent(a2); editor.addAgent(a3);
    REQUIRE(editor.countByCategory(NavAgentCategory::Humanoid) == 2);
    REQUIRE(editor.enabledAgentCount() == 2);
}

TEST_CASE("NavMeshEditor MAX_AGENTS is 16", "[Editor][S93]") {
    REQUIRE(NavMeshEditor::MAX_AGENTS == 16);
}

// ── AIBehaviorEditor ─────────────────────────────────────────────

TEST_CASE("BTNodeType names are correct", "[Editor][S93]") {
    REQUIRE(std::string(btNodeTypeName(BTNodeType::Selector))  == "Selector");
    REQUIRE(std::string(btNodeTypeName(BTNodeType::Sequence))  == "Sequence");
    REQUIRE(std::string(btNodeTypeName(BTNodeType::Parallel))  == "Parallel");
    REQUIRE(std::string(btNodeTypeName(BTNodeType::Decorator)) == "Decorator");
    REQUIRE(std::string(btNodeTypeName(BTNodeType::Task))      == "Task");
    REQUIRE(std::string(btNodeTypeName(BTNodeType::Condition)) == "Condition");
    REQUIRE(std::string(btNodeTypeName(BTNodeType::Root))      == "Root");
}

TEST_CASE("BTNodeStatus names are correct", "[Editor][S93]") {
    REQUIRE(std::string(btNodeStatusName(BTNodeStatus::Idle))    == "Idle");
    REQUIRE(std::string(btNodeStatusName(BTNodeStatus::Running)) == "Running");
    REQUIRE(std::string(btNodeStatusName(BTNodeStatus::Success)) == "Success");
    REQUIRE(std::string(btNodeStatusName(BTNodeStatus::Failure)) == "Failure");
    REQUIRE(std::string(btNodeStatusName(BTNodeStatus::Aborted)) == "Aborted");
}

TEST_CASE("BTAbortMode names are correct", "[Editor][S93]") {
    REQUIRE(std::string(btAbortModeName(BTAbortMode::None))          == "None");
    REQUIRE(std::string(btAbortModeName(BTAbortMode::Self))          == "Self");
    REQUIRE(std::string(btAbortModeName(BTAbortMode::LowerPriority)) == "LowerPriority");
    REQUIRE(std::string(btAbortModeName(BTAbortMode::Both))          == "Both");
}

TEST_CASE("BTEditorNode stores properties", "[Editor][S93]") {
    BTEditorNode node("Chase", BTNodeType::Task);
    node.setStatus(BTNodeStatus::Running);
    node.setAbortMode(BTAbortMode::Self);
    node.setChildCount(3);
    node.setBreakpoint(true);
    REQUIRE(node.name()      == "Chase");
    REQUIRE(node.type()      == BTNodeType::Task);
    REQUIRE(node.isRunning());
    REQUIRE(node.childCount() == 3);
    REQUIRE(node.hasBreakpoint());
    REQUIRE_FALSE(node.isLeaf());
}

TEST_CASE("AIBehaviorEditor add selectNode remove", "[Editor][S93]") {
    AIBehaviorEditor editor;
    BTEditorNode n1("Root",  BTNodeType::Root);
    BTEditorNode n2("Chase", BTNodeType::Task);
    REQUIRE(editor.addNode(n1));
    REQUIRE(editor.addNode(n2));
    REQUIRE(editor.nodeCount() == 2);
    REQUIRE(editor.selectNode("Chase"));
    REQUIRE(editor.selectedNode() == "Chase");
    editor.removeNode("Chase");
    REQUIRE(editor.selectedNode().empty());
}

TEST_CASE("AIBehaviorEditor rejects duplicate node name", "[Editor][S93]") {
    AIBehaviorEditor editor;
    BTEditorNode n("Idle", BTNodeType::Task);
    editor.addNode(n);
    REQUIRE_FALSE(editor.addNode(n));
}

TEST_CASE("AIBehaviorEditor countByType, runningCount, leafCount", "[Editor][S93]") {
    AIBehaviorEditor editor;
    BTEditorNode n1("T1", BTNodeType::Task);     n1.setStatus(BTNodeStatus::Running); n1.setChildCount(0);
    BTEditorNode n2("T2", BTNodeType::Task);     n2.setStatus(BTNodeStatus::Idle);    n2.setChildCount(0);
    BTEditorNode n3("S1", BTNodeType::Selector); n3.setChildCount(2);
    editor.addNode(n1); editor.addNode(n2); editor.addNode(n3);
    REQUIRE(editor.countByType(BTNodeType::Task) == 2);
    REQUIRE(editor.runningCount() == 1);
    REQUIRE(editor.leafCount()    == 2);
}

TEST_CASE("AIBehaviorEditor MAX_NODES is 256", "[Editor][S93]") {
    REQUIRE(AIBehaviorEditor::MAX_NODES == 256);
}

// ── SoundscapeEditor ─────────────────────────────────────────────

TEST_CASE("SoundscapeZoneShape names are correct", "[Editor][S93]") {
    REQUIRE(std::string(soundscapeZoneShapeName(SoundscapeZoneShape::Box))     == "Box");
    REQUIRE(std::string(soundscapeZoneShapeName(SoundscapeZoneShape::Sphere))  == "Sphere");
    REQUIRE(std::string(soundscapeZoneShapeName(SoundscapeZoneShape::Capsule)) == "Capsule");
    REQUIRE(std::string(soundscapeZoneShapeName(SoundscapeZoneShape::Convex))  == "Convex");
    REQUIRE(std::string(soundscapeZoneShapeName(SoundscapeZoneShape::Global))  == "Global");
}

TEST_CASE("SoundscapeBlendMode names are correct", "[Editor][S93]") {
    REQUIRE(std::string(soundscapeBlendModeName(SoundscapeBlendMode::Additive))    == "Additive");
    REQUIRE(std::string(soundscapeBlendModeName(SoundscapeBlendMode::Override))    == "Override");
    REQUIRE(std::string(soundscapeBlendModeName(SoundscapeBlendMode::Interpolate)) == "Interpolate");
    REQUIRE(std::string(soundscapeBlendModeName(SoundscapeBlendMode::Priority))    == "Priority");
}

TEST_CASE("SoundscapeReverbPreset names are correct", "[Editor][S93]") {
    REQUIRE(std::string(soundscapeReverbPresetName(SoundscapeReverbPreset::None))    == "None");
    REQUIRE(std::string(soundscapeReverbPresetName(SoundscapeReverbPreset::Room))    == "Room");
    REQUIRE(std::string(soundscapeReverbPresetName(SoundscapeReverbPreset::Hall))    == "Hall");
    REQUIRE(std::string(soundscapeReverbPresetName(SoundscapeReverbPreset::Cave))    == "Cave");
    REQUIRE(std::string(soundscapeReverbPresetName(SoundscapeReverbPreset::Forest))  == "Forest");
    REQUIRE(std::string(soundscapeReverbPresetName(SoundscapeReverbPreset::City))    == "City");
    REQUIRE(std::string(soundscapeReverbPresetName(SoundscapeReverbPreset::Dungeon)) == "Dungeon");
    REQUIRE(std::string(soundscapeReverbPresetName(SoundscapeReverbPreset::Custom))  == "Custom");
}

TEST_CASE("SoundscapeZone stores properties", "[Editor][S93]") {
    SoundscapeZone zone("Cave1", SoundscapeZoneShape::Box);
    zone.setBlendMode(SoundscapeBlendMode::Override);
    zone.setReverbPreset(SoundscapeReverbPreset::Cave);
    zone.setVolume(0.8f);
    zone.setPriority(5);
    zone.setFadeInTime(2.0f);
    REQUIRE(zone.name()       == "Cave1");
    REQUIRE(zone.shape()      == SoundscapeZoneShape::Box);
    REQUIRE(zone.reverb()     == SoundscapeReverbPreset::Cave);
    REQUIRE(zone.priority()   == 5);
    REQUIRE(zone.isEnabled());
}

TEST_CASE("SoundscapeEditor add setActive remove", "[Editor][S93]") {
    SoundscapeEditor editor;
    SoundscapeZone z1("Forest1", SoundscapeZoneShape::Sphere);
    SoundscapeZone z2("City1",   SoundscapeZoneShape::Box);
    REQUIRE(editor.addZone(z1));
    REQUIRE(editor.addZone(z2));
    REQUIRE(editor.zoneCount() == 2);
    REQUIRE(editor.setActiveZone("Forest1"));
    REQUIRE(editor.activeZone() == "Forest1");
    editor.removeZone("Forest1");
    REQUIRE(editor.activeZone().empty());
}

TEST_CASE("SoundscapeEditor rejects duplicate zone name", "[Editor][S93]") {
    SoundscapeEditor editor;
    SoundscapeZone z("Dungeon", SoundscapeZoneShape::Convex);
    editor.addZone(z);
    REQUIRE_FALSE(editor.addZone(z));
}

TEST_CASE("SoundscapeEditor countByShape and countByReverb", "[Editor][S93]") {
    SoundscapeEditor editor;
    SoundscapeZone z1("A", SoundscapeZoneShape::Box);    z1.setReverbPreset(SoundscapeReverbPreset::Room);
    SoundscapeZone z2("B", SoundscapeZoneShape::Sphere); z2.setReverbPreset(SoundscapeReverbPreset::Room);
    SoundscapeZone z3("C", SoundscapeZoneShape::Box);    z3.setReverbPreset(SoundscapeReverbPreset::Hall);
    editor.addZone(z1); editor.addZone(z2); editor.addZone(z3);
    REQUIRE(editor.countByShape(SoundscapeZoneShape::Box)            == 2);
    REQUIRE(editor.countByReverb(SoundscapeReverbPreset::Room)       == 2);
}

TEST_CASE("SoundscapeEditor MAX_ZONES is 64", "[Editor][S93]") {
    REQUIRE(SoundscapeEditor::MAX_ZONES == 64);
}
