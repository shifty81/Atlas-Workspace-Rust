#include "core/Engine.h"
#include "core/EnginePhase.h"
#include "core/Logger.h"
#include "ui/UIScreenGraph.h"
#include "ui/UIConstants.h"
#include "ui/GUIDSLParser.h"
#include "ui/DefaultEditorLayout.h"
#include "ui/EditorAttachProtocol.h"
#include "ui/EditorTheme.h"
#include "ui/FontBootstrap.h"
#include "ui/DiagnosticsOverlay.h"
#include "ui/LauncherScreen.h"
#include "tools/PlayInEditor.h"
#include "tools/IToolModule.h"
#include "tools/ToolingLayer.h"
#include <filesystem>
#include <iostream>

/// Names of panels that are non-closable (always present in the dock tree).
static const char* kMainPanels[] = {
    "AssetBrowser",
    "Viewport",
    "Inspector",
    "ReplayTimeline",
    "Console",
    "Permissions",
    "DeterminismStatus",
};

/// Recursively build widgets from a parsed DSL node tree.
static void ApplyDSLNode(atlas::ui::UIScreen& screen,
                         const atlas::ui::DSLNode& node,
                         uint32_t parentWidget,
                         int x, int y, int w, int h) {
    switch (node.type) {
    case atlas::ui::DSLNodeType::Panel: {
        screen.AddWidget(atlas::ui::UIWidgetType::Panel, node.name, x, y, w, h);
        break;
    }
    case atlas::ui::DSLNodeType::Split: {
        if (node.children.size() < 2) break;
        float ratio = node.ratio;
        if (node.splitDir == atlas::ui::DSLSplitDir::Horizontal) {
            int leftW = static_cast<int>(w * ratio);
            int rightW = w - leftW;
            ApplyDSLNode(screen, *node.children[0], parentWidget, x, y, leftW, h);
            ApplyDSLNode(screen, *node.children[1], parentWidget, x + leftW, y, rightW, h);
        } else {
            int topH = static_cast<int>(h * ratio);
            int botH = h - topH;
            ApplyDSLNode(screen, *node.children[0], parentWidget, x, y, w, topH);
            ApplyDSLNode(screen, *node.children[1], parentWidget, x, y + topH, w, botH);
        }
        break;
    }
    case atlas::ui::DSLNodeType::Dock: {
        float ratio = node.ratio;
        int dockX = x, dockY = y, dockW = w, dockH = h;
        switch (node.dockTarget) {
        case atlas::ui::DSLDockTarget::Right:
            dockW = static_cast<int>(w * ratio);
            dockX = x + w - dockW;
            break;
        case atlas::ui::DSLDockTarget::Left:
            dockW = static_cast<int>(w * ratio);
            break;
        case atlas::ui::DSLDockTarget::Bottom:
            dockH = static_cast<int>(h * ratio);
            dockY = y + h - dockH;
            break;
        case atlas::ui::DSLDockTarget::Top:
            dockH = static_cast<int>(h * ratio);
            break;
        default: break;
        }
        for (auto& child : node.children)
            ApplyDSLNode(screen, *child, parentWidget, dockX, dockY, dockW, dockH);
        break;
    }
    case atlas::ui::DSLNodeType::Tabs: {
        for (auto& child : node.children)
            ApplyDSLNode(screen, *child, parentWidget, x, y, w, h);
        break;
    }
    case atlas::ui::DSLNodeType::Layout: {
        for (auto& child : node.children)
            ApplyDSLNode(screen, *child, parentWidget, x, y, w, h);
        break;
    }
    }
}

/// Resolve the projects directory by searching up from the current working
/// directory for a parent that contains both CMakeLists.txt and a projects/
/// subdirectory.  Falls back to "projects" if no such parent is found.
static std::string ResolveProjectsDir() {
    std::error_code ec;
    auto cwd = std::filesystem::current_path(ec);
    if (ec) return "projects";

    // Walk up from cwd looking for the repo root.
    for (auto dir = cwd; dir.has_parent_path(); dir = dir.parent_path()) {
        if (std::filesystem::exists(dir / "CMakeLists.txt", ec) &&
            std::filesystem::is_directory(dir / "projects", ec)) {
            return (dir / "projects").string();
        }
        if (dir == dir.parent_path()) break; // reached filesystem root
    }

    return "projects"; // fallback to relative path
}

/// Resolve the asset root directory similarly — look for an assets/ directory
/// next to CMakeLists.txt, and create it if it doesn't exist.
static std::string ResolveAssetRoot() {
    std::error_code ec;
    auto cwd = std::filesystem::current_path(ec);
    if (ec) return "assets";

    for (auto dir = cwd; dir.has_parent_path(); dir = dir.parent_path()) {
        if (std::filesystem::exists(dir / "CMakeLists.txt", ec)) {
            auto assetDir = dir / "assets";
            std::filesystem::create_directories(assetDir, ec);
            return assetDir.string();
        }
        if (dir == dir.parent_path()) break;
    }

    return "assets";
}

