// S99 editor tests: QuestEditor, DialogueEditor, CutsceneScriptEditor
#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/CutsceneScriptEditor.h"
#include "NF/Editor/DialogueEditor.h"
#include "NF/Editor/QuestEditor.h"

using namespace NF;

// ── QuestEditor ──────────────────────────────────────────────────────────────

TEST_CASE("QuestStatus names", "[Editor][S99]") {
    REQUIRE(std::string(questStatusName(QuestStatus::Locked))    == "Locked");
    REQUIRE(std::string(questStatusName(QuestStatus::Available)) == "Available");
    REQUIRE(std::string(questStatusName(QuestStatus::Active))    == "Active");
    REQUIRE(std::string(questStatusName(QuestStatus::Completed)) == "Completed");
    REQUIRE(std::string(questStatusName(QuestStatus::Failed))    == "Failed");
    REQUIRE(std::string(questStatusName(QuestStatus::Abandoned)) == "Abandoned");
}

TEST_CASE("QuestCategory names", "[Editor][S99]") {
    REQUIRE(std::string(questCategoryName(QuestCategory::Main))        == "Main");
    REQUIRE(std::string(questCategoryName(QuestCategory::Side))        == "Side");
    REQUIRE(std::string(questCategoryName(QuestCategory::Daily))       == "Daily");
    REQUIRE(std::string(questCategoryName(QuestCategory::Hidden))      == "Hidden");
    REQUIRE(std::string(questCategoryName(QuestCategory::Tutorial))    == "Tutorial");
    REQUIRE(std::string(questCategoryName(QuestCategory::Achievement)) == "Achievement");
}

TEST_CASE("QuestObjectiveType names", "[Editor][S99]") {
    REQUIRE(std::string(questObjectiveTypeName(QuestObjectiveType::Collect))  == "Collect");
    REQUIRE(std::string(questObjectiveTypeName(QuestObjectiveType::Kill))     == "Kill");
    REQUIRE(std::string(questObjectiveTypeName(QuestObjectiveType::Reach))    == "Reach");
    REQUIRE(std::string(questObjectiveTypeName(QuestObjectiveType::Talk))     == "Talk");
    REQUIRE(std::string(questObjectiveTypeName(QuestObjectiveType::Craft))    == "Craft");
    REQUIRE(std::string(questObjectiveTypeName(QuestObjectiveType::Survive))  == "Survive");
    REQUIRE(std::string(questObjectiveTypeName(QuestObjectiveType::Explore))  == "Explore");
    REQUIRE(std::string(questObjectiveTypeName(QuestObjectiveType::Deliver))  == "Deliver");
    REQUIRE(std::string(questObjectiveTypeName(QuestObjectiveType::Protect))  == "Protect");
}

TEST_CASE("QuestObjective defaults", "[Editor][S99]") {
    QuestObjective obj("obj_collect_herbs", QuestObjectiveType::Collect);
    REQUIRE(obj.id()          == "obj_collect_herbs");
    REQUIRE(obj.type()        == QuestObjectiveType::Collect);
    REQUIRE(obj.targetCount() == 1u);
    REQUIRE(obj.isRequired());
}

TEST_CASE("QuestObjective mutation", "[Editor][S99]") {
    QuestObjective obj("obj_kill_wolves", QuestObjectiveType::Kill);
    obj.setDescription("Kill 5 wolves");
    obj.setTargetCount(5);
    obj.setRequired(false);
    REQUIRE(obj.description() == "Kill 5 wolves");
    REQUIRE(obj.targetCount() == 5u);
    REQUIRE(!obj.isRequired());
}

TEST_CASE("QuestAsset add objective", "[Editor][S99]") {
    QuestAsset q("q_forest", QuestCategory::Side);
    QuestObjective o1("o1", QuestObjectiveType::Collect);
    QuestObjective o2("o2", QuestObjectiveType::Reach); o2.setRequired(false);
    REQUIRE(q.addObjective(o1));
    REQUIRE(q.addObjective(o2));
    REQUIRE(!q.addObjective(o1));
    REQUIRE(q.objectiveCount()         == 2u);
    REQUIRE(q.requiredObjectiveCount() == 1u);
}

