// S100 editor tests: GameFlowGraph, StateGraphEditor, TriggerVolumeEditor
#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/TriggerVolumeEditor.h"
#include "NF/Editor/StateGraphEditor.h"
#include "NF/Editor/GameFlowGraph.h"

using namespace NF;

// ── GameFlowGraph ────────────────────────────────────────────────────────────

TEST_CASE("GameFlowNodeKind names", "[Editor][S100]") {
    REQUIRE(std::string(gameFlowNodeKindName(GameFlowNodeKind::Start))      == "Start");
    REQUIRE(std::string(gameFlowNodeKindName(GameFlowNodeKind::End))        == "End");
    REQUIRE(std::string(gameFlowNodeKindName(GameFlowNodeKind::GameState))  == "GameState");
    REQUIRE(std::string(gameFlowNodeKindName(GameFlowNodeKind::Transition)) == "Transition");
    REQUIRE(std::string(gameFlowNodeKindName(GameFlowNodeKind::Condition))  == "Condition");
    REQUIRE(std::string(gameFlowNodeKindName(GameFlowNodeKind::SubFlow))    == "SubFlow");
    REQUIRE(std::string(gameFlowNodeKindName(GameFlowNodeKind::Event))      == "Event");
    REQUIRE(std::string(gameFlowNodeKindName(GameFlowNodeKind::Delay))      == "Delay");
}

TEST_CASE("GameFlowCompileState names", "[Editor][S100]") {
    REQUIRE(std::string(gameFlowCompileStateName(GameFlowCompileState::Dirty))     == "Dirty");
    REQUIRE(std::string(gameFlowCompileStateName(GameFlowCompileState::Compiling)) == "Compiling");
    REQUIRE(std::string(gameFlowCompileStateName(GameFlowCompileState::Ok))        == "Ok");
    REQUIRE(std::string(gameFlowCompileStateName(GameFlowCompileState::Error))     == "Error");
}

TEST_CASE("GameFlowPinDir names", "[Editor][S100]") {
    REQUIRE(std::string(gameFlowPinDirName(GameFlowPinDir::Input))  == "Input");
    REQUIRE(std::string(gameFlowPinDirName(GameFlowPinDir::Output)) == "Output");
}

TEST_CASE("GameFlowNode defaults", "[Editor][S100]") {
    GameFlowNode n(1, GameFlowNodeKind::GameState);
    REQUIRE(n.id()   == 1u);
    REQUIRE(n.kind() == GameFlowNodeKind::GameState);
    REQUIRE(n.isEnabled());
    REQUIRE(!n.hasBreakpoint());
    REQUIRE(n.label()   == "");
    REQUIRE(n.comment() == "");
}

TEST_CASE("GameFlowNode mutation", "[Editor][S100]") {
    GameFlowNode n(2, GameFlowNodeKind::Condition);
    n.setLabel("Check health");
    n.setComment("Branches on HP");
    n.setEnabled(false);
    n.setBreakpoint(true);
    REQUIRE(n.label()          == "Check health");
    REQUIRE(n.comment()        == "Branches on HP");
    REQUIRE(!n.isEnabled());
    REQUIRE(n.hasBreakpoint());
}

TEST_CASE("GameFlowGraph add/remove node", "[Editor][S100]") {
    GameFlowGraph g("MainFlow");
    REQUIRE(g.name() == "MainFlow");
    REQUIRE(g.addNode(GameFlowNode(1, GameFlowNodeKind::Start)));
    REQUIRE(g.nodeCount() == 1u);
    REQUIRE(!g.addNode(GameFlowNode(1, GameFlowNodeKind::Start)));
    REQUIRE(g.removeNode(1));
    REQUIRE(g.nodeCount() == 0u);
    REQUIRE(!g.removeNode(1));
}

TEST_CASE("GameFlowGraph compile state", "[Editor][S100]") {
    GameFlowGraph g("Level1Flow");
    REQUIRE(g.compileState() == GameFlowCompileState::Dirty);
    REQUIRE(!g.isCompiled());
    g.setCompileState(GameFlowCompileState::Ok);
    REQUIRE(g.isCompiled());
}

