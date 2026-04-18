// Tests/Workspace/test_phase27_badge.cpp
// Phase 27 — Workspace Badge and Icon Registry
//
// Tests for:
//   1. BadgeKind      — enum name helpers
//   2. Badge          — overlay descriptor; isValid, equality
//   3. BadgeRegistry  — attach/detach/update with target and kind queries
//   4. IconEntry      — icon asset descriptor; isValid, equality
//   5. IconRegistry   — register/find by id, alias, category
//   6. Integration    — combined badge + icon pipeline

#include <catch2/catch_test_macros.hpp>
#include "NF/Workspace/WorkspaceBadge.h"
#include <string>
#include <vector>

using namespace NF;

// ═════════════════════════════════════════════════════════════════
// SECTION 1 — BadgeKind enum
// ═════════════════════════════════════════════════════════════════

TEST_CASE("badgeKindName returns correct strings", "[Phase27][BadgeKind]") {
    CHECK(std::string(badgeKindName(BadgeKind::Info))    == "Info");
    CHECK(std::string(badgeKindName(BadgeKind::Warning)) == "Warning");
    CHECK(std::string(badgeKindName(BadgeKind::Error))   == "Error");
    CHECK(std::string(badgeKindName(BadgeKind::Success)) == "Success");
    CHECK(std::string(badgeKindName(BadgeKind::Count))   == "Count");
    CHECK(std::string(badgeKindName(BadgeKind::Custom))  == "Custom");
}

// ═════════════════════════════════════════════════════════════════
// SECTION 2 — Badge
// ═════════════════════════════════════════════════════════════════

TEST_CASE("Badge default is invalid", "[Phase27][Badge]") {
    Badge b;
    CHECK_FALSE(b.isValid());
    CHECK(b.id.empty());
    CHECK(b.targetId.empty());
    CHECK(b.kind == BadgeKind::Info);
    CHECK(b.visible);
    CHECK(b.count == 0);
}

TEST_CASE("Badge valid construction", "[Phase27][Badge]") {
    Badge b{"badge_1", "panel_outliner", BadgeKind::Warning, "!", 0, true};
    CHECK(b.isValid());
    CHECK(b.id       == "badge_1");
    CHECK(b.targetId == "panel_outliner");
    CHECK(b.kind     == BadgeKind::Warning);
    CHECK(b.label    == "!");
}

TEST_CASE("Badge invalid without id", "[Phase27][Badge]") {
    Badge b{"", "target", BadgeKind::Info, "", 0, true};
    CHECK_FALSE(b.isValid());
}

TEST_CASE("Badge invalid without targetId", "[Phase27][Badge]") {
    Badge b{"b1", "", BadgeKind::Info, "", 0, true};
    CHECK_FALSE(b.isValid());
}

TEST_CASE("Badge equality by id", "[Phase27][Badge]") {
    Badge a{"b1", "t1", BadgeKind::Info,    "A", 0, true};
    Badge b{"b1", "t2", BadgeKind::Warning, "B", 5, false};
    Badge c{"b2", "t1", BadgeKind::Info,    "A", 0, true};
    CHECK(a == b);
    CHECK(a != c);
}

TEST_CASE("Badge Count kind with numeric count", "[Phase27][Badge]") {
    Badge b{"notif_badge", "toolbar_notif", BadgeKind::Count, "", 42, true};
    CHECK(b.isValid());
    CHECK(b.kind  == BadgeKind::Count);
    CHECK(b.count == 42);
}

// ═════════════════════════════════════════════════════════════════
// SECTION 3 — BadgeRegistry
// ═════════════════════════════════════════════════════════════════

TEST_CASE("BadgeRegistry empty state", "[Phase27][BadgeRegistry]") {
    BadgeRegistry r;
    CHECK(r.empty());
    CHECK(r.totalCount() == 0);
}

TEST_CASE("BadgeRegistry attach", "[Phase27][BadgeRegistry]") {
    BadgeRegistry r;
    Badge b{"b1", "panel_a", BadgeKind::Info, "i", 0, true};
    CHECK(r.attach(b));
    CHECK(r.isAttached("b1"));
    CHECK(r.totalCount() == 1);
}

TEST_CASE("BadgeRegistry duplicate attach fails", "[Phase27][BadgeRegistry]") {
    BadgeRegistry r;
    Badge b{"b1", "panel_a", BadgeKind::Info, "i", 0, true};
    CHECK(r.attach(b));
    CHECK_FALSE(r.attach(b));
}

TEST_CASE("BadgeRegistry invalid badge rejected", "[Phase27][BadgeRegistry]") {
    BadgeRegistry r;
    Badge b;  // invalid
    CHECK_FALSE(r.attach(b));
}

