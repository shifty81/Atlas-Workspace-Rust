#include "../editor/panels/WorldGraphPanel.h"
#include "../engine/world/WorldNodes.h"
#include <iostream>
#include <cassert>

using namespace atlas::editor;
using namespace atlas::world;

void test_graph_panel_no_graph() {
    WorldGraphPanel panel;
    panel.Draw();

    assert(panel.NodeInfos().empty());
    assert(!panel.IsCompiled());
    assert(!panel.CompileGraph());

    std::cout << "[PASS] test_graph_panel_no_graph" << std::endl;
}

void test_graph_panel_with_nodes() {
    WorldGraph graph;

    auto n1 = std::make_unique<ConstantNode>();
    n1->value = 42.0f;
    graph.AddNode(std::move(n1));

    auto n2 = std::make_unique<ConstantNode>();
    n2->value = 10.0f;
    graph.AddNode(std::move(n2));

    WorldGraphPanel panel;
    panel.SetGraph(&graph);
    panel.Draw();

    assert(panel.NodeInfos().size() == 2);
    assert(panel.GetGraph() == &graph);

    std::cout << "[PASS] test_graph_panel_with_nodes" << std::endl;
}

void test_graph_panel_select_node() {
    WorldGraphPanel panel;

    assert(panel.SelectedNode() == 0);

    panel.SelectNode(5);
    assert(panel.SelectedNode() == 5);

    std::cout << "[PASS] test_graph_panel_select_node" << std::endl;
}

void test_graph_panel_compile_and_preview() {
    WorldGraph graph;

    auto n1 = std::make_unique<ConstantNode>();
    n1->value = 42.0f;
    graph.AddNode(std::move(n1));

    WorldGraphPanel panel;
    panel.SetGraph(&graph);

    assert(panel.CompileGraph());
    assert(panel.IsCompiled());

    WorldGenContext ctx{12345, 0, 0, 0, 0};
    assert(panel.ExecutePreview(ctx));

    std::cout << "[PASS] test_graph_panel_compile_and_preview" << std::endl;
}

void test_graph_panel_summary() {
    WorldGraph graph;

    auto n1 = std::make_unique<ConstantNode>();
    n1->value = 1.0f;
    graph.AddNode(std::move(n1));

    auto n2 = std::make_unique<ConstantNode>();
    n2->value = 2.0f;
    graph.AddNode(std::move(n2));

    WorldGraphPanel panel;
    panel.SetGraph(&graph);
    panel.Draw();

    auto summary = panel.Summary();
    assert(summary.find("Nodes: 2") != std::string::npos);
    assert(summary.find("Compiled: no") != std::string::npos);

    panel.CompileGraph();
    summary = panel.Summary();
    assert(summary.find("Compiled: yes") != std::string::npos);

    std::cout << "[PASS] test_graph_panel_summary" << std::endl;
}

void test_graph_panel_node_info() {
    WorldGraph graph;

    auto n1 = std::make_unique<ConstantNode>();
    n1->value = 42.0f;
    auto id = graph.AddNode(std::move(n1));

    WorldGraphPanel panel;
    panel.SetGraph(&graph);
    panel.Draw();

    assert(panel.NodeInfos().size() == 1);
    assert(panel.NodeInfos()[0].id == id);
    assert(!panel.NodeInfos()[0].name.empty());
    assert(!panel.NodeInfos()[0].category.empty());

    std::cout << "[PASS] test_graph_panel_node_info" << std::endl;
}
