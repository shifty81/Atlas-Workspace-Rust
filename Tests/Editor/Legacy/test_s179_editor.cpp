// S179 editor tests: AIBehaviorTreeEditorV1, AIBlackboardEditorV1, AIDecoratorEditorV1
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/AIBehaviorTreeEditorV1.h"
#include "NF/Editor/AIBlackboardEditorV1.h"
#include "NF/Editor/AIDecoratorEditorV1.h"

using namespace NF;
using Catch::Approx;

// ── AIBehaviorTreeEditorV1 ───────────────────────────────────────────────────

TEST_CASE("Abtv1TreeNode validity", "[Editor][S179]") {
    Abtv1TreeNode n;
    REQUIRE(!n.isValid());
    n.id = 1; n.name = "Root";
    REQUIRE(n.isValid());
}

TEST_CASE("AIBehaviorTreeEditorV1 addNode and nodeCount", "[Editor][S179]") {
    AIBehaviorTreeEditorV1 abt;
    REQUIRE(abt.nodeCount() == 0);
    Abtv1TreeNode n; n.id = 1; n.name = "Selector";
    REQUIRE(abt.addNode(n));
    REQUIRE(abt.nodeCount() == 1);
}

TEST_CASE("AIBehaviorTreeEditorV1 addNode invalid fails", "[Editor][S179]") {
    AIBehaviorTreeEditorV1 abt;
    REQUIRE(!abt.addNode(Abtv1TreeNode{}));
}

TEST_CASE("AIBehaviorTreeEditorV1 addNode duplicate fails", "[Editor][S179]") {
    AIBehaviorTreeEditorV1 abt;
    Abtv1TreeNode n; n.id = 1; n.name = "A";
    abt.addNode(n);
    REQUIRE(!abt.addNode(n));
}

TEST_CASE("AIBehaviorTreeEditorV1 removeNode", "[Editor][S179]") {
    AIBehaviorTreeEditorV1 abt;
    Abtv1TreeNode n; n.id = 2; n.name = "B";
    abt.addNode(n);
    REQUIRE(abt.removeNode(2));
    REQUIRE(abt.nodeCount() == 0);
    REQUIRE(!abt.removeNode(2));
}

TEST_CASE("AIBehaviorTreeEditorV1 addConnection and connectionCount", "[Editor][S179]") {
    AIBehaviorTreeEditorV1 abt;
    Abtv1Connection c; c.id = 1; c.parentId = 1; c.childId = 2;
    REQUIRE(abt.addConnection(c));
    REQUIRE(abt.connectionCount() == 1);
}

TEST_CASE("AIBehaviorTreeEditorV1 addConnection invalid fails", "[Editor][S179]") {
    AIBehaviorTreeEditorV1 abt;
    REQUIRE(!abt.addConnection(Abtv1Connection{}));
}

TEST_CASE("AIBehaviorTreeEditorV1 removeConnection", "[Editor][S179]") {
    AIBehaviorTreeEditorV1 abt;
    Abtv1Connection c; c.id = 1; c.parentId = 1; c.childId = 2;
    abt.addConnection(c);
    REQUIRE(abt.removeConnection(1));
    REQUIRE(abt.connectionCount() == 0);
    REQUIRE(!abt.removeConnection(1));
}

TEST_CASE("AIBehaviorTreeEditorV1 runningCount and successCount", "[Editor][S179]") {
    AIBehaviorTreeEditorV1 abt;
    Abtv1TreeNode n1; n1.id = 1; n1.name = "A"; n1.status = Abtv1NodeStatus::Running;
    Abtv1TreeNode n2; n2.id = 2; n2.name = "B"; n2.status = Abtv1NodeStatus::Success;
    abt.addNode(n1); abt.addNode(n2);
    REQUIRE(abt.runningCount() == 1);
    REQUIRE(abt.successCount() == 1);
}

TEST_CASE("AIBehaviorTreeEditorV1 countByNodeKind", "[Editor][S179]") {
    AIBehaviorTreeEditorV1 abt;
    Abtv1TreeNode n1; n1.id = 1; n1.name = "A"; n1.kind = Abtv1NodeKind::Selector;
    Abtv1TreeNode n2; n2.id = 2; n2.name = "B"; n2.kind = Abtv1NodeKind::Leaf;
    abt.addNode(n1); abt.addNode(n2);
    REQUIRE(abt.countByNodeKind(Abtv1NodeKind::Selector) == 1);
    REQUIRE(abt.countByNodeKind(Abtv1NodeKind::Leaf)     == 1);
}

