#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"

using namespace NF;

// ── ChatRole ─────────────────────────────────────────────────────

TEST_CASE("ChatRole names", "[Editor][S55]") {
    REQUIRE(std::string(chatRoleName(ChatRole::User)) == "User");
    REQUIRE(std::string(chatRoleName(ChatRole::Assistant)) == "Assistant");
    REQUIRE(std::string(chatRoleName(ChatRole::System)) == "System");
}

// ── ChatMessage ──────────────────────────────────────────────────

TEST_CASE("ChatMessage role checks", "[Editor][S55]") {
    ChatMessage msg;
    msg.role = ChatRole::User;
    REQUIRE(msg.isUser());
    REQUIRE_FALSE(msg.isAssistant());
    REQUIRE_FALSE(msg.isSystem());
}

TEST_CASE("ChatMessage assistant role", "[Editor][S55]") {
    ChatMessage msg;
    msg.role = ChatRole::Assistant;
    REQUIRE(msg.isAssistant());
}

TEST_CASE("ChatMessage system role", "[Editor][S55]") {
    ChatMessage msg;
    msg.role = ChatRole::System;
    REQUIRE(msg.isSystem());
}

// ── ChatSession ──────────────────────────────────────────────────

TEST_CASE("ChatSession starts empty", "[Editor][S55]") {
    ChatSession session;
    REQUIRE(session.empty());
    REQUIRE(session.messageCount() == 0);
    REQUIRE(session.id() == "default");
    REQUIRE(session.lastMessage() == nullptr);
}

TEST_CASE("ChatSession custom id", "[Editor][S55]") {
    ChatSession session("my-session");
    REQUIRE(session.id() == "my-session");
}

TEST_CASE("ChatSession addUserMessage", "[Editor][S55]") {
    ChatSession session;
    session.addUserMessage("Hello");
    REQUIRE(session.messageCount() == 1);
    REQUIRE(session.messages()[0].isUser());
    REQUIRE(session.messages()[0].content == "Hello");
}

TEST_CASE("ChatSession addAssistantMessage", "[Editor][S55]") {
    ChatSession session;
    session.addAssistantMessage("Hi there!");
    REQUIRE(session.messageCount() == 1);
    REQUIRE(session.messages()[0].isAssistant());
}

TEST_CASE("ChatSession addSystemMessage", "[Editor][S55]") {
    ChatSession session;
    session.addSystemMessage("Session started");
    REQUIRE(session.messageCount() == 1);
    REQUIRE(session.messages()[0].isSystem());
}

TEST_CASE("ChatSession conversation flow", "[Editor][S55]") {
    ChatSession session;
    session.addUserMessage("What is ECS?");
    session.addAssistantMessage("Entity Component System is a design pattern...");
    session.addUserMessage("Give an example");
    session.addAssistantMessage("For example, an entity has Position and Velocity...");
    REQUIRE(session.messageCount() == 4);
    REQUIRE(session.lastMessage()->isAssistant());
}

TEST_CASE("ChatSession lastMessage", "[Editor][S55]") {
    ChatSession session;
    session.addUserMessage("First");
    session.addAssistantMessage("Second");
    REQUIRE(session.lastMessage() != nullptr);
    REQUIRE(session.lastMessage()->content == "Second");
}

TEST_CASE("ChatSession clear", "[Editor][S55]") {
    ChatSession session;
    session.addUserMessage("Msg");
    session.clear();
    REQUIRE(session.empty());
    REQUIRE(session.messageCount() == 0);
}

TEST_CASE("ChatSession message ids are unique", "[Editor][S55]") {
    ChatSession session;
    session.addUserMessage("A");
    session.addUserMessage("B");
    REQUIRE(session.messages()[0].id != session.messages()[1].id);
}

// ── AIAssistantPanel ─────────────────────────────────────────────

TEST_CASE("AIAssistantPanel name and slot", "[Editor][S55]") {
    AIAssistantPanel panel;
    REQUIRE(panel.name() == "AI Assistant");
    REQUIRE(panel.slot() == DockSlot::Right);
}

TEST_CASE("AIAssistantPanel starts disconnected", "[Editor][S55]") {
    AIAssistantPanel panel;
    REQUIRE_FALSE(panel.isConnected());
    REQUIRE(panel.aiIntegration() == nullptr);
    REQUIRE(panel.queryCount() == 0);
    REQUIRE(panel.responseCount() == 0);
}

TEST_CASE("AIAssistantPanel setInputText", "[Editor][S55]") {
    AIAssistantPanel panel;
    panel.setInputText("Hello AI");
    REQUIRE(panel.inputText() == "Hello AI");
}

TEST_CASE("AIAssistantPanel submitInput creates user message", "[Editor][S55]") {
    AIAssistantPanel panel;
    panel.setInputText("Question?");
    panel.submitInput();
    REQUIRE(panel.session().messageCount() == 1);
    REQUIRE(panel.session().messages()[0].isUser());
    REQUIRE(panel.session().messages()[0].content == "Question?");
    REQUIRE(panel.inputText().empty()); // cleared after submit
    REQUIRE(panel.queryCount() == 1);
    REQUIRE(panel.lastQuery() == "Question?");
}

TEST_CASE("AIAssistantPanel submitInput empty does nothing", "[Editor][S55]") {
    AIAssistantPanel panel;
    panel.submitInput(); // empty input
    REQUIRE(panel.session().empty());
    REQUIRE(panel.queryCount() == 0);
}

TEST_CASE("AIAssistantPanel deliverResponse", "[Editor][S55]") {
    AIAssistantPanel panel;
    panel.setInputText("Q");
    panel.submitInput();
    panel.deliverResponse("A");
    REQUIRE(panel.session().messageCount() == 2);
    REQUIRE(panel.session().messages()[1].isAssistant());
    REQUIRE(panel.session().messages()[1].content == "A");
    REQUIRE(panel.responseCount() == 1);
}

TEST_CASE("AIAssistantPanel clearHistory", "[Editor][S55]") {
    AIAssistantPanel panel;
    panel.setInputText("Q");
    panel.submitInput();
    panel.deliverResponse("A");
    panel.clearHistory();
    REQUIRE(panel.session().empty());
    REQUIRE(panel.queryCount() == 0);
    REQUIRE(panel.responseCount() == 0);
}

TEST_CASE("AIAssistantPanel connect to integration", "[Editor][S55]") {
    AIAssistantPanel panel;
    AtlasAIIntegration ai;
    ai.init();
    panel.setAIIntegration(&ai);
    REQUIRE(panel.isConnected());
    REQUIRE(panel.aiIntegration() == &ai);
    ai.shutdown();
}

TEST_CASE("AIAssistantPanel multi-turn conversation", "[Editor][S55]") {
    AIAssistantPanel panel;
    panel.setInputText("What is the weather?");
    panel.submitInput();
    panel.deliverResponse("I don't have weather data.");
    panel.setInputText("Tell me a joke");
    panel.submitInput();
    panel.deliverResponse("Why did the chicken...");
    REQUIRE(panel.session().messageCount() == 4);
    REQUIRE(panel.queryCount() == 2);
    REQUIRE(panel.responseCount() == 2);
}
