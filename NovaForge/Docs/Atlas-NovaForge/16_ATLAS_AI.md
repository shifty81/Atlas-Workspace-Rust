# AtlasAI — Intelligent Authoring Assistant

## Overview

AtlasAI is the expanded AI-assisted authoring system for the Atlas Editor.
It builds on the AI aggregator described in `08_AI_EDITOR_ASSIST.md` and adds
web aggregation, context-aware assistance, and full editor integration.

Core principles:

- AI is a tooling aid — never runtime logic
- Context-aware, permission-scoped assistant
- Operates on structured data (assets, graphs, schemas) — never raw magic
- Editor-only: disabled in CI, never in runtime
- If AI is disabled completely, the project still builds and replays identically

## Architecture

```
Editor Context
   ↓
Context Snapshot (read-only)
   ↓
Prompt Builder
   ↓
AI Backend (local or remote)
   ↓
Structured Response (JSON / DSL / AST)
   ↓
Preview + Diff
   ↓
User Approval
   ↓
Deterministic Apply
```

## AtlasAI Core (Orchestrator Model)

Atlas's own AI layer is the authority. External models (ChatGPT, web LLMs) are
pluggable advisors that feed into the orchestrator, not replacements for it.

Three layers, in priority order:

1. **Offline templates** — deterministic, no network required
2. **Web-indexed documentation and knowledge** — local knowledge base built from crawled docs
3. **External LLM integration** — optional, remote or local models

## Context Awareness

Structured context packets are sent to the AI backend, never the whole project.

Context includes:

- Editor mode and active panel
- Selected assets and their schemas
- Open graph context (nodes, edges, subgraph)
- Engine version and feature flags
- User permission level

This keeps responses relevant, cheap, and predictable.

## AI Intents

- **Explain** — describe assets, graphs, systems
- **Generate** — create GUI DSL, flow graphs, asset schemas, AtlasScript
- **Lint** — check graphs for structural issues
- **Debug** — diagnose replay divergence
- **Refactor** — AI-assisted flow graph refactoring
- **Tutorial** — generate interactive tutorials

## Allowed Output Types

- GUI DSL ✅
- Flow Graph JSON ✅
- Asset schemas ✅
- AtlasScript ✅
- Explanations ✅
- Diffs / patches ✅
- C++ code ⚠️ (suggestion only, never auto-applied)

## Safety and Determinism Rules

- AI output is not part of the runtime
- Generated assets are hashed like any other asset
- Replay files never depend on AI presence
- Every output follows: preview → diff → approve
- No mutations without explicit user approval
- All actions are permission-gated

## Permission Scoping

| Permission | Can Generate Assets | Can Suggest Code | Can Explain |
|------------|---------------------|------------------|-------------|
| ViewOnly   | ✖                   | ✖                | ✔           |
| QA         | ✖                   | ✖                | ✔           |
| Developer  | ✔                   | ✔                | ✔           |
| CI         | ✖                   | ✖                | ✖           |
| Admin      | ✔                   | ✔                | ✔           |

## Editor Panels

- **Atlas Assistant panel** — dockable, toggled with `Ctrl+Shift+A`
- **Context menu** — right-click "Ask Atlas about this..." on any asset or node
- **Assistant Diff Viewer** — preview generated changes before applying
- **Prompt history** — recent prompts and suggestions

## Backend Options

- **Remote** — OpenAI, Azure, other LLM APIs
- **Local** — local LLM exposed via API
- **Disabled** — offline or CI mode, no AI calls

All backends are abstracted behind the `IAssistantBackend` interface.

## "Add Content to Project" Flow

1. User prompt: "Add a basic inventory system"
2. AI generates: assets, flow graphs, optional UI DSL
3. Editor shows: asset tree preview, graph diff, schema validation
4. User clicks Apply
5. Assets added via the normal editor pipeline
6. No shortcuts, no magic

## Web Aggregation

- Own web crawler/indexer for documentation and knowledge sources
- Builds a local knowledge base to supplement offline templates
- Privacy-respecting: no project data is sent without explicit consent
- Indexed content is cached and versioned locally

## LLM Backend Configuration

Atlas supports multiple LLM backend options for AI-assisted authoring:

### 1. Offline Mode (Default)

The offline stub backend is used by default when no LLM configuration is provided. It returns deterministic canned responses and requires no network access.

```cpp
// Offline mode is automatic - no configuration needed
// The editor will use OfflineLLMBackend automatically
```

### 2. HTTP LLM Backend (Production)

Configure via environment variables:

```bash
# Required
export ATLAS_LLM_ENDPOINT="https://api.openai.com/v1/chat/completions"
export ATLAS_LLM_MODEL="gpt-4"
export ATLAS_LLM_API_KEY="your-api-key-here"

# Optional
export ATLAS_LLM_TIMEOUT_MS="30000"  # Default: 30 seconds
```

Supported API providers:
- **OpenAI** (GPT-3.5, GPT-4, GPT-4-turbo, and newer models)
- **Azure OpenAI** (with Azure-specific endpoint)
- **Local LLMs** (via OpenAI-compatible API like llama.cpp server, Ollama)
- **Anthropic Claude** (with compatible wrapper)

**Note**: Model availability depends on your API provider and account permissions. Check with your provider for the latest model options.

### 3. Using LLMBackendFactory

The recommended way to create an LLM backend:

```cpp
#include "engine/ai/LLMBackend.h"

// Create from environment variables
auto httpClient = /* your IHttpClient implementation */;
auto backend = atlas::ai::LLMBackendFactory::CreateFromEnv(httpClient);

// Or create with explicit parameters
auto backend = atlas::ai::LLMBackendFactory::Create(
    httpClient,
    "https://api.openai.com/v1/chat/completions",
    "gpt-4",
    "your-api-key-here"
);

// Register with the global registry
atlas::ai::LLMBackendRegistry registry;
registry.SetBackend(backend);
```

### 4. Local LLM Setup (Privacy-Focused)

For users who prefer to keep data local:

```bash
# Example: Using Ollama (https://ollama.com/)
ollama serve  # Start Ollama server on port 11434

# Configure Atlas to use local endpoint
export ATLAS_LLM_ENDPOINT="http://localhost:11434/v1/chat/completions"
export ATLAS_LLM_MODEL="llama2"
export ATLAS_LLM_API_KEY="not-required-for-local"
```

### Environment Variable Reference

| Variable | Required | Default | Description |
|----------|----------|---------|-------------|
| `ATLAS_LLM_ENDPOINT` | Yes* | None | API endpoint URL |
| `ATLAS_LLM_MODEL` | Yes* | None | Model identifier |
| `ATLAS_LLM_API_KEY` | Yes* | None | API authentication key |
| `ATLAS_LLM_TIMEOUT_MS` | No | 30000 | Request timeout in milliseconds |

*Required only when using HTTP LLM backend. Offline mode works without any configuration.

### Testing Your Configuration

```bash
# Build the editor
./build.sh editor

# Run with LLM backend (environment variables must be set)
./dist/AtlasEditor

# The AI assistant will automatically use the configured backend
# Check the console for connection status messages
```

### Troubleshooting

**Problem**: "LLM backend not available"
- **Solution**: Check that environment variables are set correctly
- Verify API key is valid
- Test endpoint connectivity: `curl -v $ATLAS_LLM_ENDPOINT`

**Problem**: "Request timeout"
- **Solution**: Increase `ATLAS_LLM_TIMEOUT_MS`
- Check network connectivity
- Try a faster model or reduce `maxTokens` in requests

**Problem**: "API key invalid"
- **Solution**: Verify API key is correct
- Check that key has proper permissions
- Ensure no extra whitespace in environment variable

### Security Notes

- API keys are **never** logged or included in crash reports
- Keys are loaded from environment variables only at startup
- No API keys are stored in configuration files or source code
- All network traffic should use HTTPS endpoints
- Consider using local LLM for sensitive projects

## See Also

- `08_AI_EDITOR_ASSIST.md` — AI aggregator and validation layer
- `15_FLOW_GRAPH.md` — flow graph system targeted by AI generation
- `12_GUI_SYSTEM.md` — GUI DSL that AI can produce
- `engine/ai/LLMBackend.h` — LLM backend interface and implementation