TEST_CASE("QuestAsset defaults", "[Editor][S99]") {
    QuestAsset q("q_main_1", QuestCategory::Main);
    REQUIRE(q.id()       == "q_main_1");
    REQUIRE(q.category() == QuestCategory::Main);
    REQUIRE(q.status()   == QuestStatus::Locked);
    REQUIRE(q.isEnabled());
}

TEST_CASE("QuestEditor add/remove", "[Editor][S99]") {
    QuestEditor ed;
    QuestAsset q("q1", QuestCategory::Tutorial);
    REQUIRE(ed.addQuest(q));
    REQUIRE(ed.questCount() == 1u);
    REQUIRE(!ed.addQuest(q));
    REQUIRE(ed.removeQuest("q1"));
    REQUIRE(ed.questCount() == 0u);
}

TEST_CASE("QuestEditor counts", "[Editor][S99]") {
    QuestEditor ed;
    QuestAsset q1("a", QuestCategory::Main); q1.setStatus(QuestStatus::Active);
    QuestAsset q2("b", QuestCategory::Side); q2.setStatus(QuestStatus::Completed);
    QuestAsset q3("c", QuestCategory::Main); q3.setStatus(QuestStatus::Active); q3.setEnabled(false);
    ed.addQuest(q1); ed.addQuest(q2); ed.addQuest(q3);
    REQUIRE(ed.questCount()                         == 3u);
    REQUIRE(ed.countByStatus(QuestStatus::Active)   == 2u);
    REQUIRE(ed.countByCategory(QuestCategory::Main) == 2u);
    REQUIRE(ed.enabledCount()                       == 2u);
}

// ── DialogueEditor ───────────────────────────────────────────────────────────

TEST_CASE("DialogueNodeType names", "[Editor][S99]") {
    REQUIRE(std::string(dialogueNodeTypeName(DialogueNodeType::Start))  == "Start");
    REQUIRE(std::string(dialogueNodeTypeName(DialogueNodeType::Speech)) == "Speech");
    REQUIRE(std::string(dialogueNodeTypeName(DialogueNodeType::Choice)) == "Choice");
    REQUIRE(std::string(dialogueNodeTypeName(DialogueNodeType::Branch)) == "Branch");
    REQUIRE(std::string(dialogueNodeTypeName(DialogueNodeType::Event))  == "Event");
    REQUIRE(std::string(dialogueNodeTypeName(DialogueNodeType::End))    == "End");
}

TEST_CASE("DialogueSpeakerRole names", "[Editor][S99]") {
    REQUIRE(std::string(dialogueSpeakerRoleName(DialogueSpeakerRole::Player))   == "Player");
    REQUIRE(std::string(dialogueSpeakerRoleName(DialogueSpeakerRole::NPC))      == "NPC");
    REQUIRE(std::string(dialogueSpeakerRoleName(DialogueSpeakerRole::Narrator)) == "Narrator");
    REQUIRE(std::string(dialogueSpeakerRoleName(DialogueSpeakerRole::System))   == "System");
}

TEST_CASE("DialogueConditionType names", "[Editor][S99]") {
    REQUIRE(std::string(dialogueConditionTypeName(DialogueConditionType::None))        == "None");
    REQUIRE(std::string(dialogueConditionTypeName(DialogueConditionType::HasItem))     == "HasItem");
    REQUIRE(std::string(dialogueConditionTypeName(DialogueConditionType::HasFlag))     == "HasFlag");
    REQUIRE(std::string(dialogueConditionTypeName(DialogueConditionType::QuestActive)) == "QuestActive");
    REQUIRE(std::string(dialogueConditionTypeName(DialogueConditionType::QuestDone))   == "QuestDone");
    REQUIRE(std::string(dialogueConditionTypeName(DialogueConditionType::StatCheck))   == "StatCheck");
    REQUIRE(std::string(dialogueConditionTypeName(DialogueConditionType::Custom))      == "Custom");
}

TEST_CASE("DialogueNode defaults", "[Editor][S99]") {
    DialogueNode n(1, DialogueNodeType::Speech);
    REQUIRE(n.id()          == 1u);
    REQUIRE(n.type()        == DialogueNodeType::Speech);
    REQUIRE(n.speakerRole() == DialogueSpeakerRole::NPC);
    REQUIRE(n.condition()   == DialogueConditionType::None);
    REQUIRE(n.isEnabled());
}