/// Widget IDs returned from BuildEditorUI for manager setup.
struct EditorWidgetIds {
    uint32_t toolbar;
    uint32_t tbPlay, tbPause, tbStop;
    uint32_t tabBar, tabScene, tabGame;
    uint32_t scenePanel, gamePanel;
    uint32_t assetScroll, entityScroll, consoleScroll;
    uint32_t consoleInput;
    uint32_t dockArea;
    uint32_t tbSaveBtn;
    // Console tab bar and System tab
    uint32_t consoleTabBar;
    uint32_t consoleTab, systemTab;
    uint32_t consoleContentPanel, systemContentPanel;
    uint32_t systemScroll;
    // Panels toggled by View menu
    uint32_t leftPanel;      // Asset Browser
    uint32_t rightPanel;     // Inspector
    uint32_t bottomPanel;    // Console area
    // Status bar
    uint32_t statusBar;
    // Menu item IDs for action wiring
    uint32_t fileNew, fileOpen, fileSave, fileExit;
    uint32_t editUndo, editRedo, editCut, editCopy, editPaste;
    uint32_t viewAssets, viewInspector, viewConsole;
    uint32_t toolsGraphEditor, toolsWorldGen, toolsProfiler, toolsSettings;
    uint32_t helpDocs, helpAbout;
};

