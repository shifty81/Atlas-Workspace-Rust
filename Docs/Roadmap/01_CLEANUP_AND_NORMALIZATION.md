> **HISTORICAL** — This document describes a completed foundation phase (Phases 0–71).
> The canonical source of truth for current and future phases is [00_MASTER_ROADMAP.md](00_MASTER_ROADMAP.md).

# Cleanup and Normalization

**Phase 0 of the Master Roadmap**

## Purpose

Bring the repo into a clean, normalized, canonical state before any feature expansion.

## Tasks

### Repo Hygiene
- [x] Add `.gitattributes` with line-ending rules
- [x] Tighten `.gitignore` (build_verify, Binaries, Intermediate, x64, Debug, Release)
- [x] Gate test dependencies behind `ATLAS_ENABLE_ONLINE_DEPS`
- [ ] Verify `vcpkg.json` contains only live dependencies

### Naming Cleanup
- [x] Rename active-path Arbiter references to AtlasAI
- [x] Rename `Atlas/Workspace/Arbiter/` → `Atlas/Workspace/AtlasAI/`
- [x] Clean Arbiter references in Source/Pipeline/
- [x] Clean Arbiter references in Tests/Pipeline/
- [x] Clean Arbiter references in Source/Editor/
- [x] Absorb SwissAgent into AtlasAI broker (SwissAgentAdapter removed, AtlasAIAdapter unified)
- [x] Remove stale Atlas Suite, MasterRepo naming from docs

### Documentation Reset
- [x] Rewrite README.md
- [x] Create Docs/Canon/* docs
- [x] Create Docs/Roadmap/* docs
- [x] Create Docs/Inventory/* docs
- [ ] Move stale/conflicting docs to Docs/Archive/
- [ ] Archive historical SpecPack content if it conflicts with SpecRollup

### Build Hygiene
- [x] Ensure `ATLAS_BUILD_TESTS` defaults to OFF
- [x] Ensure `ATLAS_ENABLE_ONLINE_DEPS` defaults to OFF
- [ ] Verify offline configure works cleanly

## Verification

After completion, run:
```
grep -r "Arbiter" Source/ Tests/ --include="*.cpp" --include="*.h"
grep -r "FetchContent_Declare" Tests/ --include="*.txt"
```

Expected: zero Arbiter hits in active paths, fetch only in guarded blocks.
