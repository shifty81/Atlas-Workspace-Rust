// Tests/Workspace/test_phase60_file_watcher.cpp
// Phase 60 — WorkspaceFileWatcher: FileEventType, FileEvent,
//             WatchEntry, FileWatcher
#include <catch2/catch_test_macros.hpp>
#include "NF/Workspace/WorkspaceFileWatcher.h"

// ═════════════════════════════════════════════════════════════════
// FileEventType
// ═════════════════════════════════════════════════════════════════

TEST_CASE("FileEventType: name helpers", "[watcher][type]") {
    REQUIRE(std::string(NF::fileEventTypeName(NF::FileEventType::Created))  == "Created");
    REQUIRE(std::string(NF::fileEventTypeName(NF::FileEventType::Modified)) == "Modified");
    REQUIRE(std::string(NF::fileEventTypeName(NF::FileEventType::Deleted))  == "Deleted");
    REQUIRE(std::string(NF::fileEventTypeName(NF::FileEventType::Renamed))  == "Renamed");
}

// ═════════════════════════════════════════════════════════════════
// FileEvent
// ═════════════════════════════════════════════════════════════════

static NF::FileEvent makeEvent(const std::string& path,
                                NF::FileEventType type = NF::FileEventType::Modified,
                                uint64_t ts = 1000) {
    NF::FileEvent ev;
    ev.path        = path;
    ev.type        = type;
    ev.timestampMs = ts;
    return ev;
}

TEST_CASE("FileEvent: default invalid", "[watcher][event]") {
    NF::FileEvent ev;
    REQUIRE_FALSE(ev.isValid());
}

TEST_CASE("FileEvent: valid with path", "[watcher][event]") {
    auto ev = makeEvent("/src/main.cpp");
    REQUIRE(ev.isValid());
}

TEST_CASE("FileEvent: equality", "[watcher][event]") {
    auto a = makeEvent("/src/main.cpp", NF::FileEventType::Modified, 1000);
    auto b = makeEvent("/src/main.cpp", NF::FileEventType::Modified, 1000);
    auto c = makeEvent("/src/other.cpp", NF::FileEventType::Modified, 1000);
    REQUIRE(a == b);
    REQUIRE(a != c);
}

TEST_CASE("FileEvent: rename with oldPath", "[watcher][event]") {
    NF::FileEvent ev;
    ev.path    = "/src/new_name.cpp";
    ev.type    = NF::FileEventType::Renamed;
    ev.oldPath = "/src/old_name.cpp";
    REQUIRE(ev.isValid());
    REQUIRE(ev.oldPath == "/src/old_name.cpp");
}

// ═════════════════════════════════════════════════════════════════
// WatchEntry
// ═════════════════════════════════════════════════════════════════

static NF::WatchEntry makeWatch(const std::string& id, const std::string& path,
                                 bool recursive = false) {
    NF::WatchEntry w;
    w.id        = id;
    w.path      = path;
    w.recursive = recursive;
    return w;
}

TEST_CASE("WatchEntry: default invalid", "[watcher][watch]") {
    NF::WatchEntry w;
    REQUIRE_FALSE(w.isValid());
}

TEST_CASE("WatchEntry: valid with id and path", "[watcher][watch]") {
    auto w = makeWatch("w1", "/src");
    REQUIRE(w.isValid());
    REQUIRE(w.enabled);
}

TEST_CASE("WatchEntry: equality by id", "[watcher][watch]") {
    auto a = makeWatch("a", "/src");
    auto b = makeWatch("a", "/other");
    auto c = makeWatch("c", "/src");
    REQUIRE(a == b);
    REQUIRE(a != c);
}

TEST_CASE("WatchEntry: filter", "[watcher][watch]") {
    auto w = makeWatch("w1", "/src");
    w.filter = "*.cpp";
    REQUIRE(w.filter == "*.cpp");
}

// ═════════════════════════════════════════════════════════════════
// FileWatcher
// ═════════════════════════════════════════════════════════════════

TEST_CASE("FileWatcher: default empty", "[watcher][main]") {
    NF::FileWatcher fw;
    REQUIRE(fw.watchCount() == 0);
    REQUIRE(fw.pendingCount() == 0);
}

TEST_CASE("FileWatcher: addWatch", "[watcher][main]") {
    NF::FileWatcher fw;
    REQUIRE(fw.addWatch(makeWatch("w1", "/src")));
    REQUIRE(fw.watchCount() == 1);
    REQUIRE(fw.hasWatch("w1"));
}

TEST_CASE("FileWatcher: addWatch rejects invalid", "[watcher][main]") {
    NF::FileWatcher fw;
    NF::WatchEntry w;
    REQUIRE_FALSE(fw.addWatch(w));
}