static EditorWidgetIds BuildEditorUI(atlas::ui::UIScreen& screen) {
    // Self-hosting: parse the default editor DSL to build the initial
    // widget tree.  If DSL parsing fails, fall back to a hard-coded layout.
    atlas::ui::GUIDSLParser parser;
    auto result = parser.Parse(atlas::editor::DefaultEditorDSL());

    // Apply editor theme colors to the hard-coded fallback widgets.
    atlas::editor::EditorTheme theme = atlas::editor::EditorTheme::Dark();
    (void)theme; // Theme values are used by the renderer; stored for future use.

    if (!result.success) {
        std::cerr << "DSL parse error: " << result.errorMessage << std::endl;
    }

    // NOTE: When DSL parsing succeeds, ApplyDSLNode only creates empty Panel
    // containers (borders with no widget content).  Until the DSL pipeline is
    // extended to populate panels with menus, buttons, text, and other
    // widgets, we fall through to the hard-coded layout below which provides
    // the full editor UI.

    // Fallback: hard-coded layout
    // Menu bar
    uint32_t menuBar = screen.AddWidget(atlas::ui::UIWidgetType::Panel, "MenuBar",
                                         0, 0, 1280, 28);

    // File menu
    uint32_t fileMenu = screen.AddWidget(atlas::ui::UIWidgetType::Menu, "File",
                                          4, 2, 50, 24);
    screen.SetParent(fileMenu, menuBar);

    uint32_t fileNew = screen.AddWidget(atlas::ui::UIWidgetType::MenuItem, "New",
                                         4, 28, 120, 24);
    screen.SetParent(fileNew, fileMenu);

    uint32_t fileOpen = screen.AddWidget(atlas::ui::UIWidgetType::MenuItem, "Open",
                                          4, 52, 120, 24);
    screen.SetParent(fileOpen, fileMenu);

    uint32_t fileSave = screen.AddWidget(atlas::ui::UIWidgetType::MenuItem, "Save",
                                          4, 76, 120, 24);
    screen.SetParent(fileSave, fileMenu);

    uint32_t fileSep1 = screen.AddWidget(atlas::ui::UIWidgetType::MenuItem, "",
                                          4, 100, 120, 8);
    screen.SetParent(fileSep1, fileMenu);
    screen.SetSeparator(fileSep1, true);

    uint32_t fileExit = screen.AddWidget(atlas::ui::UIWidgetType::MenuItem, "Exit",
                                          4, 108, 120, 24);
    screen.SetParent(fileExit, fileMenu);

    // Edit menu
    uint32_t editMenu = screen.AddWidget(atlas::ui::UIWidgetType::Menu, "Edit",
                                          58, 2, 50, 24);
    screen.SetParent(editMenu, menuBar);

    uint32_t editUndo = screen.AddWidget(atlas::ui::UIWidgetType::MenuItem, "Undo",
                                          58, 28, 120, 24);
    screen.SetParent(editUndo, editMenu);

    uint32_t editRedo = screen.AddWidget(atlas::ui::UIWidgetType::MenuItem, "Redo",
                                          58, 52, 120, 24);
    screen.SetParent(editRedo, editMenu);

    uint32_t editSep1 = screen.AddWidget(atlas::ui::UIWidgetType::MenuItem, "",
                                          58, 76, 120, 8);
    screen.SetParent(editSep1, editMenu);
    screen.SetSeparator(editSep1, true);

    uint32_t editCut = screen.AddWidget(atlas::ui::UIWidgetType::MenuItem, "Cut",
                                         58, 84, 120, 24);
    screen.SetParent(editCut, editMenu);

    uint32_t editCopy = screen.AddWidget(atlas::ui::UIWidgetType::MenuItem, "Copy",
                                          58, 108, 120, 24);
    screen.SetParent(editCopy, editMenu);

    uint32_t editPaste = screen.AddWidget(atlas::ui::UIWidgetType::MenuItem, "Paste",
                                           58, 132, 120, 24);
    screen.SetParent(editPaste, editMenu);

    // View menu
    uint32_t viewMenu = screen.AddWidget(atlas::ui::UIWidgetType::Menu, "View",
                                          112, 2, 50, 24);
    screen.SetParent(viewMenu, menuBar);

    uint32_t viewAssets = screen.AddWidget(atlas::ui::UIWidgetType::MenuItem, "Asset Browser",
                                            112, 28, 140, 24);
    screen.SetParent(viewAssets, viewMenu);

    uint32_t viewInspector = screen.AddWidget(atlas::ui::UIWidgetType::MenuItem, "Inspector",
                                               112, 52, 140, 24);
    screen.SetParent(viewInspector, viewMenu);

    uint32_t viewConsole = screen.AddWidget(atlas::ui::UIWidgetType::MenuItem, "Console",
                                             112, 76, 140, 24);
    screen.SetParent(viewConsole, viewMenu);

    // Tools menu
    uint32_t toolsMenu = screen.AddWidget(atlas::ui::UIWidgetType::Menu, "Tools",
                                           166, 2, 55, 24);
    screen.SetParent(toolsMenu, menuBar);

    uint32_t toolsGraphEditor = screen.AddWidget(atlas::ui::UIWidgetType::MenuItem, "Graph Editor",
                                                  166, 28, 140, 24);
    screen.SetParent(toolsGraphEditor, toolsMenu);

    uint32_t toolsWorldGen = screen.AddWidget(atlas::ui::UIWidgetType::MenuItem, "World Generator",
                                               166, 52, 140, 24);
    screen.SetParent(toolsWorldGen, toolsMenu);

    uint32_t toolsProfiler = screen.AddWidget(atlas::ui::UIWidgetType::MenuItem, "Profiler",
                                               166, 76, 140, 24);
    screen.SetParent(toolsProfiler, toolsMenu);

    uint32_t toolsSep1 = screen.AddWidget(atlas::ui::UIWidgetType::MenuItem, "",
                                           166, 100, 140, 8);
    screen.SetParent(toolsSep1, toolsMenu);
    screen.SetSeparator(toolsSep1, true);

    uint32_t toolsSettings = screen.AddWidget(atlas::ui::UIWidgetType::MenuItem, "Settings",
                                               166, 108, 140, 24);
    screen.SetParent(toolsSettings, toolsMenu);

    // Help menu
    uint32_t helpMenu = screen.AddWidget(atlas::ui::UIWidgetType::Menu, "Help",
                                          225, 2, 50, 24);
    screen.SetParent(helpMenu, menuBar);

    uint32_t helpDocs = screen.AddWidget(atlas::ui::UIWidgetType::MenuItem, "Documentation",
                                          225, 28, 140, 24);
    screen.SetParent(helpDocs, helpMenu);
    screen.SetShortcutLabel(helpDocs, "F1");

    uint32_t helpSep1 = screen.AddWidget(atlas::ui::UIWidgetType::MenuItem, "",
                                          225, 52, 140, 8);
    screen.SetParent(helpSep1, helpMenu);
    screen.SetSeparator(helpSep1, true);

    uint32_t helpAbout = screen.AddWidget(atlas::ui::UIWidgetType::MenuItem, "About Atlas",
                                           225, 60, 140, 24);
    screen.SetParent(helpAbout, helpMenu);

    // Keyboard shortcuts
    screen.SetShortcutLabel(fileNew, "Ctrl+N");
    screen.SetShortcutLabel(fileOpen, "Ctrl+O");
    screen.SetShortcutLabel(fileSave, "Ctrl+S");
    screen.SetShortcutLabel(editUndo, "Ctrl+Z");
    screen.SetShortcutLabel(editRedo, "Ctrl+Y");
    screen.SetShortcutLabel(editCut, "Ctrl+X");
    screen.SetShortcutLabel(editCopy, "Ctrl+C");
    screen.SetShortcutLabel(editPaste, "Ctrl+V");

    // View menu items are toggleable
    screen.SetCheckable(viewAssets, true);
    screen.SetChecked(viewAssets, true);
    screen.SetCheckable(viewInspector, true);
    screen.SetChecked(viewInspector, true);
    screen.SetCheckable(viewConsole, true);
    screen.SetChecked(viewConsole, true);

    // --- Toolbar below menu bar ---
    uint32_t toolbar = screen.AddWidget(atlas::ui::UIWidgetType::Toolbar, "MainToolbar",
                                         0, 28, 1280, 30);

    uint32_t tbPlay = screen.AddWidget(atlas::ui::UIWidgetType::Button, "Play",
                                        4, 31, 50, 24);
    screen.SetParent(tbPlay, toolbar);

    uint32_t tbPause = screen.AddWidget(atlas::ui::UIWidgetType::Button, "Pause",
                                         58, 31, 50, 24);
    screen.SetParent(tbPause, toolbar);

    uint32_t tbStop = screen.AddWidget(atlas::ui::UIWidgetType::Button, "Stop",
                                        112, 31, 50, 24);
    screen.SetParent(tbStop, toolbar);

    uint32_t tbSaveBtn = screen.AddWidget(atlas::ui::UIWidgetType::Button, "Save",
                                           166, 31, 50, 24);
    screen.SetParent(tbSaveBtn, toolbar);

    // Left panel — Project / Asset Browser
    uint32_t dockArea = screen.AddWidget(atlas::ui::UIWidgetType::DockArea, "MainDock",
                                          0, 60, 1280, 630);

    uint32_t leftPanel = screen.AddWidget(atlas::ui::UIWidgetType::Panel, "AssetBrowser",
                                           0, 60, 260, 630);
    screen.SetParent(leftPanel, dockArea);

    uint32_t assetTitle = screen.AddWidget(atlas::ui::UIWidgetType::Text, "Asset Browser",
                                            4, 64, 252, 20);
    screen.SetParent(assetTitle, leftPanel);

    uint32_t assetScroll = screen.AddWidget(atlas::ui::UIWidgetType::ScrollView, "AssetScroll",
                                             4, 86, 252, 540);
    screen.SetParent(assetScroll, leftPanel);

    uint32_t assetList = screen.AddWidget(atlas::ui::UIWidgetType::List, "Assets",
                                           4, 86, 252, 540);
    screen.SetParent(assetList, assetScroll);

    // Center area — Tab bar + Viewport panels
    uint32_t tabBar = screen.AddWidget(atlas::ui::UIWidgetType::Panel, "ViewportTabs",
                                        262, 60, 756, 28);

    uint32_t tabScene = screen.AddWidget(atlas::ui::UIWidgetType::Tab, "Scene",
                                          264, 62, 80, 24);
    screen.SetParent(tabScene, tabBar);
    screen.SetChecked(tabScene, true);

    uint32_t tabGame = screen.AddWidget(atlas::ui::UIWidgetType::Tab, "Game",
                                         348, 62, 80, 24);
    screen.SetParent(tabGame, tabBar);

    // Scene viewport panel (visible by default)
    uint32_t scenePanel = screen.AddWidget(atlas::ui::UIWidgetType::Panel, "SceneViewport",
                                            262, 88, 756, 422);

    uint32_t vpTitle = screen.AddWidget(atlas::ui::UIWidgetType::Text, "Scene Viewport",
                                         266, 92, 748, 20);
    screen.SetParent(vpTitle, scenePanel);

    // Game viewport panel (hidden by default)
    uint32_t gamePanel = screen.AddWidget(atlas::ui::UIWidgetType::Panel, "GameViewport",
                                           262, 88, 756, 422);
    screen.SetVisible(gamePanel, false);

    uint32_t gameTitle = screen.AddWidget(atlas::ui::UIWidgetType::Text, "Game Preview",
                                           266, 92, 748, 20);
    screen.SetParent(gameTitle, gamePanel);

    // Right panel — Inspector
    uint32_t rightPanel = screen.AddWidget(atlas::ui::UIWidgetType::Panel, "Inspector",
                                            1020, 60, 260, 630);

    uint32_t inspTitle = screen.AddWidget(atlas::ui::UIWidgetType::Text, "ECS Inspector",
                                           1024, 64, 252, 20);
    screen.SetParent(inspTitle, rightPanel);

    uint32_t entityScroll = screen.AddWidget(atlas::ui::UIWidgetType::ScrollView, "EntityScroll",
                                              1024, 86, 252, 540);
    screen.SetParent(entityScroll, rightPanel);

    uint32_t entityList = screen.AddWidget(atlas::ui::UIWidgetType::List, "Entities",
                                            1024, 86, 252, 540);
    screen.SetParent(entityList, entityScroll);

    // Bottom panel — Console / System tabs
    uint32_t bottomPanel = screen.AddWidget(atlas::ui::UIWidgetType::Panel, "ConsoleArea",
                                             262, 512, 756, 178);

    // Tab bar for Console / System
    uint32_t consoleTabBar = screen.AddWidget(atlas::ui::UIWidgetType::Panel, "ConsoleTabBar",
                                               262, 512, 756, 26);
    screen.SetParent(consoleTabBar, bottomPanel);

    uint32_t consoleTab = screen.AddWidget(atlas::ui::UIWidgetType::Tab, "Console",
                                            264, 514, 80, 22);
    screen.SetParent(consoleTab, consoleTabBar);
    screen.SetChecked(consoleTab, true);

    uint32_t systemTab = screen.AddWidget(atlas::ui::UIWidgetType::Tab, "System",
                                           348, 514, 80, 22);
    screen.SetParent(systemTab, consoleTabBar);

    // Console content panel (visible by default)
    uint32_t consoleContentPanel = screen.AddWidget(atlas::ui::UIWidgetType::Panel, "ConsoleContent",
                                                     262, 538, 756, 152);
    screen.SetParent(consoleContentPanel, bottomPanel);

    uint32_t consoleScroll = screen.AddWidget(atlas::ui::UIWidgetType::ScrollView, "ConsoleScroll",
                                               266, 540, 748, 124);
    screen.SetParent(consoleScroll, consoleContentPanel);

    uint32_t consoleInput = screen.AddWidget(atlas::ui::UIWidgetType::InputField, "command...",
                                              266, 664, 748, 24);
    screen.SetParent(consoleInput, consoleContentPanel);

    // System content panel (hidden by default — debug log output)
    uint32_t systemContentPanel = screen.AddWidget(atlas::ui::UIWidgetType::Panel, "SystemContent",
                                                    262, 538, 756, 152);
    screen.SetParent(systemContentPanel, bottomPanel);
    screen.SetVisible(systemContentPanel, false);

    uint32_t systemTitle = screen.AddWidget(atlas::ui::UIWidgetType::Text, "System Debug Log",
                                             266, 540, 748, 18);
    screen.SetParent(systemTitle, systemContentPanel);

    uint32_t systemScroll = screen.AddWidget(atlas::ui::UIWidgetType::ScrollView, "SystemScroll",
                                              266, 560, 748, 128);
    screen.SetParent(systemScroll, systemContentPanel);

    // Status bar at the bottom
    uint32_t statusBar = screen.AddWidget(atlas::ui::UIWidgetType::StatusBar, "Ready",
                                           0, 692, 1280, 28);

    // Store tab/panel IDs for manager setup
    EditorWidgetIds ids{};
    ids.toolbar = toolbar;
    ids.tbPlay = tbPlay; ids.tbPause = tbPause; ids.tbStop = tbStop;
    ids.tabBar = tabBar; ids.tabScene = tabScene; ids.tabGame = tabGame;
    ids.scenePanel = scenePanel; ids.gamePanel = gamePanel;
    ids.assetScroll = assetScroll; ids.entityScroll = entityScroll; ids.consoleScroll = consoleScroll;
    ids.consoleInput = consoleInput; ids.dockArea = dockArea; ids.tbSaveBtn = tbSaveBtn;
    ids.consoleTabBar = consoleTabBar; ids.consoleTab = consoleTab; ids.systemTab = systemTab;
    ids.consoleContentPanel = consoleContentPanel; ids.systemContentPanel = systemContentPanel;
    ids.systemScroll = systemScroll;
    ids.leftPanel = leftPanel; ids.rightPanel = rightPanel; ids.bottomPanel = bottomPanel;
    ids.statusBar = statusBar;
    ids.fileNew = fileNew; ids.fileOpen = fileOpen; ids.fileSave = fileSave; ids.fileExit = fileExit;
    ids.editUndo = editUndo; ids.editRedo = editRedo;
    ids.editCut = editCut; ids.editCopy = editCopy; ids.editPaste = editPaste;
    ids.viewAssets = viewAssets; ids.viewInspector = viewInspector; ids.viewConsole = viewConsole;
    ids.toolsGraphEditor = toolsGraphEditor; ids.toolsWorldGen = toolsWorldGen;
    ids.toolsProfiler = toolsProfiler; ids.toolsSettings = toolsSettings;
    ids.helpDocs = helpDocs; ids.helpAbout = helpAbout;
    return ids;
}

