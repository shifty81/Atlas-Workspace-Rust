#pragma once
// NF::EditorToolRegistry — Tool ecosystem and registration includes.
//
// This header provides access to the tool registration system,
// primary tool definitions, and shared services used across the editor.
//
// Primary tools should register through this system.
// See Docs/Canon/05_EDITOR_STRATEGY.md for tool roster policy.

// ── Tool registration and ecosystem ─────────────────────────────
#include "NF/Editor/ToolEcosystem.h"
#include "NF/Editor/ToolWindowManager.h"
#include "NF/Editor/EditorToolbar.h"
#include "NF/Editor/PluginSystem.h"
#include "NF/Editor/ProjectServices.h"

// ── Shared services ─────────────────────────────────────────────
#include "NF/Workspace/EditorEventBus.h"
#include "NF/Workspace/ConsoleCommandBus.h"
#include "NF/Editor/AssetDatabase.h"
#include "NF/Editor/AssetImporters.h"
#include "NF/Editor/BlenderImporter.h"
#include "NF/Editor/BuildConfiguration.h"
#include "NF/Editor/BuildReport.h"
#include "NF/Editor/PlatformProfile.h"
#include "NF/Editor/ResourceMonitor.h"
#include "NF/Editor/FrameStatsTracker.h"
#include "NF/Editor/Profiling.h"
#include "NF/Editor/HotReload.h"
#include "NF/Editor/VersionControl.h"
#include "NF/Editor/Collaboration.h"
#include "NF/Editor/IDEIntegration.h"
#include "NF/Editor/Scripting.h"
#include "NF/Editor/ScriptingConsole.h"
#include "NF/Editor/Localization.h"
#include "NF/Editor/ThemeManager.h"
#include "NF/Editor/AssetDependencies.h"

// ── Serialization services ──────────────────────────────────────
#include "NF/Workspace/PanelStateSerializer.h"
#include "NF/Workspace/DockTreeSerializer.h"
#include "NF/Workspace/LayoutManagerV1.h"

// ── AtlasAI services ────────────────────────────────────────────
#include "NF/Workspace/AtlasAIPanelHost.h"
#include "NF/Workspace/AIPanelSession.h"
#include "NF/Workspace/AIActionSurface.h"
#include "NF/Workspace/AIIntegration.h"
#include "NF/Workspace/AIDebugPathV1.h"
#include "NF/Workspace/LoggingRouteV1.h"
#include "NF/Workspace/CodexSnippetMirror.h"

// ── File intake ─────────────────────────────────────────────────
#include "NF/Workspace/FileIntakePipeline.h"
#include "NF/Workspace/DropTargetHandler.h"
#include "NF/Workspace/AssetImportQueue.h"

// ── UI kit services ─────────────────────────────────────────────
#include "NF/Editor/WidgetKitV1.h"
#include "NF/Editor/TooltipSystemV1.h"
#include "NF/Editor/TabBarV1.h"
#include "NF/Editor/CommandPaletteV1.h"
#include "NF/Editor/HotkeyRegistryV1.h"
#include "NF/Editor/GestureRecognizerV1.h"
#include "NF/Editor/TypographySystem.h"
#include "NF/Editor/IconographySpec.h"

// ── Project surfaces ────────────────────────────────────────────
#include "NF/Editor/ProjectSurfaceV1.h"
#include "NF/Editor/RepoSurfaceV1.h"
