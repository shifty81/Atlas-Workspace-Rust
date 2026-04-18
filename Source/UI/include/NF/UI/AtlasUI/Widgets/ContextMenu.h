#pragma once

#include "NF/UI/AtlasUI/WidgetBase.h"
#include "NF/UI/AtlasUI/WidgetTheme.h"
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace NF::UI::AtlasUI {

struct MenuItem {
    std::string label;
    std::string shortcutHint;
    std::function<void()> action;
    bool enabled = true;
    bool separator = false;
    std::vector<MenuItem> children;

    [[nodiscard]] bool isSeparator() const { return separator; }
    [[nodiscard]] bool hasChildren() const { return !children.empty(); }

    static MenuItem Separator() {
        MenuItem item;
        item.separator = true;
        return item;
    }
};

class ContextMenu final : public WidgetBase {
public:
    ContextMenu() = default;
    explicit ContextMenu(std::vector<MenuItem> items);

    void setItems(std::vector<MenuItem> items) { m_items = std::move(items); }
    void addItem(MenuItem item) { m_items.push_back(std::move(item)); }
    void clear() { m_items.clear(); }

    void open(NF::Vec2 position);
    void close();

    [[nodiscard]] bool isOpen() const { return m_open; }
    [[nodiscard]] size_t itemCount() const { return m_items.size(); }
    [[nodiscard]] const std::vector<MenuItem>& items() const { return m_items; }
    [[nodiscard]] int hoveredIndex() const { return m_hoveredIndex; }

    void measure(ILayoutContext& context) override;
    void paint(IPaintContext& context) override;
    bool handleInput(IInputContext& context) override;

private:
    std::vector<MenuItem> m_items;
    bool m_open = false;
    NF::Vec2 m_position{};
    int m_hoveredIndex = -1;
    float m_itemHeight = 24.f;
    float m_separatorHeight = 8.f;
    float m_minWidth = 160.f;
    float m_menuWidth = 160.f;
};

} // namespace NF::UI::AtlasUI
