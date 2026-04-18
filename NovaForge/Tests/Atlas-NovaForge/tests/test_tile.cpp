#include "../engine/tile/TileGraph.h"
#include "../engine/tile/TileNodes.h"
#include <iostream>
#include <cassert>

void test_tilegraph_add_nodes() {
    atlas::tile::TileGraph graph;
    auto node = std::make_unique<atlas::tile::BaseTileNode>();
    node->tileID = 1.0f;
    auto id = graph.AddNode(std::move(node));
    assert(id == 1);
    assert(graph.NodeCount() == 1);
    std::cout << "[PASS] test_tilegraph_add_nodes" << std::endl;
}

void test_tilegraph_remove_node() {
    atlas::tile::TileGraph graph;
    auto node = std::make_unique<atlas::tile::BaseTileNode>();
    node->tileID = 1.0f;
    auto id = graph.AddNode(std::move(node));
    graph.RemoveNode(id);
    assert(graph.NodeCount() == 0);
    std::cout << "[PASS] test_tilegraph_remove_node" << std::endl;
}

void test_tilegraph_compile_empty() {
    atlas::tile::TileGraph graph;
    assert(graph.Compile());
    assert(graph.IsCompiled());
    std::cout << "[PASS] test_tilegraph_compile_empty" << std::endl;
}

void test_tilegraph_compile_single_node() {
    atlas::tile::TileGraph graph;
    auto node = std::make_unique<atlas::tile::BaseTileNode>();
    node->tileID = 2.0f;
    auto id = graph.AddNode(std::move(node));
    assert(graph.Compile());

    atlas::tile::TileGenContext ctx{42, 8, 8};
    assert(graph.Execute(ctx));

    auto* output = graph.GetOutput(id, 0);
    assert(output != nullptr);
    assert(output->type == atlas::tile::TilePinType::TileID);
    assert(!output->data.empty());
    assert(output->data[0] == 2.0f);
    std::cout << "[PASS] test_tilegraph_compile_single_node" << std::endl;
}

void test_tilegraph_compile_chain() {
    atlas::tile::TileGraph graph;

    // BaseTile(grass=0) -> TileSelect input 2 (TileBelow)
    auto grassNode = std::make_unique<atlas::tile::BaseTileNode>();
    grassNode->tileID = 0.0f;
    auto grassId = graph.AddNode(std::move(grassNode));

    // BaseTile(stone=1) -> TileSelect input 3 (TileAbove)
    auto stoneNode = std::make_unique<atlas::tile::BaseTileNode>();
    stoneNode->tileID = 1.0f;
    auto stoneId = graph.AddNode(std::move(stoneNode));

    // NoiseField -> TileSelect input 0 (Field)
    auto noiseId = graph.AddNode(std::make_unique<atlas::tile::NoiseFieldNode>());

    // TileSelect
    auto selectId = graph.AddNode(std::make_unique<atlas::tile::TileSelectNode>());

    graph.AddEdge({noiseId, 0, selectId, 0});  // Field -> Field
    graph.AddEdge({grassId, 0, selectId, 2});  // TileID -> TileBelow
    graph.AddEdge({stoneId, 0, selectId, 3});  // TileID -> TileAbove

    assert(graph.Compile());

    atlas::tile::TileGenContext ctx{12345, 16, 16};
    assert(graph.Execute(ctx));

    auto* output = graph.GetOutput(selectId, 0);
    assert(output != nullptr);
    assert(output->type == atlas::tile::TilePinType::TileMap);
    assert(output->data.size() == 16 * 16);

    // All values should be either 0 (grass) or 1 (stone)
    for (float v : output->data) {
        assert(v == 0.0f || v == 1.0f);
    }
    std::cout << "[PASS] test_tilegraph_compile_chain" << std::endl;
}

void test_tilegraph_execute() {
    atlas::tile::TileGraph graph;

    auto noiseId = graph.AddNode(std::make_unique<atlas::tile::NoiseFieldNode>());

    assert(graph.Compile());

    atlas::tile::TileGenContext ctx{99, 8, 8};
    assert(graph.Execute(ctx));

    auto* output = graph.GetOutput(noiseId, 0);
    assert(output != nullptr);
    assert(output->type == atlas::tile::TilePinType::Float);
    assert(output->data.size() == 64);

    // All noise values should be in [0, 1]
    for (float v : output->data) {
        assert(v >= 0.0f && v <= 1.0f);
    }
    std::cout << "[PASS] test_tilegraph_execute" << std::endl;
}

void test_tilegraph_deterministic() {
    auto buildAndRun = [](uint32_t seed) -> std::vector<float> {
        atlas::tile::TileGraph graph;
        auto noiseId = graph.AddNode(std::make_unique<atlas::tile::NoiseFieldNode>());
        auto selectId = graph.AddNode(std::make_unique<atlas::tile::TileSelectNode>());
        graph.AddEdge({noiseId, 0, selectId, 0});
        graph.Compile();
        atlas::tile::TileGenContext ctx{seed, 16, 16};
        graph.Execute(ctx);
        return graph.GetOutput(selectId, 0)->data;
    };

    auto a = buildAndRun(42);
    auto b = buildAndRun(42);
    assert(a == b);

    auto c = buildAndRun(99);
    assert(a != c);
    std::cout << "[PASS] test_tilegraph_deterministic" << std::endl;
}