TEST_CASE("AIBehaviorTreeEditorV1 findNode returns ptr", "[Editor][S179]") {
    AIBehaviorTreeEditorV1 abt;
    Abtv1TreeNode n; n.id = 5; n.name = "Patrol";
    abt.addNode(n);
    REQUIRE(abt.findNode(5) != nullptr);
    REQUIRE(abt.findNode(5)->name == "Patrol");
    REQUIRE(abt.findNode(99) == nullptr);
}

TEST_CASE("abtv1NodeKindName covers all values", "[Editor][S179]") {
    REQUIRE(std::string(abtv1NodeKindName(Abtv1NodeKind::Root))      == "Root");
    REQUIRE(std::string(abtv1NodeKindName(Abtv1NodeKind::Sequence))  == "Sequence");
    REQUIRE(std::string(abtv1NodeKindName(Abtv1NodeKind::Parallel))  == "Parallel");
    REQUIRE(std::string(abtv1NodeKindName(Abtv1NodeKind::Decorator)) == "Decorator");
}

TEST_CASE("abtv1NodeStatusName covers all values", "[Editor][S179]") {
    REQUIRE(std::string(abtv1NodeStatusName(Abtv1NodeStatus::Idle))    == "Idle");
    REQUIRE(std::string(abtv1NodeStatusName(Abtv1NodeStatus::Failure)) == "Failure");
    REQUIRE(std::string(abtv1NodeStatusName(Abtv1NodeStatus::Aborted)) == "Aborted");
}

TEST_CASE("Abtv1TreeNode state helpers", "[Editor][S179]") {
    Abtv1TreeNode n; n.id = 1; n.name = "X";
    n.status = Abtv1NodeStatus::Running;
    REQUIRE(n.isRunning());
    n.status = Abtv1NodeStatus::Success;
    REQUIRE(n.isSuccess());
}

TEST_CASE("AIBehaviorTreeEditorV1 onChange callback", "[Editor][S179]") {
    AIBehaviorTreeEditorV1 abt;
    uint64_t notified = 0;
    abt.setOnChange([&](uint64_t id) { notified = id; });
    Abtv1TreeNode n; n.id = 7; n.name = "Chase";
    abt.addNode(n);
    REQUIRE(notified == 7);
}

// ── AIBlackboardEditorV1 ─────────────────────────────────────────────────────

TEST_CASE("Abbv1Entry validity", "[Editor][S179]") {
    Abbv1Entry e;
    REQUIRE(!e.isValid());
    e.id = 1; e.key = "TargetPos";
    REQUIRE(e.isValid());
}

TEST_CASE("AIBlackboardEditorV1 addEntry and entryCount", "[Editor][S179]") {
    AIBlackboardEditorV1 abb;
    REQUIRE(abb.entryCount() == 0);
    Abbv1Entry e; e.id = 1; e.key = "Health";
    REQUIRE(abb.addEntry(e));
    REQUIRE(abb.entryCount() == 1);
}

TEST_CASE("AIBlackboardEditorV1 addEntry invalid fails", "[Editor][S179]") {
    AIBlackboardEditorV1 abb;
    REQUIRE(!abb.addEntry(Abbv1Entry{}));
}

TEST_CASE("AIBlackboardEditorV1 addEntry duplicate fails", "[Editor][S179]") {
    AIBlackboardEditorV1 abb;
    Abbv1Entry e; e.id = 1; e.key = "A";
    abb.addEntry(e);
    REQUIRE(!abb.addEntry(e));
}

TEST_CASE("AIBlackboardEditorV1 removeEntry", "[Editor][S179]") {
    AIBlackboardEditorV1 abb;
    Abbv1Entry e; e.id = 2; e.key = "B";
    abb.addEntry(e);
    REQUIRE(abb.removeEntry(2));
    REQUIRE(abb.entryCount() == 0);
    REQUIRE(!abb.removeEntry(2));
}

TEST_CASE("AIBlackboardEditorV1 sharedCount and globalCount", "[Editor][S179]") {
    AIBlackboardEditorV1 abb;
    Abbv1Entry e1; e1.id = 1; e1.key = "A"; e1.scope = Abbv1EntryScope::Shared;
    Abbv1Entry e2; e2.id = 2; e2.key = "B"; e2.scope = Abbv1EntryScope::Global;
    abb.addEntry(e1); abb.addEntry(e2);
    REQUIRE(abb.sharedCount() == 1);
    REQUIRE(abb.globalCount() == 1);
}

TEST_CASE("AIBlackboardEditorV1 countByEntryType", "[Editor][S179]") {
    AIBlackboardEditorV1 abb;
    Abbv1Entry e1; e1.id = 1; e1.key = "A"; e1.type = Abbv1EntryType::Float;
    Abbv1Entry e2; e2.id = 2; e2.key = "B"; e2.type = Abbv1EntryType::Vector;
    abb.addEntry(e1); abb.addEntry(e2);
    REQUIRE(abb.countByEntryType(Abbv1EntryType::Float)  == 1);
    REQUIRE(abb.countByEntryType(Abbv1EntryType::Vector) == 1);
}

