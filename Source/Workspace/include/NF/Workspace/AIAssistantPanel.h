#pragma once
// NF::Editor — AtlasAI assistant panel host
#include "NF/Workspace/AIIntegration.h"
#include "NF/Editor/EditorPanel.h"
#include <functional>
#include <string>
#include <vector>

namespace NF {

// ── Chat Message ─────────────────────────────────────────────────
// Represents a single message in the AI assistant conversation.

enum class ChatRole : uint8_t { User, Assistant, System };

inline const char* chatRoleName(ChatRole r) {
    switch (r) {
        case ChatRole::User:      return "User";
        case ChatRole::Assistant: return "Assistant";
        case ChatRole::System:    return "System";
        default:                  return "Unknown";
    }
}

struct ChatMessage {
    std::string id;
    ChatRole    role = ChatRole::User;
    std::string content;
    int64_t     timestamp = 0;
    bool        pending = false;

    [[nodiscard]] bool isUser()      const { return role == ChatRole::User; }
    [[nodiscard]] bool isAssistant() const { return role == ChatRole::Assistant; }
    [[nodiscard]] bool isSystem()    const { return role == ChatRole::System; }
};

// ── Chat Session ─────────────────────────────────────────────────
// Manages a conversation thread with the AI assistant.

class ChatSession {
public:
    explicit ChatSession(const std::string& id = "default")
        : m_id(id) {}

    void addMessage(ChatMessage msg) {
        m_messages.push_back(std::move(msg));
    }

    void addUserMessage(const std::string& content) {
        ChatMessage msg;
        msg.id = "msg_" + std::to_string(m_nextMsgId++);
        msg.role = ChatRole::User;
        msg.content = content;
        msg.timestamp = static_cast<int64_t>(m_nextMsgId);
        m_messages.push_back(std::move(msg));
    }

    void addAssistantMessage(const std::string& content) {
        ChatMessage msg;
        msg.id = "msg_" + std::to_string(m_nextMsgId++);
        msg.role = ChatRole::Assistant;
        msg.content = content;
        msg.timestamp = static_cast<int64_t>(m_nextMsgId);
        m_messages.push_back(std::move(msg));
    }

    void addSystemMessage(const std::string& content) {
        ChatMessage msg;
        msg.id = "msg_" + std::to_string(m_nextMsgId++);
        msg.role = ChatRole::System;
        msg.content = content;
        msg.timestamp = static_cast<int64_t>(m_nextMsgId);
        m_messages.push_back(std::move(msg));
    }

    [[nodiscard]] const std::string& id() const { return m_id; }
    [[nodiscard]] size_t messageCount() const { return m_messages.size(); }
    [[nodiscard]] const std::vector<ChatMessage>& messages() const { return m_messages; }
    [[nodiscard]] bool empty() const { return m_messages.empty(); }

    [[nodiscard]] const ChatMessage* lastMessage() const {
        return m_messages.empty() ? nullptr : &m_messages.back();
    }

    void clear() { m_messages.clear(); m_nextMsgId = 1; }

private:
    std::string m_id;
    std::vector<ChatMessage> m_messages;
    size_t m_nextMsgId = 1;
};

// ── AI Assistant Panel ───────────────────────────────────────────
// The editor panel hosting the AtlasAI assistant.

class AIAssistantPanel : public EditorPanel {
public:
    [[nodiscard]] const std::string& name() const override { return m_name; }
    [[nodiscard]] DockSlot slot() const override { return DockSlot::Right; }

    void update(float /*dt*/) override {}

    void render(UIRenderer& ui, const Rect& bounds, const EditorTheme& theme) override {
        ui.drawRect(bounds, theme.panelBackground);
        ui.drawText(bounds.x + 8.f, bounds.y + 8.f, "Atlas AI", theme.selectionBorder);

        float y = bounds.y + 32.f;
        for (const auto& msg : m_session.messages()) {
            const char* prefix = chatRoleName(msg.role);
            std::string line = std::string(prefix) + ": " + msg.content;
            uint32_t color = msg.isUser() ? theme.panelText
                           : msg.isAssistant() ? theme.selectionBorder
                           : theme.dirtyIndicator;
            ui.drawText(bounds.x + 8.f, y, line.c_str(), color);
            y += 20.f;
        }

        // Input area at bottom
        float inputY = bounds.y + bounds.h - 28.f;
        Rect inputRect = {bounds.x + 4.f, inputY, bounds.w - 8.f, 24.f};
        ui.drawRect(inputRect, theme.inputBackground);
        if (!m_inputText.empty()) {
            ui.drawText(inputRect.x + 4.f, inputRect.y + 4.f,
                       m_inputText.c_str(), theme.inputText);
        }
    }

    void renderUI(UIContext& ctx, const Rect& bounds) override {
        (void)ctx; (void)bounds;
    }

    // Submit the current input text as a user message
    void submitInput() {
        if (m_inputText.empty()) return;
        m_session.addUserMessage(m_inputText);
        m_lastQuery = m_inputText;
        m_inputText.clear();
        ++m_queryCount;
    }

    // Deliver an AI response (called by integration layer)
    void deliverResponse(const std::string& content) {
        m_session.addAssistantMessage(content);
        ++m_responseCount;
    }

    void setInputText(const std::string& text) { m_inputText = text; }
    [[nodiscard]] const std::string& inputText() const { return m_inputText; }

    [[nodiscard]] ChatSession& session() { return m_session; }
    [[nodiscard]] const ChatSession& session() const { return m_session; }

    [[nodiscard]] size_t queryCount() const { return m_queryCount; }
    [[nodiscard]] size_t responseCount() const { return m_responseCount; }
    [[nodiscard]] const std::string& lastQuery() const { return m_lastQuery; }

    void clearHistory() {
        m_session.clear();
        m_queryCount = 0;
        m_responseCount = 0;
    }

    // Connect to AtlasAI integration
    void setAIIntegration(AtlasAIIntegration* ai) { m_ai = ai; }
    [[nodiscard]] AtlasAIIntegration* aiIntegration() const { return m_ai; }
    [[nodiscard]] bool isConnected() const { return m_ai != nullptr && m_ai->isInitialized(); }

private:
    std::string m_name = "AI Assistant";
    ChatSession m_session;
    std::string m_inputText;
    std::string m_lastQuery;
    size_t m_queryCount = 0;
    size_t m_responseCount = 0;
    AtlasAIIntegration* m_ai = nullptr;
};


} // namespace NF
