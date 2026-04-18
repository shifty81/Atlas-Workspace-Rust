#pragma once
// NF::ProjectSystemsTool — Host tool for project-specific gameplay panels.
//
// This tool provides a generic host surface where project adapters can
// inject gameplay system panels (economy, inventory, progression, etc.).
// It is the "workspace.project_systems" tool in the host-tool ID scheme.
//
// The workspace asks the loaded project adapter for panel descriptors,
// and this tool hosts them as tabs/sub-panels within a unified surface.
// Panel instances are created lazily via the descriptor's createPanel factory
// and cached for the lifetime of the loaded project.
//
// See Docs/Roadmap/04_EDITOR_CONSOLIDATION.md for consolidation plan.
// See Docs/Canon/06_WORKSPACE_VS_PROJECT_BOUNDARY.md for boundary rules.

#include "NF/Workspace/IGameProjectAdapter.h"
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

namespace NF {

// ── Host tool IDs ───────────────────────────────────────────────
// Stable identifiers for workspace host tools.
// Project adapters reference these when specifying panel placement.

namespace HostToolId {
    inline constexpr const char* ProjectBrowser     = "workspace.project_browser";
    inline constexpr const char* SceneEditor        = "workspace.scene_editor";
    inline constexpr const char* AssetEditor         = "workspace.asset_editor";
    inline constexpr const char* MaterialEditor      = "workspace.material_editor";
    inline constexpr const char* VisualLogicEditor   = "workspace.visual_logic_editor";
    inline constexpr const char* UIEditor            = "workspace.ui_editor";
    inline constexpr const char* AnimationEditor     = "workspace.animation_editor";
    inline constexpr const char* DataEditor          = "workspace.data_editor";
    inline constexpr const char* BuildTool           = "workspace.build_tool";
    inline constexpr const char* ProjectSystems      = "workspace.project_systems";
    inline constexpr const char* AtlasAI             = "workspace.atlasai";
    inline constexpr const char* IDE                 = "workspace.ide";
} // namespace HostToolId

// ── ProjectSystemsTool ──────────────────────────────────────────
// The host tool that receives and manages project-specific panels.

class ProjectSystemsTool {
public:
    // ── Descriptor management ─────────────────────────────────────
    void loadFromAdapter(const IGameProjectAdapter& adapter) {
        m_panels = adapter.panelDescriptors();
        m_livePanels.clear();       // discard any previously-instantiated panels
        m_currentProjectRoot.clear(); // stale root is invalid until notifyProjectLoaded
    }

    const std::vector<GameplaySystemPanelDescriptor>& panels() const {
        return m_panels;
    }

    size_t panelCount() const { return m_panels.size(); }

    const GameplaySystemPanelDescriptor* findPanel(const std::string& id) const {
        for (const auto& p : m_panels) {
            if (p.id == id) return &p;
        }
        return nullptr;
    }

    // ── Live panel access ─────────────────────────────────────────
    // Returns the live IEditorPanel instance for the given panelId, creating
    // it on first access via the descriptor's createPanel factory.
    // If a project is currently loaded the panel immediately receives
    // onProjectLoaded() so lazily-created panels have real data from the start.
    // Returns nullptr if the panel is not registered or has no factory.
    IEditorPanel* getOrCreatePanel(const std::string& panelId) {
        // Return cached instance if available.
        auto it = m_livePanels.find(panelId);
        if (it != m_livePanels.end()) return it->second.get();

        // Find the descriptor and invoke the factory.
        const GameplaySystemPanelDescriptor* desc = findPanel(panelId);
        if (!desc || !desc->createPanel) return nullptr;

        auto panel = desc->createPanel();
        if (!panel) return nullptr;

        IEditorPanel* raw = panel.get();
        m_livePanels.emplace(panelId, std::move(panel));

        // If a project root is already set, notify the newly-created panel
        // immediately so it can load real project data without waiting for the
        // next explicit notifyProjectLoaded() broadcast.
        if (!m_currentProjectRoot.empty())
            raw->onProjectLoaded(m_currentProjectRoot);

        return raw;
    }

    // Const version — returns nullptr if not yet instantiated.
    [[nodiscard]] const IEditorPanel* findLivePanel(const std::string& panelId) const {
        auto it = m_livePanels.find(panelId);
        return (it != m_livePanels.end()) ? it->second.get() : nullptr;
    }

    [[nodiscard]] size_t livePanelCount() const { return m_livePanels.size(); }

    // ── Project lifecycle ─────────────────────────────────────────
    // Notify all live panels when a project is loaded or unloaded.
    // Also pre-instantiate all panels with factories so they receive the
    // onProjectLoaded() call immediately rather than waiting for lazy
    // creation when the user navigates to them.
    void notifyProjectLoaded(const std::string& projectRoot) {
        // Pre-instantiate all enabled panels that have factories.
        // m_currentProjectRoot is NOT yet set so getOrCreatePanel() will
        // NOT call onProjectLoaded() — we do a single broadcast below.
        for (const auto& desc : m_panels) {
            if (desc.createPanel && desc.enabled) {
                (void)getOrCreatePanel(desc.id);
            }
        }

        // Now set the project root and broadcast to all live panels.
        m_currentProjectRoot = projectRoot;
        for (auto& [id, panel] : m_livePanels) {
            if (panel) panel->onProjectLoaded(projectRoot);
        }
    }

    void notifyProjectUnloaded() {
        m_currentProjectRoot.clear();
        for (auto& [id, panel] : m_livePanels) {
            if (panel) panel->onProjectUnloaded();
        }
    }

    // ── Cleanup ───────────────────────────────────────────────────
    // Destroy all live panels and clear descriptors.
    void reset() {
        m_livePanels.clear();
        m_panels.clear();
        m_currentProjectRoot.clear();
    }

    // ── Filtering helpers ─────────────────────────────────────────
    // Filter panels by category
    std::vector<const GameplaySystemPanelDescriptor*> panelsByCategory(
        const std::string& category) const
    {
        std::vector<const GameplaySystemPanelDescriptor*> result;
        for (const auto& p : m_panels) {
            if (p.category == category && p.enabled) {
                result.push_back(&p);
            }
        }
        return result;
    }

    // Filter visible panels
    std::vector<const GameplaySystemPanelDescriptor*> visiblePanels() const {
        std::vector<const GameplaySystemPanelDescriptor*> result;
        for (const auto& p : m_panels) {
            if (p.defaultVisible && p.enabled) {
                result.push_back(&p);
            }
        }
        return result;
    }

private:
    std::vector<GameplaySystemPanelDescriptor> m_panels;

    // Live panel instances keyed by panel ID.
    // Created lazily on first getOrCreatePanel() call.
    std::unordered_map<std::string, std::unique_ptr<IEditorPanel>> m_livePanels;

    // Project root of the currently loaded project (empty when no project).
    // Stored so lazily-created panels receive onProjectLoaded() immediately.
    std::string m_currentProjectRoot;
};

} // namespace NF