TEST_CASE("GameFlowGraph counts", "[Editor][S100]") {
    GameFlowGraph g("flow");
    GameFlowNode n1(1, GameFlowNodeKind::Start);
    GameFlowNode n2(2, GameFlowNodeKind::GameState);
    GameFlowNode n3(3, GameFlowNodeKind::GameState); n3.setBreakpoint(true);
    GameFlowNode n4(4, GameFlowNodeKind::End); n4.setEnabled(false);
    g.addNode(n1); g.addNode(n2); g.addNode(n3); g.addNode(n4);
    REQUIRE(g.nodeCount()                               == 4u);
    REQUIRE(g.enabledNodeCount()                        == 3u);
    REQUIRE(g.breakpointCount()                         == 1u);
    REQUIRE(g.countByKind(GameFlowNodeKind::GameState)  == 2u);
}

// ── StateGraphEditor ─────────────────────────────────────────────────────────

TEST_CASE("StateGraphNodeRole names", "[Editor][S100]") {
    REQUIRE(std::string(stateGraphNodeRoleName(StateGraphNodeRole::Entry))    == "Entry");
    REQUIRE(std::string(stateGraphNodeRoleName(StateGraphNodeRole::State))    == "State");
    REQUIRE(std::string(stateGraphNodeRoleName(StateGraphNodeRole::AnyState)) == "AnyState");
    REQUIRE(std::string(stateGraphNodeRoleName(StateGraphNodeRole::Exit))     == "Exit");
    REQUIRE(std::string(stateGraphNodeRoleName(StateGraphNodeRole::Conduit))  == "Conduit");
}

TEST_CASE("StateTransitionCondOp names", "[Editor][S100]") {
    REQUIRE(std::string(stateTransitionCondOpName(StateTransitionCondOp::Equal))        == "Equal");
    REQUIRE(std::string(stateTransitionCondOpName(StateTransitionCondOp::NotEqual))     == "NotEqual");
    REQUIRE(std::string(stateTransitionCondOpName(StateTransitionCondOp::GreaterThan))  == "GreaterThan");
    REQUIRE(std::string(stateTransitionCondOpName(StateTransitionCondOp::LessThan))     == "LessThan");
    REQUIRE(std::string(stateTransitionCondOpName(StateTransitionCondOp::GreaterEqual)) == "GreaterEqual");
    REQUIRE(std::string(stateTransitionCondOpName(StateTransitionCondOp::LessEqual))    == "LessEqual");
    REQUIRE(std::string(stateTransitionCondOpName(StateTransitionCondOp::Always))       == "Always");
}

TEST_CASE("StateGraphCompileResult names", "[Editor][S100]") {
    REQUIRE(std::string(stateGraphCompileResultName(StateGraphCompileResult::Ok))          == "Ok");
    REQUIRE(std::string(stateGraphCompileResultName(StateGraphCompileResult::HasErrors))   == "HasErrors");
    REQUIRE(std::string(stateGraphCompileResultName(StateGraphCompileResult::HasWarnings)) == "HasWarnings");
    REQUIRE(std::string(stateGraphCompileResultName(StateGraphCompileResult::NotCompiled)) == "NotCompiled");
}

TEST_CASE("StateGraphNode defaults", "[Editor][S100]") {
    StateGraphNode n(1, StateGraphNodeRole::State);
    REQUIRE(n.id()   == 1u);
    REQUIRE(n.role() == StateGraphNodeRole::State);
    REQUIRE(n.isEnabled());
}

TEST_CASE("StateGraphTransition defaults", "[Editor][S100]") {
    StateGraphTransition t(1, 2, StateTransitionCondOp::Always);
    REQUIRE(t.fromId()      == 1u);
    REQUIRE(t.toId()        == 2u);
    REQUIRE(t.op()          == StateTransitionCondOp::Always);
    REQUIRE(t.isEnabled());
    REQUIRE(t.priority()    == 0);
    REQUIRE(!t.hasExitTime());
}

TEST_CASE("StateGraphTransition mutation", "[Editor][S100]") {
    StateGraphTransition t(3, 4, StateTransitionCondOp::GreaterThan);
    t.setEnabled(false);
    t.setPriority(5);
    t.setHasExitTime(true);
    REQUIRE(!t.isEnabled());
    REQUIRE(t.priority()    == 5);
    REQUIRE(t.hasExitTime());
}

