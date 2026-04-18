# Atlas_Arbiter — Rule-Based Decision Engine

> **Canonical Name:** Atlas_Arbiter (part of AtlasAI)
> **Legacy Name:** ArbiterAI
> **Legacy Path:** `Tools/ArbiterAI/`

## Overview

Atlas_Arbiter is a deterministic rule engine that evaluates declarative rules against
JSON context snapshots. It is used for automated code quality enforcement, game balance
validation, and pipeline decision-making.

## Capabilities

1. Parse and evaluate declarative rule files (`.arbiter.json`)
2. Evaluate rules against world state, code context, or build output
3. Produce findings as pipeline `ChangeEvent` files
4. Integrate with CI for automated quality gates

## Roadmap

| Task | Description | Status |
|------|-------------|--------|
| AB-1 | Rule format definition and parser | ⬜ Queued |
| AB-2 | CLI evaluation engine | ⬜ Queued |
| AB-3 | Game balance rule set | ⬜ Queued |
| AB-4 | Editor panel integration | ⬜ Queued |
| AB-5 | CI gate integration | ⬜ Queued |

## Pipeline Contract

- **Reads:** `pipeline/worlds/active.world.json`
- **Writes:** `pipeline/changes/<timestamp>_atlasai_ContractIssue.change.json`
- **Never** writes directly to source files

## Reference Implementation

The legacy reference implementation is at `Tools/ArbiterAI/`. See its README for
the original design notes. Active development will occur in this directory once
S1 Tool Wiring is complete.
