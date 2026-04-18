#include "../editor/assistant/AssetGraphAssistant.h"
#include "../engine/asset_graph/AssetGraphExecutor.h"
#include <iostream>
#include <cassert>
#include <memory>

using namespace atlas::editor;
using namespace atlas::asset;

class NamedTestAssetNode : public AssetNode {
public:
    explicit NamedTestAssetNode(const std::string& name) : m_name(name) {}
    void Evaluate(const AssetContext&) override {}
    std::string Name() const override { return m_name; }
private:
    std::string m_name;
};

void test_assistant_suggest_empty() {
    AssetGraphAssistant assistant;

    auto suggestions = assistant.SuggestNodes();
    // With no executor, should suggest starting nodes
    assert(!suggestions.empty());
    assert(suggestions[0].name == "DamageState");

    std::cout << "[PASS] test_assistant_suggest_empty" << std::endl;
}

void test_assistant_suggest_with_executor() {
    AssetGraphExecutor executor;
    executor.AddNode(std::make_unique<NamedTestAssetNode>("Node1"));

    AssetGraphAssistant assistant;
    assistant.SetExecutor(&executor);

    auto suggestions = assistant.SuggestNodes();
    assert(!suggestions.empty());

    std::cout << "[PASS] test_assistant_suggest_with_executor" << std::endl;
}

void test_assistant_explain_no_executor() {
    AssetGraphAssistant assistant;

    auto explanation = assistant.ExplainGraph();
    assert(explanation == "No asset graph is loaded.");

    std::cout << "[PASS] test_assistant_explain_no_executor" << std::endl;
}

void test_assistant_explain_with_nodes() {
    AssetGraphExecutor executor;
    executor.AddNode(std::make_unique<NamedTestAssetNode>("Node1"));
    executor.AddNode(std::make_unique<NamedTestAssetNode>("Node2"));

    AssetGraphAssistant assistant;
    assistant.SetExecutor(&executor);

    auto explanation = assistant.ExplainGraph();
    assert(explanation.find("2 node(s)") != std::string::npos);
    assert(explanation.find("pipeline") != std::string::npos);

    std::cout << "[PASS] test_assistant_explain_with_nodes" << std::endl;
}

void test_assistant_mutate_no_executor() {
    AssetGraphAssistant assistant;

    auto result = assistant.MutateGraph(0.5f);
    assert(result == "No asset graph is loaded for mutation.");

    std::cout << "[PASS] test_assistant_mutate_no_executor" << std::endl;
}

void test_assistant_mutate_intensity() {
    AssetGraphExecutor executor;
    executor.AddNode(std::make_unique<NamedTestAssetNode>("Node1"));

    AssetGraphAssistant assistant;
    assistant.SetExecutor(&executor);

    auto low = assistant.MutateGraph(0.1f);
    assert(low.find("subtle") != std::string::npos);

    auto mid = assistant.MutateGraph(0.5f);
    assert(mid.find("moderate") != std::string::npos);

    auto high = assistant.MutateGraph(0.9f);
    assert(high.find("aggressive") != std::string::npos);

    auto zero = assistant.MutateGraph(0.0f);
    assert(zero.find("zero") != std::string::npos);

    std::cout << "[PASS] test_assistant_mutate_intensity" << std::endl;
}