TEST_CASE("FileWatcher: addWatch rejects duplicate", "[watcher][main]") {
    NF::FileWatcher fw;
    fw.addWatch(makeWatch("w1", "/src"));
    REQUIRE_FALSE(fw.addWatch(makeWatch("w1", "/other")));
}

TEST_CASE("FileWatcher: removeWatch", "[watcher][main]") {
    NF::FileWatcher fw;
    fw.addWatch(makeWatch("w1", "/src"));
    REQUIRE(fw.removeWatch("w1"));
    REQUIRE(fw.watchCount() == 0);
}

TEST_CASE("FileWatcher: removeWatch unknown", "[watcher][main]") {
    NF::FileWatcher fw;
    REQUIRE_FALSE(fw.removeWatch("missing"));
}

TEST_CASE("FileWatcher: enableWatch", "[watcher][main]") {
    NF::FileWatcher fw;
    fw.addWatch(makeWatch("w1", "/src"));
    REQUIRE(fw.enableWatch("w1", false));
    REQUIRE_FALSE(fw.findWatch("w1")->enabled);
    REQUIRE(fw.enableWatch("w1", true));
    REQUIRE(fw.findWatch("w1")->enabled);
}

TEST_CASE("FileWatcher: enableWatch unknown", "[watcher][main]") {
    NF::FileWatcher fw;
    REQUIRE_FALSE(fw.enableWatch("missing", true));
}

TEST_CASE("FileWatcher: pushEvent", "[watcher][main]") {
    NF::FileWatcher fw;
    REQUIRE(fw.pushEvent(makeEvent("/src/main.cpp")));
    REQUIRE(fw.pendingCount() == 1);
}

TEST_CASE("FileWatcher: pushEvent rejects invalid", "[watcher][main]") {
    NF::FileWatcher fw;
    NF::FileEvent ev;
    REQUIRE_FALSE(fw.pushEvent(ev));
}

TEST_CASE("FileWatcher: pushEvent debounces duplicate", "[watcher][main]") {
    NF::FileWatcher fw;
    fw.pushEvent(makeEvent("/src/main.cpp", NF::FileEventType::Modified));
    REQUIRE_FALSE(fw.pushEvent(makeEvent("/src/main.cpp", NF::FileEventType::Modified)));
    REQUIRE(fw.pendingCount() == 1);
}

TEST_CASE("FileWatcher: pushEvent allows same path different type", "[watcher][main]") {
    NF::FileWatcher fw;
    fw.pushEvent(makeEvent("/src/main.cpp", NF::FileEventType::Modified));
    REQUIRE(fw.pushEvent(makeEvent("/src/main.cpp", NF::FileEventType::Deleted)));
    REQUIRE(fw.pendingCount() == 2);
}

TEST_CASE("FileWatcher: consumeEvents", "[watcher][main]") {
    NF::FileWatcher fw;
    fw.pushEvent(makeEvent("/a.cpp"));
    fw.pushEvent(makeEvent("/b.cpp"));
    auto events = fw.consumeEvents();
    REQUIRE(events.size() == 2);
    REQUIRE(fw.pendingCount() == 0);
}

TEST_CASE("FileWatcher: clearPending", "[watcher][main]") {
    NF::FileWatcher fw;
    fw.pushEvent(makeEvent("/a.cpp"));
    fw.clearPending();
    REQUIRE(fw.pendingCount() == 0);
}

TEST_CASE("FileWatcher: filterByType", "[watcher][main]") {
    NF::FileWatcher fw;
    fw.pushEvent(makeEvent("/a.cpp", NF::FileEventType::Created));
    fw.pushEvent(makeEvent("/b.cpp", NF::FileEventType::Modified));
    fw.pushEvent(makeEvent("/c.cpp", NF::FileEventType::Created));

    auto created = fw.filterByType(NF::FileEventType::Created);
    REQUIRE(created.size() == 2);
}

TEST_CASE("FileWatcher: countByType", "[watcher][main]") {
    NF::FileWatcher fw;
    fw.pushEvent(makeEvent("/a.cpp", NF::FileEventType::Deleted));
    fw.pushEvent(makeEvent("/b.cpp", NF::FileEventType::Modified));
    fw.pushEvent(makeEvent("/c.cpp", NF::FileEventType::Deleted));

    REQUIRE(fw.countByType(NF::FileEventType::Deleted) == 2);
    REQUIRE(fw.countByType(NF::FileEventType::Modified) == 1);
    REQUIRE(fw.countByType(NF::FileEventType::Created) == 0);
}

TEST_CASE("FileWatcher: processPending notifies observers", "[watcher][main]") {
    NF::FileWatcher fw;
    int callCount = 0;
    std::string lastPath;
    fw.addObserver([&](const NF::FileEvent& ev) {
        ++callCount;
        lastPath = ev.path;
    });

    fw.pushEvent(makeEvent("/a.cpp"));
    fw.pushEvent(makeEvent("/b.cpp"));
    fw.processPending();

    REQUIRE(callCount == 2);
    REQUIRE(lastPath == "/b.cpp");
    REQUIRE(fw.pendingCount() == 0);
}

