// Tests/Workspace/test_phase36_ai_panel.cpp
// Phase 36 — AI Assistant Panel and AI Panel Session
//
// Tests for:
//   1. ChatRole              — enum name helpers
//   2. ChatMessage           — role helpers, fields
//   3. ChatSession           — add messages, lastMessage, clear, empty
//   4. AISessionContextType  — enum name helpers
//   5. AISessionContext      — validity
//   6. AIPanelSession        — context management, turn management, reset
//   7. Integration           — multi-turn conversation with context

#include <catch2/catch_test_macros.hpp>
// EditorTheme and DockSlot must be included before EditorPanel.h
// (which AIAssistantPanel.h pulls in transitively)
#include "NF/Workspace/SelectionService.h"
#include "NF/Editor/EditorTheme.h"
#include "NF/Workspace/AIPanelSession.h"
#include <string>

using namespace NF;

// Helper to make a valid context
static AISessionContext makeCtx(AISessionContextType type, const std::string& label,
                                const std::string& content, bool pinned = false) {
    AISessionContext c;
    c.type    = type;
    c.label   = label;
    c.content = content;
    c.pinned  = pinned;
    return c;
}

// ─────────────────────────────────────────────────────────────────
// 1. ChatRole
// ─────────────────────────────────────────────────────────────────
TEST_CASE("ChatRole name helpers correct", "[ChatRole]") {
    CHECK(std::string(chatRoleName(ChatRole::User))      == "User");
    CHECK(std::string(chatRoleName(ChatRole::Assistant)) == "Assistant");
    CHECK(std::string(chatRoleName(ChatRole::System))    == "System");
}

// ─────────────────────────────────────────────────────────────────
// 2. ChatMessage
// ─────────────────────────────────────────────────────────────────
TEST_CASE("ChatMessage default role is User", "[ChatMessage]") {
    ChatMessage m;
    CHECK(m.role == ChatRole::User);
    CHECK(m.isUser());
    CHECK_FALSE(m.isAssistant());
    CHECK_FALSE(m.isSystem());
}

TEST_CASE("ChatMessage isAssistant true when role is Assistant", "[ChatMessage]") {
    ChatMessage m;
    m.role = ChatRole::Assistant;
    CHECK(m.isAssistant());
    CHECK_FALSE(m.isUser());
}

TEST_CASE("ChatMessage isSystem true when role is System", "[ChatMessage]") {
    ChatMessage m;
    m.role = ChatRole::System;
    CHECK(m.isSystem());
}

TEST_CASE("ChatMessage pending flag defaults to false", "[ChatMessage]") {
    ChatMessage m;
    CHECK_FALSE(m.pending);
}

// ─────────────────────────────────────────────────────────────────
// 3. ChatSession
// ─────────────────────────────────────────────────────────────────
TEST_CASE("ChatSession default is empty", "[ChatSession]") {
    ChatSession cs;
    CHECK(cs.empty());
    CHECK(cs.messageCount() == 0u);
}

TEST_CASE("ChatSession addUserMessage adds with correct role", "[ChatSession]") {
    ChatSession cs;
    cs.addUserMessage("Hello");
    REQUIRE(cs.messageCount() == 1u);
    CHECK(cs.messages()[0].isUser());
    CHECK(cs.messages()[0].content == "Hello");
}

TEST_CASE("ChatSession addAssistantMessage adds with correct role", "[ChatSession]") {
    ChatSession cs;
    cs.addAssistantMessage("Hi there");
    REQUIRE(cs.messageCount() == 1u);
    CHECK(cs.messages()[0].isAssistant());
    CHECK(cs.messages()[0].content == "Hi there");
}

TEST_CASE("ChatSession addSystemMessage adds with correct role", "[ChatSession]") {
    ChatSession cs;
    cs.addSystemMessage("System initialized");
    REQUIRE(cs.messageCount() == 1u);
    CHECK(cs.messages()[0].isSystem());
}

TEST_CASE("ChatSession lastMessage returns most recent message", "[ChatSession]") {
    ChatSession cs;
    cs.addUserMessage("A");
    cs.addAssistantMessage("B");
    const ChatMessage* last = cs.lastMessage();
    REQUIRE(last != nullptr);
    CHECK(last->content == "B");
    CHECK(last->isAssistant());
}

TEST_CASE("ChatSession lastMessage returns nullptr when empty", "[ChatSession]") {
    ChatSession cs;
    CHECK(cs.lastMessage() == nullptr);
}

