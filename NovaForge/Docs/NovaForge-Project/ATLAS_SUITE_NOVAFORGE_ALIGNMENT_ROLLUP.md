ATLAS SUITE / NOVAFORGE REPO ALIGNMENT ROLLUP
Payload Version: 1.0
Date: 2026-03-31
Scope: Atlas Suite platform refactor, NovaForge repo separation, hosted-project alignment, shell/UI direction, tooling ownership split, enforcement, and documentation targets.

================================================================================
EXECUTIVE SUMMARY
================================================================================

This payload defines the immediate project direction for Atlas Suite and NovaForge.

Atlas Suite is no longer to be treated as a game repo or a game-owned tool bundle.
Atlas Suite is the platform shell, editor host, IDE host, AI broker host, and project environment.
NovaForge is a hosted external project with its own repo, loaded into Atlas Suite through manifest and extension contracts.

The correct target state is:

- Atlas Suite repo:
  - shell
  - Editor host
  - IDE host
  - AtlasAI broker
  - shared services
  - generic tooling frameworks
  - hosted-project SDK/contracts
  - project loader
  - extension runtime
  - validation, logging, Codex, build orchestration

- NovaForge repo:
  - game code
  - game assets
  - project schemas
  - project-specific authoring tools
  - project validators
  - project graph/node packs
  - project AI enrichers
  - hosted-project manifests and extension registrations

Hard rule:
If Atlas Suite breaks when NovaForge is absent, Atlas Suite is still architecturally wrong.

================================================================================
CURRENT AUDIT POSITION
================================================================================

Current repo direction is partially correct but incomplete.

What is already moving in the right direction:
- Atlas Suite is no longer intended to build NovaForge by default.
- Root architecture is trending away from a fused game repo.
- Hosted-project direction is now clear in project planning.
- Shell/app/tooling/AI responsibilities are much better defined.

What is still wrong or incomplete:
- legacy naming residue remains in active code/config/docs
- stale monorepo/game-shaped editor code still exists
- project-specific tooling is still present in Suite core
- generic tool hosts have not been fully extracted
- project loader / extension runtime still needs hard implementation
- shell UI is structurally behind target direction
- enforcement rules are not yet fully real in repo
- documentation exists conceptually but not yet rolled into a unified action package

================================================================================
LOCKED PROJECT DIRECTION
================================================================================

1. Atlas Suite is a cohesive shell, not a general-purpose OS clone.
2. The shell exists to bind together:
   - Editor
   - IDE
   - AtlasAI
3. All other tools are support systems and should stay secondary.
4. NovaForge must live in its own separate repo.
5. Atlas Suite should clone, register, validate, and host NovaForge through manifest-driven project integration.
6. Generic tooling belongs in Atlas Suite only when the host behavior is reusable.
7. Project-specific logic, nouns, rules, and workflows remain in NovaForge.
8. AtlasAI is the only visible AI broker layer.
9. Codex/local context comes before web/external fallback in the AI flow.
10. The shell should feel like one coherent environment, not disconnected tools.

================================================================================
SHELL / UI DIRECTION
================================================================================

The current blank utility-window shape is wrong.

Correct shell structure:
- top-left Atlas button acts as start/menu root
- top-right system tray for notifications, jobs, Git, status, settings
- search bar directly left of tray acts as:
  - unified search
  - command palette
  - AtlasAI prompt entry
- typed natural-language queries expand downward into a ChatGPT-like AtlasAI panel
- minimized apps live in a thin hideable left-side rail
- hover on minimized app icon shows name and state
- click restores prior app state
- right rail holds inspector/context surfaces when relevant
- bottom rail holds logs/build/output/diagnostics when relevant

Header responsibilities:
- shell entry points only
- no project-specific editor toolbars in the shell header
- command/search/AI entry
- status and system surfaces

Shell purpose:
- host and bind Editor, IDE, AtlasAI coherently
- not dominate workflow itself

================================================================================
CORE APP MODEL
================================================================================

Tier 1 core:
- Editor
- IDE
- AtlasAI

Tier 2 support:
- Build
- Logs
- Source Control
- Assets
- Codex
- Import
- Validation
- Package Manager
- Settings
- Help/Wiki

