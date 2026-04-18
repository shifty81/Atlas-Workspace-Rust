#pragma once
// NF::Editor — Minimal editor core include.
//
// This header includes only the essential editor infrastructure.
// For tool-specific or feature-specific includes, use:
//   - EditorSharedPanels.h   (shared reusable panels)
//   - EditorToolRegistry.h   (tool registration and ecosystem)
//   - CoreToolRoster.h       (registers the ~8 primary tools into a shell)
//
// Individual headers can always be included directly for faster compilation.

// ── Core editor infrastructure ──────────────────────────────────
#include "NF/Editor/EditorPanel.h"
#include "NF/Editor/EditorTheme.h"
#include "NF/Editor/DockLayout.h"
#include "NF/Editor/MenuBar.h"
#include "NF/Editor/CommandPalette.h"
#include "NF/Editor/ShortcutManager.h"
#include "NF/Editor/EditorSettings.h"
#include "NF/Editor/EditorCamera.h"

// ── Workspace shell (canonical location: NF/Workspace) ──────────
#include "NF/Workspace/WorkspaceShell.h"
#include "NF/Workspace/IHostedTool.h"
#include "NF/Workspace/ToolRegistry.h"
#include "NF/Workspace/PanelRegistry.h"
#include "NF/Workspace/WorkspaceAppRegistry.h"
#include "NF/Workspace/WorkspaceLaunchContract.h"
#include "NF/Workspace/WorkspacePanelHost.h"
#include "NF/Workspace/WorkspaceInputBridge.h"
#include "NF/Workspace/WorkspaceShellContract.h"
#include "NF/Workspace/WorkspaceLayout.h"
#include "NF/Workspace/EditorEventBus.h"
#include "NF/Workspace/SelectionService.h"
#include "NF/Workspace/UndoRedoSystem.h"
#include "NF/Workspace/LayoutPersistence.h"

// ── Shared panels (always available) ────────────────────────────
#include "NF/Editor/EditorSharedPanels.h"

// ── Tool ecosystem and registration ─────────────────────────────
#include "NF/Editor/EditorToolRegistry.h"

// ── Primary tool roster ─────────────────────────────────────────
#include "NF/Editor/CoreToolRoster.h"
