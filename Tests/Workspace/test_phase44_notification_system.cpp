// Tests/Workspace/test_phase44_notification_system.cpp
// Phase 44 — NotificationSystem (advanced channels)
//
// Tests for:
//   1. NotificationSeverity — enum name helpers
//   2. NotificationState    — enum name helpers
//   3. Notification         — state transitions (show/dismiss/expire); isError/isCritical/isVisible
//   4. NotificationChannel  — post/dismiss/find/activeCount/errorCount/clearDismissed
//   5. NotificationSystem   — createChannel/removeChannel/findChannel/post; totalActive
//   6. Integration          — multi-channel workflow, error aggregation

#include <catch2/catch_test_macros.hpp>
#include "NF/Workspace/NotificationSystem.h"
#include <string>

using namespace NF;

// ── Helper ────────────────────────────────────────────────────────

static Notification makeNotif(const std::string& id,
                               NotificationSeverity sev = NotificationSeverity::Info,
                               bool persistent = false) {
    Notification n;
    n.id         = id;
    n.title      = id + "_title";
    n.message    = id + "_msg";
    n.severity   = sev;
    n.persistent = persistent;
    return n;
}

// ─────────────────────────────────────────────────────────────────
// 1. NotificationSeverity
// ─────────────────────────────────────────────────────────────────

TEST_CASE("NotificationSeverity – all values have names", "[phase44][NotificationSeverity]") {
    CHECK(std::string(notificationSeverityName(NotificationSeverity::Info))     == "Info");
    CHECK(std::string(notificationSeverityName(NotificationSeverity::Success))  == "Success");
    CHECK(std::string(notificationSeverityName(NotificationSeverity::Warning))  == "Warning");
    CHECK(std::string(notificationSeverityName(NotificationSeverity::Error))    == "Error");
    CHECK(std::string(notificationSeverityName(NotificationSeverity::Critical)) == "Critical");
    CHECK(std::string(notificationSeverityName(NotificationSeverity::Debug))    == "Debug");
    CHECK(std::string(notificationSeverityName(NotificationSeverity::Trace))    == "Trace");
    CHECK(std::string(notificationSeverityName(NotificationSeverity::System))   == "System");
}

// ─────────────────────────────────────────────────────────────────
// 2. NotificationState
// ─────────────────────────────────────────────────────────────────

TEST_CASE("NotificationState – all values have names", "[phase44][NotificationState]") {
    CHECK(std::string(notificationStateName(NotificationState::Pending))   == "Pending");
    CHECK(std::string(notificationStateName(NotificationState::Shown))     == "Shown");
    CHECK(std::string(notificationStateName(NotificationState::Dismissed)) == "Dismissed");
    CHECK(std::string(notificationStateName(NotificationState::Expired))   == "Expired");
}

// ─────────────────────────────────────────────────────────────────
// 3. Notification
// ─────────────────────────────────────────────────────────────────

TEST_CASE("Notification – default state is Pending", "[phase44][Notification]") {
    Notification n;
    CHECK(n.state == NotificationState::Pending);
    CHECK_FALSE(n.isDismissed());
    CHECK_FALSE(n.isExpired());
    CHECK_FALSE(n.isVisible());
}

TEST_CASE("Notification – show transitions to Shown", "[phase44][Notification]") {
    Notification n;
    n.show();
    CHECK(n.isVisible());
    CHECK(n.state == NotificationState::Shown);
}

TEST_CASE("Notification – dismiss transitions to Dismissed", "[phase44][Notification]") {
    Notification n;
    n.show();
    n.dismiss();
    CHECK(n.isDismissed());
    CHECK_FALSE(n.isVisible());
}

TEST_CASE("Notification – expire transitions to Expired", "[phase44][Notification]") {
    Notification n;
    n.show();
    n.expire();
    CHECK(n.isExpired());
    CHECK_FALSE(n.isVisible());
}

TEST_CASE("Notification – isError for Error and Critical", "[phase44][Notification]") {
    Notification n;
    n.severity = NotificationSeverity::Error;
    CHECK(n.isError());
    n.severity = NotificationSeverity::Critical;
    CHECK(n.isError());
    n.severity = NotificationSeverity::Warning;
    CHECK_FALSE(n.isError());
}

TEST_CASE("Notification – isCritical only for Critical", "[phase44][Notification]") {
    Notification n;
    n.severity = NotificationSeverity::Critical;
    CHECK(n.isCritical());
    n.severity = NotificationSeverity::Error;
    CHECK_FALSE(n.isCritical());
}

TEST_CASE("Notification – durationMs default 3000", "[phase44][Notification]") {
    Notification n;
    CHECK(n.durationMs == 3000);
}