Tier 3 hosted-project extensions:
- NovaForge tools
- custom schema editors
- project-specific graph node packs
- project-specific inspectors/panels
- project analyzers/generators

Rule:
Editor + IDE are the primary full work surfaces.
Everything else supports them.

================================================================================
DECOUPLING RULES
================================================================================

Historical reference to NovaForge origin may remain in docs.
Architectural dependency may not remain in active Suite runtime.

Allowed in Atlas Suite:
- generic inspectors
- generic data editors
- graph editor hosts
- schema-driven forms
- generic spatial/world hosts if truly reusable
- generic voxel framework only if platform-wide by intent
- source control
- build/validation/logging/Codex/AI systems
- project loader and extension runtime

Not allowed in Atlas Suite core:
- fleet systems
- colony systems
- station systems
- trade/economy systems
- NovaForge mission rules
- NovaForge NPC rules
- ship-specific progression semantics
- mining/destruction semantics tied to NovaForge gameplay
- direct startup binding to NovaForge tools
- build dependence on NovaForge

================================================================================
TOOL OWNERSHIP SPLIT
================================================================================

Genericize into Atlas Suite now:
- property inspector host
- scene outliner / hierarchy
- asset browser
- graph editor host
- schema-driven form editor
- data table editor
- UI/HUD editor host
- import center
- logs / incident viewer
- validation surfaces
- patch review / diff viewer
- manifest editor
- project registry manager
- command palette
- notification center
- source control UI
- build diagnostics surfaces

Split host + NovaForge extension:
- galaxy map tooling
- ship archetype tooling
- NPC tooling
- mission tooling
- solar system tooling
- voxel editing framework
- project analyzers/generators
- reusable entity-definition editors

Keep in NovaForge:
- colony manager
- trade route editor
- station editor
- insurance/economy tooling
- fleet-specific authoring unless a truly generic host emerges
- voxel gameplay rule editors
- mining/destruction semantics editors
- all gameplay-progression/economy/faction/world-rule authoring

Archive or delete:
- stale direct-bootstrap editor code that instantiates project panels from Suite core
- dead migration/bootstrap prototypes bypassing manifest/plugin loading
- active old monorepo assumptions

================================================================================
REQUIRED PLATFORM CONTRACTS
================================================================================

Atlas Suite must formalize and implement:
- app SDK contract
- project manifest schema
- permissions/trust model
- job system
- event bus
- active context model
- validation framework
- app lifecycle
- hosted tool registration
- project capability mapping
- environment service consumption rules

These are mandatory because the current direction depends on them.

================================================================================
HOSTED PROJECT MODEL
================================================================================

NovaForge must integrate through:
- project manifest
- extension/tool registrations
- build target declarations
- launch profile declarations
- schema packs
- validators
- AI context enrichers
- project capability declarations

Atlas Suite must:
- clone/import/register project repos
- validate manifests
- load declared extensions
- expose project-specific tools only when the project is loaded
- unload them cleanly

Nothing project-specific should appear in Suite core without a declared capability.

================================================================================
MONOREPO DECISION
================================================================================

Target architecture is separate repos.

Use:
- Atlas Suite repo
- NovaForge repo

Atlas Suite should:
- clone NovaForge
- register it locally
- validate it
- host it

Monorepo is possible but is not the recommended target because it increases pressure toward:
- direct references
- build graph coupling
- game assumptions in core tooling
- blurred ownership

A temporary migration lane may exist, but final target should remain separate repos.

================================================================================
REMAINING GAP CLOSURE LANES
================================================================================

Lane 1: platform enforcement
- dependency-ban rules
- CI validation
- path/namespace validation
- manifest gating
- extension registration validation
- stale bootstrap detection

Lane 2: shell and app implementation
- real shell frame
- Atlas menu
- search/AtlasAI dropdown
- tray behavior
- minimized-app left rail
- project/home dashboards
- app layout presets

Lane 3: generic host extraction
- pull reusable hosts out of game-shaped tools
- relocate project semantics to NovaForge repo

Lane 4: hosted-project runtime
- project loader
- registry
- extension runtime
- capability mapping
- build/launch binding

Lane 5: recovery, validation, intelligence loop
- structured logging
- .logger packaging
- incident archive
- Codex ingestion
- AtlasAI debug/fix/review path
- recurrence detection