TEST_CASE("ChatSession id is preserved", "[ChatSession]") {
    ChatSession cs("my-session");
    CHECK(cs.id() == "my-session");
}

TEST_CASE("ChatSession clear removes all messages", "[ChatSession]") {
    ChatSession cs;
    cs.addUserMessage("Hello");
    cs.addAssistantMessage("World");
    cs.clear();
    CHECK(cs.empty());
    CHECK(cs.messageCount() == 0u);
}

TEST_CASE("ChatSession message ids are unique", "[ChatSession]") {
    ChatSession cs;
    cs.addUserMessage("A");
    cs.addUserMessage("B");
    REQUIRE(cs.messageCount() == 2u);
    CHECK(cs.messages()[0].id != cs.messages()[1].id);
}

// ─────────────────────────────────────────────────────────────────
// 4. AISessionContextType
// ─────────────────────────────────────────────────────────────────
TEST_CASE("AISessionContextType name helpers all values", "[AISessionContextType]") {
    CHECK(std::string(aiSessionContextTypeName(AISessionContextType::None))         == "None");
    CHECK(std::string(aiSessionContextTypeName(AISessionContextType::File))         == "File");
    CHECK(std::string(aiSessionContextTypeName(AISessionContextType::Selection))    == "Selection");
    CHECK(std::string(aiSessionContextTypeName(AISessionContextType::Error))        == "Error");
    CHECK(std::string(aiSessionContextTypeName(AISessionContextType::Notification)) == "Notification");
    CHECK(std::string(aiSessionContextTypeName(AISessionContextType::Diff))         == "Diff");
    CHECK(std::string(aiSessionContextTypeName(AISessionContextType::Log))          == "Log");
}

// ─────────────────────────────────────────────────────────────────
// 5. AISessionContext
// ─────────────────────────────────────────────────────────────────
TEST_CASE("AISessionContext default is invalid", "[AISessionContext]") {
    AISessionContext c;
    CHECK_FALSE(c.isValid()); // type=None
}

TEST_CASE("AISessionContext valid with type and content", "[AISessionContext]") {
    auto c = makeCtx(AISessionContextType::File, "main.cpp", "int main() {}");
    CHECK(c.isValid());
}

TEST_CASE("AISessionContext invalid without content", "[AISessionContext]") {
    AISessionContext c;
    c.type  = AISessionContextType::File;
    c.label = "empty.cpp";
    // content is empty
    CHECK_FALSE(c.isValid());
}

TEST_CASE("AISessionContext pinned flag defaults to false", "[AISessionContext]") {
    AISessionContext c;
    CHECK_FALSE(c.pinned);
}

// ─────────────────────────────────────────────────────────────────
// 6. AIPanelSession
// ─────────────────────────────────────────────────────────────────
TEST_CASE("AIPanelSession default is empty", "[AIPanelSession]") {
    AIPanelSession s;
    CHECK(s.isEmpty());
    CHECK(s.turnCount()    == 0u);
    CHECK(s.messageCount() == 0u);
    CHECK(s.contextCount() == 0u);
}

TEST_CASE("AIPanelSession sessionId is preserved", "[AIPanelSession]") {
    AIPanelSession s("session-42");
    CHECK(s.sessionId() == "session-42");
}

TEST_CASE("AIPanelSession submitUserTurn increments turnCount", "[AIPanelSession]") {
    AIPanelSession s;
    s.submitUserTurn("Hello AI");
    CHECK(s.turnCount() == 1u);
    CHECK(s.messageCount() == 1u);
}

TEST_CASE("AIPanelSession receiveAssistantResponse does not increment turnCount", "[AIPanelSession]") {
    AIPanelSession s;
    s.submitUserTurn("Hello");
    s.receiveAssistantResponse("Hi there");
    CHECK(s.turnCount()    == 1u); // only user turns count
    CHECK(s.messageCount() == 2u); // both messages exist
}

TEST_CASE("AIPanelSession addContext valid context", "[AIPanelSession]") {
    AIPanelSession s;
    auto ctx = makeCtx(AISessionContextType::Error, "build error", "undefined symbol");
    CHECK(s.addContext(ctx));
    CHECK(s.contextCount() == 1u);
}

TEST_CASE("AIPanelSession addContext invalid context rejected", "[AIPanelSession]") {
    AIPanelSession s;
    AISessionContext bad; // type=None
    CHECK_FALSE(s.addContext(bad));
    CHECK(s.contextCount() == 0u);
}

