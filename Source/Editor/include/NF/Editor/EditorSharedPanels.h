#pragma once
// NF::EditorSharedPanels — Includes for all shared reusable panels.
//
// These panels are owned by workspace core and shared across multiple tools.
// Include this header when you need access to the common panel set.

// ── Workspace-owned shared panels (ISharedPanel implementations) ──
#include "NF/Workspace/SharedPanels.h"

// ── Core shared panels ──────────────────────────────────────────
#include "NF/Editor/ViewportPanel.h"
#include "NF/Editor/InspectorPanel.h"
#include "NF/Editor/HierarchyPanel.h"
#include "NF/Editor/ConsolePanel.h"
#include "NF/Editor/ContentBrowser.h"
#include "NF/Editor/ContentBrowserPanel.h"
#include "NF/Editor/PropertyEditor.h"
#include "NF/Editor/SettingsPanel.h"
#include "NF/Workspace/AIAssistantPanel.h"
#include "NF/Workspace/Notifications.h"
#include "NF/Workspace/NotificationSystem.h"
#include "NF/Workspace/NotificationWorkflow.h"

// ── V1 panel implementations ────────────────────────────────────
#include "NF/Editor/PropertyGridV1.h"
#include "NF/Editor/TreeViewV1.h"
#include "NF/Editor/TableViewV1.h"
#include "NF/Editor/ComponentInspectorV1.h"
#include "NF/Editor/DiagnosticPanelV1.h"
#include "NF/Editor/ProfilerViewV1.h"
#include "NF/Editor/MemoryTrackerV1.h"
#include "NF/Editor/MemoryProfilerPanel.h"
#include "NF/Editor/PipelineMonitorPanel.h"
#include "NF/Editor/RenderStatsPanel.h"
#include "NF/Editor/RenderSettingsPanel.h"
#include "NF/Editor/LODEditorPanel.h"
#include "NF/Editor/SettingsControlPanelV1.h"
#include "NF/Editor/OnlineServicesPanel.h"

// ── Shared contracts ────────────────────────────────────────────
#include "NF/Editor/ViewportHostContract.h"
#include "NF/Editor/GraphHostContract.h"
#include "NF/Editor/GraphHostContractV1.h"
#include "NF/Editor/ScrollVirtualization.h"
#include "NF/Editor/ScrollVirtualizerV1.h"
