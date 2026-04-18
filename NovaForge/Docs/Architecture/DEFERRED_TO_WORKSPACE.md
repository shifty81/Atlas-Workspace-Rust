# NovaForge — Items Deferred to Atlas Workspace

This document lists capabilities that belong to Atlas Workspace, not NovaForge.

NovaForge provides the project-side contract. Workspace provides the implementation.

## Deferred Items

### Shell and Launcher
- Window chrome and OS-level window management
- Application lifecycle (minimize, tray, auto-update)

### AtlasAI Broker
- LLM/AI query dispatch
- Context window management
- Multi-session handling

### Codex System
- Snippet library hosting
- Fix history storage
- Cross-project search

### Build Log Routing
- .logger file watching
- Log→AI dispatch
- Diff/apply review UI

### Account Linking
- GitHub OAuth
- Google account
- License management

### Drag-Drop Shell Integration
- File type routing
- Asset intake notification
- Cross-surface drag handling

### Asset Library
- Remote asset browsing
- Workspace-wide reuse catalog

## What NovaForge Must Provide (Project-Side Contract)
- `Project/project.atlas.json` (manifest)
- `Logs/build.logger` (log output format)
- `Content/Incoming/` (intake directory)
- `Codex/` (local codex stubs, synced by Workspace)
- `NF::Pipeline` events (watched by Workspace)
