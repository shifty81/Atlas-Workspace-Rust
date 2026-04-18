#include "../engine/ui/UILogicGraph.h"
#include "../engine/ui/UILogicNodes.h"
#include <iostream>
#include <cassert>
#include <cmath>

void test_ui_logic_add_nodes() {
    atlas::ui::UILogicGraph graph;
    auto id1 = graph.AddNode(std::make_unique<atlas::ui::ConditionNode>());
    auto id2 = graph.AddNode(std::make_unique<atlas::ui::ActionNode>());
    assert(id1 == 1);
    assert(id2 == 2);
    assert(graph.NodeCount() == 2);
    std::cout << "[PASS] test_ui_logic_add_nodes" << std::endl;
}

void test_ui_logic_remove_node() {
    atlas::ui::UILogicGraph graph;
    auto id = graph.AddNode(std::make_unique<atlas::ui::ConditionNode>());
    assert(graph.NodeCount() == 1);
    graph.RemoveNode(id);
    assert(graph.NodeCount() == 0);
    std::cout << "[PASS] test_ui_logic_remove_node" << std::endl;
}

void test_ui_logic_compile_empty() {
    atlas::ui::UILogicGraph graph;
    assert(graph.Compile());
    assert(graph.IsCompiled());
    std::cout << "[PASS] test_ui_logic_compile_empty" << std::endl;
}

void test_ui_logic_compile_cycle_detection() {
    atlas::ui::UILogicGraph graph;
    auto id1 = graph.AddNode(std::make_unique<atlas::ui::ConditionNode>());
    auto id2 = graph.AddNode(std::make_unique<atlas::ui::ConditionNode>());

    // Create a cycle: id1 -> id2 -> id1
    graph.AddEdge({id1, 0, id2, 0});
    graph.AddEdge({id2, 0, id1, 0});

    assert(!graph.Compile());
    assert(!graph.IsCompiled());
    std::cout << "[PASS] test_ui_logic_compile_cycle_detection" << std::endl;
}

void test_ui_logic_condition_node() {
    atlas::ui::UILogicGraph graph;
    auto condId = graph.AddNode(std::make_unique<atlas::ui::ConditionNode>());

    assert(graph.Compile());

    atlas::ui::UILogicContext ctx{0, 0.016f};
    assert(graph.Execute(ctx));

    auto* out = graph.GetOutput(condId, 0);
    assert(out != nullptr);
    assert(out->type == atlas::ui::UILogicPinType::Bool);
    // No input connected, defaults to false
    assert(!out->data.empty());
    assert(out->data[0] == 0.0f);
    std::cout << "[PASS] test_ui_logic_condition_node" << std::endl;
}

void test_ui_logic_action_node() {
    atlas::ui::UILogicGraph graph;

    auto actionNode = std::make_unique<atlas::ui::ActionNode>();
    actionNode->actionName = "show_panel";
    auto actionId = graph.AddNode(std::move(actionNode));

    assert(graph.Compile());

    atlas::ui::UILogicContext ctx{0, 0.016f};
    assert(graph.Execute(ctx));

    auto* out = graph.GetOutput(actionId, 0);
    assert(out != nullptr);
    assert(out->type == atlas::ui::UILogicPinType::Signal);
    // No trigger signal, output should be inactive
    assert(!out->data.empty());
    assert(out->data[0] == 0.0f);
    assert(out->text == "show_panel");
    std::cout << "[PASS] test_ui_logic_action_node" << std::endl;
}

void test_ui_logic_databind_node() {
    atlas::ui::UILogicGraph graph;

    auto bindNode = std::make_unique<atlas::ui::DataBindNode>();
    bindNode->boundKey = "health";
    bindNode->boundFloat = 75.5f;
    bindNode->boundInt = 42;
    bindNode->boundString = "player1";
    bindNode->boundBool = true;
    auto bindId = graph.AddNode(std::move(bindNode));

    assert(graph.Compile());

    atlas::ui::UILogicContext ctx{0, 0.016f};
    assert(graph.Execute(ctx));

    auto* floatOut = graph.GetOutput(bindId, 0);
    assert(floatOut != nullptr);
    assert(floatOut->type == atlas::ui::UILogicPinType::Float);
    assert(std::fabs(floatOut->data[0] - 75.5f) < 0.01f);

    auto* intOut = graph.GetOutput(bindId, 1);
    assert(intOut != nullptr);
    assert(intOut->type == atlas::ui::UILogicPinType::Int);
    assert(std::fabs(intOut->data[0] - 42.0f) < 0.01f);

    auto* strOut = graph.GetOutput(bindId, 2);
    assert(strOut != nullptr);
    assert(strOut->type == atlas::ui::UILogicPinType::String);
    assert(strOut->text == "player1");

    auto* boolOut = graph.GetOutput(bindId, 3);
    assert(boolOut != nullptr);
    assert(boolOut->type == atlas::ui::UILogicPinType::Bool);
    assert(boolOut->data[0] == 1.0f);

    std::cout << "[PASS] test_ui_logic_databind_node" << std::endl;
}