TEST_CASE("AIPanelSession removeContext removes by label", "[AIPanelSession]") {
    AIPanelSession s;
    s.addContext(makeCtx(AISessionContextType::File, "main.cpp", "content"));
    CHECK(s.removeContext("main.cpp"));
    CHECK(s.contextCount() == 0u);
}

TEST_CASE("AIPanelSession removeContext fails for unknown label", "[AIPanelSession]") {
    AIPanelSession s;
    CHECK_FALSE(s.removeContext("nope.cpp"));
}

TEST_CASE("AIPanelSession findContext returns correct context", "[AIPanelSession]") {
    AIPanelSession s;
    s.addContext(makeCtx(AISessionContextType::File, "scene.cpp", "// scene"));
    const AISessionContext* c = s.findContext("scene.cpp");
    REQUIRE(c != nullptr);
    CHECK(c->type == AISessionContextType::File);
}

TEST_CASE("AIPanelSession clearUnpinnedContexts removes only unpinned", "[AIPanelSession]") {
    AIPanelSession s;
    s.addContext(makeCtx(AISessionContextType::File,  "pinned.cpp",   "data", true));
    s.addContext(makeCtx(AISessionContextType::Error, "transient.txt","error", false));
    s.clearUnpinnedContexts();
    CHECK(s.contextCount() == 1u);
    CHECK(s.findContext("pinned.cpp") != nullptr);
    CHECK(s.findContext("transient.txt") == nullptr);
}

TEST_CASE("AIPanelSession setTitle and title accessors", "[AIPanelSession]") {
    AIPanelSession s;
    s.setTitle("Debugging session");
    CHECK(s.title() == "Debugging session");
}

TEST_CASE("AIPanelSession reset clears everything", "[AIPanelSession]") {
    AIPanelSession s;
    s.submitUserTurn("Hello");
    s.receiveAssistantResponse("Hi");
    s.addContext(makeCtx(AISessionContextType::Log, "log", "line 1"));
    s.reset();
    CHECK(s.isEmpty());
    CHECK(s.turnCount()    == 0u);
    CHECK(s.contextCount() == 0u);
}

// ─────────────────────────────────────────────────────────────────
// 7. Integration — multi-turn conversation with context
// ─────────────────────────────────────────────────────────────────
TEST_CASE("AIPanelSession integration: multi-turn conversation", "[AIPanelSessionIntegration]") {
    AIPanelSession s("dev-session");

    // Attach context (e.g. a build error)
    s.addContext(makeCtx(AISessionContextType::Error,
        "build_error", "error C2065: 'foo' undeclared identifier"));

    // Turn 1
    s.submitUserTurn("Why won't this compile?");
    s.receiveAssistantResponse("You have an undeclared identifier 'foo'.");
    CHECK(s.turnCount()    == 1u);
    CHECK(s.messageCount() == 2u);

    // Turn 2
    s.submitUserTurn("How do I fix it?");
    s.receiveAssistantResponse("Declare 'foo' before use or include the right header.");
    CHECK(s.turnCount()    == 2u);
    CHECK(s.messageCount() == 4u);

    // Context still present
    CHECK(s.contextCount() == 1u);
}

TEST_CASE("AIPanelSession integration: pinned context survives clearUnpinned", "[AIPanelSessionIntegration]") {
    AIPanelSession s;
    s.addContext(makeCtx(AISessionContextType::File,      "fixed.h",     "// header", true));
    s.addContext(makeCtx(AISessionContextType::Selection, "snippet",     "int x = 0;", false));
    s.addContext(makeCtx(AISessionContextType::Diff,      "recent_diff", "--- a\n+++ b", false));

    s.clearUnpinnedContexts();

    CHECK(s.contextCount() == 1u);
    CHECK(s.findContext("fixed.h") != nullptr);
    CHECK(s.findContext("snippet") == nullptr);
}

TEST_CASE("AIPanelSession integration: reset and restart fresh session", "[AIPanelSessionIntegration]") {
    AIPanelSession s("proj-session");

    s.submitUserTurn("First question");
    s.receiveAssistantResponse("First answer");
    s.addContext(makeCtx(AISessionContextType::Log, "run.log", "crash at line 42"));
    REQUIRE(s.turnCount() == 1u);

    s.reset();
    CHECK(s.isEmpty());
    CHECK(s.turnCount()    == 0u);
    CHECK(s.contextCount() == 0u);

    // Session can be reused after reset
    s.submitUserTurn("New question");
    CHECK(s.turnCount()    == 1u);
    CHECK(s.messageCount() == 1u);
}