================================================================================
ATLAS SUITE IMMEDIATE ACTIONS
================================================================================

1. Freeze boundary
- rename active MasterRepo identity residue in code/config
- remove active NovaForge build assumptions
- ban direct Suite→NovaForge references

2. Quarantine legacy
- move legacy adapters into migration-only lane
- mark not-for-runtime ownership

3. Remove project-specific ownership from core editor
- move or split SceneTools and similar project-shaped panels
- archive stale direct bootstrap code

4. Implement shell-first rebuild
- Atlas menu
- search/AI dropdown
- tray
- left minimized-app rail
- home/project dashboard

5. Implement hosted-project runtime
- project manifest loader
- extension loader
- project registry
- capability mapper

6. Build generic tool hosts
- definition/data editors
- graph host
- inspector host
- asset host
- viewport tool host
- UI/HUD host

7. Add enforcement
- CI rules
- validation rules
- naming/path bans
- manifest-required integrations

================================================================================
NOVAFORGE IMMEDIATE ACTIONS
================================================================================

1. Formalize NovaForge as hosted project
- add project manifest
- define build/launch profiles
- define extension registrations
- define schema packs

2. Move project-specific tooling ownership here
- colony/trade/station/economy/fleet systems
- gameplay rule editors
- voxel gameplay semantics

3. Convert split tools into project extensions
- Galaxy map extension
- ship archetype extension
- mission/NPC/system extensions
- project analyzers and generators

4. Add project validators and AI context enrichers
- manifest-driven and explicit

5. Keep repo fully detachable
- no requirement that Atlas Suite compile NovaForge by default
- no assumption of Suite internals outside declared SDK/contracts

================================================================================
SUPPORTING DOCUMENTATION TO ADD
================================================================================

Atlas Suite repo should contain docs for:
- architecture direction
- shell/app model
- hosted project rules
- platform contracts
- editor tooling behavior
- IDE behavior
- validation/enforcement
- migration/move map
- manifest examples
- UI shell and wireframe planning
- decoupling charter
- project boundary rules

NovaForge repo should contain docs for:
- hosted project identity
- extension registrations
- project tooling ownership
- build/launch declarations
- schema pack ownership
- Atlas Suite integration rules
- NovaForge-specific tool and rule ownership
- project validator policy
- AI enrichment/context rules

================================================================================
IMPLEMENTATION ORDER
================================================================================

Phase 1
- repo boundary enforcement
- legacy naming cleanup
- stale bootstrap removal
- project/tool ownership classification finalized

Phase 2
- shell structure implementation
- Atlas menu
- search/AtlasAI dropdown
- tray and left app rail
- dashboard/home surfaces

Phase 3
- project loader + extension runtime
- registry
- manifests
- capability mapping

Phase 4
- generic host extraction
- editor/IDE shared tool hosts
- support systems wired into shell

Phase 5
- NovaForge hosted-project extension conversion
- validators
- AI enrichers
- build/launch integration

Phase 6
- logging/Codex/AtlasAI debug-fix loop
- recurrence handling
- approval and patch review workflow

================================================================================
NON-NEGOTIABLE RULES
================================================================================

1. Atlas Suite must run cleanly without NovaForge present.
2. NovaForge must be a hosted project, not a Suite-owned subsystem.
3. Generic host behavior belongs in Atlas Suite.
4. Game-specific semantics belong in NovaForge.
5. All project-specific tooling must arrive through manifests/extensions.
6. AtlasAI is the visible broker layer.
7. Shell remains cohesive and minimal, not bloated.
8. Editor + IDE are the primary work surfaces.
9. Support tools remain secondary and on-demand.
10. Legacy naming and architecture drift must be actively blocked.

================================================================================
DROP-IN INSTRUCTIONS
================================================================================

Use this payload as the repo alignment directive for both repos.

In Atlas Suite:
- place under repo docs / architecture / migration lane
- use as root instruction set for refactor planning and CI enforcement follow-up
- derive move map, manifests, validators, and shell implementation tasks from it

In NovaForge:
- place under docs / integration / hosted-project lane
- use as instruction set for converting NovaForge into a clean Atlas-hosted project
- derive extension manifests, tooling moves, validators, and build declarations from it

This payload is intentionally mirrored so both repos align to the same boundary from opposite sides.
