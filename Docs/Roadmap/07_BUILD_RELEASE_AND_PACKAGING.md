> **HISTORICAL** — This document describes a completed foundation phase (Phases 0–71).
> The canonical source of truth for current and future phases is [00_MASTER_ROADMAP.md](00_MASTER_ROADMAP.md).

# Build, Release, and Packaging

**Phase 6 of the Master Roadmap**

## Purpose

Stabilize build pipeline, patch workflow, and define release path.

## Tasks

### Build Presets
- [ ] Verify all presets work as documented
- [ ] Ensure CI presets support offline builds
- [ ] Document preset selection guide

### Dependency Policy
- [ ] Vendor critical dependencies where feasible
- [ ] Gate all online fetches behind options
- [ ] Maintain THIRD_PARTY_DEPENDENCY_REGISTER.md

### Patch Workflow
- [ ] Deterministic patch apply scripts
- [ ] Context menu registration/removal pairs
- [ ] Patch uninstall capability

### Release Pipeline
- [ ] Define release packaging rules
- [ ] Define version numbering strategy
- [ ] Define release validation checklist
