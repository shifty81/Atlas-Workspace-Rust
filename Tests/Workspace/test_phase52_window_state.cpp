// Tests/Workspace/test_phase52_window_state.cpp
// Phase 52 — WorkspaceWindowState: WindowBounds, MonitorInfo,
//             WindowStateEntry, WindowStateManager
#include <catch2/catch_test_macros.hpp>
#include "NF/Workspace/WorkspaceWindowState.h"

// ═════════════════════════════════════════════════════════════════
// WindowBounds
// ═════════════════════════════════════════════════════════════════

TEST_CASE("WindowBounds: default invalid", "[windowstate][bounds]") {
    NF::WindowBounds b;
    REQUIRE_FALSE(b.isValid());
    REQUIRE(b.width  == 0);
    REQUIRE(b.height == 0);
}

TEST_CASE("WindowBounds: valid with positive dimensions", "[windowstate][bounds]") {
    NF::WindowBounds b;
    b.x = 100; b.y = 50; b.width = 1280; b.height = 720;
    REQUIRE(b.isValid());
}

TEST_CASE("WindowBounds: zero width or height invalid", "[windowstate][bounds]") {
    NF::WindowBounds b;
    b.width = 0; b.height = 720;
    REQUIRE_FALSE(b.isValid());
    b.width = 1280; b.height = 0;
    REQUIRE_FALSE(b.isValid());
}

TEST_CASE("WindowBounds: contains point", "[windowstate][bounds]") {
    NF::WindowBounds b;
    b.x = 10; b.y = 20; b.width = 100; b.height = 80;
    REQUIRE(b.contains(10, 20));   // top-left corner
    REQUIRE(b.contains(60, 60));   // inside
    REQUIRE(b.contains(109, 99));  // just inside
    REQUIRE_FALSE(b.contains(110, 60));  // right edge (exclusive)
    REQUIRE_FALSE(b.contains(60, 100)); // bottom edge (exclusive)
    REQUIRE_FALSE(b.contains(9, 60));   // left of
}

TEST_CASE("WindowBounds: equality", "[windowstate][bounds]") {
    NF::WindowBounds a, b;
    a.x = 0; a.y = 0; a.width = 100; a.height = 100;
    b = a;
    REQUIRE(a == b);
    b.width = 200;
    REQUIRE(a != b);
}

TEST_CASE("WindowBounds: maximized and minimized flags", "[windowstate][bounds]") {
    NF::WindowBounds b;
    b.width = 1; b.height = 1;
    b.isMaximized = true;
    REQUIRE(b.isValid());
    REQUIRE(b.isMaximized);
    REQUIRE_FALSE(b.isMinimized);
}

// ═════════════════════════════════════════════════════════════════
// MonitorInfo
// ═════════════════════════════════════════════════════════════════

static NF::MonitorInfo makeMonitor(const std::string& id, int x, int y, int w, int h,
                                    bool primary = false) {
    NF::MonitorInfo m;
    m.id = id; m.name = id;
    m.bounds.x = x; m.bounds.y = y; m.bounds.width = w; m.bounds.height = h;
    m.isPrimary = primary;
    return m;
}

TEST_CASE("MonitorInfo: default invalid", "[windowstate][monitor]") {
    NF::MonitorInfo m;
    REQUIRE_FALSE(m.isValid());
}

TEST_CASE("MonitorInfo: valid with id and bounds", "[windowstate][monitor]") {
    auto m = makeMonitor("primary", 0, 0, 1920, 1080, true);
    REQUIRE(m.isValid());
    REQUIRE(m.isPrimary);
    REQUIRE(m.scaleFactor == 1.0f);
}

// ═════════════════════════════════════════════════════════════════
// WindowStateEntry
// ═════════════════════════════════════════════════════════════════

static NF::WindowStateEntry makeEntry(const std::string& id, int x, int y, int w, int h,
                                       const std::string& monId = "") {
    NF::WindowStateEntry e;
    e.windowId = id;
    e.bounds.x = x; e.bounds.y = y; e.bounds.width = w; e.bounds.height = h;
    e.monitorId = monId;
    return e;
}

TEST_CASE("WindowStateEntry: default invalid", "[windowstate][entry]") {
    NF::WindowStateEntry e;
    REQUIRE_FALSE(e.isValid());
}

