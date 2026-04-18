#pragma once
#include "UIGraph.h"
#include "UIScreenGraph.h"
#include "UICommandBus.h"
#include "UIRenderer.h"
#include "UIEventRouter.h"
#include "FontBootstrap.h"
#include "MenuManager.h"
#include "TabManager.h"
#include "ScrollManager.h"
#include "ToolbarManager.h"
#include "FocusManager.h"
#include "TooltipManager.h"
#include "DockManager.h"
#include "InputFieldManager.h"
#include "SliderManager.h"
#include "ComboBoxManager.h"
#include "CheckboxManager.h"
#include "TreeNodeManager.h"
#include "SplitterManager.h"
#include "ColorPickerManager.h"
#include <string>

namespace atlas::ui {

enum class GUIContext : uint8_t {
    Editor,
    Game,
    Server
};

class UIManager {
public:
    void Init(GUIContext context);
    void Shutdown();

    void SetContext(GUIContext context);
    GUIContext GetContext() const;

    void Update(const UIContext& ctx);

    void SetRenderer(UIRenderer* renderer);
    UIRenderer* GetRenderer() const;

    void Render(UIRenderer* renderer);

    UIScreen& GetScreen();
    const UIScreen& GetScreen() const;

    UIGraph& GetGraph();
    const UIGraph& GetGraph() const;

    UICommandBus& GetCommandBus();

    bool IsInitialized() const;

    // --- Viewport & DPI ---

    /// Notify the UI system that the window has been resized.
    void SetViewportSize(float width, float height);

    /// Returns the current viewport width.
    float GetViewportWidth() const;

    /// Returns the current viewport height.
    float GetViewportHeight() const;

    /// Set the display DPI scale factor.
    void SetDPIScale(float scale);

    /// Returns the current DPI scale factor.
    float GetDPIScale() const;

    // --- Input Routing ---

    /// Access the centralized event router for UI input dispatch.
    UIEventRouter& GetEventRouter();
    const UIEventRouter& GetEventRouter() const;

    /// Dispatch a platform input event through the UI layer.
    /// Returns true if the event was consumed by a UI element.
    bool DispatchEvent(const UIEvent& event);

    // --- Font System ---

    /// Access the font bootstrap for font lifecycle management.
    FontBootstrap& GetFontBootstrap();
    const FontBootstrap& GetFontBootstrap() const;

    /// Returns true when the font system is ready for text rendering.
    bool IsFontReady() const;

    // --- Menu System ---

    /// Access the menu manager for menu state and interactions.
    MenuManager& GetMenuManager();
    const MenuManager& GetMenuManager() const;

    /// Access the tab manager for tab switching.
    TabManager& GetTabManager();
    const TabManager& GetTabManager() const;

    /// Access the scroll manager for ScrollView state.
    ScrollManager& GetScrollManager();
    const ScrollManager& GetScrollManager() const;

    /// Access the toolbar manager for toolbar button interactions.
    ToolbarManager& GetToolbarManager();
    const ToolbarManager& GetToolbarManager() const;

    /// Access the focus manager for widget focus tracking.
    FocusManager& GetFocusManager();
    const FocusManager& GetFocusManager() const;

    /// Access the tooltip manager for hover tooltip display.
    TooltipManager& GetTooltipManager();
    const TooltipManager& GetTooltipManager() const;

    /// Access the dock manager for panel docking.
    DockManager& GetDockManager();
    const DockManager& GetDockManager() const;

    /// Access the input field manager for text editing.
    InputFieldManager& GetInputFieldManager();
    const InputFieldManager& GetInputFieldManager() const;

    /// Access the slider manager for slider interactions.
    SliderManager& GetSliderManager();
    const SliderManager& GetSliderManager() const;

    /// Access the combo box manager for dropdown interactions.
    ComboBoxManager& GetComboBoxManager();
    const ComboBoxManager& GetComboBoxManager() const;

    /// Access the checkbox manager for checkbox toggle interactions.
    CheckboxManager& GetCheckboxManager();
    const CheckboxManager& GetCheckboxManager() const;

    /// Access the tree node manager for expand/collapse interactions.
    TreeNodeManager& GetTreeNodeManager();
    const TreeNodeManager& GetTreeNodeManager() const;

    /// Access the splitter manager for panel resize interactions.
    SplitterManager& GetSplitterManager();
    const SplitterManager& GetSplitterManager() const;

    /// Access the color picker manager for color selection interactions.
    ColorPickerManager& GetColorPickerManager();
    const ColorPickerManager& GetColorPickerManager() const;

private:
    void RenderWidget(UIRenderer* renderer, uint32_t widgetId, int depth = 0);
    /// Second-pass render of open menu dropdowns so they appear on top of
    /// all other widgets (toolbar, panels, etc.).
    void RenderMenuOverlays(UIRenderer* renderer);
    static constexpr int kMaxRenderDepth = 64;

    GUIContext m_context = GUIContext::Editor;
    UIScreen m_screen;
    UIGraph m_graph;
    UICommandBus m_commandBus;
    UIEventRouter m_eventRouter;
    FontBootstrap m_fontBootstrap;
    MenuManager m_menuManager;
    TabManager m_tabManager;
    ScrollManager m_scrollManager;
    ToolbarManager m_toolbarManager;
    FocusManager m_focusManager;
    TooltipManager m_tooltipManager;
    DockManager m_dockManager;
    InputFieldManager m_inputFieldManager;
    SliderManager m_sliderManager;
    ComboBoxManager m_comboBoxManager;
    CheckboxManager m_checkboxManager;
    TreeNodeManager m_treeNodeManager;
    SplitterManager m_splitterManager;
    ColorPickerManager m_colorPickerManager;
    UIRenderer* m_renderer = nullptr;
    float m_viewportWidth = 0.0f;
    float m_viewportHeight = 0.0f;
    float m_dpiScale = 1.0f;
    bool m_initialized = false;
};

} // namespace atlas::ui
