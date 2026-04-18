#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"

using namespace NF;

// ── NotificationType enum ───────────────────────────────────────

TEST_CASE("NotificationType enum values exist", "[Editor][S48]") {
    REQUIRE(static_cast<uint8_t>(NotificationType::Info) == 0);
    REQUIRE(static_cast<uint8_t>(NotificationType::Success) == 1);
    REQUIRE(static_cast<uint8_t>(NotificationType::Warning) == 2);
    REQUIRE(static_cast<uint8_t>(NotificationType::Error) == 3);
}

// ── EditorNotification ──────────────────────────────────────────

TEST_CASE("EditorNotification default values", "[Editor][S48]") {
    EditorNotification n;
    REQUIRE(n.type == NotificationType::Info);
    REQUIRE(n.message.empty());
    REQUIRE(n.ttl == 3.f);
    REQUIRE(n.elapsed == 0.f);
}

TEST_CASE("EditorNotification isExpired when elapsed >= ttl", "[Editor][S48]") {
    EditorNotification n;
    n.ttl = 2.f;
    n.elapsed = 1.9f;
    REQUIRE_FALSE(n.isExpired());

    n.elapsed = 2.0f;
    REQUIRE(n.isExpired());

    n.elapsed = 3.0f;
    REQUIRE(n.isExpired());
}

TEST_CASE("EditorNotification progress at various elapsed values", "[Editor][S48]") {
    EditorNotification n;
    n.ttl = 4.f;

    n.elapsed = 0.f;
    REQUIRE(n.progress() == Catch::Approx(0.f));

    n.elapsed = 1.f;
    REQUIRE(n.progress() == Catch::Approx(0.25f));

    n.elapsed = 2.f;
    REQUIRE(n.progress() == Catch::Approx(0.5f));

    n.elapsed = 4.f;
    REQUIRE(n.progress() == Catch::Approx(1.f));

    // Beyond ttl, clamped to 1.0
    n.elapsed = 10.f;
    REQUIRE(n.progress() == Catch::Approx(1.f));
}

TEST_CASE("EditorNotification progress with zero ttl", "[Editor][S48]") {
    EditorNotification n;
    n.ttl = 0.f;
    n.elapsed = 0.f;
    REQUIRE(n.progress() == Catch::Approx(1.f));
}

// ── NotificationQueue ───────────────────────────────────────────

TEST_CASE("NotificationQueue starts empty", "[Editor][S48]") {
    NotificationQueue q;
    REQUIRE_FALSE(q.hasActive());
    REQUIRE(q.count() == 0);
    REQUIRE(q.current() == nullptr);
}

TEST_CASE("NotificationQueue push adds notifications", "[Editor][S48]") {
    NotificationQueue q;
    q.push(NotificationType::Info, "Hello");
    q.push(NotificationType::Warning, "Careful");

    REQUIRE(q.count() == 2);
    REQUIRE(q.hasActive());
}

TEST_CASE("NotificationQueue current returns front item", "[Editor][S48]") {
    NotificationQueue q;
    q.push(NotificationType::Error, "Crash!", 5.f);
    q.push(NotificationType::Info, "Normal");

    auto* c = q.current();
    REQUIRE(c != nullptr);
    REQUIRE(c->type == NotificationType::Error);
    REQUIRE(c->message == "Crash!");
    REQUIRE(c->ttl == 5.f);
}

TEST_CASE("NotificationQueue tick advances elapsed and removes expired", "[Editor][S48]") {
    NotificationQueue q;
    q.push(NotificationType::Info, "Short", 1.f);
    q.push(NotificationType::Success, "Long", 10.f);

    q.tick(1.5f);
    // "Short" (ttl=1) should be expired and removed
    REQUIRE(q.count() == 1);
    REQUIRE(q.current()->message == "Long");
}

TEST_CASE("NotificationQueue tick removes all when all expired", "[Editor][S48]") {
    NotificationQueue q;
    q.push(NotificationType::Info, "A", 1.f);
    q.push(NotificationType::Info, "B", 2.f);

    q.tick(3.f);
    REQUIRE(q.count() == 0);
    REQUIRE_FALSE(q.hasActive());
    REQUIRE(q.current() == nullptr);
}

TEST_CASE("NotificationQueue clear removes everything", "[Editor][S48]") {
    NotificationQueue q;
    q.push(NotificationType::Info, "X");
    q.push(NotificationType::Warning, "Y");
    q.push(NotificationType::Error, "Z");
    REQUIRE(q.count() == 3);

    q.clear();
    REQUIRE(q.count() == 0);
    REQUIRE_FALSE(q.hasActive());
}

TEST_CASE("NotificationQueue push with custom ttl", "[Editor][S48]") {
    NotificationQueue q;
    q.push(NotificationType::Success, "Done", 0.5f);
    REQUIRE(q.current()->ttl == 0.5f);

    q.tick(0.6f);
    REQUIRE(q.count() == 0);
}
