// S192 editor tests: DialogueNodeEditorV1, CutsceneEditorV1, LocalizationTableEditorV1
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/DialogueNodeEditorV1.h"
#include "NF/Editor/CutsceneEditorV1.h"
#include "NF/Editor/LocalizationTableEditorV1.h"

using namespace NF;
using Catch::Approx;

// ── DialogueNodeEditorV1 ─────────────────────────────────────────────────────

TEST_CASE("Dnev1DialogueNode validity", "[Editor][S192]") {
    Dnev1DialogueNode n;
    REQUIRE(!n.isValid());
    n.id = 1; n.name = "Greeting";
    REQUIRE(n.isValid());
}

TEST_CASE("DialogueNodeEditorV1 addNode and nodeCount", "[Editor][S192]") {
    DialogueNodeEditorV1 dne;
    REQUIRE(dne.nodeCount() == 0);
    Dnev1DialogueNode n; n.id = 1; n.name = "N1";
    REQUIRE(dne.addNode(n));
    REQUIRE(dne.nodeCount() == 1);
}

TEST_CASE("DialogueNodeEditorV1 addNode invalid fails", "[Editor][S192]") {
    DialogueNodeEditorV1 dne;
    REQUIRE(!dne.addNode(Dnev1DialogueNode{}));
}

TEST_CASE("DialogueNodeEditorV1 addNode duplicate fails", "[Editor][S192]") {
    DialogueNodeEditorV1 dne;
    Dnev1DialogueNode n; n.id = 1; n.name = "A";
    dne.addNode(n);
    REQUIRE(!dne.addNode(n));
}

TEST_CASE("DialogueNodeEditorV1 removeNode", "[Editor][S192]") {
    DialogueNodeEditorV1 dne;
    Dnev1DialogueNode n; n.id = 2; n.name = "B";
    dne.addNode(n);
    REQUIRE(dne.removeNode(2));
    REQUIRE(dne.nodeCount() == 0);
    REQUIRE(!dne.removeNode(2));
}

TEST_CASE("DialogueNodeEditorV1 findNode", "[Editor][S192]") {
    DialogueNodeEditorV1 dne;
    Dnev1DialogueNode n; n.id = 3; n.name = "C";
    dne.addNode(n);
    REQUIRE(dne.findNode(3) != nullptr);
    REQUIRE(dne.findNode(99) == nullptr);
}

TEST_CASE("DialogueNodeEditorV1 addBranch and branchCount", "[Editor][S192]") {
    DialogueNodeEditorV1 dne;
    Dnev1DialogueBranch b; b.id = 1; b.fromNodeId = 1; b.toNodeId = 2;
    REQUIRE(dne.addBranch(b));
    REQUIRE(dne.branchCount() == 1);
}

TEST_CASE("DialogueNodeEditorV1 removeBranch", "[Editor][S192]") {
    DialogueNodeEditorV1 dne;
    Dnev1DialogueBranch b; b.id = 1; b.fromNodeId = 1; b.toNodeId = 2;
    dne.addBranch(b);
    REQUIRE(dne.removeBranch(1));
    REQUIRE(dne.branchCount() == 0);
}

TEST_CASE("DialogueNodeEditorV1 activeCount", "[Editor][S192]") {
    DialogueNodeEditorV1 dne;
    Dnev1DialogueNode n1; n1.id = 1; n1.name = "A"; n1.state = Dnev1NodeState::Active;
    Dnev1DialogueNode n2; n2.id = 2; n2.name = "B";
    dne.addNode(n1); dne.addNode(n2);
    REQUIRE(dne.activeCount() == 1);
}

TEST_CASE("DialogueNodeEditorV1 visitedCount", "[Editor][S192]") {
    DialogueNodeEditorV1 dne;
    Dnev1DialogueNode n1; n1.id = 1; n1.name = "A"; n1.state = Dnev1NodeState::Visited;
    Dnev1DialogueNode n2; n2.id = 2; n2.name = "B"; n2.state = Dnev1NodeState::Active;
    dne.addNode(n1); dne.addNode(n2);
    REQUIRE(dne.visitedCount() == 1);
}