// ============================================================
// Helper: populate Asset Browser scroll view with project entries
// ============================================================
static void PopulateAssetBrowser(atlas::Engine& engine,
                                 const EditorWidgetIds& ids,
                                 const atlas::editor::LauncherScreen& launcher,
                                 const std::string& projectsDir,
                                 const std::string& assetRoot) {
    auto& screen = engine.GetUIManager().GetScreen();
    const atlas::ui::UIWidget* scrollW = screen.GetWidget(ids.assetScroll);
    if (!scrollW || projectsDir.empty()) return;

    std::error_code ec;
    float entryY = scrollW->y + 4.0f;
    for (auto& proj : launcher.Projects()) {
        uint32_t entry = screen.AddWidget(atlas::ui::UIWidgetType::Text,
            "[Project] " + proj.name,
            scrollW->x + 8.0f, entryY,
            scrollW->width - 16.0f, 20.0f);
        screen.SetParent(entry, ids.assetScroll);
        entryY += 22.0f;
    }
    if (std::filesystem::is_directory(assetRoot, ec)) {
        for (auto& entry : std::filesystem::directory_iterator(assetRoot, ec)) {
            std::string name = entry.path().filename().string();
            std::string prefix = entry.is_directory(ec) ? "[Dir] " : "[File] ";
            uint32_t wid = screen.AddWidget(atlas::ui::UIWidgetType::Text,
                prefix + name,
                scrollW->x + 8.0f, entryY,
                scrollW->width - 16.0f, 20.0f);
            screen.SetParent(wid, ids.assetScroll);
            entryY += 22.0f;
        }
    }
}

