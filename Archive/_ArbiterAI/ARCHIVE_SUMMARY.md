# Archive: shifty81/ArbiterAI

**Archived:** 2025-07-14
**Source:** https://github.com/shifty81/ArbiterAI
**Source SHA:** 9b2ed68ba44ac29e1585a8240d972e218f61b26c
**Merge Phase:** Phase 8
**Status:** ✅ Done

## Description

ArbiterAI is the AI engine layer for the Arbiter project. It provides a fully local,
ChatGPT-style chat interface for code planning, brainstorming, and roadmap generation
running 100% offline on Windows. It does NOT push or execute code — it is a planning
and ideation engine. Generated snippets and roadmaps feed into the Arbiter tooling repo.

**Stack:** C# WPF / .NET 8 (HostApp) + Python FastAPI (AIEngine/PythonBridge)
**License:** MIT

## Relationship to Arbiter Repo

ArbiterAI is the **earlier, simpler version** of shifty81/Arbiter. Many files have
identical SHAs between the two repos. The Arbiter repo is the more evolved version
with additional features (Installer, VS Extension, IdeWindow, Updater, Docker support,
additional project types). ArbiterAI was kept as a separate repo for independent
deployment of the AI service layer.

### Identical Files (same SHA in both repos)
- HostApp/App.xaml, PdfViewerWindow.xaml, PdfViewerWindow.xaml.cs
- HostApp/WorkspaceWindow.xaml, WorkspaceWindow.xaml.cs
- HostApp/BuildInterface/ (entire directory)
- HostApp/GitInterface/ (entire directory)
- HostApp/Utilities/ (entire directory)
- HostApp/VoiceInterface/ (entire directory)
- HostApp/LauncherWindow.xaml
- archive/webhook-integration/ (entire directory)
- AIEngine/LLaMA2-13B/ (entire directory)

### Files Unique to ArbiterAI (not in Arbiter or different)
- Repo Directive.md (352 KB — large planning document, skipped)
- README.md (9.6 KB — different from Arbiter's)
- roadmap.json (12.8 KB — smaller/earlier version vs Arbiter's 100 KB)
- Arbiter.sln (980 B — simpler, single project vs Arbiter's 1.4 KB multi-project)
- setup_arbiter.py (5.2 KB — slightly different from Arbiter's)
- HostApp versions: App.xaml.cs, AppConfig.cs, ArbiterHost.csproj, LauncherWindow.xaml.cs, MainWindow.xaml/cs, ProjectWindow.xaml/cs, SettingsWindow.xaml/cs, Themes/ (earlier versions, different SHAs)

## Full Repo Audit

### Root Files
| File | Size | Disposition |
|------|------|-------------|
| README.md | 9.6 KB | → `docs_archive/README.md` |
| Arbiter.sln | 980 B | → `usable_snippets/Arbiter.sln` |
| setup_arbiter.py | 5.2 KB | → `usable_snippets/setup_arbiter.py` |
| roadmap.json | 12.8 KB | → `usable_snippets/roadmap.json` |
| .gitignore | 745 B | Standard, not extracted |
| Repo Directive.md | 352 KB | Skipped — too large, planning doc |

### Directories
| Directory | Contents | Disposition |
|-----------|----------|-------------|
| `AIEngine/ArbiterEngine/` | Full agentic backend (port 8001) | Not extracted — overlaps with Arbiter repo |
| `AIEngine/LLaMA2-13B/` | LLaMA model config | Not extracted — identical to Arbiter |
| `AIEngine/PythonBridge/` | FastAPI bridge: fastapi_bridge.py, llm_interface.py, persona_manager.py, VoiceManager.py, model_downloader.py, requirements.txt, static/index.html | Not extracted — overlaps with Arbiter |
| `HostApp/` | C# WPF app: App.xaml, AppConfig.cs, LauncherWindow, MainWindow, ProjectWindow, SettingsWindow, PdfViewerWindow, WorkspaceWindow, BuildInterface/, Config/, GitInterface/, Themes/, Utilities/, VoiceInterface/ | Not extracted — earlier versions; Arbiter repo has more evolved code |
| `Memory/ConversationLogs/` | SQLite chat history + chat_log.md per project | Not extracted — runtime data |
| `Projects/ExampleProject/` | Example project with roadmap.json | Not extracted — example data |
| `archive/webhook-integration/` | Archived webhook code | Not extracted — identical to Arbiter |

## Extracted Files

### docs_archive/ (1 file)
README.md — describes ArbiterAI architecture, API endpoints, integration contract

### usable_snippets/ (3 files)
Arbiter.sln, setup_arbiter.py, roadmap.json — ArbiterAI-specific versions

## Migration Checklist

- [x] Audit source repo contents
- [x] Identify usable code, docs, and assets
- [x] Extract snippets to `Archive/_ArbiterAI/usable_snippets/`
- [x] Archive original docs to `Archive/_ArbiterAI/docs_archive/`
- [x] Merge relevant content into canonical locations in tempnovaforge
- [x] Mark phase as ✅ Done

## Notes

- ArbiterAI is intentionally thin in extracted content because it is the predecessor to Arbiter. The Arbiter repo (Phase 9) contains all evolved code.
- The HostApp C# WPF code, AIEngine Python code, and project structure are well-documented in the archived README for reference.
- The Repo Directive.md (352 KB) was not downloaded due to size; it contains detailed planning/directives that can be accessed from the source repo if needed.

## Original Repo

https://github.com/shifty81/ArbiterAI (to be archived once migration complete)
