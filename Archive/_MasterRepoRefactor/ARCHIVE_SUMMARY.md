# Archive: MasterRepoRefactor

**Archived:** 2026-04-05
**Source:** https://github.com/shifty81/MasterRepoRefactor
**Merge Phase:** C1 — Structure + Atlas dirs
**Status:** ✅ Merged

## What Was Taken

| Item | Destination in tempnovaforge |
|------|------------------------------|
| Atlas directory layout concept | `AtlasAI/` (Atlas_Arbiter, Atlas_SwissAgent) |
| Naming convention (Atlas_ prefix) | `Docs/Architecture/NAMING_CANON.md` |
| Hosted project contract | `Docs/Architecture/HOSTED_PROJECT_CONTRACT.md` |
| Build mode options (STANDALONE/HOSTED) | Root `CMakeLists.txt` (NF_STANDALONE, NF_HOSTED) |
| Project manifest schema | `Project/project.atlas.json` |
| Architecture direction | `Docs/Architecture/CURRENT_DIRECTION.md` |
| Deferred items list | `Docs/Architecture/DEFERRED_TO_WORKSPACE.md` |
| Gap audit methodology | `Docs/Architecture/TEMPNOVAFORGE_GAP_AUDIT.md` |

## What Was NOT Taken (and why)

| Item | Reason |
|------|--------|
| Source/ tree | tempnovaforge Source/ is canonical with G1-G20 game phases (641 tests). MasterRepoRefactor's Source/ was a subset. |
| Tests/ directory | tempnovaforge has 13 test executables with comprehensive coverage. MasterRepoRefactor's test layout was less mature. |
| Duplicate docs (ACTION_MATRIX, PHASE_*) | Will be reviewed and absorbed in follow-up consolidation passes (C3+). |
| Build scripts | tempnovaforge already has `Scripts/build_all.sh`, `generate_vs_solution.bat/.ps1`, and `Scripts/validate_project.sh`. |

## Key Context

MasterRepoRefactor was the first structural refactoring of the original MasterRepo.
Its primary contribution to tempnovaforge was the **Atlas naming convention** and the
**directory layout** that organizes AI tools under a single `AtlasAI/` broker.

**tempnovaforge is ahead of MasterRepoRefactor:** By archive time, tempnovaforge had
completed G1-G20 game phases (641 Catch2 tests), the full S0 pipeline core, real Win32
editor window, and comprehensive architecture documentation. MasterRepoRefactor's
structural contributions have been fully absorbed.

## Usable Code Snippets

See `usable_snippets/` subfolder.
