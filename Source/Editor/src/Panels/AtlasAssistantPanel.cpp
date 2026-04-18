#include "AtlasAssistantPanel.h"

namespace atlas::editor {

void AtlasAssistantPanel::Draw() {
    m_drawList.Clear();

    // Background
    m_drawList.DrawRect({0, 0, 500, 400}, {30, 30, 30, 255});

    // Title
    m_drawList.DrawRect({0, 0, 500, 24}, {50, 50, 50, 255});
    m_drawList.DrawText({4, 4, 200, 20}, "Atlas Assistant", {220, 220, 220, 255});

    // Conversation
    int32_t y = 28;
    for (const auto& prompt : m_conversation.prompts) {
        m_drawList.DrawText({4, y, 490, 16}, "> " + prompt.query, {100, 200, 255, 255});
        y += 18;
    }

    // Suggestions
    for (size_t i = 0; i < m_conversation.suggestions.size(); ++i) {
        const auto& s = m_conversation.suggestions[i];
        atlas::ui::UIColor color = s.applied
            ? atlas::ui::UIColor{100, 200, 100, 255}
            : atlas::ui::UIColor{200, 200, 200, 255};
        m_drawList.DrawText({4, y, 490, 16}, s.title, color);
        y += 16;
        m_drawList.DrawText({20, y, 474, 14}, s.description, {160, 160, 160, 255});
        y += 18;
    }
}

void AtlasAssistantPanel::SetContext(const std::string& context) {
    m_context = context;
}

std::string AtlasAssistantPanel::GetContext() const {
    return m_context;
}

void AtlasAssistantPanel::SubmitPrompt(const AssistantPrompt& prompt) {
    m_conversation.prompts.push_back(prompt);
}

const std::vector<AssistantPrompt>& AtlasAssistantPanel::Prompts() const {
    return m_conversation.prompts;
}

void AtlasAssistantPanel::AddSuggestion(const AssistantSuggestion& suggestion) {
    m_conversation.suggestions.push_back(suggestion);
}

const std::vector<AssistantSuggestion>& AtlasAssistantPanel::Suggestions() const {
    return m_conversation.suggestions;
}

size_t AtlasAssistantPanel::SuggestionCount() const {
    return m_conversation.suggestions.size();
}

void AtlasAssistantPanel::ApplySuggestion(size_t index) {
    if (index < m_conversation.suggestions.size()) {
        m_conversation.suggestions[index].applied = true;
    }
}

bool AtlasAssistantPanel::IsSuggestionApplied(size_t index) const {
    if (index < m_conversation.suggestions.size()) {
        return m_conversation.suggestions[index].applied;
    }
    return false;
}

void AtlasAssistantPanel::SetDiffPreview(const std::string& diff) {
    m_diffPreview = diff;
}

std::string AtlasAssistantPanel::GetDiffPreview() const {
    return m_diffPreview;
}

const AssistantConversation& AtlasAssistantPanel::GetConversation() const {
    return m_conversation;
}

void AtlasAssistantPanel::ClearConversation() {
    m_conversation.prompts.clear();
    m_conversation.suggestions.clear();
    m_context.clear();
    m_diffPreview.clear();
}

} // namespace atlas::editor
