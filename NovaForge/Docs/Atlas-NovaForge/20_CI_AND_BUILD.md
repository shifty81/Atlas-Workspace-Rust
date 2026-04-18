# Atlas CI Policies & Build System

## Overview

- CI is a gate, not a suggestion
- Determinism is enforced at build time, runtime, and in CI
- One-command build produces ready-to-use software
- First-run experience is "zero-friction" — no IDE required

## Build System

### Build Script Contract

The build script must:

1. Build all primary executables (Editor, Client, Server, CLI)
2. Create `/run` directory with shortcuts
3. Validate the output
4. Print a human-readable success screen

### Post-Build Layout

```
Atlas/
├─ build/
│  ├─ editor/AtlasEditor
│  ├─ client/AtlasClient
│  ├─ server/AtlasServer
│  └─ tools/AtlasCLI
├─ run/
│  ├─ AtlasEditor (symlink/shortcut)
│  ├─ AtlasClient (symlink/shortcut)
│  ├─ AtlasServer (symlink/shortcut)
│  └─ README_FIRST.txt
├─ projects/
├─ docs/
└─ atlas.build.json
```

### atlas.build.json Schema

Defines what a "ready-to-use Atlas build" means:

- Build configuration (generator, configuration, deterministic flag, hash algorithm)
- Artifact paths and expected hashes
- Run shortcut definitions
- Validation requirements (required files, hash checks)

### First-Run Experience

1. Clone repo
2. Run one script (`./build.sh` or `build_atlas.ps1`)
3. Double-click Atlas Editor in `/run`
4. Start building immediately

Rule: If a fresh machine cannot double-click AtlasEditor and build a game in under
5 minutes, the build is broken.

## CI Policies

### Determinism CI Gate

CI must fail if:

- Golden replay hashes differ
- Replay diverges on another platform
- Determinism violations are logged
- State schemas change without migration
- `determinism.json` changes without explicit version bump

### CI Workflow: atlas_determinism.yml

- Configure with strict determinism flags
- Build in Release mode
- Run golden replay tests
- Compare hash outputs against stored hashes
- Cross-run verification
- Fail on any contract violation in logs

### Contract Bot

Automated PR review bot that:

- Scans for forbidden API usage in simulation code (`std::chrono`, `rand()`, `time()`, `<random>`)
- Comments on PRs with specific violation details
- References `ATLAS_CORE_CONTRACT.md`
- Implemented via `tools/contract_scan.py`

### CI Artifacts

Every CI run produces:

- Replay hash comparison results
- Hash ladder CSV
- UI replay videos (exported from deterministic GUI replay)
- Proof counterexamples (if TLA+ checks fail)
- Lint and graph check results

### CI Output Format

Structured JSON output for dashboard integration:

- `lint_issues` — array of violations
- `proof_mismatches` — array of tick divergences
- `hash_ladder` — array of step hashes
- `replay_status` — pass/fail with details

## Build Manifest

Every build emits `build_manifest.json`:

```json
{
  "editor_hash": "...",
  "client_hash": "...",
  "layout_hash": "...",
  "dsl_version": 1
}
```

Used for: replay verification, CI dashboards, proof viewer integration.

## Determinism Build Profiles

- **Strict** — all determinism enforcement active, CI mode
- **Development** — warnings for violations, hot-reload enabled
- **Debug** — additional diagnostics, no enforcement

## Certified Build Mode

- Full hash verification of all artifacts
- Proof validation pass
- Replay stability confirmation
- Generates certification artifacts for audit
