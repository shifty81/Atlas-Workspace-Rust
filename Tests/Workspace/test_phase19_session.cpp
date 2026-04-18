// Tests/Workspace/test_phase19_session.cpp
// Phase 19 — Workspace Session Management
//
// Tests for:
//   1. SessionState — enum name helpers
//   2. RecentItem — recently opened item descriptor
//   3. SessionRecord — session snapshot with tool list and duration
//   4. SessionHistory — capped recent items and session records
//   5. SessionManager — workspace-scoped session lifecycle with observers
//   6. Integration — full session pipeline

#include <catch2/catch_test_macros.hpp>
#include "NF/Workspace/WorkspaceSession.h"
#include <string>
#include <vector>

using namespace NF;

// ═════════════════════════════════════════════════════════════════
// SECTION 1 — Enums
// ═════════════════════════════════════════════════════════════════

TEST_CASE("sessionStateName returns correct strings", "[Phase19][SessionState]") {
    CHECK(std::string(sessionStateName(SessionState::Idle))     == "Idle");
    CHECK(std::string(sessionStateName(SessionState::Starting)) == "Starting");
    CHECK(std::string(sessionStateName(SessionState::Running))  == "Running");
    CHECK(std::string(sessionStateName(SessionState::Saving))   == "Saving");
    CHECK(std::string(sessionStateName(SessionState::Closing))  == "Closing");
    CHECK(std::string(sessionStateName(SessionState::Closed))   == "Closed");
}

// ═════════════════════════════════════════════════════════════════
// SECTION 2 — RecentItem
// ═════════════════════════════════════════════════════════════════

TEST_CASE("RecentItem default is invalid", "[Phase19][RecentItem]") {
    RecentItem item;
    CHECK_FALSE(item.isValid());
    CHECK(item.path.empty());
}

TEST_CASE("RecentItem valid construction", "[Phase19][RecentItem]") {
    RecentItem item{"projects/foo.atlas", "Foo Project", "workspace", 1000};
    CHECK(item.isValid());
    CHECK(item.path == "projects/foo.atlas");
    CHECK(item.label == "Foo Project");
    CHECK(item.type == "workspace");
    CHECK(item.timestamp == 1000);
}

TEST_CASE("RecentItem equality by path", "[Phase19][RecentItem]") {
    RecentItem a{"path/a", "Label A", "type", 1};
    RecentItem b{"path/a", "Label B", "other", 2};
    RecentItem c{"path/c", "Label C", "type", 3};
    CHECK(a == b);
    CHECK(a != c);
}

// ═════════════════════════════════════════════════════════════════
// SECTION 3 — SessionRecord
// ═════════════════════════════════════════════════════════════════

TEST_CASE("SessionRecord default is invalid", "[Phase19][SessionRecord]") {
    SessionRecord r;
    CHECK_FALSE(r.isValid());
    CHECK(r.id().empty());
    CHECK(r.name().empty());
    CHECK(r.state() == SessionState::Idle);
    CHECK(r.startTime() == 0);
    CHECK(r.endTime() == 0);
    CHECK(r.duration() == 0);
}

TEST_CASE("SessionRecord valid construction", "[Phase19][SessionRecord]") {
    SessionRecord r("sess_1", "My Session");
    CHECK(r.isValid());
    CHECK(r.id() == "sess_1");
    CHECK(r.name() == "My Session");
}

TEST_CASE("SessionRecord addTool and hasTool", "[Phase19][SessionRecord]") {
    SessionRecord r("sess_1");
    CHECK_FALSE(r.hasTool("tool_a"));
    r.addTool("tool_a");
    r.addTool("tool_b");
    CHECK(r.hasTool("tool_a"));
    CHECK(r.hasTool("tool_b"));
    CHECK(r.toolIds().size() == 2);
    // Duplicate ignored
    r.addTool("tool_a");
    CHECK(r.toolIds().size() == 2);
    // Empty id ignored
    r.addTool("");
    CHECK(r.toolIds().size() == 2);
}

TEST_CASE("SessionRecord duration calculation", "[Phase19][SessionRecord]") {
    SessionRecord r("sess_1");
    r.setStartTime(100);
    r.setEndTime(200);
    CHECK(r.duration() == 100);
    // endTime < startTime → 0
    r.setEndTime(50);
    CHECK(r.duration() == 0);
}

TEST_CASE("SessionRecord equality by id", "[Phase19][SessionRecord]") {
    SessionRecord a("sess_1", "A");
    SessionRecord b("sess_1", "B");
    SessionRecord c("sess_2", "C");
    CHECK(a == b);
    CHECK(a != c);
}

TEST_CASE("SessionRecord state field", "[Phase19][SessionRecord]") {
    SessionRecord r("sess_1");
    r.setState(SessionState::Running);
    CHECK(r.state() == SessionState::Running);
}

// ═════════════════════════════════════════════════════════════════
// SECTION 4 — SessionHistory
// ═════════════════════════════════════════════════════════════════

TEST_CASE("SessionHistory empty state", "[Phase19][SessionHistory]") {
    SessionHistory h;
    CHECK(h.items().empty());
    CHECK(h.records().empty());
}

