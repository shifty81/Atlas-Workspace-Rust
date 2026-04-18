#include "../engine/weapon/WeaponGraph.h"
#include "../engine/weapon/WeaponNodes.h"
#include <iostream>
#include <cassert>
#include <cmath>

void test_weapongraph_add_nodes() {
    atlas::weapon::WeaponGraph graph;
    auto node = std::make_unique<atlas::weapon::ReceiverNode>();
    node->baseDamage = 15.0f;
    auto id = graph.AddNode(std::move(node));
    assert(id == 1);
    assert(graph.NodeCount() == 1);
    std::cout << "[PASS] test_weapongraph_add_nodes" << std::endl;
}

void test_weapongraph_remove_node() {
    atlas::weapon::WeaponGraph graph;
    auto node = std::make_unique<atlas::weapon::ReceiverNode>();
    auto id = graph.AddNode(std::move(node));
    graph.RemoveNode(id);
    assert(graph.NodeCount() == 0);
    std::cout << "[PASS] test_weapongraph_remove_node" << std::endl;
}

void test_weapongraph_compile_empty() {
    atlas::weapon::WeaponGraph graph;
    assert(graph.Compile());
    assert(graph.IsCompiled());
    std::cout << "[PASS] test_weapongraph_compile_empty" << std::endl;
}

void test_weapongraph_compile_single_node() {
    atlas::weapon::WeaponGraph graph;
    auto node = std::make_unique<atlas::weapon::ReceiverNode>();
    node->baseDamage = 20.0f;
    node->baseFireRate = 2.0f;
    auto id = graph.AddNode(std::move(node));
    assert(graph.Compile());

    atlas::weapon::WeaponContext ctx{42, 0.0f, 1};
    assert(graph.Execute(ctx));

    auto* output = graph.GetOutput(id, 0);
    assert(output != nullptr);
    assert(output->type == atlas::weapon::WeaponPinType::Component);
    assert(output->data.size() >= 4);
    // Damage should be near baseDamage (20.0 ± 10%)
    assert(output->data[0] > 15.0f && output->data[0] < 25.0f);
    std::cout << "[PASS] test_weapongraph_compile_single_node" << std::endl;
}

void test_weapongraph_compile_chain() {
    atlas::weapon::WeaponGraph graph;

    // Receiver -> Barrel -> WeaponStats
    auto receiver = std::make_unique<atlas::weapon::ReceiverNode>();
    receiver->baseDamage = 25.0f;
    receiver->baseFireRate = 3.0f;
    auto receiverId = graph.AddNode(std::move(receiver));

    auto barrelId = graph.AddNode(std::make_unique<atlas::weapon::BarrelNode>());
    auto statsId = graph.AddNode(std::make_unique<atlas::weapon::WeaponStatsNode>());

    graph.AddEdge({receiverId, 0, barrelId, 0}); // Component -> Component
    graph.AddEdge({barrelId, 0, statsId, 0});     // Component -> Component

    assert(graph.Compile());

    atlas::weapon::WeaponContext ctx{12345, 0.0f, 1};
    assert(graph.Execute(ctx));

    auto* output = graph.GetOutput(statsId, 0);
    assert(output != nullptr);
    assert(output->type == atlas::weapon::WeaponPinType::Stats);
    assert(output->data.size() >= 5);
    // DPS = damage * fireRate, should be positive
    assert(output->data[4] > 0.0f);
    std::cout << "[PASS] test_weapongraph_compile_chain" << std::endl;
}

void test_weapongraph_execute() {
    atlas::weapon::WeaponGraph graph;

    auto receiverId = graph.AddNode(std::make_unique<atlas::weapon::ReceiverNode>());
    auto recoilId = graph.AddNode(std::make_unique<atlas::weapon::RecoilProfileNode>());

    graph.AddEdge({receiverId, 0, recoilId, 0}); // Component -> Component

    assert(graph.Compile());

    atlas::weapon::WeaponContext ctx{99, 0.0f, 1};
    assert(graph.Execute(ctx));

    auto* output = graph.GetOutput(recoilId, 0);
    assert(output != nullptr);
    assert(output->type == atlas::weapon::WeaponPinType::Profile);
    assert(output->data.size() == 3);
    // All recoil/spread values should be positive
    for (float v : output->data) {
        assert(v > 0.0f);
    }
    std::cout << "[PASS] test_weapongraph_execute" << std::endl;
}

void test_weapongraph_wear_degrades_stats() {
    atlas::weapon::WeaponGraph graph;

    auto receiver = std::make_unique<atlas::weapon::ReceiverNode>();
    receiver->baseDamage = 100.0f;
    auto receiverId = graph.AddNode(std::move(receiver));
    auto statsId = graph.AddNode(std::make_unique<atlas::weapon::WeaponStatsNode>());

    graph.AddEdge({receiverId, 0, statsId, 0});
    assert(graph.Compile());

    // Pristine weapon
    atlas::weapon::WeaponContext ctxNew{42, 0.0f, 1};
    assert(graph.Execute(ctxNew));
    float dmgNew = graph.GetOutput(statsId, 0)->data[0];

    // Worn weapon
    atlas::weapon::WeaponContext ctxWorn{42, 1.0f, 1};
    assert(graph.Execute(ctxWorn));
    float dmgWorn = graph.GetOutput(statsId, 0)->data[0];

    assert(dmgNew > dmgWorn);
    std::cout << "[PASS] test_weapongraph_wear_degrades_stats" << std::endl;
}

void test_weapongraph_deterministic() {
    auto buildAndRun = [](uint64_t seed) -> std::vector<float> {
        atlas::weapon::WeaponGraph graph;
        auto receiver = std::make_unique<atlas::weapon::ReceiverNode>();
        receiver->baseDamage = 30.0f;
        auto receiverId = graph.AddNode(std::move(receiver));
        auto barrelId = graph.AddNode(std::make_unique<atlas::weapon::BarrelNode>());
        auto statsId = graph.AddNode(std::make_unique<atlas::weapon::WeaponStatsNode>());
        graph.AddEdge({receiverId, 0, barrelId, 0});
        graph.AddEdge({barrelId, 0, statsId, 0});
        graph.Compile();
        atlas::weapon::WeaponContext ctx{seed, 0.0f, 1};
        graph.Execute(ctx);
        return graph.GetOutput(statsId, 0)->data;
    };

    auto a = buildAndRun(42);
    auto b = buildAndRun(42);
    assert(a == b);

    auto c = buildAndRun(99);
    assert(a != c);
    std::cout << "[PASS] test_weapongraph_deterministic" << std::endl;
}
