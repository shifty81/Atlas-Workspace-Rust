# Project Status

> Reset Date: 2026-04-18
> Direction: Rust + Vulkan (primary). C++ in `Source/` is reference only.
> Active Phase: Phase 0 — Rust Foundation Completion

---

## Per-Crate Status

| Crate | Status | What Works |
|-------|--------|------------|
| `atlas-core` | ✅ Implemented | Logger, `StringId`, `AtlasError`, `VERSION`, `EventBus` |
| `atlas-math` | ✅ Implemented | `Vec2/3/4`, `Mat4`, `Quat`, `Aabb`, `Ray`, `Transform`, `Color` (glam-backed) |
| `atlas-ecs` | ✅ Implemented | `EntityManager`, `ComponentStore`, `SceneGraph`, `SystemRegistry`, `DeltaEditStore`, `World` |
| `atlas-pcg` | ✅ Implemented | `DeterministicRng`, `PcgManager`, 16 `PcgDomain`s, `ConstraintSolver`, `MeshGraph`, `MaterialGraph`, `LodBakingGraph`, `TerrainGenerator`, `PlanetaryBase`, `BuildQueue` |
| `atlas-world` | ✅ Implemented | `Universe`, `Galaxy`, `StarSystem`, `Planet`, `AsteroidBelt`, `AssetRegistry`, `WorldState`, `ModLoader` |
| `atlas-workspace` | ✅ Implemented | Main binary; boots renderer, runs PCG demo. `cargo run --bin atlas-workspace` |
| `atlas-renderer` | 🔄 In Progress | Vulkan context + pipeline done. `ShaderIrModule`/Compiler, `SpatialHash`, `RenderConfig`, `PbrMaterial`. Awaiting live Vulkan surface |
| `atlas-editor` | 🔄 In Progress | `EditorApp`, 5 panels (Outliner, Properties, Viewport, AssetBrowser, Console), `CommandStack`, `SelectionState`, `SpawnEntityCommand`, `DeleteEntityCommand`, `RenameEntityCommand`, `GameBuildSystem`, `GameProjectAdapter` trait, `SceneRenderer` |
| `atlas-game` | 🔄 In Progress | `GameRunner`, `NullGameModule`, `GameModule` trait. `cargo run --bin atlas-game` |
| `atlas-input` | 🔲 Stub | Empty — winit event → InputState mapping not implemented |
| `atlas-physics` | 🔲 Stub | Empty — AABB collision, rigid body, ray-cast not implemented |
| `atlas-sim` | 🔲 Stub | Empty — fixed-timestep loop, system scheduler not implemented |
| `atlas-script` | 🔲 Stub | Empty — Lua/Rhai integration not implemented |
| `atlas-animation` | 🔲 Stub | Empty — clip, channel, keyframe, interpolation not implemented |
| `atlas-ai` | 🔲 Stub | Empty — `AtlasAIBroker`, request context, conversation not implemented |
| `atlas-sound` | 🔲 Stub | Empty — audio source, mixer, SFX/music not implemented |
| `atlas-graphvm` | 🔲 Stub | Empty — node graph VM, pin types not implemented |
| `atlas-net` | 🔲 Stub | Empty — client/server, message framing not implemented |
| `atlas-schema` | 🔲 Stub | Empty — JSON schema types, property grid not implemented |
| `atlas-abi` | 🔲 Stub | Empty — stable C ABI for plugin loading not implemented |
| `atlas-asset` | 🔲 Stub | Empty — UUID asset registry, load pipeline not implemented |
| `atlas-ui` | 🔲 Stub | Empty — egui widget extensions not implemented |

Legend: ✅ Implemented | 🔄 In Progress | 🔲 Stub

---

## What Works

- **Cargo workspace**: 22 crates scaffolded, all compile cleanly
- **439 passing unit tests** across the workspace
- `atlas-core` / `atlas-math` / `atlas-ecs` / `atlas-pcg` / `atlas-world` / `atlas-workspace`: fully implemented
- `atlas-renderer`: Vulkan context + pipeline implemented; shader IR, spatial hash, PBR material types — done
- `atlas-editor`: app shell, 5 panels, command stack, selection state, entity commands — in progress
- `atlas-game`: game loop scaffolding, `GameRunner`, `GameModule` trait — in progress
- PCG pipeline: deterministic, 16 isolated domains, parallel terrain generation (Rayon)

---

## What Does NOT Work

- **Live Vulkan surface**: renderer awaiting a real display server (winit + ash-window wiring not complete)
- **PIE (Play-In-Editor)**: `PIEService` stub exists in atlas-editor; not yet connected to atlas-game `GameRunner`
- **13 stub crates**: atlas-input, atlas-physics, atlas-sim, atlas-script, atlas-animation, atlas-ai, atlas-sound, atlas-graphvm, atlas-net, atlas-schema, atlas-abi, atlas-asset, atlas-ui — all empty
- **SPIR-V shader compilation pipeline**: glslc/naga wiring not complete
- **GPU terrain mesh upload**: PCG terrain data not yet uploaded to GPU

---

## Test Count

**439 passing unit tests** (as of Phase 0 current state).

Coverage includes: atlas-core, atlas-math, atlas-ecs (all subsystems), atlas-pcg, atlas-world, atlas-sim (save system, tick scheduler, replay recorder, sim mirror, determinism registry), atlas-renderer (shader IR, spatial hash, PBR material), atlas-editor (command stack, selection, entity commands, panels), atlas-game (game runner, module), atlas-script (sandbox, assembler, VM), atlas-animation (graph), atlas-ai (behavior graph, memory), atlas-sound (graph), atlas-physics (physics system).

---

## C++ Reference

`Source/` and `NovaForge/` exist in this repo as **reference and archive only**.

- **CMake is NOT the primary build system** — Cargo is.
- The C++ implementation (Phases A–I) is the **specification** for the Rust port.
- Do not submit C++ PRs. Do not modify C++ files.
- See `Scripts/build_cpp_legacy.sh` if you need to build the C++ reference.

---

## Build Status

| Check | Status |
|-------|--------|
| `cargo build --workspace` | ✅ Clean |
| `cargo test --workspace` | ✅ 439 passing |
| `cargo clippy --workspace` | Run via `bash Scripts/check_rust.sh` |
| CMake (C++ reference) | ⚠ Legacy — not primary |
