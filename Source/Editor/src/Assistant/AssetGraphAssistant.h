#pragma once
#include <string>
#include <vector>

namespace atlas::asset { class AssetGraphExecutor; }
namespace atlas::ai { class LLMBackendRegistry; }

namespace atlas::editor {

struct NodeSuggestion {
    std::string name;
    std::string reason;
};

class AssetGraphAssistant {
public:
    void SetExecutor(asset::AssetGraphExecutor* executor) { m_executor = executor; }

    /// Set the LLM backend registry for AI-powered suggestions.
    void SetLLMBackend(atlas::ai::LLMBackendRegistry* registry) { m_llmRegistry = registry; }

    /// Returns the currently configured LLM backend registry, or nullptr.
    atlas::ai::LLMBackendRegistry* GetLLMBackend() const { return m_llmRegistry; }

    std::vector<NodeSuggestion> SuggestNodes();
    std::string ExplainGraph();
    std::string MutateGraph(float intensity);

private:
    asset::AssetGraphExecutor* m_executor = nullptr;
    atlas::ai::LLMBackendRegistry* m_llmRegistry = nullptr;
};

} // namespace atlas::editor