TEST_CASE("BadgeRegistry detach", "[Phase27][BadgeRegistry]") {
    BadgeRegistry r;
    Badge b{"b1", "panel_a", BadgeKind::Error, "!", 0, true};
    r.attach(b);
    CHECK(r.detach("b1"));
    CHECK_FALSE(r.isAttached("b1"));
}

TEST_CASE("BadgeRegistry detach unknown fails", "[Phase27][BadgeRegistry]") {
    BadgeRegistry r;
    CHECK_FALSE(r.detach("nope"));
}

TEST_CASE("BadgeRegistry update badge", "[Phase27][BadgeRegistry]") {
    BadgeRegistry r;
    Badge b{"b1", "target", BadgeKind::Info, "i", 0, true};
    r.attach(b);
    Badge updated{"b1", "target", BadgeKind::Warning, "!", 0, true};
    CHECK(r.update(updated));
    CHECK(r.findById("b1")->kind == BadgeKind::Warning);
}

TEST_CASE("BadgeRegistry update unknown fails", "[Phase27][BadgeRegistry]") {
    BadgeRegistry r;
    Badge b{"b1", "t", BadgeKind::Info, "", 0, true};
    CHECK_FALSE(r.update(b));
}

TEST_CASE("BadgeRegistry findByTarget", "[Phase27][BadgeRegistry]") {
    BadgeRegistry r;
    r.attach({"b1", "panel_a", BadgeKind::Info,    "", 0, true});
    r.attach({"b2", "panel_a", BadgeKind::Warning, "", 0, true});
    r.attach({"b3", "panel_b", BadgeKind::Error,   "", 0, true});
    auto results = r.findByTarget("panel_a");
    CHECK(results.size() == 2);
}

TEST_CASE("BadgeRegistry findByKind", "[Phase27][BadgeRegistry]") {
    BadgeRegistry r;
    r.attach({"b1", "t1", BadgeKind::Error, "", 0, true});
    r.attach({"b2", "t2", BadgeKind::Error, "", 0, true});
    r.attach({"b3", "t3", BadgeKind::Info,  "", 0, true});
    auto errors = r.findByKind(BadgeKind::Error);
    CHECK(errors.size() == 2);
}

TEST_CASE("BadgeRegistry setVisible", "[Phase27][BadgeRegistry]") {
    BadgeRegistry r;
    r.attach({"b1", "t", BadgeKind::Info, "", 0, true});
    CHECK(r.setVisible("b1", false));
    CHECK_FALSE(r.findById("b1")->visible);
    CHECK(r.setVisible("b1", true));
    CHECK(r.findById("b1")->visible);
}

TEST_CASE("BadgeRegistry setVisible unknown fails", "[Phase27][BadgeRegistry]") {
    BadgeRegistry r;
    CHECK_FALSE(r.setVisible("nope", false));
}

TEST_CASE("BadgeRegistry setCount", "[Phase27][BadgeRegistry]") {
    BadgeRegistry r;
    r.attach({"b1", "t", BadgeKind::Count, "", 5, true});
    CHECK(r.setCount("b1", 99));
    CHECK(r.findById("b1")->count == 99);
}

TEST_CASE("BadgeRegistry setCount non-Count kind fails", "[Phase27][BadgeRegistry]") {
    BadgeRegistry r;
    r.attach({"b1", "t", BadgeKind::Info, "", 0, true});
    CHECK_FALSE(r.setCount("b1", 5));
}

TEST_CASE("BadgeRegistry observer fires on attach", "[Phase27][BadgeRegistry]") {
    BadgeRegistry r;
    std::string lastId; bool lastAttached = false;
    r.addObserver([&](const Badge& b, bool att) { lastId = b.id; lastAttached = att; });
    r.attach({"b1", "t", BadgeKind::Info, "", 0, true});
    CHECK(lastId == "b1");
    CHECK(lastAttached);
}

TEST_CASE("BadgeRegistry observer fires on detach", "[Phase27][BadgeRegistry]") {
    BadgeRegistry r;
    bool lastAttached = true;
    r.addObserver([&](const Badge&, bool att) { lastAttached = att; });
    r.attach({"b1", "t", BadgeKind::Info, "", 0, true});
    r.detach("b1");
    CHECK_FALSE(lastAttached);
}

TEST_CASE("BadgeRegistry removeObserver stops notifications", "[Phase27][BadgeRegistry]") {
    BadgeRegistry r;
    int calls = 0;
    uint32_t id = r.addObserver([&](const Badge&, bool) { ++calls; });
    r.removeObserver(id);
    r.attach({"b1", "t", BadgeKind::Info, "", 0, true});
    CHECK(calls == 0);
}

