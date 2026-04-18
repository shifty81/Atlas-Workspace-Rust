#pragma once
// NF::Editor — MaterialEditorTool: primary material/shader authoring tool.
//
// Implements NF::IHostedTool for the Material Editor, one of the ~10
// primary tools in the canonical workspace roster.
//
// The Material Editor hosts:
//   Shared panels: viewport_material, inspector, asset_preview, console
//   Commands:      material.create, material.save, material.set_shader,
//                  material.add_texture, material.duplicate, material.open
//
// See Docs/Canon/05_EDITOR_STRATEGY.md for the locked tool roster.
// See Docs/Roadmap/04_EDITOR_CONSOLIDATION.md for Phase 3 status.

#include "NF/Workspace/IHostedTool.h"
#include "NF/Workspace/ToolViewRenderContext.h"
#include "NF/Workspace/IViewportSceneProvider.h"
#include <string>

namespace NF {

// ── Material editing mode ─────────────────────────────────────────

enum class MaterialEditMode : uint8_t {
    Properties, // flat property sheet for parameters + gradient widget
    NodeGraph,  // typed shader-graph with port connections (MaterialNodeEditorV1)
    Preview,    // fullscreen material preview
    PostProcess,// post-process effect stack (PostProcessEditorV1)
};

inline const char* materialEditModeName(MaterialEditMode m) {
    switch (m) {
        case MaterialEditMode::Properties:  return "Properties";
        case MaterialEditMode::NodeGraph:   return "NodeGraph";
        case MaterialEditMode::Preview:     return "Preview";
        case MaterialEditMode::PostProcess: return "PostProcess";
    }
    return "Unknown";
}

// ── Material Editor statistics ────────────────────────────────────

struct MaterialEditorStats {
    uint32_t nodeCount        = 0; // shader-graph node count
    uint32_t textureSlotCount = 0; // bound texture slots
    bool     isDirty          = false;
};

// ── MaterialEditorTool ────────────────────────────────────────────

class MaterialEditorTool final : public IHostedTool, public IViewportSceneProvider {
public:
    static constexpr const char* kToolId = "workspace.material_editor";

    MaterialEditorTool();
    ~MaterialEditorTool() override = default;

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

    // ── Material Editor interface ─────────────────────────────────

    [[nodiscard]] MaterialEditMode          editMode() const { return m_editMode; }
    void                                    setEditMode(MaterialEditMode mode);

    [[nodiscard]] const MaterialEditorStats& stats()   const { return m_stats; }
    [[nodiscard]] bool                       isDirty() const { return m_stats.isDirty; }
    void                                     markDirty();
    void                                     clearDirty();

    void setNodeCount(uint32_t count);
    void setTextureSlotCount(uint32_t count);

    [[nodiscard]] const std::string& openAssetPath() const { return m_openAssetPath; }
    void                             setOpenAssetPath(const std::string& path);

    // ── IViewportSceneProvider ────────────────────────────────────
    ViewportSceneState provideScene(ViewportHandle handle,
                                    const ViewportSlot& slot) override;

    /// Attach a material preview provider (Phase D.4 wiring).
    /// NovaForgeMaterialPreview implements IViewportSceneProvider — pass it directly.
    /// Pass nullptr to detach and revert to stub state.
    void attachMaterialPreviewProvider(IViewportSceneProvider* provider) {
        m_materialPreviewProvider = provider;
    }

    [[nodiscard]] IViewportSceneProvider* materialPreviewProvider() const {
        return m_materialPreviewProvider;
    }

    // ── Render contract ───────────────────────────────────────────
    // Renders: Material Graph | Viewport Preview | Properties — three-column layout.
    void renderToolView(const ToolViewRenderContext& ctx) const override;
    void syncPanels(WorkspacePanelHost& host) const override;

private:
    HostedToolDescriptor  m_descriptor;
    HostedToolState       m_state        = HostedToolState::Unloaded;
    mutable MaterialEditMode m_editMode  = MaterialEditMode::Properties;
    MaterialEditorStats   m_stats;
    std::string           m_openAssetPath;
    std::string           m_activeProjectId;

    // Optional material preview scene provider (Phase D.4)
    IViewportSceneProvider* m_materialPreviewProvider = nullptr;

    // ── Mutable per-view UI state (safe from const renderToolView) ─
    mutable int m_viewSelectedNode    = -1;
    mutable int m_viewSelectedEffect  = -1; // PostProcess mode

    void buildDescriptor();
};

} // namespace NF
