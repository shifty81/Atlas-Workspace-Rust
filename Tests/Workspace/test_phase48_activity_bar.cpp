// Tests/Workspace/test_phase48_activity_bar.cpp
// Phase 48 — WorkspaceActivityBar: ActivityItemKind, ActivityBarItem,
//             ActivityBarSection, ActivityBarManager
#include <catch2/catch_test_macros.hpp>
#include "NF/Workspace/WorkspaceActivityBar.h"

// ═════════════════════════════════════════════════════════════════
// ActivityItemKind enum
// ═════════════════════════════════════════════════════════════════

TEST_CASE("ActivityItemKind: name helpers", "[activitybar][kind]") {
    REQUIRE(std::string(NF::activityItemKindName(NF::ActivityItemKind::Tool))      == "Tool");
    REQUIRE(std::string(NF::activityItemKindName(NF::ActivityItemKind::Action))    == "Action");
    REQUIRE(std::string(NF::activityItemKindName(NF::ActivityItemKind::Separator)) == "Separator");
}

// ═════════════════════════════════════════════════════════════════
// ActivityBarItem
// ═════════════════════════════════════════════════════════════════

TEST_CASE("ActivityBarItem: default invalid", "[activitybar][item]") {
    NF::ActivityBarItem item;
    REQUIRE_FALSE(item.isValid());
}

TEST_CASE("ActivityBarItem: valid Tool requires id+label+toolId", "[activitybar][item]") {
    auto item = NF::ActivityBarItem::makeTool("scene.nav", "Scene Editor", "workspace.scene_editor");
    REQUIRE(item.isValid());
    REQUIRE(item.isTool());
    REQUIRE_FALSE(item.isAction());
    REQUIRE_FALSE(item.isSeparator());
}

TEST_CASE("ActivityBarItem: Tool invalid without toolId", "[activitybar][item]") {
    NF::ActivityBarItem item;
    item.id    = "x";
    item.label = "X";
    item.kind  = NF::ActivityItemKind::Tool;
    // toolId is empty → invalid
    REQUIRE_FALSE(item.isValid());
}

TEST_CASE("ActivityBarItem: valid Action requires id+label+commandId", "[activitybar][item]") {
    auto item = NF::ActivityBarItem::makeAction("toggle.cmd", "Toggle View", "view.toggle");
    REQUIRE(item.isValid());
    REQUIRE(item.isAction());
    REQUIRE_FALSE(item.isTool());
    REQUIRE_FALSE(item.isSeparator());
}

TEST_CASE("ActivityBarItem: Action invalid without commandId", "[activitybar][item]") {
    NF::ActivityBarItem item;
    item.id    = "x";
    item.label = "X";
    item.kind  = NF::ActivityItemKind::Action;
    // commandId is empty → invalid
    REQUIRE_FALSE(item.isValid());
}

TEST_CASE("ActivityBarItem: Separator only needs non-empty id", "[activitybar][item]") {
    auto item = NF::ActivityBarItem::makeSeparator("sep.1");
    // Separator: id non-empty, label is "---", no toolId/commandId needed
    REQUIRE(item.isSeparator());
    // Separator validity: kind==Separator bypasses toolId/commandId check
    REQUIRE(item.isValid());
}

TEST_CASE("ActivityBarItem: equality by id", "[activitybar][item]") {
    auto a = NF::ActivityBarItem::makeTool("scene.nav", "Scene", "scene_editor");
    auto b = NF::ActivityBarItem::makeTool("scene.nav", "OTHER", "other_id");
    auto c = NF::ActivityBarItem::makeTool("asset.nav", "Asset", "asset_editor");
    REQUIRE(a == b);
    REQUIRE(a != c);
}

TEST_CASE("ActivityBarItem: defaults", "[activitybar][item]") {
    auto item = NF::ActivityBarItem::makeTool("t.nav", "Tool", "workspace.tool");
    REQUIRE(item.enabled);
    REQUIRE_FALSE(item.pinned);
    REQUIRE(item.kind == NF::ActivityItemKind::Tool);
    REQUIRE(item.iconKey.empty());
}

