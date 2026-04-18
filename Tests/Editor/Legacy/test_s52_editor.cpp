#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"

using namespace NF;

// ── NotificationAction enum ──────────────────────────────────────

TEST_CASE("NotificationAction names", "[Editor][S52]") {
    REQUIRE(std::string(notificationActionName(NotificationAction::Show)) == "Show");
    REQUIRE(std::string(notificationActionName(NotificationAction::AutoDismiss)) == "AutoDismiss");
    REQUIRE(std::string(notificationActionName(NotificationAction::Pin)) == "Pin");
    REQUIRE(std::string(notificationActionName(NotificationAction::Suppress)) == "Suppress");
    REQUIRE(std::string(notificationActionName(NotificationAction::Log)) == "Log");
    REQUIRE(std::string(notificationActionName(NotificationAction::Sound)) == "Sound");
    REQUIRE(std::string(notificationActionName(NotificationAction::Escalate)) == "Escalate");
}

// ── WorkflowRule ─────────────────────────────────────────────────

TEST_CASE("WorkflowRule matches severity", "[Editor][S52]") {
    WorkflowRule rule;
    rule.matchSeverity = NotificationSeverity::Error;
    rule.enabled = true;
    REQUIRE(rule.matches(NotificationSeverity::Error));
    REQUIRE_FALSE(rule.matches(NotificationSeverity::Info));
}

TEST_CASE("WorkflowRule disabled doesn't match", "[Editor][S52]") {
    WorkflowRule rule;
    rule.matchSeverity = NotificationSeverity::Error;
    rule.enabled = false;
    REQUIRE_FALSE(rule.matches(NotificationSeverity::Error));
}

// ── NotificationRateLimiter ──────────────────────────────────────

TEST_CASE("NotificationRateLimiter allows within limit", "[Editor][S52]") {
    NotificationRateLimiter limiter(3, 5.f);
    REQUIRE(limiter.shouldAllow());
    REQUIRE(limiter.shouldAllow());
    REQUIRE(limiter.shouldAllow());
    REQUIRE_FALSE(limiter.shouldAllow()); // over limit
    REQUIRE(limiter.isThrottled());
}

TEST_CASE("NotificationRateLimiter resets after window", "[Editor][S52]") {
    NotificationRateLimiter limiter(2, 1.f);
    limiter.shouldAllow();
    limiter.shouldAllow();
    REQUIRE(limiter.isThrottled());
    limiter.tick(1.5f); // past window
    REQUIRE_FALSE(limiter.isThrottled());
    REQUIRE(limiter.shouldAllow());
}

TEST_CASE("NotificationRateLimiter reset()", "[Editor][S52]") {
    NotificationRateLimiter limiter(2, 5.f);
    limiter.shouldAllow();
    limiter.shouldAllow();
    REQUIRE(limiter.isThrottled());
    limiter.reset();
    REQUIRE_FALSE(limiter.isThrottled());
    REQUIRE(limiter.recentCount() == 0);
}

TEST_CASE("NotificationRateLimiter accessors", "[Editor][S52]") {
    NotificationRateLimiter limiter(10, 5.f);
    REQUIRE(limiter.maxPerWindow() == 10);
    REQUIRE(limiter.windowSeconds() == 5.f);
    limiter.setMaxPerWindow(20);
    limiter.setWindowSeconds(10.f);
    REQUIRE(limiter.maxPerWindow() == 20);
    REQUIRE(limiter.windowSeconds() == 10.f);
}

// ── NotificationPriorityQueue ────────────────────────────────────

TEST_CASE("NotificationPriorityQueue starts empty", "[Editor][S52]") {
    NotificationPriorityQueue queue;
    REQUIRE(queue.empty());
    REQUIRE(queue.size() == 0);
}

TEST_CASE("NotificationPriorityQueue enqueue orders by severity", "[Editor][S52]") {
    NotificationPriorityQueue queue;
    Notification info;
    info.id = "info"; info.severity = NotificationSeverity::Info;
    Notification error;
    error.id = "error"; error.severity = NotificationSeverity::Error;
    Notification warning;
    warning.id = "warn"; warning.severity = NotificationSeverity::Warning;

    queue.enqueue(info);
    queue.enqueue(error);
    queue.enqueue(warning);

    REQUIRE(queue.size() == 3);
    REQUIRE(queue.top().id == "error"); // highest severity first
}

TEST_CASE("NotificationPriorityQueue dequeue", "[Editor][S52]") {
    NotificationPriorityQueue queue;
    Notification n;
    n.id = "n1"; n.severity = NotificationSeverity::Warning;
    queue.enqueue(n);
    auto dequeued = queue.dequeue();
    REQUIRE(dequeued.id == "n1");
    REQUIRE(queue.empty());
}

TEST_CASE("NotificationPriorityQueue at()", "[Editor][S52]") {
    NotificationPriorityQueue queue;
    Notification n;
    n.id = "n1"; n.severity = NotificationSeverity::Info;
    queue.enqueue(n);
    REQUIRE(queue.at(0) != nullptr);
    REQUIRE(queue.at(0)->id == "n1");
    REQUIRE(queue.at(1) == nullptr);
}

TEST_CASE("NotificationPriorityQueue clear", "[Editor][S52]") {
    NotificationPriorityQueue queue;
    Notification n;
    n.id = "n1";
    queue.enqueue(n);
    queue.clear();
    REQUIRE(queue.empty());
}

// ── NotificationWorkflowEngine ───────────────────────────────────

