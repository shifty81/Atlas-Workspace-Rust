// Phase 70 — NovaForge Panel Factories (Audit Patch 6)
//
// Tests verify that:
//  1. IEditorPanel is a proper interface with panelId(), panelTitle(), isReady()
//  2. Each NovaForge gameplay panel (Economy, InventoryRules, Shop, MissionRules,
//     Progression, CharacterRules) correctly implements IEditorPanel
//  3. Panel lifecycle: onProjectLoaded() -> isReady()=true, onProjectUnloaded() -> isReady()=false
//  4. NovaForgeAdapter::panelDescriptors() provides createPanel factories for all 6 panels
//  5. createPanel factories produce valid IEditorPanel instances
//  6. ProjectSystemsTool caches live panel instances (getOrCreatePanel)
//  7. ProjectSystemsTool::notifyProjectLoaded/Unloaded propagates to live panels
//  8. ProjectSystemsTool::reset() destroys all cached panels
//  9. ProjectSystemsTool::findLivePanel() returns null before instantiation

#include "NovaForge/EditorAdapter/NovaForgeAdapter.h"
#include "NovaForge/EditorAdapter/Panels/EconomyPanel.h"
#include "NovaForge/EditorAdapter/Panels/InventoryRulesPanel.h"
#include "NovaForge/EditorAdapter/Panels/ShopPanel.h"
#include "NovaForge/EditorAdapter/Panels/MissionRulesPanel.h"
#include "NovaForge/EditorAdapter/Panels/ProgressionPanel.h"
#include "NovaForge/EditorAdapter/Panels/CharacterRulesPanel.h"
#include "NF/Workspace/ProjectSystemsTool.h"
#include "NF/Workspace/IGameProjectAdapter.h"

#include <catch2/catch_test_macros.hpp>
#include <memory>
#include <string>

// ── IEditorPanel interface ────────────────────────────────────────

TEST_CASE("IEditorPanel panelId and title are non-empty on construction", "[phase70]") {
    NovaForge::EconomyPanel ep;
    REQUIRE_FALSE(ep.panelId().empty());
    REQUIRE_FALSE(ep.panelTitle().empty());

    NovaForge::InventoryRulesPanel ip;
    REQUIRE_FALSE(ip.panelId().empty());
    REQUIRE_FALSE(ip.panelTitle().empty());

    NovaForge::ShopPanel sp;
    REQUIRE_FALSE(sp.panelId().empty());
    REQUIRE_FALSE(sp.panelTitle().empty());

    NovaForge::MissionRulesPanel mp;
    REQUIRE_FALSE(mp.panelId().empty());
    REQUIRE_FALSE(mp.panelTitle().empty());

    NovaForge::ProgressionPanel pp;
    REQUIRE_FALSE(pp.panelId().empty());
    REQUIRE_FALSE(pp.panelTitle().empty());

    NovaForge::CharacterRulesPanel cp;
    REQUIRE_FALSE(cp.panelId().empty());
    REQUIRE_FALSE(cp.panelTitle().empty());
}

TEST_CASE("IEditorPanel panel IDs match NovaForgeAdapter descriptor IDs", "[phase70]") {
    REQUIRE(std::string(NovaForge::EconomyPanel::kPanelId)        == "novaforge.economy");
    REQUIRE(std::string(NovaForge::InventoryRulesPanel::kPanelId) == "novaforge.inventory_rules");
    REQUIRE(std::string(NovaForge::ShopPanel::kPanelId)           == "novaforge.shop");
    REQUIRE(std::string(NovaForge::MissionRulesPanel::kPanelId)   == "novaforge.mission_rules");
    REQUIRE(std::string(NovaForge::ProgressionPanel::kPanelId)    == "novaforge.progression");
    REQUIRE(std::string(NovaForge::CharacterRulesPanel::kPanelId) == "novaforge.character_rules");
}

// ── Panel lifecycle ───────────────────────────────────────────────

TEST_CASE("EconomyPanel lifecycle: not ready before onProjectLoaded", "[phase70]") {
    NovaForge::EconomyPanel panel;
    REQUIRE_FALSE(panel.isReady());
}

TEST_CASE("EconomyPanel lifecycle: ready after onProjectLoaded", "[phase70]") {
    NovaForge::EconomyPanel panel;
    panel.onProjectLoaded("/some/project/root");
    REQUIRE(panel.isReady());
    REQUIRE(panel.projectRoot() == "/some/project/root");
}

