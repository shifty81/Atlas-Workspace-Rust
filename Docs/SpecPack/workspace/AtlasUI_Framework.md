# AtlasUI Framework Status

## Overview
AtlasUI is the canonical tooling UI framework. All editor/workspace UI
must use AtlasUI components — no raw ImGui, no raw Win32 controls.

## Existing Components ✅

### Widgets (Source/UI/include/NF/UI/AtlasUI/Widgets/)
- Button, Panel, TextInput, Dropdown
- TabBar, Tabs, Splitter, Toolbar
- Label, StackPanel, PropertyRow
- Tooltip, NotificationCard, Container

### Commands (Source/UI/include/NF/UI/AtlasUI/Commands/)
- CommandManager, CommandRegistry, CommandTypes
- ShortcutRouter, CommandDefaults

### Services (Source/UI/include/NF/UI/AtlasUI/Services/)
- FocusService, PopupHost, TooltipService, NotificationHost

### Theme (Source/UI/include/NF/UI/AtlasUI/Theme/)
- ThemeManager, ThemeDefaults, ThemeTokens

### Panels (Source/UI/include/NF/UI/AtlasUI/Panels/)
- InspectorPanel, HierarchyPanel, ContentBrowserPanel
- ConsolePanel, IDEPanel, GraphEditorPanel
- ViewportPanel, PipelineMonitorPanel

### Backends (Source/UI/include/NF/UI/)
- GDIBackend (Win32 production)
- OpenGLBackend (GPU-accelerated stub)
- ImGuiBackend (M1 stub)
- NullBackend (headless/testing)

## Missing Components (from gap matrix)

### Priority 1 — Framework Completion
- [ ] PropertyGrid widget (entity/component editing)
- [ ] TreeView widget (hierarchy, file trees)
- [ ] TableView widget (data grids, lists)
- [ ] ScrollView + virtualization (large data sets)
- [ ] ContextMenu widget (right-click menus)

### Priority 2 — Workspace Integration
- [ ] Layout persistence (save/restore panel arrangement)
- [ ] Viewport host contract (3D rendering surface)
- [ ] Graph host contract (node graph rendering surface)
- [ ] Typography enforcement (font stack, sizes)
- [ ] Iconography system (icon atlas, semantic icons)

### Priority 3 — Workflow
- [ ] File intake pipeline (drag-and-drop import)
- [ ] Notification severity/workflow rules
- [ ] Settings panel (app + project config)
- [ ] Search/filter bar widget
