#pragma once
// NF::Editor — VisualLogicEditorTool: primary visual scripting / blueprint tool.
//
// Implements NF::IHostedTool for the Visual Logic Editor, one of the ~10
// primary tools in the canonical workspace roster.
//
// The Visual Logic Editor hosts:
//   Shared panels: node_graph, inspector, console
//   Commands:      vl.add_node, vl.delete_node, vl.connect_pins,
//                  vl.disconnect_pins, vl.compile, vl.save
//
// See Docs/Canon/05_EDITOR_STRATEGY.md for the locked tool roster.
// See Docs/Roadmap/04_EDITOR_CONSOLIDATION.md for Phase 3 status.

#include "NF/Workspace/IHostedTool.h"
#include "NF/Workspace/ToolViewRenderContext.h"
#include <string>

namespace NF {

// ── Visual Logic editing mode ─────────────────────────────────────

enum class VisualLogicMode : uint8_t {
    Graph,        // script node-graph authoring (ScriptGraphEditorV1)
    Dialogue,     // branching dialogue tree (DialogueTreeEditorV1)
    StateMachine, // finite-state machine (StateGraphV1)
    BehaviorTree, // AI behavior tree + blackboard (AIBehaviorTreeEditorV1)
    VFX,          // VFX node graph (VFXGraphEditorV1)
    Debug,        // live execution debugger / breakpoints
    Diff,         // graph diff vs. last compile
};

inline const char* visualLogicModeName(VisualLogicMode m) {
    switch (m) {
        case VisualLogicMode::Graph:        return "Graph";
        case VisualLogicMode::Dialogue:     return "Dialogue";
        case VisualLogicMode::StateMachine: return "StateMachine";
        case VisualLogicMode::BehaviorTree: return "BehaviorTree";
        case VisualLogicMode::VFX:          return "VFX";
        case VisualLogicMode::Debug:        return "Debug";
        case VisualLogicMode::Diff:         return "Diff";
    }
    return "Unknown";
}

// ── Visual Logic Editor statistics ───────────────────────────────

struct VisualLogicEditorStats {
    uint32_t nodeCount       = 0; // nodes in the open graph
    uint32_t connectionCount = 0; // pin connections
    uint32_t errorCount      = 0; // compile errors
    bool     isDirty         = false;
    bool     isCompiling     = false;
};

// ── VisualLogicEditorTool ─────────────────────────────────────────

class VisualLogicEditorTool final : public IHostedTool {
public:
    static constexpr const char* kToolId = "workspace.visual_logic_editor";

    VisualLogicEditorTool();
    ~VisualLogicEditorTool() override = default;

    // ── IHostedTool identity ──────────────────────────────────────
    [[nodiscard]] const HostedToolDescriptor& descriptor() const override { return m_descriptor; }
    [[nodiscard]] const std::string& toolId()              const override { return m_descriptor.toolId; }

    // ── IHostedTool lifecycle ─────────────────────────────────────
    bool initialize() override;
    void shutdown()   override;
    void activate()   override;
    void suspend()    override;
    void update(float dt) override;

    [[nodiscard]] HostedToolState state() const override { return m_state; }

    // ── Project adapter hooks ─────────────────────────────────────
    void onProjectLoaded(const std::string& projectId) override;
    void onProjectUnloaded() override;

    // ── Visual Logic Editor interface ─────────────────────────────

    [[nodiscard]] VisualLogicMode                editMode() const { return m_editMode; }
    void                                         setEditMode(VisualLogicMode mode);

    [[nodiscard]] const VisualLogicEditorStats&  stats()       const { return m_stats; }
    [[nodiscard]] bool                           isDirty()     const { return m_stats.isDirty; }
    [[nodiscard]] bool                           isCompiling() const { return m_stats.isCompiling; }
    [[nodiscard]] uint32_t                       errorCount()  const { return m_stats.errorCount; }

    void markDirty();
    void clearDirty();

    void setNodeCount(uint32_t count);
    void setConnectionCount(uint32_t count);
    void setCompiling(bool compiling);
    void setErrorCount(uint32_t count);

    // ── Render contract ───────────────────────────────────────────
    // Renders: Node List | Graph Canvas | Properties — three-column layout.
    void renderToolView(const ToolViewRenderContext& ctx) const override;
    void syncPanels(WorkspacePanelHost& host) const override;

private:
    HostedToolDescriptor    m_descriptor;
    HostedToolState         m_state    = HostedToolState::Unloaded;
    mutable VisualLogicMode m_editMode = VisualLogicMode::Graph;
    VisualLogicEditorStats  m_stats;
    std::string             m_activeProjectId;

    // ── Mutable per-view UI state (safe from const renderToolView) ─
    mutable int m_viewSelectedNode    = -1; // Graph mode
    mutable int m_viewSelectedDlgNode = -1; // Dialogue mode
    mutable int m_viewSelectedState   = -1; // StateMachine mode
    mutable int m_viewSelectedBTNode  = -1; // BehaviorTree mode
    mutable int m_viewSelectedVFXNode = -1; // VFX mode

    void buildDescriptor();
};

} // namespace NF
