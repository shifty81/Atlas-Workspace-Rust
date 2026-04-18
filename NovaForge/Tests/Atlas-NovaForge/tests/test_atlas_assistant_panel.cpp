#include "../editor/panels/AtlasAssistantPanel.h"
#include <iostream>
#include <cassert>

void test_assistant_panel_name() {
    atlas::editor::AtlasAssistantPanel panel;
    assert(std::string(panel.Name()) == "Atlas Assistant");
    std::cout << "[PASS] test_assistant_panel_name" << std::endl;
}

void test_assistant_panel_context() {
    atlas::editor::AtlasAssistantPanel panel;
    assert(panel.GetContext().empty());
    panel.SetContext("entity:Player");
    assert(panel.GetContext() == "entity:Player");
    std::cout << "[PASS] test_assistant_panel_context" << std::endl;
}

void test_assistant_panel_submit_prompt() {
    atlas::editor::AtlasAssistantPanel panel;
    atlas::editor::AssistantPrompt prompt;
    prompt.query = "How do I add a component?";
    prompt.context = "ECS";
    prompt.timestamp = 100;
    panel.SubmitPrompt(prompt);
    assert(panel.Prompts().size() == 1);
    assert(panel.Prompts()[0].query == "How do I add a component?");
    std::cout << "[PASS] test_assistant_panel_submit_prompt" << std::endl;
}

void test_assistant_panel_suggestions() {
    atlas::editor::AtlasAssistantPanel panel;
    atlas::editor::AssistantSuggestion s;
    s.title = "Add Health Component";
    s.description = "Adds a Health component to the selected entity";
    s.confidence = 0.9;
    panel.AddSuggestion(s);
    assert(panel.SuggestionCount() == 1);
    assert(panel.Suggestions()[0].title == "Add Health Component");
    std::cout << "[PASS] test_assistant_panel_suggestions" << std::endl;
}

void test_assistant_panel_apply_suggestion() {
    atlas::editor::AtlasAssistantPanel panel;
    atlas::editor::AssistantSuggestion s;
    s.title = "Fix";
    panel.AddSuggestion(s);
    assert(!panel.IsSuggestionApplied(0));
    panel.ApplySuggestion(0);
    assert(panel.IsSuggestionApplied(0));
    std::cout << "[PASS] test_assistant_panel_apply_suggestion" << std::endl;
}

void test_assistant_panel_diff_preview() {
    atlas::editor::AtlasAssistantPanel panel;
    assert(panel.GetDiffPreview().empty());
    panel.SetDiffPreview("- old line\n+ new line");
    assert(panel.GetDiffPreview() == "- old line\n+ new line");
    std::cout << "[PASS] test_assistant_panel_diff_preview" << std::endl;
}

void test_assistant_panel_conversation() {
    atlas::editor::AtlasAssistantPanel panel;
    atlas::editor::AssistantPrompt p;
    p.query = "Q1";
    panel.SubmitPrompt(p);
    atlas::editor::AssistantSuggestion s;
    s.title = "S1";
    panel.AddSuggestion(s);
    const auto& conv = panel.GetConversation();
    assert(conv.prompts.size() == 1);
    assert(conv.suggestions.size() == 1);
    std::cout << "[PASS] test_assistant_panel_conversation" << std::endl;
}

void test_assistant_panel_clear() {
    atlas::editor::AtlasAssistantPanel panel;
    panel.SetContext("ctx");
    panel.SetDiffPreview("diff");
    atlas::editor::AssistantPrompt p;
    p.query = "Q";
    panel.SubmitPrompt(p);
    atlas::editor::AssistantSuggestion s;
    s.title = "S";
    panel.AddSuggestion(s);

    panel.ClearConversation();
    assert(panel.GetContext().empty());
    assert(panel.GetDiffPreview().empty());
    assert(panel.Prompts().empty());
    assert(panel.Suggestions().empty());
    std::cout << "[PASS] test_assistant_panel_clear" << std::endl;
}