void test_ui_logic_transition_node() {
    atlas::ui::UILogicGraph graph;

    // Create a signal source (condition node that outputs Bool, then we need Signal)
    // Use a TimerNode as signal source that fires immediately at tick 0
    auto timerNode = std::make_unique<atlas::ui::TimerNode>();
    timerNode->delayTicks = 0;
    auto timerId = graph.AddNode(std::move(timerNode));

    auto transNode = std::make_unique<atlas::ui::TransitionNode>();
    transNode->startValue = 0.0f;
    transNode->endValue = 100.0f;
    transNode->durationTicks = 10;
    auto transId = graph.AddNode(std::move(transNode));

    // Connect timer signal -> transition trigger
    graph.AddEdge({timerId, 0, transId, 0});

    assert(graph.Compile());

    // Execute at tick 5 — should be halfway
    atlas::ui::UILogicContext ctx{5, 0.016f};
    assert(graph.Execute(ctx));

    auto* out = graph.GetOutput(transId, 0);
    assert(out != nullptr);
    assert(out->type == atlas::ui::UILogicPinType::Float);
    assert(std::fabs(out->data[0] - 50.0f) < 0.01f);

    std::cout << "[PASS] test_ui_logic_transition_node" << std::endl;
}

void test_ui_logic_timer_node() {
    atlas::ui::UILogicGraph graph;

    auto timerNode = std::make_unique<atlas::ui::TimerNode>();
    timerNode->delayTicks = 5;
    auto timerId = graph.AddNode(std::move(timerNode));

    assert(graph.Compile());

    // Before delay: tick 3, timer self-starts but hasn't reached delay
    atlas::ui::UILogicContext ctx{3, 0.016f};
    assert(graph.Execute(ctx));
    auto* out = graph.GetOutput(timerId, 0);
    assert(out != nullptr);
    assert(out->data[0] == 0.0f);

    // At delay: tick 5, timer should fire
    ctx.tick = 5;
    assert(graph.Execute(ctx));
    out = graph.GetOutput(timerId, 0);
    assert(out != nullptr);
    assert(out->data[0] == 1.0f);

    std::cout << "[PASS] test_ui_logic_timer_node" << std::endl;
}

void test_ui_logic_gate_node_pass() {
    atlas::ui::UILogicGraph graph;

    // Timer that fires immediately as signal source
    auto timerNode = std::make_unique<atlas::ui::TimerNode>();
    timerNode->delayTicks = 0;
    auto timerId = graph.AddNode(std::move(timerNode));

    // Condition that outputs true
    auto condNode = std::make_unique<atlas::ui::ConditionNode>();
    auto condId = graph.AddNode(std::move(condNode));

    // We need a Bool source to feed the condition node
    auto bindNode = std::make_unique<atlas::ui::DataBindNode>();
    bindNode->boundBool = true;
    auto bindId = graph.AddNode(std::move(bindNode));

    auto gateId = graph.AddNode(std::make_unique<atlas::ui::GateNode>());

    // DataBind bool output (port 3) -> Condition input (port 0)
    graph.AddEdge({bindId, 3, condId, 0});
    // Timer signal (port 0) -> Gate signal input (port 0)
    graph.AddEdge({timerId, 0, gateId, 0});
    // Condition output (port 0) -> Gate condition input (port 1)
    graph.AddEdge({condId, 0, gateId, 1});

    assert(graph.Compile());

    atlas::ui::UILogicContext ctx{0, 0.016f};
    assert(graph.Execute(ctx));

    auto* out = graph.GetOutput(gateId, 0);
    assert(out != nullptr);
    assert(out->type == atlas::ui::UILogicPinType::Signal);
    // Signal should pass through: timer fires + condition is true
    assert(out->data[0] == 1.0f);

    std::cout << "[PASS] test_ui_logic_gate_node_pass" << std::endl;
}

void test_ui_logic_gate_node_block() {
    atlas::ui::UILogicGraph graph;

    // Timer that fires immediately as signal source
    auto timerNode = std::make_unique<atlas::ui::TimerNode>();
    timerNode->delayTicks = 0;
    auto timerId = graph.AddNode(std::move(timerNode));

    // Condition that outputs false
    auto condNode = std::make_unique<atlas::ui::ConditionNode>();
    auto condId = graph.AddNode(std::move(condNode));

    // DataBind with boundBool = false
    auto bindNode = std::make_unique<atlas::ui::DataBindNode>();
    bindNode->boundBool = false;
    auto bindId = graph.AddNode(std::move(bindNode));

    auto gateId = graph.AddNode(std::make_unique<atlas::ui::GateNode>());

    // DataBind bool output (port 3) -> Condition input (port 0)
    graph.AddEdge({bindId, 3, condId, 0});
    // Timer signal (port 0) -> Gate signal input (port 0)
    graph.AddEdge({timerId, 0, gateId, 0});
    // Condition output (port 0) -> Gate condition input (port 1)
    graph.AddEdge({condId, 0, gateId, 1});

    assert(graph.Compile());

    atlas::ui::UILogicContext ctx{0, 0.016f};
    assert(graph.Execute(ctx));

    auto* out = graph.GetOutput(gateId, 0);
    assert(out != nullptr);
    assert(out->type == atlas::ui::UILogicPinType::Signal);
    // Signal should be blocked: condition is false
    assert(out->data[0] == 0.0f);

    std::cout << "[PASS] test_ui_logic_gate_node_block" << std::endl;
}
