// S178 editor tests: DialogueTreeEditorV1, NarrativeBranchEditorV1, QuestScriptEditorV1
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/DialogueTreeEditorV1.h"
#include "NF/Editor/NarrativeBranchEditorV1.h"
#include "NF/Editor/QuestScriptEditorV1.h"

using namespace NF;
using Catch::Approx;

// ── DialogueTreeEditorV1 ─────────────────────────────────────────────────────

TEST_CASE("Dtev1Node validity", "[Editor][S178]") {
    Dtev1Node n;
    REQUIRE(!n.isValid());
    n.id = 1; n.name = "Start";
    REQUIRE(n.isValid());
}

TEST_CASE("DialogueTreeEditorV1 addNode and nodeCount", "[Editor][S178]") {
    DialogueTreeEditorV1 dte;
    REQUIRE(dte.nodeCount() == 0);
    Dtev1Node n; n.id = 1; n.name = "Hello";
    REQUIRE(dte.addNode(n));
    REQUIRE(dte.nodeCount() == 1);
}

TEST_CASE("DialogueTreeEditorV1 addNode invalid fails", "[Editor][S178]") {
    DialogueTreeEditorV1 dte;
    REQUIRE(!dte.addNode(Dtev1Node{}));
}

TEST_CASE("DialogueTreeEditorV1 addNode duplicate fails", "[Editor][S178]") {
    DialogueTreeEditorV1 dte;
    Dtev1Node n; n.id = 1; n.name = "A";
    dte.addNode(n);
    REQUIRE(!dte.addNode(n));
}

TEST_CASE("DialogueTreeEditorV1 removeNode", "[Editor][S178]") {
    DialogueTreeEditorV1 dte;
    Dtev1Node n; n.id = 2; n.name = "B";
    dte.addNode(n);
    REQUIRE(dte.removeNode(2));
    REQUIRE(dte.nodeCount() == 0);
    REQUIRE(!dte.removeNode(2));
}

TEST_CASE("DialogueTreeEditorV1 addEdge and edgeCount", "[Editor][S178]") {
    DialogueTreeEditorV1 dte;
    Dtev1Edge e; e.id = 1; e.fromNode = 1; e.toNode = 2; e.label = "yes";
    REQUIRE(dte.addEdge(e));
    REQUIRE(dte.edgeCount() == 1);
}

TEST_CASE("DialogueTreeEditorV1 addEdge invalid fails", "[Editor][S178]") {
    DialogueTreeEditorV1 dte;
    REQUIRE(!dte.addEdge(Dtev1Edge{}));
}

TEST_CASE("DialogueTreeEditorV1 removeEdge", "[Editor][S178]") {
    DialogueTreeEditorV1 dte;
    Dtev1Edge e; e.id = 1; e.fromNode = 1; e.toNode = 2;
    dte.addEdge(e);
    REQUIRE(dte.removeEdge(1));
    REQUIRE(dte.edgeCount() == 0);
    REQUIRE(!dte.removeEdge(1));
}

TEST_CASE("DialogueTreeEditorV1 approvedCount and lockedCount", "[Editor][S178]") {
    DialogueTreeEditorV1 dte;
    Dtev1Node n1; n1.id = 1; n1.name = "A"; n1.state = Dtev1NodeState::Approved;
    Dtev1Node n2; n2.id = 2; n2.name = "B"; n2.state = Dtev1NodeState::Locked;
    dte.addNode(n1); dte.addNode(n2);
    REQUIRE(dte.approvedCount() == 1);
    REQUIRE(dte.lockedCount()   == 1);
}

TEST_CASE("DialogueTreeEditorV1 countByNodeType", "[Editor][S178]") {
    DialogueTreeEditorV1 dte;
    Dtev1Node n1; n1.id = 1; n1.name = "A"; n1.type = Dtev1NodeType::Choice;
    Dtev1Node n2; n2.id = 2; n2.name = "B"; n2.type = Dtev1NodeType::Exit;
    dte.addNode(n1); dte.addNode(n2);
    REQUIRE(dte.countByNodeType(Dtev1NodeType::Choice) == 1);
    REQUIRE(dte.countByNodeType(Dtev1NodeType::Exit)   == 1);
}

