// Tests/Workspace/test_phase43_notifications.cpp
// Phase 43 — Notifications (NotificationQueue)
//
// Tests for:
//   1. NotificationType   — enum values
//   2. EditorNotification — isExpired, progress
//   3. NotificationQueue  — push, tick, current, hasActive, count, clear

#include <catch2/catch_test_macros.hpp>
#include "NF/Workspace/Notifications.h"
#include <string>

using namespace NF;

// ─────────────────────────────────────────────────────────────────
// 1. NotificationType
// ─────────────────────────────────────────────────────────────────

TEST_CASE("NotificationType – all four values exist", "[phase43][NotificationType]") {
    // Compile-time check: all enum values are reachable
    auto check = [](NotificationType t) -> int { return static_cast<int>(t); };
    CHECK(check(NotificationType::Info)    == 0);
    CHECK(check(NotificationType::Success) == 1);
    CHECK(check(NotificationType::Warning) == 2);
    CHECK(check(NotificationType::Error)   == 3);
}

// ─────────────────────────────────────────────────────────────────
// 2. EditorNotification
// ─────────────────────────────────────────────────────────────────

TEST_CASE("EditorNotification – default is not expired", "[phase43][EditorNotification]") {
    EditorNotification n;
    CHECK_FALSE(n.isExpired());
    CHECK(n.elapsed == 0.f);
    CHECK(n.ttl     == 3.f);
}

TEST_CASE("EditorNotification – isExpired when elapsed >= ttl", "[phase43][EditorNotification]") {
    EditorNotification n;
    n.ttl     = 2.f;
    n.elapsed = 2.f;
    CHECK(n.isExpired());
}

TEST_CASE("EditorNotification – not expired when elapsed < ttl", "[phase43][EditorNotification]") {
    EditorNotification n;
    n.ttl     = 2.f;
    n.elapsed = 1.9f;
    CHECK_FALSE(n.isExpired());
}

TEST_CASE("EditorNotification – progress 0 at start, 1 at expiry", "[phase43][EditorNotification]") {
    EditorNotification n;
    n.ttl     = 4.f;
    n.elapsed = 0.f;
    CHECK(n.progress() == 0.f);
    n.elapsed = 2.f;
    CHECK(n.progress() == 0.5f);
    n.elapsed = 4.f;
    CHECK(n.progress() == 1.f);
}

TEST_CASE("EditorNotification – progress capped at 1 when over-elapsed", "[phase43][EditorNotification]") {
    EditorNotification n;
    n.ttl     = 1.f;
    n.elapsed = 999.f;
    CHECK(n.progress() == 1.f);
}

TEST_CASE("EditorNotification – zero ttl yields progress 1", "[phase43][EditorNotification]") {
    EditorNotification n;
    n.ttl     = 0.f;
    n.elapsed = 0.f;
    CHECK(n.progress() == 1.f);
}

// ─────────────────────────────────────────────────────────────────
// 3. NotificationQueue
// ─────────────────────────────────────────────────────────────────

TEST_CASE("NotificationQueue – default empty", "[phase43][NotificationQueue]") {
    NotificationQueue q;
    CHECK(q.count()     == 0);
    CHECK_FALSE(q.hasActive());
    CHECK(q.current()   == nullptr);
}

TEST_CASE("NotificationQueue – push adds notification", "[phase43][NotificationQueue]") {
    NotificationQueue q;
    q.push(NotificationType::Info, "Hello", 3.f);
    CHECK(q.count()   == 1);
    CHECK(q.hasActive());
}

TEST_CASE("NotificationQueue – current returns first notification", "[phase43][NotificationQueue]") {
    NotificationQueue q;
    q.push(NotificationType::Warning, "Watch out!", 5.f);
    const EditorNotification* n = q.current();
    REQUIRE(n != nullptr);
    CHECK(n->type    == NotificationType::Warning);
    CHECK(n->message == "Watch out!");
    CHECK(n->ttl     == 5.f);
}

TEST_CASE("NotificationQueue – push multiple, count reflects all", "[phase43][NotificationQueue]") {
    NotificationQueue q;
    q.push(NotificationType::Info,    "a", 1.f);
    q.push(NotificationType::Success, "b", 2.f);
    q.push(NotificationType::Error,   "c", 3.f);
    CHECK(q.count() == 3);
}

TEST_CASE("NotificationQueue – tick advances elapsed", "[phase43][NotificationQueue]") {
    NotificationQueue q;
    q.push(NotificationType::Info, "msg", 2.f);
    q.tick(1.f);
    const EditorNotification* n = q.current();
    REQUIRE(n != nullptr);
    CHECK(n->elapsed == 1.f);
    CHECK_FALSE(n->isExpired());
}

TEST_CASE("NotificationQueue – tick removes expired notifications", "[phase43][NotificationQueue]") {
    NotificationQueue q;
    q.push(NotificationType::Info, "short", 1.f);
    q.push(NotificationType::Info, "long",  5.f);
    CHECK(q.count() == 2);
    q.tick(1.5f);  // first expires (1.5 >= 1.0)
    CHECK(q.count() == 1);
    CHECK(q.current()->message == "long");
}

TEST_CASE("NotificationQueue – tick removes all when all expire", "[phase43][NotificationQueue]") {
    NotificationQueue q;
    q.push(NotificationType::Info, "a", 1.f);
    q.push(NotificationType::Info, "b", 2.f);
    q.tick(3.f);
    CHECK(q.count() == 0);
    CHECK_FALSE(q.hasActive());
    CHECK(q.current() == nullptr);
}

TEST_CASE("NotificationQueue – clear empties queue", "[phase43][NotificationQueue]") {
    NotificationQueue q;
    q.push(NotificationType::Error, "err", 10.f);
    q.push(NotificationType::Info,  "info", 10.f);
    q.clear();
    CHECK(q.count()    == 0);
    CHECK_FALSE(q.hasActive());
}

TEST_CASE("NotificationQueue – push uses default ttl of 3", "[phase43][NotificationQueue]") {
    NotificationQueue q;
    q.push(NotificationType::Info, "default ttl");
    const EditorNotification* n = q.current();
    REQUIRE(n != nullptr);
    CHECK(n->ttl == 3.f);
}

// ─────────────────────────────────────────────────────────────────
// Integration
// ─────────────────────────────────────────────────────────────────

TEST_CASE("NotificationQueue integration – FIFO ordering preserved through ticks", "[phase43][integration]") {
    NotificationQueue q;
    q.push(NotificationType::Info,    "first",  2.f);
    q.push(NotificationType::Warning, "second", 5.f);
    q.push(NotificationType::Error,   "third",  8.f);

    // Before any tick: current is first
    CHECK(q.current()->message == "first");

    // Expire first
    q.tick(2.5f);
    REQUIRE(q.count() == 2);
    CHECK(q.current()->message == "second");

    // Expire second
    q.tick(3.f);
    REQUIRE(q.count() == 1);
    CHECK(q.current()->message == "third");
}

TEST_CASE("NotificationQueue integration – progress tracking for active notification", "[phase43][integration]") {
    NotificationQueue q;
    q.push(NotificationType::Success, "build complete", 4.f);
    q.tick(1.f);
    CHECK(q.current()->progress() == 0.25f);
    q.tick(1.f);
    CHECK(q.current()->progress() == 0.5f);
    q.tick(2.f);
    // After 4s total the notification expires and is removed
    CHECK(q.count() == 0);
}
