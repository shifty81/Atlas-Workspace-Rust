# Repo Audit Checklist

Use this checklist to audit the repo for missing features and direction drift.

## A. Direction and naming
- [x] AtlasUI is named and treated as the canonical tooling UI framework
- [x] Old ImGui references are removed or clearly archived/deferred
- [x] Old Arbiter naming is replaced by AtlasAI where intended (SwissAgent also absorbed into AtlasAI)
- [x] Workspace naming is consistent and user-facing naming is not drifting
- [x] Legacy experimental tool names do not appear as active canonical paths

## B. UI framework
- [x] backend abstraction exists
- [x] panels are backend-agnostic
- [x] GDI fallback exists
- [x] GPU path is present or actively scaffolded
- [x] shared widget kit exists
- [x] theme/token system exists
- [x] command/shortcut layer exists
- [x] menu/context menu layer exists
- [x] docking exists
- [x] floating host exists
- [x] tabs/chrome exist
- [x] tooltip system exists

## C. Tooling shell
- [x] workspace shell exists or is clearly scaffolded
- [x] notification center exists or is scaffolded
- [x] AtlasAI panel host exists or is scaffolded
- [x] settings/control panel exists or is scaffolded
- [x] intake flow exists or is scaffolded

## D. Persistence and usability
- [x] layout persistence exists
- [x] panel ids are stable
- [x] focus handling is standardized
- [x] property grid exists
- [x] tree/list/table systems exist
- [x] scroll/virtualization exists
- [x] typography and icon rules are defined

## E. Logging and debugging
- [x] build logs route to a logger format
- [x] logs can be surfaced in tooling
- [x] error notifications can escalate to AtlasAI
- [x] debug/fix workflow is spec'd

## F. Roadmap order
- [x] tooling-first roadmap is reflected in docs
- [x] game work is behind tooling unless explicitly selected
- [x] AtlasUI/framework tasks are near the top of the roadmap

## G. Repo hygiene
- [x] obsolete experiments are archived or clearly marked
- [x] duplicate UI implementations are reconciled
- [x] docs match current direction
- [x] test outputs route to bin/Tests

