#pragma once

#include "NF/UI/AtlasUI/Widgets/Container.h"
#include "NF/UI/AtlasUI/WidgetTheme.h"
#include <string>
#include <utility>
#include <vector>

namespace NF::UI::AtlasUI {

struct TabEntry {
    std::string title;
    std::shared_ptr<IWidget> content;
};

class TabBar final : public Container {
public:
    void addTab(std::string title, std::shared_ptr<IWidget> content);
    void measure(ILayoutContext& context) override;
    void arrange(const NF::Rect& bounds) override;
    void paint(IPaintContext& context) override;
    bool handleInput(IInputContext& context) override;

    [[nodiscard]] size_t activeIndex() const { return m_activeIndex; }
    void setActiveIndex(size_t index);

private:
    [[nodiscard]] NF::Rect tabRect(size_t index) const;

    std::vector<TabEntry> m_tabs;
    size_t m_activeIndex = 0;
    float m_tabHeight = 28.f;
};

} // namespace NF::UI::AtlasUI
