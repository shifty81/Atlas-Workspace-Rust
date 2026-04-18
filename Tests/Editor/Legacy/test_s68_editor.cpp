#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"

using namespace NF;

// ── S68: AIAssistantPanel ────────────────────────────────────────

TEST_CASE("ChatRole enum values exist", "[Editor][S68]") {
    REQUIRE(static_cast<int>(ChatRole::User)      == 0);
    REQUIRE(static_cast<int>(ChatRole::Assistant) == 1);
    REQUIRE(static_cast<int>(ChatRole::System)    == 2);
}

TEST_CASE("chatRoleName returns correct strings", "[Editor][S68]") {
    REQUIRE(std::string(chatRoleName(ChatRole::User))      == "User");
    REQUIRE(std::string(chatRoleName(ChatRole::Assistant)) == "Assistant");
    REQUIRE(std::string(chatRoleName(ChatRole::System))    == "System");
}

TEST_CASE("ChatMessage defaults to User role", "[Editor][S68]") {
    ChatMessage msg;
    REQUIRE(msg.role == ChatRole::User);
}

TEST_CASE("ChatMessage isUser true for User role", "[Editor][S68]") {
    ChatMessage msg;
    msg.role = ChatRole::User;
    REQUIRE(msg.isUser());
    REQUIRE_FALSE(msg.isAssistant());
    REQUIRE_FALSE(msg.isSystem());
}

TEST_CASE("ChatMessage isAssistant true for Assistant role", "[Editor][S68]") {
    ChatMessage msg;
    msg.role = ChatRole::Assistant;
    REQUIRE(msg.isAssistant());
    REQUIRE_FALSE(msg.isUser());
}

TEST_CASE("ChatMessage isSystem true for System role", "[Editor][S68]") {
    ChatMessage msg;
    msg.role = ChatRole::System;
    REQUIRE(msg.isSystem());
    REQUIRE_FALSE(msg.isUser());
}

TEST_CASE("ChatMessage default pending is false", "[Editor][S68]") {
    ChatMessage msg;
    REQUIRE_FALSE(msg.pending);
}

TEST_CASE("ChatSession starts empty", "[Editor][S68]") {
    ChatSession session("sess1");
    REQUIRE(session.empty());
    REQUIRE(session.messageCount() == 0);
    REQUIRE(session.id() == "sess1");
}

TEST_CASE("ChatSession addMessage increases count", "[Editor][S68]") {
    ChatSession session;
    ChatMessage msg;
    msg.id = "m1";
    msg.role = ChatRole::User;
    msg.content = "Hello";
    session.addMessage(msg);
    REQUIRE(session.messageCount() == 1);
    REQUIRE_FALSE(session.empty());
}

TEST_CASE("ChatSession addUserMessage creates User message", "[Editor][S68]") {
    ChatSession session;
    session.addUserMessage("test input");
    REQUIRE(session.messageCount() == 1);
    const auto* last = session.lastMessage();
    REQUIRE(last != nullptr);
    REQUIRE(last->role == ChatRole::User);
    REQUIRE(last->content == "test input");
}

TEST_CASE("ChatSession addAssistantMessage creates Assistant message", "[Editor][S68]") {
    ChatSession session;
    session.addAssistantMessage("Sure, here is your answer.");
    REQUIRE(session.messageCount() == 1);
    const auto* last = session.lastMessage();
    REQUIRE(last != nullptr);
    REQUIRE(last->role == ChatRole::Assistant);
}

TEST_CASE("ChatSession addSystemMessage creates System message", "[Editor][S68]") {
    ChatSession session;
    session.addSystemMessage("You are a helpful assistant.");
    const auto* last = session.lastMessage();
    REQUIRE(last != nullptr);
    REQUIRE(last->role == ChatRole::System);
}

TEST_CASE("ChatSession lastMessage returns nullptr when empty", "[Editor][S68]") {
    ChatSession session;
    REQUIRE(session.lastMessage() == nullptr);
}

TEST_CASE("ChatSession clear resets messages", "[Editor][S68]") {
    ChatSession session;
    session.addUserMessage("hi");
    session.addAssistantMessage("hello");
    session.clear();
    REQUIRE(session.empty());
    REQUIRE(session.messageCount() == 0);
}

TEST_CASE("ChatSession messages are ordered by insertion", "[Editor][S68]") {
    ChatSession session;
    session.addUserMessage("first");
    session.addAssistantMessage("second");
    session.addUserMessage("third");
    REQUIRE(session.messageCount() == 3);
    REQUIRE(session.messages()[0].content == "first");
    REQUIRE(session.messages()[2].content == "third");
}