TEST_CASE("DialogueTreeEditorV1 findNode returns ptr", "[Editor][S178]") {
    DialogueTreeEditorV1 dte;
    Dtev1Node n; n.id = 5; n.name = "Entry";
    dte.addNode(n);
    REQUIRE(dte.findNode(5) != nullptr);
    REQUIRE(dte.findNode(5)->name == "Entry");
    REQUIRE(dte.findNode(99) == nullptr);
}

TEST_CASE("dtev1NodeTypeName covers all values", "[Editor][S178]") {
    REQUIRE(std::string(dtev1NodeTypeName(Dtev1NodeType::Entry))     == "Entry");
    REQUIRE(std::string(dtev1NodeTypeName(Dtev1NodeType::Choice))    == "Choice");
    REQUIRE(std::string(dtev1NodeTypeName(Dtev1NodeType::Condition)) == "Condition");
    REQUIRE(std::string(dtev1NodeTypeName(Dtev1NodeType::Exit))      == "Exit");
}

TEST_CASE("dtev1NodeStateName covers all values", "[Editor][S178]") {
    REQUIRE(std::string(dtev1NodeStateName(Dtev1NodeState::Draft))      == "Draft");
    REQUIRE(std::string(dtev1NodeStateName(Dtev1NodeState::Approved))   == "Approved");
    REQUIRE(std::string(dtev1NodeStateName(Dtev1NodeState::Deprecated)) == "Deprecated");
}

TEST_CASE("Dtev1Node state helpers", "[Editor][S178]") {
    Dtev1Node n; n.id = 1; n.name = "X";
    n.state = Dtev1NodeState::Approved;
    REQUIRE(n.isApproved());
    n.state = Dtev1NodeState::Locked;
    REQUIRE(n.isLocked());
}

TEST_CASE("DialogueTreeEditorV1 onChange callback", "[Editor][S178]") {
    DialogueTreeEditorV1 dte;
    uint64_t notified = 0;
    dte.setOnChange([&](uint64_t id) { notified = id; });
    Dtev1Node n; n.id = 7; n.name = "Choice";
    dte.addNode(n);
    REQUIRE(notified == 7);
}

// ── NarrativeBranchEditorV1 ──────────────────────────────────────────────────

TEST_CASE("Nbrv1Branch validity", "[Editor][S178]") {
    Nbrv1Branch b;
    REQUIRE(!b.isValid());
    b.id = 1; b.name = "Opening";
    REQUIRE(b.isValid());
}

TEST_CASE("NarrativeBranchEditorV1 addBranch and branchCount", "[Editor][S178]") {
    NarrativeBranchEditorV1 nbr;
    REQUIRE(nbr.branchCount() == 0);
    Nbrv1Branch b; b.id = 1; b.name = "Act1";
    REQUIRE(nbr.addBranch(b));
    REQUIRE(nbr.branchCount() == 1);
}

TEST_CASE("NarrativeBranchEditorV1 addBranch invalid fails", "[Editor][S178]") {
    NarrativeBranchEditorV1 nbr;
    REQUIRE(!nbr.addBranch(Nbrv1Branch{}));
}

TEST_CASE("NarrativeBranchEditorV1 addBranch duplicate fails", "[Editor][S178]") {
    NarrativeBranchEditorV1 nbr;
    Nbrv1Branch b; b.id = 1; b.name = "A";
    nbr.addBranch(b);
    REQUIRE(!nbr.addBranch(b));
}

TEST_CASE("NarrativeBranchEditorV1 removeBranch", "[Editor][S178]") {
    NarrativeBranchEditorV1 nbr;
    Nbrv1Branch b; b.id = 2; b.name = "B";
    nbr.addBranch(b);
    REQUIRE(nbr.removeBranch(2));
    REQUIRE(nbr.branchCount() == 0);
    REQUIRE(!nbr.removeBranch(2));
}

TEST_CASE("NarrativeBranchEditorV1 addDecision and decisionCount", "[Editor][S178]") {
    NarrativeBranchEditorV1 nbr;
    Nbrv1Decision d; d.id = 1; d.branchId = 10; d.text = "Go left";
    REQUIRE(nbr.addDecision(d));
    REQUIRE(nbr.decisionCount() == 1);
}

TEST_CASE("NarrativeBranchEditorV1 addDecision invalid fails", "[Editor][S178]") {
    NarrativeBranchEditorV1 nbr;
    REQUIRE(!nbr.addDecision(Nbrv1Decision{}));
}