// ═════════════════════════════════════════════════════════════════
// ActivityBarSection
// ═════════════════════════════════════════════════════════════════

TEST_CASE("ActivityBarSection: default empty state", "[activitybar][section]") {
    NF::ActivityBarSection sec("Tools");
    REQUIRE(sec.name() == "Tools");
    REQUIRE(sec.empty());
    REQUIRE(sec.count() == 0);
    REQUIRE(sec.items().empty());
}

TEST_CASE("ActivityBarSection: addItem success", "[activitybar][section]") {
    NF::ActivityBarSection sec("Tools");
    auto item = NF::ActivityBarItem::makeTool("scene.nav", "Scene", "scene_editor");
    REQUIRE(sec.addItem(item));
    REQUIRE(sec.count() == 1);
    REQUIRE_FALSE(sec.empty());
    REQUIRE(sec.contains("scene.nav"));
}

TEST_CASE("ActivityBarSection: addItem duplicate rejected", "[activitybar][section]") {
    NF::ActivityBarSection sec("Tools");
    auto item = NF::ActivityBarItem::makeTool("scene.nav", "Scene", "scene_editor");
    REQUIRE(sec.addItem(item));
    REQUIRE_FALSE(sec.addItem(item)); // duplicate id
    REQUIRE(sec.count() == 1);
}

TEST_CASE("ActivityBarSection: removeItem success", "[activitybar][section]") {
    NF::ActivityBarSection sec("Tools");
    auto item = NF::ActivityBarItem::makeTool("scene.nav", "Scene", "scene_editor");
    sec.addItem(item);
    REQUIRE(sec.removeItem("scene.nav"));
    REQUIRE(sec.empty());
}

TEST_CASE("ActivityBarSection: removeItem unknown returns false", "[activitybar][section]") {
    NF::ActivityBarSection sec("Tools");
    REQUIRE_FALSE(sec.removeItem("no.such.id"));
}

TEST_CASE("ActivityBarSection: findItem returns correct pointer", "[activitybar][section]") {
    NF::ActivityBarSection sec("Tools");
    auto item = NF::ActivityBarItem::makeTool("scene.nav", "Scene", "scene_editor");
    sec.addItem(item);
    const auto* found = sec.findItem("scene.nav");
    REQUIRE(found != nullptr);
    REQUIRE(found->toolId == "scene_editor");
    REQUIRE(sec.findItem("no.such") == nullptr);
}

TEST_CASE("ActivityBarSection: findItemMut allows mutation", "[activitybar][section]") {
    NF::ActivityBarSection sec("Tools");
    auto item = NF::ActivityBarItem::makeTool("t", "T", "tool_id");
    sec.addItem(item);
    auto* mut = sec.findItemMut("t");
    REQUIRE(mut != nullptr);
    mut->label = "Updated";
    REQUIRE(sec.findItem("t")->label == "Updated");
}

TEST_CASE("ActivityBarSection: Separator item has no kind constraints", "[activitybar][section]") {
    NF::ActivityBarSection sec("Tools");
    auto sep = NF::ActivityBarItem::makeSeparator("sep.1");
    REQUIRE(sec.addItem(sep));
    REQUIRE(sec.count() == 1);
    REQUIRE(sec.findItem("sep.1")->isSeparator());
}

TEST_CASE("ActivityBarSection: clear empties section", "[activitybar][section]") {
    NF::ActivityBarSection sec("Tools");
    sec.addItem(NF::ActivityBarItem::makeTool("a", "A", "tool_a"));
    sec.addItem(NF::ActivityBarItem::makeTool("b", "B", "tool_b"));
    REQUIRE(sec.count() == 2);
    sec.clear();
    REQUIRE(sec.empty());
}

TEST_CASE("ActivityBarSection: MAX_ITEMS capacity enforced", "[activitybar][section]") {
    NF::ActivityBarSection sec("Big");
    for (size_t i = 0; i < NF::ActivityBarSection::MAX_ITEMS; ++i) {
        std::string id = "item." + std::to_string(i);
        auto item = NF::ActivityBarItem::makeTool(id, id, "tool_x");
        REQUIRE(sec.addItem(item));
    }
    // One more should fail
    auto overflow = NF::ActivityBarItem::makeTool("overflow", "over", "tool_x");
    REQUIRE_FALSE(sec.addItem(overflow));
}

