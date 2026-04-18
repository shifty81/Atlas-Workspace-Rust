#pragma once
// NF::Editor — AI panel session: persistent session state with context history
#include "NF/Workspace/AIAssistantPanel.h"
#include <cstdint>
#include <string>
#include <vector>

namespace NF {

// ── Session Context Type ──────────────────────────────────────────

enum class AISessionContextType : uint8_t {
    None,
    File,          // a source file is attached as context
    Selection,     // selected text/code
    Error,         // build/runtime error
    Notification,  // workspace notification
    Diff,          // a code diff
    Log,           // log excerpt
};

inline const char* aiSessionContextTypeName(AISessionContextType t) {
    switch (t) {
        case AISessionContextType::None:         return "None";
        case AISessionContextType::File:         return "File";
        case AISessionContextType::Selection:    return "Selection";
        case AISessionContextType::Error:        return "Error";
        case AISessionContextType::Notification: return "Notification";
        case AISessionContextType::Diff:         return "Diff";
        case AISessionContextType::Log:          return "Log";
    }
    return "Unknown";
}

// ── AI Session Context ────────────────────────────────────────────

struct AISessionContext {
    AISessionContextType type  = AISessionContextType::None;
    std::string          label;       // human-readable label (filename, etc.)
    std::string          content;     // the actual context text/content
    bool                 pinned = false;  // stays across turns

    [[nodiscard]] bool isValid() const {
        return type != AISessionContextType::None && !content.empty();
    }
};

// ── AI Panel Session ──────────────────────────────────────────────
// Wraps ChatSession with richer context management and session lifecycle.

class AIPanelSession {
public:
    explicit AIPanelSession(const std::string& sessionId = "default")
        : m_sessionId(sessionId), m_chatSession(sessionId) {}

    void reset() {
        m_chatSession.clear();
        m_contexts.clear();
        m_turnCount = 0;
    }

    // Context management
    bool addContext(const AISessionContext& ctx) {
        if (!ctx.isValid()) return false;
        m_contexts.push_back(ctx);
        return true;
    }

    bool removeContext(const std::string& label) {
        for (auto it = m_contexts.begin(); it != m_contexts.end(); ++it) {
            if (it->label == label) { m_contexts.erase(it); return true; }
        }
        return false;
    }

    void clearUnpinnedContexts() {
        m_contexts.erase(std::remove_if(m_contexts.begin(), m_contexts.end(),
            [](const AISessionContext& c) { return !c.pinned; }), m_contexts.end());
    }

    [[nodiscard]] const std::vector<AISessionContext>& contexts() const { return m_contexts; }
    [[nodiscard]] size_t contextCount() const { return m_contexts.size(); }

    [[nodiscard]] const AISessionContext* findContext(const std::string& label) const {
        for (const auto& c : m_contexts) if (c.label == label) return &c;
        return nullptr;
    }

    // Turn management
    void submitUserTurn(const std::string& message) {
        m_chatSession.addUserMessage(message);
        ++m_turnCount;
    }

    // Receive an assistant response for the current turn (does not increment turnCount)
    void receiveAssistantResponse(const std::string& message) {
        m_chatSession.addAssistantMessage(message);
    }

    [[nodiscard]] ChatSession& chatSession() { return m_chatSession; }
    [[nodiscard]] const ChatSession& chatSession() const { return m_chatSession; }

    [[nodiscard]] const std::string& sessionId()  const { return m_sessionId;  }
    [[nodiscard]] size_t             turnCount()  const { return m_turnCount;  }
    [[nodiscard]] size_t             messageCount() const { return m_chatSession.messageCount(); }
    [[nodiscard]] bool               isEmpty()    const { return m_chatSession.empty(); }

    void setTitle(const std::string& title) { m_title = title; }
    [[nodiscard]] const std::string& title() const { return m_title; }

private:
    std::string  m_sessionId;
    std::string  m_title;
    ChatSession  m_chatSession;
    std::vector<AISessionContext> m_contexts;
    size_t       m_turnCount = 0;
};

} // namespace NF
