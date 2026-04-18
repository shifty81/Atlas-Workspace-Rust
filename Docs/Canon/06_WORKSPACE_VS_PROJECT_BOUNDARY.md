# Workspace vs Project Boundary

## Core Principle

**Workspace core is generic. Projects own game meaning.**

## What Belongs in Workspace Core

- Window chrome, theming, layout
- Dock/tab system
- Tool launching and registry
- Panel framework
- Command routing
- Notification system
- Settings system
- File intake pipeline
- AtlasAI host panel
- Build pipeline framework
- Asset browser framework
- Graph editor framework
- Property editor framework
- Table editor framework
- Viewport host framework
- Validation framework
- Save/load/layout persistence
- Project adapter contracts

## What Belongs in Project Adapter (e.g., NovaForge)

- Economy formulas
- Inventory slot rules
- Loot tables
- Research/unlock logic
- Mission categories
- Faction relationships
- Rig/loadout presets
- Character progression data
- Project-specific crafting chains
- Project-specific world generation rules
- Project-specific asset categories
- Project-specific validators

## Adapter Model

Projects extend workspace through adapter contracts:

1. `IGameProjectAdapter` — project identification and extension registration
2. `IProjectPanelFactory` — project-specific panel creation
3. `IGameplaySystemPanelProvider` — gameplay system panel enumeration
4. `IGameplaySystemSchemaProvider` — project-specific schemas and validation

## Rules

- No NovaForge-specific gameplay logic in workspace core
- Workspace stays generic editor host
- Project adapters/plugins own project-specific behavior
- Project-specific panels are hosted under broader generic tools
- Projects are logically detachable from the monorepo
