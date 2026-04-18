#pragma once
// NF::Editor — AIRequestContext: AtlasAI request context and diff proposal model.
//
// Phase G.7 — AtlasAI Tool full tool wiring.
//
// AIRequestContext captures the state an AtlasAI request needs:
//   - Active document / object context (for grounding the AI response)
//   - Conversation history (messages)
//   - Diff proposals returned by the AI (add/remove/context lines per file)
//   - Apply / reject workflow
//   - Codex snippet library (promoted snippets)
//
// AtlasAITool owns one AIRequestContext per session.

#include "NF/Core/Core.h"
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

// ── Message role ───────────────────────────────────────────────────────────────

enum class AIMessageRole : uint8_t { User, Assistant, System };

inline const char* aiMessageRoleName(AIMessageRole r) {
    switch (r) {
    case AIMessageRole::User:      return "User";
    case AIMessageRole::Assistant: return "Assistant";
    case AIMessageRole::System:    return "System";
    }
    return "Unknown";
}

// ── AI message ─────────────────────────────────────────────────────────────────

struct AIMessage {
    uint32_t      id        = 0;
    AIMessageRole role      = AIMessageRole::User;
    std::string   content;
    std::string   timestamp; ///< ISO-8601 string (stub)
};

// ── Diff proposal ──────────────────────────────────────────────────────────────

enum class DiffLineAction : uint8_t { Context, Add, Remove };

inline const char* diffLineActionName(DiffLineAction a) {
    switch (a) {
    case DiffLineAction::Context: return "Context";
    case DiffLineAction::Add:     return "Add";
    case DiffLineAction::Remove:  return "Remove";
    }
    return "Unknown";
}

struct DiffProposalLine {
    DiffLineAction action  = DiffLineAction::Context;
    std::string    content;
};

enum class DiffProposalStatus : uint8_t { Pending, Applied, Rejected };

inline const char* diffProposalStatusName(DiffProposalStatus s) {
    switch (s) {
    case DiffProposalStatus::Pending:  return "Pending";
    case DiffProposalStatus::Applied:  return "Applied";
    case DiffProposalStatus::Rejected: return "Rejected";
    }
    return "Unknown";
}

struct DiffProposal {
    uint32_t                      id        = 0;
    std::string                   filename;
    DiffProposalStatus            status    = DiffProposalStatus::Pending;
    std::vector<DiffProposalLine> lines;

    [[nodiscard]] uint32_t addedLines() const {
        uint32_t n = 0;
        for (const auto& l : lines) if (l.action == DiffLineAction::Add) ++n;
        return n;
    }

    [[nodiscard]] uint32_t removedLines() const {
        uint32_t n = 0;
        for (const auto& l : lines) if (l.action == DiffLineAction::Remove) ++n;
        return n;
    }
};

// ── Codex snippet ──────────────────────────────────────────────────────────────

struct CodexSnippet {
    uint32_t    id = 0;
    std::string title;
    std::string language; ///< e.g. "cpp", "lua", "json"
    std::string content;
    std::string tags;     ///< comma-separated tags
};

// ── AIRequestContext ───────────────────────────────────────────────────────────

class AIRequestContext {
public:
    AIRequestContext() = default;

    // ── Context binding ────────────────────────────────────────────────────────

    void setActiveDocumentId(const std::string& id)  { m_activeDocumentId = id; }
    void setActiveObjectId(const std::string& id)    { m_activeObjectId   = id; }
    void addContextError(const std::string& msg)     { m_contextErrors.push_back(msg); }
    void clearContextErrors()                         { m_contextErrors.clear(); }
    void clearContext() {
        m_activeDocumentId.clear();
        m_activeObjectId.clear();
        m_contextErrors.clear();
    }

    [[nodiscard]] const std::string&              activeDocumentId() const { return m_activeDocumentId; }
    [[nodiscard]] const std::string&              activeObjectId()   const { return m_activeObjectId; }
    [[nodiscard]] const std::vector<std::string>& contextErrors()   const { return m_contextErrors; }
    [[nodiscard]] bool                            hasContext()       const {
        return !m_activeDocumentId.empty() || !m_activeObjectId.empty();
    }

    // ── Conversation ───────────────────────────────────────────────────────────

    using MessageCallback = std::function<void(const AIMessage&)>;
    void setMessageCallback(MessageCallback cb) { m_messageCallback = std::move(cb); }

