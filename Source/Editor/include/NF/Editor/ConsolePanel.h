#pragma once
// NF::Editor — Console panel
#include "NF/Core/Core.h"
#include "NF/Engine/Engine.h"
#include "NF/Renderer/Renderer.h"
#include "NF/UI/UI.h"
#include "NF/GraphVM/GraphVM.h"
#include "NF/Input/Input.h"
#include "NF/UI/UIWidgets.h"
#include <filesystem>
#include <fstream>
#include <set>
#include <map>
#include <deque>
#include <unordered_map>
#include "NF/Editor/EditorPanel.h"

namespace NF {

enum class ConsoleMessageLevel : uint8_t { Info, Warning, Error };

struct ConsoleMessage {
    std::string text;
    ConsoleMessageLevel level = ConsoleMessageLevel::Info;
    float timestamp = 0.f;
};

class ConsolePanel : public EditorPanel {
public:
    [[nodiscard]] const std::string& name() const override { return m_name; }
    [[nodiscard]] DockSlot slot() const override { return DockSlot::Bottom; }
    void update(float /*dt*/) override {}
    void render(UIRenderer& ui, const Rect& bounds, const EditorTheme& theme) override {
        ui.drawRect(bounds, 0x1A1A1AFF);
        Rect hdr{bounds.x, bounds.y, bounds.w, 22.f};
        ui.drawRect(hdr, theme.panelHeader);
        ui.drawText(bounds.x + 8.f, bounds.y + 4.f, "Console", theme.panelText);
        ui.drawRectOutline(bounds, theme.panelBorder, 1.f);

        float y = bounds.y + 26.f;
        for (auto& msg : m_messages) {
            if (y > bounds.y + bounds.h - 4.f) break;
            uint32_t color = theme.panelText;
            if (msg.level == ConsoleMessageLevel::Warning) color = 0xE8A435FF;
            if (msg.level == ConsoleMessageLevel::Error)   color = 0xF44747FF;
            ui.drawText(bounds.x + 8.f, y, msg.text, color);
            y += 16.f;
        }
    }

    void renderUI(UIContext& ctx, const Rect& bounds) override {
        ctx.beginPanel("Console", bounds);
        ctx.beginHorizontal();
        if (ctx.button("Clear")) clearMessages();
        ctx.endHorizontal();
        ctx.separator();
        float contentH = static_cast<float>(m_messages.size()) * 18.f;
        Rect scrollR{bounds.x, bounds.y + 60.f, bounds.w, bounds.h - 64.f};
        ctx.beginScrollArea("console_scroll", scrollR, contentH);
        for (auto& msg : m_messages) {
            uint32_t color = 0;
            if (msg.level == ConsoleMessageLevel::Warning) color = 0xE8A435FF;
            else if (msg.level == ConsoleMessageLevel::Error) color = 0xF44747FF;
            ctx.label(msg.text, color);
        }
        ctx.endScrollArea();
        ctx.endPanel();
    }

    void addMessage(const std::string& text, ConsoleMessageLevel level, float timestamp) {
        ConsoleMessage msg;
        msg.text = text;
        msg.level = level;
        msg.timestamp = timestamp;
        m_messages.push_back(std::move(msg));
        if (m_messages.size() > kMaxMessages) {
            m_messages.erase(m_messages.begin());
        }
    }

    /// Called by the log sink to feed NF_LOG_* messages into the console.
    void addLogMessage(LogLevel level, std::string_view category, std::string_view message) {
        ConsoleMessageLevel cl = ConsoleMessageLevel::Info;
        if (level == LogLevel::Warn)  cl = ConsoleMessageLevel::Warning;
        if (level == LogLevel::Error || level == LogLevel::Fatal) cl = ConsoleMessageLevel::Error;
        std::string text = std::string("[") + std::string(category) + "] " + std::string(message);
        addMessage(text, cl, 0.f);
    }

    void clearMessages() { m_messages.clear(); }
    [[nodiscard]] size_t messageCount() const { return m_messages.size(); }
    [[nodiscard]] const std::vector<ConsoleMessage>& messages() const { return m_messages; }

    static constexpr size_t kMaxMessages = 1000;

private:
    std::string m_name = "Console";
    std::vector<ConsoleMessage> m_messages;
};

// ── ContentBrowserPanel ──────────────────────────────────────────
// DEPRECATED: Use NF::UI::AtlasUI::ContentBrowserPanel instead (U3).


} // namespace NF
