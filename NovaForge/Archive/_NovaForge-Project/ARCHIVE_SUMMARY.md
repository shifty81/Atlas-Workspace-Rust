# Archive: shifty81/NovaForge-Project

**Archived:** 2025-07-15
**Source:** https://github.com/shifty81/NovaForge-Project
**Commit SHA:** `1994806e258a56810d78f4979fd8ebb0f30573d8`
**Merge Phase:** Phase 6
**Status:** ✅ Complete

## Repo Overview

NovaForge-Project is the standalone game project hosted by Atlas Suite. It contains a voxel-first
open-world/open-galaxy survival/building game implemented in C++ (CMake) with C# integration
services. The repo includes game source code, Atlas Suite integration layers, AI project adapters,
content/data definitions, configuration, packaging profiles, scripts, tests, and comprehensive
documentation (architecture, specs, rules, roadmap).

Key characteristics:
- C++ CMake game source under `NovaForge/` with Client, Server, Gameplay, World, UI, Save modules
- Atlas Suite HTTP bridge integration (`http://127.0.0.1:8005`)
- Project manifest v2 (`novaforge.project.json`) declaring paths, build targets, capabilities
- 20+ gameplay subsystems (Combat, Factions, Mining, Salvage, Economy, Missions, etc.)
- Rich data definitions (ships, skills, universe, NPCs, contracts, etc.)
- CI/CD via GitHub Actions (`validate.yml`)
- Packaging profiles for Client/Server dev and release builds

## Source Repo Structure

```
NovaForge-Project/
├── .github/workflows/validate.yml          # CI validation pipeline
├── .gitignore
├── ARCHITECTURE.md                         # Architecture overview
├── BUILD_RULES.md                          # Build rules and CMake module graph
├── CONTENT_RULES.md                        # Content layout and rules
├── LICENSE                                 # GPL-3.0
├── PROJECT_RULES.md                        # Project generation contract
├── README.md                               # Project overview
├── ROADMAP.md                              # Development phases
├── TASKS.md                                # Task tracking
├── novaforge.project.json                  # Project manifest v2
├── AtlasAI/ProjectAdapters/NovaForge/      # AI project adapter (bridge, ingestion)
├── Config/Client/, Config/Server/          # Config dirs (empty placeholders)
├── Content/                                # Content stubs (Buildables, Data, Definitions, etc.)
├── Docs/
│   ├── ATLAS_SUITE_NOVAFORGE_ALIGNMENT_ROLLUP.md
│   ├── NovaForge_First_Live_Ingestion_Plan.md
│   ├── Migration/README.md, SOURCE_IMPORT_POLICY.md
│   ├── NovaForge/NOVAFORGE_VOXEL_FIRST_DIRECTIVES.md, Phases/, builder/, data/, etc.
│   ├── Runtime/EXPANDED_RUNTIME_GAMEPLAY_SPEC.md
│   └── Specs/ (8 contract/schema docs)
├── Intake/manifests/, reports/, staging/   # Content intake pipeline
├── Integrations/AtlasSuite/                # Runtime bridge, Adapter, Plugins
├── NovaForge/                              # C++ game source (CMake root)
│   ├── CMakeLists.txt, client_main.cpp, server_main.cpp
│   ├── App/, Client/, Server/, Shared/     # Core modules
│   ├── Gameplay/ (22 subsystems)           # Combat, Factions, Mining, Salvage, etc.
│   ├── World/, UI/, Save/, SaveLoad/       # Infrastructure modules
│   ├── Combat/, Constructs/, Economy/, Hazards/, Interaction/, Inventory/, Missions/, Player/
│   ├── Factions/, Repair/, Runtime/, Salvage/, Scenes/
│   ├── Config/, Content/, Data/, DevWorld/, Docs/, EditorTools/, Integrations/, Tools/, Tests/
│   └── CMake/
├── Packaging/Profiles/                     # ClientDev, ClientRelease, ServerRelease
├── Release/version.json, Notes/            # Release management
├── Scripts/                                # 9 build/validation/bootstrap scripts
└── Tests/NovaForge.Tests/                  # 3 test scripts
```

## Extraction Summary

