#include "../engine/flow/FlowGraphDebugger.h"
#include "../engine/flow/GameFlowNodes.h"
#include <iostream>
#include <cassert>

void test_debugger_initial_state() {
    atlas::flow::FlowGraphDebugger dbg;
    assert(dbg.Breakpoints().empty());
    assert(!dbg.IsPaused());
    assert(dbg.ExecutionStep() == 0);
    std::cout << "[PASS] test_debugger_initial_state" << std::endl;
}

void test_debugger_add_breakpoint() {
    atlas::flow::FlowGraphDebugger dbg;
    dbg.AddBreakpoint(42);
    assert(dbg.HasBreakpoint(42));
    assert(dbg.Breakpoints().size() == 1);
    std::cout << "[PASS] test_debugger_add_breakpoint" << std::endl;
}

void test_debugger_remove_breakpoint() {
    atlas::flow::FlowGraphDebugger dbg;
    dbg.AddBreakpoint(10);
    dbg.RemoveBreakpoint(10);
    assert(!dbg.HasBreakpoint(10));
    assert(dbg.Breakpoints().empty());
    std::cout << "[PASS] test_debugger_remove_breakpoint" << std::endl;
}

void test_debugger_clear_breakpoints() {
    atlas::flow::FlowGraphDebugger dbg;
    dbg.AddBreakpoint(1);
    dbg.AddBreakpoint(2);
    dbg.AddBreakpoint(3);
    dbg.ClearBreakpoints();
    assert(dbg.Breakpoints().empty());
    std::cout << "[PASS] test_debugger_clear_breakpoints" << std::endl;
}

void test_debugger_execute_no_breakpoints() {
    atlas::flow::GameFlowGraph graph;
    auto stateId = graph.AddNode(std::make_unique<atlas::flow::StateNode>());
    graph.Compile();

    atlas::flow::FlowGraphDebugger dbg;
    dbg.SetAction(atlas::flow::DebugAction::Continue);
    atlas::flow::FlowContext ctx{0.0f, false, 1};
    dbg.ExecuteDebug(graph, ctx);

    assert(!dbg.NodeStates().empty());
    std::cout << "[PASS] test_debugger_execute_no_breakpoints" << std::endl;
}

void test_debugger_breakpoint_pause() {
    atlas::flow::GameFlowGraph graph;
    auto stateId = graph.AddNode(std::make_unique<atlas::flow::StateNode>());
    graph.Compile();

    atlas::flow::FlowGraphDebugger dbg;
    dbg.AddBreakpoint(stateId);
    dbg.SetAction(atlas::flow::DebugAction::Continue);
    atlas::flow::FlowContext ctx{0.0f, false, 1};
    dbg.ExecuteDebug(graph, ctx);

    assert(dbg.IsPaused());
    std::cout << "[PASS] test_debugger_breakpoint_pause" << std::endl;
}

void test_debugger_step_node() {
    atlas::flow::GameFlowGraph graph;
    auto id1 = graph.AddNode(std::make_unique<atlas::flow::StateNode>());
    auto id2 = graph.AddNode(std::make_unique<atlas::flow::ConditionNode>());
    graph.Compile();

    atlas::flow::FlowGraphDebugger dbg;
    dbg.SetAction(atlas::flow::DebugAction::StepNode);
    atlas::flow::FlowContext ctx{0.0f, false, 1};
    dbg.ExecuteDebug(graph, ctx);

    assert(dbg.IsPaused());
    assert(dbg.ExecutionStep() == 1);
    std::cout << "[PASS] test_debugger_step_node" << std::endl;
}
