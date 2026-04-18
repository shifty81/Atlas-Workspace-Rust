// Tests/Workspace/test_phase49_recent_files.cpp
// Phase 49 — WorkspaceRecentFiles: RecentFileKind, RecentFileEntry,
//             RecentFileList, RecentFilesManager
#include <catch2/catch_test_macros.hpp>
#include "NF/Workspace/WorkspaceRecentFiles.h"

// ═════════════════════════════════════════════════════════════════
// RecentFileKind enum
// ═════════════════════════════════════════════════════════════════

TEST_CASE("RecentFileKind: name helpers", "[recentfiles][kind]") {
    REQUIRE(std::string(NF::recentFileKindName(NF::RecentFileKind::Project)) == "Project");
    REQUIRE(std::string(NF::recentFileKindName(NF::RecentFileKind::Scene))   == "Scene");
    REQUIRE(std::string(NF::recentFileKindName(NF::RecentFileKind::Asset))   == "Asset");
    REQUIRE(std::string(NF::recentFileKindName(NF::RecentFileKind::Script))  == "Script");
    REQUIRE(std::string(NF::recentFileKindName(NF::RecentFileKind::Config))  == "Config");
    REQUIRE(std::string(NF::recentFileKindName(NF::RecentFileKind::Custom))  == "Custom");
}

// ═════════════════════════════════════════════════════════════════
// RecentFileEntry
// ═════════════════════════════════════════════════════════════════

TEST_CASE("RecentFileEntry: default invalid", "[recentfiles][entry]") {
    NF::RecentFileEntry e;
    REQUIRE_FALSE(e.isValid());
    REQUIRE(e.path.empty());
    REQUIRE_FALSE(e.pinned);
    REQUIRE(e.accessCount == 0);
}

TEST_CASE("RecentFileEntry: valid when path set", "[recentfiles][entry]") {
    NF::RecentFileEntry e;
    e.path        = "projects/demo.atlasproject";
    e.displayName = "Demo Project";
    e.kind        = NF::RecentFileKind::Project;
    REQUIRE(e.isValid());
}

TEST_CASE("RecentFileEntry: equality by path", "[recentfiles][entry]") {
    NF::RecentFileEntry a, b, c;
    a.path = "foo/bar.txt";
    b.path = "foo/bar.txt";
    c.path = "baz/qux.txt";
    REQUIRE(a == b);
    REQUIRE(a != c);
}

// ═════════════════════════════════════════════════════════════════
// RecentFileList
// ═════════════════════════════════════════════════════════════════

TEST_CASE("RecentFileList: default empty state", "[recentfiles][list]") {
    NF::RecentFileList list;
    REQUIRE(list.empty());
    REQUIRE(list.count() == 0);
    REQUIRE(list.mostRecent() == nullptr);
    REQUIRE(list.all().empty());
}

TEST_CASE("RecentFileList: record adds entry to front", "[recentfiles][list]") {
    NF::RecentFileList list;
    REQUIRE(list.record("proj/a.atlasproject", "Project A",
                        NF::RecentFileKind::Project, 1000));
    REQUIRE(list.count() == 1);
    REQUIRE_FALSE(list.empty());
    REQUIRE(list.mostRecent()->path == "proj/a.atlasproject");
    REQUIRE(list.mostRecent()->displayName == "Project A");
    REQUIRE(list.mostRecent()->accessCount == 1);
}

TEST_CASE("RecentFileList: record empty path returns false", "[recentfiles][list]") {
    NF::RecentFileList list;
    REQUIRE_FALSE(list.record("", "Bad", NF::RecentFileKind::Asset, 100));
    REQUIRE(list.empty());
}

TEST_CASE("RecentFileList: record existing path moves to front and bumps count", "[recentfiles][list]") {
    NF::RecentFileList list;
    list.record("a.txt", "A", NF::RecentFileKind::Script, 1000);
    list.record("b.txt", "B", NF::RecentFileKind::Script, 2000);
    REQUIRE(list.mostRecent()->path == "b.txt");
    // Re-record a.txt
    list.record("a.txt", "A Updated", NF::RecentFileKind::Script, 3000);
    REQUIRE(list.count() == 2);
    REQUIRE(list.mostRecent()->path == "a.txt");
    REQUIRE(list.mostRecent()->displayName == "A Updated");
    REQUIRE(list.mostRecent()->accessCount == 2);
    REQUIRE(list.mostRecent()->lastOpenedMs == 3000);
}

