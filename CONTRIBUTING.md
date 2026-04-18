# Contributing to Atlas Workspace

## Current Phase

The project is in **Milestone 0: Canon Reset and Consolidation**. Feature expansion is paused until cleanup is complete.

## Rules During Cleanup Milestone

- **No new standalone editors** — use shared panels or services instead
- **No feature spray** — focus on cleanup, consolidation, normalization
- **No stale naming** — use canonical names only (see `Docs/Canon/03_NAMING_CANON.md`)
- **No generated artifacts in commits** — build output stays in `.gitignore`

## Naming Rules

| Name | Usage |
|------|-------|
| Atlas Workspace | Product name |
| AtlasWorkspace.exe | Primary executable |
| AtlasUI | UI framework |
| AtlasAI | AI broker system |
| NovaForge | Hosted game project |

Do not use: Arbiter, Atlas Suite, MasterRepo, or any legacy naming.

## Code Conventions

- C++20
- Header-only where practical
- NF namespace for workspace modules
- NovaForge namespace for project-specific code
- Follow existing style in neighboring files

## New Editor Idea Classification

Before adding any new editor surface, classify it:

| Type | Where It Lives |
|------|---------------|
| **Tool** | Primary tool (~10 max). Needs justification. |
| **Panel** | Shared reusable UI component in workspace core. |
| **Service** | Non-visual backend system. |
| **Plugin** | Project-specific, loaded through adapter. |
| **Archive** | Not currently needed. Move to archive. |

## Doc Update Expectations

- Update canon docs when direction changes
- Update inventory docs when tools/panels change
- Keep README current with structural changes
- Use strict status labels (Done/In Progress/Partial/Planned/Blocked/Archived)

## Build Expectations

```bash
# Configure (no network, no tests)
cmake --preset debug

# Configure with tests
cmake -S . -B build -DATLAS_BUILD_TESTS=ON -DATLAS_ENABLE_ONLINE_DEPS=ON

# Build
cmake --build --preset debug --parallel
```

## Patch Workflow

- Patches must be deterministic
- Context menu registrations must have uninstall counterparts
- No one-way changes to system state
