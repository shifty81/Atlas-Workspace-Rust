#include "../engine/sound/SoundGraph.h"
#include "../engine/sound/SoundNodes.h"
#include <iostream>
#include <cassert>
#include <cmath>

void test_soundgraph_add_nodes() {
    atlas::sound::SoundGraph graph;
    auto id = graph.AddNode(std::make_unique<atlas::sound::OscillatorNode>());
    assert(id == 1);
    assert(graph.NodeCount() == 1);
    std::cout << "[PASS] test_soundgraph_add_nodes" << std::endl;
}

void test_soundgraph_remove_node() {
    atlas::sound::SoundGraph graph;
    auto id = graph.AddNode(std::make_unique<atlas::sound::OscillatorNode>());
    graph.RemoveNode(id);
    assert(graph.NodeCount() == 0);
    std::cout << "[PASS] test_soundgraph_remove_node" << std::endl;
}

void test_soundgraph_compile_empty() {
    atlas::sound::SoundGraph graph;
    assert(graph.Compile());
    assert(graph.IsCompiled());
    std::cout << "[PASS] test_soundgraph_compile_empty" << std::endl;
}

void test_soundgraph_compile_single_node() {
    atlas::sound::SoundGraph graph;
    auto id = graph.AddNode(std::make_unique<atlas::sound::OscillatorNode>());
    assert(graph.Compile());

    atlas::sound::SoundContext ctx{44100, 256, 42};
    assert(graph.Execute(ctx));

    auto* output = graph.GetOutput(id, 0);
    assert(output != nullptr);
    assert(output->type == atlas::sound::SoundPinType::AudioBuffer);
    assert(output->data.size() == 256);
    std::cout << "[PASS] test_soundgraph_compile_single_node" << std::endl;
}

void test_soundgraph_compile_chain() {
    atlas::sound::SoundGraph graph;

    auto oscId = graph.AddNode(std::make_unique<atlas::sound::OscillatorNode>());
    auto gainId = graph.AddNode(std::make_unique<atlas::sound::GainNode>());

    // Oscillator AudioBuffer -> Gain AudioBuffer input
    graph.AddEdge({oscId, 0, gainId, 0});

    assert(graph.Compile());

    atlas::sound::SoundContext ctx{44100, 128, 0};
    assert(graph.Execute(ctx));

    auto* output = graph.GetOutput(gainId, 0);
    assert(output != nullptr);
    assert(output->type == atlas::sound::SoundPinType::AudioBuffer);
    assert(output->data.size() == 128);
    std::cout << "[PASS] test_soundgraph_compile_chain" << std::endl;
}

void test_soundgraph_execute() {
    atlas::sound::SoundGraph graph;

    // Two oscillators -> Mix
    auto osc1 = graph.AddNode(std::make_unique<atlas::sound::OscillatorNode>());
    auto osc2 = graph.AddNode(std::make_unique<atlas::sound::OscillatorNode>());
    auto mixId = graph.AddNode(std::make_unique<atlas::sound::MixNode>());

    graph.AddEdge({osc1, 0, mixId, 0});
    graph.AddEdge({osc2, 0, mixId, 1});

    assert(graph.Compile());

    atlas::sound::SoundContext ctx{44100, 64, 0};
    assert(graph.Execute(ctx));

    auto* output = graph.GetOutput(mixId, 0);
    assert(output != nullptr);
    assert(output->type == atlas::sound::SoundPinType::AudioBuffer);
    assert(output->data.size() == 64);

    // Mixed identical signals should equal the original
    auto* osc1Out = graph.GetOutput(osc1, 0);
    for (size_t i = 0; i < 64; ++i) {
        float expected = osc1Out->data[i]; // (a+a)/2 == a
        assert(std::fabs(output->data[i] - expected) < 1e-5f);
    }
    std::cout << "[PASS] test_soundgraph_execute" << std::endl;
}

void test_soundgraph_deterministic() {
    auto buildAndRun = [](uint64_t seed) -> std::vector<float> {
        atlas::sound::SoundGraph graph;
        auto oscId = graph.AddNode(std::make_unique<atlas::sound::OscillatorNode>());
        auto gainId = graph.AddNode(std::make_unique<atlas::sound::GainNode>());
        graph.AddEdge({oscId, 0, gainId, 0});
        graph.Compile();
        atlas::sound::SoundContext ctx{44100, 256, seed};
        graph.Execute(ctx);
        return graph.GetOutput(gainId, 0)->data;
    };

    auto a = buildAndRun(42);
    auto b = buildAndRun(42);
    assert(a == b);

    // Running a second time with the same parameters must still match
    auto c = buildAndRun(42);
    assert(a == c);
    std::cout << "[PASS] test_soundgraph_deterministic" << std::endl;
}