TEST_CASE("RecentFileList: remove existing entry", "[recentfiles][list]") {
    NF::RecentFileList list;
    list.record("a.txt", "A", NF::RecentFileKind::Script, 100);
    list.record("b.txt", "B", NF::RecentFileKind::Script, 200);
    REQUIRE(list.remove("a.txt"));
    REQUIRE(list.count() == 1);
    REQUIRE(list.mostRecent()->path == "b.txt");
}

TEST_CASE("RecentFileList: remove unknown returns false", "[recentfiles][list]") {
    NF::RecentFileList list;
    REQUIRE_FALSE(list.remove("no.such.file"));
}

TEST_CASE("RecentFileList: findByPath", "[recentfiles][list]") {
    NF::RecentFileList list;
    list.record("x.cfg", "X", NF::RecentFileKind::Config, 500);
    const auto* found = list.findByPath("x.cfg");
    REQUIRE(found != nullptr);
    REQUIRE(found->kind == NF::RecentFileKind::Config);
    REQUIRE(list.findByPath("missing") == nullptr);
}

TEST_CASE("RecentFileList: contains", "[recentfiles][list]") {
    NF::RecentFileList list;
    list.record("a.txt", "A", NF::RecentFileKind::Asset, 100);
    REQUIRE(list.contains("a.txt"));
    REQUIRE_FALSE(list.contains("b.txt"));
}

TEST_CASE("RecentFileList: pin and unpin", "[recentfiles][list]") {
    NF::RecentFileList list;
    list.record("a.txt", "A", NF::RecentFileKind::Asset, 100);
    REQUIRE(list.pin("a.txt", true));
    REQUIRE(list.findByPath("a.txt")->pinned);
    REQUIRE(list.pinnedCount() == 1);
    REQUIRE(list.pin("a.txt", false));
    REQUIRE_FALSE(list.findByPath("a.txt")->pinned);
    REQUIRE(list.pinnedCount() == 0);
}

TEST_CASE("RecentFileList: pin unknown returns false", "[recentfiles][list]") {
    NF::RecentFileList list;
    REQUIRE_FALSE(list.pin("no.such", true));
}

TEST_CASE("RecentFileList: pinned() and unpinned() views", "[recentfiles][list]") {
    NF::RecentFileList list;
    list.record("a.txt", "A", NF::RecentFileKind::Asset, 100);
    list.record("b.txt", "B", NF::RecentFileKind::Asset, 200);
    list.record("c.txt", "C", NF::RecentFileKind::Asset, 300);
    list.pin("b.txt", true);

    auto pinned   = list.pinned();
    auto unpinned = list.unpinned();
    REQUIRE(pinned.size()   == 1);
    REQUIRE(unpinned.size() == 2);
    REQUIRE(pinned[0]->path == "b.txt");
}

TEST_CASE("RecentFileList: clearUnpinned leaves pinned intact", "[recentfiles][list]") {
    NF::RecentFileList list;
    list.record("a.txt", "A", NF::RecentFileKind::Asset, 100);
    list.record("b.txt", "B", NF::RecentFileKind::Asset, 200);
    list.pin("b.txt", true);
    list.clearUnpinned();
    REQUIRE(list.count() == 1);
    REQUIRE(list.contains("b.txt"));
    REQUIRE_FALSE(list.contains("a.txt"));
}

TEST_CASE("RecentFileList: clear removes all including pinned", "[recentfiles][list]") {
    NF::RecentFileList list;
    list.record("a.txt", "A", NF::RecentFileKind::Asset, 100);
    list.pin("a.txt", true);
    list.clear();
    REQUIRE(list.empty());
}

TEST_CASE("RecentFileList: MRU order — newest at front", "[recentfiles][list]") {
    NF::RecentFileList list;
    list.record("first.txt",  "First",  NF::RecentFileKind::Script, 1000);
    list.record("second.txt", "Second", NF::RecentFileKind::Script, 2000);
    list.record("third.txt",  "Third",  NF::RecentFileKind::Script, 3000);
    const auto& all = list.all();
    REQUIRE(all[0].path == "third.txt");
    REQUIRE(all[1].path == "second.txt");
    REQUIRE(all[2].path == "first.txt");
}

