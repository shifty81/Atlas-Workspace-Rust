# Archive: shifty81/Arbiter

**Archived:** 2025-07-14
**Source:** https://github.com/shifty81/Arbiter
**Source SHA:** aab52aac6a6c64e0394c6364e870a3d8f6e3aa26
**Merge Phase:** Phase 9
**Status:** ✅ Done

## Description

Arbiter is a self-hosted, fully offline, AI-powered development platform. It is the
evolved version of ArbiterAI, adding a full IDE experience with C# WPF client,
Visual Studio extension, installer, Docker support, and cross-platform capabilities.
The platform provides local LLM inference, agentic code generation, project management,
voice I/O, and a comprehensive Python FastAPI backend.

**Stack:** C# WPF / .NET 8 (HostApp) + Python FastAPI (AIEngine) + InnoSetup (Installer) + VSIX (VS Extension)
**License:** MIT

## Relationship to ArbiterAI Repo

Arbiter is the **more evolved version** of shifty81/ArbiterAI. It contains all ArbiterAI
code plus significant additions: Installer, VisualStudioExtension, IdeWindow, Updater,
GlobalUsings, Docker/docker-compose support, cross-platform docs, additional project
types (Novaforge, SteamServerAdmin), structured log directories, and more mature
HostApp implementations.

## Full Repo Audit

### Root Files
| File | Size | Disposition |
|------|------|-------------|
| README.md | 30 KB | → `docs_archive/README.md` |
| Arbiter.sln | 1.4 KB | → `AtlasAI/Atlas_Arbiter/usable_snippets/` |
| Dockerfile | 5.8 KB | → `AtlasAI/Atlas_Arbiter/usable_snippets/` |
| docker-compose.yml | 3.2 KB | → `AtlasAI/Atlas_Arbiter/usable_snippets/` |
| docker-entrypoint.sh | 2.5 KB | → `AtlasAI/Atlas_Arbiter/usable_snippets/` |
| setup_arbiter.py | 5 KB | → `AtlasAI/Atlas_Arbiter/usable_snippets/` |
| CROSSPLATFORM.md | 5.1 KB | → `docs_archive/CROSSPLATFORM.md` |
| ROADMAP.md | 6.4 KB | → `docs_archive/ROADMAP.md` |
| Specs.md | 8.3 KB | → `docs_archive/Specs.md` |
| Repo Directive.md | 21 KB | Skipped — planning document |
| .gitignore | 757 B | Standard, not extracted |
| roadmap.json | 100 KB | Skipped — too large, machine-generated |
| GUI.md | 323 KB | Skipped — too large, GUI planning notes |
| implement1.md | 173 KB | Skipped — too large, implementation notes |
| arbiter main directives for implementation.md | 10 KB | Skipped — planning document |

### Directories
| Directory | Contents | Disposition |
|-----------|----------|-------------|
| `AIEngine/ArbiterEngine/` | Full agentic backend engine | Not extracted individually — architecture documented in README |
| `AIEngine/LLaMA2-13B/` | LLaMA model configuration | Not extracted — model placeholder |
| `AIEngine/PythonBridge/` | FastAPI bridge server | Not extracted individually — architecture documented in README |
| `AIEngine/arbiter_cli.py` | CLI interface for Arbiter engine (12.6 KB) | → `AtlasAI/Atlas_Arbiter/usable_snippets/AIEngine/` |
| `HostApp/` | C# WPF application: App.xaml/cs, AppConfig.cs, ArbiterHost.csproj, GlobalUsings.cs, IdeWindow.xaml/cs, LauncherWindow.xaml/cs, MainWindow.xaml/cs, ProjectWindow.xaml/cs, SettingsWindow.xaml/cs, PdfViewerWindow.xaml/cs, WorkspaceWindow.xaml/cs, Updater.cs, BuildInterface/, Config/, GitInterface/, Themes/, Utilities/, VoiceInterface/ | Not extracted — C# WPF code, well-documented in README. Key unique files: IdeWindow (11+38KB), Updater.cs (6.6KB), GlobalUsings.cs (1.2KB) |
| `Installer/` | InnoSetup installer: arbiter_setup.iss, build_installer.ps1, README.md, python-embed/ | → `AtlasAI/Atlas_Arbiter/usable_snippets/Installer/` |
| `VisualStudioExtension/` | VSIX extension: ArbiterVSIX/, INSTALL.md | INSTALL.md → `AtlasAI/Atlas_Arbiter/usable_snippets/VisualStudioExtension/` |
| `Memory/ConversationLogs/` | Runtime chat history (SQLite) | Not extracted — runtime data |
| `Projects/ExampleProject/` | Example project scaffold | Not extracted — example data |
| `Projects/Novaforge/` | NovaForge project config | Not extracted — project-specific |
| `Projects/SteamServerAdmin/` | Steam server admin project | Not extracted — project-specific |
| `docs/design/` | Design documentation | Not extracted — planning docs |
| `docs/wiki/` | Wiki documentation | Not extracted — wiki docs |
| `logs/` | 6 log directories (arbiter_engine, host_app, python_bridge, self_build, steam_server_admin, vs_extension) | Not extracted — empty .gitkeep placeholders |
| `archive/webhook-integration/` | Archived webhook code | Not extracted — archived predecessor code |
| `.github/workflows/` | CI workflows | Not extracted |

## Extracted Files

### AtlasAI/Atlas_Arbiter/usable_snippets/ (10 files)
Arbiter.sln, Dockerfile, docker-compose.yml, docker-entrypoint.sh, setup_arbiter.py,
AIEngine/arbiter_cli.py, Installer/arbiter_setup.iss, Installer/build_installer.ps1,
Installer/README.md, VisualStudioExtension/INSTALL.md

### docs_archive/ (4 files)
README.md, CROSSPLATFORM.md, ROADMAP.md, Specs.md

## Migration Checklist

- [x] Audit source repo contents
- [x] Identify usable code, docs, and assets
- [x] Extract snippets to `AtlasAI/Atlas_Arbiter/usable_snippets/`
- [x] Archive original docs to `Archive/_Arbiter/docs_archive/`
- [x] Merge relevant content into canonical locations in tempnovaforge
- [x] Mark phase as ✅ Done

## Notes

- The HostApp C# WPF code (200+ KB across xaml/cs files) was not individually extracted but is well-documented in the archived README. Key unique files vs ArbiterAI: IdeWindow (embedded IDE), Updater.cs (auto-update), GlobalUsings.cs.
- The AIEngine/PythonBridge/ directory contains the FastAPI backend with 12+ LLM backends — architecture documented in README.
- Large generated/planning files (GUI.md 323KB, implement1.md 173KB, roadmap.json 100KB) were intentionally skipped.
- The Installer/ with InnoSetup scripts provides a reference for Windows distribution packaging.
- The VisualStudioExtension/ provides VS integration — INSTALL.md extracted for reference.

## Original Repo

https://github.com/shifty81/Arbiter (to be archived once migration complete)