TEST_CASE("EconomyPanel lifecycle: not ready after onProjectUnloaded", "[phase70]") {
    NovaForge::EconomyPanel panel;
    panel.onProjectLoaded("/proj");
    REQUIRE(panel.isReady());
    panel.onProjectUnloaded();
    REQUIRE_FALSE(panel.isReady());
    REQUIRE(panel.projectRoot().empty());
}

TEST_CASE("InventoryRulesPanel lifecycle: load/unload", "[phase70]") {
    NovaForge::InventoryRulesPanel panel;
    REQUIRE_FALSE(panel.isReady());
    panel.onProjectLoaded("/project");
    REQUIRE(panel.isReady());
    panel.onProjectUnloaded();
    REQUIRE_FALSE(panel.isReady());
}

TEST_CASE("ShopPanel lifecycle: load/unload", "[phase70]") {
    NovaForge::ShopPanel panel;
    REQUIRE_FALSE(panel.isReady());
    panel.onProjectLoaded("/project");
    REQUIRE(panel.isReady());
    panel.onProjectUnloaded();
    REQUIRE_FALSE(panel.isReady());
}

TEST_CASE("MissionRulesPanel lifecycle: load/unload", "[phase70]") {
    NovaForge::MissionRulesPanel panel;
    REQUIRE_FALSE(panel.isReady());
    panel.onProjectLoaded("/project");
    REQUIRE(panel.isReady());
    panel.onProjectUnloaded();
    REQUIRE_FALSE(panel.isReady());
}

TEST_CASE("ProgressionPanel lifecycle: load/unload", "[phase70]") {
    NovaForge::ProgressionPanel panel;
    REQUIRE_FALSE(panel.isReady());
    panel.onProjectLoaded("/project");
    REQUIRE(panel.isReady());
    panel.onProjectUnloaded();
    REQUIRE_FALSE(panel.isReady());
}

TEST_CASE("CharacterRulesPanel lifecycle: load/unload", "[phase70]") {
    NovaForge::CharacterRulesPanel panel;
    REQUIRE_FALSE(panel.isReady());
    panel.onProjectLoaded("/project");
    REQUIRE(panel.isReady());
    panel.onProjectUnloaded();
    REQUIRE_FALSE(panel.isReady());
}

// ── IEditorPanel::update() no-op ────────────────────────────────

TEST_CASE("All NovaForge panels update() does not crash", "[phase70]") {
    NovaForge::EconomyPanel        ep; REQUIRE_NOTHROW(ep.update(0.016f));
    NovaForge::InventoryRulesPanel ip; REQUIRE_NOTHROW(ip.update(0.016f));
    NovaForge::ShopPanel           sp; REQUIRE_NOTHROW(sp.update(0.016f));
    NovaForge::MissionRulesPanel   mp; REQUIRE_NOTHROW(mp.update(0.016f));
    NovaForge::ProgressionPanel    pp; REQUIRE_NOTHROW(pp.update(0.016f));
    NovaForge::CharacterRulesPanel cp; REQUIRE_NOTHROW(cp.update(0.016f));
}

// ── NovaForgeAdapter descriptor factories ────────────────────────

TEST_CASE("NovaForgeAdapter provides 6 panel descriptors after initialize", "[phase70]") {
    NovaForge::NovaForgeAdapter adapter("/tmp/novaforge_project");
    REQUIRE(adapter.initialize());
    auto descs = adapter.panelDescriptors();
    REQUIRE(descs.size() == 6u);
}

TEST_CASE("NovaForgeAdapter all descriptors have non-empty createPanel factories", "[phase70]") {
    NovaForge::NovaForgeAdapter adapter("/tmp/novaforge_project");
    adapter.initialize();
    for (const auto& desc : adapter.panelDescriptors()) {
        REQUIRE(static_cast<bool>(desc.createPanel));
    }
}

TEST_CASE("NovaForgeAdapter createPanel factories produce valid IEditorPanel instances", "[phase70]") {
    NovaForge::NovaForgeAdapter adapter("/tmp/novaforge_project");
    adapter.initialize();
    for (const auto& desc : adapter.panelDescriptors()) {
        auto panel = desc.createPanel();
        REQUIRE(panel != nullptr);
        REQUIRE(panel->panelId() == desc.id);
        REQUIRE_FALSE(panel->panelTitle().empty());
    }
}

