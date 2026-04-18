add_library(NF_UI STATIC
    src/UI.cpp

    # AtlasUI — Core
    src/AtlasUI/AtlasUI.cpp
    src/AtlasUI/PanelHost.cpp
    src/AtlasUI/DrawListDispatcher.cpp

    # AtlasUI — Theme
    src/AtlasUI/Theme/ThemeDefaults.cpp
    src/AtlasUI/Theme/ThemeManager.cpp

    # AtlasUI — Widgets
    src/AtlasUI/Widgets/Button.cpp
    src/AtlasUI/Widgets/Checkbox.cpp
    src/AtlasUI/Widgets/Panel.cpp
    src/AtlasUI/Widgets/TabBar.cpp
    src/AtlasUI/Widgets/Tabs.cpp
    src/AtlasUI/Widgets/StackPanel.cpp
    src/AtlasUI/Widgets/Splitter.cpp
    src/AtlasUI/Widgets/Toolbar.cpp
    src/AtlasUI/Widgets/Tooltip.cpp
    src/AtlasUI/Widgets/TextInput.cpp
    src/AtlasUI/Widgets/Dropdown.cpp
    src/AtlasUI/Widgets/PropertyRow.cpp
    src/AtlasUI/Widgets/NotificationCard.cpp
    src/AtlasUI/Widgets/ContextMenu.cpp
    src/AtlasUI/Widgets/TreeView.cpp
    src/AtlasUI/Widgets/ScrollView.cpp
    src/AtlasUI/Widgets/PropertyGrid.cpp
    src/AtlasUI/Widgets/TableView.cpp
    src/AtlasUI/Widgets/MenuBar.cpp
    src/AtlasUI/Widgets/StatusBar.cpp
    src/AtlasUI/Widgets/RadioButton.cpp
    src/AtlasUI/Widgets/Slider.cpp
    src/AtlasUI/Widgets/ProgressBar.cpp

    # AtlasUI — Commands
    src/AtlasUI/Commands/CommandTypes.cpp
    src/AtlasUI/Commands/CommandRegistry.cpp
    src/AtlasUI/Commands/CommandManager.cpp
    src/AtlasUI/Commands/ShortcutRouter.cpp
    src/AtlasUI/Commands/CommandDefaults.cpp

    # AtlasUI — Panels
    src/AtlasUI/Panels/InspectorPanel.cpp
    src/AtlasUI/Panels/HierarchyPanel.cpp
    src/AtlasUI/Panels/ContentBrowserPanel.cpp
    src/AtlasUI/Panels/ConsolePanel.cpp
    src/AtlasUI/Panels/IDEPanel.cpp
    src/AtlasUI/Panels/GraphEditorPanel.cpp
    src/AtlasUI/Panels/ViewportPanel.cpp
    src/AtlasUI/Panels/PipelineMonitorPanel.cpp

    # AtlasUI — Services
    src/AtlasUI/Services/FocusService.cpp
    src/AtlasUI/Services/PopupHost.cpp
    src/AtlasUI/Services/TooltipService.cpp
    src/AtlasUI/Services/NotificationHost.cpp
)
target_include_directories(NF_UI PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(NF_UI PUBLIC NF::Core NF::Renderer PRIVATE NF_CompileOptions)
add_library(NF::UI ALIAS NF_UI)
