// Tests/Workspace/test_phase26_context_menu.cpp
// Phase 26 — Workspace Context Menu System
//
// Tests for:
//   1. MenuItemKind      — enum name helpers
//   2. ContextMenuItem   — descriptor; isValid, separator helper, equality
//   3. ContextMenu       — ordered item list with submenu trees
//   4. ContextMenuManager — named menu registry; open/close; action/lifecycle observers
//   5. Integration        — full context menu pipeline

#include <catch2/catch_test_macros.hpp>
#include "NF/Workspace/WorkspaceContextMenu.h"
#include <string>
#include <vector>

using namespace NF;

// ═════════════════════════════════════════════════════════════════
// SECTION 1 — MenuItemKind enum
// ═════════════════════════════════════════════════════════════════

TEST_CASE("menuItemKindName returns correct strings", "[Phase26][MenuItemKind]") {
    CHECK(std::string(menuItemKindName(MenuItemKind::Action))    == "Action");
    CHECK(std::string(menuItemKindName(MenuItemKind::Separator)) == "Separator");
    CHECK(std::string(menuItemKindName(MenuItemKind::Submenu))   == "Submenu");
}

// ═════════════════════════════════════════════════════════════════
// SECTION 2 — ContextMenuItem
// ═════════════════════════════════════════════════════════════════

TEST_CASE("ContextMenuItem default is invalid", "[Phase26][ContextMenuItem]") {
    ContextMenuItem item;
    CHECK_FALSE(item.isValid());
    CHECK(item.id.empty());
    CHECK(item.kind == MenuItemKind::Action);
    CHECK(item.enabled);
}

TEST_CASE("ContextMenuItem valid action construction", "[Phase26][ContextMenuItem]") {
    ContextMenuItem item{"undo", "Undo", MenuItemKind::Action, true, "Ctrl+Z", "ico_undo"};
    CHECK(item.isValid());
    CHECK(item.id       == "undo");
    CHECK(item.label    == "Undo");
    CHECK(item.kind     == MenuItemKind::Action);
    CHECK(item.shortcut == "Ctrl+Z");
    CHECK(item.icon     == "ico_undo");
}

TEST_CASE("ContextMenuItem invalid without id", "[Phase26][ContextMenuItem]") {
    ContextMenuItem item{"", "Label", MenuItemKind::Action, true, "", ""};
    CHECK_FALSE(item.isValid());
}

TEST_CASE("ContextMenuItem invalid action without label", "[Phase26][ContextMenuItem]") {
    ContextMenuItem item{"id1", "", MenuItemKind::Action, true, "", ""};
    CHECK_FALSE(item.isValid());
}

TEST_CASE("ContextMenuItem separator valid with id only", "[Phase26][ContextMenuItem]") {
    auto sep = ContextMenuItem::separator("sep_1");
    CHECK(sep.isValid());
    CHECK(sep.id == "sep_1");
    CHECK(sep.kind == MenuItemKind::Separator);
    CHECK(sep.label.empty());
}

TEST_CASE("ContextMenuItem separator invalid without id", "[Phase26][ContextMenuItem]") {
    auto sep = ContextMenuItem::separator("");
    CHECK_FALSE(sep.isValid());
}

TEST_CASE("ContextMenuItem equality by id", "[Phase26][ContextMenuItem]") {
    ContextMenuItem a{"id_1", "A", MenuItemKind::Action, true, "", ""};
    ContextMenuItem b{"id_1", "B", MenuItemKind::Submenu, false, "", ""};
    ContextMenuItem c{"id_2", "A", MenuItemKind::Action, true, "", ""};
    CHECK(a == b);
    CHECK(a != c);
}

// ═════════════════════════════════════════════════════════════════
// SECTION 3 — ContextMenu
// ═════════════════════════════════════════════════════════════════

