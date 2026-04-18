# Atlas Workspace

Atlas Workspace is a native C++ workspace platform for building games, tools, and development pipelines. It provides a unified host for editors, build systems, AtlasAI workflows, and project orchestration.

Atlas Workspace is a **generic host environment**. Game projects such as NovaForge are developed inside it but do not define the workspace core.

---

## Current Status

Active phase: **Milestone 0 – Canon Reset and Consolidation**

The repository contains real structure and systems, but is undergoing cleanup and normalization before further expansion.

---

## State Overview

### Done
- Workspace-first architecture defined
- Atlas Workspace is the primary executable direction
- AtlasUI selected as standard UI framework
- AtlasAI established as canonical AI broker name
- Monorepo structure in place
- Hosted project structure (NovaForge) present
- Broad spec and documentation coverage exists
- AtlasUI widget kit (19+ widgets), docking, theming, command layer

### In Progress
- Repo normalization and cleanup
- Active naming migration (Arbiter → AtlasAI)
- Documentation reset
- Workspace bootstrap cleanup
- Editor consolidation

### Partial
- UI backend strategy implementation (GDI active, D3D11 targeted)
- Shared panel system extraction
- AtlasAI/Codex workflow wiring
- Test/dependency cleanup
- Hosted-project gating

### Planned
- D3D11 UI backend
- DirectWrite text backend
- Final editor tool roster (~10 primary tools)
- Project/plugin loading system
- Release pipeline

### Archived / Deferred
- WPF shell direction
- ImGui tooling path
- Arbiter naming
- One-off editor expansion

---

## Current Roadmap

### 1. Cleanup and Normalization
- Line ending normalization (.gitattributes)
- Repo hygiene enforcement
- Active naming cleanup (Arbiter → AtlasAI)
- Doc canon reset
- Test dependency gating

### 2. Workspace Core Stabilization
- Clean host bootstrap (AtlasWorkspace.exe)
- Enforce workspace/project boundaries
- Tool/app registry cleanup

### 3. AtlasUI Backend Strategy
- GDI marked fallback only
- D3D11 target backend
- DirectWrite text path
- Legacy OpenGL/GLFW isolation

### 4. Editor Consolidation
- Reduce tool count to ~10 primary tools
- Convert one-off editors → shared panels/services
- Define shared systems and panel residency

### 5. AtlasAI Integration
- Broker flow formalization
- Build-log and error-handling routing
- Codex mirroring and snippet promotion

### 6. Hosted Project Support
- NovaForge as hosted project with adapter
- Build gating
- Plugin/project model

### 7. Build, Patch, and Release Pipeline
- Build presets and dependency policy
- Patch apply/remove workflow
- Packaging and release path

---

## Consolidation Still Required

### Naming
- Remove legacy naming in active paths (Arbiter, Atlas Suite, MasterRepo)
- User-facing strings must reflect Atlas Workspace canon

### Structural
- Line-ending normalization locked
- Generated artifacts excluded from source audits
- Active docs separated from historical archives

### Editor
- Primary editor roster trimmed to intended core tool set
- Shared panels replace duplicated editor-like surfaces
- Non-core one-off editors merged, deferred, or archived

### Backend
- AtlasUI backend selection formalized
- GDI stops acting like the practical default
- D3D11 + DirectWrite become the intended primary path

---

## Core Tool Direction

Target: ~10 primary tools supported by shared panels and services.

| Tool | Purpose |
|------|---------|
| Workspace Browser | Project and workspace navigation |
| Scene Editor | Scene/world editing and viewport |
| Asset Editor | Asset management and editing |
| Material Editor | Material/shader authoring |
| Visual Logic Editor | Blueprint/graph-based logic |
| UI Editor | UI layout and design |
| Animation Editor | Animation timeline and state |
| Data Editor | Data tables and configuration |
| Build Tool | Build pipeline and packaging |
| AtlasAI Tool | AI broker, diagnostics, Codex |
| Project Systems Tool | Hosted-project gameplay panels (via adapter) |

Everything else becomes a shared panel, shared service, mode, plugin, or archive.

---

## Known Drift

- Editor surface area overspread (~312 headers, target ~10 tools)
- Legacy naming residue (Arbiter in active paths)
- Backend mismatch (GDI active, D3D11 targeted)
- Stale docs reflecting older project states
- Generated artifact pollution in source tree