// ═════════════════════════════════════════════════════════════════
// ActivityBarManager
// ═════════════════════════════════════════════════════════════════

TEST_CASE("ActivityBarManager: default empty state", "[activitybar][manager]") {
    NF::ActivityBarManager mgr;
    REQUIRE(mgr.empty());
    REQUIRE(mgr.sectionCount() == 0);
    REQUIRE_FALSE(mgr.hasActiveItem());
    REQUIRE(mgr.activeItemId().empty());
}

TEST_CASE("ActivityBarManager: createSection success", "[activitybar][manager]") {
    NF::ActivityBarManager mgr;
    auto* sec = mgr.createSection("Tools");
    REQUIRE(sec != nullptr);
    REQUIRE(sec->name() == "Tools");
    REQUIRE(mgr.sectionCount() == 1);
    REQUIRE_FALSE(mgr.empty());
    REQUIRE(mgr.hasSection("Tools"));
}

TEST_CASE("ActivityBarManager: createSection duplicate rejected", "[activitybar][manager]") {
    NF::ActivityBarManager mgr;
    REQUIRE(mgr.createSection("Tools") != nullptr);
    REQUIRE(mgr.createSection("Tools") == nullptr); // duplicate
    REQUIRE(mgr.sectionCount() == 1);
}

TEST_CASE("ActivityBarManager: createSection empty name rejected", "[activitybar][manager]") {
    NF::ActivityBarManager mgr;
    REQUIRE(mgr.createSection("") == nullptr);
    REQUIRE(mgr.sectionCount() == 0);
}

TEST_CASE("ActivityBarManager: removeSection success", "[activitybar][manager]") {
    NF::ActivityBarManager mgr;
    mgr.createSection("Tools");
    REQUIRE(mgr.removeSection("Tools"));
    REQUIRE(mgr.sectionCount() == 0);
    REQUIRE_FALSE(mgr.hasSection("Tools"));
}

TEST_CASE("ActivityBarManager: removeSection unknown returns false", "[activitybar][manager]") {
    NF::ActivityBarManager mgr;
    REQUIRE_FALSE(mgr.removeSection("no.such.section"));
}

TEST_CASE("ActivityBarManager: findSection returns correct pointer", "[activitybar][manager]") {
    NF::ActivityBarManager mgr;
    mgr.createSection("Tools");
    auto* sec = mgr.findSection("Tools");
    REQUIRE(sec != nullptr);
    REQUIRE(sec->name() == "Tools");
    REQUIRE(mgr.findSection("Missing") == nullptr);
}

TEST_CASE("ActivityBarManager: addItem to section", "[activitybar][manager]") {
    NF::ActivityBarManager mgr;
    mgr.createSection("Tools");
    auto item = NF::ActivityBarItem::makeTool("scene.nav", "Scene", "scene_editor");
    REQUIRE(mgr.addItem("Tools", item));
    REQUIRE(mgr.findSection("Tools")->count() == 1);
}

TEST_CASE("ActivityBarManager: addItem to unknown section fails", "[activitybar][manager]") {
    NF::ActivityBarManager mgr;
    auto item = NF::ActivityBarItem::makeTool("scene.nav", "Scene", "scene_editor");
    REQUIRE_FALSE(mgr.addItem("Missing", item));
}

TEST_CASE("ActivityBarManager: removeItem searches all sections", "[activitybar][manager]") {
    NF::ActivityBarManager mgr;
    mgr.createSection("A");
    mgr.createSection("B");
    mgr.addItem("A", NF::ActivityBarItem::makeTool("a.nav", "A", "tool_a"));
    mgr.addItem("B", NF::ActivityBarItem::makeTool("b.nav", "B", "tool_b"));
    REQUIRE(mgr.removeItem("b.nav"));
    REQUIRE(mgr.findSection("B")->empty());
    REQUIRE(mgr.findSection("A")->count() == 1);
}