TEST_CASE("AIBlackboardEditorV1 countByScope", "[Editor][S179]") {
    AIBlackboardEditorV1 abb;
    Abbv1Entry e1; e1.id = 1; e1.key = "A"; e1.scope = Abbv1EntryScope::Local;
    Abbv1Entry e2; e2.id = 2; e2.key = "B"; e2.scope = Abbv1EntryScope::Persistent;
    abb.addEntry(e1); abb.addEntry(e2);
    REQUIRE(abb.countByScope(Abbv1EntryScope::Local)      == 1);
    REQUIRE(abb.countByScope(Abbv1EntryScope::Persistent) == 1);
}

TEST_CASE("AIBlackboardEditorV1 findEntry returns ptr", "[Editor][S179]") {
    AIBlackboardEditorV1 abb;
    Abbv1Entry e; e.id = 6; e.key = "EnemyPos";
    abb.addEntry(e);
    REQUIRE(abb.findEntry(6) != nullptr);
    REQUIRE(abb.findEntry(6)->key == "EnemyPos");
    REQUIRE(abb.findEntry(99) == nullptr);
}

TEST_CASE("abbv1EntryTypeName covers all values", "[Editor][S179]") {
    REQUIRE(std::string(abbv1EntryTypeName(Abbv1EntryType::Bool))   == "Bool");
    REQUIRE(std::string(abbv1EntryTypeName(Abbv1EntryType::String)) == "String");
    REQUIRE(std::string(abbv1EntryTypeName(Abbv1EntryType::Object)) == "Object");
    REQUIRE(std::string(abbv1EntryTypeName(Abbv1EntryType::Enum))   == "Enum");
}

TEST_CASE("abbv1EntryScopeName covers all values", "[Editor][S179]") {
    REQUIRE(std::string(abbv1EntryScopeName(Abbv1EntryScope::Local))      == "Local");
    REQUIRE(std::string(abbv1EntryScopeName(Abbv1EntryScope::Persistent)) == "Persistent");
}

TEST_CASE("Abbv1Entry scope helpers", "[Editor][S179]") {
    Abbv1Entry e; e.id = 1; e.key = "X";
    e.scope = Abbv1EntryScope::Shared;
    REQUIRE(e.isShared());
    e.scope = Abbv1EntryScope::Global;
    REQUIRE(e.isGlobal());
}

TEST_CASE("AIBlackboardEditorV1 onChange callback", "[Editor][S179]") {
    AIBlackboardEditorV1 abb;
    uint64_t notified = 0;
    abb.setOnChange([&](uint64_t id) { notified = id; });
    Abbv1Entry e; e.id = 9; e.key = "Speed";
    abb.addEntry(e);
    REQUIRE(notified == 9);
}

// ── AIDecoratorEditorV1 ──────────────────────────────────────────────────────

TEST_CASE("Adcv1Decorator validity", "[Editor][S179]") {
    Adcv1Decorator d;
    REQUIRE(!d.isValid());
    d.id = 1; d.name = "Cooldown";
    REQUIRE(d.isValid());
}

TEST_CASE("AIDecoratorEditorV1 addDecorator and decoratorCount", "[Editor][S179]") {
    AIDecoratorEditorV1 adc;
    REQUIRE(adc.decoratorCount() == 0);
    Adcv1Decorator d; d.id = 1; d.name = "Loop";
    REQUIRE(adc.addDecorator(d));
    REQUIRE(adc.decoratorCount() == 1);
}

TEST_CASE("AIDecoratorEditorV1 addDecorator invalid fails", "[Editor][S179]") {
    AIDecoratorEditorV1 adc;
    REQUIRE(!adc.addDecorator(Adcv1Decorator{}));
}

TEST_CASE("AIDecoratorEditorV1 addDecorator duplicate fails", "[Editor][S179]") {
    AIDecoratorEditorV1 adc;
    Adcv1Decorator d; d.id = 1; d.name = "A";
    adc.addDecorator(d);
    REQUIRE(!adc.addDecorator(d));
}

TEST_CASE("AIDecoratorEditorV1 removeDecorator", "[Editor][S179]") {
    AIDecoratorEditorV1 adc;
    Adcv1Decorator d; d.id = 2; d.name = "B";
    adc.addDecorator(d);
    REQUIRE(adc.removeDecorator(2));
    REQUIRE(adc.decoratorCount() == 0);
    REQUIRE(!adc.removeDecorator(2));
}

