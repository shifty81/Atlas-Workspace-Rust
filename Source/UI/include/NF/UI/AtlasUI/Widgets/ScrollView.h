#pragma once

#include "NF/UI/AtlasUI/WidgetBase.h"
#include "NF/UI/AtlasUI/WidgetTheme.h"
#include <algorithm>
#include <memory>

namespace NF::UI::AtlasUI {

class ScrollView final : public WidgetBase {
public:
    ScrollView() = default;

    void setContent(std::shared_ptr<IWidget> content) { m_content = std::move(content); }
    [[nodiscard]] std::shared_ptr<IWidget> content() const { return m_content; }

    void setContentHeight(float height) { m_contentHeight = height; }
    [[nodiscard]] float contentHeight() const { return m_contentHeight; }

    void setScrollOffset(float offset);
    [[nodiscard]] float scrollOffset() const { return m_scrollOffset; }

    void scrollBy(float delta);
    void scrollToTop() { setScrollOffset(0.f); }
    void scrollToBottom();

    [[nodiscard]] float maxScrollOffset() const;
    [[nodiscard]] float viewportHeight() const { return m_bounds.h; }
    [[nodiscard]] bool canScroll() const { return m_contentHeight > m_bounds.h; }
    [[nodiscard]] float scrollFraction() const;
    [[nodiscard]] float thumbHeight() const;
    [[nodiscard]] float thumbY() const;

    [[nodiscard]] float scrollBarWidth() const { return m_scrollBarWidth; }

    void measure(ILayoutContext& context) override;
    void arrange(const NF::Rect& bounds) override;
    void paint(IPaintContext& context) override;
    bool handleInput(IInputContext& context) override;

private:
    std::shared_ptr<IWidget> m_content;
    float m_contentHeight = 0.f;
    float m_scrollOffset = 0.f;
    float m_scrollBarWidth = 10.f;
    float m_scrollStep = 20.f;
    bool m_thumbDragging = false;
    float m_dragStartY = 0.f;
    float m_dragStartOffset = 0.f;
};

} // namespace NF::UI::AtlasUI