| Source Path | Destination | Status | Notes |
|---|---|---|---|
| `README.md` | `Docs/NovaForge-Project/README.md` | ✅ Extracted | Project overview |
| `ARCHITECTURE.md` | `Docs/NovaForge-Project/ARCHITECTURE.md` | ✅ Extracted | Architecture doc |
| `BUILD_RULES.md` | `Docs/NovaForge-Project/BUILD_RULES.md` | ✅ Extracted | Build rules & CMake graph |
| `CONTENT_RULES.md` | `Docs/NovaForge-Project/CONTENT_RULES.md` | ✅ Extracted | Content layout rules |
| `PROJECT_RULES.md` | `Docs/NovaForge-Project/PROJECT_RULES.md` | ✅ Extracted | Generation contract |
| `ROADMAP.md` | `Docs/NovaForge-Project/ROADMAP.md` | ✅ Extracted | Development roadmap |
| `TASKS.md` | `Docs/NovaForge-Project/TASKS.md` | ✅ Extracted | Task tracking |
| `Docs/ATLAS_SUITE_NOVAFORGE_ALIGNMENT_ROLLUP.md` | `Docs/NovaForge-Project/ATLAS_SUITE_NOVAFORGE_ALIGNMENT_ROLLUP.md` | ✅ Extracted | Alignment directive |
| `Docs/NovaForge_First_Live_Ingestion_Plan.md` | `Docs/NovaForge-Project/NovaForge_First_Live_Ingestion_Plan.md` | ✅ Extracted | Ingestion plan |
| `Docs/Migration/README.md` | `Docs/NovaForge-Project/Migration/README.md` | ✅ Extracted | Migration overview |
| `Docs/Migration/SOURCE_IMPORT_POLICY.md` | `Docs/NovaForge-Project/Migration/SOURCE_IMPORT_POLICY.md` | ✅ Extracted | Import policy |
| `Docs/NovaForge/NOVAFORGE_VOXEL_FIRST_DIRECTIVES.md` | `Docs/NovaForge-Project/NovaForge/NOVAFORGE_VOXEL_FIRST_DIRECTIVES.md` | ✅ Extracted | Voxel-first directives |
| `Docs/Runtime/EXPANDED_RUNTIME_GAMEPLAY_SPEC.md` | `Docs/NovaForge-Project/Runtime/EXPANDED_RUNTIME_GAMEPLAY_SPEC.md` | ✅ Extracted | Runtime gameplay spec |
| `Docs/Specs/BUILD_AND_PACKAGE_CONTRACT.md` | `Docs/NovaForge-Project/Specs/BUILD_AND_PACKAGE_CONTRACT.md` | ✅ Extracted | Build/package contract |
| `Docs/Specs/CONTENT_DATA_CONTRACT.md` | `Docs/NovaForge-Project/Specs/CONTENT_DATA_CONTRACT.md` | ✅ Extracted | Content data contract |
| `Docs/Specs/EDITOR_SURFACE_CONTRACT.md` | `Docs/NovaForge-Project/Specs/EDITOR_SURFACE_CONTRACT.md` | ✅ Extracted | Editor surface contract |
| `Docs/Specs/HOSTED_PROJECT_CONTRACT.md` | `Docs/NovaForge-Project/Specs/HOSTED_PROJECT_CONTRACT.md` | ✅ Extracted | Hosted project contract |
| `Docs/Specs/PROJECT_MANIFEST_SCHEMA.md` | `Docs/NovaForge-Project/Specs/PROJECT_MANIFEST_SCHEMA.md` | ✅ Extracted | Manifest schema |
| `Docs/Specs/RELEASE_OPERATIONS.md` | `Docs/NovaForge-Project/Specs/RELEASE_OPERATIONS.md` | ✅ Extracted | Release operations |
| `Docs/Specs/RUNTIME_EDITOR_BRIDGE.md` | `Docs/NovaForge-Project/Specs/RUNTIME_EDITOR_BRIDGE.md` | ✅ Extracted | Runtime bridge spec |
| `Docs/Specs/SAVE_PROFILE_ENVIRONMENT.md` | `Docs/NovaForge-Project/Specs/SAVE_PROFILE_ENVIRONMENT.md` | ✅ Extracted | Save/profile spec |
| `Scripts/bootstrap_repo.ps1` | `Archive/_NovaForge-Project/scripts/bootstrap_repo.ps1` | ✅ Extracted | Repo bootstrap (PS) |
| `Scripts/bootstrap_repo.sh` | `Archive/_NovaForge-Project/scripts/bootstrap_repo.sh` | ✅ Extracted | Repo bootstrap (bash) |
| `Scripts/copilot_bootstrap_prompt.txt` | `Archive/_NovaForge-Project/scripts/copilot_bootstrap_prompt.txt` | ✅ Extracted | Copilot prompt |
| `Scripts/local_commands.md` | `Archive/_NovaForge-Project/scripts/local_commands.md` | ✅ Extracted | Local commands reference |
| `Scripts/package.sh` | `Archive/_NovaForge-Project/scripts/package.sh` | ✅ Extracted | Packaging script |
| `Scripts/validate_content.sh` | `Archive/_NovaForge-Project/scripts/validate_content.sh` | ✅ Extracted | Content validation |
| `Scripts/validate_project.ps1` | `Archive/_NovaForge-Project/scripts/validate_project.ps1` | ✅ Extracted | Project validation (PS) |
| `Scripts/validate_project.sh` | `Archive/_NovaForge-Project/scripts/validate_project.sh` | ✅ Extracted | Project validation (bash) |
| `Scripts/verify_build.sh` | `Archive/_NovaForge-Project/scripts/verify_build.sh` | ✅ Extracted | Build verification |
| `Tests/NovaForge.Tests/run_tests.sh` | `Archive/_NovaForge-Project/tests/run_tests.sh` | ✅ Extracted | Test runner |
| `Tests/NovaForge.Tests/test_bridge_roundtrip.sh` | `Archive/_NovaForge-Project/tests/test_bridge_roundtrip.sh` | ✅ Extracted | Bridge roundtrip test |
| `Tests/NovaForge.Tests/test_manifest.sh` | `Archive/_NovaForge-Project/tests/test_manifest.sh` | ✅ Extracted | Manifest validation test |
| `novaforge.project.json` | `Archive/_NovaForge-Project/novaforge.project.json` | ✅ Extracted | Project manifest v2 |
| `.github/workflows/validate.yml` | `Archive/_NovaForge-Project/workflows/validate.yml` | ✅ Extracted | CI workflow |
| `Release/version.json` | `Archive/_NovaForge-Project/version.json` | ✅ Extracted | Version config |
| `Packaging/Profiles/ClientDev.json` | `Archive/_NovaForge-Project/packaging/ClientDev.json` | ✅ Extracted | Dev packaging profile |
| `Packaging/Profiles/ClientRelease.json` | `Archive/_NovaForge-Project/packaging/ClientRelease.json` | ✅ Extracted | Client release profile |
| `Packaging/Profiles/ServerRelease.json` | `Archive/_NovaForge-Project/packaging/ServerRelease.json` | ✅ Extracted | Server release profile |
| `Intake/manifests/legacy_source_manifest_template.csv` | `Archive/_NovaForge-Project/intake/legacy_source_manifest_template.csv` | ✅ Extracted | Legacy manifest template |