---

## Repo Layout

```
Source/          Core source for workspace, engine, UI, renderer, editor, and tooling
  Core/          Foundation types, logging, memory
  Engine/        Atlas Engine: ECS, behavior trees, asset system, scene graph
  Renderer/      Rendering subsystem
  Physics/       Physics subsystem
  Audio/         Audio subsystem
  Animation/     Animation subsystem
  Input/         Input subsystem
  Networking/    Networking subsystem
  UI/            AtlasUI framework
  GraphVM/       Visual graph/scripting VM
  AI/            AtlasAI broker
  Pipeline/      Build orchestration and workspace broker
  Editor/        Workspace editor host
  Programs/      Executable entrypoints
Docs/            Canon docs, roadmap docs, inventory docs, and archives
  Canon/         Locked project direction and policies
  Roadmap/       Phased execution plan
  Inventory/     Tool inventories, scrub lists, cleanup manifests
  Archive/       Historical material not considered active canon
NovaForge/       Hosted project under active development through Workspace
Tools/           Standalone or support tooling
Schemas/         Shared schema definitions (atlas.project.v1, atlas.build.v1)
Scripts/         Build, patch, and validation scripts
Tests/           Catch2 test suite
Project/         Project descriptors and config
Archive/         Historical material
```

---

## Building

```bash
# Configure workspace (tests off, no network fetch)
cmake --preset debug

# Configure with tests (requires network or local Catch2)
cmake -S . -B build -DATLAS_BUILD_TESTS=ON -DATLAS_ENABLE_ONLINE_DEPS=ON

# Build
cmake --build --preset debug --parallel

# Run tests (when enabled)
ctest --preset debug
```

---

## Documentation Index

### Canon
- [Project Status](Docs/Canon/00_PROJECT_STATUS.md)
- [Locked Direction](Docs/Canon/01_LOCKED_DIRECTION.md)
- [Naming Canon](Docs/Canon/03_NAMING_CANON.md)
- [UI Backend Strategy](Docs/Canon/04_UI_BACKEND_STRATEGY.md)
- [Editor Strategy](Docs/Canon/05_EDITOR_STRATEGY.md)
- [Workspace vs Project Boundary](Docs/Canon/06_WORKSPACE_VS_PROJECT_BOUNDARY.md)
- [Build and Dependency Policy](Docs/Canon/07_BUILD_AND_DEPENDENCY_POLICY.md)
- [Definition of Done](Docs/Canon/09_DEFINITION_OF_DONE.md)
- [Status Legend](Docs/Canon/10_REPO_STATUS_LEGEND.md)
- [Module Boundaries](Docs/Canon/11_MODULE_BOUNDARIES.md)

### Roadmap
- [Master Roadmap](Docs/Roadmap/00_MASTER_ROADMAP.md)
- [Cleanup and Normalization](Docs/Roadmap/01_CLEANUP_AND_NORMALIZATION.md)
- [Editor Consolidation](Docs/Roadmap/04_EDITOR_CONSOLIDATION.md)

### Inventory
- [Editor Tool Inventory](Docs/Inventory/EDITOR_TOOL_INVENTORY.md)
- [Panel and Service Matrix](Docs/Inventory/PANEL_AND_SERVICE_MATRIX.md)
- [Legacy Name Scrub List](Docs/Inventory/LEGACY_NAME_SCRUB_LIST.md)
- [Repo Cleanup Manifest](Docs/Inventory/REPO_CLEANUP_MANIFEST.md)

---

## Immediate Priority

1. Normalize the repo
2. Lock the documentation
3. Complete naming cleanup
4. Stabilize workspace host
5. Fix UI backend strategy
6. Consolidate editors

Feature expansion should wait until those are complete.

---

## Rules

- No feature expansion during cleanup milestone
- No new standalone editors without justification
- No stale naming in active paths
- No generated artifacts in repo snapshots
- No `.git` in audit zips
- Presence of a file does not equal completion

---

## Audit Rules

- No `.git/` in audit zips
- No `build/`, `.vs/`, `Binaries/`, `Intermediate/` in audit zips
- Generated artifacts excluded
- Current README and canon docs updated with major structural changes