TEST_CASE("DialogueNodeEditorV1 countByType", "[Editor][S192]") {
    DialogueNodeEditorV1 dne;
    Dnev1DialogueNode n1; n1.id = 1; n1.name = "A"; n1.nodeType = Dnev1NodeType::NPC;
    Dnev1DialogueNode n2; n2.id = 2; n2.name = "B"; n2.nodeType = Dnev1NodeType::Player;
    Dnev1DialogueNode n3; n3.id = 3; n3.name = "C"; n3.nodeType = Dnev1NodeType::NPC;
    dne.addNode(n1); dne.addNode(n2); dne.addNode(n3);
    REQUIRE(dne.countByType(Dnev1NodeType::NPC) == 2);
    REQUIRE(dne.countByType(Dnev1NodeType::Player) == 1);
}

TEST_CASE("DialogueNodeEditorV1 branchesFromNode", "[Editor][S192]") {
    DialogueNodeEditorV1 dne;
    Dnev1DialogueBranch b1; b1.id = 1; b1.fromNodeId = 10; b1.toNodeId = 11;
    Dnev1DialogueBranch b2; b2.id = 2; b2.fromNodeId = 10; b2.toNodeId = 12;
    Dnev1DialogueBranch b3; b3.id = 3; b3.fromNodeId = 20; b3.toNodeId = 21;
    dne.addBranch(b1); dne.addBranch(b2); dne.addBranch(b3);
    REQUIRE(dne.branchesFromNode(10) == 2);
    REQUIRE(dne.branchesFromNode(20) == 1);
}

TEST_CASE("DialogueNodeEditorV1 onChange callback", "[Editor][S192]") {
    DialogueNodeEditorV1 dne;
    uint64_t notified = 0;
    dne.setOnChange([&](uint64_t id) { notified = id; });
    Dnev1DialogueNode n; n.id = 5; n.name = "E";
    dne.addNode(n);
    REQUIRE(notified == 5);
}

TEST_CASE("Dnev1DialogueNode state helpers", "[Editor][S192]") {
    Dnev1DialogueNode n; n.id = 1; n.name = "X";
    n.state = Dnev1NodeState::Active;  REQUIRE(n.isActive());
    n.state = Dnev1NodeState::Visited; REQUIRE(n.isVisited());
    n.state = Dnev1NodeState::Locked;  REQUIRE(n.isLocked());
}

TEST_CASE("dnev1NodeTypeName all values", "[Editor][S192]") {
    REQUIRE(std::string(dnev1NodeTypeName(Dnev1NodeType::NPC))       == "NPC");
    REQUIRE(std::string(dnev1NodeTypeName(Dnev1NodeType::Player))    == "Player");
    REQUIRE(std::string(dnev1NodeTypeName(Dnev1NodeType::Narrator))  == "Narrator");
    REQUIRE(std::string(dnev1NodeTypeName(Dnev1NodeType::Choice))    == "Choice");
    REQUIRE(std::string(dnev1NodeTypeName(Dnev1NodeType::Action))    == "Action");
    REQUIRE(std::string(dnev1NodeTypeName(Dnev1NodeType::Condition)) == "Condition");
}

// ── CutsceneEditorV1 ─────────────────────────────────────────────────────────

TEST_CASE("Ccev1CutsceneTrack validity", "[Editor][S192]") {
    Ccev1CutsceneTrack t;
    REQUIRE(!t.isValid());
    t.id = 1; t.name = "MainCam";
    REQUIRE(t.isValid());
}

TEST_CASE("CutsceneEditorV1 addTrack and trackCount", "[Editor][S192]") {
    CutsceneEditorV1 cce;
    REQUIRE(cce.trackCount() == 0);
    Ccev1CutsceneTrack t; t.id = 1; t.name = "T1";
    REQUIRE(cce.addTrack(t));
    REQUIRE(cce.trackCount() == 1);
}

TEST_CASE("CutsceneEditorV1 addTrack invalid fails", "[Editor][S192]") {
    CutsceneEditorV1 cce;
    REQUIRE(!cce.addTrack(Ccev1CutsceneTrack{}));
}

TEST_CASE("CutsceneEditorV1 addTrack duplicate fails", "[Editor][S192]") {
    CutsceneEditorV1 cce;
    Ccev1CutsceneTrack t; t.id = 1; t.name = "A";
    cce.addTrack(t);
    REQUIRE(!cce.addTrack(t));
}