TEST_CASE("RecentFileList: capacity evicts oldest unpinned", "[recentfiles][list]") {
    NF::RecentFileList list;
    // Fill to capacity
    for (size_t i = 0; i < NF::RecentFileList::MAX_ENTRIES; ++i) {
        std::string p = "file" + std::to_string(i) + ".txt";
        list.record(p, p, NF::RecentFileKind::Asset, static_cast<uint64_t>(i));
    }
    REQUIRE(list.count() == NF::RecentFileList::MAX_ENTRIES);
    // Record one more — oldest (file0) should be evicted
    list.record("new.txt", "New", NF::RecentFileKind::Asset, 9999);
    REQUIRE(list.count() == NF::RecentFileList::MAX_ENTRIES);
    REQUIRE_FALSE(list.contains("file0.txt"));
    REQUIRE(list.contains("new.txt"));
}

TEST_CASE("RecentFileList: pinned entries survive eviction", "[recentfiles][list]") {
    NF::RecentFileList list;
    // Add one entry and pin it
    list.record("pinned.txt", "Pinned", NF::RecentFileKind::Asset, 0);
    list.pin("pinned.txt", true);
    // Fill to capacity with unpinned entries
    for (size_t i = 1; i < NF::RecentFileList::MAX_ENTRIES; ++i) {
        std::string p = "file" + std::to_string(i) + ".txt";
        list.record(p, p, NF::RecentFileKind::Asset, static_cast<uint64_t>(i));
    }
    REQUIRE(list.count() == NF::RecentFileList::MAX_ENTRIES);
    // Record one more — an unpinned (not "pinned.txt") gets evicted
    list.record("new.txt", "New", NF::RecentFileKind::Asset, 9999);
    REQUIRE(list.contains("pinned.txt")); // still present
    REQUIRE(list.contains("new.txt"));
}

// ═════════════════════════════════════════════════════════════════
// RecentFilesManager
// ═════════════════════════════════════════════════════════════════

TEST_CASE("RecentFilesManager: default empty state", "[recentfiles][manager]") {
    NF::RecentFilesManager mgr;
    REQUIRE(mgr.globalRecent().empty());
    REQUIRE(mgr.listForKind(NF::RecentFileKind::Project).empty());
}

TEST_CASE("RecentFilesManager: record adds to correct kind list", "[recentfiles][manager]") {
    NF::RecentFilesManager mgr;
    REQUIRE(mgr.record("p.atlasproject", "Demo", NF::RecentFileKind::Project, 1000));
    REQUIRE(mgr.listForKind(NF::RecentFileKind::Project).count() == 1);
    REQUIRE(mgr.listForKind(NF::RecentFileKind::Scene).empty());
}

TEST_CASE("RecentFilesManager: record empty path returns false", "[recentfiles][manager]") {
    NF::RecentFilesManager mgr;
    REQUIRE_FALSE(mgr.record("", "Bad", NF::RecentFileKind::Project, 100));
    REQUIRE(mgr.listForKind(NF::RecentFileKind::Project).empty());
}

TEST_CASE("RecentFilesManager: find returns correct entry", "[recentfiles][manager]") {
    NF::RecentFilesManager mgr;
    mgr.record("a.scene", "Scene A", NF::RecentFileKind::Scene, 2000);
    const auto* e = mgr.find("a.scene", NF::RecentFileKind::Scene);
    REQUIRE(e != nullptr);
    REQUIRE(e->displayName == "Scene A");
    REQUIRE(mgr.find("missing.scene", NF::RecentFileKind::Scene) == nullptr);
}

TEST_CASE("RecentFilesManager: remove from correct kind list", "[recentfiles][manager]") {
    NF::RecentFilesManager mgr;
    mgr.record("a.scene", "A", NF::RecentFileKind::Scene, 100);
    REQUIRE(mgr.remove("a.scene", NF::RecentFileKind::Scene));
    REQUIRE(mgr.listForKind(NF::RecentFileKind::Scene).empty());
}

TEST_CASE("RecentFilesManager: remove unknown returns false", "[recentfiles][manager]") {
    NF::RecentFilesManager mgr;
    REQUIRE_FALSE(mgr.remove("no.such", NF::RecentFileKind::Asset));
}