TEST_CASE("ContextMenu default is invalid", "[Phase26][ContextMenu]") {
    ContextMenu menu;
    CHECK_FALSE(menu.isValid());
    CHECK(menu.empty());
    CHECK(menu.itemCount() == 0);
}

TEST_CASE("ContextMenu valid construction", "[Phase26][ContextMenu]") {
    ContextMenu menu("ctx_edit");
    CHECK(menu.isValid());
    CHECK(menu.id() == "ctx_edit");
}

TEST_CASE("ContextMenu addItem", "[Phase26][ContextMenu]") {
    ContextMenu menu("m");
    ContextMenuItem item{"undo", "Undo", MenuItemKind::Action, true, "Ctrl+Z", ""};
    CHECK(menu.addItem(item));
    CHECK(menu.contains("undo"));
    CHECK(menu.itemCount() == 1);
}

TEST_CASE("ContextMenu duplicate addItem fails", "[Phase26][ContextMenu]") {
    ContextMenu menu("m");
    ContextMenuItem item{"undo", "Undo", MenuItemKind::Action, true, "", ""};
    CHECK(menu.addItem(item));
    CHECK_FALSE(menu.addItem(item));
}

TEST_CASE("ContextMenu invalid item rejected", "[Phase26][ContextMenu]") {
    ContextMenu menu("m");
    ContextMenuItem item;  // invalid
    CHECK_FALSE(menu.addItem(item));
}

TEST_CASE("ContextMenu removeItem", "[Phase26][ContextMenu]") {
    ContextMenu menu("m");
    menu.addItem({"undo", "Undo", MenuItemKind::Action, true, "", ""});
    CHECK(menu.removeItem("undo"));
    CHECK(menu.empty());
}

TEST_CASE("ContextMenu removeItem unknown fails", "[Phase26][ContextMenu]") {
    ContextMenu menu("m");
    CHECK_FALSE(menu.removeItem("nope"));
}

TEST_CASE("ContextMenu updateItem", "[Phase26][ContextMenu]") {
    ContextMenu menu("m");
    menu.addItem({"undo", "Undo", MenuItemKind::Action, true, "", ""});
    ContextMenuItem updated{"undo", "Undo All", MenuItemKind::Action, false, "Ctrl+Shift+Z", ""};
    CHECK(menu.updateItem(updated));
    auto* found = menu.findItem("undo");
    CHECK(found->label   == "Undo All");
    CHECK_FALSE(found->enabled);
}

TEST_CASE("ContextMenu updateItem unknown fails", "[Phase26][ContextMenu]") {
    ContextMenu menu("m");
    ContextMenuItem item{"nope", "Nope", MenuItemKind::Action, true, "", ""};
    CHECK_FALSE(menu.updateItem(item));
}

TEST_CASE("ContextMenu findItem", "[Phase26][ContextMenu]") {
    ContextMenu menu("m");
    menu.addItem({"cut", "Cut", MenuItemKind::Action, true, "Ctrl+X", "ico_cut"});
    auto* found = menu.findItem("cut");
    CHECK(found != nullptr);
    CHECK(found->shortcut == "Ctrl+X");
}

TEST_CASE("ContextMenu separator can be added", "[Phase26][ContextMenu]") {
    ContextMenu menu("m");
    menu.addItem({"undo", "Undo", MenuItemKind::Action, true, "", ""});
    menu.addItem(ContextMenuItem::separator("sep_1"));
    menu.addItem({"copy", "Copy", MenuItemKind::Action, true, "", ""});
    CHECK(menu.itemCount() == 3);
    CHECK(menu.findItem("sep_1")->kind == MenuItemKind::Separator);
}

TEST_CASE("ContextMenu attachSubmenu", "[Phase26][ContextMenu]") {
    ContextMenu parent("parent");
    parent.addItem({"sub_edit", "Edit", MenuItemKind::Submenu, true, "", ""});

    ContextMenu sub("sub_edit");
    sub.addItem({"cut", "Cut", MenuItemKind::Action, true, "", ""});

    CHECK(parent.attachSubmenu(sub));
    CHECK(parent.findSubmenu("sub_edit") != nullptr);
    CHECK(parent.findSubmenu("sub_edit")->contains("cut"));
}

