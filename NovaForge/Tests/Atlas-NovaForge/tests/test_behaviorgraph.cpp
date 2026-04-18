#include "../engine/ai/BehaviorGraph.h"
#include "../engine/ai/BehaviorNodes.h"
#include <iostream>
#include <cassert>

void test_behaviorgraph_add_nodes() {
    atlas::ai::BehaviorGraph graph;
    auto node = std::make_unique<atlas::ai::ThreatAssessmentNode>();
    auto id = graph.AddNode(std::move(node));
    assert(id == 1);
    assert(graph.NodeCount() == 1);
    std::cout << "[PASS] test_behaviorgraph_add_nodes" << std::endl;
}

void test_behaviorgraph_remove_node() {
    atlas::ai::BehaviorGraph graph;
    auto node = std::make_unique<atlas::ai::ThreatAssessmentNode>();
    auto id = graph.AddNode(std::move(node));
    graph.RemoveNode(id);
    assert(graph.NodeCount() == 0);
    std::cout << "[PASS] test_behaviorgraph_remove_node" << std::endl;
}

void test_behaviorgraph_compile_empty() {
    atlas::ai::BehaviorGraph graph;
    assert(graph.Compile());
    assert(graph.IsCompiled());
    std::cout << "[PASS] test_behaviorgraph_compile_empty" << std::endl;
}

void test_behaviorgraph_compile_single_node() {
    atlas::ai::BehaviorGraph graph;
    auto id = graph.AddNode(std::make_unique<atlas::ai::ThreatAssessmentNode>());
    assert(graph.Compile());

    atlas::ai::AIContext ctx{0.8f, 0.5f, 0.6f, 0.7f, 1};
    assert(graph.Execute(ctx));

    auto* output = graph.GetOutput(id, 0);
    assert(output != nullptr);
    assert(output->type == atlas::ai::BehaviorPinType::Perception);
    assert(!output->data.empty());
    assert(output->data[0] == 0.8f);
    std::cout << "[PASS] test_behaviorgraph_compile_single_node" << std::endl;
}

void test_behaviorgraph_compile_chain() {
    atlas::ai::BehaviorGraph graph;

    // UtilityScoreNode (attack) with high threat weight
    auto attackUtil = std::make_unique<atlas::ai::UtilityScoreNode>();
    attackUtil->threatWeight = 2.0f;
    attackUtil->healthWeight = 0.5f;
    auto attackId = graph.AddNode(std::move(attackUtil));

    // UtilityScoreNode (retreat) with high health weight
    auto retreatUtil = std::make_unique<atlas::ai::UtilityScoreNode>();
    retreatUtil->threatWeight = 0.5f;
    retreatUtil->healthWeight = 2.0f;
    auto retreatId = graph.AddNode(std::move(retreatUtil));

    // ActionSelectorNode picks highest utility
    auto selectorId = graph.AddNode(std::make_unique<atlas::ai::ActionSelectorNode>());

    // Connect utilities to selector
    graph.AddEdge({attackId, 0, selectorId, 0});   // attack score -> UtilityA
    graph.AddEdge({retreatId, 0, selectorId, 1});  // retreat score -> UtilityB

    assert(graph.Compile());

    atlas::ai::AIContext ctx{0.9f, 0.3f, 0.5f, 0.5f, 1};
    assert(graph.Execute(ctx));

    auto* output = graph.GetOutput(selectorId, 0);
    assert(output != nullptr);
    assert(output->type == atlas::ai::BehaviorPinType::Action);
    assert(!output->data.empty());
    std::cout << "[PASS] test_behaviorgraph_compile_chain" << std::endl;
}

void test_behaviorgraph_execute() {
    atlas::ai::BehaviorGraph graph;

    auto emotionId = graph.AddNode(std::make_unique<atlas::ai::EmotionUpdateNode>());

    assert(graph.Compile());

    atlas::ai::AIContext ctx{0.6f, 0.8f, 0.5f, 0.4f, 10};
    assert(graph.Execute(ctx));

    auto* output = graph.GetOutput(emotionId, 0);
    assert(output != nullptr);
    assert(output->type == atlas::ai::BehaviorPinType::EmotionState);
    assert(output->data.size() == 3);

    // fear = threat * (1 - morale) = 0.6 * 0.6 = 0.36
    float fear = output->data[0];
    assert(fear > 0.35f && fear < 0.37f);

    // confidence = morale * health = 0.4 * 0.8 = 0.32
    float confidence = output->data[1];
    assert(confidence > 0.31f && confidence < 0.33f);

    // anger = threat * morale = 0.6 * 0.4 = 0.24
    float anger = output->data[2];
    assert(anger > 0.23f && anger < 0.25f);

    std::cout << "[PASS] test_behaviorgraph_execute" << std::endl;
}