TEST_CASE("RecentFilesManager: pin and unpin", "[recentfiles][manager]") {
    NF::RecentFilesManager mgr;
    mgr.record("cfg.json", "Cfg", NF::RecentFileKind::Config, 300);
    REQUIRE(mgr.pin("cfg.json", NF::RecentFileKind::Config, true));
    REQUIRE(mgr.find("cfg.json", NF::RecentFileKind::Config)->pinned);
    REQUIRE(mgr.pin("cfg.json", NF::RecentFileKind::Config, false));
    REQUIRE_FALSE(mgr.find("cfg.json", NF::RecentFileKind::Config)->pinned);
}

TEST_CASE("RecentFilesManager: pin unknown returns false", "[recentfiles][manager]") {
    NF::RecentFilesManager mgr;
    REQUIRE_FALSE(mgr.pin("no.such", NF::RecentFileKind::Config, true));
}

TEST_CASE("RecentFilesManager: globalRecent merges all kinds", "[recentfiles][manager]") {
    NF::RecentFilesManager mgr;
    mgr.record("a.atlasproject", "A", NF::RecentFileKind::Project, 3000);
    mgr.record("b.scene",        "B", NF::RecentFileKind::Scene,   2000);
    mgr.record("c.asset",        "C", NF::RecentFileKind::Asset,   1000);
    auto recent = mgr.globalRecent();
    REQUIRE(recent.size() == 3);
    // Sorted newest first
    REQUIRE(recent[0].path == "a.atlasproject");
    REQUIRE(recent[1].path == "b.scene");
    REQUIRE(recent[2].path == "c.asset");
}

TEST_CASE("RecentFilesManager: globalRecent capped at MAX_GLOBAL", "[recentfiles][manager]") {
    NF::RecentFilesManager mgr;
    // Add more entries than MAX_GLOBAL across kinds
    for (size_t i = 0; i < NF::RecentFilesManager::MAX_GLOBAL + 5; ++i) {
        std::string p = "file" + std::to_string(i) + ".asset";
        mgr.record(p, p, NF::RecentFileKind::Asset, static_cast<uint64_t>(i));
    }
    auto recent = mgr.globalRecent();
    REQUIRE(recent.size() == NF::RecentFilesManager::MAX_GLOBAL);
}

TEST_CASE("RecentFilesManager: clearKind removes only that kind", "[recentfiles][manager]") {
    NF::RecentFilesManager mgr;
    mgr.record("a.atlasproject", "A", NF::RecentFileKind::Project, 100);
    mgr.record("b.scene",        "B", NF::RecentFileKind::Scene,   200);
    mgr.clearKind(NF::RecentFileKind::Project);
    REQUIRE(mgr.listForKind(NF::RecentFileKind::Project).empty());
    REQUIRE(mgr.listForKind(NF::RecentFileKind::Scene).count() == 1);
}

TEST_CASE("RecentFilesManager: clearAll empties everything", "[recentfiles][manager]") {
    NF::RecentFilesManager mgr;
    mgr.record("a.atlasproject", "A", NF::RecentFileKind::Project, 100);
    mgr.record("b.scene",        "B", NF::RecentFileKind::Scene,   200);
    mgr.clearAll();
    REQUIRE(mgr.globalRecent().empty());
}

TEST_CASE("RecentFilesManager: clearAllUnpinned preserves pinned across kinds", "[recentfiles][manager]") {
    NF::RecentFilesManager mgr;
    mgr.record("a.atlasproject", "A", NF::RecentFileKind::Project, 100);
    mgr.record("b.scene",        "B", NF::RecentFileKind::Scene,   200);
    mgr.pin("b.scene", NF::RecentFileKind::Scene, true);
    mgr.clearAllUnpinned();
    REQUIRE(mgr.listForKind(NF::RecentFileKind::Project).empty());
    REQUIRE(mgr.listForKind(NF::RecentFileKind::Scene).count() == 1);
    REQUIRE(mgr.find("b.scene", NF::RecentFileKind::Scene) != nullptr);
}