// ============================================================
// Helper: configure Tab and Scroll managers
// ============================================================
static void SetupTabsAndScrolling(atlas::Engine& engine,
                                  const EditorWidgetIds& ids) {
    auto& tabMgr = engine.GetUIManager().GetTabManager();
    if (ids.tabScene != 0) {
        tabMgr.SetTabContent(ids.tabScene, ids.scenePanel);
        tabMgr.SetTabContent(ids.tabGame, ids.gamePanel);
    }
    if (ids.consoleTab != 0) {
        tabMgr.SetTabContent(ids.consoleTab, ids.consoleContentPanel);
        tabMgr.SetTabContent(ids.systemTab, ids.systemContentPanel);
    }

    auto& scrollMgr = engine.GetUIManager().GetScrollManager();
    if (ids.assetScroll != 0) {
        scrollMgr.RegisterScrollView(ids.assetScroll, 1000.0f);
        scrollMgr.RegisterScrollView(ids.entityScroll, 800.0f);
        scrollMgr.RegisterScrollView(ids.consoleScroll, 500.0f);
    }
    if (ids.systemScroll != 0) {
        scrollMgr.RegisterScrollView(ids.systemScroll, 2000.0f);
    }
}

// ============================================================
// Helper: install logger sink that feeds Console and System scroll areas
// ============================================================
static void SetupLoggerSink(atlas::Engine& engine,
                            const EditorWidgetIds& ids) {
    atlas::ui::UIScreen* screenPtr = &engine.GetUIManager().GetScreen();
    uint32_t sysScrollId = ids.systemScroll;
    uint32_t consScrollId = ids.consoleScroll;
    atlas::Logger::SetSink([screenPtr, sysScrollId, consScrollId](const std::string& line) {
        const atlas::ui::UIWidget* scrollW = screenPtr->GetWidget(sysScrollId);
        if (scrollW) {
            auto children = screenPtr->GetChildren(sysScrollId);
            float baseY = scrollW->y + 2.0f;
            float lineY = baseY + static_cast<float>(children.size()) * atlas::ui::kLogLineSpacing;
            uint32_t textId = screenPtr->AddWidget(
                atlas::ui::UIWidgetType::Text, line,
                scrollW->x + 4.0f, lineY,
                scrollW->width - 8.0f, atlas::ui::kLogEntryHeight);
            screenPtr->SetParent(textId, sysScrollId);
        }
        const atlas::ui::UIWidget* consW = screenPtr->GetWidget(consScrollId);
        if (consW) {
            auto children = screenPtr->GetChildren(consScrollId);
            float baseY = consW->y + 2.0f;
            float lineY = baseY + static_cast<float>(children.size()) * atlas::ui::kLogLineSpacing;
            uint32_t textId = screenPtr->AddWidget(
                atlas::ui::UIWidgetType::Text, line,
                consW->x + 4.0f, lineY,
                consW->width - 8.0f, atlas::ui::kLogEntryHeight);
            screenPtr->SetParent(textId, consScrollId);
        }
    });
}

