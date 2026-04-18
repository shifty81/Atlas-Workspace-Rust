#include <iostream>
#include <cassert>
#include <cmath>
#include <memory>
#include "animation/DeterministicAnimationGraph.h"

using namespace atlas::animation;

void test_det_anim_add_nodes() {
    DeterministicAnimationGraph graph;
    auto id1 = graph.AddNode(std::make_unique<RestPoseNode>());
    auto id2 = graph.AddNode(std::make_unique<FKNode>());
    assert(graph.NodeCount() == 2);
    assert(id1 != id2);
    std::cout << "[PASS] test_det_anim_add_nodes" << std::endl;
}

void test_det_anim_remove_node() {
    DeterministicAnimationGraph graph;
    auto id1 = graph.AddNode(std::make_unique<RestPoseNode>());
    auto id2 = graph.AddNode(std::make_unique<FKNode>());
    graph.AddEdge({id1, 0, id2, 0});
    graph.RemoveNode(id1);
    assert(graph.NodeCount() == 1);
    std::cout << "[PASS] test_det_anim_remove_node" << std::endl;
}

void test_det_anim_compile_empty() {
    DeterministicAnimationGraph graph;
    assert(graph.Compile() == true);
    assert(graph.IsCompiled() == true);
    std::cout << "[PASS] test_det_anim_compile_empty" << std::endl;
}

void test_det_anim_rest_pose() {
    DeterministicAnimationGraph graph;
    auto id = graph.AddNode(std::make_unique<RestPoseNode>());
    assert(graph.Compile());
    BoneContext ctx;
    ctx.boneCount = 4;
    assert(graph.Execute(ctx));
    auto* out = graph.GetOutput(id, 0);
    assert(out != nullptr);
    assert(out->data.size() == 4 * 7);
    // Check identity quaternion for each bone
    for (uint32_t i = 0; i < 4; ++i) {
        assert(out->data[i * 7 + 0] == 0.0f); // posX
        assert(out->data[i * 7 + 6] == 1.0f); // rotW
    }
    std::cout << "[PASS] test_det_anim_rest_pose" << std::endl;
}

void test_det_anim_fk() {
    DeterministicAnimationGraph graph;
    auto restID = graph.AddNode(std::make_unique<RestPoseNode>());
    auto fkNode = std::make_unique<FKNode>();
    fkNode->rotationAngle = 0.5f;
    auto fkID = graph.AddNode(std::move(fkNode));
    graph.AddEdge({restID, 0, fkID, 0});
    assert(graph.Compile());
    BoneContext ctx;
    ctx.boneCount = 2;
    assert(graph.Execute(ctx));
    auto* out = graph.GetOutput(fkID, 0);
    assert(out != nullptr);
    // Each bone's rotY should be 0.5
    for (uint32_t i = 0; i < 2; ++i) {
        assert(std::fabs(out->data[i * 7 + 4] - 0.5f) < 1e-5f);
    }
    std::cout << "[PASS] test_det_anim_fk" << std::endl;
}

void test_det_anim_ik() {
    DeterministicAnimationGraph graph;
    auto restID = graph.AddNode(std::make_unique<RestPoseNode>());
    auto ikNode = std::make_unique<IKNode>();
    ikNode->targetX = 1.0f;
    ikNode->targetY = 0.0f;
    ikNode->targetZ = 0.0f;
    ikNode->iterations = 4;
    auto ikID = graph.AddNode(std::move(ikNode));
    graph.AddEdge({restID, 0, ikID, 0});
    assert(graph.Compile());
    BoneContext ctx;
    ctx.boneCount = 4;
    assert(graph.Execute(ctx));
    auto* out = graph.GetOutput(ikID, 0);
    assert(out != nullptr);
    assert(out->data.size() == 4 * 7);
    std::cout << "[PASS] test_det_anim_ik" << std::endl;
}

