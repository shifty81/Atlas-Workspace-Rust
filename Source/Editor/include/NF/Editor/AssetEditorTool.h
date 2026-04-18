#pragma once
// NF::Editor — AssetEditorTool: primary asset browsing and management tool.
//
// Implements NF::IHostedTool for the Asset Editor, one of the ~10
// primary tools in the canonical workspace roster.
//
// The Asset Editor hosts:
//   Shared panels: content_browser, inspector, asset_preview, console
//   Commands:      asset.import, asset.delete, asset.rename,
//                  asset.duplicate, asset.refresh, asset.open,
//                  asset.create_folder
//
// See Docs/Canon/05_EDITOR_STRATEGY.md for the locked tool roster.
// See Docs/Roadmap/04_EDITOR_CONSOLIDATION.md for Phase 3 status.

#include "NF/Workspace/IHostedTool.h"
#include "NF/Workspace/ToolViewRenderContext.h"
#include "NF/Workspace/IViewportSceneProvider.h"
#include <string>

namespace NF {

// ── Asset filter mode ─────────────────────────────────────────────

enum class AssetFilterMode : uint8_t {
    All,        // show all asset types
    Textures,   // filter to texture assets
    Materials,  // filter to material assets
    Meshes,     // filter to static/skeletal mesh assets
    Audio,      // filter to audio clip/bank assets
    Scripts,    // filter to script assets
    Prefabs,    // filter to prefab/archetype assets
    Data,       // filter to game data / config assets (.json, .yaml, etc.)
};

inline const char* assetFilterModeName(AssetFilterMode m) {
    switch (m) {
        case AssetFilterMode::All:       return "All";
        case AssetFilterMode::Textures:  return "Textures";
        case AssetFilterMode::Materials: return "Materials";
        case AssetFilterMode::Meshes:    return "Meshes";
        case AssetFilterMode::Audio:     return "Audio";
        case AssetFilterMode::Scripts:   return "Scripts";
        case AssetFilterMode::Prefabs:   return "Prefabs";
        case AssetFilterMode::Data:      return "Data";
    }
    return "Unknown";
}

// ── Asset Editor statistics ───────────────────────────────────────

struct AssetEditorStats {
    uint32_t totalAssetCount    = 0; // total assets in content browser
    uint32_t selectionCount     = 0; // currently selected assets
    uint32_t filteredAssetCount = 0; // assets visible after filter/search
    bool     isDirty            = false; // unsaved import/rename pending
};

// ── AssetEditorTool ───────────────────────────────────────────────

class AssetEditorTool final : public IHostedTool, public IViewportSceneProvider {
public:
    static constexpr const char* kToolId = "workspace.asset_editor";

    AssetEditorTool();
    ~AssetEditorTool() override = default;

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

    // ── Asset Editor interface ────────────────────────────────────

    [[nodiscard]] AssetFilterMode       filterMode()  const { return m_filterMode; }
    void                                setFilterMode(AssetFilterMode mode);

    [[nodiscard]] const std::string&    searchQuery() const { return m_searchQuery; }
    void                                setSearchQuery(const std::string& query);

    [[nodiscard]] const AssetEditorStats& stats()    const { return m_stats; }
    [[nodiscard]] bool                    isDirty()  const { return m_stats.isDirty; }
    void                                  markDirty();
    void                                  clearDirty();

    // Asset counts (updated when content browser refreshes)
    [[nodiscard]] uint32_t selectionCount()     const { return m_stats.selectionCount; }
    [[nodiscard]] uint32_t totalAssetCount()    const { return m_stats.totalAssetCount; }
    [[nodiscard]] uint32_t filteredAssetCount() const { return m_stats.filteredAssetCount; }

    void setSelectionCount(uint32_t count);
    void setTotalAssetCount(uint32_t count);
    void setFilteredAssetCount(uint32_t count);

    // ── IViewportSceneProvider ────────────────────────────────────
    ViewportSceneState provideScene(ViewportHandle handle,
                                    const ViewportSlot& slot) override;

    /// Attach a scene provider for the asset preview viewport (Phase D.3 wiring).
    /// NovaForgeAssetPreview implements IViewportSceneProvider — pass it directly.
    /// Pass nullptr to detach and revert to stub state.
    void attachAssetPreviewProvider(IViewportSceneProvider* provider) {
        m_assetPreviewProvider = provider;
    }

    [[nodiscard]] IViewportSceneProvider* assetPreviewProvider() const {
        return m_assetPreviewProvider;
    }

    // ── Render contract ───────────────────────────────────────────
    // Renders: Content Browser (wide) | Preview | Inspector — three-column layout.
    void renderToolView(const ToolViewRenderContext& ctx) const override;
    void syncPanels(WorkspacePanelHost& host) const override;

private:
    HostedToolDescriptor m_descriptor;
    HostedToolState      m_state      = HostedToolState::Unloaded;
    mutable AssetFilterMode m_filterMode = AssetFilterMode::All;
    AssetEditorStats     m_stats;
    std::string          m_searchQuery;
    std::string          m_activeProjectId;

    void buildDescriptor();

    // Optional asset preview scene provider (Phase D.3)
    IViewportSceneProvider* m_assetPreviewProvider = nullptr;

    // ── Mutable per-view UI state (safe from const renderToolView) ─
    // Selected asset tile index in content browser, -1 = none.
    mutable int m_viewSelectedTile = -1;
};

} // namespace NF
