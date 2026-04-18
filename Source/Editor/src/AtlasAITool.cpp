// NF::Editor — AtlasAITool implementation.
//
// Eighth real NF::IHostedTool from Phase 3 consolidation.
// Manages the AtlasAI assistant, inline suggestions, and Codex browser
// within AtlasWorkspace.

#include "NF/Editor/AtlasAITool.h"
#include "NF/Workspace/ToolViewRenderContext.h"
#include "NF/Workspace/WorkspacePanelHost.h"
#include "NF/Workspace/WorkspaceShell.h"
#include <cctype>
#include <cstdio>
#include <cstring>
#include <string>

namespace NF {

AtlasAITool::AtlasAITool() {
    buildDescriptor();
}

void AtlasAITool::buildDescriptor() {
    m_descriptor.toolId      = kToolId;
    m_descriptor.displayName = "AtlasAI";
    m_descriptor.category    = HostedToolCategory::AIAssistant;
    m_descriptor.isPrimary   = true;
    m_descriptor.acceptsProjectExtensions = false;

    m_descriptor.supportedPanels = {
        "panel.ai_chat",
        "panel.ai_suggestions",
        "panel.console",
    };

    m_descriptor.commands = {
        "ai.query",
        "ai.apply_suggestion",
        "ai.clear_history",
        "ai.toggle_inline",
        "ai.promote_snippet",
    };
}

bool AtlasAITool::initialize() {
    if (m_state != HostedToolState::Unloaded) return false;
    m_assistMode = AIAssistMode::Chat;
    m_stats      = {};
    m_state      = HostedToolState::Ready;
    return true;
}

void AtlasAITool::shutdown() {
    m_state           = HostedToolState::Unloaded;
    m_assistMode      = AIAssistMode::Chat;
    m_stats           = {};
    m_activeProjectId = {};
}

void AtlasAITool::activate() {
    if (m_state == HostedToolState::Ready || m_state == HostedToolState::Suspended) {
        m_state = HostedToolState::Active;
    }
}

void AtlasAITool::suspend() {
    if (m_state == HostedToolState::Active) {
        m_stats.isProcessing = false;
        m_state = HostedToolState::Suspended;
    }
}

void AtlasAITool::update(float /*dt*/) {
    // AI responses are driven by async broker callbacks, not per-frame polling.
}

void AtlasAITool::onProjectLoaded(const std::string& projectId) {
    m_activeProjectId = projectId;
    clearSession();
}

void AtlasAITool::onProjectUnloaded() {
    m_activeProjectId = {};
    clearSession();
}

void AtlasAITool::setAssistMode(AIAssistMode mode) {
    m_assistMode = mode;
}

void AtlasAITool::setProcessing(bool processing) {
    m_stats.isProcessing = processing;
}

void AtlasAITool::setMessageCount(uint32_t count) {
    m_stats.messageCount = count;
}

void AtlasAITool::setPendingSuggestionCount(uint32_t count) {
    m_stats.pendingSuggestionCount = count;
}

void AtlasAITool::setCodexSnippetCount(uint32_t count) {
    m_stats.codexSnippetCount = count;
}

void AtlasAITool::clearSession() {
    m_stats = {};
}

void AtlasAITool::renderToolView(const ToolViewRenderContext& ctx) const {
    // Three-column layout: Session (20%) | Chat/Codex/Suggestions (60%) | Context (20%)
    const float sessionW = ctx.w * 0.20f;
    const float mainW    = ctx.w * 0.60f;
    const float ctxW     = ctx.w - sessionW - mainW;

    static constexpr float kApproxCharWidth = 7.f;

    // ── Session panel ─────────────────────────────────────────────
    ctx.drawPanel(ctx.x, ctx.y, sessionW, ctx.h, "Session");
    {
        uint32_t totalMsgs = m_stats.messageCount
                           + static_cast<uint32_t>(m_chatHistory.size());
        char msgBuf[32];
        std::snprintf(msgBuf, sizeof(msgBuf), "%u messages", totalMsgs);
        ctx.ui.drawText(ctx.x + 8.f, ctx.y + 30.f, msgBuf, ctx.kTextSecond);
        char snippetBuf[32];
        std::snprintf(snippetBuf, sizeof(snippetBuf), "%u snippets", m_stats.codexSnippetCount);
        ctx.ui.drawText(ctx.x + 8.f, ctx.y + 48.f, snippetBuf, ctx.kTextSecond);
        if (m_stats.pendingSuggestionCount > 0) {
            char pendBuf[32];
            std::snprintf(pendBuf, sizeof(pendBuf), "%u pending", m_stats.pendingSuggestionCount);
            ctx.drawStatusPill(ctx.x + 8.f, ctx.y + 70.f, pendBuf, ctx.kAccentBlue);
        }

        // Clear session button
        if (ctx.drawButton(ctx.x + 8.f, ctx.y + ctx.h - 30.f, sessionW - 16.f, 20.f,
                           "Clear Session")) {
            m_chatHistory.clear();
            m_inputBuffer.clear();
            m_inputFocused = false;
        }
    }

    // ── Main panel — Chat / Codex / Suggestions ───────────────────
    ctx.drawPanel(ctx.x + sessionW, ctx.y, mainW, ctx.h, aiAssistModeName(m_assistMode));

    // Mode switch tabs (Chat / Codex / Suggestions)
    static constexpr struct { const char* label; AIAssistMode mode; } kModes[] = {
        {"Chat",        AIAssistMode::Chat},
        {"Codex",       AIAssistMode::Codex},
        {"Suggestions", AIAssistMode::Suggestions},
    };
    {
        float tx = ctx.x + sessionW + 8.f;
        float ty = ctx.y + 28.f;
        for (const auto& m : kModes) {
            float tw = static_cast<float>(std::strlen(m.label)) * 6.f + 16.f;
            bool active = (m_assistMode == m.mode);
            uint32_t bg = active ? ctx.kAccentBlue : ctx.kButtonBg;
            if (ctx.drawButton(tx, ty, tw, 16.f, m.label, bg))
                m_assistMode = m.mode;
            tx += tw + 4.f;
        }
    }

    if (m_stats.isProcessing)
        ctx.drawStatusPill(ctx.x + sessionW + mainW - 110.f, ctx.y + 30.f,
                           "Processing...", ctx.kGreen);

    // Chat input box at bottom of main panel
    Rect inputBoxR = {ctx.x + sessionW + 4.f, ctx.y + ctx.h - 30.f, mainW - 8.f - 50.f, 24.f};
    Rect sendBtnR  = {ctx.x + sessionW + mainW - 50.f, ctx.y + ctx.h - 30.f, 42.f, 24.f};

    // Click-to-focus
    if (ctx.mouse.leftPressed)
        m_inputFocused = inputBoxR.contains(ctx.mouse.x, ctx.mouse.y);

    // Process typed text when focused — copy first for safe iteration
    if (m_inputFocused && !ctx.mouse.typedText.empty()) {
        const std::string typedSnapshot = ctx.mouse.typedText;
        for (char c : typedSnapshot) {
            if (c == '\b') {
                if (!m_inputBuffer.empty()) m_inputBuffer.pop_back();
            } else if (c == '\r' || c == '\n') {
                if (!m_inputBuffer.empty()) {
                    std::string userMsg = m_inputBuffer;
                    m_chatHistory.push_back({true, userMsg});
                    // Stub AI response
                    std::string resp = "Understood. Analyzing workspace context...";
                    auto lower = userMsg;
                    for (char& ch : lower)
                        ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
                    if (lower.find("hello") != std::string::npos ||
                        lower.find("hi") != std::string::npos)
                        resp = "Hello! I'm AtlasAI. How can I assist with your project?";
                    else if (lower.find("scene") != std::string::npos)
                        resp = "Scene Editor: use Hierarchy to select entities and Inspector to edit properties.";
                    else if (lower.find("build") != std::string::npos)
                        resp = "Build Tool: click 'Build Now' in the Build Config panel.";
                    else if (lower.find("asset") != std::string::npos)
                        resp = "Asset Editor: filter by type using the buttons, then click a tile.";
                    else if (lower.find("help") != std::string::npos)
                        resp = "Available tools: Scene, Asset, Material, Animation, Data, Logic, Build, IDE. Ask me anything!";
                    m_chatHistory.push_back({false, resp});
                    m_inputBuffer.clear();
                }
            } else {
                if (m_inputBuffer.size() < 256) m_inputBuffer += c;
            }
        }
    }

    // Render chat messages
    float msgAreaTop = ctx.y + 50.f;
    float msgAreaBot = inputBoxR.y - 4.f;
    float msgH = msgAreaBot - msgAreaTop;
    {
        // Draw from newest messages that fit
        int startIdx = 0;
        {
            float spaceLeft = msgH;
            for (int i = static_cast<int>(m_chatHistory.size()) - 1; i >= 0; --i) {
                spaceLeft -= 32.f;
                if (spaceLeft < 0.f) { startIdx = i + 1; break; }
            }
        }
        if (m_chatHistory.empty()) {
            const char* hint = "Ask AtlasAI anything...";
            float hx = ctx.x + sessionW + (mainW - static_cast<float>(std::strlen(hint)) * kApproxCharWidth) * 0.5f;
            float hy = msgAreaTop + (msgH - 14.f) * 0.5f;
            ctx.ui.drawText(hx, hy, hint, ctx.kTextMuted);
        } else {
            float cy2 = msgAreaTop;
            for (int i = startIdx; i < static_cast<int>(m_chatHistory.size()); ++i) {
                if (cy2 + 26.f > msgAreaBot) break;
                const auto& entry = m_chatHistory[static_cast<size_t>(i)];
                uint32_t bg = entry.isUser ? 0x1E2A4AFF : 0x252535FF;
                std::string line = (entry.isUser ? "You: " : "AI: ") + entry.text;
                size_t maxChars = static_cast<size_t>((mainW - 20.f) / kApproxCharWidth);
                if (line.size() > maxChars) line = line.substr(0, maxChars - 2) + "..";
                ctx.ui.drawRect({ctx.x + sessionW + 6.f, cy2, mainW - 12.f, 26.f}, bg);
                ctx.ui.drawText(ctx.x + sessionW + 12.f, cy2 + 6.f, line,
                                entry.isUser ? 0x7ABAFFFF : ctx.kTextSecond);
                cy2 += 30.f;
            }
        }
    }

    // Input box + Send button
    uint32_t inputBorder = m_inputFocused ? ctx.kAccentBlue : ctx.kBorder;
    ctx.ui.drawRect(inputBoxR, m_inputFocused ? 0x1E1E3AFF : 0x2A2A3AFF);
    ctx.ui.drawRectOutline(inputBoxR, inputBorder, 1.f);
    if (m_inputBuffer.empty() && !m_inputFocused) {
        ctx.ui.drawText(inputBoxR.x + 6.f, inputBoxR.y + 5.f,
                        "Type a message...", ctx.kTextMuted);
    } else {
        std::string display = m_inputBuffer + (m_inputFocused ? "|" : "");
        size_t maxChars = static_cast<size_t>((mainW - 70.f) / kApproxCharWidth);
        if (display.size() > maxChars) display = display.substr(display.size() - maxChars);
        ctx.ui.drawText(inputBoxR.x + 6.f, inputBoxR.y + 5.f, display, ctx.kTextPrimary);
    }
    if (ctx.drawButton(sendBtnR.x, sendBtnR.y, sendBtnR.w, sendBtnR.h, "Send",
                       ctx.kAccentBlue, 0x0090FFFF)) {
        if (!m_inputBuffer.empty()) {
            m_chatHistory.push_back({true, m_inputBuffer});
            m_chatHistory.push_back({false, "Processing your request..."});
            m_inputBuffer.clear();
        }
    }

    // ── Context panel ─────────────────────────────────────────────
    ctx.drawPanel(ctx.x + sessionW + mainW, ctx.y, ctxW, ctx.h, "Context");
    const float cpx = ctx.x + sessionW + mainW;
    ctx.ui.drawText(cpx + 8.f, ctx.y + 30.f, "Active File", ctx.kTextSecond);
    ctx.ui.drawText(cpx + 8.f, ctx.y + 48.f, "(none)", ctx.kTextMuted);
    if (ctx.shell && ctx.shell->hasProject()) {
        ctx.ui.drawText(cpx + 8.f, ctx.y + 70.f, "Project:", ctx.kTextSecond);
        std::string projName = ctx.shell->projectAdapter()
                             ? ctx.shell->projectAdapter()->projectDisplayName()
                             : "(loaded)";
        if (projName.size() > 18) projName = projName.substr(0, 15) + "...";
        ctx.ui.drawText(cpx + 8.f, ctx.y + 86.f, projName, ctx.kTextMuted);
    }
}


void AtlasAITool::syncPanels(WorkspacePanelHost& host) const {
    (void)host;
}

} // namespace NF