TEST_CASE("RecentFilesManager: observer fires on record", "[recentfiles][manager]") {
    NF::RecentFilesManager mgr;
    std::vector<std::pair<std::string, bool>> log;
    mgr.addObserver([&](const NF::RecentFileEntry& e, bool recorded) {
        log.push_back({e.path, recorded});
    });
    mgr.record("x.scene", "X", NF::RecentFileKind::Scene, 100);
    REQUIRE(log.size() == 1);
    REQUIRE(log[0].first  == "x.scene");
    REQUIRE(log[0].second == true);
}

TEST_CASE("RecentFilesManager: observer fires on remove", "[recentfiles][manager]") {
    NF::RecentFilesManager mgr;
    mgr.record("x.scene", "X", NF::RecentFileKind::Scene, 100);

    std::vector<std::pair<std::string, bool>> log;
    mgr.addObserver([&](const NF::RecentFileEntry& e, bool recorded) {
        log.push_back({e.path, recorded});
    });
    mgr.remove("x.scene", NF::RecentFileKind::Scene);
    REQUIRE(log.size() == 1);
    REQUIRE(log[0].first  == "x.scene");
    REQUIRE(log[0].second == false);
}

TEST_CASE("RecentFilesManager: clearObservers stops notifications", "[recentfiles][manager]") {
    NF::RecentFilesManager mgr;
    int count = 0;
    mgr.addObserver([&](const NF::RecentFileEntry&, bool) { ++count; });
    mgr.record("a.scene", "A", NF::RecentFileKind::Scene, 100);
    REQUIRE(count == 1);
    mgr.clearObservers();
    mgr.record("b.scene", "B", NF::RecentFileKind::Scene, 200);
    REQUIRE(count == 1); // no more notifications
}

TEST_CASE("RecentFilesManager: serialize empty produces empty string", "[recentfiles][manager]") {
    NF::RecentFilesManager mgr;
    REQUIRE(mgr.serialize().empty());
}

TEST_CASE("RecentFilesManager: serialize round-trip", "[recentfiles][manager]") {
    NF::RecentFilesManager mgr;
    mgr.record("p.atlasproject", "Demo Project",  NF::RecentFileKind::Project, 5000);
    mgr.record("s.scene",        "Level 1",       NF::RecentFileKind::Scene,   4000);
    mgr.record("cfg.json",       "Config",        NF::RecentFileKind::Config,  3000);
    mgr.pin("p.atlasproject", NF::RecentFileKind::Project, true);

    std::string text = mgr.serialize();
    REQUIRE_FALSE(text.empty());

    NF::RecentFilesManager mgr2;
    int loaded = mgr2.deserialize(text);
    REQUIRE(loaded == 3);
    REQUIRE(mgr2.find("p.atlasproject", NF::RecentFileKind::Project) != nullptr);
    REQUIRE(mgr2.find("p.atlasproject", NF::RecentFileKind::Project)->pinned);
    REQUIRE(mgr2.find("s.scene",        NF::RecentFileKind::Scene)   != nullptr);
    REQUIRE(mgr2.find("cfg.json",       NF::RecentFileKind::Config)  != nullptr);
}

TEST_CASE("RecentFilesManager: serialize escapes pipe in path", "[recentfiles][manager]") {
    NF::RecentFilesManager mgr;
    // Path with a pipe character
    mgr.record("path|with|pipe.txt", "Pipe Path", NF::RecentFileKind::Custom, 100);
    std::string text = mgr.serialize();
    // Should not have bare pipes in the path field (check the line splits correctly)
    NF::RecentFilesManager mgr2;
    int loaded = mgr2.deserialize(text);
    REQUIRE(loaded == 1);
    REQUIRE(mgr2.find("path|with|pipe.txt", NF::RecentFileKind::Custom) != nullptr);
}

TEST_CASE("RecentFilesManager: deserialize empty string returns 0", "[recentfiles][manager]") {
    NF::RecentFilesManager mgr;
    REQUIRE(mgr.deserialize("") == 0);
    REQUIRE(mgr.globalRecent().empty());
}

TEST_CASE("RecentFilesManager: deserialize clears existing data first", "[recentfiles][manager]") {
    NF::RecentFilesManager mgr;
    mgr.record("old.scene", "Old", NF::RecentFileKind::Scene, 100);

    NF::RecentFilesManager src;
    src.record("new.scene", "New", NF::RecentFileKind::Scene, 200);
    std::string text = src.serialize();

    mgr.deserialize(text);
    REQUIRE_FALSE(mgr.listForKind(NF::RecentFileKind::Scene).contains("old.scene"));
    REQUIRE(mgr.listForKind(NF::RecentFileKind::Scene).contains("new.scene"));
}

