# Atlas Workspace Shell

## Overview
The Atlas Workspace is the development environment that hosts NovaForge
and all tooling. It acts as a development OS — NovaForge is a hosted project.

## Architecture
```
Atlas Workspace (Shell)
├── AtlasUI Framework (widget kit, themes, commands)
├── Panel Host (docking, floating, tabs)
├── Notification Center
├── AtlasAI Broker (debugging, suggestions)
├── Codex System (knowledge base)
├── Settings Panel
└── Hosted Projects
    └── NovaForge (game + editor)
```

## Existing Implementation
- AtlasUI framework: 15+ widgets, themes, commands, services
- 8 AtlasUI panels (Viewport, Inspector, Hierarchy, Console, ContentBrowser, Graph, IDE, Pipeline)
- EditorApp with full command/undo system
- DockLayout with 5 slot positions
- Win32 GDI backend + GLFW/ImGui stub backend (M1)
- Pipeline module with tool adapters

## Missing (from SpecRollup audit)
- Workspace shell app registration
- Notification center workflow/severity rules
- AtlasAI panel host implementation
- Settings/control panel
- File intake/drop pipeline
- Layout persistence
- PropertyGrid / Tree / Table systems
- Scroll + virtualization
- Typography and iconography enforcement

## Strict Rules
1. Workspace ≠ Game — game logic never enters tooling layer
2. Game remains detachable project
3. All systems must be schema-driven
4. Editor validates what runtime enforces