TEST_CASE("DialogueNode mutation", "[Editor][S99]") {
    DialogueNode n(2, DialogueNodeType::Choice);
    n.setSpeakerRole(DialogueSpeakerRole::Player);
    n.setText("I'll take the sword.");
    n.setCondition(DialogueConditionType::HasItem);
    n.setEnabled(false);
    REQUIRE(n.speakerRole() == DialogueSpeakerRole::Player);
    REQUIRE(n.text()        == "I'll take the sword.");
    REQUIRE(n.condition()   == DialogueConditionType::HasItem);
    REQUIRE(!n.isEnabled());
}

TEST_CASE("DialogueGraph add/remove node", "[Editor][S99]") {
    DialogueGraph g("intro_scene");
    DialogueNode n1(10, DialogueNodeType::Start);
    DialogueNode n2(11, DialogueNodeType::Speech);
    REQUIRE(g.addNode(n1));
    REQUIRE(g.addNode(n2));
    REQUIRE(!g.addNode(n1));
    REQUIRE(g.nodeCount() == 2u);
    REQUIRE(g.removeNode(10));
    REQUIRE(g.nodeCount() == 1u);
}

TEST_CASE("DialogueGraph counts", "[Editor][S99]") {
    DialogueGraph g("talk_smith");
    g.addNode(DialogueNode(1, DialogueNodeType::Start));
    DialogueNode s1(2, DialogueNodeType::Speech);
    DialogueNode s2(3, DialogueNodeType::Speech); s2.setSpeakerRole(DialogueSpeakerRole::Player);
    g.addNode(s1); g.addNode(s2);
    g.addNode(DialogueNode(4, DialogueNodeType::End));
    REQUIRE(g.nodeCount()                                 == 4u);
    REQUIRE(g.countByType(DialogueNodeType::Speech)       == 2u);
    REQUIRE(g.countBySpeaker(DialogueSpeakerRole::Player) == 1u);
}

TEST_CASE("DialogueEditor add/remove graph", "[Editor][S99]") {
    DialogueEditor ed;
    DialogueGraph g("g1");
    REQUIRE(ed.addGraph(g));
    REQUIRE(ed.graphCount() == 1u);
    REQUIRE(!ed.addGraph(g));
    REQUIRE(ed.removeGraph("g1"));
    REQUIRE(ed.graphCount() == 0u);
}

TEST_CASE("DialogueEditor total nodes", "[Editor][S99]") {
    DialogueEditor ed;
    DialogueGraph g1("intro"); g1.addNode(DialogueNode(1, DialogueNodeType::Start));
    DialogueGraph g2("outro"); g2.addNode(DialogueNode(2, DialogueNodeType::End));
    g2.addNode(DialogueNode(3, DialogueNodeType::Speech));
    ed.addGraph(g1); ed.addGraph(g2);
    REQUIRE(ed.totalNodeCount() == 3u);
}

// ── CutsceneScriptEditor ─────────────────────────────────────────────────────

TEST_CASE("CutsceneScriptLineType names", "[Editor][S99]") {
    REQUIRE(std::string(cutsceneScriptLineTypeName(CutsceneScriptLineType::Dialogue))    == "Dialogue");
    REQUIRE(std::string(cutsceneScriptLineTypeName(CutsceneScriptLineType::Subtitle))    == "Subtitle");
    REQUIRE(std::string(cutsceneScriptLineTypeName(CutsceneScriptLineType::Caption))     == "Caption");
    REQUIRE(std::string(cutsceneScriptLineTypeName(CutsceneScriptLineType::Narration))   == "Narration");
    REQUIRE(std::string(cutsceneScriptLineTypeName(CutsceneScriptLineType::Instruction)) == "Instruction");
    REQUIRE(std::string(cutsceneScriptLineTypeName(CutsceneScriptLineType::Comment))     == "Comment");
}