TEST_CASE("CutsceneEditorV1 removeTrack", "[Editor][S192]") {
    CutsceneEditorV1 cce;
    Ccev1CutsceneTrack t; t.id = 2; t.name = "B";
    cce.addTrack(t);
    REQUIRE(cce.removeTrack(2));
    REQUIRE(cce.trackCount() == 0);
    REQUIRE(!cce.removeTrack(2));
}

TEST_CASE("CutsceneEditorV1 findTrack", "[Editor][S192]") {
    CutsceneEditorV1 cce;
    Ccev1CutsceneTrack t; t.id = 3; t.name = "C";
    cce.addTrack(t);
    REQUIRE(cce.findTrack(3) != nullptr);
    REQUIRE(cce.findTrack(99) == nullptr);
}

TEST_CASE("CutsceneEditorV1 addKeyframe and keyframeCount", "[Editor][S192]") {
    CutsceneEditorV1 cce;
    Ccev1CutsceneKeyframe kf; kf.id = 1; kf.trackId = 10; kf.timeMs = 500.0f;
    REQUIRE(cce.addKeyframe(kf));
    REQUIRE(cce.keyframeCount() == 1);
}

TEST_CASE("CutsceneEditorV1 removeKeyframe", "[Editor][S192]") {
    CutsceneEditorV1 cce;
    Ccev1CutsceneKeyframe kf; kf.id = 1; kf.trackId = 10; kf.timeMs = 1000.0f;
    cce.addKeyframe(kf);
    REQUIRE(cce.removeKeyframe(1));
    REQUIRE(cce.keyframeCount() == 0);
}

TEST_CASE("CutsceneEditorV1 visibleCount", "[Editor][S192]") {
    CutsceneEditorV1 cce;
    Ccev1CutsceneTrack t1; t1.id = 1; t1.name = "A"; t1.state = Ccev1TrackState::Visible;
    Ccev1CutsceneTrack t2; t2.id = 2; t2.name = "B";
    cce.addTrack(t1); cce.addTrack(t2);
    REQUIRE(cce.visibleCount() == 1);
}

TEST_CASE("CutsceneEditorV1 lockedCount", "[Editor][S192]") {
    CutsceneEditorV1 cce;
    Ccev1CutsceneTrack t1; t1.id = 1; t1.name = "A"; t1.state = Ccev1TrackState::Locked;
    Ccev1CutsceneTrack t2; t2.id = 2; t2.name = "B"; t2.state = Ccev1TrackState::Visible;
    cce.addTrack(t1); cce.addTrack(t2);
    REQUIRE(cce.lockedCount() == 1);
}

TEST_CASE("CutsceneEditorV1 countByType", "[Editor][S192]") {
    CutsceneEditorV1 cce;
    Ccev1CutsceneTrack t1; t1.id = 1; t1.name = "A"; t1.trackType = Ccev1TrackType::Camera;
    Ccev1CutsceneTrack t2; t2.id = 2; t2.name = "B"; t2.trackType = Ccev1TrackType::Audio;
    Ccev1CutsceneTrack t3; t3.id = 3; t3.name = "C"; t3.trackType = Ccev1TrackType::Camera;
    cce.addTrack(t1); cce.addTrack(t2); cce.addTrack(t3);
    REQUIRE(cce.countByType(Ccev1TrackType::Camera) == 2);
    REQUIRE(cce.countByType(Ccev1TrackType::Audio) == 1);
}

TEST_CASE("CutsceneEditorV1 keyframesForTrack", "[Editor][S192]") {
    CutsceneEditorV1 cce;
    Ccev1CutsceneKeyframe k1; k1.id = 1; k1.trackId = 10; k1.timeMs = 0.0f;
    Ccev1CutsceneKeyframe k2; k2.id = 2; k2.trackId = 10; k2.timeMs = 500.0f;
    Ccev1CutsceneKeyframe k3; k3.id = 3; k3.trackId = 20; k3.timeMs = 250.0f;
    cce.addKeyframe(k1); cce.addKeyframe(k2); cce.addKeyframe(k3);
    REQUIRE(cce.keyframesForTrack(10) == 2);
    REQUIRE(cce.keyframesForTrack(20) == 1);
}