TEST_CASE("NarrativeBranchEditorV1 activeCount and completeCount", "[Editor][S178]") {
    NarrativeBranchEditorV1 nbr;
    Nbrv1Branch b1; b1.id = 1; b1.name = "A"; b1.state = Nbrv1BranchState::Active;
    Nbrv1Branch b2; b2.id = 2; b2.name = "B"; b2.state = Nbrv1BranchState::Complete;
    nbr.addBranch(b1); nbr.addBranch(b2);
    REQUIRE(nbr.activeCount()   == 1);
    REQUIRE(nbr.completeCount() == 1);
}

TEST_CASE("NarrativeBranchEditorV1 countByBranchType", "[Editor][S178]") {
    NarrativeBranchEditorV1 nbr;
    Nbrv1Branch b1; b1.id = 1; b1.name = "A"; b1.type = Nbrv1BranchType::Choice;
    Nbrv1Branch b2; b2.id = 2; b2.name = "B"; b2.type = Nbrv1BranchType::Random;
    nbr.addBranch(b1); nbr.addBranch(b2);
    REQUIRE(nbr.countByBranchType(Nbrv1BranchType::Choice) == 1);
    REQUIRE(nbr.countByBranchType(Nbrv1BranchType::Random) == 1);
}

TEST_CASE("NarrativeBranchEditorV1 findBranch returns ptr", "[Editor][S178]") {
    NarrativeBranchEditorV1 nbr;
    Nbrv1Branch b; b.id = 6; b.name = "Climax";
    nbr.addBranch(b);
    REQUIRE(nbr.findBranch(6) != nullptr);
    REQUIRE(nbr.findBranch(6)->name == "Climax");
    REQUIRE(nbr.findBranch(99) == nullptr);
}

TEST_CASE("nbrv1BranchTypeName covers all values", "[Editor][S178]") {
    REQUIRE(std::string(nbrv1BranchTypeName(Nbrv1BranchType::Linear))      == "Linear");
    REQUIRE(std::string(nbrv1BranchTypeName(Nbrv1BranchType::Conditional)) == "Conditional");
    REQUIRE(std::string(nbrv1BranchTypeName(Nbrv1BranchType::Event))       == "Event");
}

TEST_CASE("nbrv1BranchStateName covers all values", "[Editor][S178]") {
    REQUIRE(std::string(nbrv1BranchStateName(Nbrv1BranchState::Inactive)) == "Inactive");
    REQUIRE(std::string(nbrv1BranchStateName(Nbrv1BranchState::Hidden))   == "Hidden");
}

TEST_CASE("NarrativeBranchEditorV1 onChange callback", "[Editor][S178]") {
    NarrativeBranchEditorV1 nbr;
    uint64_t notified = 0;
    nbr.setOnChange([&](uint64_t id) { notified = id; });
    Nbrv1Branch b; b.id = 3; b.name = "C";
    nbr.addBranch(b);
    REQUIRE(notified == 3);
}

// ── QuestScriptEditorV1 ──────────────────────────────────────────────────────

TEST_CASE("Qscv1Script validity", "[Editor][S178]") {
    Qscv1Script s;
    REQUIRE(!s.isValid());
    s.id = 1; s.name = "MainQuest";
    REQUIRE(s.isValid());
}

TEST_CASE("QuestScriptEditorV1 addScript and scriptCount", "[Editor][S178]") {
    QuestScriptEditorV1 qsc;
    REQUIRE(qsc.scriptCount() == 0);
    Qscv1Script s; s.id = 1; s.name = "Q1";
    REQUIRE(qsc.addScript(s));
    REQUIRE(qsc.scriptCount() == 1);
}

TEST_CASE("QuestScriptEditorV1 addScript invalid fails", "[Editor][S178]") {
    QuestScriptEditorV1 qsc;
    REQUIRE(!qsc.addScript(Qscv1Script{}));
}

TEST_CASE("QuestScriptEditorV1 addScript duplicate fails", "[Editor][S178]") {
    QuestScriptEditorV1 qsc;
    Qscv1Script s; s.id = 1; s.name = "A";
    qsc.addScript(s);
    REQUIRE(!qsc.addScript(s));
}