TEST_CASE("CutsceneScriptState names", "[Editor][S99]") {
    REQUIRE(std::string(cutsceneScriptStateName(CutsceneScriptState::Draft))    == "Draft");
    REQUIRE(std::string(cutsceneScriptStateName(CutsceneScriptState::Review))   == "Review");
    REQUIRE(std::string(cutsceneScriptStateName(CutsceneScriptState::Approved)) == "Approved");
    REQUIRE(std::string(cutsceneScriptStateName(CutsceneScriptState::Locked))   == "Locked");
    REQUIRE(std::string(cutsceneScriptStateName(CutsceneScriptState::Archived)) == "Archived");
}

TEST_CASE("CutsceneScriptLocaleStatus names", "[Editor][S99]") {
    REQUIRE(std::string(cutsceneScriptLocaleStatusName(CutsceneScriptLocaleStatus::Missing))    == "Missing");
    REQUIRE(std::string(cutsceneScriptLocaleStatusName(CutsceneScriptLocaleStatus::InProgress)) == "InProgress");
    REQUIRE(std::string(cutsceneScriptLocaleStatusName(CutsceneScriptLocaleStatus::Done))       == "Done");
    REQUIRE(std::string(cutsceneScriptLocaleStatusName(CutsceneScriptLocaleStatus::Verified))   == "Verified");
}

TEST_CASE("CutsceneScriptLine defaults", "[Editor][S99]") {
    CutsceneScriptLine l(1, CutsceneScriptLineType::Dialogue);
    REQUIRE(l.id()        == 1u);
    REQUIRE(l.type()      == CutsceneScriptLineType::Dialogue);
    REQUIRE(l.startTime() == 0.0f);
    REQUIRE(l.endTime()   == 0.0f);
    REQUIRE(l.isEnabled());
}

TEST_CASE("CutsceneScriptLine mutation", "[Editor][S99]") {
    CutsceneScriptLine l(2, CutsceneScriptLineType::Subtitle);
    l.setSpeaker("Hero");
    l.setText("I will save the world.");
    l.setStartTime(1.5f);
    l.setEndTime(4.0f);
    l.setEnabled(false);
    REQUIRE(l.speaker()    == "Hero");
    REQUIRE(l.text()       == "I will save the world.");
    REQUIRE(l.startTime()  == 1.5f);
    REQUIRE(l.endTime()    == 4.0f);
    REQUIRE(!l.isEnabled());
}

TEST_CASE("CutsceneScriptEditor add/remove line", "[Editor][S99]") {
    CutsceneScriptEditor ed;
    CutsceneScriptLine l(10, CutsceneScriptLineType::Caption);
    REQUIRE(ed.addLine(l));
    REQUIRE(ed.lineCount() == 1u);
    REQUIRE(!ed.addLine(l));
    REQUIRE(ed.removeLine(10));
    REQUIRE(ed.lineCount() == 0u);
}

TEST_CASE("CutsceneScriptEditor state and locale", "[Editor][S99]") {
    CutsceneScriptEditor ed;
    REQUIRE(ed.state()        == CutsceneScriptState::Draft);
    REQUIRE(ed.localeStatus() == CutsceneScriptLocaleStatus::Missing);
    REQUIRE(!ed.isApproved());
    ed.setState(CutsceneScriptState::Approved);
    REQUIRE(ed.isApproved());
    ed.setState(CutsceneScriptState::Locked);
    REQUIRE(ed.isApproved());
    ed.setLocaleStatus(CutsceneScriptLocaleStatus::Verified);
    REQUIRE(ed.localeStatus() == CutsceneScriptLocaleStatus::Verified);
}

TEST_CASE("CutsceneScriptEditor line counts", "[Editor][S99]") {
    CutsceneScriptEditor ed;
    CutsceneScriptLine l1(1, CutsceneScriptLineType::Dialogue);
    CutsceneScriptLine l2(2, CutsceneScriptLineType::Subtitle); l2.setEnabled(false);
    CutsceneScriptLine l3(3, CutsceneScriptLineType::Dialogue);
    ed.addLine(l1); ed.addLine(l2); ed.addLine(l3);
    REQUIRE(ed.lineCount()    == 3u);
    REQUIRE(ed.enabledCount() == 2u);
    REQUIRE(ed.countByLineType(CutsceneScriptLineType::Dialogue) == 2u);
    REQUIRE(ed.countByLineType(CutsceneScriptLineType::Subtitle) == 1u);
}