TEST_CASE("NovaForgeAdapter descriptor IDs match known panel ID constants", "[phase70]") {
    NovaForge::NovaForgeAdapter adapter("/tmp/novaforge_project");
    adapter.initialize();
    auto descs = adapter.panelDescriptors();

    auto hasId = [&descs](const std::string& id) {
        for (const auto& d : descs) { if (d.id == id) return true; }
        return false;
    };

    REQUIRE(hasId("novaforge.economy"));
    REQUIRE(hasId("novaforge.inventory_rules"));
    REQUIRE(hasId("novaforge.shop"));
    REQUIRE(hasId("novaforge.mission_rules"));
    REQUIRE(hasId("novaforge.progression"));
    REQUIRE(hasId("novaforge.character_rules"));
}

TEST_CASE("NovaForgeAdapter descriptors have correct hostToolId", "[phase70]") {
    NovaForge::NovaForgeAdapter adapter("/tmp/novaforge_project");
    adapter.initialize();
    for (const auto& desc : adapter.panelDescriptors()) {
        REQUIRE(desc.hostToolId == "workspace.project_systems");
    }
}

TEST_CASE("NovaForgeAdapter visible panels are the expected 4", "[phase70]") {
    NovaForge::NovaForgeAdapter adapter("/tmp/novaforge_project");
    adapter.initialize();
    int visibleCount = 0;
    for (const auto& desc : adapter.panelDescriptors()) {
        if (desc.defaultVisible) ++visibleCount;
    }
    // economy + inventory_rules + mission_rules + progression = 4 visible
    REQUIRE(visibleCount == 4);
}

// ── ProjectSystemsTool live panel caching ────────────────────────

TEST_CASE("ProjectSystemsTool has zero live panels before any get call", "[phase70]") {
    NF::ProjectSystemsTool pst;
    NovaForge::NovaForgeAdapter adapter("/tmp/proj");
    adapter.initialize();
    pst.loadFromAdapter(adapter);

    REQUIRE(pst.livePanelCount() == 0u);
}

TEST_CASE("ProjectSystemsTool::getOrCreatePanel creates panel on first call", "[phase70]") {
    NF::ProjectSystemsTool pst;
    NovaForge::NovaForgeAdapter adapter("/tmp/proj");
    adapter.initialize();
    pst.loadFromAdapter(adapter);

    NF::IEditorPanel* panel = pst.getOrCreatePanel("novaforge.economy");
    REQUIRE(panel != nullptr);
    REQUIRE(panel->panelId() == "novaforge.economy");
    REQUIRE(pst.livePanelCount() == 1u);
}

TEST_CASE("ProjectSystemsTool::getOrCreatePanel returns same instance on second call", "[phase70]") {
    NF::ProjectSystemsTool pst;
    NovaForge::NovaForgeAdapter adapter("/tmp/proj");
    adapter.initialize();
    pst.loadFromAdapter(adapter);

    NF::IEditorPanel* first  = pst.getOrCreatePanel("novaforge.economy");
    NF::IEditorPanel* second = pst.getOrCreatePanel("novaforge.economy");
    REQUIRE(first == second);
    REQUIRE(pst.livePanelCount() == 1u); // still 1, not 2
}

TEST_CASE("ProjectSystemsTool::getOrCreatePanel returns null for unknown panelId", "[phase70]") {
    NF::ProjectSystemsTool pst;
    REQUIRE(pst.getOrCreatePanel("no.such.panel") == nullptr);
}

TEST_CASE("ProjectSystemsTool::findLivePanel returns null before panel is instantiated", "[phase70]") {
    NF::ProjectSystemsTool pst;
    NovaForge::NovaForgeAdapter adapter("/tmp/proj");
    adapter.initialize();
    pst.loadFromAdapter(adapter);

    REQUIRE(pst.findLivePanel("novaforge.shop") == nullptr);
}

TEST_CASE("ProjectSystemsTool::findLivePanel returns panel after instantiation", "[phase70]") {
    NF::ProjectSystemsTool pst;
    NovaForge::NovaForgeAdapter adapter("/tmp/proj");
    adapter.initialize();
    pst.loadFromAdapter(adapter);

    pst.getOrCreatePanel("novaforge.shop");
    REQUIRE(pst.findLivePanel("novaforge.shop") != nullptr);
}