void test_det_anim_blend_tree() {
    DeterministicAnimationGraph graph;
    auto restID = graph.AddNode(std::make_unique<RestPoseNode>());
    auto fkNode = std::make_unique<FKNode>();
    fkNode->rotationAngle = 1.0f;
    auto fkID = graph.AddNode(std::move(fkNode));
    graph.AddEdge({restID, 0, fkID, 0});

    auto blendNode = std::make_unique<BlendTreeNode>();
    blendNode->weight = 0.5f;
    auto blendID = graph.AddNode(std::move(blendNode));
    graph.AddEdge({restID, 0, blendID, 0}); // PoseA = rest
    graph.AddEdge({fkID, 0, blendID, 1});    // PoseB = FK

    assert(graph.Compile());
    BoneContext ctx;
    ctx.boneCount = 2;
    assert(graph.Execute(ctx));
    auto* out = graph.GetOutput(blendID, 0);
    assert(out != nullptr);
    // Blended rotY should be ~0.5 (halfway between 0 and 1)
    assert(std::fabs(out->data[4] - 0.5f) < 1e-5f);
    std::cout << "[PASS] test_det_anim_blend_tree" << std::endl;
}

void test_det_anim_bone_mask() {
    DeterministicAnimationGraph graph;
    auto fkNode = std::make_unique<FKNode>();
    fkNode->rotationAngle = 1.0f;
    auto fkID = graph.AddNode(std::move(fkNode));

    auto maskNode = std::make_unique<BoneMaskNode>();
    maskNode->mask = {true, false, true, false};
    auto maskID = graph.AddNode(std::move(maskNode));
    graph.AddEdge({fkID, 0, maskID, 0});

    assert(graph.Compile());
    BoneContext ctx;
    ctx.boneCount = 4;
    assert(graph.Execute(ctx));
    auto* out = graph.GetOutput(maskID, 0);
    assert(out != nullptr);
    // Bone 0 (active): rotY should be 1.0
    assert(std::fabs(out->data[0 * 7 + 4] - 1.0f) < 1e-5f);
    // Bone 1 (masked out): all zeroed
    for (int c = 0; c < 7; ++c)
        assert(out->data[1 * 7 + c] == 0.0f);
    // Bone 2 (active): rotY should be 1.0
    assert(std::fabs(out->data[2 * 7 + 4] - 1.0f) < 1e-5f);
    std::cout << "[PASS] test_det_anim_bone_mask" << std::endl;
}

void test_det_anim_additive_blend() {
    DeterministicAnimationGraph graph;
    auto restID = graph.AddNode(std::make_unique<RestPoseNode>());
    auto fkNode = std::make_unique<FKNode>();
    fkNode->rotationAngle = 0.3f;
    auto fkID = graph.AddNode(std::move(fkNode));
    graph.AddEdge({restID, 0, fkID, 0});

    auto addNode = std::make_unique<AdditiveBlendNode>();
    addNode->strength = 0.5f;
    auto addID = graph.AddNode(std::move(addNode));
    graph.AddEdge({restID, 0, addID, 0}); // Base
    graph.AddEdge({fkID, 0, addID, 1});    // Additive

    assert(graph.Compile());
    BoneContext ctx;
    ctx.boneCount = 2;
    assert(graph.Execute(ctx));
    auto* out = graph.GetOutput(addID, 0);
    assert(out != nullptr);
    // Base rotY=0, additive rotY=0.3, strength=0.5 => 0 + 0.3*0.5 = 0.15
    assert(std::fabs(out->data[4] - 0.15f) < 1e-5f);
    std::cout << "[PASS] test_det_anim_additive_blend" << std::endl;
}

void test_det_anim_deterministic() {
    // Run the same graph twice and verify identical outputs
    auto run = []() {
        DeterministicAnimationGraph graph;
        auto restID = graph.AddNode(std::make_unique<RestPoseNode>());
        auto fkNode = std::make_unique<FKNode>();
        fkNode->rotationAngle = 0.25f;
        auto fkID = graph.AddNode(std::move(fkNode));
        graph.AddEdge({restID, 0, fkID, 0});
        auto ikNode = std::make_unique<IKNode>();
        ikNode->targetX = 2.0f;
        ikNode->targetY = 1.0f;
        ikNode->targetZ = 0.5f;
        auto ikID = graph.AddNode(std::move(ikNode));
        graph.AddEdge({fkID, 0, ikID, 0});
        graph.Compile();
        BoneContext ctx;
        ctx.boneCount = 4;
        ctx.tick = 42;
        graph.Execute(ctx);
        return graph.GetOutput(ikID, 0)->data;
    };
    auto result1 = run();
    auto result2 = run();
    assert(result1.size() == result2.size());
    for (size_t i = 0; i < result1.size(); ++i) {
        assert(result1[i] == result2[i]);
    }
    std::cout << "[PASS] test_det_anim_deterministic" << std::endl;
}