TEST_CASE("WindowStateEntry: valid with id and bounds", "[windowstate][entry]") {
    auto e = makeEntry("main", 100, 100, 800, 600);
    REQUIRE(e.isValid());
}

TEST_CASE("WindowStateEntry: invalid without window id", "[windowstate][entry]") {
    NF::WindowStateEntry e;
    e.bounds.width = 800; e.bounds.height = 600;
    REQUIRE_FALSE(e.isValid());
}

// ═════════════════════════════════════════════════════════════════
// WindowStateManager
// ═════════════════════════════════════════════════════════════════

TEST_CASE("WindowStateManager: default empty", "[windowstate][manager]") {
    NF::WindowStateManager mgr;
    REQUIRE(mgr.monitorCount() == 0);
    REQUIRE(mgr.entryCount()   == 0);
    REQUIRE(mgr.primaryMonitor() == nullptr);
}

TEST_CASE("WindowStateManager: addMonitor valid", "[windowstate][manager]") {
    NF::WindowStateManager mgr;
    REQUIRE(mgr.addMonitor(makeMonitor("m1", 0, 0, 1920, 1080, true)));
    REQUIRE(mgr.monitorCount() == 1);
    REQUIRE(mgr.findMonitor("m1") != nullptr);
    REQUIRE(mgr.primaryMonitor() != nullptr);
    REQUIRE(mgr.primaryMonitor()->id == "m1");
}

TEST_CASE("WindowStateManager: addMonitor invalid rejected", "[windowstate][manager]") {
    NF::WindowStateManager mgr;
    NF::MonitorInfo bad; // no id, no bounds
    REQUIRE_FALSE(mgr.addMonitor(bad));
}

TEST_CASE("WindowStateManager: addMonitor duplicate rejected", "[windowstate][manager]") {
    NF::WindowStateManager mgr;
    mgr.addMonitor(makeMonitor("m1", 0, 0, 1920, 1080));
    REQUIRE_FALSE(mgr.addMonitor(makeMonitor("m1", 0, 0, 2560, 1440)));
    REQUIRE(mgr.monitorCount() == 1);
}

TEST_CASE("WindowStateManager: only one primary monitor", "[windowstate][manager]") {
    NF::WindowStateManager mgr;
    mgr.addMonitor(makeMonitor("m1", 0,    0, 1920, 1080, true));
    mgr.addMonitor(makeMonitor("m2", 1920, 0, 2560, 1440, true)); // also primary
    // m1 should have been cleared of primary flag
    REQUIRE(mgr.findMonitor("m1")->isPrimary == false);
    REQUIRE(mgr.findMonitor("m2")->isPrimary == true);
    REQUIRE(mgr.primaryMonitor()->id == "m2");
}

TEST_CASE("WindowStateManager: removeMonitor", "[windowstate][manager]") {
    NF::WindowStateManager mgr;
    mgr.addMonitor(makeMonitor("m1", 0, 0, 1920, 1080));
    REQUIRE(mgr.removeMonitor("m1"));
    REQUIRE(mgr.monitorCount() == 0);
}

TEST_CASE("WindowStateManager: removeMonitor unknown returns false", "[windowstate][manager]") {
    NF::WindowStateManager mgr;
    REQUIRE_FALSE(mgr.removeMonitor("no.such"));
}

TEST_CASE("WindowStateManager: save new entry", "[windowstate][manager]") {
    NF::WindowStateManager mgr;
    REQUIRE(mgr.save(makeEntry("main", 0, 0, 1280, 720)));
    REQUIRE(mgr.entryCount() == 1);
    REQUIRE(mgr.has("main"));
}

TEST_CASE("WindowStateManager: save invalid entry returns false", "[windowstate][manager]") {
    NF::WindowStateManager mgr;
    NF::WindowStateEntry bad;
    REQUIRE_FALSE(mgr.save(bad));
}

TEST_CASE("WindowStateManager: save updates existing entry", "[windowstate][manager]") {
    NF::WindowStateManager mgr;
    mgr.save(makeEntry("main", 0, 0, 800, 600));
    auto updated = makeEntry("main", 100, 100, 1280, 720);
    REQUIRE(mgr.save(updated));
    REQUIRE(mgr.entryCount() == 1); // no duplicate
    REQUIRE(mgr.restore("main")->bounds.width == 1280);
}

