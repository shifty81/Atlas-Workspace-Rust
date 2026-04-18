#pragma once
#include "../ui/EditorPanel.h"
#include "../../engine/ui/UIDrawList.h"
#include <string>
#include <vector>
#include <cstdint>

namespace atlas::editor {

struct AssistantPrompt {
    std::string query;
    std::string context;
    uint64_t timestamp = 0;
};

struct AssistantSuggestion {
    std::string title;
    std::string description;
    std::string diffPreview;
    double confidence = 0.0;
    bool applied = false;
};

struct AssistantConversation {
    std::vector<AssistantPrompt> prompts;
    std::vector<AssistantSuggestion> suggestions;
};

class AtlasAssistantPanel : public EditorPanel {
public:
    const char* Name() const override { return "Atlas Assistant"; }
    void Draw() override;

    void SetContext(const std::string& context);
    std::string GetContext() const;

    void SubmitPrompt(const AssistantPrompt& prompt);
    const std::vector<AssistantPrompt>& Prompts() const;

    void AddSuggestion(const AssistantSuggestion& suggestion);
    const std::vector<AssistantSuggestion>& Suggestions() const;
    size_t SuggestionCount() const;
    void ApplySuggestion(size_t index);
    bool IsSuggestionApplied(size_t index) const;

    void SetDiffPreview(const std::string& diff);
    std::string GetDiffPreview() const;

    const AssistantConversation& GetConversation() const;
    void ClearConversation();

    const atlas::ui::UIDrawList& GetDrawList() const { return m_drawList; }

private:
    std::string m_context;
    std::string m_diffPreview;
    AssistantConversation m_conversation;
    atlas::ui::UIDrawList m_drawList;
};

} // namespace atlas::editor
