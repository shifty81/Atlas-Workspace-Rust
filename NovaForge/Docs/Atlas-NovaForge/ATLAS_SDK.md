# Atlas SDK — Public Developer Guide

Welcome to the Atlas SDK.

Atlas is a deterministic simulation platform designed for long-lived
projects, replay stability, and safe engine evolution.

---

## What You Get

The SDK provides:

- Stable C ABI capsules
- Deterministic scripting APIs
- Project templates
- Atlas Editor integration
- Replay & migration tooling

You do **not** get:

- Engine internals
- Rendering backends
- Editor source
- Private headers

---

## Project Creation

Create a new project using one of two scaffolders:

### Constitution-Compliant Project (Editor-focused)

```bash
python tools/create_atlas_project.py MyGame
```

This generates a `.atlas` project with determinism traits, schema
versioning, and replay/migration directories. Open the result in the
Atlas Editor.

### Full Game Module (CMake-focused)

```bash
python tools/atlas_init.py --name MyGame --namespace mygame
```

This generates a game module with `Plugin.toml`, `CMakeLists.txt`, and
C++ scaffolding that links against `AtlasEngine` and `AtlasGameplay`.

---

## .atlas Project File

```json
{
  "project": {
    "name": "MyGame",
    "version": "1.0.0",
    "binary_abi": "atlas_abi_v1",
    "determinism_profile": "fixed32_v1",
    "requires": {
      "replay_support": true,
      "migration_support": true
    }
  }
}
```

---

## System Rules (Mandatory)

Every system must declare determinism traits:

```cpp
ATLAS_SYSTEM_TRAITS(MySystem,
    (SystemTraits{ "MySystem", true, true, true, __FILE__ })
);
```

Missing traits will block:

- Builds
- Replays
- Distribution

---

## Determinism Rules

**Allowed:**

- Fixed-point math
- Ordered containers (`std::map`, `std::set`)
- Seeded RNG (`DeterministicRNG`)
- Single-threaded logic

**Forbidden:**

- OS time (`std::chrono`, `time()`, `clock()`)
- Filesystem access (`fopen`, `std::ifstream`)
- Threads (`std::thread`, `std::mutex`)
- Unordered containers (`std::unordered_map`)
- Floating-point equality
- Platform SIMD intrinsics

See `tools/determinism_rules.yaml` for the complete machine-readable
rule set.

---

## Scripts

**Supported runtimes:**

- Lua (no JIT)
- Wren

**Script rules:**

- Run during simulation tick only
- Are sandboxed
- Are hashed and replay-validated

---

## Replays & Saves

Atlas guarantees:

- Replays remain valid across engine versions
- Saves load via migrations
- Desyncs are detectable and reproducible

Breaking replay compatibility without migration is **forbidden**.

---

## Binary Compatibility (ABI)

Projects target an explicit ABI:

```json
"binary_abi": "atlas_abi_v1"
```

This allows:

- Running old binaries on new engines
- Safe engine upgrades
- Long-term project support

---

## CI & Validation

All Atlas projects are validated for:

- Determinism (contract scanner)
- Replay safety (golden replay checks)
- Migration safety (schema version tracking)
- ABI compatibility

Invalid projects will not load.

---

## Distribution

Projects can be:

- Shipped standalone
- Distributed via Steam
- Run on dedicated servers

The engine remains independent.

---

## Marketplace Asset Integration

Atlas supports importing assets from external marketplaces:

- **itch.io** — `.obj`, `.fbx`, `.png` via REST API
- **Unreal Marketplace** — `.uasset` binary header parsing and conversion
- **Unity Asset Store** — `.prefab` YAML parsing and conversion

All imported assets are:

- Hash-verified for determinism
- Immutable once imported
- Tracked in the asset dependency graph

See `docs/MARKETPLACE_IMPORTING.md` for setup details.

---

## Tools

| Tool | Purpose |
|------|---------|
| `tools/create_atlas_project.py` | Create constitution-compliant project |
| `tools/atlas_init.py` | Create full game module scaffold |
| `tools/contract_scan.py` | Scan for contract violations |
| `tools/replay_minimizer.py` | Reduce failing replays to minimal repro |
| `tools/replay_inspector.py` | Inspect replay contents |
| `tools/crash_reporter.py` | Bundle crash reports |
| `tools/state_diff_viewer.py` | View state diffs |
| `tools/verify_dependencies.sh` | Verify build dependencies |

---

## Final Rule

Atlas enforces correctness automatically.
If something is blocked, it is protecting your project.