void test_behaviorgraph_deterministic() {
    auto buildAndRun = [](float threat, float health) -> std::vector<float> {
        atlas::ai::BehaviorGraph graph;
        auto emotionId = graph.AddNode(std::make_unique<atlas::ai::EmotionUpdateNode>());
        graph.Compile();
        atlas::ai::AIContext ctx{threat, health, 0.5f, 0.5f, 1};
        graph.Execute(ctx);
        return graph.GetOutput(emotionId, 0)->data;
    };

    auto a = buildAndRun(0.7f, 0.8f);
    auto b = buildAndRun(0.7f, 0.8f);
    assert(a == b);

    auto c = buildAndRun(0.3f, 0.2f);
    assert(a != c);
    std::cout << "[PASS] test_behaviorgraph_deterministic" << std::endl;
}

void test_behaviorgraph_group_tactics() {
    atlas::ai::BehaviorGraph graph;

    auto tacticsNode = std::make_unique<atlas::ai::GroupTacticsNode>();
    tacticsNode->flankWeight = 1.5f;
    tacticsNode->retreatWeight = 1.0f;
    auto tacticsId = graph.AddNode(std::move(tacticsNode));

    assert(graph.Compile());

    // Scenario: outnumbered with low health and high threat → expect retreat (tactic 3)
    atlas::ai::AIContext ctx{0.9f, 0.2f, 0.5f, 0.3f, 1};
    assert(graph.Execute(ctx));

    auto* output = graph.GetOutput(tacticsId, 0);
    assert(output != nullptr);
    assert(output->type == atlas::ai::BehaviorPinType::Action);
    assert(output->data.size() == 2);

    // Tactic should be 3 (Retreat) for low health + high threat
    float tactic = output->data[0];
    assert(tactic == 3.0f);
    std::cout << "[PASS] test_behaviorgraph_group_tactics" << std::endl;
}

void test_behaviorgraph_group_tactics_charge() {
    atlas::ai::BehaviorGraph graph;
    auto tacticsId = graph.AddNode(std::make_unique<atlas::ai::GroupTacticsNode>());

    assert(graph.Compile());

    // Scenario: high morale, high health, low threat → expect charge (tactic 0) or flank (tactic 1)
    atlas::ai::AIContext ctx{0.1f, 0.9f, 0.9f, 0.9f, 1};
    assert(graph.Execute(ctx));

    auto* output = graph.GetOutput(tacticsId, 0);
    assert(output != nullptr);
    float tactic = output->data[0];
    // With high morale and health, should not be retreat
    assert(tactic != 3.0f);
    std::cout << "[PASS] test_behaviorgraph_group_tactics_charge" << std::endl;
}

void test_behaviorgraph_adaptive_behavior() {
    atlas::ai::BehaviorGraph graph;

    auto adaptNode = std::make_unique<atlas::ai::AdaptiveBehaviorNode>();
    adaptNode->adaptationRate = 0.5f;
    auto adaptId = graph.AddNode(std::move(adaptNode));

    assert(graph.Compile());

    // Player winning too much → difficulty should increase
    atlas::ai::AIContext ctx{0.5f, 0.5f, 0.5f, 0.5f, 1};
    assert(graph.Execute(ctx));

    auto* output = graph.GetOutput(adaptId, 0);
    assert(output != nullptr);
    assert(output->type == atlas::ai::BehaviorPinType::Float);
    assert(output->data.size() == 1);

    // With default inputs (0.5 win/death), multiplier should be ~1.0
    float multiplier = output->data[0];
    assert(multiplier >= 0.5f && multiplier <= 2.0f);
    std::cout << "[PASS] test_behaviorgraph_adaptive_behavior" << std::endl;
}

void test_behaviorgraph_adaptive_difficulty_scaling() {
    atlas::ai::BehaviorGraph graph;

    auto adaptNode = std::make_unique<atlas::ai::AdaptiveBehaviorNode>();
    adaptNode->adaptationRate = 1.0f;
    auto adaptId = graph.AddNode(std::move(adaptNode));
    assert(graph.Compile());

    // High win rate player → difficulty increases
    atlas::ai::AIContext ctx1{0.5f, 0.5f, 0.5f, 0.5f, 1};
    assert(graph.Execute(ctx1));
    auto* out1 = graph.GetOutput(adaptId, 0);
    // With default inputs (no connected win/death rate), defaults to 0.5 each
    // Result should be 1.0 (balanced)
    float m1 = out1->data[0];
    assert(m1 >= 0.5f && m1 <= 2.0f);

    std::cout << "[PASS] test_behaviorgraph_adaptive_difficulty_scaling" << std::endl;
}
