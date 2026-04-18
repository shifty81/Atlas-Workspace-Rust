#pragma once
// NovaForge::NovaForgeAdapter — Project adapter for the NovaForge game.
//
// This adapter registers NovaForge-specific gameplay system panels with
// the Atlas Workspace. All NovaForge-specific authoring surfaces are
// exposed through this adapter rather than as standalone workspace editors.
//
// See Docs/Canon/06_WORKSPACE_VS_PROJECT_BOUNDARY.md

#include "NF/Workspace/IGameProjectAdapter.h"
#include "NovaForge/EditorAdapter/NovaForgeProjectBootstrap.h"
#include "NovaForge/EditorAdapter/Panels/EconomyPanel.h"
#include "NovaForge/EditorAdapter/Panels/InventoryRulesPanel.h"
#include "NovaForge/EditorAdapter/Panels/ShopPanel.h"
#include "NovaForge/EditorAdapter/Panels/MissionRulesPanel.h"
#include "NovaForge/EditorAdapter/Panels/ProgressionPanel.h"
#include "NovaForge/EditorAdapter/Panels/CharacterRulesPanel.h"
#include <filesystem>
#include <string>
#include <vector>

namespace NovaForge {

class NovaForgeAdapter final : public NF::IGameProjectAdapter {
public:
    /// Construct with the absolute path to the NovaForge project root directory.
    /// This is the directory that contains NovaForge.atlas / novaforge.project.json.
    explicit NovaForgeAdapter(std::string projectRoot)
        : m_projectRoot(std::move(projectRoot)) {
        // Pre-compute absolute content roots once so contentRoots() is allocation-free.
        // Accept both capitalized (canonical) and lowercase (sample/Linux) forms.
        namespace fs = std::filesystem;
        fs::path root(m_projectRoot);

        auto addIfExists = [&](const char* canonical, const char* fallback) {
            fs::path p = root / canonical;
            if (fs::exists(p) && fs::is_directory(p)) {
                m_contentRoots.push_back(p.string());
            } else {
                fs::path q = root / fallback;
                if (fs::exists(q) && fs::is_directory(q))
                    m_contentRoots.push_back(q.string());
            }
        };

        addIfExists("Content", "content");
        addIfExists("Data",    "data");
    }

    std::string projectId() const override { return "novaforge"; }
    std::string projectDisplayName() const override { return "NovaForge"; }
    std::string projectRoot() const override { return m_projectRoot; }

    bool initialize() override {
        // Run project bootstrap (validation only — does not block adapter init)
        NovaForgeProjectBootstrap bs(m_projectRoot, &m_assetCatalog);
        bs.run(m_bootstrapContract);

        // Register gameplay system panels
        m_panels = buildPanelDescriptors();
        return true;
    }

    void shutdown() override {
        m_panels.clear();
    }

    std::vector<NF::GameplaySystemPanelDescriptor> panelDescriptors() const override {
        return m_panels;
    }

    std::vector<std::string> contentRoots() const override {
        return m_contentRoots;
    }

    std::vector<std::string> customCommands() const override {
        return {
            "novaforge.build_game",
            "novaforge.build_server",
            "novaforge.launch_editor",
            "novaforge.validate_assets"
        };
    }

    [[nodiscard]] const NF::ProjectLoadContract& bootstrapContract() const { return m_bootstrapContract; }
    [[nodiscard]] const NF::AssetCatalog& assetCatalog() const { return m_assetCatalog; }

private:
    std::string m_projectRoot;
    std::vector<std::string> m_contentRoots;
    std::vector<NF::GameplaySystemPanelDescriptor> m_panels;
    NF::AssetCatalog m_assetCatalog;
    NF::ProjectLoadContract m_bootstrapContract;

    static std::vector<NF::GameplaySystemPanelDescriptor> buildPanelDescriptors() {
        std::vector<NF::GameplaySystemPanelDescriptor> panels;

        // Economy panel — game currency, pricing, economy balancing
        {
            NF::GameplaySystemPanelDescriptor d;
            d.id = "novaforge.economy";
            d.displayName = "Economy";
            d.hostToolId = "workspace.project_systems";
            d.category = "Gameplay";
            d.projectId = "novaforge";
            d.defaultVisible = true;
            d.enabled = true;
            d.createPanel = []() -> std::unique_ptr<NF::IEditorPanel> {
                return std::make_unique<EconomyPanel>();
            };
            panels.push_back(std::move(d));
        }

        // Inventory rules panel — item slots, storage, equipment rules
        {
            NF::GameplaySystemPanelDescriptor d;
            d.id = "novaforge.inventory_rules";
            d.displayName = "Inventory Rules";
            d.hostToolId = "workspace.project_systems";
            d.category = "Gameplay";
            d.projectId = "novaforge";
            d.defaultVisible = true;
            d.enabled = true;
            d.createPanel = []() -> std::unique_ptr<NF::IEditorPanel> {
                return std::make_unique<InventoryRulesPanel>();
            };
            panels.push_back(std::move(d));
        }

        // Shop panel — item shop configuration
        {
            NF::GameplaySystemPanelDescriptor d;
            d.id = "novaforge.shop";
            d.displayName = "Shop";
            d.hostToolId = "workspace.project_systems";
            d.category = "Gameplay";
            d.projectId = "novaforge";
            d.defaultVisible = false;
            d.enabled = true;
            d.createPanel = []() -> std::unique_ptr<NF::IEditorPanel> {
                return std::make_unique<ShopPanel>();
            };
            panels.push_back(std::move(d));
        }

        // Mission rules panel — quest, mission, objective rules
        {
            NF::GameplaySystemPanelDescriptor d;
            d.id = "novaforge.mission_rules";
            d.displayName = "Mission Rules";
            d.hostToolId = "workspace.project_systems";
            d.category = "Gameplay";
            d.projectId = "novaforge";
            d.defaultVisible = true;
            d.enabled = true;
            d.createPanel = []() -> std::unique_ptr<NF::IEditorPanel> {
                return std::make_unique<MissionRulesPanel>();
            };
            panels.push_back(std::move(d));
        }

        // Progression panel — XP, leveling, achievements, unlock paths
        {
            NF::GameplaySystemPanelDescriptor d;
            d.id = "novaforge.progression";
            d.displayName = "Progression";
            d.hostToolId = "workspace.project_systems";
            d.category = "Gameplay";
            d.projectId = "novaforge";
            d.defaultVisible = true;
            d.enabled = true;
            d.createPanel = []() -> std::unique_ptr<NF::IEditorPanel> {
                return std::make_unique<ProgressionPanel>();
            };
            panels.push_back(std::move(d));
        }

        // Character rules panel — character creation, appearance, classes
        {
            NF::GameplaySystemPanelDescriptor d;
            d.id = "novaforge.character_rules";
            d.displayName = "Character Rules";
            d.hostToolId = "workspace.project_systems";
            d.category = "Gameplay";
            d.projectId = "novaforge";
            d.defaultVisible = false;
            d.enabled = true;
            d.createPanel = []() -> std::unique_ptr<NF::IEditorPanel> {
                return std::make_unique<CharacterRulesPanel>();
            };
            panels.push_back(std::move(d));
        }

        return panels;
    }
};

} // namespace NovaForge