TEST_CASE("QuestScriptEditorV1 removeScript", "[Editor][S178]") {
    QuestScriptEditorV1 qsc;
    Qscv1Script s; s.id = 2; s.name = "B";
    qsc.addScript(s);
    REQUIRE(qsc.removeScript(2));
    REQUIRE(qsc.scriptCount() == 0);
    REQUIRE(!qsc.removeScript(2));
}

TEST_CASE("QuestScriptEditorV1 addStep and stepCount", "[Editor][S178]") {
    QuestScriptEditorV1 qsc;
    Qscv1Step st; st.id = 1; st.scriptId = 10; st.description = "Talk to NPC";
    REQUIRE(qsc.addStep(st));
    REQUIRE(qsc.stepCount() == 1);
}

TEST_CASE("QuestScriptEditorV1 addStep invalid fails", "[Editor][S178]") {
    QuestScriptEditorV1 qsc;
    REQUIRE(!qsc.addStep(Qscv1Step{}));
}

TEST_CASE("QuestScriptEditorV1 activeCount and completeCount", "[Editor][S178]") {
    QuestScriptEditorV1 qsc;
    Qscv1Script s1; s1.id = 1; s1.name = "A"; s1.state = Qscv1ScriptState::Active;
    Qscv1Script s2; s2.id = 2; s2.name = "B"; s2.state = Qscv1ScriptState::Complete;
    qsc.addScript(s1); qsc.addScript(s2);
    REQUIRE(qsc.activeCount()   == 1);
    REQUIRE(qsc.completeCount() == 1);
}

TEST_CASE("QuestScriptEditorV1 countByStepType", "[Editor][S178]") {
    QuestScriptEditorV1 qsc;
    Qscv1Step st1; st1.id = 1; st1.scriptId = 1; st1.description = "X"; st1.type = Qscv1StepType::Trigger;
    Qscv1Step st2; st2.id = 2; st2.scriptId = 1; st2.description = "Y"; st2.type = Qscv1StepType::Wait;
    qsc.addStep(st1); qsc.addStep(st2);
    REQUIRE(qsc.countByStepType(Qscv1StepType::Trigger) == 1);
    REQUIRE(qsc.countByStepType(Qscv1StepType::Wait)    == 1);
}

TEST_CASE("QuestScriptEditorV1 findScript returns ptr", "[Editor][S178]") {
    QuestScriptEditorV1 qsc;
    Qscv1Script s; s.id = 8; s.name = "SideQuest";
    qsc.addScript(s);
    REQUIRE(qsc.findScript(8) != nullptr);
    REQUIRE(qsc.findScript(8)->name == "SideQuest");
    REQUIRE(qsc.findScript(99) == nullptr);
}

TEST_CASE("qscv1ScriptStateName covers all values", "[Editor][S178]") {
    REQUIRE(std::string(qscv1ScriptStateName(Qscv1ScriptState::Draft))     == "Draft");
    REQUIRE(std::string(qscv1ScriptStateName(Qscv1ScriptState::Failed))    == "Failed");
    REQUIRE(std::string(qscv1ScriptStateName(Qscv1ScriptState::Abandoned)) == "Abandoned");
}

TEST_CASE("qscv1StepTypeName covers all values", "[Editor][S178]") {
    REQUIRE(std::string(qscv1StepTypeName(Qscv1StepType::Trigger)) == "Trigger");
    REQUIRE(std::string(qscv1StepTypeName(Qscv1StepType::Branch))  == "Branch");
    REQUIRE(std::string(qscv1StepTypeName(Qscv1StepType::End))     == "End");
}

TEST_CASE("Qscv1Script state helpers", "[Editor][S178]") {
    Qscv1Script s; s.id = 1; s.name = "X";
    s.state = Qscv1ScriptState::Active;
    REQUIRE(s.isActive());
    s.state = Qscv1ScriptState::Complete;
    REQUIRE(s.isComplete());
}

TEST_CASE("QuestScriptEditorV1 onChange callback", "[Editor][S178]") {
    QuestScriptEditorV1 qsc;
    uint64_t notified = 0;
    qsc.setOnChange([&](uint64_t id) { notified = id; });
    Qscv1Script s; s.id = 4; s.name = "D";
    qsc.addScript(s);
    REQUIRE(notified == 4);
}
