# NovaForge Editor Adapter

This directory contains the NovaForge project adapter for Atlas Workspace.

## Purpose

The adapter registers NovaForge-specific gameplay system panels with the
workspace, allowing them to be hosted inside generic workspace tools
(primarily the Project Systems Tool).

## Layout

```
EditorAdapter/
  include/NovaForge/EditorAdapter/
    NovaForgeAdapter.h         — Main project adapter implementation
  src/
    (future .cpp files for complex panel implementations)
```

## Panels Provided

| Panel ID | Display Name | Category | Host Tool |
|----------|-------------|----------|-----------|
| novaforge.economy | Economy | Gameplay | workspace.project_systems |
| novaforge.inventory_rules | Inventory Rules | Gameplay | workspace.project_systems |
| novaforge.shop | Shop | Gameplay | workspace.project_systems |
| novaforge.mission_rules | Mission Rules | Gameplay | workspace.project_systems |
| novaforge.progression | Progression | Gameplay | workspace.project_systems |
| novaforge.character_rules | Character Rules | Gameplay | workspace.project_systems |

## Rules

- This adapter is the only way NovaForge extends workspace tooling
- No NovaForge-specific logic should live in workspace core
- Panels are hosted under `workspace.project_systems`, not as standalone editors
- See `Docs/Canon/06_WORKSPACE_VS_PROJECT_BOUNDARY.md` for boundary rules
