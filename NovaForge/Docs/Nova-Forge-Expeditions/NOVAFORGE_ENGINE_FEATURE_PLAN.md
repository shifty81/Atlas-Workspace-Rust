# NovaForge → AtlasForge Engine Feature Integration Plan

> **Purpose**: Compare engine-relevant features from [shifty81/NovaForge](https://github.com/shifty81/NovaForge) against the AtlasForge engine and assess the benefits of merging each feature. This is a **planning document only** — no code has been merged yet.
>
> **Scope**: Engine features only. Game-specific content (ships, factions, missions, economy, NPC data) is excluded.

---

## 1. Repository Overview

| | **AtlasForge** | **NovaForge** |
|---|---|---|
| **Purpose** | Modular, data-driven game engine & simulation platform | PVE space simulator built on the Atlas Engine |
| **Language** | C++20 | C++17 |
| **Graphics** | OpenGL (engine/render) | OpenGL 4.5 (deferred rendering, PBR) |
| **Engine Modules** | 40+ subsystems | 13 engine modules + client/server/editor |
| **Editor** | Standalone editor with 14+ dockable panels | Standalone editor + 32 header-only in-client editor tools (F12 toggle) |
| **Projects** | 3 (eveoffline, arena2d, atlas-sample) | 1 (NovaForge game) |
| **Build** | CMake + build.sh | CMake + Makefile + build scripts |
| **Tests** | tests/ directory | atlas_tests/ (374+ assertions) |
| **Determinism** | Hash-ladder verification, CI gates | Seed-locked PCG, deterministic RNG |

---

## 2. Feature-by-Feature Comparison

### 2.1 Rendering Pipeline

| Feature | AtlasForge | NovaForge | Gap |
|---------|-----------|-----------|-----|
| Forward rendering | ✅ engine/render | ✅ | — |
| Deferred rendering (G-Buffer) | ❌ | ✅ (PBR, shadow mapping, bloom, tone mapping) | **NovaForge ahead** |
| Instanced rendering | ❌ | ✅ | **NovaForge ahead** |
| LOD system | ✅ engine/render | ✅ (frustum culling + LOD manager) | Comparable |
| Post-processing (bloom, tone mapping) | ❌ | ✅ | **NovaForge ahead** |

**Benefit of merging**: Deferred rendering with PBR materials enables more complex lighting scenarios (many lights without per-light passes). Post-processing adds visual polish. Instanced rendering drastically improves draw call performance for large entity counts (1000+).

### 2.2 Atlas UI Framework

| Feature | AtlasForge | NovaForge | Gap |
|---------|-----------|-----------|-----|
| Engine UI system | ✅ engine/ui (custom) | ✅ Atlas UI (immediate-mode, GPU-accelerated) | **Different implementations** |
| Immediate-mode API | Unknown | ✅ (`if (button(...))` pattern) | **NovaForge has clean API** |
| Single draw-call batching | Unknown | ✅ All UI in one GPU pass | **NovaForge optimized** |
| Sci-fi widget library | ❌ | ✅ (panels, arcs, capacitor rings, module slots, overview tables, target cards) | **NovaForge ahead** |
| Theming system | Unknown | ✅ Full `Theme` struct (teal, amber, colorblind-safe) | **NovaForge ahead** |
| Zero external dependencies | Unknown | ✅ Only OpenGL 3.3 | **NovaForge portable** |
| Drag-to-move panels | Unknown | ✅ Stateful panels with persistence | **NovaForge ahead** |
| Text input, combobox, slider, checkbox | Unknown | ✅ Complete widget set | **NovaForge comprehensive** |

**Benefit of merging**: The Atlas UI framework is explicitly designed to be reusable in other projects (just copy 4 header + 3 cpp files). It provides a complete immediate-mode UI toolkit with no dependencies beyond OpenGL 3.3. The sci-fi widget set (status arcs, capacitor rings, module slots) would give AtlasForge a polished, production-ready UI layer.

### 2.3 Editor Tool Layer (In-Client Overlay)

| Feature | AtlasForge | NovaForge | Gap |
|---------|-----------|-----------|-----|
| Standalone editor | ✅ editor/ (14+ panels) | ✅ editor/ (17 panels) | Comparable |
| In-client editor overlay (F12 toggle) | ❌ | ✅ `EditorToolLayer` with compile-out flag | **NovaForge ahead** |
| Header-only tools | ❌ | ✅ 32 tools in `cpp_client/include/editor/` | **NovaForge ahead** |
| ITool interface | ❌ | ✅ `Name()`, `Activate()`, `Deactivate()`, `Update()`, `IsActive()` | **NovaForge ahead** |
| EditorCommandBus | ❌ | ✅ Decoupled FIFO command queue | **NovaForge ahead** |
| UndoableCommandBus | ❌ | ✅ Full undo/redo stack | **NovaForge ahead** |
| EditorEventBus | ❌ | ✅ Publish/subscribe for editor events | **NovaForge ahead** |
| DeltaEditStore | ✅ engine/ecs (DeltaEditStore exists) | ✅ Records property edits across PCG regen | Comparable |
| SceneBookmarkManager | ❌ | ✅ Save/restore camera + scene checkpoints | **NovaForge ahead** |
| LayerTagSystem | ❌ | ✅ Entity categorization and visibility control | **NovaForge ahead** |
| Zero-overhead release builds | ❌ | ✅ `NOVAFORGE_EDITOR_TOOLS=OFF` compiles out everything | **NovaForge ahead** |

**Specific tools NovaForge has**:
- Animation editor (keyframes, blend weights)
- Batch operations (mass transformations)
- IK rig tool (inverse kinematics chains)
- Material/shader live editing
- PCG snapshot/rollback manager
- DeltaEdits merge tool
- Edit propagation across similar assets
- Visual diff tool (current vs PCG baseline)
- Live edit mode (edit while simulation runs)
- Environment control (gravity, wind, atmosphere)
- NPC spawner tool
- Prefab library (drag-and-drop reusable modules)
- Simulation step controller (frame-step, pause)

**Benefit of merging**: The EditorToolLayer pattern is a major productivity enhancement — developers can iterate on content without leaving the running game. The compile-out flag (`#ifdef`) ensures zero overhead in release builds. The 32 header-only tools cover animation, physics, IK, materials, PCG, NPC spawning, and more. This would significantly extend AtlasForge's editor capabilities.

### 2.4 Procedural Content Generation (PCG) Framework

| Feature | AtlasForge | NovaForge | Gap |
|---------|-----------|-----------|-----|
| Procedural modeling | ✅ engine/procedural, docs/17_PROCEDURAL_MODELING.md | ✅ Extensive PCG framework | Comparable |
| Deterministic RNG | ✅ (hash-ladder) | ✅ Custom xorshift64* (no std::rand) | Comparable |
| Seed hierarchy | Unknown | ✅ Universe→Galaxy→System→Sector→Object | **NovaForge more structured** |
| PCG domains | Unknown | ✅ 16 domains (Universe, Galaxy, Ship, Asteroid, NPC, Fleet, Loot, Mission, etc.) | **NovaForge ahead** |
| PCG Manager (central authority) | Unknown | ✅ Owns universe seed, creates scoped contexts | **NovaForge ahead** |
| Ship generation | ❌ | ✅ Hull class selection, stat derivation, weapon fitting, validation | **NovaForge ahead** |
| Fleet doctrine generation | ❌ | ✅ 5 doctrine types (Brawler, Sniper, Kite, Logistics, CapitalSupport) | **NovaForge ahead** |
| Asteroid field generation | ❌ | ✅ Disc-shaped belts, ore distribution by security level | **NovaForge ahead** |
| Anomaly generation | ❌ | ✅ 6 site types, difficulty scaling | **NovaForge ahead** |
| NPC encounter generation | ❌ | ✅ Multi-wave, faction-appropriate | **NovaForge ahead** |
| Interior-driven hull generation | ❌ | ✅ Interior modules → hallways → hull mesh wrapping | **NovaForge ahead** |
| PCG verification | ❌ | ✅ Server-authoritative verification (`pcg_verify.h`) | **NovaForge ahead** |
| Constraint solver | ❌ | ✅ Genetic-style fitting for ship weapons | **NovaForge ahead** |
| Spine-based hull grammar | ❌ | ✅ `spine_hull_generator.h` | **NovaForge ahead** |

**Benefit of merging**: NovaForge's PCG framework is mature and battle-tested with 16 isolated domains ensuring that changing one generator never affects another. The seed hierarchy enables deterministic world regeneration — players can share universe seeds and get bit-identical results. The ship/fleet/anomaly generators would give AtlasForge powerful content generation capabilities out of the box. The framework's `World = Generate(Seed, Rules, Version)` principle aligns perfectly with AtlasForge's determinism-first philosophy.

### 2.5 Procedural Ship/Station/Asset Generation

| Feature | AtlasForge | NovaForge | Gap |
|---------|-----------|-----------|-----|
| Procedural ship modeling | ✅ (docs/17_PROCEDURAL_MODELING.md) | ✅ Recursive interior→hull pipeline | **NovaForge more advanced** |
| Interior node system | ❌ | ✅ 10+ module types (Deck, Belly Bay, Rover, Drone Bay, Airlock, etc.) | **NovaForge ahead** |
| Hull mesh generation | ❌ | ✅ Convex hull from interior volumes, gap filling, stretch-wrap | **NovaForge ahead** |
| Module tier system (1-5) | ❌ | ✅ Visual upgrades per tier with procedural detail | **NovaForge ahead** |
| Build/upgrade queue | ❌ | ✅ Parallel timers, deterministic order, skill/drone modifiers | **NovaForge ahead** |
| Planetary base generation | ❌ | ✅ Same recursive pipeline as ships, flat-site integration | **NovaForge ahead** |
| Blender integration | ✅ docs/BLENDER_ADDON_DESIGN.md (design spec) | ✅ tools/BlenderSpaceshipGenerator/ (implementation) | **NovaForge has implementation** |
| Module size scaling (XS-XL) | ❌ | ✅ 2XS=1S, 1M=3S, 1L=5S, 1XL=2L | **NovaForge ahead** |

**Benefit of merging**: The recursive procedural generation pipeline (interior modules → hallways → hull wrapping) is a sophisticated system that generates ships, stations, bases, and vehicles from a unified algorithm. The interior-driven exterior approach means game objects are functionally accurate — what you see on the outside reflects what's inside. Combined with the existing Blender addon design spec in AtlasForge, this would create a complete content pipeline.

### 2.6 Blender Integration Tools

| Feature | AtlasForge | NovaForge | Gap |
|---------|-----------|-----------|-----|
| Blender addon design spec | ✅ docs/BLENDER_ADDON_DESIGN.md (173KB, comprehensive) | ✅ blenderaddon1.md (same/similar content) | Comparable |
| BlenderSpaceshipGenerator | ❌ | ✅ tools/BlenderSpaceshipGenerator/ (with ENGINE_INTEGRATION.md) | **NovaForge has tool** |
| Engine integration pipeline | ❌ | ✅ Documented export→engine asset pipeline | **NovaForge ahead** |

**Benefit of merging**: NovaForge has an actual Blender spaceship generator tool with engine integration documentation. Merging this into AtlasForge would complement the existing design spec (docs/BLENDER_ADDON_DESIGN.md) with a working tool and concrete integration pipeline.

### 2.7 Client Architecture & Rendering

| Feature | AtlasForge | NovaForge | Gap |
|---------|-----------|-----------|-----|
| Deferred rendering | ❌ | ✅ G-Buffer, PBR materials | **NovaForge ahead** |
| Shadow mapping | ❌ | ✅ | **NovaForge ahead** |
| Post-processing (bloom, tone mapping) | ❌ | ✅ | **NovaForge ahead** |
| Instanced rendering | ❌ | ✅ | **NovaForge ahead** |
| GLSL shaders | Unknown | ✅ cpp_client/shaders/ | Possibly comparable |
| Spatial hash system | ❌ | ✅ Efficient proximity queries at scale | **NovaForge ahead** |
| Frustum culling | Unknown | ✅ | Possibly comparable |

**Benefit of merging**: A deferred rendering pipeline with PBR, shadows, and post-processing would upgrade AtlasForge's visual output significantly. The spatial hash system enables efficient proximity queries critical for large-scale simulations (1000+ entities).

### 2.8 Architecture & Build System

| Feature | AtlasForge | NovaForge | Gap |
|---------|-----------|-----------|-----|
| Docker support | ❌ | ✅ Dockerfile for server container | **NovaForge ahead** |
| CMake presets | ❌ | ✅ CMakePresets.json (linux, macOS, windows) | **NovaForge ahead** |
| Makefile shortcuts | ❌ | ✅ `make build`, `make test-engine`, etc. | **NovaForge ahead** |
| Dual C++/Python server | ❌ | ✅ cpp_server + server.py fallback | **NovaForge ahead** |
| TLA+ formal specifications | ❌ | ✅ specs/ (ECS, layout, replay) | **NovaForge ahead** |
| Editor-in-client build flag | ❌ | ✅ `NOVAFORGE_EDITOR_TOOLS` compile flag | **NovaForge ahead** |

**Benefit of merging**: Docker support simplifies deployment and testing. CMake presets standardize multi-platform builds. The Makefile provides convenient shortcuts. TLA+ specifications provide formal verification of critical engine invariants (ECS correctness, layout safety, replay determinism).

### 2.9 Testing & Verification

| Feature | AtlasForge | NovaForge | Gap |
|---------|-----------|-----------|-----|
| Engine unit tests | ✅ tests/ | ✅ atlas_tests/ (374+ assertions) | Comparable |
| Editor tool tests | ❌ | ✅ 35+ dedicated test files | **NovaForge ahead** |
| PCG verification | ❌ | ✅ `pcg_verify.h` (server-authoritative) | **NovaForge ahead** |
| Contract scanning | ❌ | ✅ `tools/contract_scan.py` | **NovaForge ahead** |

**Benefit of merging**: NovaForge has comprehensive test coverage for all 32 editor tools and the PCG framework. Merging these test patterns would improve AtlasForge's test infrastructure.

---

## 3. Benefits Summary

### High-Impact Features (Recommended Priority)

| Priority | Feature | Impact | Effort | Benefit |
|:--------:|---------|--------|--------|---------|
| **1** | **Editor Tool Layer** (32 tools, F12 toggle) | 🔴 High | Medium | Massive productivity gain — edit content in running game with zero release overhead |
| **2** | **Atlas UI Framework** (immediate-mode, GPU-batched) | 🔴 High | Medium | Production-ready UI toolkit with sci-fi widgets, theming, zero dependencies |
| **3** | **PCG Framework** (seed hierarchy, 16 domains) | 🔴 High | Medium | Deterministic world generation with domain isolation and bit-identical regeneration |
| **4** | **Deferred Rendering + PBR** | 🔴 High | High | Major visual quality upgrade with complex lighting and post-processing |
| **5** | **Blender Generator Tool** | 🟡 Medium | Low | Working procedural asset generator with engine integration docs |

### Medium-Impact Features

| Priority | Feature | Impact | Effort | Benefit |
|:--------:|---------|--------|--------|---------|
| **6** | **Procedural Ship/Station Pipeline** | 🟡 Medium | High | Recursive interior→hull generation for complex game objects |
| **7** | **Spatial Hash System** | 🟡 Medium | Low | Efficient proximity queries for large entity counts |
| **8** | **Docker + CMake Presets** | 🟡 Medium | Low | Standardized builds and deployment |
| **9** | **TLA+ Specifications** | 🟡 Medium | Low | Formal verification of ECS, layout, and replay invariants |
| **10** | **Editor Tool Tests** (35+ test files) | 🟡 Medium | Low | Comprehensive test patterns for editor infrastructure |

### Low-Impact / Already Covered

| Feature | Notes |
|---------|-------|
| Basic ECS | AtlasForge already has a mature ECS |
| Graph VM | AtlasForge already has GraphVM + compiler |
| Networking | Both have CS + P2P, lockstep/rollback |
| Asset system | Both have binary format + hot reload |
| Simulation | Both have deterministic tick scheduling |
| World generation | Both have world layouts + LOD |

---

## 4. Proposed Integration Phases

### Phase A: Low-Risk Documentation & Build Tooling
**Effort: Low | Risk: None | Timeline: Immediate**

- [x] Integrate Makefile convenience targets (build, test, etc.)
- [x] Add CMakePresets.json for multi-platform build standardization
- [x] Add Dockerfile for headless server deployment
- [x] Import TLA+ specifications to `specs/` directory
- [x] Import BlenderSpaceshipGenerator tool to `tools/blender-addon/`

### Phase B: Atlas UI Framework
**Effort: Medium | Risk: Low | Timeline: 1-2 weeks**

- [x] Copy Atlas UI files (4 headers + 3 cpp) into engine/ui/atlas/
- [x] Integrate with existing engine/ui module
- [x] Validate widget rendering with OpenGL 3.3
- [x] Add theming support configuration
- [x] Port widget test coverage

### Phase C: Editor Tool Layer Infrastructure
**Effort: Medium | Risk: Medium | Timeline: 2-3 weeks**

- [x] Implement ITool interface in engine/tools/
- [x] Implement EditorCommandBus and UndoableCommandBus
- [x] Implement EditorEventBus (publish/subscribe)
- [x] Implement SceneBookmarkManager
- [x] Implement LayerTagSystem
- [x] Add `ATLAS_EDITOR_TOOLS` compile flag for in-client overlay
- [x] Port key editor tools (start with 10 highest-value tools)
- [x] Import editor tool test suite

### Phase D: PCG Framework
**Effort: Medium | Risk: Medium | Timeline: 2-3 weeks**

- [x] Implement deterministic RNG (xorshift64*) in engine/procedural/
- [x] Implement seed hierarchy (Universe→Galaxy→System→Sector→Object)
- [x] Implement PCG domain isolation (16 domains)
- [x] Implement PCGManager (central seed authority)
- [x] Implement PCG verification (server-authoritative)
- [x] Implement constraint solver for fitting validation
- [x] Port PCG test coverage

### Phase E: Rendering Pipeline Upgrade
**Effort: High | Risk: High | Timeline: 4-6 weeks**

- [x] Implement G-Buffer for deferred rendering
- [x] Add PBR material support
- [x] Add shadow mapping
- [x] Add post-processing pipeline (bloom, tone mapping)
- [x] Add instanced rendering for large entity counts
- [x] Implement spatial hash system for proximity queries
- [x] Performance benchmarking and optimization

### Phase F: Procedural Generation Pipeline
**Effort: High | Risk: Medium | Timeline: 3-4 weeks**

- [x] Port interior node system (10+ module types)
- [x] Port hull mesh generation (convex hull + gap filling + stretch-wrap)
- [x] Port module tier system (1-5 with visual upgrades)
- [x] Port build/upgrade queue system
- [x] Port planetary base generation
- [x] Port module size scaling rules

---

## 5. Risk Assessment

| Risk | Severity | Mitigation |
|------|----------|------------|
| C++17 vs C++20 compatibility | Low | NovaForge uses C++17; AtlasForge is C++20 — forward compatible |
| Rendering pipeline conflicts | Medium | Deferred rendering is additive; keep forward path as fallback |
| PCG seed format divergence | Low | Both use deterministic seeds; standardize on AtlasForge's hash-ladder |
| Editor tool namespace collisions | Low | Use `atlas::editor::` namespace prefix |
| Build system complexity | Low | CMake options are additive; existing builds unaffected |
| Test infrastructure differences | Low | Adapt NovaForge's test patterns to AtlasForge's test framework |

---

## 6. What NOT to Merge (Game Content)

The following NovaForge content is **game-specific** and should NOT be merged into AtlasForge's engine:

- Ship definitions (102+ ships, 159+ modules)
- Faction data (Solari, Veyren, Aurelian, Keldari)
- Mission templates and objectives
- Economy data (market, industry, mining)
- NPC spawn configurations
- Universe layout (solar systems, stargates)
- Skill trees and training data
- Corporation/social systems data
- Insurance, bounty, contract data

These belong in game projects (like `projects/eveoffline/`), not in the engine.

---

## 7. Conclusion

NovaForge has developed significant engine-level features that would benefit AtlasForge:

1. **Editor Tool Layer** — The biggest productivity win. 32 tools with F12 toggle and zero-cost release builds.
2. **Atlas UI Framework** — A polished, reusable, zero-dependency UI toolkit.
3. **PCG Framework** — Mature deterministic generation with domain isolation and formal verification.
4. **Rendering upgrades** — Deferred rendering, PBR, post-processing, and instanced rendering.
5. **Build/deploy tooling** — Docker, CMake presets, Makefile shortcuts.

The integration can be phased from low-risk (docs, build tooling) to high-impact (rendering pipeline) over 6-12 weeks. AtlasForge's existing architecture (modular, data-driven, deterministic) aligns well with NovaForge's engine features, minimizing integration risk.

**Recommendation**: Start with Phase A (documentation & build tooling) and Phase B (Atlas UI Framework) for immediate, low-risk improvements, then proceed to Phase C (Editor Tool Layer) for the highest-impact productivity gain.

---

*Document generated: 2026-03-05*
*Source: [shifty81/NovaForge](https://github.com/shifty81/NovaForge) analysis*
*Target: [shifty81/AtlasForge](https://github.com/shifty81/AtlasForge) engine integration*
