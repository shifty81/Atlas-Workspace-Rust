#pragma once

#include "NF/UI/AtlasUI/PanelBase.h"
#include "NF/UI/AtlasUI/WidgetTheme.h"
#include "NF/UI/AtlasUI/WidgetHelpers.h"
#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

namespace NF::UI::AtlasUI {

/// Message level for console entries.
enum class MessageLevel : uint8_t { Info, Warning, Error };

/// AtlasUI ConsolePanel — displays log messages with level filtering.
/// Replaces the legacy NF::Editor::ConsolePanel for the AtlasUI framework.
class ConsolePanel final : public PanelBase {
public:
    ConsolePanel()
        : PanelBase("atlas.console", "Console") {}

    void paint(IPaintContext& context) override;
    bool handleInput(IInputContext& context) override;

    struct Message {
        std::string  text;
        MessageLevel level     = MessageLevel::Info;
        float        timestamp = 0.f;
    };

    void addMessage(const std::string& text, MessageLevel level, float timestamp = 0.f) {
        m_messages.push_back({text, level, timestamp});
        if (m_messages.size() > kMaxMessages)
            m_messages.erase(m_messages.begin());
        // Auto-scroll to show the newest message.
        m_scrollOffset = std::max(0.f, contentHeight() - viewportHeight());
    }

    void clearMessages() {
        m_messages.clear();
        m_scrollOffset = 0.f;
    }
    [[nodiscard]] size_t messageCount() const { return m_messages.size(); }
    [[nodiscard]] const std::vector<Message>& messages() const { return m_messages; }

    static constexpr size_t kMaxMessages = 1000;

private:
    static constexpr float kScrollBarW = 10.f;
    static constexpr float kRowH       = 16.f;
    static constexpr float kHeaderH    = 22.f;

    std::vector<Message> m_messages;

    float m_scrollOffset  = 0.f;
    bool  m_thumbDragging = false;
    float m_dragStartY    = 0.f;
    float m_dragStartOff  = 0.f;

    [[nodiscard]] float contentHeight()  const { return static_cast<float>(m_messages.size()) * kRowH; }
    [[nodiscard]] float viewportHeight() const { return std::max(0.f, m_bounds.h - kHeaderH); }
    [[nodiscard]] float maxScroll()      const { return std::max(0.f, contentHeight() - viewportHeight()); }
    [[nodiscard]] float thumbHeight() const {
        float vpH = viewportHeight();
        float ctH = contentHeight();
        if (ctH <= vpH) return vpH;
        return std::max(20.f, vpH * std::min(1.f, vpH / ctH));
    }
    [[nodiscard]] float thumbY() const {
        float track = viewportHeight() - thumbHeight();
        float frac  = (maxScroll() > 0.f) ? (m_scrollOffset / maxScroll()) : 0.f;
        return (m_bounds.y + kHeaderH) + track * frac;
    }
    void clampScroll() {
        m_scrollOffset = std::max(0.f, std::min(m_scrollOffset, maxScroll()));
    }
};

} // namespace NF::UI::AtlasUI
