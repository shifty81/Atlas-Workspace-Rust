#include "../engine/asset_graph/AssetGraph.h"
#include "../engine/asset_graph/AssetGraphExecutor.h"
#include "../engine/asset_graph/DamageState.h"
#include <iostream>
#include <cassert>

using namespace atlas::asset;

// Test node that tracks evaluation
class TestAssetNode : public AssetNode {
public:
    bool evaluated = false;
    AssetSeed lastSeed = 0;
    uint32_t lastLod = 0;

    void Evaluate(const AssetContext& ctx) override {
        evaluated = true;
        lastSeed = ctx.seed;
        lastLod = ctx.lod;
    }

    std::string Name() const override { return "TestNode"; }
};

void test_asset_context() {
    AssetContext ctx{ 12345, 2 };
    assert(ctx.seed == 12345);
    assert(ctx.lod == 2);
    std::cout << "[PASS] test_asset_context" << std::endl;
}

void test_asset_graph_executor_empty() {
    AssetGraphExecutor executor;
    assert(executor.NodeCount() == 0);
    AssetContext ctx{ 1, 0 };
    executor.Execute(ctx);  // should not crash
    std::cout << "[PASS] test_asset_graph_executor_empty" << std::endl;
}

void test_asset_graph_executor_single_node() {
    AssetGraphExecutor executor;
    auto node = std::make_unique<TestAssetNode>();
    auto* ptr = node.get();
    executor.AddNode(std::move(node));

    assert(executor.NodeCount() == 1);

    AssetContext ctx{ 42, 3 };
    executor.Execute(ctx);

    assert(ptr->evaluated);
    assert(ptr->lastSeed == 42);
    assert(ptr->lastLod == 3);
    std::cout << "[PASS] test_asset_graph_executor_single_node" << std::endl;
}

void test_asset_graph_executor_multiple_nodes() {
    AssetGraphExecutor executor;
    auto n1 = std::make_unique<TestAssetNode>();
    auto n2 = std::make_unique<TestAssetNode>();
    auto* p1 = n1.get();
    auto* p2 = n2.get();
    executor.AddNode(std::move(n1));
    executor.AddNode(std::move(n2));

    assert(executor.NodeCount() == 2);

    AssetContext ctx{ 99, 1 };
    executor.Execute(ctx);

    assert(p1->evaluated);
    assert(p2->evaluated);
    std::cout << "[PASS] test_asset_graph_executor_multiple_nodes" << std::endl;
}

void test_damage_state() {
    DamageState state;
    assert(state.EventCount() == 0);

    state.Apply({ 0.5f, 2.0f, 1 });
    assert(state.EventCount() == 1);

    state.Apply({ 0.8f, 3.0f, 2 });
    assert(state.EventCount() == 2);

    assert(state.events[0].magnitude == 0.5f);
    assert(state.events[1].hitRegion == 2);

    state.Clear();
    assert(state.EventCount() == 0);
    std::cout << "[PASS] test_damage_state" << std::endl;
}
