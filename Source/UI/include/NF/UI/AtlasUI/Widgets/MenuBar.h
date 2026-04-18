#pragma once
// NF::UI::AtlasUI::MenuBar — horizontal menu bar widget.
// Displays a row of category labels; clicking a category opens a drop-down.

#include "NF/UI/AtlasUI/WidgetBase.h"
#include "NF/UI/AtlasUI/WidgetHelpers.h"
#include "NF/UI/AtlasUI/WidgetTheme.h"
#include <functional>
#include <string>
#include <vector>

namespace NF::UI::AtlasUI {

struct MenuBarEntry {
    std::string label;
    std::function<void()> action;
    bool separator = false;
};

struct MenuBarCategory {
    std::string label;
    std::vector<MenuBarEntry> entries;

    void addItem(std::string itemLabel, std::function<void()> action) {
        entries.push_back({std::move(itemLabel), std::move(action), false});
    }
    void addSeparator() {
        entries.push_back({"", nullptr, true});
    }
    [[nodiscard]] size_t entryCount() const { return entries.size(); }
};

class MenuBar final : public WidgetBase {
public:
    MenuBarCategory& addCategory(std::string label);
    [[nodiscard]] MenuBarCategory* findCategory(const std::string& label);
    [[nodiscard]] const std::vector<MenuBarCategory>& categories() const { return m_categories; }
    [[nodiscard]] size_t categoryCount() const { return m_categories.size(); }
    [[nodiscard]] int openCategoryIndex() const { return m_openIdx; }

    void closeMenu() { m_openIdx = -1; }

    void measure(ILayoutContext& context) override;
    void arrange(const NF::Rect& bounds) override;
    void paint(IPaintContext& context) override;
    bool handleInput(IInputContext& context) override;

    static constexpr float kBarHeight = 28.f;
    static constexpr float kItemHeight = 22.f;
    static constexpr float kCategoryPadX = 14.f;
    static constexpr float kDropShadow = 1.f;

private:
    [[nodiscard]] NF::Rect categoryRect(size_t idx) const;
    [[nodiscard]] NF::Rect dropDownRect(size_t idx) const;
    [[nodiscard]] NF::Rect dropEntryRect(size_t catIdx, size_t entryIdx) const;

    std::vector<MenuBarCategory> m_categories;
    std::vector<float> m_catWidths;
    int m_openIdx = -1;
    int m_hoveredCat = -1;
    int m_hoveredEntry = -1;
};

} // namespace NF::UI::AtlasUI