TEST_CASE("AIAssistantPanel name is AI Assistant", "[Editor][S68]") {
    AIAssistantPanel panel;
    REQUIRE(panel.name() == "AI Assistant");
}

TEST_CASE("AIAssistantPanel slot is Right", "[Editor][S68]") {
    AIAssistantPanel panel;
    REQUIRE(panel.slot() == DockSlot::Right);
}

TEST_CASE("AIAssistantPanel session starts empty", "[Editor][S68]") {
    AIAssistantPanel panel;
    REQUIRE(panel.session().empty());
}

TEST_CASE("AIAssistantPanel queryCount starts at zero", "[Editor][S68]") {
    AIAssistantPanel panel;
    REQUIRE(panel.queryCount() == 0);
}

TEST_CASE("AIAssistantPanel responseCount starts at zero", "[Editor][S68]") {
    AIAssistantPanel panel;
    REQUIRE(panel.responseCount() == 0);
}

TEST_CASE("AIAssistantPanel inputText starts empty", "[Editor][S68]") {
    AIAssistantPanel panel;
    REQUIRE(panel.inputText().empty());
}

TEST_CASE("AIAssistantPanel setInputText stores value", "[Editor][S68]") {
    AIAssistantPanel panel;
    panel.setInputText("help me");
    REQUIRE(panel.inputText() == "help me");
}

TEST_CASE("AIAssistantPanel submitInput clears inputText", "[Editor][S68]") {
    AIAssistantPanel panel;
    panel.setInputText("what is ECS?");
    panel.submitInput();
    REQUIRE(panel.inputText().empty());
}

TEST_CASE("AIAssistantPanel submitInput increments queryCount", "[Editor][S68]") {
    AIAssistantPanel panel;
    panel.setInputText("query");
    panel.submitInput();
    REQUIRE(panel.queryCount() == 1);
}

TEST_CASE("AIAssistantPanel submitInput adds user message to session", "[Editor][S68]") {
    AIAssistantPanel panel;
    panel.setInputText("tell me about ECS");
    panel.submitInput();
    REQUIRE(panel.session().messageCount() == 1);
    REQUIRE(panel.session().lastMessage()->role == ChatRole::User);
}

TEST_CASE("AIAssistantPanel submitInput stores lastQuery", "[Editor][S68]") {
    AIAssistantPanel panel;
    panel.setInputText("what is ECS?");
    panel.submitInput();
    REQUIRE(panel.lastQuery() == "what is ECS?");
}

TEST_CASE("AIAssistantPanel submitInput with empty input is no-op", "[Editor][S68]") {
    AIAssistantPanel panel;
    panel.submitInput();
    REQUIRE(panel.queryCount() == 0);
    REQUIRE(panel.session().empty());
}

TEST_CASE("AIAssistantPanel deliverResponse adds assistant message", "[Editor][S68]") {
    AIAssistantPanel panel;
    panel.deliverResponse("Here is the answer.");
    REQUIRE(panel.session().messageCount() == 1);
    REQUIRE(panel.session().lastMessage()->role == ChatRole::Assistant);
}

TEST_CASE("AIAssistantPanel deliverResponse increments responseCount", "[Editor][S68]") {
    AIAssistantPanel panel;
    panel.deliverResponse("answer");
    REQUIRE(panel.responseCount() == 1);
}

TEST_CASE("AIAssistantPanel clearHistory resets all", "[Editor][S68]") {
    AIAssistantPanel panel;
    panel.setInputText("q");
    panel.submitInput();
    panel.deliverResponse("a");
    panel.clearHistory();
    REQUIRE(panel.session().empty());
    REQUIRE(panel.queryCount() == 0);
    REQUIRE(panel.responseCount() == 0);
}

TEST_CASE("AIAssistantPanel aiIntegration starts null", "[Editor][S68]") {
    AIAssistantPanel panel;
    REQUIRE(panel.aiIntegration() == nullptr);
}

TEST_CASE("AIAssistantPanel isConnected false when no integration", "[Editor][S68]") {
    AIAssistantPanel panel;
    REQUIRE_FALSE(panel.isConnected());
}

TEST_CASE("AIAssistantPanel setAIIntegration stores pointer", "[Editor][S68]") {
    AIAssistantPanel panel;
    AtlasAIIntegration ai;
    panel.setAIIntegration(&ai);
    REQUIRE(panel.aiIntegration() == &ai);
}

TEST_CASE("AIAssistantPanel is an EditorPanel", "[Editor][S68]") {
    AIAssistantPanel panel;
    EditorPanel* base = &panel;
    REQUIRE(base != nullptr);
}
