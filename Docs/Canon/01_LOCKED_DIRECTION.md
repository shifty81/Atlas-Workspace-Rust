# Locked Direction

These decisions are fixed. Do not revisit without explicit project-level review.

## Workspace

Atlas Workspace is a **generic development host**, not a game.

- `AtlasWorkspace.exe` is the primary executable
- Workspace is a platform for editors, tools, build systems, and project orchestration
- No game-specific logic in workspace core
- `Source/Workspace/` (NF::Workspace) is the OS-like host layer — owns shell, registries, services
- `Source/Editor/` (NF::Editor) contains only primary tool implementations and sub-panels
- WorkspaceShell is tool-agnostic; tools register via factory/CoreToolRoster, not hardcoded includes

## UI

**AtlasUI** is the standard UI framework.

- No canonical ImGui path for core tooling
- WPF shell direction is archived/deferred
- Native C++ tooling is the active path
- Win32 is the baseline platform for current tooling

## AI

**AtlasAI** is the canonical broker system.

- AtlasAI is the visible middle-man broker
- Preferred flow: local/internal first, then web, then broader model/provider layers
- Build errors and logs route through AtlasAI with notification-driven fix flow
- Historical name "Arbiter" is retired from active paths

## Projects

Projects (e.g., NovaForge) are hosted and remain logically detachable.

- Projects load through workspace adapters/plugins
- Project-specific gameplay logic does not live in workspace core
- NovaForge can live in the monorepo for development but is not built by default

## Editor Philosophy

- Limited number of primary tools (~10)
- Shared panels and services replace one-off editors
- No uncontrolled tool expansion
- Consolidation before feature growth

## Backend Strategy

- **GDI** = fallback only
- **D3D11** = target UI backend
- **DirectWrite** = target text backend
- **OpenGL/GLFW** = compatibility-only if retained

## Repo Philosophy

- Structure before features
- Consolidation before expansion
- Presence is not completion
- A file, folder, class, editor, or stub existing in the repo does not mean the system is implemented
