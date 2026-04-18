#pragma once
// NF::Editor — CoreToolRoster: registers the canonical primary tool set.
//
// This header provides registerCoreTools() which populates a WorkspaceShell
// with the ~8 primary editor tools. It lives in the Editor module (not
// Workspace) because the Workspace module is tool-agnostic — it doesn't
// know about specific tool implementations.
//
// Usage:
//   WorkspaceShell shell;
//   NF::registerCoreTools(shell);  // register all primary tools
//   shell.initialize();            // boot the workspace
//
// See Docs/Canon/05_EDITOR_STRATEGY.md for the locked tool roster.

#include "NF/Editor/SceneEditorTool.h"
#include "NF/Editor/AssetEditorTool.h"
#include "NF/Editor/MaterialEditorTool.h"
#include "NF/Editor/AnimationEditorTool.h"
#include "NF/Editor/DataEditorTool.h"
#include "NF/Editor/VisualLogicEditorTool.h"
#include "NF/Editor/BuildTool.h"
#include "NF/Editor/AtlasAITool.h"
#include "NF/Editor/IDETool.h"
#include "NF/Workspace/WorkspaceShell.h"
#include <memory>

namespace NF {

/// Register the canonical primary tool roster into a WorkspaceShell.
/// Tools are registered as factories in Atlas-bar display order.
/// Call this before shell.initialize().
inline void registerCoreTools(WorkspaceShell& shell) {
    shell.registerToolFactory([] { return std::make_unique<SceneEditorTool>(); });
    shell.registerToolFactory([] { return std::make_unique<AssetEditorTool>(); });
    shell.registerToolFactory([] { return std::make_unique<MaterialEditorTool>(); });
    shell.registerToolFactory([] { return std::make_unique<AnimationEditorTool>(); });
    shell.registerToolFactory([] { return std::make_unique<DataEditorTool>(); });
    shell.registerToolFactory([] { return std::make_unique<VisualLogicEditorTool>(); });
    shell.registerToolFactory([] { return std::make_unique<BuildTool>(); });
    shell.registerToolFactory([] { return std::make_unique<AtlasAITool>(); });
    shell.registerToolFactory([] { return std::make_unique<IDETool>(); });
}

} // namespace NF