TEST_CASE("ActivityBarManager: removeItem unknown returns false", "[activitybar][manager]") {
    NF::ActivityBarManager mgr;
    REQUIRE_FALSE(mgr.removeItem("no.such.item"));
}

TEST_CASE("ActivityBarManager: findItem searches all sections", "[activitybar][manager]") {
    NF::ActivityBarManager mgr;
    mgr.createSection("A");
    mgr.addItem("A", NF::ActivityBarItem::makeTool("t.nav", "T", "tool_t"));
    const auto* found = mgr.findItem("t.nav");
    REQUIRE(found != nullptr);
    REQUIRE(found->toolId == "tool_t");
    REQUIRE(mgr.findItem("missing") == nullptr);
}

TEST_CASE("ActivityBarManager: setActiveItem success", "[activitybar][manager]") {
    NF::ActivityBarManager mgr;
    mgr.createSection("Tools");
    mgr.addItem("Tools", NF::ActivityBarItem::makeTool("t.nav", "T", "tool_t"));
    REQUIRE(mgr.setActiveItem("t.nav"));
    REQUIRE(mgr.activeItemId() == "t.nav");
    REQUIRE(mgr.hasActiveItem());
    REQUIRE(mgr.isActive("t.nav"));
}

TEST_CASE("ActivityBarManager: setActiveItem unknown returns false", "[activitybar][manager]") {
    NF::ActivityBarManager mgr;
    REQUIRE_FALSE(mgr.setActiveItem("no.such.item"));
    REQUIRE_FALSE(mgr.hasActiveItem());
}

TEST_CASE("ActivityBarManager: setActiveItem disabled returns false", "[activitybar][manager]") {
    NF::ActivityBarManager mgr;
    mgr.createSection("Tools");
    auto item = NF::ActivityBarItem::makeTool("t.nav", "T", "tool_t");
    item.enabled = false;
    mgr.addItem("Tools", item);
    REQUIRE_FALSE(mgr.setActiveItem("t.nav"));
    REQUIRE_FALSE(mgr.hasActiveItem());
}

TEST_CASE("ActivityBarManager: clearActiveItem clears selection", "[activitybar][manager]") {
    NF::ActivityBarManager mgr;
    mgr.createSection("Tools");
    mgr.addItem("Tools", NF::ActivityBarItem::makeTool("t.nav", "T", "tool_t"));
    mgr.setActiveItem("t.nav");
    REQUIRE(mgr.hasActiveItem());
    mgr.clearActiveItem();
    REQUIRE_FALSE(mgr.hasActiveItem());
    REQUIRE(mgr.activeItemId().empty());
}

TEST_CASE("ActivityBarManager: enableItem mutates item", "[activitybar][manager]") {
    NF::ActivityBarManager mgr;
    mgr.createSection("Tools");
    mgr.addItem("Tools", NF::ActivityBarItem::makeTool("t.nav", "T", "tool_t"));
    REQUIRE(mgr.enableItem("t.nav", false));
    REQUIRE_FALSE(mgr.findItem("t.nav")->enabled);
    REQUIRE(mgr.enableItem("t.nav", true));
    REQUIRE(mgr.findItem("t.nav")->enabled);
}

TEST_CASE("ActivityBarManager: enableItem unknown returns false", "[activitybar][manager]") {
    NF::ActivityBarManager mgr;
    REQUIRE_FALSE(mgr.enableItem("no.item", false));
}

TEST_CASE("ActivityBarManager: observer fires on setActiveItem", "[activitybar][manager]") {
    NF::ActivityBarManager mgr;
    mgr.createSection("Tools");
    mgr.addItem("Tools", NF::ActivityBarItem::makeTool("t.nav", "T", "tool_t"));

    std::vector<std::pair<std::string, bool>> log;
    mgr.addObserver([&](const std::string& id, bool activated) {
        log.push_back({id, activated});
    });

    mgr.setActiveItem("t.nav");
    REQUIRE(log.size() == 1);
    REQUIRE(log[0].first == "t.nav");
    REQUIRE(log[0].second == true);
}

