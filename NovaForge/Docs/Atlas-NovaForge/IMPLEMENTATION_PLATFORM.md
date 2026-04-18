# Atlas Engine — Platform & Ecosystem Implementation Reference

> Companion to `IMPLEMENTATION_CORE.md`. Covers platform infrastructure,
> project architecture, mod ecosystem, multiplayer determinism, and SDK distribution.
>
> **Note:** This document describes both implemented features and planned designs.
> Items marked as "Planned" or "Stub" are not yet fully functional.

---

## Table of Contents

1. [Multi-Project Architecture](#1-multi-project-architecture)
2. [Runtime Adapter ABI](#2-runtime-adapter-abi)
3. [Schema System](#3-schema-system)
4. [Graph Linting & Diff](#4-graph-linting--diff)
5. [Project Hot-Reload](#5-project-hot-reload)
6. [Editor Extensions](#6-editor-extensions)
7. [Adapter Test Harness](#7-adapter-test-harness)
8. [Mod System](#8-mod-system)
9. [Asset Compression & Encryption](#9-asset-compression--encryption)
10. [Performance Profiling](#10-performance-profiling)
11. [Deterministic Time & Clock](#11-deterministic-time--clock)
12. [Asset Provenance & Lineage](#12-asset-provenance--lineage)
13. [Project Templates](#13-project-templates)
14. [SDK & Distribution](#14-sdk--distribution)
15. [Multiplayer Determinism Testing](#15-multiplayer-determinism-testing)
16. [Reference Project: Arena2D](#16-reference-project-arena2d)

---

## 1. Multi-Project Architecture

The engine builds independently. Projects are optional plugins discovered at
configure time from a `Projects/` directory. Each project contains a `Plugin.toml`
declaring engine version, content paths, and adapter entry points.

```
Atlas/
├── engine/
├── editor/
├── projects/
│   ├── eveoffline/eveoffline.atlas
│   ├── arena2d/arena2d.atlas
│   └── atlas-sample/sample.atlas
└── CMakeLists.txt
```

```toml
[project]
name        = "MyGame"
engine_ver  = "0.1"
content     = "Content/"
schemas     = "Schemas/"
adapter     = "src/MyAdapter.cpp"
entry_point = "Atlas_CreateRuntimeAdapter"
```

| Rule | Description |
|------|-------------|
| **Deletion Test** | Engine MUST build with `Projects/` deleted entirely. |
| **Auto-discovery** | CMake scans `Projects/*/Plugin.toml` at configure time. |
| **CI modes** | `engine-only` (no projects) and `engine+project` (with selected project). |

## 2. Runtime Adapter ABI

The **RuntimeAdapter** is the formal contract between engine and game code.
Game logic lives behind this boundary and is loaded dynamically via a C-linkage
factory function.

```cpp
class RuntimeAdapter {
public:
    virtual ~RuntimeAdapter() = default;
    virtual void Initialize()                        = 0;
    virtual void Consume(const GraphOutputView& out) = 0;
    virtual void Shutdown()                          = 0;
};

// Shared libraries expose this single entry point
extern "C" RuntimeAdapter* Atlas_CreateRuntimeAdapter();
```

### Hard Rules

- **No allocation during `Consume()`** — pre-allocate in `Initialize()`.
- **No engine calls** — the adapter never reaches back into engine internals.
- **Read-only `GraphOutputView`** — treat as an immutable snapshot.
- **Stateless / snapshot-based** — each `Consume()` call is self-contained.

## 3. Schema System

### Validation

**SchemaValidator** enforces structural correctness:

| Check | Condition |
|-------|-----------|
| Non-empty IDs | Every schema node must have a non-blank identifier. |
| Positive versions | Version numbers are `>= 1`. |
| No duplicate node IDs | All node IDs unique within a schema. |

Runs in the **editor** (on save), **CI** (on every commit), and optionally at
**runtime** (dev builds only).

### Migration

```cpp
struct SchemaMigrationStep {
    uint32_t fromVersion;
    uint32_t toVersion;
    std::function<void(SchemaData&)> migrate;
};
```

Rules: no implicit migrations, no downgrades, all migrations logged with
timestamp and result.

### Editor Visualization

Migrations display as a horizontal timeline with version arrows showing the
explicit step chain from oldest to newest schema version.

## 4. Graph Linting & Diff

### GraphLinter

Static analysis pass that detects:

| Warning | Description |
|---------|-------------|
| Unused nodes | Nodes with no outgoing connections. |
| Dead outputs | Output pins never consumed. |
| Constant chains | Chains that always resolve to a constant. |
| Unstable order | Non-deterministic evaluation paths. |
| Redundant math | Identity operations or cancelling pairs. |

### GraphDiff

```cpp
struct DiffOp {
    enum Kind { Added, Removed, Modified };
    Kind kind; NodeId nodeId; std::string detail;
};
std::vector<DiffOp> GraphDiff::Diff(const Graph& a, const Graph& b);
```

### Editor Visualization

- **Green** — added nodes.
- **Red** — removed nodes.
- **Yellow** — modified nodes.
- Timeline slider to scrub between graph revisions.

## 5. Project Hot-Reload

`ProjectHotReload` watches the project directory and polls for changes.

| Asset Type | Behavior |
|------------|----------|
| Schemas | Additions OK; mutations **forbidden** while running. |
| Graphs | Edits OK; structural breaks **forbidden**. |
| Assets | Full re-import on change. |

Hot-reload **never touches live runtime state** — changes stage and swap on the next safe frame boundary.

## 6. Editor Extensions

```cpp
class EditorExtension {
public:
    virtual ~EditorExtension() = default;
    virtual void Register(EditorContext& ctx) = 0;
};
extern "C" EditorExtension* Atlas_CreateEditorExtension();
```

Extensions are optional, crash-isolated, and have no runtime memory access.

## 7. Adapter Test Harness

```cpp
class AdapterTestHarness {
public:
    void LoadAdapter(const std::string& path);
    void FeedOutput(const GraphOutputMock& mock);
    void Verify(const ExpectedState& expected);
};
```

`GraphOutputMock` provides deterministic, canned data. Rules: no engine state, no editor state, no randomness. Headless and reusable by CI, plugin authors, and modders.

## 8. Mod System

### 8.1 Security & Signing

| Trust Level | Description | Multiplayer |
|-------------|-------------|-------------|
| **Unsigned** | Local development only. | Disabled. |
| **Signed** | Author identity verified via public key. | Allowed. |
| **Trusted** | Engine-approved / curated. | Allowed. |

```cpp
struct ModSignature {
    std::string author;
    std::string publicKey;
    std::string signature;  // Base64
};
bool ModVerifier::Verify(const ModSignature& sig, const ModPackage& pkg);
```

`Verify()` is called **before** any mod code is loaded.

### 8.2 Mod Signing CLI

```
atlas sign-mod --input MyMod --author "Author Name"
→ mod.sig (SHA-256 signature) + mod.pub (public key)
```

Offline signing, no engine dependency, standalone CLI tool.

### 8.3 OS-Level Sandboxing

| Platform | Mechanism |
|----------|-----------|
| **Windows** | Job Objects + restricted tokens + DLL load isolation. |
| **Linux** | `fork()` + privilege drop + `seccomp` (allow: `read, write, exit, brk, mmap, munmap`). |
| **macOS** | Helper process + entitlement restriction. |
| **Fallback** | ABI boundary enforcement if OS sandbox unavailable. |

### 8.4 `.atlasmod` Pack Format

Binary header: magic `"ATMD"` (4 ASCII bytes, stored as `0x41 0x54 0x4D 0x44`),
`uint32_t` version, `uint32_t` flags, `uint8_t[32]` SHA-256 content hash.
All multi-byte fields are little-endian.

```
mod.atlasmod
├── header.bin       ├── mod.sig
├── content.pak      └── mod.pub
└── manifest.toml
```

Deterministic file ordering inside `content.pak`. Compression applied before hashing.

### 8.5 Mod Browser UI

```cpp
struct ModEntry {
    std::string id, author, version;
    bool installed, signed, trusted;
};
class ModBrowserModel {
public:
    std::vector<ModEntry> ListAvailable();
    void Install(const std::string& modId);
    void Uninstall(const std::string& modId);
};
```

Permissions confirmation before install. Multiplayer: only **Signed + Deterministic** mods display an "MP-Safe" badge.

### 8.6 Multiplayer Mod Negotiation

```cpp
struct SessionModList {
    struct Entry { std::string modId; uint8_t sha256[32]; };
    std::vector<Entry> mods;
};
```

1. Server sends `SessionModList` (required mods + per-mod SHA-256 hashes).
2. Client compares against local mods.
3. **Exact hash match required** — any mismatch = connection denied.
   Hash comparison MUST be constant-time to prevent timing side-channels.

## 9. Asset Compression & Encryption

| Context | Algorithm | Rationale |
|---------|-----------|-----------|
| Runtime | **LZ4** | Fast decompression. |
| Offline | **Zstd** | High ratio for distribution. |

**AES-256** encryption: keys derived via a proper KDF (e.g., HKDF-SHA256) from
engine version + project ID + optional license key. Applied at pack time;
decrypted in memory; keys never on disk.

| Asset Type | Encrypted |
|------------|-----------|
| Art, audio, binary assets | Yes (optional). |
| Graphs, schemas, bytecode | **Never.** |

> Not DRM — "IP hygiene" and casual extraction deterrence.

## 10. Performance Profiling

```cpp
{ ProfileScope scope("GraphCompile"); /* ... */ }
```

- **Ring buffer** backend — fixed-size, overwrites oldest entries.
- **JSON dump** for external tooling; **editor timeline view** for flame graphs.
- **No OS timers in runtime** — simulation clock ticks only. OS wall-clock
  permitted only in editor tooling and editor-side profiling views.

Target areas: GraphIR→bytecode compilation, VM execution, asset streaming,
editor rendering. Runtime profiling uses tick counts; wall-clock conversion is
an editor-only display concern.

## 11. Deterministic Time & Clock

```cpp
class SimulationClock {
public:
    uint64_t CurrentTick() const;
    double   TickDelta() const;
};
class TickAuthority {
public:
    void Advance();
    void SetRate(double ticksPerSecond);
};
```

**No plugin may query wall-clock.** No `std::chrono` outside the editor. All runtime time flows through `SimulationClock` — mandatory for rollback, replay, and server-authoritative simulation.

## 12. Asset Provenance & Lineage

```cpp
struct AssetProvenance {
    std::string sourceGraph;
    uint64_t    seed;
    std::vector<std::string> mutations;
    uint32_t    version;
    enum Author { Human, AI, System };
    Author      author;
};
```

Enables "why does this asset look like this?" debugging by tracing through graph, seed, and mutation chain. Tracks mod trust level of originating source.

## 13. Project Templates

| Template | Description |
|----------|-------------|
| `Templates/GameMinimal/` | Bare-bones project scaffold. |
| `Templates/2D_TileGame/` | Tile-based 2D starter. |
| `Templates/FPS_Sandbox/` | First-person sandbox starter. |

Flow: editor copies template → rewrites IDs to fresh UUIDs → runs `SchemaValidator` immediately.

## 14. SDK & Distribution

| Component | Description |
|-----------|-------------|
| Engine headers | Public API for adapter development. |
| SDK headers | Helpers, test harness, mock types. |
| CLI tools | `atlas` CLI (build, sign, pack, lint). |
| Templates | Project starters (§13). |
| Documentation | API reference, guides, examples. |

**Versioned** (matches engine major version), **relocatable** (no hard-coded paths), **offline-capable**, **ABI stable per major version**.

## 15. Multiplayer Determinism Testing

```cpp
struct DeterminismTestConfig {
    uint32_t simulations       = 3;     // 1 server + 2 clients
    uint32_t totalTicks        = 10000;
    uint32_t hashIntervalTicks = 60;
};
```

Every 60 ticks, compare state hashes across all instances. Injects artificial latency, rollback rewinds, and reordered packets. **Hash divergence = CI build failure.**

## 16. Reference Project: Arena2D

Complete minimal project demonstrating the full engine plugin contract.

```
projects/arena2d/
├── Plugin.toml
├── arena2d.atlas            (Project manifest)
├── module/                  (IGameModule implementation)
│   ├── Arena2DModule.h
│   ├── Arena2DModule.cpp
│   └── CMakeLists.txt
├── worlds/                  (TileGraph files)
│   └── arena.tilegraph
├── ai/                      (BehaviorGraph files)
│   └── enemy_patrol.behaviorgraph
├── data/                    (Game content)
│   └── enemies.json
├── config/                  (Runtime settings)
│   └── runtime.json
└── assets/                  (Art assets)
```

**Arena2DModule** implements `IGameModule`:
- 3 factions (Heroes, Monsters, Neutrals) with hostility/friendly relations
- 2 economy resources (Gold, XP)
- 3 replication rules (Position: every-tick unreliable, Health/SpriteInfo: on-change reliable)
- 3 server rules (enemySpawnRate, playerDamageMultiplier, xpMultiplier)
- `CreateArena2DModule()` factory for dynamic loading

### Completion Criteria

- [x] Editor paints tiles via the tile schema.
- [x] Characters spawn from `CharacterGraph` evaluation.
- [x] All graphs compile and execute without errors.
- [x] Sprite bake pipeline produces valid atlas output.
- [x] **No engine hacks** — Arena2D uses only public API and adapter ABI.
