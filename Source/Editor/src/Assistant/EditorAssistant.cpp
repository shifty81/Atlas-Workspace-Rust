#include "EditorAssistant.h"

namespace atlas::editor {

using namespace interaction;

Utterance EditorAssistant::Route(
    const Intent& intent,
    const Utterance& source)
{
    // Known intents with hardcoded responses (fast path)
    if (intent.name == "ExplainPerformance") {
        return {
            "This region is slow because a noise warp node "
            "is recalculated per LOD. Consider caching.",
            0, 0,
            { InteractionMode::Debug, true }
        };
    }

    if (intent.name == "ExplainGraphNode") {
        return {
            "This node modifies domain coordinates before "
            "sampling noise, increasing terrain variation.",
            0, 0,
            { InteractionMode::Debug, true }
        };
    }

    // If an LLM backend is available, forward the intent for an AI response
    if (m_llmRegistry) {
        atlas::ai::LLMRequest req;
        req.systemPrompt = "You are the Atlas Engine editor assistant. "
                           "Answer concisely about engine systems.";
        req.prompt = intent.name + ": " + source.text;
        req.temperature = 0.4f;
        req.maxTokens = 128;

        auto resp = m_llmRegistry->Complete(req);
        if (resp.success && !resp.text.empty()) {
            return {
                resp.text,
                0, 0,
                { InteractionMode::Debug, true }
            };
        }
    }

    return {
        "I don't have an explanation for that yet.",
        0, 0,
        { InteractionMode::Debug, true }
    };
}

void EditorAssistant::SetLLMBackend(atlas::ai::LLMBackendRegistry* registry) {
    m_llmRegistry = registry;
}

atlas::ai::LLMBackendRegistry* EditorAssistant::GetLLMBackend() const {
    return m_llmRegistry;
}

} // namespace atlas::editor
