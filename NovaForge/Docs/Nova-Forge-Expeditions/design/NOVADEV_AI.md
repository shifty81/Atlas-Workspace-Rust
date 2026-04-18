# NovaForge Dev AI — Design Specification

**Status:** Phases 0–2 complete. Phases 3–7 planned.

---

## Overview

A fully offline, in-project AI development assistant for NovaForge. It provides an interactive REPL and auto-iterate loop powered by a local LLM (Ollama / LM Studio), requiring no cloud API keys.

Located in `ai_dev/` at the repository root.

---

## Architecture

```
ai_dev/
├── core/
│   ├── agent_loop.py       — REPL + CLI entry point
│   ├── context_manager.py  — repository index + LLM context builder
│   ├── llm_interface.py    — Ollama / LM Studio REST client
│   └── prompt_templates.py — subsystem-specific prompt preambles
├── tools/
│   ├── build_runner.py     — CMake/C#/Lua/Blender/GLSL runner
│   ├── ecs_scaffold.py     — ECS system stub generator
│   ├── feedback_parser.py  — GCC/MSVC/CMake error parser
│   ├── file_ops.py         — safe R/W with snapshot/rollback
│   ├── git_ops.py          — git integration
│   ├── hot_reload.py       — file watcher + IPC reload dispatch
│   ├── ipc_bridge.py       — TCP client for Atlas Engine IPC
│   ├── blender_bridge.py   — headless Blender asset generation
│   ├── pcg_learner.py      — placement learning from approved edits
│   ├── plugin_loader.py    — optional plugin/extension loader
│   ├── media_pipeline.py   — AI texture/model/audio/video stubs
│   ├── stable_diffusion_interface.py — SD WebUI stub
│   ├── audio_pipeline.py   — TTS/SFX/music generation stubs
│   ├── video_pipeline.py   — FFmpeg video assembly stubs
│   ├── versioning.py       — semver + changelog generation
│   ├── zip_tools.py        — archive utilities
│   ├── installer_builder.py — NSIS/portable package stubs
│   └── qa_pipeline.py      — test runner + JSON validation
├── tests/                  — Python unit tests (76 tests, all passing)
├── models/README.md        — model selection guide
├── plugins/                — optional plugin packages (empty)
├── workspace/              — runtime data (ignored by git except .gitkeep)
│   └── snapshots/          — per-edit file snapshots for rollback
└── README.md               — quick-start guide
```

---

## Implementation Phases

### Phase 0 — Seed Baseline ✅
- Interactive REPL (`agent_loop.py`)
- Local LLM via Ollama/LM Studio REST API
- Repository context indexer with keyword scoring
- Safe file I/O with snapshot/rollback
- CMake build runner (configure, build, run tests)
- GCC/Clang/MSVC/linker error parser
- ECS scaffold generator

### Phase 1 — Iteration Tools ✅
- Auto-iterate loop: prompt → code → build → parse errors → LLM fix
- Test runner (per-system and all-tests)
- Git operations (status/diff/log/commit/branch/stash)
- CLI mode (`--prompt`, `--auto`, `--auto-approve`, `-n`)
- Session persistence (JSON log)
- `missing` command: list ECS systems missing .cpp or test

### Phase 2 — Hot Reload + Multi-Language ✅
- IPC bridge: TCP client for Atlas Engine hot-reload signals
- Hot reload manager: Lua, GLSL, JSON, UI panels via IPC
- C# / .NET build and test runner
- GLSL directory validator (glslangValidator)
- Language-aware prompt templates (11 subsystems)
- Blender headless script runner (Phase 6 prerequisite)
- Plugin loader for optional extensions

### Phase 3 — Overlay 📋
- `AIPromptPanel` in Atlas Editor (top-right overlay)
- `AISuggestionPanel` (diff preview + approve/reject)
- Hotkey: `Ctrl+~` to toggle the AI panel

### Phase 4 — Placement + PCG 📋
- Free-move placement mode in the editor
- `PCGLearner`: record approved placements → suggest defaults
- Automatic PCG rule generation from samples

### Phase 5 — GUI + Shaders 📋
- AtlasUI layout generation from text prompts
- GLSL shader generation from natural language descriptions

### Phase 6 — Asset Generation 📋
- Blender bridge for procedural props from text prompts
- Ship hull generation via BlenderSpaceshipGenerator
- Stable Diffusion interface for textures and icons

### Phase 7 — Full Automation 📋
- Test generation from system headers
- Release pipeline (version bump → tag → build → package → installer)
- Multi-agent architecture (dedicated code/test/QA agents)

---

## Configuration

| Variable | Default | Description |
|----------|---------|-------------|
| `LLM_BASE_URL` | `http://localhost:11434` | Ollama or LM Studio API endpoint |
| `LLM_MODEL` | `deepseek-coder` | Model to use |
| `LLM_TIMEOUT` | `120` | Request timeout in seconds |
| `NOVAFORGE_ROOT` | auto-detected | Repository root path |
| `BLENDER_PATH` | auto-detected | Path to Blender executable |

---

## Running

```bash
# Interactive session
python ai_dev/core/agent_loop.py

# One-shot prompt
python ai_dev/core/agent_loop.py --prompt "Show me all ECS systems missing .cpp"

# Auto-iterate: fix build errors
python ai_dev/core/agent_loop.py --auto "Fix the linker errors" --auto-approve -n 10

# Run unit tests
python -m unittest discover -s ai_dev/tests -v
```

---

## Related Documents

- [`docs/CODING_GUIDELINES.md`](../CODING_GUIDELINES.md)
- [`ai_dev/models/README.md`](../../ai_dev/models/README.md)
- [`ai_dev/plugins/README.md`](../../ai_dev/plugins/README.md)
