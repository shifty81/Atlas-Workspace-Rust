#include "UIManager.h"
#include "UIConstants.h"

namespace atlas::ui {

// --- MenuItem rendering constants ---
static constexpr const char* kCheckmarkSymbol = "\xe2\x9c\x93"; // ✓

/// Compute horizontal offset for icon placement, accounting for checkmark space.
static int32_t IconOffsetX(int32_t rectX, bool isCheckable) {
    return rectX + (isCheckable ? 18 : 2);
}

void UIManager::Init(GUIContext context) {
    m_context = context;

    std::string screenName;
    switch (context) {
        case GUIContext::Editor: screenName = "EditorScreen"; break;
        case GUIContext::Game:   screenName = "GameScreen";   break;
        case GUIContext::Server: screenName = "ServerScreen"; break;
    }

    m_screen.Init(screenName);
    m_menuManager.Init(&m_screen);
    m_tabManager.Init(&m_screen);
    m_scrollManager.Init(&m_screen);
    m_toolbarManager.Init(&m_screen);
    m_focusManager.Init(&m_screen);
    m_tooltipManager.Init(&m_screen);
    m_dockManager.Init(&m_screen);
    m_inputFieldManager.Init(&m_screen);
    m_sliderManager.Init(&m_screen);
    m_comboBoxManager.Init(&m_screen);
    m_checkboxManager.Init(&m_screen);
    m_treeNodeManager.Init(&m_screen);
    m_splitterManager.Init(&m_screen);
    m_colorPickerManager.Init(&m_screen);
    m_initialized = true;
}

void UIManager::Shutdown() {
    m_fontBootstrap.Shutdown();
    m_eventRouter.Clear();
    m_commandBus.Clear();
    m_renderer = nullptr;
    m_viewportWidth = 0.0f;
    m_viewportHeight = 0.0f;
    m_dpiScale = 1.0f;
    m_initialized = false;
}

void UIManager::SetContext(GUIContext context) {
    m_context = context;
}

GUIContext UIManager::GetContext() const {
    return m_context;
}

void UIManager::Update(const UIContext& ctx) {
    if (!m_initialized) return;

    m_commandBus.Dispatch();

    if (m_graph.IsCompiled()) {
        m_graph.Execute(ctx);
    }
}

void UIManager::SetRenderer(UIRenderer* renderer) {
    m_renderer = renderer;
}

UIRenderer* UIManager::GetRenderer() const {
    return m_renderer;
}

void UIManager::Render(UIRenderer* renderer) {
    if (!m_initialized || !renderer) return;

    // Walk root-level widgets (parentId == 0) and render them
    auto roots = m_screen.GetChildren(0);
    for (uint32_t id : roots) {
        RenderWidget(renderer, id);
    }

    // Second pass: re-render open menu dropdowns on top of all other UI.
    // Menu items are children of Menu widgets whose dropdown area can overlap
    // with other panels (e.g. the toolbar).  Drawing them again in a second
    // pass ensures they appear above everything else.
    RenderMenuOverlays(renderer);
}

void UIManager::RenderMenuOverlays(UIRenderer* renderer) {
    for (uint32_t i = 1; i < kMaxWidgetId; ++i) {
        const UIWidget* widget = m_screen.GetWidget(i);
        if (!widget || !widget->visible) continue;
        if (widget->type != UIWidgetType::Menu) continue;
        if (!widget->isMenuOpen) continue;

        auto children = m_screen.GetChildren(i);
        if (children.empty()) continue;

        // Compute dropdown bounding box from children
        int32_t minX = INT32_MAX, minY = INT32_MAX;
        int32_t maxX = INT32_MIN, maxY = INT32_MIN;
        for (uint32_t childId : children) {
            const UIWidget* child = m_screen.GetWidget(childId);
            if (!child || !child->visible) continue;
            int32_t cx = static_cast<int32_t>(child->x);
            int32_t cy = static_cast<int32_t>(child->y);
            int32_t cw = static_cast<int32_t>(child->width);
            int32_t ch = static_cast<int32_t>(child->height);
            if (cx < minX) minX = cx;
            if (cy < minY) minY = cy;
            if (cx + cw > maxX) maxX = cx + cw;
            if (cy + ch > maxY) maxY = cy + ch;
        }

        if (minX >= maxX || minY >= maxY) continue;

        // Draw opaque dropdown background
        UIRect dropBg = {minX - 1, minY - 1, (maxX - minX) + 2, (maxY - minY) + 2};
        UIColor bgColor = {45, 47, 50, 255};
        renderer->DrawRect(dropBg, bgColor);
        UIColor borderColor = {70, 73, 75, 255};
        renderer->DrawBorder(dropBg, 1, borderColor);

        // Re-render each menu item child on top
        for (uint32_t childId : children) {
            RenderWidget(renderer, childId);
        }
    }
}

void UIManager::RenderWidget(UIRenderer* renderer, uint32_t widgetId, int depth) {
    if (depth >= kMaxRenderDepth) return;
    const UIWidget* widget = m_screen.GetWidget(widgetId);
    if (!widget || !widget->visible) return;

    UIRect rect;
    rect.x = static_cast<int32_t>(widget->x);
    rect.y = static_cast<int32_t>(widget->y);
    rect.w = static_cast<int32_t>(widget->width);
    rect.h = static_cast<int32_t>(widget->height);

    switch (widget->type) {
        case UIWidgetType::Panel: {
            UIColor bg = {43, 43, 43, 255};
            renderer->DrawRect(rect, bg);
            UIColor border = {70, 73, 75, 255};
            renderer->DrawBorder(rect, 1, border);
            break;
        }
        case UIWidgetType::Button: {
            UIColor bg = widget->isHovered ? UIColor{70, 75, 82, 255} : UIColor{55, 58, 62, 255};
            renderer->DrawRect(rect, bg);
            UIColor border = widget->isHovered ? UIColor{90, 95, 105, 255} : UIColor{80, 83, 88, 255};
            renderer->DrawBorder(rect, 1, border);
            UIColor textColor = {220, 220, 220, 255};
            renderer->DrawText(rect, widget->name, textColor);
            break;
        }
        case UIWidgetType::Text: {
            UIColor textColor = {220, 220, 220, 255};
            renderer->DrawText(rect, widget->name, textColor);
            break;
        }
        case UIWidgetType::Image: {
            UIColor tint = {255, 255, 255, 255};
            renderer->DrawImage(rect, 0, tint);
            break;
        }
        case UIWidgetType::List: {
            UIColor bg = {35, 37, 40, 255};
            renderer->DrawRect(rect, bg);
            UIColor border = {70, 73, 75, 255};
            renderer->DrawBorder(rect, 1, border);
            break;
        }
        case UIWidgetType::SlotGrid: {
            UIColor bg = {43, 43, 43, 255};
            renderer->DrawRect(rect, bg);
            UIColor border = {70, 73, 75, 255};
            renderer->DrawBorder(rect, 1, border);
            break;
        }
        case UIWidgetType::InputField: {
            UIColor bg = {35, 37, 40, 255};
            renderer->DrawRect(rect, bg);
            bool focused = (m_focusManager.GetFocusedWidgetId() == widgetId);
            UIColor border = focused ? UIColor{90, 140, 210, 255} : UIColor{70, 100, 150, 255};
            renderer->DrawBorder(rect, focused ? 2 : 1, border);
            UIColor textColor = widget->name.empty() ? UIColor{100, 100, 100, 255} : UIColor{200, 200, 200, 255};
            std::string displayText = widget->name;
            // Show placeholder when empty and not focused
            if (displayText.empty() && !focused) {
                displayText = m_inputFieldManager.GetPlaceholder(widgetId);
                textColor = {100, 100, 100, 255};
            }
            renderer->DrawText(rect, displayText, textColor);
            // Draw cursor when focused
            if (focused) {
                size_t cursorPos = m_inputFieldManager.GetCursorPos(widgetId);
                int32_t cursorX = rect.x + 2 + static_cast<int32_t>(cursorPos) * kFontCharAdvance;
                UIColor cursorColor = {220, 220, 220, 255};
                UIRect cursorRect = {cursorX, rect.y + 2, 2, rect.h - 4};
                renderer->DrawRect(cursorRect, cursorColor);
            }
            break;
        }
        case UIWidgetType::Menu: {
            // Menu button in menu bar
            UIColor bg = widget->isMenuOpen ? UIColor{65, 68, 72, 255} : UIColor{43, 43, 43, 255};
            renderer->DrawRect(rect, bg);
            if (widget->isHovered || widget->isMenuOpen) {
                UIColor highlight = {75, 78, 82, 255};
                renderer->DrawRect(rect, highlight);
            }
            UIColor textColor = {220, 220, 220, 255};
            renderer->DrawText(rect, widget->name, textColor);
            break;
        }
        case UIWidgetType::MenuItem: {
            if (widget->isSeparator) {
                // Draw separator line
                UIColor separatorColor = {70, 73, 75, 255};
                UIRect sepRect = {rect.x + 4, rect.y + rect.h / 2, rect.w - 8, 1};
                renderer->DrawRect(sepRect, separatorColor);
            } else if (widget->isDisabled) {
                // Disabled menu item — grayed-out text, no hover highlight
                UIColor bg = {45, 47, 50, 255};
                renderer->DrawRect(rect, bg);
                UIColor textColor = {100, 100, 100, 255};
                renderer->DrawText(rect, widget->name, textColor);
                // Draw shortcut label if present, also grayed out
                if (!widget->shortcutLabel.empty()) {
                    UIRect shortcutRect = {rect.x + rect.w - 80, rect.y, 70, rect.h};
                    renderer->DrawText(shortcutRect, widget->shortcutLabel, textColor);
                }
                // Draw icon if present, grayed out
                if (widget->iconId != 0) {
                    UIColor iconTint = {100, 100, 100, 255};
                    UIRect iconRect = {IconOffsetX(rect.x, widget->isCheckable), rect.y + 2, rect.h - 4, rect.h - 4};
                    renderer->DrawIcon(iconRect, widget->iconId, iconTint);
                }
            } else {
                // Normal menu item
                UIColor bg = widget->isHovered ? UIColor{65, 115, 180, 255} : UIColor{45, 47, 50, 255};
                renderer->DrawRect(rect, bg);
                // Checkmark indicator
                if (widget->isCheckable) {
                    UIColor checkColor = widget->isChecked ? UIColor{220, 220, 220, 255} : UIColor{80, 80, 80, 255};
                    UIRect checkRect = {rect.x + 2, rect.y, 16, rect.h};
                    renderer->DrawText(checkRect, widget->isChecked ? kCheckmarkSymbol : " ", checkColor);
                }
                // Icon rendering
                if (widget->iconId != 0) {
                    UIColor iconTint = {255, 255, 255, 255};
                    UIRect iconRect = {IconOffsetX(rect.x, widget->isCheckable), rect.y + 2, rect.h - 4, rect.h - 4};
                    renderer->DrawIcon(iconRect, widget->iconId, iconTint);
                }
                UIColor textColor = {220, 220, 220, 255};
                renderer->DrawText(rect, widget->name, textColor);
                // Draw shortcut label right-aligned if present
                if (!widget->shortcutLabel.empty()) {
                    UIColor shortcutColor = {160, 160, 160, 255};
                    UIRect shortcutRect = {rect.x + rect.w - 80, rect.y, 70, rect.h};
                    renderer->DrawText(shortcutRect, widget->shortcutLabel, shortcutColor);
                }
                // Draw submenu indicator arrow if this item has a submenu
                if (widget->hasSubmenu) {
                    UIColor arrowColor = {180, 180, 180, 255};
                    UIRect arrowRect = {rect.x + rect.w - 16, rect.y, 12, rect.h};
                    renderer->DrawText(arrowRect, ">", arrowColor);
                }
            }
            break;
        }
        case UIWidgetType::Toolbar: {
            UIColor bg = {50, 52, 56, 255};
            renderer->DrawRect(rect, bg);
            UIColor borderBottom = {70, 73, 75, 255};
            UIRect bottomLine = {rect.x, rect.y + rect.h - 1, rect.w, 1};
            renderer->DrawRect(bottomLine, borderBottom);
            break;
        }
        case UIWidgetType::StatusBar: {
            UIColor bg = {30, 31, 34, 255};
            renderer->DrawRect(rect, bg);
            UIColor borderTop = {70, 73, 75, 255};
            UIRect topLine = {rect.x, rect.y, rect.w, 1};
            renderer->DrawRect(topLine, borderTop);
            UIColor textColor = {160, 160, 160, 255};
            renderer->DrawText(rect, widget->name, textColor);
            break;
        }
        case UIWidgetType::Tooltip: {
            UIColor bg = {60, 62, 66, 240};
            renderer->DrawRect(rect, bg);
            UIColor border = {100, 103, 108, 255};
            renderer->DrawBorder(rect, 1, border);
            UIColor textColor = {220, 220, 220, 255};
            renderer->DrawText(rect, widget->name, textColor);
            break;
        }
        case UIWidgetType::Tab: {
            UIColor bg = widget->isHovered ? UIColor{55, 58, 62, 255} : UIColor{43, 43, 43, 255};
            renderer->DrawRect(rect, bg);
            if (widget->isChecked) {
                // Active tab: highlight bottom border
                UIColor activeBar = {65, 115, 180, 255};
                UIRect barRect = {rect.x, rect.y + rect.h - 2, rect.w, 2};
                renderer->DrawRect(barRect, activeBar);
            }
            UIColor textColor = widget->isChecked ? UIColor{220, 220, 220, 255} : UIColor{160, 160, 160, 255};
            renderer->DrawText(rect, widget->name, textColor);
            break;
        }
        case UIWidgetType::ScrollView: {
            UIColor bg = {35, 37, 40, 255};
            renderer->DrawRect(rect, bg);
            UIColor border = {70, 73, 75, 255};
            renderer->DrawBorder(rect, 1, border);
            break;
        }
        case UIWidgetType::DockArea: {
            UIColor bg = {38, 40, 43, 255};
            renderer->DrawRect(rect, bg);
            UIColor border = {60, 63, 67, 255};
            renderer->DrawBorder(rect, 1, border);
            break;
        }
        case UIWidgetType::Checkbox: {
            // Draw checkbox box
            UIRect boxRect = {rect.x, rect.y + (rect.h - 14) / 2, 14, 14};
            UIColor boxBg = {35, 37, 40, 255};
            renderer->DrawRect(boxRect, boxBg);
            UIColor boxBorder = {70, 73, 75, 255};
            renderer->DrawBorder(boxRect, 1, boxBorder);
            if (widget->isChecked) {
                UIColor checkColor = {65, 115, 180, 255};
                renderer->DrawText(boxRect, kCheckmarkSymbol, checkColor);
            }
            // Draw label text to the right
            UIRect labelRect = {rect.x + 20, rect.y, rect.w - 20, rect.h};
            UIColor textColor = {220, 220, 220, 255};
            renderer->DrawText(labelRect, widget->name, textColor);
            break;
        }
        case UIWidgetType::Slider: {
            // Draw track
            int32_t trackY = rect.y + rect.h / 2 - 2;
            UIRect trackRect = {rect.x, trackY, rect.w, 4};
            UIColor trackBg = {35, 37, 40, 255};
            renderer->DrawRect(trackRect, trackBg);
            // Draw filled portion
            int32_t fillW = static_cast<int32_t>(static_cast<float>(rect.w) * widget->value);
            UIRect fillRect = {rect.x, trackY, fillW, 4};
            UIColor fillColor = {65, 115, 180, 255};
            renderer->DrawRect(fillRect, fillColor);
            // Draw thumb
            int32_t thumbX = rect.x + fillW - 6;
            UIRect thumbRect = {thumbX, rect.y + rect.h / 2 - 6, 12, 12};
            UIColor thumbColor = {220, 220, 220, 255};
            renderer->DrawRect(thumbRect, thumbColor);
            break;
        }
        case UIWidgetType::ProgressBar: {
            // Draw background
            UIColor bg = {35, 37, 40, 255};
            renderer->DrawRect(rect, bg);
            UIColor border = {70, 73, 75, 255};
            renderer->DrawBorder(rect, 1, border);
            // Draw filled portion
            int32_t fillW = static_cast<int32_t>(static_cast<float>(rect.w) * widget->value);
            UIRect fillRect = {rect.x, rect.y, fillW, rect.h};
            UIColor fillColor = {65, 115, 180, 255};
            renderer->DrawRect(fillRect, fillColor);
            // Draw name text centered
            UIColor textColor = {220, 220, 220, 255};
            renderer->DrawText(rect, widget->name, textColor);
            break;
        }
        case UIWidgetType::ComboBox: {
            UIColor bg = {35, 37, 40, 255};
            renderer->DrawRect(rect, bg);
            UIColor border = {70, 100, 150, 255};
            renderer->DrawBorder(rect, 1, border);
            UIColor textColor = {220, 220, 220, 255};
            renderer->DrawText(rect, widget->name, textColor);
            // Draw dropdown arrow on the right
            UIRect arrowRect = {rect.x + rect.w - 20, rect.y, 20, rect.h};
            UIColor arrowColor = {180, 180, 180, 255};
            renderer->DrawText(arrowRect, "\xe2\x96\xbc", arrowColor); // ▼
            break;
        }
        case UIWidgetType::TreeNode: {
            // Draw expand/collapse indicator indented by treeDepth
            int32_t indent = widget->treeDepth * 16;
            UIRect indicatorRect = {rect.x + indent, rect.y, 16, rect.h};
            UIColor indicatorColor = {180, 180, 180, 255};
            if (widget->isExpanded) {
                renderer->DrawText(indicatorRect, "\xe2\x96\xbe", indicatorColor); // ▾
            } else {
                renderer->DrawText(indicatorRect, "\xe2\x96\xb8", indicatorColor); // ▸
            }
            // Draw name text
            UIRect labelRect = {rect.x + indent + 16, rect.y, rect.w - indent - 16, rect.h};
            UIColor textColor = {220, 220, 220, 255};
            renderer->DrawText(labelRect, widget->name, textColor);
            break;
        }
        case UIWidgetType::Splitter: {
            UIColor bg = {55, 58, 62, 255};
            renderer->DrawRect(rect, bg);
            break;
        }
        case UIWidgetType::ColorPicker: {
            // Draw color swatch
            UIColor swatch = {widget->colorR, widget->colorG, widget->colorB, widget->colorA};
            renderer->DrawRect(rect, swatch);
            UIColor border = {70, 73, 75, 255};
            renderer->DrawBorder(rect, 1, border);
            // Draw name text
            UIColor textColor = {220, 220, 220, 255};
            UIRect labelRect = {rect.x + rect.w + 4, rect.y, 100, rect.h};
            renderer->DrawText(labelRect, widget->name, textColor);
            break;
        }
    }

    // Menu dropdowns only render children when open
    if (widget->type == UIWidgetType::Menu && !widget->isMenuOpen) {
        return;
    }

    // Render children
    auto children = m_screen.GetChildren(widgetId);
    for (uint32_t childId : children) {
        RenderWidget(renderer, childId, depth + 1);
    }
}

UIScreen& UIManager::GetScreen() {
    return m_screen;
}

const UIScreen& UIManager::GetScreen() const {
    return m_screen;
}

UIGraph& UIManager::GetGraph() {
    return m_graph;
}

const UIGraph& UIManager::GetGraph() const {
    return m_graph;
}

UICommandBus& UIManager::GetCommandBus() {
    return m_commandBus;
}

bool UIManager::IsInitialized() const {
    return m_initialized;
}

void UIManager::SetViewportSize(float width, float height) {
    float oldW = m_viewportWidth;
    float oldH = m_viewportHeight;
    m_viewportWidth = width;
    m_viewportHeight = height;

    // Proportionally scale the widget layout when the viewport changes
    if (oldW > 0.0f && oldH > 0.0f && width > 0.0f && height > 0.0f) {
        m_screen.ScaleLayout(oldW, oldH, width, height);
    }
}

float UIManager::GetViewportWidth() const {
    return m_viewportWidth;
}

float UIManager::GetViewportHeight() const {
    return m_viewportHeight;
}

void UIManager::SetDPIScale(float scale) {
    m_dpiScale = scale;
}

float UIManager::GetDPIScale() const {
    return m_dpiScale;
}

UIEventRouter& UIManager::GetEventRouter() {
    return m_eventRouter;
}

const UIEventRouter& UIManager::GetEventRouter() const {
    return m_eventRouter;
}

bool UIManager::DispatchEvent(const UIEvent& event) {
    if (!m_initialized) return false;
    
    // Let slider manager handle drag events first
    if (m_sliderManager.HandleEvent(event)) {
        return true;
    }

    // Let splitter manager handle drag events
    if (m_splitterManager.HandleEvent(event)) {
        return true;
    }

    // Let combo box manager handle click events
    if (m_comboBoxManager.HandleEvent(event)) {
        return true;
    }

    // Let color picker manager handle click events
    if (m_colorPickerManager.HandleEvent(event)) {
        return true;
    }

    // Let menu manager handle the event first
    if (m_menuManager.HandleEvent(event)) {
        return true;
    }

    // Let checkbox manager handle click events
    if (m_checkboxManager.HandleEvent(event)) {
        return true;
    }

    // Let tree node manager handle click events
    if (m_treeNodeManager.HandleEvent(event)) {
        return true;
    }

    // Update hover states for interactive widgets on mouse move
    if (event.type == UIEvent::Type::MouseMove) {
        for (uint32_t i = 1; i < kMaxWidgetId; ++i) {
            UIWidget* w = m_screen.GetWidgetMutable(i);
            if (!w || !w->visible) continue;
            if (w->type == UIWidgetType::Button ||
                w->type == UIWidgetType::Tab) {
                bool inside = (event.x >= w->x && event.x < w->x + w->width &&
                               event.y >= w->y && event.y < w->y + w->height);
                w->isHovered = inside;
            }
        }
    }

    // Handle scroll wheel events via ScrollManager
    if (event.type == UIEvent::Type::ScrollWheel) {
        if (m_scrollManager.HandleScrollWheel(event.x, event.y, event.scrollDelta)) {
            return true;
        }
    }

    // Handle tab clicks
    if (event.type == UIEvent::Type::MouseDown && event.mouseButton == 0) {
        if (m_tabManager.HandleClick(event.x, event.y)) {
            return true;
        }
        if (m_toolbarManager.HandleClick(event.x, event.y)) {
            return true;
        }
        // Handle general button clicks (non-toolbar buttons)
        for (uint32_t i = 1; i < kMaxWidgetId; ++i) {
            const UIWidget* w = m_screen.GetWidget(i);
            if (!w || !w->visible) continue;
            if (w->type != UIWidgetType::Button) continue;
            // Skip toolbar buttons (already handled above)
            const UIWidget* parent = m_screen.GetWidget(w->parentId);
            if (parent && parent->type == UIWidgetType::Toolbar) continue;
            if (event.x >= w->x && event.x < w->x + w->width &&
                event.y >= w->y && event.y < w->y + w->height) {
                UICommand cmd;
                cmd.type = UICommandType::ButtonPress;
                cmd.targetWidgetId = i;
                m_commandBus.Enqueue(std::move(cmd));
                return true;
            }
        }
        // Update focus on click
        m_focusManager.HandleClick(event.x, event.y);
    }

    // Route keyboard/text events to input field manager if a field is focused
    if (event.type == UIEvent::Type::KeyDown || event.type == UIEvent::Type::TextInput) {
        uint32_t focusedId = m_focusManager.GetFocusedWidgetId();
        if (focusedId != 0 && m_inputFieldManager.IsRegistered(focusedId)) {
            if (m_inputFieldManager.HandleEvent(event, focusedId)) {
                return true;
            }
        }
        // Tab key cycles focus
        if (m_focusManager.HandleKeyEvent(event)) {
            return true;
        }
    }

    return m_eventRouter.Dispatch(event);
}

FontBootstrap& UIManager::GetFontBootstrap() {
    return m_fontBootstrap;
}

const FontBootstrap& UIManager::GetFontBootstrap() const {
    return m_fontBootstrap;
}

bool UIManager::IsFontReady() const {
    return m_fontBootstrap.IsReady();
}

MenuManager& UIManager::GetMenuManager() {
    return m_menuManager;
}

const MenuManager& UIManager::GetMenuManager() const {
    return m_menuManager;
}

TabManager& UIManager::GetTabManager() {
    return m_tabManager;
}

const TabManager& UIManager::GetTabManager() const {
    return m_tabManager;
}

ScrollManager& UIManager::GetScrollManager() {
    return m_scrollManager;
}

const ScrollManager& UIManager::GetScrollManager() const {
    return m_scrollManager;
}

ToolbarManager& UIManager::GetToolbarManager() {
    return m_toolbarManager;
}

const ToolbarManager& UIManager::GetToolbarManager() const {
    return m_toolbarManager;
}

FocusManager& UIManager::GetFocusManager() {
    return m_focusManager;
}

const FocusManager& UIManager::GetFocusManager() const {
    return m_focusManager;
}

TooltipManager& UIManager::GetTooltipManager() {
    return m_tooltipManager;
}

const TooltipManager& UIManager::GetTooltipManager() const {
    return m_tooltipManager;
}

DockManager& UIManager::GetDockManager() {
    return m_dockManager;
}

const DockManager& UIManager::GetDockManager() const {
    return m_dockManager;
}

InputFieldManager& UIManager::GetInputFieldManager() {
    return m_inputFieldManager;
}

const InputFieldManager& UIManager::GetInputFieldManager() const {
    return m_inputFieldManager;
}

SliderManager& UIManager::GetSliderManager() {
    return m_sliderManager;
}

const SliderManager& UIManager::GetSliderManager() const {
    return m_sliderManager;
}

ComboBoxManager& UIManager::GetComboBoxManager() {
    return m_comboBoxManager;
}

const ComboBoxManager& UIManager::GetComboBoxManager() const {
    return m_comboBoxManager;
}

CheckboxManager& UIManager::GetCheckboxManager() {
    return m_checkboxManager;
}

const CheckboxManager& UIManager::GetCheckboxManager() const {
    return m_checkboxManager;
}

TreeNodeManager& UIManager::GetTreeNodeManager() {
    return m_treeNodeManager;
}

const TreeNodeManager& UIManager::GetTreeNodeManager() const {
    return m_treeNodeManager;
}

SplitterManager& UIManager::GetSplitterManager() {
    return m_splitterManager;
}

const SplitterManager& UIManager::GetSplitterManager() const {
    return m_splitterManager;
}

ColorPickerManager& UIManager::GetColorPickerManager() {
    return m_colorPickerManager;
}

const ColorPickerManager& UIManager::GetColorPickerManager() const {
    return m_colorPickerManager;
}

} // namespace atlas::ui
