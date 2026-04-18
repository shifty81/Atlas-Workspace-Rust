#pragma once
// NF::Editor — AtlasAITool: AtlasAI assistant, Codex, and diagnostics tool.
//
// Implements NF::IHostedTool for the AtlasAI Tool, one of the ~10
// primary tools in the canonical workspace roster.
//
// The AtlasAI Tool hosts:
//   Shared panels: ai_chat, ai_suggestions, console
//   Commands:      ai.query, ai.apply_suggestion, ai.clear_history,
//                  ai.toggle_inline, ai.promote_snippet
//
// See Docs/Canon/05_EDITOR_STRATEGY.md for the locked tool roster.
// See Docs/Roadmap/04_EDITOR_CONSOLIDATION.md for Phase 3 status.

#include "NF/Workspace/IHostedTool.h"
#include "NF/Workspace/ToolViewRenderContext.h"
#include <string>
#include <vector>

namespace NF {

// ── AtlasAI assist mode ───────────────────────────────────────────

enum class AIAssistMode : uint8_t {
    Chat,        // conversational assistant
    Suggestions, // inline code / asset suggestions list
    Codex,       // Codex browser: stored snippets and patterns
};

inline const char* aiAssistModeName(AIAssistMode m) {
    switch (m) {
        case AIAssistMode::Chat:        return "Chat";
        case AIAssistMode::Suggestions: return "Suggestions";
        case AIAssistMode::Codex:       return "Codex";
    }
    return "Unknown";
}

// ── AtlasAI Tool statistics ───────────────────────────────────────

struct AtlasAIToolStats {
    uint32_t messageCount           = 0; // messages in current session
    uint32_t pendingSuggestionCount = 0; // un-acted inline suggestions
    uint32_t codexSnippetCount      = 0; // promoted Codex entries
    bool     isProcessing           = false; // awaiting AI response
};

// ── AtlasAITool ──────────────────────────────────────────────────

class AtlasAITool final : public IHostedTool {
public:
    static constexpr const char* kToolId = "workspace.atlas_ai";

    AtlasAITool();
    ~AtlasAITool() override = default;

    // ── IHostedTool identity ──────────────────────────────────────
    [[nodiscard]] const HostedToolDescriptor& descriptor() const override { return m_descriptor; }
    [[nodiscard]] const std::string& toolId()              const override { return m_descriptor.toolId; }

    // ── IHostedTool lifecycle ─────────────────────────────────────
    bool initialize() override;
    void shutdown()   override;
    void activate()   override;
    void suspend()    override;
    void update(float dt) override;

    [[nodiscard]] HostedToolState state() const override { return m_state; }

    // ── Project adapter hooks ─────────────────────────────────────
    void onProjectLoaded(const std::string& projectId) override;
    void onProjectUnloaded() override;

    // ── AtlasAI Tool interface ────────────────────────────────────

    [[nodiscard]] AIAssistMode           assistMode()    const { return m_assistMode; }
    void                                 setAssistMode(AIAssistMode mode);

    [[nodiscard]] const AtlasAIToolStats& stats()          const { return m_stats; }
    [[nodiscard]] bool                    isProcessing()   const { return m_stats.isProcessing; }
    [[nodiscard]] uint32_t                messageCount()   const { return m_stats.messageCount; }
    [[nodiscard]] uint32_t                pendingSuggestionCount() const { return m_stats.pendingSuggestionCount; }
    [[nodiscard]] uint32_t                codexSnippetCount()      const { return m_stats.codexSnippetCount; }

    void setProcessing(bool processing);
    void setMessageCount(uint32_t count);
    void setPendingSuggestionCount(uint32_t count);
    void setCodexSnippetCount(uint32_t count);
    void clearSession();

    // ── Render contract ───────────────────────────────────────────
    // Renders: Session | Chat/Codex/Suggestions | Context — three-column layout.
    void renderToolView(const ToolViewRenderContext& ctx) const override;
    void syncPanels(WorkspacePanelHost& host) const override;

private:
    HostedToolDescriptor m_descriptor;
    HostedToolState      m_state      = HostedToolState::Unloaded;
    mutable AIAssistMode m_assistMode = AIAssistMode::Chat;
    AtlasAIToolStats     m_stats;
    std::string          m_activeProjectId;

    // ── Mutable per-view UI state (safe from const renderToolView) ─
    struct AIChatEntry { bool isUser; std::string text; };
    mutable std::vector<AIChatEntry> m_chatHistory;
    mutable std::string              m_inputBuffer;
    mutable bool                     m_inputFocused = false;

    void buildDescriptor();
};

} // namespace NF
