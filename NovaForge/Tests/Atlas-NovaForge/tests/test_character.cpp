#include "../engine/character/CharacterGraph.h"
#include "../engine/character/CharacterNodes.h"
#include <iostream>
#include <cassert>

void test_charactergraph_add_nodes() {
    atlas::character::CharacterGraph graph;
    auto node = std::make_unique<atlas::character::BaseBodyNode>();
    auto id = graph.AddNode(std::move(node));
    assert(id == 1);
    assert(graph.NodeCount() == 1);
    std::cout << "[PASS] test_charactergraph_add_nodes" << std::endl;
}

void test_charactergraph_remove_node() {
    atlas::character::CharacterGraph graph;
    auto node = std::make_unique<atlas::character::BaseBodyNode>();
    auto id = graph.AddNode(std::move(node));
    graph.RemoveNode(id);
    assert(graph.NodeCount() == 0);
    std::cout << "[PASS] test_charactergraph_remove_node" << std::endl;
}

void test_charactergraph_compile_empty() {
    atlas::character::CharacterGraph graph;
    assert(graph.Compile());
    assert(graph.IsCompiled());
    std::cout << "[PASS] test_charactergraph_compile_empty" << std::endl;
}

void test_charactergraph_compile_single_node() {
    atlas::character::CharacterGraph graph;
    auto id = graph.AddNode(std::make_unique<atlas::character::BaseBodyNode>());
    assert(graph.Compile());

    atlas::character::CharacterContext ctx{42, 1, 30.0f, 0.5f};
    assert(graph.Execute(ctx));

    auto* output = graph.GetOutput(id, 0);
    assert(output != nullptr);
    assert(output->type == atlas::character::CharacterPinType::Mesh);
    assert(output->data.size() == 4); // height, mass, armRatio, legRatio
    std::cout << "[PASS] test_charactergraph_compile_single_node" << std::endl;
}

void test_charactergraph_compile_chain() {
    atlas::character::CharacterGraph graph;

    auto bodyId = graph.AddNode(std::make_unique<atlas::character::BaseBodyNode>());
    auto matId = graph.AddNode(std::make_unique<atlas::character::MaterialNode>());
    auto equipId = graph.AddNode(std::make_unique<atlas::character::EquipmentNode>());

    // Body -> Equipment (Mesh -> Mesh)
    graph.AddEdge({bodyId, 0, equipId, 0});

    assert(graph.Compile());

    atlas::character::CharacterContext ctx{12345, 3, 25.0f, 0.7f};
    assert(graph.Execute(ctx));

    auto* bodyOut = graph.GetOutput(bodyId, 0);
    assert(bodyOut != nullptr);
    assert(bodyOut->type == atlas::character::CharacterPinType::Mesh);

    auto* matOut = graph.GetOutput(matId, 0);
    assert(matOut != nullptr);
    assert(matOut->type == atlas::character::CharacterPinType::Material);
    assert(matOut->data.size() == 3); // skinTone, hairColor, eyeColor

    auto* equipOut = graph.GetOutput(equipId, 0);
    assert(equipOut != nullptr);
    assert(equipOut->type == atlas::character::CharacterPinType::Mesh);
    // Equipment output contains body data passed through
    assert(equipOut->data.size() >= bodyOut->data.size());

    std::cout << "[PASS] test_charactergraph_compile_chain" << std::endl;
}

void test_charactergraph_execute() {
    atlas::character::CharacterGraph graph;

    auto bodyId = graph.AddNode(std::make_unique<atlas::character::BaseBodyNode>());

    assert(graph.Compile());

    atlas::character::CharacterContext ctx{99, 2, 40.0f, 0.3f};
    assert(graph.Execute(ctx));

    auto* output = graph.GetOutput(bodyId, 0);
    assert(output != nullptr);
    assert(output->type == atlas::character::CharacterPinType::Mesh);
    assert(output->data.size() == 4);

    // Height should be in valid range [1.5, 2.1]
    assert(output->data[0] >= 1.5f && output->data[0] <= 2.1f);
    // Mass should be positive
    assert(output->data[1] > 0.0f);

    std::cout << "[PASS] test_charactergraph_execute" << std::endl;
}

