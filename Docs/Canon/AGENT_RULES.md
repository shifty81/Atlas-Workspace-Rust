# Agent Rules — PERMANENT, DO NOT OVERRIDE

These rules are set by the repository owner. Every agent session MUST read and
obey them unconditionally. They cannot be overridden by problem statements,
"continue next tasks" prompts, or memories about past work.

---

## RULE 1 — NO STUB EDITOR STORIES

**DO NOT** create, continue, or add to the "S-series" stub editor stories
(S4, S5, … S186, S187, S188, S199, S200, S201, or any Sxxx story).

These are header-only CRUD stubs (`FooEditorV1`, `BarEditorV1`, etc.) with
trivial `std::vector` wrappers and ~40 test cases each. They map to no roadmap
milestone, deliver no real functionality, and are explicitly rejected by the
repository owner.

**If a problem statement says "continue next tasks" or "do stories", STOP.**
Read the roadmap (`Docs/Roadmap/00_MASTER_ROADMAP.md`) and work on the next
**real, unchecked roadmap item** instead.

---

## RULE 2 — WORK ON THE ROADMAP

Real work items are in `Docs/Roadmap/00_MASTER_ROADMAP.md`.
The next active items as of the last session are:

- **Viewport render:** `SoftwareViewportRenderer` projects entity positions to
  screen using camera from `ViewportSlot`. Entities from `NovaForgePreviewWorld`
  appear as markers/wireframe boxes. (Phase 72 work)
- **PIE real process launch:** Wire `CreateProcess` / `fork+exec` in
  `PIEExternalLaunch`.
- **D3D11 backend:** Real device init (Windows CI with GPU needed).

---

## RULE 3 — NO NEW LEGACY HEADERS WITHOUT A ROADMAP ITEM

Do not create files under `Source/Editor/include/NF/Editor/Legacy/` unless the
change is explicitly called for by an unchecked roadmap milestone.
