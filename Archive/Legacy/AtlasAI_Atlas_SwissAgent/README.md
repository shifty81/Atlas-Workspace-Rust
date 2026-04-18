# Atlas_SwissAgent — Conversational AI Query Tool

> **Canonical Name:** Atlas_SwissAgent (part of AtlasAI)
> **Legacy Name:** SwissAgent
> **Legacy Path:** `Tools/SwissAgent/`

## Overview

Atlas_SwissAgent is a multi-purpose conversational AI agent that provides
workspace-aware code generation, analysis, and query capabilities. It is the
"ask anything" front-end for the NovaForge development workflow.

## Capabilities

1. Context-aware code queries (understands project structure)
2. Code generation from natural language prompts
3. Code review and refactoring suggestions
4. Session-based conversations with history

## Roadmap

| Task | Description | Status |
|------|-------------|--------|
| SA-1 | CLI scaffold and session management | ⬜ Queued |
| SA-2 | Context-aware project indexing | ⬜ Queued |
| SA-3 | Code generation from prompts | ⬜ Queued |
| SA-4 | Editor panel integration | ⬜ Queued |
| SA-5 | Full multi-turn session support | ⬜ Queued |

## Pipeline Contract

- **Reads:** `pipeline/changes/*.change.json` events
- **Writes:** `pipeline/changes/<timestamp>_atlasai_SwissResponse.change.json`
- **Never** writes directly to source files

## Reference Implementation

The legacy reference implementation is at `Tools/SwissAgent/`. See its README for
the original design notes. Active development will occur in this directory once
S1 Tool Wiring is complete.
