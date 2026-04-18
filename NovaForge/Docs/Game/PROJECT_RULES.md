# NovaForge — Project Rules

## Hard Boundary

The following must **never** appear inside `Source/Game/` or any engine module:

- AtlasAI broker systems, agent protocols, or AI routing code
- SwissAgent, ArbiterAI, ContractScanner, or ReplayMinimizer integration code
- Any `AtlasAI/` directory reference from engine/game headers

**AtlasAI lives under `AtlasAI/` at the repo root. Full stop.**
Game code communicates with AI systems only through the pipeline (`.novaforge/pipeline/`).
Engine modules have zero knowledge of AI broker systems.

---

## Non-Negotiable Rules

1. **C++20 only** — No legacy standards. Use `std::format`, concepts, ranges where appropriate.
2. **Custom UI only** — Do not introduce ImGui. The editor uses a fully custom UI renderer (OpenGL quad batching + stb_easy_font).
3. **Voxel-first** — Structure, mining, repair, damage, and destruction are all voxel operations. The low-poly visual wrapper comes later.
4. **Editor does not ship** — The editor is a development tool. `NovaForgeGame` is the shippable artifact.
5. **Determinism first** — All simulation must be bit-exact reproducible for networking and replay.
6. **CMake is truth** — Build structure is defined in CMake. No secondary build systems.
7. **Phases over features** — Each phase has a tight, locked deliverable. Nothing is merged that doesn't map to the current phase goal.
8. **AtlasAI boundary** — See Hard Boundary above. AtlasAI broker systems belong in `AtlasAI/`, not `Source/`.

## Coding Rules

- Use `NF::` namespace for all engine code
- Module headers in `include/NF/<Module>/`
- Module sources in `src/`
- No circular dependencies between modules
- No `using namespace` in headers
- Prefer `std::string_view` over `const std::string&` for read-only parameters
- Use `[[nodiscard]]` on functions that return values that should not be ignored
- No raw `new`/`delete` — use smart pointers or container ownership

## Module Dependency Rules

- Core depends on nothing
- All modules may depend on Core
- Engine depends only on Core
- Renderer depends on Core and Engine
- Game depends on Core, Engine, UI, Renderer
- Editor depends on Core, Engine, Renderer, Game, UI, GraphVM, Pipeline
- Editor code must not leak into the shipping game executable
- AtlasAI broker code must not appear in any Source/ module

## File Organization

- One class per header file (with small helpers allowed)
- Matching `.h` and `.cpp` file names
- Test files in `Tests/<Module>/test_*.cpp`
- Game data in `Data/` as JSON — fully moddable
- Schemas in `Schemas/` — versioned JSON schemas
- Configuration in `Config/`
- Game content in `Content/`
- AI broker systems in `AtlasAI/`
- Tools in `Tools/`
- Archives in `Archive/`

## Git Rules

- Commit messages: imperative mood, present tense
- One logical change per commit
- Tests must pass before merging