TEST_CASE("FileWatcher: clearObservers", "[watcher][main]") {
    NF::FileWatcher fw;
    int callCount = 0;
    fw.addObserver([&](const NF::FileEvent&) { ++callCount; });
    fw.clearObservers();
    fw.pushEvent(makeEvent("/a.cpp"));
    fw.processPending();
    REQUIRE(callCount == 0);
}

TEST_CASE("FileWatcher: serialize empty", "[watcher][serial]") {
    NF::FileWatcher fw;
    REQUIRE(fw.serialize().empty());
}

TEST_CASE("FileWatcher: serialize round-trip", "[watcher][serial]") {
    NF::FileWatcher fw;
    auto w1 = makeWatch("w1", "/src", true);
    w1.filter = "*.cpp";
    fw.addWatch(w1);
    auto w2 = makeWatch("w2", "/assets");
    w2.enabled = false;
    fw.addWatch(w2);

    std::string data = fw.serialize();
    REQUIRE_FALSE(data.empty());

    NF::FileWatcher fw2;
    REQUIRE(fw2.deserialize(data));
    REQUIRE(fw2.watchCount() == 2);

    const auto* r1 = fw2.findWatch("w1");
    REQUIRE(r1 != nullptr);
    REQUIRE(r1->path == "/src");
    REQUIRE(r1->recursive == true);
    REQUIRE(r1->filter == "*.cpp");

    const auto* r2 = fw2.findWatch("w2");
    REQUIRE(r2 != nullptr);
    REQUIRE(r2->enabled == false);
}

TEST_CASE("FileWatcher: serialize pipe in path", "[watcher][serial]") {
    NF::FileWatcher fw;
    fw.addWatch(makeWatch("w1", "/path|with|pipes"));

    std::string data = fw.serialize();
    NF::FileWatcher fw2;
    fw2.deserialize(data);
    REQUIRE(fw2.findWatch("w1")->path == "/path|with|pipes");
}

TEST_CASE("FileWatcher: deserialize empty", "[watcher][serial]") {
    NF::FileWatcher fw;
    REQUIRE(fw.deserialize(""));
    REQUIRE(fw.watchCount() == 0);
}

TEST_CASE("FileWatcher: clear", "[watcher][main]") {
    NF::FileWatcher fw;
    fw.addWatch(makeWatch("w1", "/src"));
    fw.pushEvent(makeEvent("/a.cpp"));
    int calls = 0;
    fw.addObserver([&](const NF::FileEvent&) { ++calls; });
    fw.clear();
    REQUIRE(fw.watchCount() == 0);
    REQUIRE(fw.pendingCount() == 0);
}

// ═════════════════════════════════════════════════════════════════
// Integration
// ═════════════════════════════════════════════════════════════════

TEST_CASE("Integration: multi-watch file watcher workflow", "[watcher][integration]") {
    NF::FileWatcher fw;
    fw.addWatch(makeWatch("src", "/src", true));
    fw.addWatch(makeWatch("assets", "/assets"));

    fw.pushEvent(makeEvent("/src/main.cpp", NF::FileEventType::Modified));
    fw.pushEvent(makeEvent("/src/utils.h", NF::FileEventType::Created));
    fw.pushEvent(makeEvent("/assets/texture.png", NF::FileEventType::Deleted));

    REQUIRE(fw.pendingCount() == 3);
    REQUIRE(fw.countByType(NF::FileEventType::Modified) == 1);
    REQUIRE(fw.countByType(NF::FileEventType::Created) == 1);
    REQUIRE(fw.countByType(NF::FileEventType::Deleted) == 1);

    std::vector<std::string> processed;
    fw.addObserver([&](const NF::FileEvent& ev) { processed.push_back(ev.path); });
    fw.processPending();

    REQUIRE(processed.size() == 3);
    REQUIRE(fw.pendingCount() == 0);
}

TEST_CASE("Integration: serialize/deserialize preserves watch config", "[watcher][integration]") {
    NF::FileWatcher fw;
    auto w1 = makeWatch("w1", "/project/src", true);
    w1.filter = "*.{cpp,h}";
    fw.addWatch(w1);

    auto w2 = makeWatch("w2", "/project/docs");
    w2.enabled = false;
    fw.addWatch(w2);

    std::string data = fw.serialize();
    NF::FileWatcher fw2;
    fw2.deserialize(data);

    REQUIRE(fw2.watchCount() == 2);
    REQUIRE(fw2.findWatch("w1")->recursive == true);
    REQUIRE(fw2.findWatch("w1")->filter == "*.{cpp,h}");
    REQUIRE(fw2.findWatch("w2")->enabled == false);
}
