#pragma once

#include "NF/UI/AtlasUI/WidgetBase.h"
#include "NF/UI/AtlasUI/WidgetTheme.h"
#include <functional>
#include <memory>
#include <string>
#include <variant>
#include <vector>

namespace NF::UI::AtlasUI {

struct PropertyValue {
    using Variant = std::variant<std::string, float, int, bool>;
    Variant value;

    PropertyValue() : value(std::string{}) {}
    explicit PropertyValue(std::string v) : value(std::move(v)) {}
    explicit PropertyValue(const char* v) : value(std::string(v)) {}
    explicit PropertyValue(float v) : value(v) {}
    explicit PropertyValue(int v) : value(v) {}
    explicit PropertyValue(bool v) : value(v) {}

    [[nodiscard]] std::string asString() const;
};

struct PropertyItem {
    std::string name;
    PropertyValue value;
    std::string category;
    bool readOnly = false;
    bool expanded = true;
    std::vector<PropertyItem> children;

    [[nodiscard]] bool isGroup() const { return !children.empty(); }
    [[nodiscard]] size_t childCount() const { return children.size(); }
};

class PropertyGrid final : public WidgetBase {
public:
    using ChangeHandler = std::function<void(const std::string& name, const PropertyValue& newValue)>;

    PropertyGrid() = default;

    void setItems(std::vector<PropertyItem> items) { m_items = std::move(items); m_flatDirty = true; }
    void addItem(PropertyItem item) { m_items.push_back(std::move(item)); m_flatDirty = true; }
    void clear() { m_items.clear(); m_flatNodes.clear(); m_flatDirty = true; }

    void setOnChange(ChangeHandler handler) { m_onChange = std::move(handler); }
    void setLabelWidth(float width) { m_labelWidth = width; }

    [[nodiscard]] const std::vector<PropertyItem>& items() const { return m_items; }
    [[nodiscard]] size_t itemCount() const { return m_items.size(); }
    [[nodiscard]] float labelWidth() const { return m_labelWidth; }

    [[nodiscard]] size_t visibleRowCount() const;

    bool toggleGroup(const std::string& name);

    void measure(ILayoutContext& context) override;
    void arrange(const NF::Rect& bounds) override;
    void paint(IPaintContext& context) override;
    bool handleInput(IInputContext& context) override;

private:
    struct FlatEntry {
        PropertyItem* item = nullptr;
        int depth = 0;
        bool isGroupHeader = false;
        float y = 0.f;
    };

    void rebuildFlat();
    void flattenItems(std::vector<PropertyItem>& items, int depth);
    bool toggleInItems(std::vector<PropertyItem>& items, const std::string& name);

    std::vector<PropertyItem> m_items;
    std::vector<FlatEntry> m_flatNodes;
    ChangeHandler m_onChange;
    float m_labelWidth = 140.f;
    float m_rowHeight = 24.f;
    int m_hoveredRow = -1;
    bool m_flatDirty = true;
};

} // namespace NF::UI::AtlasUI
