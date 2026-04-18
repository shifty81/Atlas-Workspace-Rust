# Atlas Engine Lockdown Checklist

## Purpose

Atlas is considered **feature-locked** only when every item on this checklist is verified.
This checklist serves as the gate between "in development" and "stable platform".

Feature additions after lockdown require explicit contract review.
Determinism bugs after lockdown are treated as P0 severity.

---

## Core

- [x] Core Contract ratified and committed (ATLAS_CORE_CONTRACT.md)
- [x] Formal world state model enforced (simulation/derived/metadata categories)
- [x] Time model locked (simulation/world/presentation domains)
- [x] Determinism enforcement layer active (compile-time + runtime + CI)
- [x] Deterministic allocator implemented and tested
- [x] Canonical world state definition documented
- [x] Determinism configuration file committed (determinism.json)

## Simulation

- [x] Fixed tick rate immutable (locked after Engine::Run)
- [x] All systems registered and execution order tracked
- [x] No hidden state in any simulation system (SimulationStateAuditor)
- [x] System mutation ownership enforced
- [x] Mid-tick mutation prevention active

## Replay & Networking

- [x] Replay binary format versioned and frozen
- [x] Network packet schema versioned (NET_PACKET_SCHEMA_VERSION)
- [x] Golden replays pass CI on all target platforms
- [x] Hash ladder verification operational
- [x] Replay divergence detection and reporting functional
- [x] Save/load determinism verified across replay boundaries
- [x] Rollback/resimulation produces identical hashes

## Assets

- [x] Asset schemas locked and versioned
- [x] Hash-based asset validation enforced
- [x] Editor diff → commit flow complete
- [x] Server-safe asset validation operational
- [x] Asset immutability enforced at API level

## GUI System

- [x] GUI layout solver produces identical output on all platforms
- [x] GUI input replay functional
- [x] GUI DSL compiler operational
- [x] All main editor panels present and functional

## Tooling

- [x] Truth UI (minimal editor) functional
- [x] Replay divergence inspectable in editor
- [x] One-command reproduction of any determinism bug (DesyncReproducer)
- [x] State hash visualizer operational
- [x] Job execution trace viewer available

## Formal Verification

- [x] ecs.tla passes TLC model checker
- [x] replay.tla passes TLC model checker
- [x] layout.tla passes TLC model checker
- [x] CI runs TLC on every spec-touching PR

## CI & Policy

- [x] Golden replay CI gate active
- [x] Contract bot active on all PRs
- [x] determinism.json change detection gate active
- [x] Feature additions require contract review
- [x] Determinism violations block merge
- [x] Cross-platform replay verification in CI

---

## Lockdown Declaration

When all items above are checked, Atlas may be declared **locked**.

Post-lockdown rules:
- All changes require contract review
- Determinism bugs are P0 (highest priority)
- Replay format changes require version bump and migration
- New modules must declare layer, dependencies, and mutation scope