// ═════════════════════════════════════════════════════════════════
// Integration
// ═════════════════════════════════════════════════════════════════

TEST_CASE("RecentFiles integration: project open workflow", "[recentfiles][integration]") {
    NF::RecentFilesManager mgr;

    // Simulate opening several projects over time
    mgr.record("projects/alpha.atlasproject", "Alpha",   NF::RecentFileKind::Project, 1000);
    mgr.record("projects/beta.atlasproject",  "Beta",    NF::RecentFileKind::Project, 2000);
    mgr.record("projects/gamma.atlasproject", "Gamma",   NF::RecentFileKind::Project, 3000);

    // Also open some scenes within the project
    mgr.record("alpha/level1.scene", "Level 1", NF::RecentFileKind::Scene, 4000);
    mgr.record("alpha/level2.scene", "Level 2", NF::RecentFileKind::Scene, 5000);

    // Check per-kind counts
    REQUIRE(mgr.listForKind(NF::RecentFileKind::Project).count() == 3);
    REQUIRE(mgr.listForKind(NF::RecentFileKind::Scene).count()   == 2);

    // Global should list 5, newest first
    auto global = mgr.globalRecent();
    REQUIRE(global.size() == 5);
    REQUIRE(global[0].path == "alpha/level2.scene");

    // Pin a project so it survives clearAllUnpinned
    mgr.pin("projects/alpha.atlasproject", NF::RecentFileKind::Project, true);
    mgr.clearAllUnpinned();

    REQUIRE(mgr.listForKind(NF::RecentFileKind::Project).count() == 1);
    REQUIRE(mgr.listForKind(NF::RecentFileKind::Scene).empty());
    REQUIRE(mgr.find("projects/alpha.atlasproject", NF::RecentFileKind::Project) != nullptr);
}

TEST_CASE("RecentFiles integration: observer tracks all operations", "[recentfiles][integration]") {
    NF::RecentFilesManager mgr;

    std::vector<std::string> events;
    mgr.addObserver([&](const NF::RecentFileEntry& e, bool recorded) {
        events.push_back((recorded ? "record:" : "remove:") + e.path);
    });

    mgr.record("a.scene", "A", NF::RecentFileKind::Scene, 100);
    mgr.record("b.scene", "B", NF::RecentFileKind::Scene, 200);
    mgr.remove("a.scene", NF::RecentFileKind::Scene);

    REQUIRE(events.size() == 3);
    REQUIRE(events[0] == "record:a.scene");
    REQUIRE(events[1] == "record:b.scene");
    REQUIRE(events[2] == "remove:a.scene");
}

TEST_CASE("RecentFiles integration: access-count increments on re-record", "[recentfiles][integration]") {
    NF::RecentFilesManager mgr;
    mgr.record("x.asset", "X", NF::RecentFileKind::Asset, 100);
    mgr.record("x.asset", "X", NF::RecentFileKind::Asset, 200);
    mgr.record("x.asset", "X", NF::RecentFileKind::Asset, 300);
    const auto* e = mgr.find("x.asset", NF::RecentFileKind::Asset);
    REQUIRE(e != nullptr);
    REQUIRE(e->accessCount == 3);
    REQUIRE(e->lastOpenedMs == 300);
}

TEST_CASE("RecentFiles integration: full serialize/deserialize preserves accessCount and pinned", "[recentfiles][integration]") {
    NF::RecentFilesManager src;
    src.record("proj.atlasproject", "Proj", NF::RecentFileKind::Project, 1000);
    src.record("proj.atlasproject", "Proj", NF::RecentFileKind::Project, 2000); // accessCount=2
    src.pin("proj.atlasproject", NF::RecentFileKind::Project, true);

    std::string text = src.serialize();

    NF::RecentFilesManager dst;
    dst.deserialize(text);

    const auto* e = dst.find("proj.atlasproject", NF::RecentFileKind::Project);
    REQUIRE(e != nullptr);
    REQUIRE(e->accessCount == 2);
    REQUIRE(e->pinned);
    REQUIRE(e->lastOpenedMs == 2000);
}