TEST_CASE("ContextMenu attachSubmenu fails if parent item not Submenu kind", "[Phase26][ContextMenu]") {
    ContextMenu parent("parent");
    parent.addItem({"act", "Action", MenuItemKind::Action, true, "", ""});

    ContextMenu sub("act");
    sub.addItem({"x", "X", MenuItemKind::Action, true, "", ""});

    CHECK_FALSE(parent.attachSubmenu(sub));
}

TEST_CASE("ContextMenu clear removes all items and submenus", "[Phase26][ContextMenu]") {
    ContextMenu menu("m");
    menu.addItem({"a", "A", MenuItemKind::Action, true, "", ""});
    menu.clear();
    CHECK(menu.empty());
}

// ═════════════════════════════════════════════════════════════════
// SECTION 4 — ContextMenuManager
// ═════════════════════════════════════════════════════════════════

TEST_CASE("ContextMenuManager empty state", "[Phase26][ContextMenuManager]") {
    ContextMenuManager m;
    CHECK(m.allMenuIds().empty());
    CHECK_FALSE(m.hasOpenMenu());
    CHECK(m.openMenuId().empty());
}

TEST_CASE("ContextMenuManager registerMenu", "[Phase26][ContextMenuManager]") {
    ContextMenuManager m;
    ContextMenu menu("ctx_scene");
    CHECK(m.registerMenu(menu));
    CHECK(m.isRegistered("ctx_scene"));
}

TEST_CASE("ContextMenuManager duplicate register fails", "[Phase26][ContextMenuManager]") {
    ContextMenuManager m;
    ContextMenu menu("ctx_scene");
    CHECK(m.registerMenu(menu));
    CHECK_FALSE(m.registerMenu(menu));
}

TEST_CASE("ContextMenuManager invalid menu rejected", "[Phase26][ContextMenuManager]") {
    ContextMenuManager m;
    ContextMenu menu;  // invalid
    CHECK_FALSE(m.registerMenu(menu));
}

TEST_CASE("ContextMenuManager unregisterMenu", "[Phase26][ContextMenuManager]") {
    ContextMenuManager m;
    m.registerMenu(ContextMenu("m1"));
    CHECK(m.unregisterMenu("m1"));
    CHECK_FALSE(m.isRegistered("m1"));
}

TEST_CASE("ContextMenuManager unregister unknown fails", "[Phase26][ContextMenuManager]") {
    ContextMenuManager m;
    CHECK_FALSE(m.unregisterMenu("nope"));
}

TEST_CASE("ContextMenuManager openMenu", "[Phase26][ContextMenuManager]") {
    ContextMenuManager m;
    m.registerMenu(ContextMenu("m1"));
    CHECK(m.openMenu("m1"));
    CHECK(m.isOpen("m1"));
    CHECK(m.hasOpenMenu());
    CHECK(m.openMenuId() == "m1");
}

TEST_CASE("ContextMenuManager openMenu unknown fails", "[Phase26][ContextMenuManager]") {
    ContextMenuManager m;
    CHECK_FALSE(m.openMenu("nope"));
}

TEST_CASE("ContextMenuManager openMenu same menu twice fails", "[Phase26][ContextMenuManager]") {
    ContextMenuManager m;
    m.registerMenu(ContextMenu("m1"));
    m.openMenu("m1");
    CHECK_FALSE(m.openMenu("m1"));
}

TEST_CASE("ContextMenuManager closeMenu", "[Phase26][ContextMenuManager]") {
    ContextMenuManager m;
    m.registerMenu(ContextMenu("m1"));
    m.openMenu("m1");
    CHECK(m.closeMenu());
    CHECK_FALSE(m.hasOpenMenu());
}