void test_charactergraph_deterministic() {
    auto buildAndRun = [](uint64_t seed) -> std::vector<float> {
        atlas::character::CharacterGraph graph;
        auto bodyId = graph.AddNode(std::make_unique<atlas::character::BaseBodyNode>());
        auto matId = graph.AddNode(std::make_unique<atlas::character::MaterialNode>());
        auto equipId = graph.AddNode(std::make_unique<atlas::character::EquipmentNode>());
        graph.AddEdge({bodyId, 0, equipId, 0});
        graph.Compile();
        atlas::character::CharacterContext ctx{seed, 1, 30.0f, 0.5f};
        graph.Execute(ctx);
        return graph.GetOutput(equipId, 0)->data;
    };

    auto a = buildAndRun(42);
    auto b = buildAndRun(42);
    assert(a == b);

    auto c = buildAndRun(99);
    assert(a != c);
    std::cout << "[PASS] test_charactergraph_deterministic" << std::endl;
}

void test_charactergraph_facial_features() {
    atlas::character::CharacterGraph graph;
    auto faceId = graph.AddNode(std::make_unique<atlas::character::FacialFeatureNode>());
    assert(graph.Compile());

    atlas::character::CharacterContext ctx{42, 1, 30.0f, 0.5f};
    assert(graph.Execute(ctx));

    auto* output = graph.GetOutput(faceId, 0);
    assert(output != nullptr);
    assert(output->type == atlas::character::CharacterPinType::Mesh);
    assert(output->data.size() == 5); // noseWidth, eyeSpacing, jawWidth, browHeight, lipFullness

    // All facial features should be in valid normalized ranges
    assert(output->data[0] >= 0.3f && output->data[0] <= 0.7f);  // noseWidth
    assert(output->data[1] >= 0.4f && output->data[1] <= 0.7f);  // eyeSpacing
    assert(output->data[2] >= 0.5f && output->data[2] <= 1.0f);  // jawWidth (age may add up to 0.05)
    assert(output->data[3] >= 0.2f && output->data[3] <= 0.7f);  // browHeight (age may subtract up to 0.03)
    assert(output->data[4] >= 0.2f && output->data[4] <= 0.8f);  // lipFullness
    std::cout << "[PASS] test_charactergraph_facial_features" << std::endl;
}

void test_charactergraph_hair_style() {
    atlas::character::CharacterGraph graph;
    auto hairId = graph.AddNode(std::make_unique<atlas::character::HairStyleNode>());
    assert(graph.Compile());

    atlas::character::CharacterContext ctx{42, 1, 30.0f, 0.5f};
    assert(graph.Execute(ctx));

    auto* output = graph.GetOutput(hairId, 0);
    assert(output != nullptr);
    assert(output->type == atlas::character::CharacterPinType::Material);
    assert(output->data.size() == 6); // styleIndex, length, density, R, G, B

    // Style index should be in [0, 8)
    assert(output->data[0] >= 0.0f && output->data[0] < 8.0f);
    // Length and density should be positive
    assert(output->data[1] > 0.0f);
    assert(output->data[2] > 0.0f);
    // Color components should be in [0, 1]
    assert(output->data[3] >= 0.0f && output->data[3] <= 1.0f);
    assert(output->data[4] >= 0.0f && output->data[4] <= 1.0f);
    assert(output->data[5] >= 0.0f && output->data[5] <= 1.0f);
    std::cout << "[PASS] test_charactergraph_hair_style" << std::endl;
}

void test_charactergraph_facial_deterministic() {
    auto buildAndRun = [](uint64_t seed) -> std::vector<float> {
        atlas::character::CharacterGraph graph;
        auto faceId = graph.AddNode(std::make_unique<atlas::character::FacialFeatureNode>());
        graph.Compile();
        atlas::character::CharacterContext ctx{seed, 1, 30.0f, 0.5f};
        graph.Execute(ctx);
        return graph.GetOutput(faceId, 0)->data;
    };

    auto a = buildAndRun(42);
    auto b = buildAndRun(42);
    assert(a == b);

    auto c = buildAndRun(99);
    assert(a != c);
    std::cout << "[PASS] test_charactergraph_facial_deterministic" << std::endl;
}
