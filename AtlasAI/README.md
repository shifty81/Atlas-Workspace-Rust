# AtlasAI — Unified AI Broker

> **Canonical Name:** AtlasAI
> **Legacy Names:** ArbiterAI, SwissAgent, Arbiter — all consolidated and archived.
> **Canonical Location:** `AtlasAI/` (repo root)

## Overview

AtlasAI is the **single AI integration point** for Atlas Workspace. It provides
the broker layer through which all AI-driven functionality flows — build log
analysis, code suggestions, notifications, and diagnostic assistance.

AtlasAI is a workspace-level shared service, not a standalone application.
It is hosted inside the workspace shell and accessible from all tools.

## Architecture

AtlasAI operates as a broker surface within the workspace:

```
┌──────────────────────────────────────────┐
│            Atlas Workspace Shell         │
│                                          │
│  ┌────────────────────────────────────┐  │
│  │          AtlasAI Broker            │  │
│  │                                    │  │
│  │  - Build log routing              │  │
│  │  - Notification → AI expansion    │  │
│  │  - Code suggestions               │  │
│  │  - Diagnostic assistance          │  │
│  │  - Codex snippet mirroring        │  │
│  └────────────────────────────────────┘  │
│                                          │
│  Integrated into:                        │
│  - AtlasAI Panel (workspace.atlasai)     │
│  - Notification edge                     │
│  - Console/log routing                   │
└──────────────────────────────────────────┘
```

## Integration Points

| Integration | Method |
|-------------|--------|
| Workspace → AtlasAI | AtlasAIPanelHost, AIPanelSession, AIActionSurface |
| Build → AtlasAI | Build log routing via LoggingRouteV1 |
| Codex → AtlasAI | Snippet mirroring via CodexSnippetMirror |
| Debug → AtlasAI | Debug path via AIDebugPathV1 |

## Naming Canon

All AI features follow the `AtlasAI` prefix or `NF::` namespace:

- `NF::AtlasAIPanelHost`
- `NF::AIPanelSession`
- `NF::AIActionSurface`
- `NF::AIDebugPathV1`
- `NF::CodexSnippetMirror`

See `Docs/Canon/03_NAMING_CANON.md` for full naming rules.

## Current Status

- **Phase:** Milestone 0 complete, workspace integration planned for Phase 4
- **Naming:** Fully migrated from Arbiter → AtlasAI
- **Legacy:** Atlas_Arbiter and Atlas_SwissAgent archived to `Archive/Legacy/`

## Legacy Tool Migration

| Legacy Path | Status |
|-------------|--------|
| `Tools/ArbiterAI/` | ✅ Archived to `Archive/Legacy/Tools_ArbiterAI/` |
| `Tools/SwissAgent/` | ✅ Archived to `Archive/Legacy/Tools_SwissAgent/` |
| `AtlasAI/Atlas_Arbiter/` | ✅ Archived to `Archive/Legacy/AtlasAI_Atlas_Arbiter/` |
| `AtlasAI/Atlas_SwissAgent/` | ✅ Archived to `Archive/Legacy/AtlasAI_Atlas_SwissAgent/` |
