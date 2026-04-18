#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"

using namespace NF;

// --- NotificationSeverity names ---

TEST_CASE("NotificationSeverity names cover all 8 values", "[Editor][S24]") {
    REQUIRE(std::string(notificationSeverityName(NotificationSeverity::Info))     == "Info");
    REQUIRE(std::string(notificationSeverityName(NotificationSeverity::Success))  == "Success");
    REQUIRE(std::string(notificationSeverityName(NotificationSeverity::Warning))  == "Warning");
    REQUIRE(std::string(notificationSeverityName(NotificationSeverity::Error))    == "Error");
    REQUIRE(std::string(notificationSeverityName(NotificationSeverity::Critical)) == "Critical");
    REQUIRE(std::string(notificationSeverityName(NotificationSeverity::Debug))    == "Debug");
    REQUIRE(std::string(notificationSeverityName(NotificationSeverity::Trace))    == "Trace");
    REQUIRE(std::string(notificationSeverityName(NotificationSeverity::System))   == "System");
}

// --- NotificationState names ---

TEST_CASE("NotificationState names cover all 4 values", "[Editor][S24]") {
    REQUIRE(std::string(notificationStateName(NotificationState::Pending))   == "Pending");
    REQUIRE(std::string(notificationStateName(NotificationState::Shown))     == "Shown");
    REQUIRE(std::string(notificationStateName(NotificationState::Dismissed)) == "Dismissed");
    REQUIRE(std::string(notificationStateName(NotificationState::Expired))   == "Expired");
}

// --- Notification state transitions ---

TEST_CASE("Notification show/dismiss/expire state transitions", "[Editor][S24]") {
    Notification n;
    n.id = "notif-1"; n.title = "Hello";

    REQUIRE(n.state == NotificationState::Pending);
    n.show();
    REQUIRE(n.state == NotificationState::Shown);
    n.dismiss();
    REQUIRE(n.state == NotificationState::Dismissed);
    n.expire();
    REQUIRE(n.state == NotificationState::Expired);
}

TEST_CASE("Notification isVisible only when Shown", "[Editor][S24]") {
    Notification n;
    n.id = "notif-2";

    REQUIRE_FALSE(n.isVisible());
    n.show();
    REQUIRE(n.isVisible());
    n.dismiss();
    REQUIRE_FALSE(n.isVisible());
}

TEST_CASE("Notification isError threshold is >= Error", "[Editor][S24]") {
    Notification n;
    n.id = "notif-3";

    n.severity = NotificationSeverity::Warning;
    REQUIRE_FALSE(n.isError());

    n.severity = NotificationSeverity::Error;
    REQUIRE(n.isError());

    n.severity = NotificationSeverity::Critical;
    REQUIRE(n.isError());
}

TEST_CASE("Notification isCritical only for Critical severity", "[Editor][S24]") {
    Notification n;
    n.id = "notif-4";

    n.severity = NotificationSeverity::Error;
    REQUIRE_FALSE(n.isCritical());

    n.severity = NotificationSeverity::Critical;
    REQUIRE(n.isCritical());

    n.severity = NotificationSeverity::System;
    REQUIRE_FALSE(n.isCritical());
}

// --- NotificationChannel ---

TEST_CASE("NotificationChannel post and duplicate rejection", "[Editor][S24]") {
    NotificationChannel ch("ui");
    Notification a; a.id = "a"; a.title = "A";
    Notification b; b.id = "b"; b.title = "B";
    Notification dup; dup.id = "a"; dup.title = "Dup";

    REQUIRE(ch.post(a));
    REQUIRE(ch.post(b));
    REQUIRE_FALSE(ch.post(dup));
    REQUIRE(ch.notificationCount() == 2);
}

TEST_CASE("NotificationChannel dismiss by id", "[Editor][S24]") {
    NotificationChannel ch("system");
    Notification n; n.id = "msg-1"; n.title = "Info";
    ch.post(n);

    REQUIRE(ch.dismiss("msg-1"));
    REQUIRE(ch.find("msg-1")->isDismissed());
    REQUIRE_FALSE(ch.dismiss("unknown"));
}

TEST_CASE("NotificationChannel activeCount counts Shown notifications", "[Editor][S24]") {
    NotificationChannel ch("hud");
    Notification a; a.id = "a";
    Notification b; b.id = "b";
    ch.post(a);
    ch.post(b);
    REQUIRE(ch.activeCount() == 2);

    ch.dismiss("a");
    REQUIRE(ch.activeCount() == 1);
}

TEST_CASE("NotificationChannel errorCount counts error+ notifications", "[Editor][S24]") {
    NotificationChannel ch("log");
    Notification info; info.id = "i1"; info.severity = NotificationSeverity::Info;
    Notification err;  err.id  = "e1"; err.severity  = NotificationSeverity::Error;
    Notification crit; crit.id = "c1"; crit.severity = NotificationSeverity::Critical;
    ch.post(info);
    ch.post(err);
    ch.post(crit);

    REQUIRE(ch.errorCount() == 2);
}

TEST_CASE("NotificationChannel clearDismissed removes Dismissed and Expired", "[Editor][S24]") {
    NotificationChannel ch("events");
    Notification a; a.id = "a";
    Notification b; b.id = "b";
    Notification c; c.id = "c";
    ch.post(a);
    ch.post(b);
    ch.post(c);

    ch.dismiss("a");
    ch.find("b")->expire();

    REQUIRE(ch.clearDismissed() == 2);
    REQUIRE(ch.notificationCount() == 1);
}

// --- NotificationSystem ---

TEST_CASE("NotificationSystem createChannel and duplicate rejection", "[Editor][S24]") {
    NotificationSystem sys;
    REQUIRE(sys.createChannel("ui")     != nullptr);
    REQUIRE(sys.createChannel("system") != nullptr);
    REQUIRE(sys.createChannel("ui")     == nullptr); // duplicate
    REQUIRE(sys.channelCount() == 2);
}

TEST_CASE("NotificationSystem post routes to channel", "[Editor][S24]") {
    NotificationSystem sys;
    sys.createChannel("alerts");

    Notification n; n.id = "alert-1"; n.title = "Test";
    REQUIRE(sys.post("alerts", n));
    REQUIRE_FALSE(sys.post("missing", n));

    auto* ch = sys.findChannel("alerts");
    REQUIRE(ch != nullptr);
    REQUIRE(ch->notificationCount() == 1);
}

TEST_CASE("NotificationSystem totalActive sums across channels", "[Editor][S24]") {
    NotificationSystem sys;
    sys.createChannel("ch1");
    sys.createChannel("ch2");

    Notification a; a.id = "a";
    Notification b; b.id = "b";
    Notification c; c.id = "c";
    sys.post("ch1", a);
    sys.post("ch1", b);
    sys.post("ch2", c);

    REQUIRE(sys.totalActive() == 3);

    sys.findChannel("ch1")->dismiss("a");
    REQUIRE(sys.totalActive() == 2);
}