    uint32_t submitUserMessage(const std::string& content) {
        AIMessage msg;
        msg.id      = ++m_nextMessageId;
        msg.role    = AIMessageRole::User;
        msg.content = content;
        m_messages.push_back(msg);
        if (m_messageCallback) m_messageCallback(msg);
        return msg.id;
    }

    uint32_t submitAssistantMessage(const std::string& content) {
        AIMessage msg;
        msg.id      = ++m_nextMessageId;
        msg.role    = AIMessageRole::Assistant;
        msg.content = content;
        m_messages.push_back(msg);
        if (m_messageCallback) m_messageCallback(msg);
        return msg.id;
    }

    [[nodiscard]] uint32_t messageCount() const {
        return static_cast<uint32_t>(m_messages.size());
    }

    [[nodiscard]] const std::vector<AIMessage>& messages() const { return m_messages; }

    [[nodiscard]] const AIMessage* findMessage(uint32_t id) const {
        for (const auto& m : m_messages) if (m.id == id) return &m;
        return nullptr;
    }

    void clearConversation() {
        m_messages.clear();
        m_nextMessageId = 0;
    }

    // ── Diff proposals ─────────────────────────────────────────────────────────

    using ProposalCallback = std::function<void(const DiffProposal&)>;
    void setProposalCallback(ProposalCallback cb) { m_proposalCallback = std::move(cb); }

    uint32_t addProposal(const std::string& filename,
                          std::vector<DiffProposalLine> lines) {
        DiffProposal prop;
        prop.id       = ++m_nextProposalId;
        prop.filename = filename;
        prop.lines    = std::move(lines);
        prop.status   = DiffProposalStatus::Pending;
        m_proposals.push_back(prop);
        if (m_proposalCallback) m_proposalCallback(m_proposals.back());
        return prop.id;
    }

    bool applyProposal(uint32_t id) {
        for (auto& p : m_proposals) {
            if (p.id == id && p.status == DiffProposalStatus::Pending) {
                p.status = DiffProposalStatus::Applied;
                return true;
            }
        }
        return false;
    }

    bool rejectProposal(uint32_t id) {
        for (auto& p : m_proposals) {
            if (p.id == id && p.status == DiffProposalStatus::Pending) {
                p.status = DiffProposalStatus::Rejected;
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] uint32_t proposalCount() const {
        return static_cast<uint32_t>(m_proposals.size());
    }

    [[nodiscard]] uint32_t pendingProposalCount() const {
        uint32_t n = 0;
        for (const auto& p : m_proposals) {
            if (p.status == DiffProposalStatus::Pending) ++n;
        }
        return n;
    }

    [[nodiscard]] const std::vector<DiffProposal>& proposals() const { return m_proposals; }

    [[nodiscard]] const DiffProposal* findProposal(uint32_t id) const {
        for (const auto& p : m_proposals) if (p.id == id) return &p;
        return nullptr;
    }

    // ── Codex snippets ─────────────────────────────────────────────────────────

    uint32_t addCodexSnippet(const std::string& title, const std::string& language,
                               const std::string& content, const std::string& tags = "") {
        CodexSnippet s;
        s.id       = ++m_nextSnippetId;
        s.title    = title;
        s.language = language;
        s.content  = content;
        s.tags     = tags;
        m_snippets.push_back(std::move(s));
        return s.id;
    }

    bool removeCodexSnippet(uint32_t id) {
        for (auto it = m_snippets.begin(); it != m_snippets.end(); ++it) {
            if (it->id == id) {
                m_snippets.erase(it);
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] uint32_t snippetCount() const {
        return static_cast<uint32_t>(m_snippets.size());
    }

    [[nodiscard]] const std::vector<CodexSnippet>& snippets() const { return m_snippets; }

    [[nodiscard]] const CodexSnippet* findSnippet(uint32_t id) const {
        for (const auto& s : m_snippets) if (s.id == id) return &s;
        return nullptr;
    }

private:
    std::string              m_activeDocumentId;
    std::string              m_activeObjectId;
    std::vector<std::string> m_contextErrors;

    uint32_t                m_nextMessageId  = 0u;
    std::vector<AIMessage>  m_messages;
    MessageCallback         m_messageCallback;

    uint32_t                  m_nextProposalId = 0u;
    std::vector<DiffProposal> m_proposals;
    ProposalCallback          m_proposalCallback;

    uint32_t                   m_nextSnippetId = 0u;
    std::vector<CodexSnippet>  m_snippets;
};

} // namespace NF