TEST_CASE("WindowStateManager: restore by windowId", "[windowstate][manager]") {
    NF::WindowStateManager mgr;
    mgr.save(makeEntry("main", 10, 20, 800, 600));
    const auto* e = mgr.restore("main");
    REQUIRE(e != nullptr);
    REQUIRE(e->bounds.x == 10);
    REQUIRE(e->bounds.y == 20);
}

TEST_CASE("WindowStateManager: restore unknown returns nullptr", "[windowstate][manager]") {
    NF::WindowStateManager mgr;
    REQUIRE(mgr.restore("no.such") == nullptr);
}

TEST_CASE("WindowStateManager: remove entry", "[windowstate][manager]") {
    NF::WindowStateManager mgr;
    mgr.save(makeEntry("main", 0, 0, 800, 600));
    REQUIRE(mgr.remove("main"));
    REQUIRE(mgr.entryCount() == 0);
}

TEST_CASE("WindowStateManager: remove unknown returns false", "[windowstate][manager]") {
    NF::WindowStateManager mgr;
    REQUIRE_FALSE(mgr.remove("no.such"));
}

TEST_CASE("WindowStateManager: isOnMonitor true when center overlaps", "[windowstate][manager]") {
    NF::WindowStateManager mgr;
    mgr.addMonitor(makeMonitor("m1", 0, 0, 1920, 1080));
    auto entry = makeEntry("main", 100, 100, 800, 600, "m1"); // center=(500,400)
    REQUIRE(mgr.isOnMonitor(entry, "m1"));
}

TEST_CASE("WindowStateManager: isOnMonitor false when center outside", "[windowstate][manager]") {
    NF::WindowStateManager mgr;
    mgr.addMonitor(makeMonitor("m1", 0, 0, 1920, 1080));
    // Window positioned entirely off the monitor
    auto entry = makeEntry("main", 2000, 0, 800, 600, "m1"); // center=(2400,300)
    REQUIRE_FALSE(mgr.isOnMonitor(entry, "m1"));
}

TEST_CASE("WindowStateManager: monitorForEntry returns primary as fallback", "[windowstate][manager]") {
    NF::WindowStateManager mgr;
    mgr.addMonitor(makeMonitor("primary", 0, 0, 1920, 1080, true));
    // Entry is off all monitors
    auto entry = makeEntry("win", 5000, 5000, 100, 100);
    const auto* mon = mgr.monitorForEntry(entry);
    REQUIRE(mon != nullptr);
    REQUIRE(mon->id == "primary");
}

TEST_CASE("WindowStateManager: observer fires on save", "[windowstate][manager]") {
    NF::WindowStateManager mgr;
    int count = 0;
    mgr.addObserver([&]{ ++count; });
    mgr.save(makeEntry("main", 0, 0, 800, 600));
    REQUIRE(count == 1);
}

TEST_CASE("WindowStateManager: observer fires on remove", "[windowstate][manager]") {
    NF::WindowStateManager mgr;
    mgr.save(makeEntry("main", 0, 0, 800, 600));
    int count = 0;
    mgr.addObserver([&]{ ++count; });
    mgr.remove("main");
    REQUIRE(count == 1);
}

TEST_CASE("WindowStateManager: clearObservers stops notifications", "[windowstate][manager]") {
    NF::WindowStateManager mgr;
    int count = 0;
    mgr.addObserver([&]{ ++count; });
    mgr.save(makeEntry("a", 0, 0, 100, 100));
    REQUIRE(count == 1);
    mgr.clearObservers();
    mgr.save(makeEntry("b", 0, 0, 200, 200));
    REQUIRE(count == 1);
}

TEST_CASE("WindowStateManager: serialize empty is empty string", "[windowstate][manager]") {
    NF::WindowStateManager mgr;
    REQUIRE(mgr.serialize().empty());
}

