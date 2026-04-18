# Archive: shifty81/SwissAgent

**Archived:** 2025-07-14
**Source:** https://github.com/shifty81/SwissAgent
**Source SHA:** d5e7655241e87b782c4802fb48a203ecb7fe805d
**Merge Phase:** Phase 7
**Status:** ✅ Done

## Description

SwissAgent is a self-hosted, fully offline AI development platform built in Python.
It provides a Monaco-based web IDE, 42+ capability modules (200+ tools), multi-backend
LLM support (Ollama, LocalAI, OpenWebUI, API, local GGUF), an agentic AI loop, plugin
system, autonomous self-build capabilities, and comprehensive REST API.

**Stack:** Python 3.10+ / FastAPI / Monaco Editor / Electron (optional) / C++ native (optional)
**License:** MIT

## Full Repo Audit

### Root Files
| File | Size | Disposition |
|------|------|-------------|
| README.md | 18 KB | → `docs_archive/README.md` |
| pyproject.toml | 821 B | → `AtlasAI/Atlas_SwissAgent/usable_snippets/` |
| Dockerfile | 850 B | → `AtlasAI/Atlas_SwissAgent/usable_snippets/` |
| docker-compose.yml | 3.5 KB | → `AtlasAI/Atlas_SwissAgent/usable_snippets/` |
| LICENSE | 35 KB | MIT — standard, not extracted |
| .gitignore | 1.3 KB | Standard, not extracted |
| Chat.md | 545 KB | Chat log — skipped (too large, ephemeral) |
| ideas.md | 133 KB | Brainstorm notes — skipped (ephemeral) |
| new implementations.md | 942 KB | Implementation notes — skipped (too large, ephemeral) |
| install.log | 10 KB | Build artifact — skipped |

### Directories
| Directory | Contents | Disposition |
|-----------|----------|-------------|
| `core/` | 12 Python files — agent loop, CLI, API server, config, permissions, plugin/module loaders, self-build, task runner, tool registry | Key files → `AtlasAI/Atlas_SwissAgent/usable_snippets/core/` (api_server.py 423KB skipped — too large) |
| `llm/` | 13 Python files — LLM backend adapters (Ollama, Anthropic, Gemini, LMStudio, LocalAI, OpenWebUI, Tabby, llama.cpp, local GGUF) | Key files → `AtlasAI/Atlas_SwissAgent/usable_snippets/llm/` |
| `modules/` | 40 subdirectories — ai_persona, animation, api, asset, audio, binary, blender, build, cache, ci, database, debug, doc, editor, filesystem, git, image, import_project, index, installer, job, knowledge, memory, network, package, pipeline, profile, project_profile, render, resource, roadmap, scaffold, script, security, server, shader, template, test, tile, ui, zip | Not extracted individually (each contains `__init__.py` + tools.json + module code). Noted for reference. |
| `plugins/` | 3 plugin dirs: example, open_webui_tool, test_api_scaffold_plugin | Not extracted — plugin examples |
| `tools/` | 5 Python files — build_runner, feedback_parser, media_pipeline, registry | → `AtlasAI/Atlas_SwissAgent/usable_snippets/tools/` |
| `audio_pipeline/` | tts_sfx.py — offline TTS and SFX generation | → `AtlasAI/Atlas_SwissAgent/usable_snippets/audio_pipeline/` |
| `stable_diffusion/` | stable_diffusion_interface.py — AUTOMATIC1111 interface | → `AtlasAI/Atlas_SwissAgent/usable_snippets/stable_diffusion/` |
| `stage_manager/` | stage_manager.py — project milestone tracker | → `AtlasAI/Atlas_SwissAgent/usable_snippets/stage_manager/` |
| `dev_mode/` | self_upgrade.py — agent self-upgrade and module patching | → `AtlasAI/Atlas_SwissAgent/usable_snippets/dev_mode/` |
| `gui/` | app.js (332KB), index.html (89KB), style.css (60KB) — Monaco web IDE | Skipped — too large, tightly coupled to SwissAgent backend |
| `electron/` | main.js, preload.js, launcher.html, package.json, README.md — Electron wrapper | Not extracted — optional wrapper |
| `native/` | CMakeLists.txt, README.md, src/, scripts/, assets/ — Win32/WebView2 C++ native app | Not extracted — platform-specific |
| `configs/` | config.toml | → `AtlasAI/Atlas_SwissAgent/usable_snippets/configs/` |
| `scripts/` | install.sh, setup.py, docker-build.sh, run_tests.py | → `AtlasAI/Atlas_SwissAgent/usable_snippets/scripts/` |
| `templates/` | cpp/, default/, javascript/, python/ — project scaffolding templates | Not extracted — scaffolding templates |
| `tests/` | test_api.py (222KB), test_core.py (6.3KB), test_filesystem.py (1.3KB), test_modules.py (40KB) | Not extracted — test suite (large files, SwissAgent-specific) |
| `docs/` | README.md, SETUP.md, self_iteration.md, AI_INTEGRATION_AUDIT.md, banner.svg | → `docs_archive/` |
| `workspace/` | Task files, notes, roadmap.json, sample_project/ — runtime workspace data | Not extracted — runtime/ephemeral |
| `projects/` | .gitkeep only | Empty placeholder |
| `cache/`, `logs/`, `models/` | .gitkeep only | Empty placeholders |
| `.github/workflows/` | CI workflow(s) | Not extracted |

## Extracted Files

### AtlasAI/Atlas_SwissAgent/usable_snippets/ (30 files)
Core agent architecture, LLM abstraction, tools, audio/image/stage pipelines, config, scripts, Docker.

### docs_archive/ (6 files)
README.md, docs_README.md, SETUP.md, self_iteration.md, AI_INTEGRATION_AUDIT.md, banner.svg

## Migration Checklist

- [x] Audit source repo contents
- [x] Identify usable code, docs, and assets
- [x] Extract snippets to `AtlasAI/Atlas_SwissAgent/usable_snippets/`
- [x] Archive original docs to `Archive/_SwissAgent/docs_archive/`
- [x] Merge relevant content into canonical locations in tempnovaforge
- [x] Mark phase as ✅ Done

## Notes

- The full modules/ directory (40 subdirectories) was not extracted individually — each module follows a consistent pattern (\_\_init\_\_.py + tools.json + handler code). The module architecture is documented in the README.
- Large generated/ephemeral files (Chat.md, ideas.md, new implementations.md, gui/app.js, core/api_server.py, test files) were intentionally skipped.
- The native/ C++ app and electron/ wrapper are platform-specific and not needed for the consolidated repo.

## Original Repo

https://github.com/shifty81/SwissAgent (to be archived once migration complete)