// ─────────────────────────────────────────────────────────────────
// 4. NotificationChannel
// ─────────────────────────────────────────────────────────────────

TEST_CASE("NotificationChannel – constructed with name, initially empty", "[phase44][NotificationChannel]") {
    NotificationChannel ch("build");
    CHECK(ch.name()              == "build");
    CHECK(ch.notificationCount() == 0);
    CHECK(ch.activeCount()       == 0);
}

TEST_CASE("NotificationChannel – post shows notification", "[phase44][NotificationChannel]") {
    NotificationChannel ch("ch");
    CHECK(ch.post(makeNotif("n1")));
    CHECK(ch.notificationCount() == 1);
    CHECK(ch.activeCount()       == 1);  // post() calls show()
}

TEST_CASE("NotificationChannel – post rejects duplicate id", "[phase44][NotificationChannel]") {
    NotificationChannel ch("ch");
    ch.post(makeNotif("n1"));
    CHECK_FALSE(ch.post(makeNotif("n1")));
    CHECK(ch.notificationCount() == 1);
}

TEST_CASE("NotificationChannel – find returns mutable pointer", "[phase44][NotificationChannel]") {
    NotificationChannel ch("ch");
    ch.post(makeNotif("n1", NotificationSeverity::Warning));
    Notification* n = ch.find("n1");
    REQUIRE(n != nullptr);
    CHECK(n->severity == NotificationSeverity::Warning);
}

TEST_CASE("NotificationChannel – find returns nullptr for unknown", "[phase44][NotificationChannel]") {
    NotificationChannel ch("ch");
    CHECK(ch.find("ghost") == nullptr);
}

TEST_CASE("NotificationChannel – dismiss changes state", "[phase44][NotificationChannel]") {
    NotificationChannel ch("ch");
    ch.post(makeNotif("n1"));
    CHECK(ch.dismiss("n1"));
    Notification* n = ch.find("n1");
    REQUIRE(n != nullptr);
    CHECK(n->isDismissed());
    CHECK(ch.activeCount() == 0);
}

TEST_CASE("NotificationChannel – dismiss unknown returns false", "[phase44][NotificationChannel]") {
    NotificationChannel ch("ch");
    CHECK_FALSE(ch.dismiss("ghost"));
}

TEST_CASE("NotificationChannel – errorCount counts Error+ severity", "[phase44][NotificationChannel]") {
    NotificationChannel ch("ch");
    ch.post(makeNotif("n1", NotificationSeverity::Info));
    ch.post(makeNotif("n2", NotificationSeverity::Error));
    ch.post(makeNotif("n3", NotificationSeverity::Critical));
    CHECK(ch.errorCount() == 2);
}

TEST_CASE("NotificationChannel – clearDismissed removes dismissed and expired", "[phase44][NotificationChannel]") {
    NotificationChannel ch("ch");
    ch.post(makeNotif("n1"));
    ch.post(makeNotif("n2"));
    ch.post(makeNotif("n3"));
    ch.dismiss("n1");
    ch.find("n2")->expire();
    size_t removed = ch.clearDismissed();
    CHECK(removed == 2);
    CHECK(ch.notificationCount() == 1);  // n3 remains
}

TEST_CASE("NotificationChannel – clearDismissed returns 0 when nothing to clear", "[phase44][NotificationChannel]") {
    NotificationChannel ch("ch");
    ch.post(makeNotif("n1"));
    CHECK(ch.clearDismissed() == 0);
    CHECK(ch.notificationCount() == 1);
}

// ─────────────────────────────────────────────────────────────────
// 5. NotificationSystem
// ─────────────────────────────────────────────────────────────────

TEST_CASE("NotificationSystem – default empty", "[phase44][NotificationSystem]") {
    NotificationSystem sys;
    CHECK(sys.channelCount() == 0);
    CHECK(sys.totalActive()  == 0);
}

TEST_CASE("NotificationSystem – createChannel returns pointer", "[phase44][NotificationSystem]") {
    NotificationSystem sys;
    NotificationChannel* ch = sys.createChannel("errors");
    REQUIRE(ch != nullptr);
    CHECK(ch->name() == "errors");
    CHECK(sys.channelCount() == 1);
}

TEST_CASE("NotificationSystem – createChannel rejects duplicate name", "[phase44][NotificationSystem]") {
    NotificationSystem sys;
    sys.createChannel("errors");
    CHECK(sys.createChannel("errors") == nullptr);
    CHECK(sys.channelCount() == 1);
}