// ============================================================
// Helper: wire toolbar buttons (Play / Pause / Stop / Save)
// ============================================================
static void SetupToolbar(atlas::Engine& engine,
                         const EditorWidgetIds& ids,
                         atlas::editor::PlayInEditor& playInEditor,
                         std::function<void(const std::string&)> updateStatus) {
    engine.GetUIManager().GetToolbarManager().SetButtonCallback(
        [&engine, &ids, &playInEditor, updateStatus](uint32_t /*toolbarId*/, uint32_t buttonId) {
            if (buttonId == ids.tbPlay) {
                if (playInEditor.Mode() == atlas::editor::PIEMode::Paused) {
                    playInEditor.Resume();
                    atlas::Logger::Info("Simulation resumed");
                    updateStatus("Simulating...");
                } else if (playInEditor.Mode() == atlas::editor::PIEMode::Stopped) {
                    playInEditor.StartSimulation(engine);
                    atlas::Logger::Info("Simulation started");
                    updateStatus("Simulating...");
                }
            } else if (buttonId == ids.tbPause) {
                if (playInEditor.Mode() == atlas::editor::PIEMode::Simulating) {
                    playInEditor.Pause();
                    atlas::Logger::Info("Simulation paused");
                    updateStatus("Paused");
                }
            } else if (buttonId == ids.tbStop) {
                if (playInEditor.Mode() != atlas::editor::PIEMode::Stopped) {
                    playInEditor.StopSimulation(engine);
                    atlas::Logger::Info("Simulation stopped");
                    updateStatus("Ready");
                }
            } else if (buttonId == ids.tbSaveBtn) {
                atlas::Logger::Info("Project saved");
                updateStatus("Project saved");
            }
        }
    );
}

