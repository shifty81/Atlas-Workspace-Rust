#include "../engine/flow/GameFlowGraph.h"
#include "../engine/flow/GameFlowNodes.h"
#include <iostream>
#include <cassert>

void test_gameflowgraph_add_nodes() {
    atlas::flow::GameFlowGraph graph;
    auto node = std::make_unique<atlas::flow::StateNode>();
    auto id = graph.AddNode(std::move(node));
    assert(id == 1);
    assert(graph.NodeCount() == 1);
    std::cout << "[PASS] test_gameflowgraph_add_nodes" << std::endl;
}

void test_gameflowgraph_remove_node() {
    atlas::flow::GameFlowGraph graph;
    auto node = std::make_unique<atlas::flow::StateNode>();
    auto id = graph.AddNode(std::move(node));
    graph.RemoveNode(id);
    assert(graph.NodeCount() == 0);
    std::cout << "[PASS] test_gameflowgraph_remove_node" << std::endl;
}

void test_gameflowgraph_compile_empty() {
    atlas::flow::GameFlowGraph graph;
    assert(graph.Compile());
    assert(graph.IsCompiled());
    std::cout << "[PASS] test_gameflowgraph_compile_empty" << std::endl;
}

void test_gameflowgraph_compile_single_node() {
    atlas::flow::GameFlowGraph graph;
    auto state = std::make_unique<atlas::flow::StateNode>();
    state->stateName = "Gameplay";
    auto id = graph.AddNode(std::move(state));
    assert(graph.Compile());

    atlas::flow::FlowContext ctx{0.0f, false, 1};
    assert(graph.Execute(ctx));

    auto* output = graph.GetOutput(id, 0);
    assert(output != nullptr);
    assert(output->type == atlas::flow::FlowPinType::State);
    assert(!output->data.empty());
    assert(output->data[0] == 1.0f);
    assert(output->text == "Gameplay");
    std::cout << "[PASS] test_gameflowgraph_compile_single_node" << std::endl;
}

void test_gameflowgraph_compile_chain() {
    atlas::flow::GameFlowGraph graph;

    auto stateId = graph.AddNode(std::make_unique<atlas::flow::StateNode>());
    auto condId = graph.AddNode(std::make_unique<atlas::flow::ConditionNode>());
    auto transId = graph.AddNode(std::make_unique<atlas::flow::TransitionNode>());

    // State -> Transition (State input)
    graph.AddEdge({stateId, 0, transId, 0});
    // Condition -> Transition (Bool input)
    graph.AddEdge({condId, 0, transId, 1});

    assert(graph.Compile());

    atlas::flow::FlowContext ctx{1.0f, true, 1};
    assert(graph.Execute(ctx));

    auto* output = graph.GetOutput(transId, 0);
    assert(output != nullptr);
    assert(output->type == atlas::flow::FlowPinType::Trigger);
    std::cout << "[PASS] test_gameflowgraph_compile_chain" << std::endl;
}

void test_gameflowgraph_execute() {
    atlas::flow::GameFlowGraph graph;

    auto condId = graph.AddNode(std::make_unique<atlas::flow::ConditionNode>());
    assert(graph.Compile());

    atlas::flow::FlowContext ctx{0.0f, false, 1};
    assert(graph.Execute(ctx));

    auto* output = graph.GetOutput(condId, 0);
    assert(output != nullptr);
    assert(output->type == atlas::flow::FlowPinType::Bool);
    assert(!output->data.empty());
    // Value 0.0 < threshold 0.5 → result should be 0
    assert(output->data[0] == 0.0f);
    std::cout << "[PASS] test_gameflowgraph_execute" << std::endl;
}

void test_gameflowgraph_deterministic() {
    auto buildAndRun = [](float elapsed) -> std::vector<float> {
        atlas::flow::GameFlowGraph graph;
        auto stateId = graph.AddNode(std::make_unique<atlas::flow::StateNode>());
        auto condId = graph.AddNode(std::make_unique<atlas::flow::ConditionNode>());
        auto transId = graph.AddNode(std::make_unique<atlas::flow::TransitionNode>());
        graph.AddEdge({stateId, 0, transId, 0});
        graph.AddEdge({condId, 0, transId, 1});
        graph.Compile();
        atlas::flow::FlowContext ctx{elapsed, true, 1};
        graph.Execute(ctx);
        return graph.GetOutput(transId, 0)->data;
    };

    auto a = buildAndRun(1.0f);
    auto b = buildAndRun(1.0f);
    assert(a == b);

    // Different elapsed time doesn't affect this chain (no timer node), so same result
    auto c = buildAndRun(2.0f);
    assert(a == c);
    std::cout << "[PASS] test_gameflowgraph_deterministic" << std::endl;
}