TEST_CASE("NotificationSystem – removeChannel succeeds", "[phase44][NotificationSystem]") {
    NotificationSystem sys;
    sys.createChannel("temp");
    CHECK(sys.removeChannel("temp"));
    CHECK(sys.channelCount() == 0);
}

TEST_CASE("NotificationSystem – removeChannel unknown returns false", "[phase44][NotificationSystem]") {
    NotificationSystem sys;
    CHECK_FALSE(sys.removeChannel("ghost"));
}

TEST_CASE("NotificationSystem – findChannel returns pointer", "[phase44][NotificationSystem]") {
    NotificationSystem sys;
    sys.createChannel("build");
    NotificationChannel* ch = sys.findChannel("build");
    REQUIRE(ch != nullptr);
    CHECK(ch->name() == "build");
}

TEST_CASE("NotificationSystem – findChannel returns nullptr for unknown", "[phase44][NotificationSystem]") {
    NotificationSystem sys;
    CHECK(sys.findChannel("unknown") == nullptr);
}

TEST_CASE("NotificationSystem – post to existing channel succeeds", "[phase44][NotificationSystem]") {
    NotificationSystem sys;
    sys.createChannel("build");
    CHECK(sys.post("build", makeNotif("n1")));
    CHECK(sys.totalActive() == 1);
}

TEST_CASE("NotificationSystem – post to unknown channel fails", "[phase44][NotificationSystem]") {
    NotificationSystem sys;
    CHECK_FALSE(sys.post("missing", makeNotif("n1")));
}

TEST_CASE("NotificationSystem – totalActive sums across channels", "[phase44][NotificationSystem]") {
    NotificationSystem sys;
    sys.createChannel("ch1");
    sys.createChannel("ch2");
    sys.post("ch1", makeNotif("a"));
    sys.post("ch1", makeNotif("b"));
    sys.post("ch2", makeNotif("c"));
    CHECK(sys.totalActive() == 3);
}

TEST_CASE("NotificationSystem – totalActive drops when dismissed", "[phase44][NotificationSystem]") {
    NotificationSystem sys;
    sys.createChannel("ch");
    sys.post("ch", makeNotif("n1"));
    sys.post("ch", makeNotif("n2"));
    sys.findChannel("ch")->dismiss("n1");
    CHECK(sys.totalActive() == 1);
}

// ─────────────────────────────────────────────────────────────────
// Integration
// ─────────────────────────────────────────────────────────────────

TEST_CASE("NotificationSystem integration – multi-channel error aggregation", "[phase44][integration]") {
    NotificationSystem sys;
    sys.createChannel("build");
    sys.createChannel("runtime");
    sys.createChannel("asset");

    sys.post("build",   makeNotif("b1", NotificationSeverity::Error));
    sys.post("build",   makeNotif("b2", NotificationSeverity::Warning));
    sys.post("runtime", makeNotif("r1", NotificationSeverity::Critical));
    sys.post("asset",   makeNotif("a1", NotificationSeverity::Info));

    CHECK(sys.totalActive() == 4);
    CHECK(sys.findChannel("build")->errorCount()   == 1);
    CHECK(sys.findChannel("runtime")->errorCount() == 1);
    CHECK(sys.findChannel("asset")->errorCount()   == 0);
}

TEST_CASE("NotificationSystem integration – dismiss and clearDismissed across channel", "[phase44][integration]") {
    NotificationSystem sys;
    sys.createChannel("main");

    sys.post("main", makeNotif("n1"));
    sys.post("main", makeNotif("n2"));
    sys.post("main", makeNotif("n3"));

    sys.findChannel("main")->dismiss("n1");
    sys.findChannel("main")->dismiss("n2");

    CHECK(sys.totalActive() == 1);
    size_t cleared = sys.findChannel("main")->clearDismissed();
    CHECK(cleared == 2);
    CHECK(sys.findChannel("main")->notificationCount() == 1);
}

TEST_CASE("NotificationSystem integration – persistent notification survives dismiss of others", "[phase44][integration]") {
    NotificationSystem sys;
    sys.createChannel("alerts");

    sys.post("alerts", makeNotif("pers",   NotificationSeverity::Critical, true));
    sys.post("alerts", makeNotif("normal", NotificationSeverity::Info,    false));

    // Both are active
    CHECK(sys.totalActive() == 2);

    // Dismiss non-persistent
    sys.findChannel("alerts")->dismiss("normal");
    CHECK(sys.totalActive() == 1);

    // Persistent one still visible
    Notification* p = sys.findChannel("alerts")->find("pers");
    REQUIRE(p != nullptr);
    CHECK(p->isVisible());
    CHECK(p->persistent);
}