TEST_CASE("BadgeRegistry clear empties all badges", "[Phase27][BadgeRegistry]") {
    BadgeRegistry r;
    r.attach({"b1", "t", BadgeKind::Info, "", 0, true});
    r.attach({"b2", "t", BadgeKind::Error,"", 0, true});
    r.clear();
    CHECK(r.empty());
}

// ═════════════════════════════════════════════════════════════════
// SECTION 4 — IconEntry
// ═════════════════════════════════════════════════════════════════

TEST_CASE("IconEntry default is invalid", "[Phase27][IconEntry]") {
    IconEntry e;
    CHECK_FALSE(e.isValid());
    CHECK(e.id.empty());
    CHECK(e.path.empty());
    CHECK(e.size == 16);
}

TEST_CASE("IconEntry valid construction", "[Phase27][IconEntry]") {
    IconEntry e{"ico_save", "icons/save.png", "save", "toolbar", 24};
    CHECK(e.isValid());
    CHECK(e.id       == "ico_save");
    CHECK(e.path     == "icons/save.png");
    CHECK(e.alias    == "save");
    CHECK(e.category == "toolbar");
    CHECK(e.size     == 24);
}

TEST_CASE("IconEntry invalid without id", "[Phase27][IconEntry]") {
    IconEntry e{"", "path/icon.png", "", "", 16};
    CHECK_FALSE(e.isValid());
}

TEST_CASE("IconEntry invalid without path", "[Phase27][IconEntry]") {
    IconEntry e{"ico_1", "", "", "", 16};
    CHECK_FALSE(e.isValid());
}

TEST_CASE("IconEntry equality by id", "[Phase27][IconEntry]") {
    IconEntry a{"ico_1", "a.png", "a", "ui", 16};
    IconEntry b{"ico_1", "b.png", "b", "tb", 24};
    IconEntry c{"ico_2", "a.png", "a", "ui", 16};
    CHECK(a == b);
    CHECK(a != c);
}

// ═════════════════════════════════════════════════════════════════
// SECTION 5 — IconRegistry
// ═════════════════════════════════════════════════════════════════

TEST_CASE("IconRegistry empty state", "[Phase27][IconRegistry]") {
    IconRegistry r;
    CHECK(r.empty());
    CHECK(r.count() == 0);
}

TEST_CASE("IconRegistry registerIcon", "[Phase27][IconRegistry]") {
    IconRegistry r;
    IconEntry e{"ico_1", "icons/1.png", "", "ui", 16};
    CHECK(r.registerIcon(e));
    CHECK(r.isRegistered("ico_1"));
    CHECK(r.count() == 1);
}

TEST_CASE("IconRegistry duplicate register fails", "[Phase27][IconRegistry]") {
    IconRegistry r;
    IconEntry e{"ico_1", "icons/1.png", "", "ui", 16};
    CHECK(r.registerIcon(e));
    CHECK_FALSE(r.registerIcon(e));
}

TEST_CASE("IconRegistry invalid entry rejected", "[Phase27][IconRegistry]") {
    IconRegistry r;
    IconEntry e;  // invalid
    CHECK_FALSE(r.registerIcon(e));
}

TEST_CASE("IconRegistry unregisterIcon", "[Phase27][IconRegistry]") {
    IconRegistry r;
    r.registerIcon({"ico_1", "p.png", "", "ui", 16});
    CHECK(r.unregisterIcon("ico_1"));
    CHECK_FALSE(r.isRegistered("ico_1"));
}

TEST_CASE("IconRegistry unregister unknown fails", "[Phase27][IconRegistry]") {
    IconRegistry r;
    CHECK_FALSE(r.unregisterIcon("nope"));
}

TEST_CASE("IconRegistry findById", "[Phase27][IconRegistry]") {
    IconRegistry r;
    r.registerIcon({"ico_1", "icons/file.png", "", "editor", 32});
    auto* found = r.findById("ico_1");
    CHECK(found != nullptr);
    CHECK(found->path == "icons/file.png");
    CHECK(found->size == 32);
}

TEST_CASE("IconRegistry findByAlias", "[Phase27][IconRegistry]") {
    IconRegistry r;
    r.registerIcon({"ico_save", "icons/save.png", "save", "toolbar", 16});
    auto* found = r.findByAlias("save");
    CHECK(found != nullptr);
    CHECK(found->id == "ico_save");
}

