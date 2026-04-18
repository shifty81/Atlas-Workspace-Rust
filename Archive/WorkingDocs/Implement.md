I audited the latest zip. State is better than the earlier drops, but it is still not clean.

Top line:

The repo now has a coherent AtlasWorkspace root, a real top-level CMake, a primary AtlasWorkspace executable, AtlasUI source, editor source, NovaForge hosted under the workspace, and spec rollups that match a lot of the project direction.
It is still carrying too much legacy and too much surface area.
The zip is not shipping cleanly as a repo snapshot.

What is solid now:

CMakeLists.txt is structured sanely. Core, Engine, Renderer, Physics, Audio, Animation, Input, Networking, UI, GraphVM, AI, Pipeline, Editor, and the workspace program are split into subdirectories.
Source/Programs/AtlasWorkspace/main.cpp does make AtlasWorkspace.exe the primary entrypoint and registers child apps through a registry. That matches the workspace-first direction.
A no-tests configure works. I was able to configure the workspace successfully with tests disabled.
The repo has a real custom UI path. AtlasUI headers and widget sources are present. This is no longer just a paper design.
Docs in Docs/SpecRollup are aligned with the main rules you locked in: Atlas Workspace as host, AtlasUI standard, no canonical ImGui tooling UI, tooling-first roadmap, NovaForge logically detachable.

What is still wrong:

The zip includes the full .git directory. That alone makes the zip messy and oversized. The packed git object store is about 144 MB by itself.
The repo explodes into fake modifications after extraction. git diff --stat reports about 4082 files changed with equal insertions and deletions. That is classic line-ending normalization damage. There is no .gitattributes at repo root.
Old naming is still present. There are still 79 Arbiter references outside the ideal end state. Some are archived, but some still live under active workspace paths like Atlas/Workspace/Arbiter.
WPF is mostly not active code now, which is good, but there are still a few doc references.
ImGui is not active in the core workspace tooling path, but the repo still has a lot of ImGui references in docs, legacy NovaForge docs, validators, and compatibility notes. That means the cleanup is incomplete, not finished.
The editor layer is massively overexpanded. Source/Editor/include/NF/Editor has 312 headers and Source/Editor/src has 148 cpp files. That does not reflect the “trim actual editors to about 10 targeted tools” rule. It looks like story-driven expansion outran consolidation.
Source/Editor/include/NF/Editor/Editor.h is a giant umbrella include that drags in an enormous number of editor modules. That will hurt compile times, clarity, and ownership boundaries.
Source/Editor/src/Editor.cpp is basically empty. That means some of the editor structure exists as breadth more than depth.
The workspace is still using the GDI path in the primary executable. I found no D3D11 or DirectWrite implementation in the active workspace source. So the project is still on the fallback/bootstrap renderer path, not the preferred GPU-first tooling path.
The OpenGL backend and GLFW provider are still explicitly stubbed in places.
Tests depend on FetchContent for Catch2 from GitHub. In an offline environment, cmake --preset ci-debug fails during configure because it cannot clone Catch2. That makes the repo less portable and less repeatable than it should be.

Build reality from this audit:

cmake --preset ci-debug failed here because Catch2 is fetched from GitHub during configure.
A direct configure with tests off succeeded:
cmake -S . -B /tmp/atlas_nontest -G Ninja -DATLAS_BUILD_TESTS=OFF -DATLAS_BUILD_WORKSPACE=ON
A build started and compiled multiple core targets before I stopped it. So the repo is not dead. It is partially buildable in this environment, but not fully validated here on Windows.

Biggest drift against your locked project direction:

Too many editor modules instead of a consolidated tool roster.
Legacy Arbiter naming still survives in active paths.
GDI is still effectively the active workspace UI backend.
Repo packaging is dirty because line endings are not controlled.
Old docs and legacy branches of thought are still living beside current direction instead of being fully archived or scrubbed.

Severity order:

Critical

Missing .gitattributes and resulting full-repo line-ending churn.
Zip includes .git.
Tests require live network fetch for Catch2.

High

Editor layer is still bloated and not consolidated to the intended tool model.
Legacy Arbiter naming still exists in active workspace paths.
GDI remains the real active workspace path instead of GPU-first tooling UI.

Medium

Too many legacy docs and ImGui-era references remain.
Workspace source tree still mixes current direction with old archive-style content in a way that raises audit noise.
Giant umbrella includes in editor codebase.

My blunt read:

This is now a real monorepo skeleton with a working direction.
It is not yet a clean, normalized, consolidated workspace repo.
The next pass should be structural cleanup, not more feature spraying.

Best next moves:

Add a root .gitattributes and normalize the repo once.
Stop zipping .git for audit drops unless you explicitly want git history inspected.
Do a hard legacy scrub for active-path Arbiter remnants.
Collapse the editor surface into the actual 10-ish primary tools and push the rest behind shared panels, contracts, or archive.
Move the workspace renderer toward D3D11/DirectWrite as the real primary path and keep GDI as recovery only.
Vendor or locally mirror Catch2, or gate tests so configure does not require network access.

Strong conclusion:

Direction: mostly correct.
Packaging hygiene: poor.
Consolidation: incomplete.
Build maturity: partial.
Ready for another giant feature patch: no.
Ready for a cleanup-and-normalization pass: yes.

I can turn this into a strict file-by-file fix order next.

I can turn this into a strict file-by-file fix order next.

Yes. Here is the strict file-by-file fix order.

Phase 0. Repo hygiene first

These are first because they poison every later patch if left unresolved.

1. Root packaging controls

Add

.gitattributes
.gitignore review pass if needed

Purpose

Stop full-repo LF/CRLF churn
Normalize patches and local apply behavior
Prevent future audit noise

Minimum .gitattributes targets

* text=auto
*.sln text eol=crlf
*.vcxproj text eol=crlf
*.props text eol=crlf
*.targets text eol=crlf
*.bat text eol=crlf
*.ps1 text eol=crlf
*.cmd text eol=crlf
*.cpp text
*.h text
*.hpp text
*.c text
*.cc text
*.inl text
*.cmake text
CMakeLists.txt text
*.png binary
*.jpg binary
*.ico binary
*.ttf binary
*.otf binary
*.zip binary
*.patch text
2. Zip/export rules

Remove from audit zips

.git/
build/
out/
.vs/
Binaries/
Intermediate/
cache folders
generated logs unless specifically auditing them

Purpose

Smaller zips
Cleaner audits
Less false drift
Phase 1. Naming scrub and project identity lock

These files define what the repo is. Fix them before deeper refactors.

3. Top-level identity files

Review and fix

CMakeLists.txt
README.md
root docs that still describe old naming or structure
any root launch scripts

Targets

AtlasWorkspace is the primary product
Workspace is generic host platform
NovaForge is hosted as a project, not core logic
No user-facing Arbiter
No user-facing MasterRepo.exe
4. Active-path Arbiter scrub

Search and refactor active code/docs first

any Source/.../Arbiter*
any Atlas/Workspace/Arbiter*
any workspace-facing docs that still say Arbiter
config keys, identifiers, command names, settings panels

Rename to

AtlasAI

Do not start with archive folders
Start with live source, build scripts, and live docs only.

Phase 2. Build graph stabilization

Get the repo structurally dependable before tool consolidation.

5. Root CMake stabilization

Review

CMakeLists.txt
all add_subdirectory(...) calls
options for tests, workspace app, NovaForge, tools

Fix goals

Offline configure works
Tests are optional and off by default in audit mode
Hosted project paths are clearly gated
Legacy or dead modules are not pulled by default
6. Test dependency isolation

Review

test root CMakeLists.txt
any FetchContent or ExternalProject blocks
Catch2 integration

Fix goals

No hard network dependency for configure
Either:
vendor Catch2 locally, or
only fetch when ATLAS_ENABLE_ONLINE_DEPS=ON, or
disable tests cleanly by default
Phase 3. Workspace executable and host path

This is the real product. Clean this before editor sprawl.

7. Workspace entrypoint

Review

Source/Programs/AtlasWorkspace/main.cpp

Fix goals

AtlasWorkspace.exe remains primary entry
child tools register cleanly
no stale naming
no stale WPF assumptions
host boot is thin and deterministic
8. Workspace app shell layer

Review

workspace app bootstrap files near main
app registry files
shell initialization files
startup menu / Atlas bar sources if present

Fix goals

Atlas Workspace is generic host shell
Atlas bar naming is correct
no game-specific assumptions in workspace core
tool launch is modular
Phase 4. Rendering path correction

Right now this is one of the highest-value technical fixes.

9. Workspace renderer bootstrap

Review

workspace render host files
AtlasUI render bridge files
GDI paint path
any OpenGL stub backend
any GLFW provider stubs

Fix goals

Mark GDI as fallback only
route primary UI backend toward D3D11
route text toward DirectWrite
isolate old OpenGL/GLFW stub code from default path
10. AtlasUI backend contracts

Review

AtlasUI renderer interfaces
text interfaces
backend selection points

Fix goals

explicit backend interface split:
layout/input
primitive rendering
text rendering
texture/icon rendering
make D3D11 + DirectWrite the intended primary pair
keep fallback backend behind a feature switch
Phase 5. Editor consolidation pass

This is the biggest structural drift versus your target.

11. Editor umbrella include blast radius

Review

Source/Editor/include/NF/Editor/Editor.h

Fix goals

stop giant umbrella include behavior
split into focused module headers
define a minimal editor core include
move rarely used features behind targeted includes
12. Editor bootstrap reality check

Review

Source/Editor/src/Editor.cpp

Fix goals

either make it the real editor composition root
or remove the illusion and move composition into actual module registries
eliminate empty-shell architecture
13. Editor module inventory pass

Review