// ============================================================
// Helper: wire menu item callbacks (File / Edit / View / Tools / Help)
// ============================================================
static void SetupMenuCallbacks(atlas::Engine& engine,
                               const EditorWidgetIds& ids,
                               atlas::editor::PlayInEditor& playInEditor,
                               atlas::editor::LauncherScreen& launcher,
                               const std::string& projectsDir,
                               std::function<void(const std::string&)> updateStatus) {
    engine.GetUIManager().GetMenuManager().SetMenuItemCallback(
        [&engine, &ids, &playInEditor, updateStatus, &launcher, &projectsDir](uint32_t /*menuId*/, uint32_t itemId) {
            auto& screen = engine.GetUIManager().GetScreen();

            // File menu
            if (itemId == ids.fileNew) {
                atlas::Logger::Info("File > New Project");
                updateStatus("New project created");
            } else if (itemId == ids.fileOpen) {
                atlas::Logger::Info("File > Open Project — scanning " + projectsDir);
                launcher.ScanProjects(projectsDir);
                if (launcher.Projects().empty()) {
                    atlas::Logger::Warn("No projects found in " + projectsDir);
                } else {
                    for (size_t i = 0; i < launcher.Projects().size(); ++i) {
                        atlas::Logger::Info("  [" + std::to_string(i) + "] " + launcher.Projects()[i].name
                                            + " (" + launcher.Projects()[i].path + ")");
                    }
                    launcher.SelectProject(0);
                    launcher.ConfirmSelection();
                    if (auto* proj = launcher.SelectedProject()) {
                        atlas::Logger::Info("Opened project: " + proj->name);
                        updateStatus("Project: " + proj->name);
                    }
                }
            } else if (itemId == ids.fileSave) {
                atlas::Logger::Info("File > Save Project");
                updateStatus("Project saved");
            } else if (itemId == ids.fileExit) {
                atlas::Logger::Info("File > Exit");
                engine.RequestExit();

            // Edit menu
            } else if (itemId == ids.editUndo) {
                atlas::Logger::Info("Edit > Undo");
                updateStatus("Undo");
            } else if (itemId == ids.editRedo) {
                atlas::Logger::Info("Edit > Redo");
                updateStatus("Redo");
            } else if (itemId == ids.editCut) {
                atlas::Logger::Info("Edit > Cut");
            } else if (itemId == ids.editCopy) {
                atlas::Logger::Info("Edit > Copy");
            } else if (itemId == ids.editPaste) {
                atlas::Logger::Info("Edit > Paste");

            // View menu (toggle panel visibility)
            } else if (itemId == ids.viewAssets) {
                const atlas::ui::UIWidget* w = screen.GetWidget(ids.leftPanel);
                if (w) {
                    bool newVis = !w->visible;
                    screen.SetVisible(ids.leftPanel, newVis);
                    atlas::Logger::Info(std::string("View > Asset Browser: ") + (newVis ? "shown" : "hidden"));
                    updateStatus(std::string("Asset Browser ") + (newVis ? "shown" : "hidden"));
                }
            } else if (itemId == ids.viewInspector) {
                const atlas::ui::UIWidget* w = screen.GetWidget(ids.rightPanel);
                if (w) {
                    bool newVis = !w->visible;
                    screen.SetVisible(ids.rightPanel, newVis);
                    atlas::Logger::Info(std::string("View > Inspector: ") + (newVis ? "shown" : "hidden"));
                    updateStatus(std::string("Inspector ") + (newVis ? "shown" : "hidden"));
                }
            } else if (itemId == ids.viewConsole) {
                const atlas::ui::UIWidget* w = screen.GetWidget(ids.bottomPanel);
                if (w) {
                    bool newVis = !w->visible;
                    screen.SetVisible(ids.bottomPanel, newVis);
                    atlas::Logger::Info(std::string("View > Console: ") + (newVis ? "shown" : "hidden"));
                    updateStatus(std::string("Console ") + (newVis ? "shown" : "hidden"));
                }

            // Tools menu
            } else if (itemId == ids.toolsGraphEditor) {
                atlas::Logger::Info("Tools > Graph Editor");
                updateStatus("Graph Editor");
            } else if (itemId == ids.toolsWorldGen) {
                atlas::Logger::Info("Tools > World Generator");
                updateStatus("World Generator");
            } else if (itemId == ids.toolsProfiler) {
                atlas::Logger::Info("Tools > Profiler");
                updateStatus("Profiler");
            } else if (itemId == ids.toolsSettings) {
                atlas::Logger::Info("Tools > Settings");
                updateStatus("Settings");

            // Help menu
            } else if (itemId == ids.helpDocs) {
                atlas::Logger::Info("Help > Documentation (F1)");
                updateStatus("Documentation");
            } else if (itemId == ids.helpAbout) {
                atlas::Logger::Info("Help > About Atlas Engine v0.1");
                updateStatus("About Atlas");
            }
        }
    );
}

// ============================================================
// Helper: wire console input field commands
// ============================================================
static void SetupConsoleInput(atlas::Engine& engine,
                              const EditorWidgetIds& ids,
                              atlas::editor::PlayInEditor& playInEditor,
                              std::function<void(const std::string&)> updateStatus) {
    auto& inputMgr = engine.GetUIManager().GetInputFieldManager();
    if (ids.consoleInput == 0) return;

    inputMgr.RegisterField(ids.consoleInput, "command...");
    inputMgr.SetTextSubmitCallback(
        [&engine, &playInEditor, updateStatus](uint32_t /*fieldId*/, const std::string& text) {
            if (text == "help") {
                atlas::Logger::Info("Available commands: help, clear, status, exit/quit, play, pause, stop");
            } else if (text == "exit" || text == "quit") {
                atlas::Logger::Info("Exit requested via console");
                engine.RequestExit();
            } else if (text == "status") {
                std::string mode;
                switch (playInEditor.Mode()) {
                    case atlas::editor::PIEMode::Stopped:    mode = "Stopped"; break;
                    case atlas::editor::PIEMode::Simulating: mode = "Simulating"; break;
                    case atlas::editor::PIEMode::Paused:     mode = "Paused"; break;
                    case atlas::editor::PIEMode::Possessed:  mode = "Possessed"; break;
                }
                atlas::Logger::Info("Status: PIE=" + mode
                                   + " Ticks=" + std::to_string(playInEditor.TicksSimulated()));
            } else if (text == "play") {
                if (playInEditor.Mode() == atlas::editor::PIEMode::Stopped) {
                    playInEditor.StartSimulation(engine);
                    atlas::Logger::Info("Simulation started via console");
                    updateStatus("Simulating...");
                } else if (playInEditor.Mode() == atlas::editor::PIEMode::Paused) {
                    playInEditor.Resume();
                    atlas::Logger::Info("Simulation resumed via console");
                    updateStatus("Simulating...");
                }
            } else if (text == "pause") {
                if (playInEditor.Mode() == atlas::editor::PIEMode::Simulating) {
                    playInEditor.Pause();
                    atlas::Logger::Info("Simulation paused via console");
                    updateStatus("Paused");
                }
            } else if (text == "stop") {
                if (playInEditor.Mode() != atlas::editor::PIEMode::Stopped) {
                    playInEditor.StopSimulation(engine);
                    atlas::Logger::Info("Simulation stopped via console");
                    updateStatus("Ready");
                }
            } else if (text == "clear") {
                atlas::Logger::Info("Console cleared");
            } else if (!text.empty()) {
                atlas::Logger::Info("Unknown command: " + text + " (type 'help' for available commands)");
            }
        }
    );
}