TEST_CASE("IconRegistry find by id or alias", "[Phase27][IconRegistry]") {
    IconRegistry r;
    r.registerIcon({"ico_open", "icons/open.png", "open", "toolbar", 16});
    CHECK(r.find("ico_open") != nullptr);
    CHECK(r.find("open")     != nullptr);
    CHECK(r.find("nope")     == nullptr);
}

TEST_CASE("IconRegistry findByCategory", "[Phase27][IconRegistry]") {
    IconRegistry r;
    r.registerIcon({"i1", "a.png", "", "toolbar", 16});
    r.registerIcon({"i2", "b.png", "", "toolbar", 16});
    r.registerIcon({"i3", "c.png", "", "editor",  16});
    auto results = r.findByCategory("toolbar");
    CHECK(results.size() == 2);
    auto editorResults = r.findByCategory("editor");
    CHECK(editorResults.size() == 1);
}

TEST_CASE("IconRegistry allIds", "[Phase27][IconRegistry]") {
    IconRegistry r;
    r.registerIcon({"i1", "a.png", "", "ui", 16});
    r.registerIcon({"i2", "b.png", "", "ui", 16});
    r.registerIcon({"i3", "c.png", "", "ui", 16});
    CHECK(r.allIds().size() == 3);
}

TEST_CASE("IconRegistry clear", "[Phase27][IconRegistry]") {
    IconRegistry r;
    r.registerIcon({"i1", "a.png", "", "ui", 16});
    r.clear();
    CHECK(r.empty());
}

// ═════════════════════════════════════════════════════════════════
// SECTION 6 — Integration
// ═════════════════════════════════════════════════════════════════

TEST_CASE("Full badge pipeline: attach → setCount → detach → observer", "[Phase27][Integration]") {
    BadgeRegistry r;

    std::vector<std::pair<std::string,bool>> events;
    r.addObserver([&](const Badge& b, bool att) { events.push_back({b.id, att}); });

    r.attach({"notif_count", "toolbar_notif", BadgeKind::Count, "", 0, true});
    r.setCount("notif_count", 5);
    r.setCount("notif_count", 12);
    r.detach("notif_count");

    // attach(1) + setCount×2 updates(2) + detach(1) = 4 events
    CHECK(events.size() == 4);
    CHECK(events[0].second == true);   // attach
    CHECK(events[3].second == false);  // detach
}

TEST_CASE("Multi-target badge queries", "[Phase27][Integration]") {
    BadgeRegistry r;
    r.attach({"e1", "outliner",    BadgeKind::Error,   "!", 0, true});
    r.attach({"e2", "outliner",    BadgeKind::Warning, "?", 0, true});
    r.attach({"e3", "content_browser", BadgeKind::Error, "!", 0, true});

    auto outErrors = r.findByTarget("outliner");
    CHECK(outErrors.size() == 2);

    auto allErrors = r.findByKind(BadgeKind::Error);
    CHECK(allErrors.size() == 2);

    auto allWarnings = r.findByKind(BadgeKind::Warning);
    CHECK(allWarnings.size() == 1);
}

TEST_CASE("Icon registry: alias lookup enables short names", "[Phase27][Integration]") {
    IconRegistry icons;
    icons.registerIcon({"ico_folder_open",  "icons/folder_open.png",  "folder",  "ui",      16});
    icons.registerIcon({"ico_file_new",     "icons/file_new.png",     "new",     "toolbar", 24});
    icons.registerIcon({"ico_save",         "icons/save.png",         "save",    "toolbar", 24});
    icons.registerIcon({"ico_settings",     "icons/settings.png",     "settings","menu",    16});

    // By alias
    CHECK(icons.find("folder")   != nullptr);
    CHECK(icons.find("save")     != nullptr);
    CHECK(icons.find("new")      != nullptr);
    CHECK(icons.find("settings") != nullptr);

    auto toolbarIcons = icons.findByCategory("toolbar");
    CHECK(toolbarIcons.size() == 2);
}

TEST_CASE("clearObservers stops all badge notifications", "[Phase27][Integration]") {
    BadgeRegistry r;
    int calls = 0;
    r.addObserver([&](const Badge&, bool) { ++calls; });
    r.clearObservers();
    r.attach({"b1", "t", BadgeKind::Info, "", 0, true});
    CHECK(calls == 0);
}

TEST_CASE("Multiple badge observers all fire", "[Phase27][Integration]") {
    BadgeRegistry r;
    int c1 = 0, c2 = 0;
    r.addObserver([&](const Badge&, bool) { ++c1; });
    r.addObserver([&](const Badge&, bool) { ++c2; });
    r.attach({"b1", "t", BadgeKind::Error, "!", 0, true});
    CHECK(c1 == 1);
    CHECK(c2 == 1);
}
