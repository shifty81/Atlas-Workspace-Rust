#include "UIScreenGraph.h"
#include <algorithm>

namespace atlas::ui {

void UIScreen::Init(const std::string& name) {
    m_name = name;
    m_widgets.clear();
    m_nextId = 1;
}

const std::string& UIScreen::GetName() const {
    return m_name;
}

uint32_t UIScreen::AddWidget(UIWidgetType type, const std::string& name, float x, float y, float w, float h) {
    uint32_t id = m_nextId++;
    UIWidget widget;
    widget.id = id;
    widget.type = type;
    widget.name = name;
    widget.x = x;
    widget.y = y;
    widget.width = w;
    widget.height = h;
    m_widgets[id] = widget;
    return id;
}

void UIScreen::RemoveWidget(uint32_t id) {
    m_widgets.erase(id);
}

const UIWidget* UIScreen::GetWidget(uint32_t id) const {
    auto it = m_widgets.find(id);
    if (it != m_widgets.end()) {
        return &it->second;
    }
    return nullptr;
}

size_t UIScreen::WidgetCount() const {
    return m_widgets.size();
}

void UIScreen::SetVisible(uint32_t id, bool visible) {
    auto it = m_widgets.find(id);
    if (it != m_widgets.end()) {
        it->second.visible = visible;
    }
}

bool UIScreen::IsVisible(uint32_t id) const {
    auto it = m_widgets.find(id);
    if (it != m_widgets.end()) {
        return it->second.visible;
    }
    return false;
}

void UIScreen::SetParent(uint32_t childId, uint32_t parentId) {
    auto it = m_widgets.find(childId);
    if (it != m_widgets.end()) {
        it->second.parentId = parentId;
    }
}

std::vector<uint32_t> UIScreen::GetChildren(uint32_t parentId) const {
    std::vector<uint32_t> children;
    for (const auto& [id, widget] : m_widgets) {
        if (widget.parentId == parentId) {
            children.push_back(id);
        }
    }
    // Sort by ID to guarantee insertion-order (IDs are sequential).
    std::sort(children.begin(), children.end());
    return children;
}

void UIScreen::SetMenuOpen(uint32_t id, bool open) {
    auto it = m_widgets.find(id);
    if (it != m_widgets.end()) {
        it->second.isMenuOpen = open;
    }
}

bool UIScreen::IsMenuOpen(uint32_t id) const {
    auto it = m_widgets.find(id);
    if (it != m_widgets.end()) {
        return it->second.isMenuOpen;
    }
    return false;
}

void UIScreen::SetHovered(uint32_t id, bool hovered) {
    auto it = m_widgets.find(id);
    if (it != m_widgets.end()) {
        it->second.isHovered = hovered;
    }
}

bool UIScreen::IsHovered(uint32_t id) const {
    auto it = m_widgets.find(id);
    if (it != m_widgets.end()) {
        return it->second.isHovered;
    }
    return false;
}

void UIScreen::SetSeparator(uint32_t id, bool isSeparator) {
    auto it = m_widgets.find(id);
    if (it != m_widgets.end()) {
        it->second.isSeparator = isSeparator;
    }
}

void UIScreen::SetDisabled(uint32_t id, bool disabled) {
    auto it = m_widgets.find(id);
    if (it != m_widgets.end()) {
        it->second.isDisabled = disabled;
    }
}

bool UIScreen::IsDisabled(uint32_t id) const {
    auto it = m_widgets.find(id);
    if (it != m_widgets.end()) {
        return it->second.isDisabled;
    }
    return false;
}

void UIScreen::SetShortcutLabel(uint32_t id, const std::string& label) {
    auto it = m_widgets.find(id);
    if (it != m_widgets.end()) {
        it->second.shortcutLabel = label;
    }
}

void UIScreen::SetHasSubmenu(uint32_t id, bool hasSubmenu) {
    auto it = m_widgets.find(id);
    if (it != m_widgets.end()) {
        it->second.hasSubmenu = hasSubmenu;
    }
}

void UIScreen::SetCheckable(uint32_t id, bool checkable) {
    auto it = m_widgets.find(id);
    if (it != m_widgets.end()) {
        it->second.isCheckable = checkable;
    }
}

bool UIScreen::IsCheckable(uint32_t id) const {
    auto it = m_widgets.find(id);
    if (it != m_widgets.end()) {
        return it->second.isCheckable;
    }
    return false;
}

void UIScreen::SetChecked(uint32_t id, bool checked) {
    auto it = m_widgets.find(id);
    if (it != m_widgets.end()) {
        it->second.isChecked = checked;
    }
}

bool UIScreen::IsChecked(uint32_t id) const {
    auto it = m_widgets.find(id);
    if (it != m_widgets.end()) {
        return it->second.isChecked;
    }
    return false;
}

void UIScreen::SetIconId(uint32_t id, uint32_t iconId) {
    auto it = m_widgets.find(id);
    if (it != m_widgets.end()) {
        it->second.iconId = iconId;
    }
}

uint32_t UIScreen::GetIconId(uint32_t id) const {
    auto it = m_widgets.find(id);
    if (it != m_widgets.end()) {
        return it->second.iconId;
    }
    return 0;
}

UIWidget* UIScreen::GetWidgetMutable(uint32_t id) {
    auto it = m_widgets.find(id);
    if (it != m_widgets.end()) {
        return &it->second;
    }
    return nullptr;
}

void UIScreen::SetValue(uint32_t id, float value) {
    auto it = m_widgets.find(id);
    if (it != m_widgets.end()) {
        it->second.value = value;
    }
}

float UIScreen::GetValue(uint32_t id) const {
    auto it = m_widgets.find(id);
    if (it != m_widgets.end()) {
        return it->second.value;
    }
    return 0.0f;
}

void UIScreen::SetValueRange(uint32_t id, float minVal, float maxVal) {
    auto it = m_widgets.find(id);
    if (it != m_widgets.end()) {
        it->second.minValue = minVal;
        it->second.maxValue = maxVal;
    }
}

float UIScreen::GetMinValue(uint32_t id) const {
    auto it = m_widgets.find(id);
    if (it != m_widgets.end()) {
        return it->second.minValue;
    }
    return 0.0f;
}

float UIScreen::GetMaxValue(uint32_t id) const {
    auto it = m_widgets.find(id);
    if (it != m_widgets.end()) {
        return it->second.maxValue;
    }
    return 1.0f;
}

void UIScreen::SetSelectedIndex(uint32_t id, int32_t index) {
    auto it = m_widgets.find(id);
    if (it != m_widgets.end()) {
        it->second.selectedIndex = index;
    }
}

int32_t UIScreen::GetSelectedIndex(uint32_t id) const {
    auto it = m_widgets.find(id);
    if (it != m_widgets.end()) {
        return it->second.selectedIndex;
    }
    return -1;
}

void UIScreen::SetComboOpen(uint32_t id, bool open) {
    auto it = m_widgets.find(id);
    if (it != m_widgets.end()) {
        it->second.isOpen = open;
    }
}

bool UIScreen::IsComboOpen(uint32_t id) const {
    auto it = m_widgets.find(id);
    if (it != m_widgets.end()) {
        return it->second.isOpen;
    }
    return false;
}

void UIScreen::SetExpanded(uint32_t id, bool expanded) {
    auto it = m_widgets.find(id);
    if (it != m_widgets.end()) {
        it->second.isExpanded = expanded;
    }
}

bool UIScreen::IsExpanded(uint32_t id) const {
    auto it = m_widgets.find(id);
    if (it != m_widgets.end()) {
        return it->second.isExpanded;
    }
    return false;
}

void UIScreen::SetTreeDepth(uint32_t id, int32_t depth) {
    auto it = m_widgets.find(id);
    if (it != m_widgets.end()) {
        it->second.treeDepth = depth;
    }
}

int32_t UIScreen::GetTreeDepth(uint32_t id) const {
    auto it = m_widgets.find(id);
    if (it != m_widgets.end()) {
        return it->second.treeDepth;
    }
    return 0;
}

void UIScreen::SetColor(uint32_t id, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    auto it = m_widgets.find(id);
    if (it != m_widgets.end()) {
        it->second.colorR = r;
        it->second.colorG = g;
        it->second.colorB = b;
        it->second.colorA = a;
    }
}

void UIScreen::GetColor(uint32_t id, uint8_t& r, uint8_t& g, uint8_t& b, uint8_t& a) const {
    auto it = m_widgets.find(id);
    if (it != m_widgets.end()) {
        r = it->second.colorR;
        g = it->second.colorG;
        b = it->second.colorB;
        a = it->second.colorA;
    } else {
        r = 255; g = 255; b = 255; a = 255;
    }
}

void UIScreen::ScaleLayout(float oldWidth, float oldHeight,
                           float newWidth, float newHeight) {
    if (oldWidth <= 0.0f || oldHeight <= 0.0f) return;
    if (newWidth <= 0.0f || newHeight <= 0.0f) return;

    float sx = newWidth / oldWidth;
    float sy = newHeight / oldHeight;

    for (auto& [id, w] : m_widgets) {
        w.x      = w.x * sx;
        w.y      = w.y * sy;
        w.width  = w.width * sx;
        w.height = w.height * sy;
    }
}

} // namespace atlas::ui
