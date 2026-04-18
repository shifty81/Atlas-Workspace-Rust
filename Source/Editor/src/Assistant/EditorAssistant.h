#pragma once
#include "../../engine/interaction/InteractionRouter.h"
#include "../../engine/ai/LLMBackend.h"

namespace atlas::editor {

class EditorAssistant final
    : public interaction::InteractionRouter
{
public:
    interaction::Utterance Route(
        const interaction::Intent& intent,
        const interaction::Utterance& source
    ) override;

    /// Set the LLM backend registry for AI-powered responses.
    /// When set, unknown intents are forwarded to the LLM.
    void SetLLMBackend(atlas::ai::LLMBackendRegistry* registry);

    /// Returns the currently configured LLM backend registry, or nullptr.
    atlas::ai::LLMBackendRegistry* GetLLMBackend() const;

private:
    atlas::ai::LLMBackendRegistry* m_llmRegistry = nullptr;
};

} // namespace atlas::editor