TEST_CASE("ProjectSystemsTool::getOrCreatePanel all 6 panels", "[phase70]") {
    NF::ProjectSystemsTool pst;
    NovaForge::NovaForgeAdapter adapter("/tmp/proj");
    adapter.initialize();
    pst.loadFromAdapter(adapter);

    const std::vector<std::string> ids = {
        "novaforge.economy", "novaforge.inventory_rules", "novaforge.shop",
        "novaforge.mission_rules", "novaforge.progression", "novaforge.character_rules"
    };
    for (const auto& id : ids) {
        NF::IEditorPanel* p = pst.getOrCreatePanel(id);
        REQUIRE(p != nullptr);
        REQUIRE(p->panelId() == id);
    }
    REQUIRE(pst.livePanelCount() == 6u);
}

// ── ProjectSystemsTool project lifecycle propagation ─────────────

TEST_CASE("ProjectSystemsTool::notifyProjectLoaded makes all live panels ready", "[phase70]") {
    NF::ProjectSystemsTool pst;
    NovaForge::NovaForgeAdapter adapter("/tmp/proj");
    adapter.initialize();
    pst.loadFromAdapter(adapter);

    // Instantiate some panels
    pst.getOrCreatePanel("novaforge.economy");
    pst.getOrCreatePanel("novaforge.progression");

    // Before notify — panels are not ready
    REQUIRE_FALSE(pst.findLivePanel("novaforge.economy")->isReady());

    pst.notifyProjectLoaded("/tmp/proj");

    REQUIRE(pst.findLivePanel("novaforge.economy")->isReady());
    REQUIRE(pst.findLivePanel("novaforge.progression")->isReady());
}

TEST_CASE("ProjectSystemsTool::notifyProjectUnloaded makes all live panels not ready", "[phase70]") {
    NF::ProjectSystemsTool pst;
    NovaForge::NovaForgeAdapter adapter("/tmp/proj");
    adapter.initialize();
    pst.loadFromAdapter(adapter);

    pst.getOrCreatePanel("novaforge.economy");
    pst.notifyProjectLoaded("/tmp/proj");
    REQUIRE(pst.findLivePanel("novaforge.economy")->isReady());

    pst.notifyProjectUnloaded();
    REQUIRE_FALSE(pst.findLivePanel("novaforge.economy")->isReady());
}

// ── ProjectSystemsTool reset ──────────────────────────────────────

TEST_CASE("ProjectSystemsTool::reset destroys all live panels and clears descriptors", "[phase70]") {
    NF::ProjectSystemsTool pst;
    NovaForge::NovaForgeAdapter adapter("/tmp/proj");
    adapter.initialize();
    pst.loadFromAdapter(adapter);

    pst.getOrCreatePanel("novaforge.economy");
    pst.getOrCreatePanel("novaforge.shop");
    REQUIRE(pst.livePanelCount()  == 2u);
    REQUIRE(pst.panelCount()      == 6u);

    pst.reset();
    REQUIRE(pst.livePanelCount()  == 0u);
    REQUIRE(pst.panelCount()      == 0u);
}

TEST_CASE("ProjectSystemsTool::loadFromAdapter clears previously-cached live panels", "[phase70]") {
    NF::ProjectSystemsTool pst;
    NovaForge::NovaForgeAdapter adapterA("/tmp/proj_a");
    adapterA.initialize();
    pst.loadFromAdapter(adapterA);

    pst.getOrCreatePanel("novaforge.economy");
    REQUIRE(pst.livePanelCount() == 1u);

    // Load a new adapter — live panels from previous load must be discarded.
    NovaForge::NovaForgeAdapter adapterB("/tmp/proj_b");
    adapterB.initialize();
    pst.loadFromAdapter(adapterB);
    REQUIRE(pst.livePanelCount() == 0u);
}

// ── Descriptor helpers still work ────────────────────────────────

TEST_CASE("ProjectSystemsTool panelsByCategory returns Gameplay panels", "[phase70]") {
    NF::ProjectSystemsTool pst;
    NovaForge::NovaForgeAdapter adapter("/tmp/proj");
    adapter.initialize();
    pst.loadFromAdapter(adapter);

    auto gp = pst.panelsByCategory("Gameplay");
    REQUIRE(gp.size() == 6u);
}

TEST_CASE("ProjectSystemsTool visiblePanels returns 4 default-visible panels", "[phase70]") {
    NF::ProjectSystemsTool pst;
    NovaForge::NovaForgeAdapter adapter("/tmp/proj");
    adapter.initialize();
    pst.loadFromAdapter(adapter);

    auto vp = pst.visiblePanels();
    REQUIRE(vp.size() == 4u);
}