TEST_CASE("CutsceneEditorV1 onChange callback", "[Editor][S192]") {
    CutsceneEditorV1 cce;
    uint64_t notified = 0;
    cce.setOnChange([&](uint64_t id) { notified = id; });
    Ccev1CutsceneTrack t; t.id = 4; t.name = "D";
    cce.addTrack(t);
    REQUIRE(notified == 4);
}

TEST_CASE("Ccev1CutsceneTrack state helpers", "[Editor][S192]") {
    Ccev1CutsceneTrack t; t.id = 1; t.name = "X";
    t.state = Ccev1TrackState::Visible; REQUIRE(t.isVisible());
    t.state = Ccev1TrackState::Locked;  REQUIRE(t.isLocked());
    t.state = Ccev1TrackState::Solo;    REQUIRE(t.isSolo());
}

TEST_CASE("ccev1TrackTypeName all values", "[Editor][S192]") {
    REQUIRE(std::string(ccev1TrackTypeName(Ccev1TrackType::Camera))   == "Camera");
    REQUIRE(std::string(ccev1TrackTypeName(Ccev1TrackType::Actor))    == "Actor");
    REQUIRE(std::string(ccev1TrackTypeName(Ccev1TrackType::Audio))    == "Audio");
    REQUIRE(std::string(ccev1TrackTypeName(Ccev1TrackType::Event))    == "Event");
    REQUIRE(std::string(ccev1TrackTypeName(Ccev1TrackType::Subtitle)) == "Subtitle");
    REQUIRE(std::string(ccev1TrackTypeName(Ccev1TrackType::VFX))      == "VFX");
}

// ── LocalizationTableEditorV1 ─────────────────────────────────────────────────

TEST_CASE("Ltev1LocalizationEntry validity", "[Editor][S192]") {
    Ltev1LocalizationEntry e;
    REQUIRE(!e.isValid());
    e.id = 1; e.key = "ui.button.ok";
    REQUIRE(e.isValid());
}

TEST_CASE("LocalizationTableEditorV1 addEntry and entryCount", "[Editor][S192]") {
    LocalizationTableEditorV1 lte;
    REQUIRE(lte.entryCount() == 0);
    Ltev1LocalizationEntry e; e.id = 1; e.key = "k1";
    REQUIRE(lte.addEntry(e));
    REQUIRE(lte.entryCount() == 1);
}

TEST_CASE("LocalizationTableEditorV1 addEntry invalid fails", "[Editor][S192]") {
    LocalizationTableEditorV1 lte;
    REQUIRE(!lte.addEntry(Ltev1LocalizationEntry{}));
}

TEST_CASE("LocalizationTableEditorV1 addEntry duplicate fails", "[Editor][S192]") {
    LocalizationTableEditorV1 lte;
    Ltev1LocalizationEntry e; e.id = 1; e.key = "k";
    lte.addEntry(e);
    REQUIRE(!lte.addEntry(e));
}

TEST_CASE("LocalizationTableEditorV1 removeEntry", "[Editor][S192]") {
    LocalizationTableEditorV1 lte;
    Ltev1LocalizationEntry e; e.id = 2; e.key = "k2";
    lte.addEntry(e);
    REQUIRE(lte.removeEntry(2));
    REQUIRE(lte.entryCount() == 0);
    REQUIRE(!lte.removeEntry(2));
}

TEST_CASE("LocalizationTableEditorV1 findEntry", "[Editor][S192]") {
    LocalizationTableEditorV1 lte;
    Ltev1LocalizationEntry e; e.id = 3; e.key = "k3";
    lte.addEntry(e);
    REQUIRE(lte.findEntry(3) != nullptr);
    REQUIRE(lte.findEntry(99) == nullptr);
}

TEST_CASE("LocalizationTableEditorV1 addTranslation and translationCount", "[Editor][S192]") {
    LocalizationTableEditorV1 lte;
    Ltev1Translation t; t.id = 1; t.entryId = 10; t.locale = "fr"; t.text = "Bonjour";
    REQUIRE(lte.addTranslation(t));
    REQUIRE(lte.translationCount() == 1);
}

TEST_CASE("LocalizationTableEditorV1 removeTranslation", "[Editor][S192]") {
    LocalizationTableEditorV1 lte;
    Ltev1Translation t; t.id = 1; t.entryId = 10; t.locale = "de"; t.text = "Hallo";
    lte.addTranslation(t);
    REQUIRE(lte.removeTranslation(1));
    REQUIRE(lte.translationCount() == 0);
}