TEST_CASE("StateGraphEditor add nodes", "[Editor][S100]") {
    StateGraphEditor ed;
    REQUIRE(ed.addNode(StateGraphNode(1, StateGraphNodeRole::Entry)));
    REQUIRE(ed.addNode(StateGraphNode(2, StateGraphNodeRole::State)));
    REQUIRE(!ed.addNode(StateGraphNode(1, StateGraphNodeRole::AnyState)));
    REQUIRE(ed.nodeCount() == 2u);
}

TEST_CASE("StateGraphEditor add transitions", "[Editor][S100]") {
    StateGraphEditor ed;
    REQUIRE(ed.addTransition(StateGraphTransition(1, 2, StateTransitionCondOp::Always)));
    REQUIRE(ed.addTransition(StateGraphTransition(2, 3, StateTransitionCondOp::Equal)));
    REQUIRE(ed.transitionCount() == 2u);
}

TEST_CASE("StateGraphEditor compile result", "[Editor][S100]") {
    StateGraphEditor ed;
    REQUIRE(ed.compileResult() == StateGraphCompileResult::NotCompiled);
    REQUIRE(!ed.isValid());
    ed.setCompileResult(StateGraphCompileResult::Ok);
    REQUIRE(ed.isValid());
}

TEST_CASE("StateGraphEditor counts", "[Editor][S100]") {
    StateGraphEditor ed;
    ed.addNode(StateGraphNode(1, StateGraphNodeRole::Entry));
    ed.addNode(StateGraphNode(2, StateGraphNodeRole::State));
    ed.addNode(StateGraphNode(3, StateGraphNodeRole::State));
    ed.addNode(StateGraphNode(4, StateGraphNodeRole::Exit));
    StateGraphTransition t1(1, 2, StateTransitionCondOp::Always);
    StateGraphTransition t2(2, 3, StateTransitionCondOp::Equal); t2.setEnabled(false);
    ed.addTransition(t1); ed.addTransition(t2);
    REQUIRE(ed.nodeCount()                               == 4u);
    REQUIRE(ed.countByRole(StateGraphNodeRole::State)    == 2u);
    REQUIRE(ed.enabledTransitionCount()                  == 1u);
}

// ── TriggerVolumeEditor ──────────────────────────────────────────────────────

TEST_CASE("TriggerVolumeShape names", "[Editor][S100]") {
    REQUIRE(std::string(triggerVolumeShapeName(TriggerVolumeShape::Box))     == "Box");
    REQUIRE(std::string(triggerVolumeShapeName(TriggerVolumeShape::Sphere))  == "Sphere");
    REQUIRE(std::string(triggerVolumeShapeName(TriggerVolumeShape::Capsule)) == "Capsule");
    REQUIRE(std::string(triggerVolumeShapeName(TriggerVolumeShape::Cylinder))== "Cylinder");
    REQUIRE(std::string(triggerVolumeShapeName(TriggerVolumeShape::Convex))  == "Convex");
}

TEST_CASE("TriggerVolumeEvent names", "[Editor][S100]") {
    REQUIRE(std::string(triggerVolumeEventName(TriggerVolumeEvent::OnEnter))     == "OnEnter");
    REQUIRE(std::string(triggerVolumeEventName(TriggerVolumeEvent::OnExit))      == "OnExit");
    REQUIRE(std::string(triggerVolumeEventName(TriggerVolumeEvent::OnStay))      == "OnStay");
    REQUIRE(std::string(triggerVolumeEventName(TriggerVolumeEvent::OnEnterExit)) == "OnEnterExit");
}

TEST_CASE("TriggerVolumeFilterMode names", "[Editor][S100]") {
    REQUIRE(std::string(triggerVolumeFilterModeName(TriggerVolumeFilterMode::All))     == "All");
    REQUIRE(std::string(triggerVolumeFilterModeName(TriggerVolumeFilterMode::Player))  == "Player");
    REQUIRE(std::string(triggerVolumeFilterModeName(TriggerVolumeFilterMode::NPC))     == "NPC");
    REQUIRE(std::string(triggerVolumeFilterModeName(TriggerVolumeFilterMode::Physics)) == "Physics");
    REQUIRE(std::string(triggerVolumeFilterModeName(TriggerVolumeFilterMode::Custom))  == "Custom");
}

