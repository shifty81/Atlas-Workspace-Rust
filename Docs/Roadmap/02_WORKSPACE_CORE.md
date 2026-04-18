> **HISTORICAL** — This document describes a completed foundation phase (Phases 0–71).
> The canonical source of truth for current and future phases is [00_MASTER_ROADMAP.md](00_MASTER_ROADMAP.md).

# Workspace Core Stabilization

**Phase 1 of the Master Roadmap**

## Purpose

Make the workspace host bootstrap clean, deterministic, and generic.
WorkspaceShell is the "OS" layer — it owns every registry and manager
and acts as the composition root for all development tools.

## Tasks

### Entrypoint
- [x] Verify AtlasWorkspace.exe is the single primary entry
- [x] Clean stale naming in window titles, app IDs, command IDs
- [x] Remove stale WPF or MasterRepo assumptions
- [x] Ensure NovaForge is not hard-wired as workspace core boot logic

### App/Tool Registry
- [x] Verify child tools register through clean registry
- [x] Remove stale registrations (archived S-story stubs to Legacy/)
- [x] Document tool registration model (CoreToolRoster.h + Docs/Canon/)

### Workspace/Project Boundary
- [x] Verify no NovaForge-specific logic in workspace core
- [x] Define project adapter contract interfaces (IGameProjectAdapter.h)
- [x] Wire project adapter manager into workspace shell

### Shell Subsystems
- [x] WorkspaceShell (composition root, factory-based tool registration)
- [x] ToolRegistry (hosted primary tools with lifecycle)
- [x] PanelRegistry (shared panel factory + lifecycle)
- [x] LayoutManager (LayoutManagerV1, LayoutPersistenceManager)
- [x] CommandManager (ConsoleCommandBus — wired into WorkspaceShell)
- [x] SelectionManager (SelectionService — wired into WorkspaceShell)
- [x] NotificationCenter (NotificationSystem + WorkspaceShellContract)
- [ ] ProjectManager (full multi-project, open/recent UI — Phase 5)
- [x] ProjectAdapterManager (IGameProjectAdapter + loadProject/unloadProject)
- [x] AtlasAIHost (AtlasAIPanelHost, BrokerFlowController — Phase 4)
- [x] AssetIntakeService (FileIntakePipeline, AssetImportQueue)
- [x] LogConsoleService (LoggingRouteV1, BuildLogRouter — Phase 4)
- [x] EventBus (EditorEventBus — wired into WorkspaceShell)

### Build Quality
- [x] Fix C4100 warnings in LayoutPersistence.h and Scripting.h
- [x] Fix C4458 warnings in WidgetKitV1.h (id parameter hides class member)
- [x] Reduce heavy boilerplate includes in ConsoleCommandBus.h, EditorEventBus.h, SelectionService.h
- [x] Phase 2 tests: ConsoleCommandBus, SelectionService, EditorEventBus, WorkspaceShell accessors