TEST_CASE("LocalizationTableEditorV1 translatedCount", "[Editor][S192]") {
    LocalizationTableEditorV1 lte;
    Ltev1LocalizationEntry e1; e1.id = 1; e1.key = "a"; e1.state = Ltev1EntryState::Translated;
    Ltev1LocalizationEntry e2; e2.id = 2; e2.key = "b";
    lte.addEntry(e1); lte.addEntry(e2);
    REQUIRE(lte.translatedCount() == 1);
}

TEST_CASE("LocalizationTableEditorV1 verifiedCount", "[Editor][S192]") {
    LocalizationTableEditorV1 lte;
    Ltev1LocalizationEntry e1; e1.id = 1; e1.key = "a"; e1.state = Ltev1EntryState::Verified;
    Ltev1LocalizationEntry e2; e2.id = 2; e2.key = "b"; e2.state = Ltev1EntryState::Translated;
    lte.addEntry(e1); lte.addEntry(e2);
    REQUIRE(lte.verifiedCount() == 1);
}

TEST_CASE("LocalizationTableEditorV1 countByContext", "[Editor][S192]") {
    LocalizationTableEditorV1 lte;
    Ltev1LocalizationEntry e1; e1.id = 1; e1.key = "a"; e1.context = Ltev1TextContext::UI;
    Ltev1LocalizationEntry e2; e2.id = 2; e2.key = "b"; e2.context = Ltev1TextContext::Dialogue;
    Ltev1LocalizationEntry e3; e3.id = 3; e3.key = "c"; e3.context = Ltev1TextContext::UI;
    lte.addEntry(e1); lte.addEntry(e2); lte.addEntry(e3);
    REQUIRE(lte.countByContext(Ltev1TextContext::UI) == 2);
    REQUIRE(lte.countByContext(Ltev1TextContext::Dialogue) == 1);
}

TEST_CASE("LocalizationTableEditorV1 translationsForEntry", "[Editor][S192]") {
    LocalizationTableEditorV1 lte;
    Ltev1Translation t1; t1.id = 1; t1.entryId = 10; t1.locale = "fr"; t1.text = "A";
    Ltev1Translation t2; t2.id = 2; t2.entryId = 10; t2.locale = "de"; t2.text = "B";
    Ltev1Translation t3; t3.id = 3; t3.entryId = 20; t3.locale = "es"; t3.text = "C";
    lte.addTranslation(t1); lte.addTranslation(t2); lte.addTranslation(t3);
    REQUIRE(lte.translationsForEntry(10) == 2);
    REQUIRE(lte.translationsForEntry(20) == 1);
}

TEST_CASE("LocalizationTableEditorV1 onChange callback", "[Editor][S192]") {
    LocalizationTableEditorV1 lte;
    uint64_t notified = 0;
    lte.setOnChange([&](uint64_t id) { notified = id; });
    Ltev1LocalizationEntry e; e.id = 6; e.key = "k6";
    lte.addEntry(e);
    REQUIRE(notified == 6);
}

TEST_CASE("Ltev1LocalizationEntry state helpers", "[Editor][S192]") {
    Ltev1LocalizationEntry e; e.id = 1; e.key = "x";
    e.state = Ltev1EntryState::Translated; REQUIRE(e.isTranslated());
    e.state = Ltev1EntryState::Verified;   REQUIRE(e.isVerified());
    e.state = Ltev1EntryState::Deprecated; REQUIRE(e.isDeprecated());
}

TEST_CASE("ltev1TextContextName all values", "[Editor][S192]") {
    REQUIRE(std::string(ltev1TextContextName(Ltev1TextContext::UI))       == "UI");
    REQUIRE(std::string(ltev1TextContextName(Ltev1TextContext::Dialogue)) == "Dialogue");
    REQUIRE(std::string(ltev1TextContextName(Ltev1TextContext::Tutorial)) == "Tutorial");
    REQUIRE(std::string(ltev1TextContextName(Ltev1TextContext::Item))     == "Item");
    REQUIRE(std::string(ltev1TextContextName(Ltev1TextContext::Quest))    == "Quest");
    REQUIRE(std::string(ltev1TextContextName(Ltev1TextContext::System))   == "System");
}