TEST_CASE("SessionHistory addItem and findItem", "[Phase19][SessionHistory]") {
    SessionHistory h;
    RecentItem item{"path/a", "A", "ws", 1};
    h.addItem(item);
    CHECK(h.items().size() == 1);
    auto* found = h.findItem("path/a");
    REQUIRE(found != nullptr);
    CHECK(found->label == "A");
    CHECK(h.findItem("nonexistent") == nullptr);
}

TEST_CASE("SessionHistory addItem dedup moves to front", "[Phase19][SessionHistory]") {
    SessionHistory h;
    h.addItem({"path/a", "A", "ws", 1});
    h.addItem({"path/b", "B", "ws", 2});
    h.addItem({"path/c", "C", "ws", 3});
    // Now re-add path/a — should move to front
    h.addItem({"path/a", "A updated", "ws", 10});
    CHECK(h.items().size() == 3);
    CHECK(h.items().front().path == "path/a");
    CHECK(h.items().front().label == "A updated");
}

TEST_CASE("SessionHistory addItem invalid rejected", "[Phase19][SessionHistory]") {
    SessionHistory h;
    h.addItem({}); // invalid
    CHECK(h.items().empty());
}

TEST_CASE("SessionHistory removeItem", "[Phase19][SessionHistory]") {
    SessionHistory h;
    h.addItem({"path/a", "A", "ws", 1});
    h.addItem({"path/b", "B", "ws", 2});
    h.removeItem("path/a");
    CHECK(h.items().size() == 1);
    CHECK(h.findItem("path/a") == nullptr);
    CHECK(h.findItem("path/b") != nullptr);
}

TEST_CASE("SessionHistory addRecord and findRecord", "[Phase19][SessionHistory]") {
    SessionHistory h;
    SessionRecord r("sess_1", "My Session");
    h.addRecord(r);
    CHECK(h.records().size() == 1);
    auto* found = h.findRecord("sess_1");
    REQUIRE(found != nullptr);
    CHECK(found->name() == "My Session");
    CHECK(h.findRecord("nonexistent") == nullptr);
}

TEST_CASE("SessionHistory invalid record rejected", "[Phase19][SessionHistory]") {
    SessionHistory h;
    h.addRecord(SessionRecord{}); // invalid
    CHECK(h.records().empty());
}

TEST_CASE("SessionHistory MAX_ITEMS enforcement", "[Phase19][SessionHistory]") {
    SessionHistory h;
    for (int i = 0; i < 65; ++i)
        h.addItem({std::string("path/") + std::to_string(i), "label", "ws", (uint64_t)i});
    CHECK(h.items().size() == SessionHistory::MAX_ITEMS);
}

TEST_CASE("SessionHistory clear", "[Phase19][SessionHistory]") {
    SessionHistory h;
    h.addItem({"path/a", "A", "ws", 1});
    h.addRecord(SessionRecord{"sess_1"});
    h.clear();
    CHECK(h.items().empty());
    CHECK(h.records().empty());
}

// ═════════════════════════════════════════════════════════════════
// SECTION 5 — SessionManager
// ═════════════════════════════════════════════════════════════════

TEST_CASE("SessionManager initial state is Idle", "[Phase19][SessionManager]") {
    SessionManager mgr;
    CHECK(mgr.state() == SessionState::Idle);
    CHECK_FALSE(mgr.isRunning());
}

TEST_CASE("SessionManager start success", "[Phase19][SessionManager]") {
    SessionManager mgr;
    CHECK(mgr.start("Test Session"));
    CHECK(mgr.isRunning());
    CHECK(mgr.state() == SessionState::Running);
}

TEST_CASE("SessionManager start while running fails", "[Phase19][SessionManager]") {
    SessionManager mgr;
    mgr.start("Session 1");
    CHECK_FALSE(mgr.start("Session 2"));
    CHECK(mgr.isRunning());
}

TEST_CASE("SessionManager stop success", "[Phase19][SessionManager]") {
    SessionManager mgr;
    mgr.start("Session 1");
    CHECK(mgr.stop());
    CHECK_FALSE(mgr.isRunning());
    CHECK(mgr.state() == SessionState::Idle);
}

TEST_CASE("SessionManager stop while idle fails", "[Phase19][SessionManager]") {
    SessionManager mgr;
    CHECK_FALSE(mgr.stop());
}

TEST_CASE("SessionManager save while running", "[Phase19][SessionManager]") {
    SessionManager mgr;
    mgr.start("Session 1");
    CHECK(mgr.save());
}

TEST_CASE("SessionManager save while idle fails", "[Phase19][SessionManager]") {
    SessionManager mgr;
    CHECK_FALSE(mgr.save());
}

TEST_CASE("SessionManager isRunning reflects state", "[Phase19][SessionManager]") {
    SessionManager mgr;
    CHECK_FALSE(mgr.isRunning());
    mgr.start("S");
    CHECK(mgr.isRunning());
    mgr.stop();
    CHECK_FALSE(mgr.isRunning());
}

