# Locked Project Direction

## Workspace and project boundary
- Atlas Workspace is the main development environment and tooling shell.
- NovaForge is the main hosted project under active development in this environment.
- NovaForge stays logically detachable and should not be built by default as part of workspace builds.
- Atlas Workspace is the build, edit, packaging, and release environment for hosted projects.
- GitHub integration should exist directly in the workspace.
- The Git tool should be its own app inside the workspace shell.

## Platform and UI direction
- Win32 is the baseline platform and runtime direction for initial built tools and applications.
- AtlasUI is the standard UI framework for all development-facing tooling and interfaces.
- No ImGui for canonical tooling UI.
- Use a custom retained-mode UI framework with backend abstraction.
- GPU-first main rendering path for tooling UI, with GDI fallback for safe mode/bootstrap/recovery.

## UI standardization
All development-facing surfaces should share:
- docking
- floating windows
- tabs
- panel chrome
- rounded corners
- hover states
- pressed-state button behavior
- tooltips
- theme tokens
- command/shortcut routing
- menu systems
- consistent sizing and layout behavior

## Workspace goals
The workspace should eventually host:
- editor shell
- AtlasAI chat/broker panel
- notification center
- IDE/tool windows
- asset library
- snippet/codex surfaces
- project intake flows
- control panel/settings
- file and repo operations

## Tooling-first priority rule
Actual game development work should be pushed to the back of the roadmap unless explicitly chosen.
AtlasUI/framework/tooling standardization should be at the front of the roadmap by default.

## AI/broker direction
- AtlasAI is the only visible middle-man broker.
- Preferred flow: local audit/Codex first, then web research, then OpenAI model layer, then optional hidden fallback providers.
- Build errors and logs should route into AtlasAI workflows.

## Logging and debugging direction
- Build logs and log structures should be packaged into a `.logger` system file.
- Logs should be archived into Codex for later debugging and bug-fix retrieval.
- AtlasAI should be able to offer debug/fix proposals against logged errors.

## NovaForge gameplay priority rule
By default, gameplay expansion is not the front-line roadmap item.
Tooling, framework, shell, workflow, and editor systems come first.