TEST_CASE("ActivityBarManager: observer fires deactivate then activate on switch", "[activitybar][manager]") {
    NF::ActivityBarManager mgr;
    mgr.createSection("Tools");
    mgr.addItem("Tools", NF::ActivityBarItem::makeTool("a.nav", "A", "tool_a"));
    mgr.addItem("Tools", NF::ActivityBarItem::makeTool("b.nav", "B", "tool_b"));

    std::vector<std::pair<std::string, bool>> log;
    mgr.addObserver([&](const std::string& id, bool activated) {
        log.push_back({id, activated});
    });

    mgr.setActiveItem("a.nav"); // → activate a
    mgr.setActiveItem("b.nav"); // → deactivate a, activate b

    REQUIRE(log.size() == 3);
    REQUIRE(log[0] == std::make_pair(std::string("a.nav"), true));
    REQUIRE(log[1] == std::make_pair(std::string("a.nav"), false));
    REQUIRE(log[2] == std::make_pair(std::string("b.nav"), true));
}

TEST_CASE("ActivityBarManager: observer fires on clearActiveItem", "[activitybar][manager]") {
    NF::ActivityBarManager mgr;
    mgr.createSection("Tools");
    mgr.addItem("Tools", NF::ActivityBarItem::makeTool("t.nav", "T", "tool_t"));
    mgr.setActiveItem("t.nav");

    std::vector<std::pair<std::string, bool>> log;
    mgr.addObserver([&](const std::string& id, bool activated) {
        log.push_back({id, activated});
    });

    mgr.clearActiveItem();
    REQUIRE(log.size() == 1);
    REQUIRE(log[0].first == "t.nav");
    REQUIRE(log[0].second == false);
}

TEST_CASE("ActivityBarManager: clearObservers stops notifications", "[activitybar][manager]") {
    NF::ActivityBarManager mgr;
    mgr.createSection("Tools");
    mgr.addItem("Tools", NF::ActivityBarItem::makeTool("t.nav", "T", "tool_t"));

    int count = 0;
    mgr.addObserver([&](const std::string&, bool) { ++count; });
    mgr.setActiveItem("t.nav");
    REQUIRE(count == 1);

    mgr.clearObservers();
    mgr.clearActiveItem();
    REQUIRE(count == 1); // no further notifications
}

TEST_CASE("ActivityBarManager: MAX_SECTIONS capacity enforced", "[activitybar][manager]") {
    NF::ActivityBarManager mgr;
    for (size_t i = 0; i < NF::ActivityBarManager::MAX_SECTIONS; ++i) {
        std::string name = "Section." + std::to_string(i);
        REQUIRE(mgr.createSection(name) != nullptr);
    }
    REQUIRE(mgr.createSection("Overflow") == nullptr);
    REQUIRE(mgr.sectionCount() == NF::ActivityBarManager::MAX_SECTIONS);
}

TEST_CASE("ActivityBarManager: setActiveItem no-op if same item", "[activitybar][manager]") {
    NF::ActivityBarManager mgr;
    mgr.createSection("Tools");
    mgr.addItem("Tools", NF::ActivityBarItem::makeTool("t.nav", "T", "tool_t"));

    std::vector<std::pair<std::string, bool>> log;
    mgr.addObserver([&](const std::string& id, bool activated) {
        log.push_back({id, activated});
    });

    mgr.setActiveItem("t.nav");
    mgr.setActiveItem("t.nav"); // same id again — should not fire deactivate+activate
    REQUIRE(log.size() == 1);   // only the first activation
}

TEST_CASE("ActivityBarManager: clear removes all sections, items, observers", "[activitybar][manager]") {
    NF::ActivityBarManager mgr;
    mgr.createSection("Tools");
    mgr.addItem("Tools", NF::ActivityBarItem::makeTool("t.nav", "T", "tool_t"));
    mgr.setActiveItem("t.nav");
    int count = 0;
    mgr.addObserver([&](const std::string&, bool) { ++count; });

    mgr.clear();
    REQUIRE(mgr.empty());
    REQUIRE(mgr.sectionCount() == 0);
    REQUIRE_FALSE(mgr.hasActiveItem());
    // observers also cleared
    mgr.createSection("S");
    mgr.addItem("S", NF::ActivityBarItem::makeTool("x.nav", "X", "tool_x"));
    mgr.setActiveItem("x.nav");
    REQUIRE(count == 0); // no observer left
}

