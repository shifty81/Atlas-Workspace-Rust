#pragma once
// NF::Editor — IHostedTool: abstract interface for primary workspace-hosted tools.
//
// Every primary tool in the workspace implements this contract.
// The workspace shell uses ToolRegistry to manage these tools.
// Tools interact with shared panels and services through the shell — they do not
// own duplicated infrastructure.
//
// See Docs/Canon/05_EDITOR_STRATEGY.md for the locked ~10-tool roster.
// See ProjectSystemsTool.h for the host-tool ID namespace.

#include "NF/Input/Input.h"
#include <cstdint>
#include <string>
#include <vector>

namespace NF { struct ToolViewRenderContext; } // forward — defined in ToolViewRenderContext.h
namespace NF { class WorkspacePanelHost; }   // forward — defined in WorkspacePanelHost.h

namespace NF {

// ── Hosted Tool Category ──────────────────────────────────────────
// Broad classification for UI grouping and Atlas-bar ordering.

enum class HostedToolCategory : uint8_t {
    ProjectBrowser,      // project browser / asset management
    SceneEditing,        // scene / world / level editing
    AssetAuthoring,      // material, shader, texture authoring
    LogicAuthoring,      // visual logic / blueprint / state-machine
    AnimationAuthoring,  // animation, sequencer, cinematics
    DataEditing,         // data tables, configs, localization
    BuildPackaging,      // build / packaging / deploy
    AIAssistant,         // AtlasAI / diagnostics / codex
    ProjectSystems,      // project-specific extensions via adapter
    Utility,             // one-off utilities (UI editor, profiler, etc.)
};

inline const char* hostedToolCategoryName(HostedToolCategory c) {
    switch (c) {
        case HostedToolCategory::ProjectBrowser:     return "ProjectBrowser";
        case HostedToolCategory::SceneEditing:       return "SceneEditing";
        case HostedToolCategory::AssetAuthoring:      return "AssetAuthoring";
        case HostedToolCategory::LogicAuthoring:      return "LogicAuthoring";
        case HostedToolCategory::AnimationAuthoring:  return "AnimationAuthoring";
        case HostedToolCategory::DataEditing:         return "DataEditing";
        case HostedToolCategory::BuildPackaging:      return "BuildPackaging";
        case HostedToolCategory::AIAssistant:         return "AIAssistant";
        case HostedToolCategory::ProjectSystems:      return "ProjectSystems";
        case HostedToolCategory::Utility:             return "Utility";
    }
    return "Unknown";
}

// ── Hosted Tool Lifecycle State ───────────────────────────────────

enum class HostedToolState : uint8_t {
    Unloaded,   // not yet initialized
    Ready,      // initialized, available for activation
    Active,     // currently the focused tool
    Suspended,  // was active, now backgrounded
};

inline const char* hostedToolStateName(HostedToolState s) {
    switch (s) {
        case HostedToolState::Unloaded:  return "Unloaded";
        case HostedToolState::Ready:     return "Ready";
        case HostedToolState::Active:    return "Active";
        case HostedToolState::Suspended: return "Suspended";
    }
    return "Unknown";
}

// ── Hosted Tool Descriptor ────────────────────────────────────────
// Static description of a tool — its identity, dock preferences,
// supported shared panels, and command contributions.

struct HostedToolDescriptor {
    std::string         toolId;          // e.g. "workspace.scene_editor"
    std::string         displayName;     // e.g. "Scene Editor"
    HostedToolCategory  category  = HostedToolCategory::Utility;
    bool                isPrimary = true; // primary tools appear on the Atlas bar

    // Shared panels this tool uses (by panel-id)
    std::vector<std::string> supportedPanels;

    // Commands contributed by this tool
    std::vector<std::string> commands;

    // Does this tool accept project-adapter extensions?
    bool acceptsProjectExtensions = false;

    [[nodiscard]] bool isValid() const {
        return !toolId.empty() && !displayName.empty();
    }
};

// ── IHostedTool ───────────────────────────────────────────────────
// Abstract interface every hosted tool implements.

class IHostedTool {
public:
    virtual ~IHostedTool() = default;

    // ── Identity ──────────────────────────────────────────────────
    [[nodiscard]] virtual const HostedToolDescriptor& descriptor() const = 0;
    [[nodiscard]] virtual const std::string& toolId() const = 0;

    // ── Lifecycle ─────────────────────────────────────────────────
    virtual bool initialize() = 0;   // transition Unloaded → Ready
    virtual void shutdown()   = 0;   // transition * → Unloaded
    virtual void activate()   = 0;   // transition Ready/Suspended → Active
    virtual void suspend()    = 0;   // transition Active → Suspended
    virtual void update(float dt) = 0;

    [[nodiscard]] virtual HostedToolState state() const = 0;

    // ── Optional hooks ────────────────────────────────────────────
    // Subclasses may override to handle project adapter load/unload.
    virtual void onProjectLoaded(const std::string& /*projectId*/) {}
    virtual void onProjectUnloaded() {}

    // ── Optional input access ─────────────────────────────────────
    // Called by the workspace shell when a tool becomes active and an InputSystem
    // is available.  The pointer is valid until onDetachInput() is called.
    // Tools that need per-frame camera / gizmo input override these two hooks
    // and cache the pointer.  Do NOT call delete on the pointer.
    virtual void onAttachInput(const InputSystem* /*input*/) {}
    virtual void onDetachInput() {}

    // Called by the InputRouter / workspace shell when focus changes so that
    // tools can update cursor, highlight, or mode display.  Default is a no-op.
    virtual void onInputFocusChanged(bool /*focused*/) {}

    // ── Render contract ───────────────────────────────────────────
    // Called by WorkspaceRenderer::renderActiveToolView() each paint frame when
    // this tool is the active tool.  The tool is responsible for drawing its
    // own panel layout (hierarchy, viewport, inspector, log, etc.) into the
    // content area described by ctx.  The default no-op keeps non-UI tools valid.
    virtual void renderToolView(const ToolViewRenderContext& /*ctx*/) const {}

    // ── Panel sync contract ───────────────────────────────────────
    // Called by the main render loop each frame after WorkspaceRenderer::render()
    // when this tool is active.  The tool pushes its current data model into the
    // AtlasUI panel host so that WorkspacePanelHost::renderPanels() shows live
    // content rather than static placeholders.  Default is a no-op — tools that
    // do not use WorkspacePanelHost need not override this.
    virtual void syncPanels(WorkspacePanelHost& /*host*/) const {}
};

} // namespace NF