TEST_CASE("ContextMenuManager closeMenu with nothing open fails", "[Phase26][ContextMenuManager]") {
    ContextMenuManager m;
    CHECK_FALSE(m.closeMenu());
}

TEST_CASE("ContextMenuManager opening second menu closes first", "[Phase26][ContextMenuManager]") {
    ContextMenuManager m;
    m.registerMenu(ContextMenu("m1"));
    m.registerMenu(ContextMenu("m2"));
    m.openMenu("m1");
    m.openMenu("m2");
    CHECK_FALSE(m.isOpen("m1"));
    CHECK(m.isOpen("m2"));
}

TEST_CASE("ContextMenuManager unregister closes open menu", "[Phase26][ContextMenuManager]") {
    ContextMenuManager m;
    m.registerMenu(ContextMenu("m1"));
    m.openMenu("m1");
    m.unregisterMenu("m1");
    CHECK_FALSE(m.hasOpenMenu());
}

TEST_CASE("ContextMenuManager activateItem fires action observer", "[Phase26][ContextMenuManager]") {
    ContextMenuManager m;
    ContextMenu menu("ctx");
    menu.addItem({"undo", "Undo", MenuItemKind::Action, true, "Ctrl+Z", ""});
    m.registerMenu(menu);

    std::string firedMenu, firedItem;
    m.addActionObserver([&](const std::string& mid, const std::string& iid) {
        firedMenu = mid; firedItem = iid;
    });

    CHECK(m.activateItem("ctx", "undo"));
    CHECK(firedMenu == "ctx");
    CHECK(firedItem == "undo");
}

TEST_CASE("ContextMenuManager activateItem disabled item fails", "[Phase26][ContextMenuManager]") {
    ContextMenuManager m;
    ContextMenu menu("ctx");
    menu.addItem({"undo", "Undo", MenuItemKind::Action, false, "", ""});
    m.registerMenu(menu);
    int calls = 0;
    m.addActionObserver([&](const std::string&, const std::string&) { ++calls; });
    CHECK_FALSE(m.activateItem("ctx", "undo"));
    CHECK(calls == 0);
}

TEST_CASE("ContextMenuManager activateItem separator fails", "[Phase26][ContextMenuManager]") {
    ContextMenuManager m;
    ContextMenu menu("ctx");
    menu.addItem(ContextMenuItem::separator("sep"));
    m.registerMenu(menu);
    CHECK_FALSE(m.activateItem("ctx", "sep"));
}

TEST_CASE("ContextMenuManager lifecycle observer fires on open/close", "[Phase26][ContextMenuManager]") {
    ContextMenuManager m;
    m.registerMenu(ContextMenu("m1"));

    std::vector<std::pair<std::string,bool>> events;
    m.addLifecycleObserver([&](const std::string& id, bool opened) {
        events.push_back({id, opened});
    });

    m.openMenu("m1");
    m.closeMenu();

    CHECK(events.size() == 2);
    CHECK(events[0] == std::make_pair(std::string("m1"), true));
    CHECK(events[1] == std::make_pair(std::string("m1"), false));
}

TEST_CASE("ContextMenuManager removeObserver stops notifications", "[Phase26][ContextMenuManager]") {
    ContextMenuManager m;
    m.registerMenu(ContextMenu("m1"));
    int calls = 0;
    uint32_t id = m.addLifecycleObserver([&](const std::string&, bool) { ++calls; });
    m.removeObserver(id);
    m.openMenu("m1");
    CHECK(calls == 0);
}

TEST_CASE("ContextMenuManager allMenuIds", "[Phase26][ContextMenuManager]") {
    ContextMenuManager m;
    m.registerMenu(ContextMenu("m1"));
    m.registerMenu(ContextMenu("m2"));
    m.registerMenu(ContextMenu("m3"));
    CHECK(m.allMenuIds().size() == 3);
}

