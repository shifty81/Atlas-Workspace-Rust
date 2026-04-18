#include "../engine/ui/UISceneGraph.h"
#include <iostream>
#include <cassert>

using namespace atlas::ui;

void test_scene_graph_init() {
    UISceneGraph graph;
    graph.Init("TestScene");
    assert(graph.GetName() == "TestScene");
    assert(graph.Root().ChildCount() == 0);
    std::cout << "[PASS] test_scene_graph_init" << std::endl;
}

void test_scene_graph_add_child() {
    UISceneGraph graph;
    graph.Init("TestScene");

    auto child = std::make_unique<UISceneNode>();
    child->name = "Panel1";
    graph.Root().AddChild(std::move(child));

    assert(graph.Root().ChildCount() == 1);
    std::cout << "[PASS] test_scene_graph_add_child" << std::endl;
}

void test_scene_graph_remove_child() {
    UISceneGraph graph;
    graph.Init("TestScene");

    auto child = std::make_unique<UISceneNode>();
    child->name = "Panel1";
    graph.Root().AddChild(std::move(child));
    assert(graph.Root().ChildCount() == 1);

    uint32_t id = graph.Root().Children()[0]->id;
    graph.Root().RemoveChild(id);
    assert(graph.Root().ChildCount() == 0);
    std::cout << "[PASS] test_scene_graph_remove_child" << std::endl;
}

void test_scene_graph_find_child() {
    UISceneGraph graph;
    graph.Init("TestScene");

    auto child = std::make_unique<UISceneNode>();
    child->name = "FindMe";
    graph.Root().AddChild(std::move(child));

    uint32_t id = graph.Root().Children()[0]->id;
    auto* found = graph.Root().FindChild(id);
    assert(found != nullptr);
    assert(found->name == "FindMe");
    std::cout << "[PASS] test_scene_graph_find_child" << std::endl;
}

void test_scene_graph_find_nested_child() {
    UISceneGraph graph;
    graph.Init("TestScene");

    auto parent = std::make_unique<UISceneNode>();
    parent->name = "Parent";
    auto nested = std::make_unique<UISceneNode>();
    nested->name = "Nested";
    parent->AddChild(std::move(nested));
    graph.Root().AddChild(std::move(parent));

    auto& parentRef = *graph.Root().Children()[0];
    uint32_t nestedId = parentRef.Children()[0]->id;
    auto* found = graph.Root().FindChild(nestedId);
    assert(found != nullptr);
    assert(found->name == "Nested");
    std::cout << "[PASS] test_scene_graph_find_nested_child" << std::endl;
}

void test_scene_graph_layout_vertical() {
    UISceneGraph graph;
    graph.Init("TestScene");
    graph.Root().layoutDir = SceneLayoutDir::Vertical;

    auto c1 = std::make_unique<UISceneNode>();
    c1->name = "Top";
    c1->constraint.weight = 1.0f;
    auto c2 = std::make_unique<UISceneNode>();
    c2->name = "Bottom";
    c2->constraint.weight = 1.0f;

    graph.Root().AddChild(std::move(c1));
    graph.Root().AddChild(std::move(c2));

    graph.Layout({0, 0, 800, 600});

    auto& top = *graph.Root().Children()[0];
    auto& bot = *graph.Root().Children()[1];
    assert(top.bounds.h == 300);
    assert(bot.bounds.h == 300);
    assert(top.bounds.y == 0);
    assert(bot.bounds.y == 300);
    std::cout << "[PASS] test_scene_graph_layout_vertical" << std::endl;
}

void test_scene_graph_layout_horizontal() {
    UISceneGraph graph;
    graph.Init("TestScene");
    graph.Root().layoutDir = SceneLayoutDir::Horizontal;

    auto c1 = std::make_unique<UISceneNode>();
    c1->constraint.weight = 1.0f;
    auto c2 = std::make_unique<UISceneNode>();
    c2->constraint.weight = 3.0f;

    graph.Root().AddChild(std::move(c1));
    graph.Root().AddChild(std::move(c2));

    graph.Layout({0, 0, 800, 600});

    auto& left = *graph.Root().Children()[0];
    auto& right = *graph.Root().Children()[1];
    assert(left.bounds.w == 200);
    assert(right.bounds.w == 600);
    std::cout << "[PASS] test_scene_graph_layout_horizontal" << std::endl;
}

void test_scene_graph_hit_test() {
    UISceneGraph graph;
    graph.Init("TestScene");
    graph.Root().Layout({0, 0, 800, 600});

    assert(graph.Root().HitTest(100, 100));
    assert(!graph.Root().HitTest(900, 100));
    assert(!graph.Root().HitTest(-1, -1));
    std::cout << "[PASS] test_scene_graph_hit_test" << std::endl;
}

void test_scene_graph_draw_tree() {
    UISceneGraph graph;
    graph.Init("TestScene");

    auto child = std::make_unique<UISceneNode>();
    child->name = "Child";
    graph.Root().AddChild(std::move(child));

    UIDrawList drawList;
    graph.DrawAll(drawList);
    // Base nodes draw nothing, but the call should not crash
    assert(drawList.CommandCount() == 0);
    std::cout << "[PASS] test_scene_graph_draw_tree" << std::endl;
}

void test_scene_graph_invisible_child_skipped() {
    UISceneGraph graph;
    graph.Init("TestScene");
    graph.Root().layoutDir = SceneLayoutDir::Vertical;

    auto c1 = std::make_unique<UISceneNode>();
    c1->constraint.weight = 1.0f;
    c1->visible = false;
    auto c2 = std::make_unique<UISceneNode>();
    c2->constraint.weight = 1.0f;

    graph.Root().AddChild(std::move(c1));
    graph.Root().AddChild(std::move(c2));

    graph.Layout({0, 0, 800, 600});

    auto& visChild = *graph.Root().Children()[1];
    // Only visible child gets the full height
    assert(visChild.bounds.h == 600);
    std::cout << "[PASS] test_scene_graph_invisible_child_skipped" << std::endl;
}

void test_scene_graph_dispatch_event() {
    UISceneGraph graph;
    graph.Init("TestScene");
    graph.Layout({0, 0, 800, 600});

    UIEvent e;
    e.type = UIEvent::Type::MouseDown;
    e.x = 100;
    e.y = 100;
    // No children to consume, should return false
    bool consumed = graph.DispatchEvent(e);
    assert(!consumed);
    std::cout << "[PASS] test_scene_graph_dispatch_event" << std::endl;
}
