#include "../engine/flow/FlowGraphRefactorer.h"
#include <iostream>
#include <cassert>

static std::vector<atlas::flow::FlowNodeInfo> MakeSampleGraph() {
    // Graph: 1 -> 2 -> 3 -> 4, plus dead node 5 and 6
    std::vector<atlas::flow::FlowNodeInfo> nodes;

    atlas::flow::FlowNodeInfo n1;
    n1.id = 1; n1.name = "Start"; n1.category = "control";
    n1.outputEdges = {2};
    nodes.push_back(n1);

    atlas::flow::FlowNodeInfo n2;
    n2.id = 2; n2.name = "Process"; n2.category = "logic";
    n2.inputEdges = {1}; n2.outputEdges = {3};
    nodes.push_back(n2);

    atlas::flow::FlowNodeInfo n3;
    n3.id = 3; n3.name = "Transform"; n3.category = "logic";
    n3.inputEdges = {2}; n3.outputEdges = {4};
    nodes.push_back(n3);

    atlas::flow::FlowNodeInfo n4;
    n4.id = 4; n4.name = "End"; n4.category = "control";
    n4.inputEdges = {3};
    nodes.push_back(n4);

    // Dead node: no outputs and not referenced
    atlas::flow::FlowNodeInfo n5;
    n5.id = 5; n5.name = "Orphan"; n5.category = "misc";
    nodes.push_back(n5);

    // Another dead node
    atlas::flow::FlowNodeInfo n6;
    n6.id = 6; n6.name = "Unused"; n6.category = "misc";
    nodes.push_back(n6);

    return nodes;
}

void test_flow_refactorer_load() {
    atlas::flow::FlowGraphRefactorer r;
    r.LoadGraph(MakeSampleGraph());
    assert(r.NodeCount() == 6);
    assert(r.GetNode(1) != nullptr);
    assert(r.GetNode(99) == nullptr);
    std::cout << "[PASS] test_flow_refactorer_load" << std::endl;
}

void test_flow_refactorer_find_dead_nodes() {
    atlas::flow::FlowGraphRefactorer r;
    r.LoadGraph(MakeSampleGraph());
    auto dead = r.FindDeadNodes();
    assert(dead.size() == 2);
    // Dead nodes should be 5 and 6
    assert(dead[0] == 5);
    assert(dead[1] == 6);
    std::cout << "[PASS] test_flow_refactorer_find_dead_nodes" << std::endl;
}

void test_flow_refactorer_find_chains() {
    atlas::flow::FlowGraphRefactorer r;
    r.LoadGraph(MakeSampleGraph());
    auto chains = r.FindLinearChains();
    assert(!chains.empty());
    // Should find chain: 1->2->3->4
    bool foundLong = false;
    for (const auto& c : chains) {
        if (c.size() >= 3) foundLong = true;
    }
    assert(foundLong);
    std::cout << "[PASS] test_flow_refactorer_find_chains" << std::endl;
}

void test_flow_refactorer_find_duplicates() {
    atlas::flow::FlowGraphRefactorer r;
    // Add nodes with duplicate name|category
    std::vector<atlas::flow::FlowNodeInfo> nodes;
    atlas::flow::FlowNodeInfo n1; n1.id = 1; n1.name = "Add"; n1.category = "math";
    atlas::flow::FlowNodeInfo n2; n2.id = 2; n2.name = "Add"; n2.category = "math";
    atlas::flow::FlowNodeInfo n3; n3.id = 3; n3.name = "Sub"; n3.category = "math";
    nodes.push_back(n1);
    nodes.push_back(n2);
    nodes.push_back(n3);
    r.LoadGraph(nodes);
    auto dups = r.FindDuplicatePatterns();
    assert(dups.size() == 2);
    std::cout << "[PASS] test_flow_refactorer_find_duplicates" << std::endl;
}

void test_flow_refactorer_analyze() {
    atlas::flow::FlowGraphRefactorer r;
    r.LoadGraph(MakeSampleGraph());
    auto suggestions = r.Analyze();
    assert(!suggestions.empty());
    std::cout << "[PASS] test_flow_refactorer_analyze" << std::endl;
}

void test_flow_refactorer_remove_dead() {
    atlas::flow::FlowGraphRefactorer r;
    r.LoadGraph(MakeSampleGraph());
    auto result = r.RemoveDeadNodes();
    assert(result.success);
    assert(result.nodesRemoved == 2);
    assert(r.NodeCount() == 4);
    assert(r.GetNode(5) == nullptr);
    assert(r.GetNode(6) == nullptr);
    std::cout << "[PASS] test_flow_refactorer_remove_dead" << std::endl;
}

void test_flow_refactorer_rename() {
    atlas::flow::FlowGraphRefactorer r;
    r.LoadGraph(MakeSampleGraph());
    auto result = r.RenameNode(1, "Begin");
    assert(result.success);
    assert(r.GetNode(1)->name == "Begin");

    auto fail = r.RenameNode(999, "X");
    assert(!fail.success);
    std::cout << "[PASS] test_flow_refactorer_rename" << std::endl;
}

void test_flow_refactorer_simplify_chain() {
    atlas::flow::FlowGraphRefactorer r;
    r.LoadGraph(MakeSampleGraph());
    auto result = r.SimplifyChain({2, 3});
    assert(result.success);
    assert(result.nodesRemoved == 1);
    assert(r.GetNode(3) == nullptr);
    assert(r.GetNode(2) != nullptr);
    std::cout << "[PASS] test_flow_refactorer_simplify_chain" << std::endl;
}

void test_flow_refactorer_history() {
    atlas::flow::FlowGraphRefactorer r;
    r.LoadGraph(MakeSampleGraph());
    r.RemoveDeadNodes();
    r.RenameNode(1, "X");
    assert(r.RefactorHistory().size() == 2);
    std::cout << "[PASS] test_flow_refactorer_history" << std::endl;
}

void test_flow_refactorer_clear() {
    atlas::flow::FlowGraphRefactorer r;
    r.LoadGraph(MakeSampleGraph());
    r.RemoveDeadNodes();
    r.Clear();
    assert(r.NodeCount() == 0);
    assert(r.RefactorHistory().empty());
    std::cout << "[PASS] test_flow_refactorer_clear" << std::endl;
}