TEST_CASE("NotificationWorkflowEngine starts empty", "[Editor][S52]") {
    NotificationWorkflowEngine engine;
    REQUIRE(engine.ruleCount() == 0);
    REQUIRE(engine.processed() == 0);
    REQUIRE(engine.suppressed() == 0);
}

TEST_CASE("NotificationWorkflowEngine addRule", "[Editor][S52]") {
    NotificationWorkflowEngine engine;
    WorkflowRule rule;
    rule.name = "error_pin";
    rule.matchSeverity = NotificationSeverity::Error;
    rule.action = NotificationAction::Pin;
    REQUIRE(engine.addRule(rule));
    REQUIRE(engine.ruleCount() == 1);
}

TEST_CASE("NotificationWorkflowEngine addRule duplicate fails", "[Editor][S52]") {
    NotificationWorkflowEngine engine;
    WorkflowRule rule;
    rule.name = "rule1";
    engine.addRule(rule);
    REQUIRE_FALSE(engine.addRule(rule));
}

TEST_CASE("NotificationWorkflowEngine removeRule", "[Editor][S52]") {
    NotificationWorkflowEngine engine;
    WorkflowRule rule;
    rule.name = "rule1";
    engine.addRule(rule);
    REQUIRE(engine.removeRule("rule1"));
    REQUIRE(engine.ruleCount() == 0);
    REQUIRE_FALSE(engine.removeRule("nonexistent"));
}

TEST_CASE("NotificationWorkflowEngine findRule", "[Editor][S52]") {
    NotificationWorkflowEngine engine;
    WorkflowRule rule;
    rule.name = "myRule";
    engine.addRule(rule);
    REQUIRE(engine.findRule("myRule") != nullptr);
    REQUIRE(engine.findRule("nope") == nullptr);
}

TEST_CASE("NotificationWorkflowEngine enableRule", "[Editor][S52]") {
    NotificationWorkflowEngine engine;
    WorkflowRule rule;
    rule.name = "r1";
    rule.enabled = true;
    engine.addRule(rule);
    engine.enableRule("r1", false);
    REQUIRE_FALSE(engine.findRule("r1")->enabled);
}

TEST_CASE("NotificationWorkflowEngine processNotification applies rule", "[Editor][S52]") {
    NotificationWorkflowEngine engine;
    WorkflowRule rule;
    rule.name = "error_pin";
    rule.matchSeverity = NotificationSeverity::Error;
    rule.action = NotificationAction::Pin;
    engine.addRule(rule);

    Notification n;
    n.id = "n1";
    n.severity = NotificationSeverity::Error;
    auto action = engine.processNotification(n);
    REQUIRE(action == NotificationAction::Pin);
    REQUIRE(engine.processed() == 1);
}

TEST_CASE("NotificationWorkflowEngine processNotification default Show", "[Editor][S52]") {
    NotificationWorkflowEngine engine;
    Notification n;
    n.id = "n1";
    n.severity = NotificationSeverity::Info;
    auto action = engine.processNotification(n);
    REQUIRE(action == NotificationAction::Show);
}

TEST_CASE("NotificationWorkflowEngine rate limiting suppresses", "[Editor][S52]") {
    NotificationWorkflowEngine engine;
    engine.rateLimiter().setMaxPerWindow(2);
    engine.rateLimiter().setWindowSeconds(10.f);

    Notification n1, n2, n3;
    n1.id = "1"; n2.id = "2"; n3.id = "3";
    engine.processNotification(n1);
    engine.processNotification(n2);
    auto action = engine.processNotification(n3);
    REQUIRE(action == NotificationAction::Suppress);
    REQUIRE(engine.suppressed() == 1);
}

TEST_CASE("NotificationWorkflowEngine loadDefaults", "[Editor][S52]") {
    NotificationWorkflowEngine engine;
    engine.loadDefaults();
    REQUIRE(engine.ruleCount() == 7);
    REQUIRE(engine.findRule("info_auto_dismiss") != nullptr);
    REQUIRE(engine.findRule("error_pin") != nullptr);
    REQUIRE(engine.findRule("critical_pin") != nullptr);
    REQUIRE(engine.findRule("trace_suppress") != nullptr);
}

TEST_CASE("NotificationWorkflowEngine tick resets rate limiter", "[Editor][S52]") {
    NotificationWorkflowEngine engine;
    engine.rateLimiter().setMaxPerWindow(1);
    engine.rateLimiter().setWindowSeconds(1.f);
    Notification n;
    n.id = "n1";
    engine.processNotification(n);
    REQUIRE(engine.rateLimiter().isThrottled());
    engine.tick(2.f);
    REQUIRE_FALSE(engine.rateLimiter().isThrottled());
}

TEST_CASE("NotificationWorkflowEngine rules sorted by priority", "[Editor][S52]") {
    NotificationWorkflowEngine engine;
    engine.loadDefaults();
    const auto& rules = engine.rules();
    for (size_t i = 1; i < rules.size(); ++i) {
        REQUIRE(rules[i - 1].priority >= rules[i].priority);
    }
}

TEST_CASE("NotificationWorkflowEngine MAX_RULES limit", "[Editor][S52]") {
    NotificationWorkflowEngine engine;
    for (size_t i = 0; i < NotificationWorkflowEngine::MAX_RULES; ++i) {
        WorkflowRule r;
        r.name = "r" + std::to_string(i);
        engine.addRule(r);
    }
    WorkflowRule extra;
    extra.name = "extra";
    REQUIRE_FALSE(engine.addRule(extra));
}