// ═════════════════════════════════════════════════════════════════
// Integration
// ═════════════════════════════════════════════════════════════════

TEST_CASE("ActivityBar integration: multi-section tool navigator", "[activitybar][integration]") {
    NF::ActivityBarManager mgr;
    mgr.createSection("Primary Tools");
    mgr.createSection("Utilities");

    mgr.addItem("Primary Tools", NF::ActivityBarItem::makeTool(
        "scene.nav", "Scene Editor", "workspace.scene_editor"));
    mgr.addItem("Primary Tools", NF::ActivityBarItem::makeTool(
        "asset.nav", "Asset Editor", "workspace.asset_editor"));
    mgr.addItem("Utilities", NF::ActivityBarItem::makeAction(
        "cmd.palette", "Command Palette", "command.palette.open"));
    mgr.addItem("Utilities", NF::ActivityBarItem::makeSeparator("sep.1"));

    REQUIRE(mgr.findSection("Primary Tools")->count() == 2);
    REQUIRE(mgr.findSection("Utilities")->count() == 2);

    // Activate scene tool
    std::vector<std::string> activated;
    mgr.addObserver([&](const std::string& id, bool act) {
        if (act) activated.push_back(id);
    });

    REQUIRE(mgr.setActiveItem("scene.nav"));
    REQUIRE(mgr.activeItemId() == "scene.nav");

    // Switch to asset
    REQUIRE(mgr.setActiveItem("asset.nav"));
    REQUIRE(mgr.activeItemId() == "asset.nav");

    REQUIRE(activated.size() == 2);
    REQUIRE(activated[0] == "scene.nav");
    REQUIRE(activated[1] == "asset.nav");
}

TEST_CASE("ActivityBar integration: disable and re-enable item", "[activitybar][integration]") {
    NF::ActivityBarManager mgr;
    mgr.createSection("Tools");
    mgr.addItem("Tools", NF::ActivityBarItem::makeTool("t.nav", "T", "tool_t"));

    mgr.setActiveItem("t.nav");
    REQUIRE(mgr.activeItemId() == "t.nav");

    // Disable the active item
    mgr.enableItem("t.nav", false);
    // Trying to activate disabled fails
    mgr.clearActiveItem();
    REQUIRE_FALSE(mgr.setActiveItem("t.nav"));

    // Re-enable
    mgr.enableItem("t.nav", true);
    REQUIRE(mgr.setActiveItem("t.nav"));
    REQUIRE(mgr.isActive("t.nav"));
}

TEST_CASE("ActivityBar integration: multiple observers", "[activitybar][integration]") {
    NF::ActivityBarManager mgr;
    mgr.createSection("Tools");
    mgr.addItem("Tools", NF::ActivityBarItem::makeTool("t.nav", "T", "tool_t"));

    int obs1 = 0, obs2 = 0;
    mgr.addObserver([&](const std::string&, bool) { ++obs1; });
    mgr.addObserver([&](const std::string&, bool) { ++obs2; });

    mgr.setActiveItem("t.nav");
    REQUIRE(obs1 == 1);
    REQUIRE(obs2 == 1);

    mgr.clearActiveItem();
    REQUIRE(obs1 == 2);
    REQUIRE(obs2 == 2);
}

TEST_CASE("ActivityBar integration: sections() view is stable", "[activitybar][integration]") {
    NF::ActivityBarManager mgr;
    mgr.createSection("A");
    mgr.createSection("B");
    mgr.addItem("A", NF::ActivityBarItem::makeTool("a1", "A1", "tool_a1"));
    mgr.addItem("B", NF::ActivityBarItem::makeTool("b1", "B1", "tool_b1"));

    size_t total = 0;
    for (const auto& sec : mgr.sections())
        total += sec.count();
    REQUIRE(total == 2);
}
