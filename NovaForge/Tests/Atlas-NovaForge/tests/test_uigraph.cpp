#include "../engine/ui/UIGraph.h"
#include "../engine/ui/UINodes.h"
#include <iostream>
#include <cassert>

void test_uigraph_add_nodes() {
    atlas::ui::UIGraph graph;
    auto node = std::make_unique<atlas::ui::PanelNode>();
    auto id = graph.AddNode(std::move(node));
    assert(id == 1);
    assert(graph.NodeCount() == 1);
    std::cout << "[PASS] test_uigraph_add_nodes" << std::endl;
}

void test_uigraph_remove_node() {
    atlas::ui::UIGraph graph;
    auto node = std::make_unique<atlas::ui::PanelNode>();
    auto id = graph.AddNode(std::move(node));
    graph.RemoveNode(id);
    assert(graph.NodeCount() == 0);
    std::cout << "[PASS] test_uigraph_remove_node" << std::endl;
}

void test_uigraph_compile_empty() {
    atlas::ui::UIGraph graph;
    assert(graph.Compile());
    assert(graph.IsCompiled());
    std::cout << "[PASS] test_uigraph_compile_empty" << std::endl;
}

void test_uigraph_compile_single_node() {
    atlas::ui::UIGraph graph;
    auto panel = std::make_unique<atlas::ui::PanelNode>();
    panel->width = 200.0f;
    panel->height = 150.0f;
    auto id = graph.AddNode(std::move(panel));
    assert(graph.Compile());

    atlas::ui::UIContext ctx{1920.0f, 1080.0f, 0.016f, 1};
    assert(graph.Execute(ctx));

    auto* output = graph.GetOutput(id, 0);
    assert(output != nullptr);
    assert(output->type == atlas::ui::UIPinType::Layout);
    assert(output->data.size() == 4);
    assert(output->data[2] == 200.0f);
    assert(output->data[3] == 150.0f);
    std::cout << "[PASS] test_uigraph_compile_single_node" << std::endl;
}

void test_uigraph_compile_chain() {
    atlas::ui::UIGraph graph;

    auto panelId = graph.AddNode(std::make_unique<atlas::ui::PanelNode>());
    auto textId = graph.AddNode(std::make_unique<atlas::ui::TextNode>());

    // Panel Layout -> Text Layout input
    graph.AddEdge({panelId, 0, textId, 1});

    assert(graph.Compile());

    atlas::ui::UIContext ctx{1920.0f, 1080.0f, 0.016f, 1};
    assert(graph.Execute(ctx));

    auto* output = graph.GetOutput(textId, 0);
    assert(output != nullptr);
    assert(output->type == atlas::ui::UIPinType::Layout);
    assert(output->data.size() == 4);
    std::cout << "[PASS] test_uigraph_compile_chain" << std::endl;
}

void test_uigraph_execute() {
    atlas::ui::UIGraph graph;

    auto panelId = graph.AddNode(std::make_unique<atlas::ui::PanelNode>());
    auto buttonId = graph.AddNode(std::make_unique<atlas::ui::ButtonNode>());

    // Panel Layout -> Button Layout input
    graph.AddEdge({panelId, 0, buttonId, 0});

    assert(graph.Compile());

    atlas::ui::UIContext ctx{1920.0f, 1080.0f, 0.016f, 1};
    assert(graph.Execute(ctx));

    auto* output = graph.GetOutput(buttonId, 0);
    assert(output != nullptr);
    assert(output->type == atlas::ui::UIPinType::Action);
    assert(!output->data.empty());
    assert(output->data[0] == 1.0f);  // Active because layout was provided
    std::cout << "[PASS] test_uigraph_execute" << std::endl;
}

void test_uigraph_deterministic() {
    auto buildAndRun = [](float w, float h) -> std::vector<float> {
        atlas::ui::UIGraph graph;
        auto panel = std::make_unique<atlas::ui::PanelNode>();
        panel->width = w;
        panel->height = h;
        auto panelId = graph.AddNode(std::move(panel));
        auto listId = graph.AddNode(std::make_unique<atlas::ui::ListNode>());
        graph.AddEdge({panelId, 0, listId, 0});
        graph.Compile();
        atlas::ui::UIContext ctx{1920.0f, 1080.0f, 0.016f, 1};
        graph.Execute(ctx);
        return graph.GetOutput(listId, 0)->data;
    };

    auto a = buildAndRun(100.0f, 50.0f);
    auto b = buildAndRun(100.0f, 50.0f);
    assert(a == b);

    auto c = buildAndRun(200.0f, 75.0f);
    assert(a != c);
    std::cout << "[PASS] test_uigraph_deterministic" << std::endl;
}
