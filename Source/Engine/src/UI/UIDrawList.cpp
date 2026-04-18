#include "UIDrawList.h"

namespace atlas::ui {

void UIDrawList::DrawRect(const UIRect& rect, const UIColor& color) {
    m_commands.push_back({UIDrawCmd::Kind::Rect, rect, color, 0, 0, {}});
}

void UIDrawList::DrawText(const UIRect& rect, const std::string& text, const UIColor& color) {
    m_commands.push_back({UIDrawCmd::Kind::Text, rect, color, 0, 0, text});
}

void UIDrawList::DrawIcon(const UIRect& rect, uint32_t iconId, const UIColor& tint) {
    m_commands.push_back({UIDrawCmd::Kind::Icon, rect, tint, 0, iconId, {}});
}

void UIDrawList::DrawBorder(const UIRect& rect, int32_t thickness, const UIColor& color) {
    m_commands.push_back({UIDrawCmd::Kind::Border, rect, color, thickness, 0, {}});
}

void UIDrawList::DrawImage(const UIRect& rect, uint32_t textureId, const UIColor& tint) {
    m_commands.push_back({UIDrawCmd::Kind::Image, rect, tint, 0, textureId, {}});
}

void UIDrawList::Flush(UIRenderer* renderer) const {
    if (!renderer) return;

    for (const auto& cmd : m_commands) {
        switch (cmd.kind) {
            case UIDrawCmd::Kind::Rect:
                renderer->DrawRect(cmd.rect, cmd.color);
                break;
            case UIDrawCmd::Kind::Text:
                renderer->DrawText(cmd.rect, cmd.text, cmd.color);
                break;
            case UIDrawCmd::Kind::Icon:
                renderer->DrawIcon(cmd.rect, cmd.resourceId, cmd.color);
                break;
            case UIDrawCmd::Kind::Border:
                renderer->DrawBorder(cmd.rect, cmd.thickness, cmd.color);
                break;
            case UIDrawCmd::Kind::Image:
                renderer->DrawImage(cmd.rect, cmd.resourceId, cmd.color);
                break;
        }
    }
}

void UIDrawList::Clear() {
    m_commands.clear();
}

size_t UIDrawList::CommandCount() const {
    return m_commands.size();
}

const std::vector<UIDrawCmd>& UIDrawList::Commands() const {
    return m_commands;
}

} // namespace atlas::ui
