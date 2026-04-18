#include "../engine/animation/AnimationGraph.h"
#include "../engine/animation/AnimationNodes.h"
#include <iostream>
#include <cassert>

void test_animationgraph_add_nodes() {
    atlas::animation::AnimationGraph graph;
    auto node = std::make_unique<atlas::animation::ClipNode>();
    auto id = graph.AddNode(std::move(node));
    assert(id == 1);
    assert(graph.NodeCount() == 1);
    std::cout << "[PASS] test_animationgraph_add_nodes" << std::endl;
}

void test_animationgraph_remove_node() {
    atlas::animation::AnimationGraph graph;
    auto node = std::make_unique<atlas::animation::ClipNode>();
    auto id = graph.AddNode(std::move(node));
    graph.RemoveNode(id);
    assert(graph.NodeCount() == 0);
    std::cout << "[PASS] test_animationgraph_remove_node" << std::endl;
}

void test_animationgraph_compile_empty() {
    atlas::animation::AnimationGraph graph;
    assert(graph.Compile());
    assert(graph.IsCompiled());
    std::cout << "[PASS] test_animationgraph_compile_empty" << std::endl;
}

void test_animationgraph_compile_single_node() {
    atlas::animation::AnimationGraph graph;
    auto node = std::make_unique<atlas::animation::ClipNode>();
    node->clipLength = 2.0f;
    auto id = graph.AddNode(std::move(node));
    assert(graph.Compile());

    atlas::animation::AnimContext ctx{0.016f, 1.0f, 0.0f, 0.0f, 0};
    assert(graph.Execute(ctx));

    auto* output = graph.GetOutput(id, 0);
    assert(output != nullptr);
    assert(output->type == atlas::animation::AnimPinType::Pose);
    assert(!output->data.empty());
    std::cout << "[PASS] test_animationgraph_compile_single_node" << std::endl;
}

void test_animationgraph_compile_chain() {
    atlas::animation::AnimationGraph graph;

    // ClipA -> Blend port 0 (PoseA)
    auto clipAId = graph.AddNode(std::make_unique<atlas::animation::ClipNode>());

    // ClipB -> Blend port 1 (PoseB)
    auto clipBId = graph.AddNode(std::make_unique<atlas::animation::ClipNode>());

    // Blend
    auto blendId = graph.AddNode(std::make_unique<atlas::animation::BlendNode>());

    graph.AddEdge({clipAId, 0, blendId, 0});  // PoseA
    graph.AddEdge({clipBId, 0, blendId, 1});  // PoseB

    assert(graph.Compile());

    atlas::animation::AnimContext ctx{0.016f, 1.0f, 0.0f, 0.0f, 10};
    assert(graph.Execute(ctx));

    auto* output = graph.GetOutput(blendId, 0);
    assert(output != nullptr);
    assert(output->type == atlas::animation::AnimPinType::Pose);
    assert(output->data.size() == 8);

    // With default weight=0.5, result should be average of both clips
    auto* poseA = graph.GetOutput(clipAId, 0);
    auto* poseB = graph.GetOutput(clipBId, 0);
    assert(poseA != nullptr && poseB != nullptr);
    for (size_t i = 0; i < output->data.size(); ++i) {
        float expected = poseA->data[i] * 0.5f + poseB->data[i] * 0.5f;
        assert(std::abs(output->data[i] - expected) < 0.001f);
    }
    std::cout << "[PASS] test_animationgraph_compile_chain" << std::endl;
}

void test_animationgraph_execute() {
    atlas::animation::AnimationGraph graph;

    // StateMachine with idle state
    auto smId = graph.AddNode(std::make_unique<atlas::animation::StateMachineNode>());
    assert(graph.Compile());

    atlas::animation::AnimContext ctx{0.016f, 1.0f, 0.0f, 0.0f, 5};
    assert(graph.Execute(ctx));

    auto* output = graph.GetOutput(smId, 0);
    assert(output != nullptr);
    assert(output->type == atlas::animation::AnimPinType::Pose);
    assert(output->data.size() == 8);

    // Default trigger=0 means idle: all zeros
    for (float v : output->data) {
        assert(v == 0.0f);
    }
    std::cout << "[PASS] test_animationgraph_execute" << std::endl;
}

void test_animationgraph_modifier() {
    atlas::animation::AnimationGraph graph;

    // Clip -> Modifier
    auto clipId = graph.AddNode(std::make_unique<atlas::animation::ClipNode>());
    auto modNode = std::make_unique<atlas::animation::ModifierNode>();
    modNode->modifierType = atlas::animation::ModifierType::Damage;
    auto modId = graph.AddNode(std::move(modNode));

    graph.AddEdge({clipId, 0, modId, 0});  // Pose -> Pose input

    assert(graph.Compile());

    // Execute with damage
    atlas::animation::AnimContext ctx{0.016f, 1.0f, 0.0f, 0.5f, 0};
    assert(graph.Execute(ctx));

    auto* clipOut = graph.GetOutput(clipId, 0);
    auto* modOut = graph.GetOutput(modId, 0);
    assert(clipOut != nullptr && modOut != nullptr);
    assert(modOut->data.size() == clipOut->data.size());

    // Damage modifier adds ctx.damageLevel * intensity * 0.1 to each element
    for (size_t i = 0; i < modOut->data.size(); ++i) {
        float expected = clipOut->data[i] + 0.5f * 1.0f * 0.1f;
        assert(std::abs(modOut->data[i] - expected) < 0.001f);
    }
    std::cout << "[PASS] test_animationgraph_modifier" << std::endl;
}

void test_animationgraph_deterministic() {
    auto buildAndRun = [](uint32_t tick) -> std::vector<float> {
        atlas::animation::AnimationGraph graph;
        auto clipId = graph.AddNode(std::make_unique<atlas::animation::ClipNode>());
        auto blendId = graph.AddNode(std::make_unique<atlas::animation::BlendNode>());
        graph.AddEdge({clipId, 0, blendId, 0});
        graph.Compile();
        atlas::animation::AnimContext ctx{0.016f, 1.0f, 0.0f, 0.0f, tick};
        graph.Execute(ctx);
        return graph.GetOutput(blendId, 0)->data;
    };

    auto a = buildAndRun(42);
    auto b = buildAndRun(42);
    assert(a == b);

    auto c = buildAndRun(99);
    assert(a == c);  // Same inputs with no time variation => same output
    std::cout << "[PASS] test_animationgraph_deterministic" << std::endl;
}