TEST_CASE("ContextMenuManager clear removes all menus", "[Phase26][ContextMenuManager]") {
    ContextMenuManager m;
    m.registerMenu(ContextMenu("m1"));
    m.openMenu("m1");
    m.clear();
    CHECK(m.allMenuIds().empty());
    CHECK_FALSE(m.hasOpenMenu());
}

// ═════════════════════════════════════════════════════════════════
// SECTION 5 — Integration
// ═════════════════════════════════════════════════════════════════

TEST_CASE("Full context menu pipeline: register → open → activate → close", "[Phase26][Integration]") {
    ContextMenuManager m;

    ContextMenu editMenu("ctx_edit");
    editMenu.addItem({"undo", "Undo", MenuItemKind::Action, true, "Ctrl+Z", ""});
    editMenu.addItem(ContextMenuItem::separator("sep_1"));
    editMenu.addItem({"cut",  "Cut",  MenuItemKind::Action, true, "Ctrl+X", ""});
    editMenu.addItem({"copy", "Copy", MenuItemKind::Action, true, "Ctrl+C", ""});
    editMenu.addItem({"paste","Paste",MenuItemKind::Action, true, "Ctrl+V", ""});
    m.registerMenu(editMenu);

    std::vector<std::pair<std::string,bool>> lifecycle;
    std::string activatedItem;
    m.addLifecycleObserver([&](const std::string& id, bool opened) { lifecycle.push_back({id, opened}); });
    m.addActionObserver([&](const std::string&, const std::string& iid) { activatedItem = iid; });

    m.openMenu("ctx_edit");
    CHECK(m.isOpen("ctx_edit"));
    m.activateItem("ctx_edit", "copy");
    CHECK(activatedItem == "copy");
    m.closeMenu();

    CHECK(lifecycle.size() == 2);
    CHECK(lifecycle[0].second == true);
    CHECK(lifecycle[1].second == false);
}

TEST_CASE("Submenu tree preserved on findMenu", "[Phase26][Integration]") {
    ContextMenuManager m;

    ContextMenu parent("parent");
    parent.addItem({"file_sub", "File", MenuItemKind::Submenu, true, "", ""});

    ContextMenu fileSub("file_sub");
    fileSub.addItem({"new_proj", "New Project", MenuItemKind::Action, true, "Ctrl+N", ""});
    fileSub.addItem({"open_proj","Open Project",MenuItemKind::Action, true, "Ctrl+O", ""});

    parent.attachSubmenu(fileSub);
    m.registerMenu(parent);

    const ContextMenu* found = m.findMenu("parent");
    CHECK(found != nullptr);
    CHECK(found->findSubmenu("file_sub") != nullptr);
    CHECK(found->findSubmenu("file_sub")->contains("new_proj"));
}

TEST_CASE("Opening second menu auto-closes first with lifecycle events", "[Phase26][Integration]") {
    ContextMenuManager m;
    m.registerMenu(ContextMenu("m1"));
    m.registerMenu(ContextMenu("m2"));

    std::vector<std::pair<std::string,bool>> events;
    m.addLifecycleObserver([&](const std::string& id, bool opened) { events.push_back({id, opened}); });

    m.openMenu("m1");
    m.openMenu("m2");

    // m1 opened, m1 closed (by m2 open), m2 opened
    CHECK(events.size() == 3);
    CHECK(events[0] == std::make_pair(std::string("m1"), true));
    CHECK(events[1] == std::make_pair(std::string("m1"), false));
    CHECK(events[2] == std::make_pair(std::string("m2"), true));
}

TEST_CASE("clearObservers stops all notifications", "[Phase26][Integration]") {
    ContextMenuManager m;
    m.registerMenu(ContextMenu("m1"));
    int calls = 0;
    m.addLifecycleObserver([&](const std::string&, bool) { ++calls; });
    m.clearObservers();
    m.openMenu("m1");
    CHECK(calls == 0);
}