## Skipped Content

| Source Path | Reason |
|---|---|
| `NovaForge/` (entire C++ game source) | Game source already consolidated in main repo; this is the canonical copy in the source repo |
| `Integrations/AtlasSuite/` | Integration layer already present in consolidated repo structure |
| `AtlasAI/ProjectAdapters/` | AI adapter already present in consolidated repo structure |
| `Content/` (11 empty subdirs) | Empty placeholder directories (`.gitkeep` only) |
| `Config/Client/`, `Config/Server/` | Empty placeholder directories (`.gitkeep` only) |
| `Intake/staging/`, `Intake/reports/.gitkeep` | Empty staging/placeholder directories |
| `Release/Notes/` | Empty placeholder directory |
| `LICENSE` | Standard GPL-3.0; already present in consolidated repo root |
| `.gitignore` | Repo-specific; not applicable to consolidated repo |
| `Docs/NovaForge/Phases/`, `builder/`, `data/`, `factions/`, `gameplay/`, `world/` | Empty placeholder directories (`.gitkeep` only) |
| `NovaForge/CMake/.gitkeep` | Empty placeholder |
| `Packaging/Profiles/.gitkeep` | Empty placeholder |
| `Tests/NovaForge.Tests/.gitkeep` | Empty placeholder |

## Migration Checklist

- [x] Audit source repo contents (full tree exploration of all directories)
- [x] Identify usable code, docs, and assets
- [x] Extract docs (21 files) to `Docs/NovaForge-Project/`
- [x] Extract scripts (9 files) to `Archive/_NovaForge-Project/scripts/`
- [x] Extract tests (3 files) to `Archive/_NovaForge-Project/tests/`
- [x] Extract reference configs (7 files) to `Archive/_NovaForge-Project/`
- [x] Verify all 41 extracted files are non-zero
- [x] Skip already-consolidated game source (NovaForge/, Integrations/, AtlasAI/)
- [x] Skip empty placeholder directories
- [x] Update ARCHIVE_SUMMARY.md with full audit
- [x] Mark phase as ✅ Done
