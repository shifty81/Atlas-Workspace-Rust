# Naming Canon

## Approved Names

| Name | Usage |
|------|-------|
| **Atlas Workspace** | User-facing product name |
| **AtlasWorkspace.exe** | Primary executable |
| **AtlasUI** | UI framework |
| **AtlasAI** | AI broker system |
| **NovaForge** | Hosted game project |

## Core Symbols

| Name | Meaning |
|------|---------|
| AtlasWorkspace | The development host platform executable and repo |
| AtlasUI | The native C++ UI framework (widgets, theme, draw lists) |
| AtlasAI | The AI broker system (formerly Arbiter + SwissAgent) |
| NovaForge | The hosted game project (adapter-loaded, logically detachable) |
| WorkspaceShell | Composition root — owns registries, managers, services |
| IHostedTool | Interface for primary workspace tools |
| IGameProjectAdapter | Interface for project plugin loading |
| ISharedPanel | Interface for reusable workspace panels |

## Removed Names

These names must not appear in active code, docs, or user-facing strings:

| Old Name | Replacement | Notes |
|----------|-------------|-------|
| Arbiter | AtlasAI | ✅ Active-path references migrated; historical preserved in Archive |
| ArbiterAI | AtlasAI | ✅ Tool and module references migrated |
| SwissAgent | AtlasAI | ✅ Absorbed into AtlasAIAdapter; SwissAgentAdapter removed |
| EditorApp | WorkspaceShell + WorkspaceRenderer | Legacy dual-runtime path deprecated |
| NF_Editor (standalone) | NF::Workspace + NF::Editor | Split into OS layer and tool layer |
| Atlas Suite | Atlas Workspace | Product naming |
| AtlasToolingSuite | Atlas Workspace | Product naming |
| MasterRepo.exe | AtlasWorkspace.exe | Executable naming |

## Case Variants

| Context | Format |
|---------|--------|
| User-facing text | Atlas Workspace |
| Executable | AtlasWorkspace.exe |
| C++ namespace | NF (existing convention) |
| CMake target | AtlasWorkspace |
| Config keys/macros | ATLAS_AI (not ARBITER) |
| File names/symbols | atlas_ai (where snake_case fits) |

## Namespace Conventions

- `NF::` — root namespace for all workspace modules
- `NF::Workspace` — OS-like host layer (shell, registries, services)
- `NF::Editor` — primary tool implementations
- `NF::UI` — AtlasUI widget and rendering system
- `NovaForge::` — project-specific code (adapter, panels, game systems)

## Rules

- No legacy names in active code paths
- Historical names allowed **only** in `/Archive/` or explicitly labeled historical docs
- User-facing strings must follow this canon
- New code must use canonical names from creation
- Rename drift must be caught and corrected immediately