TEST_CASE("TriggerVolume defaults", "[Editor][S100]") {
    TriggerVolume v("spawn_zone", TriggerVolumeShape::Box);
    REQUIRE(v.name()        == "spawn_zone");
    REQUIRE(v.shape()       == TriggerVolumeShape::Box);
    REQUIRE(v.event()       == TriggerVolumeEvent::OnEnter);
    REQUIRE(v.filterMode()  == TriggerVolumeFilterMode::All);
    REQUIRE(v.isEnabled());
    REQUIRE(!v.isDebugVisible());
    REQUIRE(!v.repeats());
    REQUIRE(v.cooldown()    == 0.0f);
}

TEST_CASE("TriggerVolume mutation", "[Editor][S100]") {
    TriggerVolume v("death_zone", TriggerVolumeShape::Sphere);
    v.setEvent(TriggerVolumeEvent::OnStay);
    v.setFilterMode(TriggerVolumeFilterMode::Player);
    v.setEnabled(false);
    v.setDebugVisible(true);
    v.setRepeat(true);
    v.setCooldown(2.0f);
    REQUIRE(v.event()      == TriggerVolumeEvent::OnStay);
    REQUIRE(v.filterMode() == TriggerVolumeFilterMode::Player);
    REQUIRE(!v.isEnabled());
    REQUIRE(v.isDebugVisible());
    REQUIRE(v.repeats());
    REQUIRE(v.cooldown()   == 2.0f);
}

TEST_CASE("TriggerVolumeEditor add/remove", "[Editor][S100]") {
    TriggerVolumeEditor ed;
    TriggerVolume v("zone1", TriggerVolumeShape::Box);
    REQUIRE(ed.addVolume(v));
    REQUIRE(ed.volumeCount() == 1u);
    REQUIRE(!ed.addVolume(v));
    REQUIRE(ed.removeVolume("zone1"));
    REQUIRE(ed.volumeCount() == 0u);
    REQUIRE(!ed.removeVolume("zone1"));
}

TEST_CASE("TriggerVolumeEditor counts", "[Editor][S100]") {
    TriggerVolumeEditor ed;
    TriggerVolume v1("a", TriggerVolumeShape::Box);
    v1.setEvent(TriggerVolumeEvent::OnEnter); v1.setFilterMode(TriggerVolumeFilterMode::Player); v1.setDebugVisible(true);
    TriggerVolume v2("b", TriggerVolumeShape::Sphere);
    v2.setEvent(TriggerVolumeEvent::OnEnter); v2.setEnabled(false);
    TriggerVolume v3("c", TriggerVolumeShape::Box);
    v3.setEvent(TriggerVolumeEvent::OnExit);  v3.setFilterMode(TriggerVolumeFilterMode::NPC);
    ed.addVolume(v1); ed.addVolume(v2); ed.addVolume(v3);
    REQUIRE(ed.volumeCount()                                     == 3u);
    REQUIRE(ed.enabledCount()                                    == 2u);
    REQUIRE(ed.countByShape(TriggerVolumeShape::Box)             == 2u);
    REQUIRE(ed.countByEvent(TriggerVolumeEvent::OnEnter)         == 2u);
    REQUIRE(ed.countByFilter(TriggerVolumeFilterMode::Player)    == 1u);
    REQUIRE(ed.debugVisibleCount()                               == 1u);
}

TEST_CASE("TriggerVolumeEditor find", "[Editor][S100]") {
    TriggerVolumeEditor ed;
    TriggerVolume v("puzzle_zone", TriggerVolumeShape::Capsule);
    ed.addVolume(v);
    auto* found = ed.findVolume("puzzle_zone");
    REQUIRE(found != nullptr);
    REQUIRE(found->shape() == TriggerVolumeShape::Capsule);
    REQUIRE(ed.findVolume("missing") == nullptr);
}
