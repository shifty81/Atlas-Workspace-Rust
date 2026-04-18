# Locked Direction

These decisions are fixed. Do not revisit without explicit project-level review.

## Rust-First Rules (Effective 2026-04-18)

1. **Rust is the primary language.** All new systems are written in Rust. C++ in `Source/` and `NovaForge/` is reference/archive only.
2. **Vulkan is the rendering backend.** No other GPU API. `atlas-renderer` owns this.
3. **Cargo is the build system.** CMake is legacy. `Scripts/build_rust.sh` is the primary build entry point.
4. **NovaForge is being ported to Rust.** The C++ implementation is the spec. Port follows Phases 2–4 of the roadmap.
5. **`atlas-editor` hosts the game.** `atlas-game` runs standalone OR embedded in `atlas-editor` via PIE.
6. **PCG is deterministic and seed-based.** `PcgManager` + 16 `PcgDomain`s. Never break determinism.
7. **Safe Rust only.** No `unsafe` in application code. Vulkan FFI is contained in `atlas-renderer`.

---

## Workspace

`atlas-editor` is the primary executable host (`cargo run --bin atlas-workspace` or `cargo run --bin atlas-editor`).

- Workspace is a platform for editors, tools, build systems, and project orchestration
- No game-specific logic in workspace core
- `atlas-editor` (Rust/egui) is the OS-like host layer — owns shell, registries, services
- `WorkspaceShell` is tool-agnostic; tools register via `IEditorTool` trait

## UI

**egui** is the standard UI framework for Rust tooling.

- `atlas-ui` will provide widget extensions and a panel framework
- No C++ AtlasUI/D3D11/GDI code in new Rust systems

## AI

**AtlasAI** (`atlas-ai`) is the canonical broker system.

- `AtlasAIBroker` is the visible middle-man
- Preferred flow: local/internal first, then web, then broader model/provider layers
- Build errors and logs route through AtlasAI with notification-driven fix flow
- Historical name "Arbiter" is retired from active paths

## Projects

Projects (e.g., NovaForge) are hosted and remain logically detachable.

- Projects load through workspace adapters (`IGameProjectAdapter` trait in `atlas-editor`)
- Project-specific gameplay logic lives in `atlas-game` crate or dedicated crates
- `atlas-editor` **never** imports game logic directly

## Editor Philosophy

- Limited number of primary tools (~10, matching the C++ WorkspaceShell roster)
- Shared panels and services replace one-off editors
- No uncontrolled tool expansion
- Consolidation before feature growth

## Repo Philosophy

- Structure before features
- Consolidation before expansion
- Presence is not completion
- A file, folder, crate, or stub existing in the repo does not mean the system is implemented