Source/Editor/include/NF/Editor/**
Source/Editor/src/**

Output needed
Create four buckets:

keep as primary tools
convert to shared panels/services
defer/archive
merge into another tool
14. Primary tool roster enforcement

From your locked direction, collapse to about 10 tools max.

Likely keep bucket

Project/Workspace Browser
Scene/World Editor
Asset Editor
Material/Shader Editor
Blueprint or visual logic editor
UI Editor
Animation Editor
Data/Table Editor
Build/Packaging tool
AtlasAI/Codex/Diagnostics tool

Everything else should be evaluated as:

panel
mode
service
plugin
archive
15. Shared panel extraction

Review shared-capable systems

outliner
inspector
content browser
log/output
command/search
notifications
properties/details
viewport overlays

Fix goals

shared once, reused everywhere
not duplicated per editor
panel compatibility matrix becomes real code structure
Phase 6. Docs and design-source cleanup

The codebase is carrying too much historical clutter.

16. Spec rollup alignment

Review

Docs/SpecRollup/**

Fix goals

keep only current canonical direction in primary rollups
move stale alternatives into Docs/Archive
remove conflicting statements about:
repo structure
WPF shell path
ImGui path
NovaForge residency
Arbiter naming
17. Legacy doc scrub

Search for and clean

ImGui
Arbiter
WPF
old exe names
old repo topology language

Rule

active docs corrected
historical docs archived and labeled clearly as historical
Phase 7. Workspace vs project boundary enforcement

This matters because drift keeps reappearing here.

18. Workspace core boundary review

Review

workspace shell
workspace app registry
core tooling libs
AtlasAI/Codex systems

Fix goals

no NovaForge-specific gameplay logic in workspace core
Workspace stays generic editor host
project adapters/plugins own project-specific behavior
19. NovaForge hosted-project boundary review

Review

NovaForge integration points
build toggles
project descriptors
plugin or project-loading paths

Fix goals

NovaForge can live in the monorepo for development
but is not built by default unless enabled
remains logically detachable
Phase 8. Tooling and patch workflow hardening

This supports the way you are actually working.

20. Patch application support

Review/add

patch apply script
patch uninstall/scrub scripts where relevant
optional context menu registration scripts
optional context menu removal scripts

Fix goals

local patch apply is deterministic
uninstall exists for every shell integration
no one-way registry changes
21. Repo audit tool alignment

If the refactor/audit utility is staying in scope:

Fix goals

audit can classify:
dead code
reusable snippets
project-specific code
universalizable code
stale naming
backend stubs
outputs map to the real workspace cleanup plan
Exact execution order

Use this order. Do not skip ahead.

.gitattributes
zip/export hygiene rules
root CMakeLists.txt
test/Catch2 gating
root identity docs and launch naming
active-path Arbiter → AtlasAI rename
Source/Programs/AtlasWorkspace/main.cpp
workspace shell/bootstrap files
AtlasUI backend/render bridge files
GDI/OpenGL/GLFW fallback isolation
Source/Editor/include/NF/Editor/Editor.h
Source/Editor/src/Editor.cpp
full editor module inventory
primary tool roster collapse
shared panel extraction
Docs/SpecRollup/**
archive stale docs
workspace/project boundary cleanup
NovaForge host gating
patch/apply/remove workflow tooling
audit utility alignment
What to avoid right now

Do not do these before cleanup:

new editor feature spray
more one-off tools
deeper NovaForge gameplay work
more UI skinning polish
large patch packs that assume line endings are already fixed
Fastest high-value bundle

If you want the first real cleanup bundle, it should contain only this:

root .gitattributes
root CMake/test gating fixes
active-path Arbiter rename to AtlasAI
workspace main/bootstrap naming cleanup
GDI marked fallback, backend contract prepared for D3D11/DirectWrite
editor umbrella include reduction
editor inventory report stub
doc rollup scrub for current canon

That is the correct first bulk pass.

I can convert this into a literal file-by-file manifest with statuses like edit, rename, archive, and delete.

I can convert this into a literal file-by-file manifest with statuses like edit, rename, archive, and delete.
Thought for 28s

Yes. Here is the literal cleanup manifest for the latest zip.

Format:

edit = modify in place
rename = move or rename path
archive = move out of active path into archive
delete = remove from repo snapshot
keep = keep, but verify
Pass 1. Repo hygiene and build stability
delete   /.git/                                reason: should not be inside audit zips
edit     /.gitignore                           reason: tighten generated/build exclusions if missing
edit     /.gitattributes                       reason: normalize line endings and patch behavior
edit     /CMakeLists.txt                       reason: test gating, offline-safe configure, default build hygiene
edit     /CMakePresets.json                    reason: make preset behavior match offline/test-gated workflow
edit     /Tests/CMakeLists.txt                 reason: remove hard configure dependency on live Catch2 fetch
edit     /vcpkg.json                           reason: verify only live dependencies that are truly required
edit     /NovaForge/vcpkg.json                 reason: same, keep hosted project dependency scope separate
archive  /build_verify/                        reason: generated verification tree should not live as canonical source
delete   /build_verify/Makefile                reason: generated artifact
delete   /build_verify/Source/**               reason: generated artifact
keep     /Directory.Build.props                reason: verify only, useful if actually active
keep     /cmake/CompilerSettings.cmake         reason: verify warning and standard policy
keep     /cmake/WindowsToolchain.cmake         reason: verify target/toolchain policy
Pass 2. Product identity and top-level canon
edit     /README.md                            reason: lock current workspace-first canon
edit     /Docs/SpecRollup/01_LOCKED_PROJECT_DIRECTION.md
edit     /Docs/SpecRollup/02_ATLASUI_STANDARD.md
edit     /Docs/SpecRollup/03_PANEL_REWRITE_STANDARD_V1.md
edit     /Docs/SpecRollup/09_REPO_AUDIT_CHECKLIST.md
archive  /Docs/SpecPack/                       reason: move if it conflicts with SpecRollup canon
edit     /Project/project.atlas.json           reason: verify product naming and hosted-project assumptions
keep     /LICENSE
keep     /Schemas/atlas.build.v1.json
keep     /Schemas/atlas.project.v1.json
Pass 3. Active-path Arbiter to AtlasAI cleanup

These are active or user-visible and should be fixed first.

edit     /AtlasAI/README.md                    reason: remove legacy-forward presentation, keep historical note brief
edit     /AtlasAI/ATLAS_AI_OVERVIEW.md         reason: same
edit     /Atlas/Workspace/README.md            reason: still points to Arbiter path and commands
rename   /Atlas/Workspace/Arbiter/             -> /Atlas/Workspace/AtlasAI/     reason: active workspace path rename
edit     /Source/Pipeline/src/WorkspaceBroker.cpp
edit     /Source/Programs/AtlasWorkspace/main.cpp
edit     /Source/Programs/AtlasWorkspace/CMakeLists.txt
edit     /Tests/Pipeline/test_workspace_broker.cpp
edit     /Tests/Pipeline/test_tool_wiring.cpp
edit     /Tests/Pipeline/test_pipeline.cpp
edit     /Tests/Pipeline/test_full_suite.cpp
edit     /Tests/Pipeline/test_arbiter_reasoner.cpp
archive  /Tools/ArbiterAI/                     reason: legacy tool path, superseded by AtlasAI
archive  /Tools/SwissAgent/                    reason: legacy naming/path, keep only if intentionally preserved as subcomponent source
keep     /Tools/AtlasAI/                       reason: canonical tool path
archive  /AtlasAI/Atlas_Arbiter/usable_snippets/**  reason: historical snippets, not active workspace canon
Pass 4. Workspace executable and shell
keep     /Source/Programs/AtlasWorkspace/main.cpp        reason: primary entrypoint is correct
edit     /Source/Programs/AtlasWorkspace/main.cpp        reason: strip active GDI-first assumptions, clean AtlasAI naming
keep     /Source/Programs/AtlasWorkspace/CMakeLists.txt
edit     /Source/Programs/AtlasWorkspace/CMakeLists.txt  reason: verify only workspace app is authoritative top-level exe
edit     /Source/Core/**                                 reason: only where shell boot contracts leak old assumptions
edit     /Source/UI/**                                   reason: align shell/UI backend selection with AtlasUI canon
Pass 5. Rendering backend correction
edit     /Source/Programs/AtlasWorkspace/main.cpp        reason: GDI should be fallback, not practical primary path
edit     /Source/UI/include/NF/UI/GDIBackend.h           reason: explicitly mark fallback-only role
edit     /Source/UI/src/GDIBackend.cpp                   reason: same
edit     /Source/UI/include/NF/UI/OpenGLBackend.h        reason: isolate as non-canonical or compatibility backend
edit     /Source/UI/src/OpenGLBackend.cpp                reason: same
edit     /Source/Renderer/include/NF/Renderer/Renderer.h reason: avoid implying OpenGL is the long-term default RHI
edit     /Source/Renderer/src/Pipeline/GLViewportFramebuffer.h
edit     /Source/Renderer/src/Pipeline/GLViewportFramebuffer.cpp
archive  /Tests/UI/test_ui.cpp                           reason: if it encodes obsolete backend expectations
edit     /Tests/UI/test_m1_editor.cpp                    reason: update tests away from OpenGL-as-canonical assumptions

If you are doing the backend shift properly, add these new paths in the same pass:

add      /Source/UI/include/NF/UI/D3D11Backend.h
add      /Source/UI/src/D3D11Backend.cpp
add      /Source/UI/include/NF/UI/DirectWriteTextBackend.h
add      /Source/UI/src/DirectWriteTextBackend.cpp
add      /Source/UI/include/NF/UI/UIBackendSelector.h
add      /Source/UI/src/UIBackendSelector.cpp
Pass 6. Editor sprawl reduction

These are the biggest consolidation targets.

Core editor composition files
edit     /Source/Editor/include/NF/Editor/Editor.h       reason: giant umbrella include must be broken apart
edit     /Source/Editor/src/Editor.cpp                   reason: make it real composition root or shrink it
edit     /Source/Editor/include/NF/Editor/EditorApp.h
edit     /Source/Editor/include/NF/Editor/EditorPanel.h
edit     /Source/Editor/include/NF/Editor/DockLayout.h
edit     /Source/Editor/include/NF/Editor/LayoutPersistence.h
edit     /Source/Editor/include/NF/Editor/MenuBar.h
edit     /Source/Editor/include/NF/Editor/CommandPalette.h
edit     /Source/Editor/include/NF/Editor/ContentBrowser.h
edit     /Source/Editor/include/NF/Editor/InspectorPanel.h
edit     /Source/Editor/include/NF/Editor/HierarchyPanel.h
edit     /Source/Editor/include/NF/Editor/ConsolePanel.h
edit     /Source/Editor/include/NF/Editor/NotificationSystem.h
edit     /Source/Editor/include/NF/Editor/AtlasAIPanelHost.h
Keep as likely primary tools

These fit the target tool roster better.

keep     /Source/Editor/include/NF/Editor/AnimationEditor.h
keep     /Source/Editor/include/NF/Editor/MaterialEditor.h
keep     /Source/Editor/include/NF/Editor/DataTableEditor.h
keep     /Source/Editor/include/NF/Editor/PackagingEditor.h
keep     /Source/Editor/include/NF/Editor/BuildPipelineEditorV1.h
keep     /Source/Editor/include/NF/Editor/GraphEditorPanel.h
keep     /Source/Editor/include/NF/Editor/LogicGraph.h
keep     /Source/Editor/include/NF/Editor/HUDEditor.h
keep     /Source/Editor/include/NF/Editor/ParticleEditor.h
keep     /Source/Editor/include/NF/Editor/AudioMixerEditor.h
edit     /Source/Editor/include/NF/Editor/LevelDesignToolkit.h   reason: likely fold into scene/world editor
Convert to shared panels or services

These should not exist as standalone editors unless they are truly separate apps.

edit     /Source/Editor/include/NF/Editor/ContentBrowserPanel.h   -> shared panel
edit     /Source/Editor/include/NF/Editor/ComponentInspectorV1.h  -> shared panel
edit     /Source/Editor/include/NF/Editor/DiagnosticPanelV1.h     -> shared panel
edit     /Source/Editor/include/NF/Editor/MemoryProfilerPanel.h   -> shared panel
edit     /Source/Editor/include/NF/Editor/PipelineMonitorPanel.h  -> shared panel
edit     /Source/Editor/include/NF/Editor/NotificationCenterEditor.h -> shared panel
edit     /Source/Editor/include/NF/Editor/OnlineServicesPanel.h   -> shared panel
edit     /Source/Editor/include/NF/Editor/GraphHostContract.h     -> shared service contract
edit     /Source/Editor/include/NF/Editor/GraphHostContractV1.h   -> shared service contract
edit     /Source/Editor/include/NF/Editor/FileIntakePipeline.h    -> shared service
edit     /Source/Editor/include/NF/Editor/CodexSnippetMirror.h    -> shared service
edit     /Source/Editor/include/NF/Editor/EditorEventBus.h        -> shared service
edit     /Source/Editor/include/NF/Editor/ConsoleCommandBus.h     -> shared service
Archive or merge likely overspray

These are the ones most obviously violating the 10-tool direction.

archive  /Source/Editor/include/NF/Editor/ArcadeGameEditor.h
archive  /Source/Editor/include/NF/Editor/DailyQuestEditor.h
archive  /Source/Editor/include/NF/Editor/GameEconomyEditor.h
archive  /Source/Editor/include/NF/Editor/InventoryEditor.h
archive  /Source/Editor/include/NF/Editor/ItemShopEditor.h
archive  /Source/Editor/include/NF/Editor/LeaderboardEditor.h
archive  /Source/Editor/include/NF/Editor/LobbyEditor.h
archive  /Source/Editor/include/NF/Editor/LiveOpsEditor.h
archive  /Source/Editor/include/NF/Editor/MatchReplayEditor.h
archive  /Source/Editor/include/NF/Editor/MatchmakingEditor.h
archive  /Source/Editor/include/NF/Editor/MiniGameEditor.h
archive  /Source/Editor/include/NF/Editor/MusicSequencer.h
archive  /Source/Editor/include/NF/Editor/DungeonGenerator.h
archive  /Source/Editor/include/NF/Editor/EcosystemEditor.h
archive  /Source/Editor/include/NF/Editor/BiomeEditor.h
archive  /Source/Editor/include/NF/Editor/CloudStorageEditor.h
archive  /Source/Editor/include/NF/Editor/CloudSyncEditor.h
archive  /Source/Editor/include/NF/Editor/BroadcastEditor.h
archive  /Source/Editor/include/NF/Editor/ExperimentEditor.h
archive  /Source/Editor/include/NF/Editor/BenchmarkSuiteEditor.h
archive  /Source/Editor/include/NF/Editor/LoadTestEditor.h
archive  /Source/Editor/include/NF/Editor/BandwidthProfileEditor.h
archive  /Source/Editor/include/NF/Editor/LatencySimEditor.h
archive  /Source/Editor/include/NF/Editor/PacketCapture.h
archive  /Source/Editor/include/NF/Editor/NetworkTopologyEditor.h
archive  /Source/Editor/include/NF/Editor/MultiplayerPreview.h
archive  /Source/Editor/include/NF/Editor/AccessibilityEditor.h
archive  /Source/Editor/include/NF/Editor/ColorblindSimulator.h
archive  /Source/Editor/include/NF/Editor/AchievementEditor.h
archive  /Source/Editor/include/NF/Editor/CharacterCreatorEditor.h
archive  /Source/Editor/include/NF/Editor/CostumeEditor.h
archive  /Source/Editor/include/NF/Editor/LanguagePackEditor.h
archive  /Source/Editor/include/NF/Editor/LocalizationKeyEditor.h
archive  /Source/Editor/include/NF/Editor/FontRegistryEditor.h
archive  /Source/Editor/include/NF/Editor/IconEditor.h
archive  /Source/Editor/include/NF/Editor/IconographySpec.h
archive  /Source/Editor/include/NF/Editor/MenuLayoutEditor.h
archive  /Source/Editor/include/NF/Editor/DisplayModeEditor.h
archive  /Source/Editor/include/NF/Editor/GamepadConfigurator.h
archive  /Source/Editor/include/NF/Editor/ControlSchemeEditor.h
archive  /Source/Editor/include/NF/Editor/AxisMappingEditor.h
archive  /Source/Editor/include/NF/Editor/InputBindingsEditor.h
archive  /Source/Editor/include/NF/Editor/InputActionEditor.h
AI editor files

These should be merged into AtlasAI tooling, not explode into standalone editors.

edit     /Source/Editor/include/NF/Editor/AIAssistantPanel.h      -> AtlasAI shared panel
archive  /Source/Editor/include/NF/Editor/AIBehaviorEditor.h
archive  /Source/Editor/include/NF/Editor/AIDebugPathV1.h
archive  /Source/Editor/include/NF/Editor/AIDecisionEditor.h
archive  /Source/Editor/include/NF/Editor/AIGoalEditorV1.h
archive  /Source/Editor/include/NF/Editor/AINavMeshEditorV1.h
archive  /Source/Editor/include/NF/Editor/AIPathfindEditor.h
archive  /Source/Editor/include/NF/Editor/AIPerceptionEditor.h
archive  /Source/Editor/include/NF/Editor/AISpawnEditorV1.h
edit     /Source/Editor/include/NF/Editor/AtlasAIPanelHost.h      reason: keep as canonical AI surface
Pass 7. Legacy docs and archives
archive  /AtlasAI/Atlas_Arbiter/                  reason: preserve as historical import, not active canon
archive  /AtlasAI/Atlas_SwissAgent/               reason: same
archive  /Atlas/Workspace/Arbiter/                reason: after rename or migration
archive  /Tools/ContractScanner/                  reason: keep only if confirmed active
archive  /Tools/ReplayMinimizer/                  reason: keep only if confirmed active
archive  /Tools/Validation/                       reason: keep only if confirmed active
archive  /Archive/_Arbiter/
archive  /Archive/_ArbiterAI/
archive  /Archive/_AtlasToolingSuite/
archive  /Archive/_MasterRepo/
archive  /Archive/_MasterRepoRefactor/
archive  /Archive/_SwissAgent/
archive  /Archive/MasterRepo/
keep     /Archive/.gitkeep
Pass 8. NovaForge boundary enforcement
edit     /NovaForge/CMakeLists.txt                reason: hosted project should not build by default unless enabled
edit     /NovaForge/Docs/**                       reason: remove conflicting workspace-core assumptions
edit     /NovaForge/Source/**                     reason: only where it leaks into workspace core
keep     /NovaForge/Project/
keep     /NovaForge/Config/
keep     /NovaForge/Data/
keep     /NovaForge/Tools/
Pass 9. Script and patch workflow hardening
edit     /build.cmd                               reason: align with workspace-first, no hidden assumptions
edit     /Scripts/build.sh
edit     /Scripts/build_all.sh
edit     /Scripts/build_win.cmd
edit     /Scripts/generate_vs_solution.bat
edit     /Scripts/generate_vs_solution.ps1
edit     /Scripts/validate_project.sh
add      /Scripts/apply_patch.ps1                reason: deterministic local patch apply
add      /Scripts/remove_patch_context_menu.reg  reason: uninstall pair for any context menu integration
add      /Scripts/add_patch_context_menu.reg     reason: only if you still want shell integration
Immediate delete list for clean audit zips

Use this every time before zipping an audit snapshot.

delete /.git/
delete /build/
delete /.vs/
delete /Binaries/
delete /Intermediate/
delete /build_verify/
delete any generated logs not intentionally under /Logs/
Immediate rename list
rename /Atlas/Workspace/Arbiter/ -> /Atlas/Workspace/AtlasAI/
rename active user-facing Arbiter strings -> AtlasAI
rename active tests/classes only where they are user-facing or integration-facing
Immediate archive list

These can be moved without waiting for deep refactor if they are not active compile dependencies.

archive /Tools/ArbiterAI/
archive /Tools/SwissAgent/
archive /AtlasAI/Atlas_Arbiter/usable_snippets/
archive /build_verify/
archive stale editor one-off headers listed above
archive stale docs that still describe ImGui, WPF host paths, or old repo identity as current
First bulk pass only

If you want the safest first cleanup commit, do only this subset:

edit    /.gitattributes
edit    /CMakeLists.txt
edit    /Tests/CMakeLists.txt
edit    /README.md
edit    /Atlas/Workspace/README.md
edit    /AtlasAI/README.md
edit    /AtlasAI/ATLAS_AI_OVERVIEW.md
rename  /Atlas/Workspace/Arbiter/ -> /Atlas/Workspace/AtlasAI/
edit    /Source/Programs/AtlasWorkspace/main.cpp
edit    /Source/Programs/AtlasWorkspace/CMakeLists.txt
edit    /Source/Editor/include/NF/Editor/Editor.h
edit    /Source/Editor/src/Editor.cpp
archive /Tools/ArbiterAI/
archive /Tools/SwissAgent/
archive /build_verify/
delete  /.git/

That is the correct literal first manifest.

I can turn this next into a patch-ready checklist with exact search/replace targets and commit order.

I can turn this next into a patch-ready checklist with exact search/replace targets and commit order.

Yes. Here is the patch-ready checklist with exact search/replace targets and commit order.

Commit 1. Normalize repo text behavior first

Do this before any rename or cleanup.

Files
/.gitattributes
/.gitignore
/CMakeLists.txt
/Tests/CMakeLists.txt
/CMakePresets.json
Exact actions
A. Add or replace .gitattributes

Use this baseline:

* text=auto

*.sln      text eol=crlf
*.vcxproj  text eol=crlf
*.vcproj   text eol=crlf
*.props    text eol=crlf
*.targets  text eol=crlf
*.filters  text eol=crlf
*.bat      text eol=crlf
*.cmd      text eol=crlf
*.ps1      text eol=crlf

*.c        text
*.cc       text
*.cpp      text
*.cxx      text
*.h        text
*.hh       text
*.hpp      text
*.hxx      text
*.inl      text
*.ipp      text

*.cmake    text
CMakeLists.txt text
*.py       text
*.json     text
*.md       text
*.txt      text
*.yml      text
*.yaml     text
*.patch    text

*.png      binary
*.jpg      binary
*.jpeg     binary
*.bmp      binary
*.ico      binary
*.ttf      binary
*.otf      binary
*.zip      binary
*.7z       binary
*.dll      binary
*.lib      binary
*.pdb      binary
*.exe      binary
B. Tighten .gitignore

Ensure these are ignored if missing:

/build/
/build_*/
/out/
/.vs/
/.idea/
/Binaries/
/Intermediate/
/x64/
/Debug/
/Release/
/cmake-build-*/
/build_verify/
/Logs/*.tmp
C. Root CMake search targets

Search in /CMakeLists.txt and /Tests/CMakeLists.txt for:

FetchContent_Declare(
FetchContent_MakeAvailable(
ExternalProject_Add(
Catch2
GTest
googletest
D. Replace hard online fetch with guarded option

Add root options if missing:

option(ATLAS_BUILD_TESTS "Build tests" OFF)
option(ATLAS_ENABLE_ONLINE_DEPS "Allow online dependency fetches" OFF)

Then wrap tests:

if(ATLAS_BUILD_TESTS)
    add_subdirectory(Tests)
endif()

Then in Tests/CMakeLists.txt:

If Catch2 is vendored locally, use local path first.
If not, fail only when tests are explicitly enabled and dependency is unavailable.
Do not fetch from network unless ATLAS_ENABLE_ONLINE_DEPS=ON.

Target replacement pattern:

if(NOT ATLAS_BUILD_TESTS)
    return()
endif()

and guard any fetch block with:

if(ATLAS_ENABLE_ONLINE_DEPS)
    # fetch logic
else()
    message(FATAL_ERROR "Tests requested but online deps are disabled and no local test framework was found.")
endif()
Commit message
repo: normalize line endings and gate online test dependencies
Commit 2. Clean top-level product identity
Files
/README.md
/Project/project.atlas.json
/Docs/SpecRollup/01_LOCKED_PROJECT_DIRECTION.md
/Docs/SpecRollup/02_ATLASUI_STANDARD.md
/Docs/SpecRollup/09_REPO_AUDIT_CHECKLIST.md
/Source/Programs/AtlasWorkspace/CMakeLists.txt
Exact search/replace targets

Search globally in active root docs and config for:

MasterRepo.exe
Atlas Suite
AtlasToolingSuite
workspace merged into the game
NovaForge is core workspace logic
WPF shell is active path
ImGui is the standard UI

Replace with:

Atlas Workspace
AtlasWorkspace.exe
generic workspace host
NovaForge is a hosted project
Qt/WPF paths are historical or deferred if still mentioned
AtlasUI is the standard UI
Specific rules
Atlas Workspace is the user-facing product name.
AtlasWorkspace.exe is the primary executable.
NovaForge is loaded and developed through Workspace, but not core workspace logic.
No doc should claim ImGui is the standard tooling UI.
No doc should claim WPF is the active shell path.
Commit message
docs: lock workspace-first identity and current repo canon
Commit 3. Active-path Arbiter to AtlasAI rename

Do live code and live docs first. Leave historical archive references alone until later.

Paths
/Atlas/Workspace/Arbiter/
/Atlas/Workspace/README.md
/AtlasAI/README.md
/AtlasAI/ATLAS_AI_OVERVIEW.md
/Source/Pipeline/src/WorkspaceBroker.cpp
/Source/Programs/AtlasWorkspace/main.cpp
/Source/Programs/AtlasWorkspace/CMakeLists.txt
/Tests/Pipeline/test_workspace_broker.cpp
/Tests/Pipeline/test_tool_wiring.cpp
/Tests/Pipeline/test_pipeline.cpp
/Tests/Pipeline/test_full_suite.cpp
/Tests/Pipeline/test_arbiter_reasoner.cpp
Exact search targets

Search global active paths only for:

Arbiter
ARBITER
arbiter
AtlasArbiter
ArbiterAI
Workspace/Arbiter
arbiter_reasoner
Replace targets
String replacements
Arbiter           -> AtlasAI
ARBITER           -> ATLAS_AI        only for config keys/macros if appropriate
arbiter           -> atlas_ai        only for file names, symbols, ids where style fits
AtlasArbiter      -> AtlasAI
ArbiterAI         -> AtlasAI
Workspace/Arbiter -> Workspace/AtlasAI
arbiter_reasoner  -> atlas_ai_reasoner
Be careful

Do not blind replace across archive folders on this pass.
Do not rename variables inside clearly historical snippets or archived markdown yet.
Do not force ATLAS_AI if existing macro style uses ATLASAI.

Path rename
/Atlas/Workspace/Arbiter/ -> /Atlas/Workspace/AtlasAI/
Manual verification hotspots
command ids
config section names
registry keys
panel titles
startup registrations
tests with string equality
path literals in CMake or scripts
Commit message
rename: migrate active Arbiter paths and symbols to AtlasAI
Commit 4. Fix workspace entrypoint and host bootstrap
Files
/Source/Programs/AtlasWorkspace/main.cpp
/Source/Programs/AtlasWorkspace/CMakeLists.txt
Search targets in main.cpp
GDI
OpenGL
GLFW
Arbiter
MasterRepo
WPF
NovaForge boot by default
Required edits
Keep AtlasWorkspace as the primary app entry.
Ensure child tools register through a clean app/tool registry.
Remove stale naming in menu titles, window titles, app ids, command ids.
Ensure NovaForge is not hard-wired as workspace core boot logic.
If there is direct GDI startup as the normal path, refactor to backend selection or at minimum mark it fallback-only.
Replace patterns

Examples:

"Arbiter"                  -> "AtlasAI"
"MasterRepo"               -> "Atlas Workspace"
"Atlas Suite"              -> "Atlas Workspace"
"NovaForge Editor"         -> only where it wrongly labels the primary host

If window title is wrong:

"AtlasWorkspace - NovaForge" -> "Atlas Workspace"

unless that label is intentionally project-contextual after project load.

Commit message
workspace: clean host bootstrap naming and primary entry behavior
Commit 5. Demote GDI and isolate legacy UI backends
Files
/Source/UI/include/NF/UI/GDIBackend.h
/Source/UI/src/GDIBackend.cpp
/Source/UI/include/NF/UI/OpenGLBackend.h
/Source/UI/src/OpenGLBackend.cpp
/Source/Programs/AtlasWorkspace/main.cpp
Search targets
class GDIBackend
CreateCompatibleDC
BitBlt
StretchBlt
SwapBuffers
glfw
opengl
OpenGLBackend
default backend
primary backend
Required changes
Mark GDI as fallback or bootstrap backend in comments and naming.
Stop implying OpenGL is the canonical long-term workspace UI backend.
If there is backend selection logic, add explicit prioritization comments:
D3D11
GDI fallback only
If D3D11 code does not exist yet, do not fake it. Just structure selection points for it.
Safe comment/header replacements

In GDI backend headers or docs:

Primary UI backend         -> Fallback UI backend
Default renderer           -> Bootstrap/fallback renderer

In OpenGL backend headers or docs:

Main renderer              -> Compatibility renderer
Default viewport backend   -> Legacy/compatibility viewport backend
Add placeholder selection contract if absent

Add these files if needed:

/Source/UI/include/NF/UI/UIBackendSelector.h
/Source/UI/src/UIBackendSelector.cpp

Minimal job:

expose enum for backend type
expose stub selection method
centralize current fallback choice instead of hardcoding in main.cpp
Commit message
ui: isolate GDI and legacy OpenGL as fallback-compatible backends
Commit 6. Break up editor umbrella include
Files
/Source/Editor/include/NF/Editor/Editor.h
/Source/Editor/src/Editor.cpp
Search targets in Editor.h

Look for long runs of:

#include "NF/Editor/
Required action

Split Editor.h into:

minimal editor core include
tool registry include
shared panels include
optional feature-specific includes moved out
Suggested replacement structure
Editor.h should keep only things like:
#pragma once
#include "NF/Editor/EditorApp.h"
#include "NF/Editor/EditorPanel.h"
#include "NF/Editor/EditorEventBus.h"
#include "NF/Editor/DockLayout.h"
#include "NF/Editor/LayoutPersistence.h"
#include "NF/Editor/MenuBar.h"
Move broad tool includes into new files

Add:

/Source/Editor/include/NF/Editor/EditorSharedPanels.h
/Source/Editor/include/NF/Editor/EditorToolRegistry.h

Then relocate includes there.

Search targets in Editor.cpp
TODO
stub
placeholder
empty
return;
Required action

Either:

make Editor.cpp the real composition root that registers shared panels and primary tools, or
shrink it and move composition to registry source files.

Do not leave it as empty theater.

Commit message
editor: reduce umbrella include blast radius and define real composition root
Commit 7. Collapse editor sprawl into keep, merge, archive

This is an inventory commit. Do not try to fully rewrite every editor in one shot.

Add one manifest doc
/Docs/SpecRollup/EDITOR_TOOL_INVENTORY_V1.md
Its sections
KEEP_PRIMARY
MERGE_TO_SHARED_PANEL
MERGE_TO_SHARED_SERVICE
ARCHIVE
DEFER
Exact likely archive search targets

These are the first one-off editor names to search and move from active registry references:

ArcadeGameEditor
DailyQuestEditor
GameEconomyEditor
InventoryEditor
ItemShopEditor
LeaderboardEditor
LobbyEditor
LiveOpsEditor
MatchReplayEditor
MatchmakingEditor
MiniGameEditor
CloudStorageEditor
CloudSyncEditor
BroadcastEditor
ExperimentEditor
BenchmarkSuiteEditor
LoadTestEditor
BandwidthProfileEditor
LatencySimEditor
PacketCapture
NetworkTopologyEditor
AccessibilityEditor
ColorblindSimulator
AchievementEditor
CostumeEditor
MenuLayoutEditor
DisplayModeEditor
GamepadConfigurator
AxisMappingEditor
InputActionEditor
InputBindingsEditor
Required action
Remove these from any central editor registry or startup registration.
Do not delete yet.
Move files to archive tree only if compile graph proves they are inactive.
Otherwise mark them excluded from build.
Commit message
editor: inventory one-off tools and remove non-core editors from active roster
Commit 8. Extract shared panels and services
Candidate files
/Source/Editor/include/NF/Editor/ContentBrowserPanel.h
/Source/Editor/include/NF/Editor/ComponentInspectorV1.h
/Source/Editor/include/NF/Editor/DiagnosticPanelV1.h
/Source/Editor/include/NF/Editor/MemoryProfilerPanel.h
/Source/Editor/include/NF/Editor/PipelineMonitorPanel.h
/Source/Editor/include/NF/Editor/FileIntakePipeline.h
/Source/Editor/include/NF/Editor/CodexSnippetMirror.h
/Source/Editor/include/NF/Editor/EditorEventBus.h
/Source/Editor/include/NF/Editor/ConsoleCommandBus.h
Search targets
class .*Editor :
registerTool(
CreateTool(
ToolId::
Required action

Anything that is functionally a panel or service should no longer register as a top-level editor app unless absolutely necessary.

Rename patterns where appropriate:

NotificationCenterEditor -> NotificationCenterPanel
DiagnosticEditor         -> DiagnosticsPanel
ContentBrowserEditor     -> ContentBrowserPanel

Only do this where the class is actually a panel. Do not blind rename.

Commit message
editor: convert shared capabilities from standalone editors into reusable panels and services
Commit 9. Archive stale legacy tools and old imports
Paths
/Tools/ArbiterAI/
/Tools/SwissAgent/
/AtlasAI/Atlas_Arbiter/
/AtlasAI/Atlas_SwissAgent/
/build_verify/
Required action

Move under archive if still valuable:

/Archive/_ArbiterAI/
/Archive/_SwissAgent/
/Archive/_ImportedSnapshots/build_verify/
Or delete from repo snapshot if generated only.
Search targets before move
add_subdirectory(
target_link_libraries(
#include
"Tools/ArbiterAI
"Tools/SwissAgent
build_verify

Make sure no active build or include path depends on them.

Commit message
archive: move legacy AI imports and generated verification artifacts out of active tree
Commit 10. Clean docs that still claim old canon
Search targets across /Docs, /README.md, active project docs
ImGui
Dear ImGui
WPF
Arbiter
MasterRepo
Atlas Suite
merged into workspace core
workspace game-specific core
Replace rules
If the statement is meant as historical context, prefix it clearly with Historical: or move to archive.
If the statement is presented as current behavior, rewrite it.

Examples:

ImGui is the standard UI               -> AtlasUI is the standard tooling UI
WPF shell is the active shell path     -> WPF shell work is archived/deferred
Arbiter broker                         -> AtlasAI broker
Atlas Suite                            -> Atlas Workspace
NovaForge merged into core             -> NovaForge is hosted by Workspace and remains logically detachable
Commit message
docs: archive stale canon and align active docs to current workspace direction
Exact commit order

Use this exact order.

repo: normalize line endings and gate online test dependencies
docs: lock workspace-first identity and current repo canon
rename: migrate active Arbiter paths and symbols to AtlasAI
workspace: clean host bootstrap naming and primary entry behavior
ui: isolate GDI and legacy OpenGL as fallback-compatible backends
editor: reduce umbrella include blast radius and define real composition root
editor: inventory one-off tools and remove non-core editors from active roster
editor: convert shared capabilities from standalone editors into reusable panels and services
archive: move legacy AI imports and generated verification artifacts out of active tree
docs: archive stale canon and align active docs to current workspace direction
Grep checklist

Run these after each relevant commit.

After commit 1
FetchContent_Declare(
Catch2
googletest
ExternalProject_Add(

Expected:

still allowed only in guarded blocks
not unconditional in default configure path
After commit 3
Arbiter
arbiter
ArbiterAI
Workspace/Arbiter

Expected:

zero hits in active runtime paths
remaining hits only in archive or historical docs
After commit 5
default backend
primary backend
OpenGL is default
GDI is default

Expected:

no misleading claims in active headers or docs
After commit 6 through 8
registerTool(
CreateTool(
ToolId::

Expected:

reduced number of top-level editor registrations
shared panels no longer misregistered as full editors
After commit 10
ImGui
WPF
Atlas Suite
MasterRepo
Arbiter

Expected:

only archival or explicitly historical references remain
Hard rules during patching
Do not rename archive content in the same pass as active code.
Do not mix backend shift with deep renderer rewrite.
Do not delete one-off editors until build references are checked.
Do not feature-expand while doing cleanup.
Do not zip .git, build, .vs, or build_verify in the next audit drop.
Fastest first execution block

If you want the safest minimal patch set right now, do only this block first:

1. add .gitattributes
2. gate tests in root CMake and Tests/CMakeLists
3. fix README and SpecRollup canon
4. rename active Arbiter path to AtlasAI
5. clean main.cpp titles and ids
6. demote GDI/OpenGL wording to fallback/compat
7. shrink Editor.h

That is the correct first real cleanup sequence.

I can turn this into a literal PowerShell-driven search/replace execution sheet next.

can we generate the rest of what needs to happen  as well in a full repo  reset of roadmap and get all the documentation in place also the readme front page  is pretty bland the road map on front page should be a condensed but detailed list showing whats actually done what is planned and what consilidation still needed or normilizations or renames 

Yes. The correct move is a documentation reset pack that does four things at once:

locks the current canon
resets the roadmap around cleanup before feature growth
makes the README front page actually useful
turns scattered spec drift into one controlled doc tree

Below is the full structure.

1. Repo documentation reset plan
Primary goals
Stop roadmap drift
Stop old naming from surviving in active docs
Stop feature-sprawl from looking “done” when it is only stubbed
Make repo status readable in one pass
Separate done, partial, planned, deferred, and archived
New documentation hierarchy
/README.md

/Docs/
  /Canon/
    00_PROJECT_STATUS.md
    01_LOCKED_DIRECTION.md
    02_REPO_STRUCTURE.md
    03_NAMING_CANON.md
    04_UI_BACKEND_STRATEGY.md
    05_EDITOR_STRATEGY.md
    06_WORKSPACE_VS_PROJECT_BOUNDARY.md
    07_BUILD_AND_DEPENDENCY_POLICY.md
    08_PATCH_AND_AUDIT_WORKFLOW.md

  /Roadmap/
    00_MASTER_ROADMAP.md
    01_CLEANUP_AND_NORMALIZATION.md
    02_WORKSPACE_CORE.md
    03_ATLASUI_AND_RENDER_BACKENDS.md
    04_EDITOR_CONSOLIDATION.md
    05_ATLASAI_AND_CODEX.md
    06_PROJECT_HOSTING_AND_NOVAFORGE.md
    07_BUILD_RELEASE_AND_PACKAGING.md

  /Inventory/
    EDITOR_TOOL_INVENTORY.md
    PANEL_AND_SERVICE_MATRIX.md
    LEGACY_NAME_SCRUB_LIST.md
    DOC_CONFLICT_INDEX.md
    REPO_CLEANUP_MANIFEST.md

  /Archive/
    legacy docs moved here

That becomes the real doc spine.

2. What the roadmap should reset around

The roadmap should no longer read like feature brainstorming. It should read like an execution ladder.

Correct roadmap order
Phase 1. Cleanup and normalization

This must be first.

Includes:

.gitattributes
line-ending normalization
active-path Arbiter → AtlasAI
root README rewrite
doc canon reset
test/dependency gating
removal of bad zip hygiene
archive stale docs
root repo structure cleanup
Phase 2. Workspace core stabilization

After cleanup.

Includes:

AtlasWorkspace.exe as primary entry
shell bootstrap cleanup
app/tool registry cleanup
workspace/project boundary enforcement
generic host behavior
no project-specific core leakage
Phase 3. AtlasUI and backend correction

After core stabilization.

Includes:

AtlasUI contract cleanup
GDI fallback only
D3D11 primary target path
DirectWrite primary text path
legacy OpenGL/GLFW isolation
backend selector contract
Phase 4. Editor consolidation

Only after the shell is stable.

Includes:

reduce top-level editors to about 10
convert one-off editors into shared panels/services
define shared panel residency
tool registry cleanup
remove fake breadth
Phase 5. AtlasAI and Codex integration

After the editor surface is rationalized.

Includes:

AtlasAI canonical naming
broker flow docs
debug handoff flow
Codex mirror rules
snippet promotion rules
build log archive flow
Phase 6. Hosted project support

Then hosted project flow.

Includes:

NovaForge as hosted project
project loading contracts
build gating
detachable project boundary
plugin or adapter hooks
Phase 7. Packaging and release

Last.

Includes:

build pipeline
release pipeline
patch application workflow
shell integration scripts
audit scripts
packaging rules

That is the proper reset.

3. What README front page should become

The front page should stop being generic and start acting like a live control panel.

README front page structure

Use this order:

Project identity
Current status
What is actually working
What is partially implemented
What is being consolidated now
What is planned next
Repo layout
Build notes
Documentation index
Current cleanup priorities
4. README front page draft

You can use this almost directly.

# Atlas Workspace

Atlas Workspace is a native C++ workspace host for game development tooling, editor workflows, build orchestration, project hosting, AtlasAI broker workflows, and reusable shared development tools.

It is being built as a **generic workspace platform** first. Game projects such as **NovaForge** are hosted and developed through the workspace, but do not define the workspace core.

---

## Current Status

Atlas Workspace is in an active **cleanup, normalization, and consolidation phase**.

The repo already contains real source structure, workspace boot code, AtlasUI foundations, editor systems, hosted-project structure, and a large body of design/spec material.

The current priority is **not feature spraying**. The current priority is to bring the repo into a clean, stable, canonical state so future implementation work lands on solid structure.

---

## Current State Summary

### Done or materially present
- Top-level monorepo structure is in place
- `AtlasWorkspace` is the primary executable direction
- Workspace-first product direction is established
- AtlasUI exists as the custom UI direction
- Core engine/tooling subdirectories exist in source layout
- Hosted project structure exists for NovaForge
- Major spec coverage exists across workspace, tooling, and editor direction
- AtlasAI direction has replaced Arbiter at the project level

### Partial or incomplete
- Workspace host bootstrap still needs cleanup and hardening
- UI backend strategy is not fully corrected yet
- GDI is still too close to active path instead of fallback-only
- D3D11 and DirectWrite primary backend path still needs formal implementation
- Editor surface area is too large and not yet consolidated to the intended tool roster
- Shared panel/service extraction is incomplete
- Active-path naming cleanup is incomplete
- Some docs still reflect older project states and legacy assumptions
- Test/dependency flow still needs offline-safe cleanup

### In active consolidation now
- repo normalization
- documentation reset
- naming scrub
- editor consolidation
- backend strategy cleanup
- workspace vs hosted-project boundary enforcement

---

## Current Roadmap

### 1. Cleanup and normalization
- add root `.gitattributes`
- normalize line endings
- remove bad audit zip contents
- gate online-only test dependencies
- clean active-path naming drift
- archive stale or conflicting docs
- reset repo canon documentation

### 2. Workspace core stabilization
- harden `AtlasWorkspace.exe` as the primary host
- clean workspace bootstrap and app registration
- enforce generic workspace host behavior
- remove project-specific leakage from workspace core

### 3. AtlasUI and render backend correction
- formalize AtlasUI backend contracts
- keep GDI as fallback only
- establish D3D11 as primary UI backend target
- establish DirectWrite as primary text backend target
- isolate legacy OpenGL/GLFW compatibility paths

### 4. Editor consolidation
- collapse editor sprawl into a focused primary tool roster
- convert one-off editor concepts into shared panels and services
- reduce umbrella include blast radius
- formalize panel residency and compatibility rules

### 5. AtlasAI and Codex integration
- complete active AtlasAI naming migration
- formalize broker flow
- wire build-log and error-handling flow into AtlasAI
- define Codex mirroring, validation, deduplication, and snippet promotion

### 6. Hosted project support
- keep NovaForge hosted through Workspace
- preserve logical detachment of hosted projects
- formalize project loading and build gating
- support future external project/plugin hosting

### 7. Build, patch, and release pipeline
- stabilize build presets and dependency policy
- finalize patch apply/remove workflow
- improve repo audit tooling
- define packaging and release path

---

## Consolidation Still Required

### Naming
- remaining active-path `Arbiter` references must be migrated to `AtlasAI`
- stale product naming must be removed from active docs and code
- user-facing names must reflect Atlas Workspace canon only

### Structural normalization
- line-ending normalization must be locked
- repo zip hygiene must be enforced
- generated artifacts must stop polluting source audits
- active docs must be separated from historical archives

### Editor rationalization
- primary editor roster must be trimmed to the intended core tool set
- shared panels must replace duplicated editor-like surfaces
- non-core one-off editors must be merged, deferred, or archived

### Backend correction
- AtlasUI backend selection must be formalized
- GDI must stop acting like the practical default
- D3D11 + DirectWrite must become the real intended primary path

---

## Planned Core Tool Direction

The intended editor/tool surface is a compact set of primary tools supported by shared panels and services.

Planned core direction:
- Workspace/Project Browser
- Scene/World Editor
- Asset Editor
- Material/Shader Editor
- Visual Logic Editor
- UI Editor
- Animation Editor
- Data/Table Editor
- Build/Packaging Tool
- AtlasAI/Codex/Diagnostics Tool

Everything else should become:
- a shared panel
- a shared service
- a mode inside an existing tool
- a hosted plugin
- or an archived/deferred concept

---

## Repo Layout

```text
Source/          Core source for workspace, engine, UI, renderer, editor, and tooling
Docs/            Canon docs, roadmap docs, inventory docs, and archives
Project/         Workspace-level project descriptors and config
NovaForge/       Hosted project under active development through Workspace
Tools/           Standalone or support tooling that is still active and canonical
Schemas/         Shared schema definitions
Scripts/         Build, patch, and validation scripts
Archive/         Historical material not considered active canon
Documentation Index
Canon
Docs/Canon/00_PROJECT_STATUS.md
Docs/Canon/01_LOCKED_DIRECTION.md
Docs/Canon/03_NAMING_CANON.md
Docs/Canon/04_UI_BACKEND_STRATEGY.md
Docs/Canon/05_EDITOR_STRATEGY.md
Roadmap
Docs/Roadmap/00_MASTER_ROADMAP.md
Docs/Roadmap/01_CLEANUP_AND_NORMALIZATION.md
Docs/Roadmap/04_EDITOR_CONSOLIDATION.md
Docs/Roadmap/05_ATLASAI_AND_CODEX.md
Inventory
Docs/Inventory/EDITOR_TOOL_INVENTORY.md
Docs/Inventory/PANEL_AND_SERVICE_MATRIX.md
Docs/Inventory/LEGACY_NAME_SCRUB_LIST.md
Docs/Inventory/REPO_CLEANUP_MANIFEST.md
Build Notes

Current focus is repo cleanup and structural stabilization. Build behavior, test dependency policy, and backend implementation details are still under active correction.

Tests and online dependency fetches should remain explicitly gated. Audit snapshots should not include .git, generated build trees, or editor-generated junk.

Immediate Priority

The next correct work is:

normalize the repo
lock the docs
finish active naming cleanup
stabilize workspace bootstrap
correct backend strategy
consolidate the editor surface

Feature expansion should wait until those are complete.


---

# 5. New core docs that should exist

These are the missing docs that make the roadmap real.

## `Docs/Canon/00_PROJECT_STATUS.md`
Purpose:
- single source of truth for actual repo condition

Sections:
- active phase
- build status
- source of drift
- immediate priorities
- blocked work
- next milestone

## `Docs/Canon/01_LOCKED_DIRECTION.md`
Purpose:
- one page that locks what the workspace is

Must state:
- Atlas Workspace is generic host
- AtlasUI is standard tooling UI
- AtlasAI is canonical broker name
- NovaForge is hosted project, not workspace core
- actual tools should be trimmed to focused roster
- GDI is fallback
- D3D11/DirectWrite are target primary path

## `Docs/Canon/03_NAMING_CANON.md`
Purpose:
- eliminate recurring rename drift

Must define:
- Atlas Workspace
- AtlasWorkspace.exe
- AtlasAI
- AtlasUI
- NovaForge
- “rig” terminology
- old names no longer allowed in active paths

## `Docs/Canon/04_UI_BACKEND_STRATEGY.md`
Purpose:
- freeze backend direction

Must define:
- AtlasUI layer responsibilities
- backend selector responsibilities
- GDI fallback-only
- D3D11 primary UI backend target
- DirectWrite primary text path
- OpenGL/GLFW compatibility-only if retained

## `Docs/Canon/05_EDITOR_STRATEGY.md`
Purpose:
- stop editor explosion

Must define:
- 10-ish primary tools max
- shared panel model
- shared services model
- what counts as editor vs panel vs service vs plugin
- what gets archived

## `Docs/Canon/06_WORKSPACE_VS_PROJECT_BOUNDARY.md`
Purpose:
- stop repo identity drift

Must define:
- workspace core is generic
- projects are hosted
- project-specific gameplay systems do not live in core workspace
- adapters/plugins own project integration

## `Docs/Roadmap/00_MASTER_ROADMAP.md`
Purpose:
- master execution ladder only
- no brainstorm clutter
- every line tied to a real milestone

---

# 6. Inventory docs that should exist

These turn cleanup into something executable.

## `Docs/Inventory/EDITOR_TOOL_INVENTORY.md`
Columns:
- item
- current type
- desired type
- keep/merge/archive/defer
- notes

## `Docs/Inventory/PANEL_AND_SERVICE_MATRIX.md`
Columns:
- component
- shared panel or service
- used by which tools
- residency rules
- current state

## `Docs/Inventory/LEGACY_NAME_SCRUB_LIST.md`
Columns:
- old name
- new name
- search scope
- status

Examples:
- Arbiter → AtlasAI
- Atlas Suite → Atlas Workspace
- MasterRepo.exe → AtlasWorkspace.exe

## `Docs/Inventory/DOC_CONFLICT_INDEX.md`
Columns:
- file
- conflicting statement
- correct canon
- action needed

## `Docs/Inventory/REPO_CLEANUP_MANIFEST.md`
This is the literal repo action sheet:
- edit
- rename
- archive
- delete
- verify

---

# 7. How statuses should be written

The roadmap must stop implying all present code is complete.

Use five states only:
- **Done**
- **In Progress**
- **Partial**
- **Planned**
- **Archived**

Do not use vague labels like:
- basically done
- mostly there
- nearly complete
- kind of working

Those are audit poison.

---

# 8. Condensed roadmap style for the front page

This is the exact style to use.

## Done
- workspace-first repo direction established
- Atlas Workspace as primary host direction
- AtlasUI chosen as tooling UI standard
- hosted project structure for NovaForge present
- broad spec coverage exists

## In Progress
- repo normalization
- active naming cleanup
- documentation reset
- workspace bootstrap cleanup
- editor consolidation

## Partial
- AtlasUI runtime backend path
- shared panel extraction
- AtlasAI/Codex workflow wiring
- hosted-project gating
- build/test dependency cleanup

## Planned
- D3D11 backend
- DirectWrite text path
- final core tool roster
- project/plugin loading contracts
- release pipeline

## Archived or Deferred
- WPF shell path
- legacy Arbiter naming
- ImGui-standard assumptions
- oversized one-off editors
- stale imported tool trees not part of canon

That gives the front page teeth.

---

# 9. Recommended documentation reset commit order

1. rewrite `README.md`
2. add `/Docs/Canon/*`
3. add `/Docs/Roadmap/*`
4. add `/Docs/Inventory/*`
5. move stale docs into `/Docs/Archive/`
6. update cross-links
7. remove old roadmap duplication
8. add status language consistently across active docs

---

# 10. Strong final recommendation

The repo needs a **full documentation reset milestone** before any more serious feature expansion.

Call the milestone:

## `Milestone 0: Canon Reset and Consolidation`

Its success criteria:
- README rewritten
- canon docs in place
- roadmap reset
- inventories created
- old conflicting docs archived
- naming canon documented
- cleanup priorities visible on front page
- editor consolidation explicitly tracked
- backend strategy explicitly tracked

That is the correct reset point.

I can generate the actual contents for the full doc set next, file by file, starting with `README.md`, `00_PROJECT_STATUS.md`, `01_LOCKED_DIRECTION.md`, and `00_MASTER_ROADMAP.md`.
anything else?

Yes. A few more things should be locked now or the repo will drift again.

Missing control docs

Add these too:

Docs/Canon/09_DEFINITION_OF_DONE.md

This prevents fake completion.

Should define:

what counts as Done
what counts as Partial
what counts as Stub
what counts as Archived
minimum proof required for “done”:
builds
launches
renders
loads
saves
test coverage if applicable
docs updated
Docs/Canon/10_REPO_STATUS_LEGEND.md

This makes roadmap labels consistent.

Use fixed tags only:

Done
In Progress
Partial
Planned
Blocked
Archived
Docs/Canon/11_MODULE_OWNERSHIP_AND_BOUNDARIES.md

This stops code from leaking across layers.

Define:

what belongs in Source/Core
what belongs in Source/UI
what belongs in Source/Editor
what belongs in Source/Programs
what belongs in NovaForge
what must never live in workspace core
Missing engineering inventories
Docs/Inventory/BUILD_FLAGS_AND_OPTIONS.md

List every important CMake option and what it does.

Include:

tests
online deps
workspace app
NovaForge build toggle
tool toggles
debug validation toggles
Docs/Inventory/THIRD_PARTY_DEPENDENCY_REGISTER.md

Track every dependency.

Columns:

dependency
source
used by
required or optional
online fetch or vendored
keep/replace/remove
Docs/Inventory/ACTIVE_EXECUTABLES_AND_TOOLS.md

Track real runnable things.

Columns:

executable/tool
purpose
current state
entrypoint path
keep/merge/archive

This is important because the repo currently looks broader than it really is.

Missing technical strategy docs
Docs/Roadmap/08_TESTING_AND_VALIDATION.md

Needed because cleanup without validation becomes guesswork.

Should define:

smoke tests
build verification
launch verification
UI backend verification
editor registration verification
doc sync verification
Docs/Roadmap/09_ARCHIVE_AND_DEPRECATION_PROCESS.md

Needed so stale systems stop lingering forever.

Define:

when something gets archived
when something gets deleted
when something is merely excluded from build
how historical docs are labeled
Docs/Roadmap/10_RENAME_AND_NORMALIZATION_WAVE.md

Put all renames in one controlled wave.

Include:

Arbiter → AtlasAI
Atlas Suite → Atlas Workspace
any old exe names
any stale product-facing labels
path-level renames
identifier-level renames
doc-only renames
Missing README sections

The front page should also have these small sections:

“Known Drift”

A blunt list:

editor overspread
legacy naming residue
backend mismatch
stale docs
generated artifact pollution
“What Not To Build Yet”

This is valuable.

Examples:

new one-off editors
more backend experiments
more hosted-project-specific core logic
extra polish passes before consolidation
“Audit Rules”

State:

no .git in audit zips
no build folders in audit zips
generated artifacts excluded
current README and canon docs updated with major structural changes
Missing repo root support files

These are not docs, but they support the reset:

.gitattributes
cleaned .gitignore
CONTRIBUTING.md
Docs/README.md as a doc index
optional STATUS.md at root if you want a fast repo-health snapshot
CONTRIBUTING.md should exist

Even if you are the main contributor, it helps a lot.

It should define:

naming rules
archive rules
doc update expectations
patch workflow
build expectations
no feature spray during cleanup milestone
how to classify new editor ideas:
tool
panel
service
plugin
archive
One more important milestone

Add this after the doc reset:

Milestone 1: Structural Truth Pass

Goal:
make the repo reflect what is actually real.

Success criteria:

every active top-level editor/tool is accounted for
every archived item is marked clearly
every roadmap item has one status only
every canonical doc matches current repo structure
every stale name in active paths is scrubbed
every generated artifact is removed from source snapshot

That milestone matters more than adding new code right now.

Final missing principle

Put this in canon:

Presence is not completion.
A file, folder, class, editor, or stub existing in the repo does not mean the system is implemented.

That one sentence should drive the whole reset.

The only other major thing after this is generating the actual doc pack itself.

The only other major thing after this is generating the actual doc pack itself.

Correct. Generate the doc pack now. This is the full initial set, ready to drop into the repo.

/README.md
# Atlas Workspace

Atlas Workspace is a native C++ workspace platform for building games, tools, and development pipelines. It provides a unified host for editors, build systems, AtlasAI workflows, and project orchestration.

Atlas Workspace is a **generic host environment**. Game projects such as NovaForge are developed inside it but do not define the workspace core.

---

## Current Status

Active phase: **Milestone 0 – Canon Reset and Consolidation**

The repository contains real structure and systems, but is undergoing cleanup and normalization before further expansion.

---

## State Overview

### Done
- Workspace-first architecture defined
- Atlas Workspace is the primary executable direction
- AtlasUI selected as standard UI framework
- AtlasAI established as canonical AI broker name
- Monorepo structure in place
- Hosted project structure (NovaForge) present

### In Progress
- Repo normalization and cleanup
- Active naming migration (Arbiter → AtlasAI)
- Documentation reset
- Workspace bootstrap cleanup
- Editor consolidation

### Partial
- UI backend strategy implementation
- Shared panel system extraction
- AtlasAI/Codex workflow wiring
- Test/dependency cleanup

### Planned
- D3D11 UI backend
- DirectWrite text backend
- Final editor tool roster
- Project/plugin loading system
- Release pipeline

### Archived / Deferred
- WPF shell direction
- ImGui tooling path
- Arbiter naming
- One-off editor expansion

---

## Current Roadmap

### 1. Cleanup and Normalization
- line ending normalization
- repo hygiene enforcement
- naming cleanup
- doc canon reset

### 2. Workspace Core Stabilization
- clean host bootstrap
- enforce workspace boundaries
- tool/app registry cleanup

### 3. AtlasUI Backend Strategy
- GDI fallback only
- D3D11 target backend
- DirectWrite text path

### 4. Editor Consolidation
- reduce tool count
- convert editors → panels/services
- define shared systems

### 5. AtlasAI Integration
- broker flow
- debugging pipeline
- Codex mirroring

### 6. Hosted Project Support
- NovaForge as hosted project
- build gating
- plugin/project model

---

## Consolidation Still Required

- remove legacy naming in active paths
- reduce editor surface area
- fix backend mismatch
- archive stale docs
- remove generated artifacts from repo

---

## Core Tool Direction

Target: ~10 primary tools supported by shared panels.

- Workspace Browser
- Scene Editor
- Asset Editor
- Material Editor
- Visual Logic Editor
- UI Editor
- Animation Editor
- Data Editor
- Build Tool
- AtlasAI Tool

---

## Repo Layout


Source/
Docs/
NovaForge/
Tools/
Schemas/
Scripts/
Archive/


---

## Immediate Priority

1. normalize repo  
2. lock documentation  
3. complete naming cleanup  
4. stabilize workspace host  
5. fix UI backend strategy  
6. consolidate editors  

---

## Rules

- No feature expansion during cleanup
- No new standalone editors
- No stale naming in active paths
- No generated artifacts in repo snapshots
/Docs/Canon/00_PROJECT_STATUS.md
# Project Status

Current Phase: **Milestone 0 – Canon Reset and Consolidation**

## Build Status
- Partial
- Workspace config builds with tests disabled
- Dependency handling incomplete

## Repo Condition
- Structure present
- Naming inconsistent
- Docs inconsistent
- Editor surface oversized
- Backend strategy incomplete

## Active Problems
- line ending instability
- legacy naming (Arbiter)
- editor sprawl
- backend mismatch (GDI vs intended GPU path)
- stale documentation

## Immediate Goals
- normalize repo
- unify naming
- reset docs
- reduce editor surface
- define backend strategy

## Blocked Work
- major UI work blocked by backend strategy
- editor expansion blocked by consolidation pass
- AI integration blocked by naming cleanup

## Next Milestone
Milestone 1: Structural Truth Pass
/Docs/Canon/01_LOCKED_DIRECTION.md
# Locked Direction

## Workspace
Atlas Workspace is a **generic development host**, not a game.

## UI
AtlasUI is the standard UI framework.

## AI
AtlasAI is the canonical broker system.

## Projects
Projects (e.g., NovaForge) are hosted and remain logically detachable.

## Editor Philosophy
- limited number of primary tools
- shared panels and services
- no uncontrolled tool expansion

## Backend Strategy
- GDI = fallback only
- D3D11 = target UI backend
- DirectWrite = target text backend

## Repo Philosophy
- structure before features
- consolidation before expansion
/Docs/Canon/03_NAMING_CANON.md
# Naming Canon

## Approved Names
- Atlas Workspace
- AtlasWorkspace.exe
- AtlasUI
- AtlasAI
- NovaForge

## Removed Names
- Arbiter
- Atlas Suite
- MasterRepo.exe

## Rules
- no legacy names in active code
- historical names allowed only in archive
- user-facing strings must follow canon
/Docs/Canon/04_UI_BACKEND_STRATEGY.md
# UI Backend Strategy

## Layers
- AtlasUI (layout, widgets)
- Backend (rendering + text)

## Backends

### Primary Target
- D3D11
- DirectWrite

### Fallback
- GDI

### Compatibility
- OpenGL (optional, not canonical)

## Rules
- backend must be selectable
- no hardcoded backend in main
- text rendering separated from draw backend
/Docs/Canon/05_EDITOR_STRATEGY.md
# Editor Strategy

## Goals
- reduce editor count
- maximize reuse
- enforce shared panels

## Definitions
- Tool: top-level editor
- Panel: reusable UI component
- Service: backend system

## Limits
- ~10 primary tools

## Rules
- no one-off editors without justification
- panels shared across tools
- services centralized
/Docs/Roadmap/00_MASTER_ROADMAP.md
# Master Roadmap

## Phase 0 – Canon Reset
cleanup, naming, docs, normalization

## Phase 1 – Workspace Core
bootstrap, registry, boundaries

## Phase 2 – UI Backend
D3D11 + DirectWrite

## Phase 3 – Editor Consolidation
tool reduction, panel extraction

## Phase 4 – AtlasAI
broker, Codex, debug flow

## Phase 5 – Project Hosting
NovaForge integration

## Phase 6 – Release Pipeline
build, packaging, deployment
/Docs/Inventory/EDITOR_TOOL_INVENTORY.md
# Editor Tool Inventory

| Item | Current | Target | Action |
|------|--------|--------|--------|
| AnimationEditor | Tool | Tool | Keep |
| InventoryEditor | Tool | Panel | Merge |
| MatchmakingEditor | Tool | Archive | Archive |
/Docs/Inventory/LEGACY_NAME_SCRUB_LIST.md
# Legacy Name Scrub List

| Old | New | Status |
|-----|-----|--------|
| Arbiter | AtlasAI | In Progress |
| Atlas Suite | Atlas Workspace | In Progress |
| MasterRepo.exe | AtlasWorkspace.exe | Pending |
/Docs/Canon/09_DEFINITION_OF_DONE.md
# Definition of Done

A system is DONE only if:
- compiles
- runs
- integrates
- documented
- no stub logic

Otherwise:
- Partial
- Stub
- Planned
/Docs/Canon/10_REPO_STATUS_LEGEND.md
# Status Legend

- Done
- In Progress
- Partial
- Planned
- Blocked
- Archived
/Docs/Canon/11_MODULE_BOUNDARIES.md
# Module Boundaries

## Workspace Core
generic host only

## Editor
tools and panels only

## UI
render + widgets

## Projects
game-specific logic only

## Rule
no cross-layer leakage
Final note

This is the minimum viable doc pack.

Once this is in place:

roadmap becomes enforceable
audits become consistent
cleanup becomes measurable
feature work becomes safe again

Next step, if you want:

I can generate the Editor Inventory fully populated from your repo and the actual consolidation decisions per file.

I can generate the Editor Inventory fully populated from your repo and the actual consolidation decisions per file.
Thought for 1m 32s

Yes. I generated it from the repo scan.

Files:

Full editor inventory and consolidation decisions

Top-line result from Source/Editor/include/NF/Editor:

312 headers scanned
20 keep as primary or canonical surfaces
139 keep as shared panels, services, or contracts
108 merge into existing primary tools
45 archive or defer from the active top-level roster

Most important conclusions:

The repo is still far beyond the intended primary tool count.
A large amount of the surface should be panels and services, not standalone editors.
The strongest archive bucket is the game-specific and ops-heavy one-off editors.
AtlasAI, content browsing, build, asset, graph, UI, animation, material, packaging, and core workspace surfaces are the clearest keep set.

Highest-value active keep set:

AtlasAIPanelHost.h
AnimationEditor.h
AnimBlueprintEditor.h
MaterialEditor.h
ShaderGraphEditor.h
DataTableEditor.h
PackagingEditor.h
BuildPipelineEditorV1.h
HUDEditor.h
UIDesignEditor.h
TimelineEditor.h
TextureEditor.h
StaticMeshEditor.h
PrefabEditor.h
TerrainEditor.h
ParticleEditor.h
AudioMixerEditor.h

Strongest archive/defer examples:

ArcadeGameEditor.h
DailyQuestEditor.h
GameEconomyEditor.h
ItemShopEditor.h
LeaderboardEditor.h
LobbyEditor.h
LiveOpsEditor.h
MatchReplayEditor.h
MatchmakingEditor.h
MiniGameEditor.h
CloudStorageEditor.h
CloudSyncEditor.h
BenchmarkSuiteEditor.h
LoadTestEditor.h
BandwidthProfileEditor.h
LatencySimEditor.h
PacketCapture.h
AccessibilityEditor.h
ColorblindSimulator.h

One caveat:

This inventory is based on the repo structure and file roles, not a full compile-graph proof of every dependency edge. It is the correct consolidation map for the current direction, but final delete vs archive decisions should still be checked against build references.

Next strongest move is turning this into:

Docs/Inventory/EDITOR_TOOL_INVENTORY.md
Docs/Inventory/PANEL_AND_SERVICE_MATRIX.md
a literal phase-by-phase archive and merge plan for the editor tree
can we refactor those specific editors into gameplay system editor panels that maybe tie directly in thru the novaforge adapter considering  some of this is very specific to novaforge

Yes. That is the correct refactor for the NovaForge-specific ones.

Do not keep those as top-level workspace editors. Move them behind a NovaForge adapter layer and expose them as gameplay system panels inside a smaller set of canonical tools.

Correct split
Keep in Workspace core

Only generic editor infrastructure:

panel framework
tool registry
docking/layout
property inspection
outliner/content browser
graph host
data table host
viewport host
notifications/logging
AtlasAI host
build/package host
project adapter contracts
Move behind NovaForge adapter

Anything that is game-rule-specific, economy-specific, faction-specific, mission-specific, progression-specific, or live-ops flavored.

Examples from your current overspray that should become NovaForge panels or modules, not workspace-core editors:

GameEconomyEditor
InventoryEditor
ItemShopEditor
DailyQuestEditor
LobbyEditor
LeaderboardEditor
MiniGameEditor
MatchmakingEditor
MatchReplayEditor
AchievementEditor
CharacterCreatorEditor
likely parts of biome/ecosystem/dungeon tooling if they are NovaForge content-authoring specific rather than generic procedural framework tools
Best target structure

Use a three-layer model.

Layer 1. Workspace core

Generic host only.

Suggested contracts:

IGameProjectAdapter
IGameplaySystemPanelProvider
IGameplaySystemSchemaProvider
IProjectToolExtension
IProjectPanelFactory
Layer 2. NovaForge adapter

NovaForge-specific integration bridge.

Suggested path:

NovaForge/Source/EditorAdapter/
NovaForge/Source/EditorAdapter/include/NovaForge/EditorAdapter/...
NovaForge/Source/EditorAdapter/src/...

This layer:

registers NovaForge-only panels
maps NovaForge data/assets/components into generic workspace panels
supplies schemas, categories, commands, validators
injects panel tabs into approved host tools
Layer 3. NovaForge gameplay system panels

These are not “editors” anymore. They are panels hosted inside broader tools.

What they should become
Convert economy-like editors into panels

Example mapping:

GameEconomyEditor -> NovaForgeEconomyPanel
ItemShopEditor -> NovaForgeShopPanel
InventoryEditor -> NovaForgeInventoryRulesPanel
AchievementEditor -> NovaForgeProgressionPanel
DailyQuestEditor -> NovaForgeMissionRulesPanel

These should live under something like:

Data/System Editor
Gameplay Rules Tool
Project Systems Tool

Not as standalone apps.

Convert multiplayer/ops-like editors carefully

If NovaForge is not at that implementation stage yet, these should be adapter modules or archived stubs, not visible tools.

Example:

LobbyEditor -> NovaForgeSessionPanel
LeaderboardEditor -> NovaForgeRankingPanel
MatchmakingEditor -> archive or hidden adapter panel until relevant
MatchReplayEditor -> archive until replay system exists
LiveOpsEditor -> archive; this is not core priority
Convert character/game-specific content editors
CharacterCreatorEditor -> NovaForgeCharacterRulesPanel
CostumeEditor -> NovaForgeAppearancePanel

Host these under:

Entity/Actor Editor
Gameplay Systems Tool
Data Authoring Tool
Best host tools for them

Do not invent a new top-level tool for each system. Use a few broad hosts.

Recommended host tools
Project Systems Tool
economy
inventory rules
progression
missions
factions
research
crafting chains
World/Scenario Tool
biome rules
encounter tables
spawn rules
POI/system generation
Entity/Actor Tool
rigs
loadouts
AI archetypes
appearance rules
component presets
Network/Simulation Tool
only truly generic simulation/network diagnostics
Build/Packaging Tool
project packaging, validation, deployment

That keeps Workspace clean and lets NovaForge extend meaningfully.

How the adapter should work
Core contract

Workspace should ask the loaded project:

what panels do you provide
which host tool do they belong to
what data schemas do they edit
what commands/actions do they expose
what validation rules apply
what assets/data tables back them
Example registration model

Conceptually:

struct GameplaySystemPanelDescriptor
{
    std::string id;
    std::string displayName;
    std::string hostToolId;
    std::string category;
    std::function<std::unique_ptr<IEditorPanel>()> createPanel;
    bool defaultVisible = false;
};

NovaForge adapter returns a list of these, and Workspace inserts them into approved hosts.

Strong rule to lock now

Workspace core owns generic capability. NovaForge adapter owns game meaning.

That one rule fixes most of the repo drift.

What should stay generic vs project-specific
Keep generic
graph editor framework
property editor
table editor
asset browser
viewport
command system
panel host system
save/load/layout
validation framework
build pipeline framework
AI host panel framework
Make NovaForge-specific
economy formulas
inventory slot rules
loot tables
research unlock logic
mission categories
faction relationships
rig/loadout presets
character progression data
project-specific crafting chains
project-specific world generation rules
What to do with the current specific editors

Use four outcomes only:

Convert to adapter panel

For systems you know NovaForge actually needs.

Merge into broader NovaForge systems panel

For narrow tools that do not deserve their own tab.

Hide behind feature flag

For future systems not ready for active use.

Archive

For things that are not aligned with current priority.

Best immediate refactor set

These are the best first conversions:

GameEconomyEditor -> NovaForgeEconomyPanel
InventoryEditor -> NovaForgeInventoryRulesPanel
ItemShopEditor -> NovaForgeShopPanel
DailyQuestEditor -> NovaForgeMissionRulesPanel
AchievementEditor -> NovaForgeProgressionPanel
CharacterCreatorEditor -> NovaForgeCharacterRulesPanel

Then host them all under:

Project Systems Tool

That gives you one strong game-systems surface instead of six weak editors.

Suggested folder result
Source/Editor/include/NF/Editor/
  ProjectSystemsTool.h
  GameplaySystemsHostPanel.h
  IProjectGameplayPanelProvider.h

NovaForge/Source/EditorAdapter/include/NovaForge/EditorAdapter/
  NovaForgeAdapter.h
  NovaForgeGameplayPanelRegistry.h
  NovaForgeEconomyPanel.h
  NovaForgeInventoryRulesPanel.h
  NovaForgeMissionRulesPanel.h
  NovaForgeProgressionPanel.h
  NovaForgeCharacterRulesPanel.h

NovaForge/Source/EditorAdapter/src/
  NovaForgeAdapter.cpp
  NovaForgeGameplayPanelRegistry.cpp
  NovaForgeEconomyPanel.cpp
  NovaForgeInventoryRulesPanel.cpp
  NovaForgeMissionRulesPanel.cpp
  NovaForgeProgressionPanel.cpp
  NovaForgeCharacterRulesPanel.cpp
Recommendation

Yes. Refactor those specific editors into NovaForge gameplay system panels through the NovaForge adapter. That is cleaner, matches your hosted-project direction, and stops the workspace from filling up with game-specific top-level tools.

The strongest next artifact is a NovaForge Adapter + Gameplay System Panel Contract v1 with exact C++ interfaces, host-tool IDs, registration flow, and the first six panel conversions.

The strongest next artifact is a NovaForge Adapter + Gameplay System Panel Contract v1 with exact C++ interfaces, host-tool IDs, registration flow, and the first six panel conversions. lets alwo audit every chat in this project and summarize where we are on project over all compared to zip and lets lock in everything for this repo reset please

Yes. Here is the locked repo-reset state, based on the project chats here and the latest zip audit.

Overall status

The project has moved out of the pure brainstorming phase. It now has enough real structure that the main risk is no longer “missing ideas.” The main risk is drift.

Current truth:

core direction is much clearer than earlier chats
repo structure is materially present
workspace-first identity is mostly present
AtlasUI is locked as the UI standard
AtlasAI naming is mostly established at the design level
NovaForge is understood as a hosted project, not workspace core
editor/tool ambitions are still oversized relative to the intended final structure
docs, naming, backend strategy, and editor boundaries still need a hard reset

So the correct next milestone is not more feature expansion. It is a repo reset and canon lock.

Where the project stands versus the latest zip
What the chats clearly locked in

From the project history, these points are now consistent and should be treated as fixed:

Workspace identity
Atlas Workspace is the main product and main executable.
Other tools should launch from inside Atlas Workspace.
Workspace is a generic editor/workspace platform, not a game-specific core.
UI direction
AtlasUI is the standard UI framework.
No canonical ImGui path for core tooling.
WPF is no longer the active forward path and should remain archived/deferred.
Native C++ tooling direction is the active path.
Win32 is the baseline platform direction for current tooling.
AI and broker direction
AtlasAI is the canonical name.
AtlasAI is the visible middle-man broker.
Preferred flow is local/internal first, then web, then broader model/provider layers behind AtlasAI.
Build errors and logs should route through AtlasAI with notification-driven fix flow.
Repo/project boundary
Atlas Workspace is the host.
NovaForge is a hosted project under development through Workspace.
NovaForge remains logically detachable and should not define Workspace core.
Workspace should support project adapters/plugins.
Editor philosophy
Actual editor/tool count should be trimmed to about 10 targeted primary tools.
Shared panels and services should replace duplicated one-off editors.
Tooling standardization and consolidation should come before major gameplay feature work.
Camera/input/editor direction
Editor viewports use ghost-style/FPS-style camera behavior by default.
Editor/game/workspace input bindings should be separated by context.
Menuing, context menus, and control binding surfaces need to be real tooling, not hardcoded behavior.
Codex/archive/logging direction
Build logs should be packaged, archived, and mirrored into Codex-like systems.
Successful reusable interactions should be mirrored into Codex under validation/deduplication rules.
Zip files and loose files in project root need proper intake/archive handling.
What the latest zip shows

The latest zip proves some of that direction is materially present, but not fully cleaned up:

Present in repo
top-level monorepo structure
real source tree
AtlasWorkspace primary program path
AtlasUI source presence
hosted NovaForge structure
large documentation/spec surface
broad editor framework footprint
Still wrong in repo
.git included in audit zip
missing or ineffective line-ending normalization
active-path legacy naming still present
GDI too close to active UI path
OpenGL/GLFW legacy path not fully isolated
editor tree far too broad
docs still carry conflicting historical statements
tests/dependency flow not fully offline-safe

So compared to the chats, the repo is directionally correct but structurally inconsistent.

Full project summary by major area
1. Product identity

This is mostly settled now.

Locked:

product name: Atlas Workspace
primary executable: AtlasWorkspace.exe
AI name: AtlasAI
UI framework: AtlasUI

Still needing repo cleanup:

remove remaining active Arbiter naming
remove stale Atlas Suite and similar legacy presentation from active docs/code
remove any old executable naming in active paths
2. Workspace core

The chats now strongly support a generic host model.

Locked:

Workspace core is generic
no game-specific core logic in Workspace
Workspace hosts editors, tools, build systems, project loading, AtlasAI, logging, Codex, and integrations
major tools are their own apps within the Workspace shell

Repo implication:

workspace bootstrap and registry need cleanup
project-specific behavior must move behind adapters
NovaForge-specific systems should not remain top-level workspace-core editors
3. UI/backend strategy

Direction is locked, implementation is not.

Locked:

AtlasUI is standard
native C++ forward path
GDI should not be the intended long-term active path
D3D11 plus DirectWrite is the correct primary target direction
legacy backends can remain only as compatibility/fallback

Repo implication:

backend selector and backend strategy docs must be formalized
GDI must be marked fallback-only
OpenGL/GLFW path must be isolated as legacy/compatibility if retained
backend work should follow repo reset, not precede it
4. Editor/tool strategy

This is where the strongest drift exists.

Locked:

about 10 primary tools max
everything else becomes a panel, service, mode, plugin, or archive
shared panel residency should be real
editor surface should be consolidated before new feature growth

Repo reality:

current editor tree is still far too broad
many one-off editors are really project-specific system surfaces
many should be merged into broader host tools
many should be NovaForge adapter panels, not core workspace tools
5. NovaForge integration

This is now clear enough to lock.

Locked:

NovaForge is a hosted project
it is worked on through Workspace
it remains logically detachable
it should expose project-specific tools through adapter/extension contracts
project-specific gameplay system authoring belongs behind the NovaForge adapter

Repo implication:

economy/inventory/quests/progression/character-like editors should be converted into NovaForge gameplay panels
those panels should be hosted under broader generic workspace tools
Workspace core keeps generic infrastructure only
6. AtlasAI/Codex/logging

This is one of the more consistent parts of the chat history.

Locked:

AtlasAI is the visible broker layer
build failures route into AtlasAI
notifications can expand into AtlasAI chat
logs/history/fixes should be mirrored into Codex-like reusable stores
successful patterns should become reusable future context/snippets under validation rules

Repo implication:

keep this in roadmap
do not overbuild UI around it yet
first lock naming/docs/flow contracts
7. Patch/refactor/audit tooling

This area has also become clear.

Locked:

local patch application matters
audit and refactor tooling should help classify code into keep/merge/archive/universalize buckets
uninstallers should exist for any shell integration
patch workflows should be deterministic
generated artifacts should not pollute source audits

Repo implication:

scripts, patch docs, and audit docs belong in the reset pack
repo hygiene must precede bigger patch waves
Locked repo reset decision set

This is the canon reset. Treat these as fixed.

Locked identity
Atlas Workspace is the host platform
AtlasWorkspace.exe is the primary entry
AtlasUI is the standard tooling UI
AtlasAI is the canonical broker/AI name
NovaForge is a hosted project, not Workspace core
Locked architecture
Workspace core remains generic
project-specific logic lives behind adapters/plugins
editor surface is capped to a focused primary roster
shared panels/services replace one-off editors
logs/build knowledge flow into AtlasAI/Codex systems
Locked UI/backend policy
GDI = fallback only
D3D11 = target primary UI backend
DirectWrite = target primary text backend
OpenGL/GLFW = compatibility-only if retained
Locked doc policy
active docs must reflect only current canon
conflicting older material moves to archive
roadmap statuses must use strict labels only:
Done
In Progress
Partial
Planned
Blocked
Archived
Locked cleanup policy
no .git in audit zips
no build folders in audit zips
no generated artifacts in audit zips
line ending normalization is mandatory
active-path legacy naming must be scrubbed
presence of files does not equal completion
Locked editor policy
no new one-off standalone editors during reset
NovaForge-specific authoring surfaces become adapter panels
project-specific gameplay systems are hosted under broader generic tools
NovaForge Adapter + Gameplay System Panel Contract v1

This is the strongest next artifact and should be locked now.

Purpose

Allow NovaForge-specific systems to extend Workspace without polluting Workspace core.

Core interfaces
IGameProjectAdapter

Responsibilities:

identify project type
register project extensions
expose gameplay/data/editor integrations
provide validation and schema hooks
IProjectPanelFactory

Responsibilities:

create project-specific panels
return descriptors for host insertion
IGameplaySystemPanelProvider

Responsibilities:

enumerate gameplay system panels
define host tool placement
define categories and visibility
expose feature flags
IGameplaySystemSchemaProvider

Responsibilities:

expose project-specific schemas
provide validation for project-authored data
supply property editors or table mappings where needed
Suggested descriptor
struct GameplaySystemPanelDescriptor
{
    std::string id;
    std::string displayName;
    std::string hostToolId;
    std::string category;
    std::string projectId;
    bool defaultVisible = false;
    bool enabled = true;
    std::function<std::unique_ptr<IEditorPanel>()> createPanel;
};
Generic host-tool IDs

Use a small stable set.

workspace.project_browser
workspace.scene_editor
workspace.asset_editor
workspace.material_editor
workspace.visual_logic_editor
workspace.ui_editor
workspace.animation_editor
workspace.data_editor
workspace.build_tool
workspace.project_systems
workspace.atlasai

workspace.project_systems is the key addition for NovaForge gameplay surfaces.

Registration flow
Workspace startup
Workspace loads current project descriptor
Workspace resolves matching project adapter
adapter registers project extensions and panels
host tools request panel descriptors from adapter
allowed panels appear in host tools by category
Panel visibility
some panels hidden by default
future/incomplete systems can be feature-flagged off
adapter decides per-project visibility
First six panel conversions

These should be done first.

GameEconomyEditor -> NovaForgeEconomyPanel
InventoryEditor -> NovaForgeInventoryRulesPanel
ItemShopEditor -> NovaForgeShopPanel
DailyQuestEditor -> NovaForgeMissionRulesPanel
AchievementEditor -> NovaForgeProgressionPanel
CharacterCreatorEditor -> NovaForgeCharacterRulesPanel

All six should be hosted under:

workspace.project_systems
Suggested folder layout
Source/Editor/include/NF/Editor/
  IGameProjectAdapter.h
  IProjectPanelFactory.h
  IGameplaySystemPanelProvider.h
  IGameplaySystemSchemaProvider.h
  GameplaySystemPanelDescriptor.h
  ProjectSystemsTool.h
  GameplaySystemsHostPanel.h

NovaForge/Source/EditorAdapter/include/NovaForge/EditorAdapter/
  NovaForgeAdapter.h
  NovaForgeGameplayPanelRegistry.h
  NovaForgeEconomyPanel.h
  NovaForgeInventoryRulesPanel.h
  NovaForgeShopPanel.h
  NovaForgeMissionRulesPanel.h
  NovaForgeProgressionPanel.h
  NovaForgeCharacterRulesPanel.h

NovaForge/Source/EditorAdapter/src/
  NovaForgeAdapter.cpp
  NovaForgeGameplayPanelRegistry.cpp
  NovaForgeEconomyPanel.cpp
  NovaForgeInventoryRulesPanel.cpp
  NovaForgeShopPanel.cpp
  NovaForgeMissionRulesPanel.cpp
  NovaForgeProgressionPanel.cpp
  NovaForgeCharacterRulesPanel.cpp
Full repo reset milestone plan
Milestone 0. Canon Reset and Consolidation

Success criteria:

README rewritten
canon docs added
roadmap reset added
inventory docs added
.gitattributes added
line endings normalized
active-path naming scrub underway
stale docs archived
editor inventory documented
repo hygiene rules documented
Milestone 1. Structural Truth Pass

Success criteria:

every active top-level tool accounted for
every editor marked keep/merge/archive/defer
every active doc aligned to canon
every active-path stale name scrubbed
generated artifacts removed from snapshot
build/test options documented
project boundaries documented
Milestone 2. Workspace Core Stabilization

Success criteria:

AtlasWorkspace host bootstrap cleaned
app/tool registry rationalized
project adapter contract wired
Workspace generic core enforced
Milestone 3. Backend Strategy Pass

Success criteria:

backend selector contract exists
GDI fallback only
D3D11/DirectWrite path formally targeted
legacy compatibility paths isolated
Milestone 4. Editor Consolidation Pass

Success criteria:

primary tool roster reduced to target set
shared panel extraction real
one-off tools removed from active registry
NovaForge gameplay system panels hosted through adapter
Final blunt summary

Compared to the chats, the zip is not off-course, but it is overgrown and under-normalized.

The project is now clear enough that the right move is:

lock the documentation
lock the reset rules
lock the adapter split
perform cleanup before expansion

That is the correct repo reset state.

The next strongest artifact is a single canonical repo reset pack containing:

rewritten README.md
/Docs/Canon/*
/Docs/Roadmap/*
/Docs/Inventory/*
NovaForge adapter contract doc
editor consolidation doc
repo reset milestone doc
naming canon doc
backend strategy doc
the workspace gui and how the tooling interact

Locked direction:

Workspace GUI

The GUI should behave like a native host OS shell for development tools, not like one giant monolithic editor.

Core shell model

Atlas Workspace is the primary executable and shell. It owns:

global window chrome and theming
dock/tab system
tool launching
project loading
notifications
command routing
AtlasAI access
shared panels
settings
file intake
layout persistence

The shell should feel like a focused desktop environment for dev work.

Main GUI zones
1. Top frame

Persistent workspace chrome.

Contains:

loaded project name
active tool name
global search / command palette
quick actions
window controls
status indicators
2. Atlas bar

This is the app launcher and system strip.

Locked behavior:

labeled Atlas
can auto-hide when idle
launches major tools
exposes notifications and system actions
acts like the shell spine, not just a taskbar clone
3. Central work area

This is where tools open.

Rules:

tools open as docked tabs or detachable windows
each major tool is its own app surface inside Workspace
tools can share panels without duplicating implementations
the work area must support saved layouts per tool and per project
4. Shared side/bottom panel regions

Used for reusable panels.

Examples:

content browser
outliner
inspector
console/log
notifications
AtlasAI chat panel
diagnostics
asset preview
properties/details

These are shared workspace services, not separate editors.

5. Notification and AI edge

A slide-out area tied to notifications and AtlasAI.

Locked behavior:

build/debug notifications appear here
clicking a relevant notification can expand into AtlasAI context
file intake actions can appear here
the AI chat can widen from this region when needed
Tool interaction model

The tooling should interact through a hosted app + shared service model.

Tool types

There should be only a few categories.

Primary tools

These are top-level work surfaces.

Target set:

Project Browser
Scene/World Editor
Asset Editor
Material/Shader Editor
Visual Logic Editor
UI Editor
Animation Editor
Data Editor
Build/Packaging Tool
AtlasAI / Diagnostics / Codex Tool
optional Project Systems Tool for hosted-project gameplay authoring
Shared panels

These are reusable UI surfaces used by many tools.

Examples:

inspector
outliner
content browser
log/output
command palette
properties
notification center
Shared services

These are non-visual systems.

Examples:

command bus
selection model
undo/redo
event bus
asset resolution
layout save/load
validation
AtlasAI broker bridge
Interaction rules

A tool should not directly own everything it uses.

Instead:

tool asks shell for shared services
shell provides shared panels
tool registers commands and contexts
project adapter can inject project-specific panels
notifications and logs flow through common channels

That prevents tool duplication.

Recommended interaction architecture
1. Workspace shell owns lifecycle

The shell creates:

tool registry
panel registry
layout manager
notification manager
command manager
project adapter manager
AtlasAI host
2. Tools register with the shell

Each tool provides:

id
display name
icon id
default layout
commands
supported shared panels
optional project extension points
3. Shared panels bind to active context

Example:

inspector does not belong to only Scene Editor
it binds to whichever tool/context owns current selection
same for outliner, details, log, and search
4. Project adapters extend generic tools

NovaForge should not add six top-level apps.

Instead it injects:

gameplay system panels
project validators
project commands
schemas
templates
data views

into generic host tools.

Best user flow
Boot

Workspace launches into:

Atlas bar
project chooser or recent projects
optional dashboard
notifications
quick access to major tools
Project load

When a project loads:

tool registry updates
project adapter loads
project-specific panels become available
layouts switch to project context
asset/content roots rebind
AtlasAI gains current project context
Tool use

When user opens a tool:

tool appears in central workspace region
allowed shared panels attach around it
commands/context switch to active tool
project adapter can add extra tabs or project panels if supported
Build/debug

When build runs:

output goes to shared log
progress goes to status/notification layer
errors can be pushed to AtlasAI
AtlasAI can present diff/fix suggestions in a controlled UI
GUI principles to lock
Principle 1

Workspace is the shell. Tools are hosted apps.

Principle 2

Panels are reusable. Editors are limited.

Principle 3

Project-specific meaning comes from adapters, not core shell sprawl.

Principle 4

AtlasAI is a shared broker surface, not a random popup.

Principle 5

Everything important routes through common services.

That means:

one selection system
one command system
one notification system
one layout system
one logging system
one project adapter system
Suggested shell subsystems

Minimum host subsystems:

WorkspaceShell
ToolRegistry
PanelRegistry
LayoutManager
CommandManager
SelectionManager
NotificationCenter
ProjectManager
ProjectAdapterManager
AtlasAIHost
AssetIntakeService
LogConsoleService
Suggested tool contract

Each hosted tool should expose something like:

tool id
title
category
startup behavior
default dock layout
supported shared panels
command set
project extension support
context object for selection/inspection
Suggested project adapter contract

NovaForge adapter should expose:

project id
project display name
injected panels
gameplay system tabs
validators
schema providers
content roots
custom commands
feature flags
What this means for NovaForge-specific tooling

Do not keep:

GameEconomyEditor
InventoryEditor
ItemShopEditor
DailyQuestEditor

as separate workspace apps.

Instead:

host them under a Project Systems Tool
inject them through NovaForge adapter
present them as tabs/panels
bind them to shared data editing, validation, and AtlasAI help flows
Final locked model

Atlas Workspace GUI should be:

a native shell
with a launcher/status spine
a docked central work area
shared reusable panels
hosted primary tools
project adapter injection
AtlasAI integrated into notifications, logs, and workflow resolution

Tooling should interact through:

shared services
common context routing
panel reuse
adapter-based project extension
strict separation of generic host vs project-specific meaning

The strongest next artifact is a Workspace GUI + Tool Interaction Contract v1 with exact tool roles, shell regions, panel residency rules, command routing, and adapter injection flow.







all of this logically needs reworked into the true sense of that the workspace needs to be  and that is the workspace housing the AtlasAI  that connects to all the editors around 10 - 15  we have enough panels floating around that there ns no reason these shouldnt be refactored into  standalone editors logically grouping features up and shoving them in the right places for  the editors them selves  some of it should be directly implemented into the workspace its self  the project editors and the Ai panels as well  lets  reset the repo to  address all of this and redo the road map to   implemnt all of this immediatley!  in structured phases lets plan this out