// ============================================================
// Helper: wire miscellaneous UI managers (Dock, Focus, Tooltip, etc.)
// ============================================================
static void SetupMiscManagers(atlas::Engine& engine,
                              const EditorWidgetIds& ids) {
    auto& dockMgr = engine.GetUIManager().GetDockManager();
    if (ids.dockArea != 0)
        dockMgr.RegisterDockArea(ids.dockArea);

    engine.GetUIManager().GetFocusManager().SetFocusChangedCallback(
        [](uint32_t newId, uint32_t oldId) {
            atlas::Logger::Info("Focus changed: " + std::to_string(oldId)
                                + " -> " + std::to_string(newId));
        }
    );

    auto& tooltipMgr = engine.GetUIManager().GetTooltipManager();
    if (ids.tbSaveBtn != 0) {
        auto& screen = engine.GetUIManager().GetScreen();
        uint32_t saveTip = screen.AddWidget(atlas::ui::UIWidgetType::Tooltip,
                                             "Save project (Ctrl+S)", 0, 0, 140, 20);
        tooltipMgr.SetTooltip(ids.tbSaveBtn, saveTip);
    }

    engine.GetUIManager().GetCheckboxManager().SetCheckboxChangedCallback(
        [](uint32_t widgetId, bool checked) {
            atlas::Logger::Info("Checkbox toggled: widget=" + std::to_string(widgetId)
                                + " checked=" + std::to_string(checked));
        }
    );

    engine.GetUIManager().GetTreeNodeManager().SetTreeNodeToggledCallback(
        [](uint32_t widgetId, bool expanded) {
            atlas::Logger::Info("TreeNode toggled: widget=" + std::to_string(widgetId)
                                + " expanded=" + std::to_string(expanded));
        }
    );

    engine.GetUIManager().GetSplitterManager().SetSplitterMovedCallback(
        [](uint32_t widgetId, float position) {
            atlas::Logger::Info("Splitter moved: widget=" + std::to_string(widgetId)
                                + " position=" + std::to_string(position));
        }
    );

    engine.GetUIManager().GetColorPickerManager().SetColorChangedCallback(
        [](uint32_t widgetId, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
            atlas::Logger::Info("Color changed: widget=" + std::to_string(widgetId)
                                + " rgba=(" + std::to_string(r) + "," + std::to_string(g)
                                + "," + std::to_string(b) + "," + std::to_string(a) + ")");
        }
    );
}

// ============================================================
// main — Editor entry point
// ============================================================

int main() {
    atlas::EngineConfig cfg;
    cfg.mode = atlas::EngineMode::Editor;

    atlas::Engine engine(cfg);
    engine.InitCore();
    engine.InitRender();
    engine.InitUI();
    engine.InitECS();
    engine.InitNetworking();
    engine.InitEditor();

    // --- Font bootstrap (must run after renderer init) ---
    std::string assetRoot = ResolveAssetRoot();
    atlas::ui::FontBootstrap fontBootstrap;
    fontBootstrap.Init(assetRoot);

    // Set up default attach mode (standalone)
    atlas::editor::EditorAttachProtocol attach;
    attach.Init();
    atlas::editor::AttachConfig attachCfg;
    attachCfg.mode = atlas::editor::AttachMode::Standalone;
    attach.Connect(attachCfg);

    // --- Launcher screen: scan for projects ---
    std::string projectsDir = ResolveProjectsDir();
    atlas::editor::LauncherScreen launcher;
    launcher.ScanProjects(projectsDir);

    if (launcher.IsQuitRequested()) {
        fontBootstrap.Shutdown();
        return 0;
    }

    // Auto-select first project if one was found
    if (!launcher.Projects().empty() && !launcher.IsProjectChosen()) {
        launcher.SelectProject(0);
        launcher.ConfirmSelection();
        if (auto* proj = launcher.SelectedProject()) {
            atlas::Logger::Info("Launcher: opening project '" + proj->name
                                + "' (" + proj->path + ")");
        }
    }

    auto ids = BuildEditorUI(engine.GetUIManager().GetScreen());

    // --- Tooling layer: activate for the editor session ---
    atlas::tools::ToolingLayer toolingLayer;
    toolingLayer.Activate();

    // --- Play-In-Editor controller ---
    atlas::editor::PlayInEditor playInEditor;

    // Status-bar update helper
    auto updateStatus = [&engine, &ids](const std::string& text) {
        if (ids.statusBar != 0) {
            atlas::ui::UIWidget* w = engine.GetUIManager().GetScreen().GetWidgetMutable(ids.statusBar);
            if (w) w->name = text;
        }
    };

    // Wire UI subsystems via extracted helpers
    PopulateAssetBrowser(engine, ids, launcher, projectsDir, assetRoot);
    SetupTabsAndScrolling(engine, ids);
    SetupLoggerSink(engine, ids);
    SetupToolbar(engine, ids, playInEditor, updateStatus);
    SetupMenuCallbacks(engine, ids, playInEditor, launcher, projectsDir, updateStatus);
    SetupConsoleInput(engine, ids, playInEditor, updateStatus);
    SetupMiscManagers(engine, ids);

    // Enable diagnostics overlay by default in editor
    atlas::ui::DiagnosticsOverlay::SetEnabled(true);

    engine.Run();

    toolingLayer.Registry().Clear();
    fontBootstrap.Shutdown();

    return 0;
}