TEST_CASE("SessionManager currentRecord has correct name", "[Phase19][SessionManager]") {
    SessionManager mgr;
    mgr.start("My Session");
    CHECK(mgr.currentRecord().name() == "My Session");
    CHECK(mgr.currentRecord().isValid());
}

TEST_CASE("SessionManager addRecentItem and recentItems", "[Phase19][SessionManager]") {
    SessionManager mgr;
    mgr.addRecentItem({"path/a", "A", "ws", 1});
    CHECK(mgr.recentItems().size() == 1);
    CHECK(mgr.recentItems().front().path == "path/a");
}

TEST_CASE("SessionManager clearRecent clears items", "[Phase19][SessionManager]") {
    SessionManager mgr;
    mgr.addRecentItem({"path/a", "A", "ws", 1});
    mgr.clearRecent();
    CHECK(mgr.recentItems().empty());
}

TEST_CASE("SessionManager history has record after stop", "[Phase19][SessionManager]") {
    SessionManager mgr;
    mgr.start("Session X");
    mgr.stop();
    CHECK(mgr.history().records().size() == 1);
    CHECK(mgr.history().records().front().name() == "Session X");
}

TEST_CASE("SessionManager observer notified on start", "[Phase19][SessionManager]") {
    SessionManager mgr;
    std::vector<SessionState> states;
    mgr.addObserver([&](SessionState s) { states.push_back(s); });
    mgr.start("S");
    // Should get Starting then Running
    REQUIRE(states.size() >= 2);
    CHECK(states[0] == SessionState::Starting);
    CHECK(states[1] == SessionState::Running);
}

TEST_CASE("SessionManager observer notified on stop", "[Phase19][SessionManager]") {
    SessionManager mgr;
    std::vector<SessionState> states;
    mgr.start("S");
    mgr.addObserver([&](SessionState s) { states.push_back(s); });
    mgr.stop();
    // Should get Saving, Closed, Idle
    REQUIRE(states.size() >= 2);
    CHECK(states.front() == SessionState::Saving);
}

TEST_CASE("SessionManager removeObserver stops notifications", "[Phase19][SessionManager]") {
    SessionManager mgr;
    int count = 0;
    auto id = mgr.addObserver([&](SessionState) { ++count; });
    mgr.start("S");
    int countAfterStart = count;
    mgr.removeObserver(id);
    mgr.stop();
    CHECK(count == countAfterStart); // No more calls after remove
}

TEST_CASE("SessionManager clearObservers stops all", "[Phase19][SessionManager]") {
    SessionManager mgr;
    int count = 0;
    mgr.addObserver([&](SessionState) { ++count; });
    mgr.addObserver([&](SessionState) { ++count; });
    mgr.start("S");
    int countAfterStart = count;
    mgr.clearObservers();
    mgr.stop();
    CHECK(count == countAfterStart);
}

// ═════════════════════════════════════════════════════════════════
// SECTION 6 — Integration
// ═════════════════════════════════════════════════════════════════

TEST_CASE("Integration: full session lifecycle", "[Phase19][Integration]") {
    SessionManager mgr;
    mgr.start("Project Alpha");
    mgr.addRecentItem({"projects/alpha.atlas", "Alpha", "workspace", 100});
    CHECK(mgr.save());
    CHECK(mgr.currentRecord().name() == "Project Alpha");
    mgr.stop();
    CHECK_FALSE(mgr.isRunning());
    CHECK(mgr.history().records().size() == 1);
    CHECK(mgr.recentItems().size() == 1);
}

TEST_CASE("Integration: multiple sessions accumulated in history", "[Phase19][Integration]") {
    SessionManager mgr;
    for (int i = 0; i < 3; ++i) {
        mgr.start("Session " + std::to_string(i));
        mgr.stop();
    }
    CHECK(mgr.history().records().size() == 3);
}

TEST_CASE("Integration: recent items dedup across sessions", "[Phase19][Integration]") {
    SessionManager mgr;
    mgr.addRecentItem({"path/a", "A", "ws", 1});
    mgr.addRecentItem({"path/b", "B", "ws", 2});
    mgr.addRecentItem({"path/a", "A updated", "ws", 3}); // dedup
    CHECK(mgr.recentItems().size() == 2);
    CHECK(mgr.recentItems().front().path == "path/a");
}

TEST_CASE("Integration: observer tracks all state changes", "[Phase19][Integration]") {
    SessionManager mgr;
    std::vector<SessionState> states;
    mgr.addObserver([&](SessionState s) { states.push_back(s); });
    mgr.start("S");
    mgr.stop();
    // start: Starting, Running; stop: Saving, Closed, Idle
    CHECK(states.size() == 5);
    CHECK(states[0] == SessionState::Starting);
    CHECK(states[1] == SessionState::Running);
    CHECK(states[2] == SessionState::Saving);
}

TEST_CASE("Integration: session name preserved in record", "[Phase19][Integration]") {
    SessionManager mgr;
    mgr.start("Important Session");
    mgr.stop();
    auto* rec = mgr.history().findRecord(mgr.history().records().front().id());
    REQUIRE(rec != nullptr);
    CHECK(rec->name() == "Important Session");
}
