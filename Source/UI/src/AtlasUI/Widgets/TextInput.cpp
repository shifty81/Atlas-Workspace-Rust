#include "NF/UI/AtlasUI/Widgets/TextInput.h"
#include "NF/UI/AtlasUI/WidgetHelpers.h"

namespace NF::UI::AtlasUI {

TextInput::TextInput(std::string placeholder)
    : m_placeholder(std::move(placeholder)) {}

void TextInput::setText(const std::string& text) {
    m_text = text;
    m_cursor = m_text.size();
}

void TextInput::measure(ILayoutContext&) {
    m_bounds.h = 28.f;
}

void TextInput::paint(IPaintContext& context) {
    if (!m_visible) return;

    Color bg = Theme::ColorToken::Background;
    Color border = m_focused ? Theme::ColorToken::Accent : Theme::ColorToken::Border;
    context.fillRect(m_bounds, bg);
    context.drawRect(m_bounds, border);

    NF::Rect textArea = insetRect(m_bounds, Theme::Spacing::Small, Theme::Spacing::Tiny);
    if (m_text.empty()) {
        context.drawText(textArea, m_placeholder, 0, Theme::ColorToken::TextMuted);
    } else {
        context.drawText(textArea, m_text, 0, Theme::ColorToken::Text);
    }

    if (m_focused) {
        constexpr float kCharWidth = 8.f;
        float cursorX = textArea.x + static_cast<float>(m_cursor) * kCharWidth;
        NF::Rect cursorRect = {cursorX, textArea.y, 1.f, textArea.h};
        context.fillRect(cursorRect, Theme::ColorToken::Text);
    }
}

bool TextInput::handleInput(IInputContext& context) {
    if (!m_visible) return false;
    bool inside = rectContains(m_bounds, context.mousePosition());

    // Focus management: click inside to gain focus, click outside to lose it.
    if (context.primaryDown()) {
        if (inside) {
            m_focused = true;
            context.requestFocus(this);
        } else {
            m_focused = false;
        }
    }

    // Process typed text only when focused.
    if (m_focused) {
        std::string_view typed = context.typedText();
        if (!typed.empty()) {
            bool changed = false;
            for (char ch : typed) {
                if (ch == '\b') {
                    // Backspace: remove character before cursor.
                    if (m_cursor > 0) {
                        m_text.erase(m_cursor - 1, 1);
                        --m_cursor;
                        changed = true;
                    }
                } else if (ch == '\r' || ch == '\n') {
                    // Enter: unfocus (single-line widget).
                    m_focused = false;
                } else if (static_cast<unsigned char>(ch) >= 0x20) {
                    // Printable character (>= ASCII 0x20 = space; excludes all control chars).
                    // Insert at cursor position.
                    m_text.insert(m_cursor, 1, ch);
                    ++m_cursor;
                    changed = true;
                }
            }
            if (changed && m_onChange) {
                m_onChange(m_text);
            }
        }
    }

    return inside;
}

} // namespace NF::UI::AtlasUI
