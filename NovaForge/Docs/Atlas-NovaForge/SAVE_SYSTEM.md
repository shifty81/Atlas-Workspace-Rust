# Atlas Save System

> Specification for the Atlas deterministic save/load system.
>
> See also: `ATLAS_CORE_CONTRACT.md`, `14_REPLAY_AND_PROOFS.md`

---

## Overview

The save system captures a `WorldSnapshot` plus replay metadata into a
versioned binary file. Loading a save restores the exact simulation state
so that all subsequent ticks match a reference replay.

### Design Principles

- A save file is a snapshot of `SimulatedState` at a specific tick.
- After load, the simulation must produce bit-identical results to
  continuing from that tick without the save/load boundary.
- Save format is versioned for forward compatibility.
- Hash integrity is verified on every load.

---

## File Format (`.asav`)

### Header (48 bytes)

| Offset | Size | Field | Description |
|--------|------|-------|-------------|
| 0 | 4 | `magic` | `0x41534156` (`"ASAV"`) |
| 4 | 4 | `version` | Format version (currently `1`) |
| 8 | 4 | `tickRate` | Simulation tick rate at save time |
| 12 | 8 | `saveTick` | Tick number when save was taken |
| 20 | 8 | `stateHash` | Deterministic hash of saved data |
| 28 | 4 | `seed` | World generation seed |
| 32 | 4 | `ecsDataSize` | Size of serialized ECS data |
| 36 | 4 | `auxDataSize` | Size of auxiliary data |
| 40 | 4 | `metadataSize` | Size of metadata string |

### Body

| Section | Description |
|---------|-------------|
| ECS data | Serialized entity/component state |
| Auxiliary data | Additional simulation state (optional) |
| Metadata | Human-readable string (save name, timestamp, etc.) |

All multi-byte values are little-endian.

---

## Hash Verification

On save, the system computes a deterministic hash over all saved state
using `StateHasher::HashCombine()`. On load, the hash is recomputed
and compared to the stored value. A mismatch produces `SaveResult::HashMismatch`.

---

## Save Results

| Result | Meaning |
|--------|---------|
| `Success` | Operation completed successfully |
| `FileNotFound` | Save file does not exist |
| `InvalidFormat` | File is not a valid `.asav` file |
| `VersionMismatch` | File version is unsupported |
| `HashMismatch` | Data integrity check failed |
| `IOError` | Filesystem I/O error |

---

## Console Commands

The editor console supports save/load operations:

| Command | Description |
|---------|-------------|
| `save <path>` | Save current world state to file |
| `load <path>` | Load world state from file |
| `time.info` | Display current tick, rate, and world time |

---

## Engine Integration

The `Engine` class owns a `SaveSystem` instance accessible via
`Engine::GetSaveSystem()`. The save system uses the engine's ECS
serialization and TimeModel to capture state at the correct tick.

---

## Files

| File | Purpose |
|------|---------|
| `engine/sim/SaveSystem.h` | Header with `SaveHeader`, `SaveResult`, `SaveSystem` |
| `engine/sim/SaveSystem.cpp` | Implementation |
| `tests/test_save_system.cpp` | Unit tests (7 tests) |
| `tests/test_engine_integration.cpp` | Engine integration tests |
| `editor/panels/ConsolePanel.cpp` | Console save/load commands |