TEST_CASE("WindowStateManager: serialize round-trip", "[windowstate][manager]") {
    NF::WindowStateManager mgr;
    auto e1 = makeEntry("main", 100, 50, 1280, 720, "mon1");
    e1.workspaceId = "project.alpha";
    e1.lastSavedMs = 99999;
    e1.bounds.isMaximized = true;
    mgr.save(e1);

    auto e2 = makeEntry("floater", 200, 200, 400, 300);
    mgr.save(e2);

    std::string text = mgr.serialize();
    REQUIRE_FALSE(text.empty());

    NF::WindowStateManager mgr2;
    int loaded = mgr2.deserialize(text);
    REQUIRE(loaded == 2);

    const auto* r1 = mgr2.restore("main");
    REQUIRE(r1 != nullptr);
    REQUIRE(r1->bounds.x         == 100);
    REQUIRE(r1->bounds.width     == 1280);
    REQUIRE(r1->bounds.isMaximized);
    REQUIRE(r1->monitorId        == "mon1");
    REQUIRE(r1->workspaceId      == "project.alpha");
    REQUIRE(r1->lastSavedMs      == 99999);

    const auto* r2 = mgr2.restore("floater");
    REQUIRE(r2 != nullptr);
    REQUIRE(r2->bounds.width == 400);
}

TEST_CASE("WindowStateManager: deserialize empty returns 0", "[windowstate][manager]") {
    NF::WindowStateManager mgr;
    REQUIRE(mgr.deserialize("") == 0);
}

TEST_CASE("WindowStateManager: deserialize clears existing entries", "[windowstate][manager]") {
    NF::WindowStateManager mgr;
    mgr.save(makeEntry("old", 0, 0, 100, 100));

    NF::WindowStateManager src;
    src.save(makeEntry("new", 0, 0, 200, 200));
    std::string text = src.serialize();

    mgr.deserialize(text);
    REQUIRE(mgr.restore("old") == nullptr);
    REQUIRE(mgr.restore("new") != nullptr);
}

TEST_CASE("WindowStateManager: clear removes all", "[windowstate][manager]") {
    NF::WindowStateManager mgr;
    mgr.addMonitor(makeMonitor("m1", 0, 0, 1920, 1080));
    mgr.save(makeEntry("main", 0, 0, 800, 600));
    mgr.clear();
    REQUIRE(mgr.monitorCount() == 0);
    REQUIRE(mgr.entryCount()   == 0);
}

// ═════════════════════════════════════════════════════════════════
// Integration
// ═════════════════════════════════════════════════════════════════

TEST_CASE("WindowState integration: multi-monitor layout save and restore", "[windowstate][integration]") {
    NF::WindowStateManager mgr;
    mgr.addMonitor(makeMonitor("primary",   0,    0, 1920, 1080, true));
    mgr.addMonitor(makeMonitor("secondary", 1920, 0, 2560, 1440, false));

    // Main window on primary monitor
    auto main = makeEntry("main", 200, 100, 1500, 900, "primary");
    main.workspaceId = "workspace.alpha";
    mgr.save(main);

    // Inspector on secondary monitor
    auto inspector = makeEntry("inspector", 2100, 50, 600, 1300, "secondary");
    mgr.save(inspector);

    REQUIRE(mgr.entryCount() == 2);
    REQUIRE(mgr.isOnMonitor(*mgr.restore("main"),      "primary"));
    REQUIRE(mgr.isOnMonitor(*mgr.restore("inspector"), "secondary"));
    REQUIRE_FALSE(mgr.isOnMonitor(*mgr.restore("main"), "secondary"));

    // Serialize and restore
    std::string state = mgr.serialize();
    NF::WindowStateManager mgr2;
    int loaded = mgr2.deserialize(state);
    REQUIRE(loaded == 2);
    REQUIRE(mgr2.restore("main")->workspaceId == "workspace.alpha");
}

TEST_CASE("WindowState integration: orphaned window falls back to primary monitor", "[windowstate][integration]") {
    NF::WindowStateManager mgr;
    mgr.addMonitor(makeMonitor("primary", 0, 0, 1920, 1080, true));

    // Window saved on monitor that no longer exists
    auto orphan = makeEntry("tool", 5000, 5000, 400, 300, "missing_monitor");
    mgr.save(orphan);

    const auto* mon = mgr.monitorForEntry(*mgr.restore("tool"));
    REQUIRE(mon != nullptr);
    REQUIRE(mon->id == "primary"); // falls back to primary
}