TEST_CASE("AIDecoratorEditorV1 addCondition and conditionCount", "[Editor][S179]") {
    AIDecoratorEditorV1 adc;
    Adcv1Condition c; c.id = 1; c.decoratorId = 10; c.key = "health";
    REQUIRE(adc.addCondition(c));
    REQUIRE(adc.conditionCount() == 1);
}

TEST_CASE("AIDecoratorEditorV1 addCondition invalid fails", "[Editor][S179]") {
    AIDecoratorEditorV1 adc;
    REQUIRE(!adc.addCondition(Adcv1Condition{}));
}

TEST_CASE("AIDecoratorEditorV1 removeCondition", "[Editor][S179]") {
    AIDecoratorEditorV1 adc;
    Adcv1Condition c; c.id = 1; c.decoratorId = 5; c.key = "alive";
    adc.addCondition(c);
    REQUIRE(adc.removeCondition(1));
    REQUIRE(adc.conditionCount() == 0);
    REQUIRE(!adc.removeCondition(1));
}

TEST_CASE("AIDecoratorEditorV1 countByDecoratorType", "[Editor][S179]") {
    AIDecoratorEditorV1 adc;
    Adcv1Decorator d1; d1.id = 1; d1.name = "A"; d1.type = Adcv1DecoratorType::Loop;
    Adcv1Decorator d2; d2.id = 2; d2.name = "B"; d2.type = Adcv1DecoratorType::Retry;
    adc.addDecorator(d1); adc.addDecorator(d2);
    REQUIRE(adc.countByDecoratorType(Adcv1DecoratorType::Loop)  == 1);
    REQUIRE(adc.countByDecoratorType(Adcv1DecoratorType::Retry) == 1);
}

TEST_CASE("AIDecoratorEditorV1 countByConditionOp", "[Editor][S179]") {
    AIDecoratorEditorV1 adc;
    Adcv1Condition c1; c1.id = 1; c1.decoratorId = 1; c1.key = "x"; c1.op = Adcv1ConditionOp::Equal;
    Adcv1Condition c2; c2.id = 2; c2.decoratorId = 1; c2.key = "y"; c2.op = Adcv1ConditionOp::Greater;
    adc.addCondition(c1); adc.addCondition(c2);
    REQUIRE(adc.countByConditionOp(Adcv1ConditionOp::Equal)   == 1);
    REQUIRE(adc.countByConditionOp(Adcv1ConditionOp::Greater) == 1);
}

TEST_CASE("AIDecoratorEditorV1 findDecorator returns ptr", "[Editor][S179]") {
    AIDecoratorEditorV1 adc;
    Adcv1Decorator d; d.id = 4; d.name = "TimeLimit";
    adc.addDecorator(d);
    REQUIRE(adc.findDecorator(4) != nullptr);
    REQUIRE(adc.findDecorator(4)->name == "TimeLimit");
    REQUIRE(adc.findDecorator(99) == nullptr);
}

TEST_CASE("adcv1DecoratorTypeName covers all values", "[Editor][S179]") {
    REQUIRE(std::string(adcv1DecoratorTypeName(Adcv1DecoratorType::Cooldown))   == "Cooldown");
    REQUIRE(std::string(adcv1DecoratorTypeName(Adcv1DecoratorType::Inverter))   == "Inverter");
    REQUIRE(std::string(adcv1DecoratorTypeName(Adcv1DecoratorType::TimeLimit))  == "TimeLimit");
    REQUIRE(std::string(adcv1DecoratorTypeName(Adcv1DecoratorType::Blackboard)) == "Blackboard");
}

TEST_CASE("adcv1ConditionOpName covers all values", "[Editor][S179]") {
    REQUIRE(std::string(adcv1ConditionOpName(Adcv1ConditionOp::NotEqual))  == "NotEqual");
    REQUIRE(std::string(adcv1ConditionOpName(Adcv1ConditionOp::LessEq))    == "LessEq");
    REQUIRE(std::string(adcv1ConditionOpName(Adcv1ConditionOp::GreaterEq)) == "GreaterEq");
    REQUIRE(std::string(adcv1ConditionOpName(Adcv1ConditionOp::IsSet))     == "IsSet");
    REQUIRE(std::string(adcv1ConditionOpName(Adcv1ConditionOp::IsNotSet))  == "IsNotSet");
}

TEST_CASE("AIDecoratorEditorV1 onChange callback", "[Editor][S179]") {
    AIDecoratorEditorV1 adc;
    uint64_t notified = 0;
    adc.setOnChange([&](uint64_t id) { notified = id; });
    Adcv1Decorator d; d.id = 3; d.name = "C";
    adc.addDecorator(d);
    REQUIRE(notified == 3);
}
