// Tests/Workspace/test_phase4_atlasai.cpp
// Phase 4 — AtlasAI and Codex Integration tests.
//
// Tests for:
//   1. BrokerFlowController — formal broker→reasoner→action flow
//   2. BuildLogRouter — build-log routing into AtlasAI
//   3. SnippetPromotionRules — validation, dedup, promotion rules
//   4. CodexSnippetMirror — enhanced validation/dedup integration

#include <catch2/catch_test_macros.hpp>
// Pull in DockSlot + EditorTheme before AIAssistantPanel (transitive dep)
#include "NF/Workspace/SelectionService.h"
#include "NF/Editor/EditorTheme.h"
#include "NF/Workspace/BrokerFlowController.h"
#include "NF/Workspace/BuildLogRouter.h"
#include "NF/Workspace/SnippetPromotionRules.h"
#include "NF/Workspace/CodexSnippetMirror.h"

using namespace NF;

// ═════════════════════════════════════════════════════════════════
// SECTION 1: BrokerFlowController
// ═════════════════════════════════════════════════════════════════

// ── BrokerFlowStage helpers ──────────────────────────────────────

TEST_CASE("brokerFlowStageName returns correct strings", "[BrokerFlowController][stage]") {
    CHECK(std::string(brokerFlowStageName(BrokerFlowStage::Idle))      == "Idle");
    CHECK(std::string(brokerFlowStageName(BrokerFlowStage::Indexing))  == "Indexing");
    CHECK(std::string(brokerFlowStageName(BrokerFlowStage::Reasoning)) == "Reasoning");
    CHECK(std::string(brokerFlowStageName(BrokerFlowStage::Surfacing)) == "Surfacing");
    CHECK(std::string(brokerFlowStageName(BrokerFlowStage::Complete))  == "Complete");
    CHECK(std::string(brokerFlowStageName(BrokerFlowStage::Error))     == "Error");
}

// ── BrokerFlowController — unbound ──────────────────────────────

TEST_CASE("BrokerFlowController default state is Idle and unbound", "[BrokerFlowController]") {
    BrokerFlowController ctrl;
    CHECK(ctrl.currentStage() == BrokerFlowStage::Idle);
    CHECK(ctrl.flowsCompleted() == 0u);
    CHECK(ctrl.totalViolations() == 0u);
    CHECK_FALSE(ctrl.isBound());
}

TEST_CASE("BrokerFlowController processEvent fails if not bound", "[BrokerFlowController]") {
    BrokerFlowController ctrl;
    ChangeEvent event;
    event.tool = "test";
    event.eventType = ChangeEventType::AssetImported;
    event.path = "test.glb";
    PipelineDirectories dirs;
    auto result = ctrl.processEvent(event, dirs);
    CHECK_FALSE(result.success);
    CHECK(result.finalStage == BrokerFlowStage::Error);
}

// ── BrokerFlowController — bound flow ───────────────────────────

TEST_CASE("BrokerFlowController processes event through full flow", "[BrokerFlowController]") {
    WorkspaceBroker broker;
    AtlasAIReasoner reasoner;
    AIActionSurface surface;
    NotificationSystem notifs;
    AtlasAIIntegration aiInteg;
    aiInteg.init();

    reasoner.loadDefaultRules();

    std::string sessionId = broker.createSession("TestProject");

    BrokerFlowController ctrl;
    ctrl.bind(&broker, &reasoner, &surface, &notifs, &aiInteg);
    ctrl.setSessionId(sessionId);
    CHECK(ctrl.isBound());

    // Create a contract issue event that should trigger rules
    ChangeEvent event;
    event.tool      = "ContractScanner";
    event.eventType = ChangeEventType::ContractIssue;
    event.path      = "src/main.cpp";
    event.timestamp = 12345;

    PipelineDirectories dirs;
    dirs.changes = "/tmp/test_broker_flow_changes";

    auto result = ctrl.processEvent(event, dirs);
    CHECK(result.success);
    CHECK(result.finalStage == BrokerFlowStage::Complete);
    CHECK(result.eventPath == "src/main.cpp");
    CHECK(result.eventType == ChangeEventType::ContractIssue);
    CHECK(result.violationCount > 0u);
    CHECK(ctrl.flowsCompleted() == 1u);
    CHECK(ctrl.totalViolations() > 0u);
}

TEST_CASE("BrokerFlowController surfaces violations as actions", "[BrokerFlowController]") {
    WorkspaceBroker broker;
    AtlasAIReasoner reasoner;
    AIActionSurface surface;
    NotificationSystem notifs;

    reasoner.loadDefaultRules();
    std::string sid = broker.createSession("Test");

    BrokerFlowController ctrl;
    ctrl.bind(&broker, &reasoner, &surface, &notifs);
    ctrl.setSessionId(sid);

    ChangeEvent event;
    event.tool      = "ContractScanner";
    event.eventType = ChangeEventType::ContractIssue;
    event.path      = "src/game.cpp";
    event.timestamp = 100;

    PipelineDirectories dirs;
    dirs.changes = "/tmp/test_broker_actions";

    auto result = ctrl.processEvent(event, dirs);
    CHECK(result.actionsProposed > 0u);
    CHECK(surface.totalCount() > 0u);
}

TEST_CASE("BrokerFlowController posts notifications for errors", "[BrokerFlowController]") {
    WorkspaceBroker broker;
    AtlasAIReasoner reasoner;
    AIActionSurface surface;
    NotificationSystem notifs;

    reasoner.loadDefaultRules();
    std::string sid = broker.createSession("Test");

    BrokerFlowConfig cfg;
    cfg.notifyMinSeverity = RuleSeverity::Error;

    BrokerFlowController ctrl;
    ctrl.bind(&broker, &reasoner, &surface, &notifs);
    ctrl.setSessionId(sid);
    ctrl.setConfig(cfg);

    ChangeEvent event;
    event.tool      = "ContractScanner";
    event.eventType = ChangeEventType::ContractIssue;
    event.path      = "src/main.cpp";
    event.timestamp = 100;

    PipelineDirectories dirs;
    dirs.changes = "/tmp/test_broker_notifs";

    auto result = ctrl.processEvent(event, dirs);
    // ContractIssue rules are severity Error, so should produce notifications
    CHECK(result.notificationsPosted > 0u);
    CHECK(notifs.totalActive() > 0u);
}

TEST_CASE("BrokerFlowController config disables auto-surface", "[BrokerFlowController]") {
    WorkspaceBroker broker;
    AtlasAIReasoner reasoner;
    AIActionSurface surface;
    NotificationSystem notifs;

    reasoner.loadDefaultRules();
    std::string sid = broker.createSession("Test");

    BrokerFlowConfig cfg;
    cfg.autoSurfaceActions = false;
    cfg.autoNotify = false;

    BrokerFlowController ctrl;
    ctrl.bind(&broker, &reasoner, &surface, &notifs);
    ctrl.setSessionId(sid);
    ctrl.setConfig(cfg);

    ChangeEvent event;
    event.tool      = "ContractScanner";
    event.eventType = ChangeEventType::ContractIssue;
    event.path      = "src/test.cpp";
    event.timestamp = 100;

    PipelineDirectories dirs;
    dirs.changes = "/tmp/test_broker_cfg";

    auto result = ctrl.processEvent(event, dirs);
    CHECK(result.success);
    CHECK(result.actionsProposed == 0u);
    CHECK(result.notificationsPosted == 0u);
}

TEST_CASE("BrokerFlowController feeds insights to AtlasAI", "[BrokerFlowController]") {
    WorkspaceBroker broker;
    AtlasAIReasoner reasoner;
    AIActionSurface surface;
    NotificationSystem notifs;
    AtlasAIIntegration aiInteg;
    aiInteg.init();

    reasoner.loadDefaultRules();
    std::string sid = broker.createSession("Test");

    BrokerFlowController ctrl;
    ctrl.bind(&broker, &reasoner, &surface, &notifs, &aiInteg);
    ctrl.setSessionId(sid);

    CHECK(aiInteg.totalEvents() == 0u);

    ChangeEvent event;
    event.tool      = "Editor";
    event.eventType = ChangeEventType::AssetImported;
    event.path      = "models/tree.glb";
    event.timestamp = 100;

    PipelineDirectories dirs;
    dirs.changes = "/tmp/test_broker_insights";

    ctrl.processEvent(event, dirs);
    CHECK(aiInteg.totalEvents() > 0u);
}

TEST_CASE("BrokerFlowController sessionId roundtrip", "[BrokerFlowController]") {
    BrokerFlowController ctrl;
    ctrl.setSessionId("session_42");
    CHECK(ctrl.sessionId() == "session_42");
}

TEST_CASE("BrokerFlowController multiple events accumulate stats", "[BrokerFlowController]") {
    WorkspaceBroker broker;
    AtlasAIReasoner reasoner;
    AIActionSurface surface;
    NotificationSystem notifs;

    reasoner.loadDefaultRules();
    std::string sid = broker.createSession("Test");

    BrokerFlowController ctrl;
    ctrl.bind(&broker, &reasoner, &surface, &notifs);
    ctrl.setSessionId(sid);

    PipelineDirectories dirs;
    dirs.changes = "/tmp/test_broker_multi";

    for (int i = 0; i < 5; ++i) {
        ChangeEvent event;
        event.tool      = "Editor";
        event.eventType = ChangeEventType::ContractIssue;
        event.path      = "src/file" + std::to_string(i) + ".cpp";
        event.timestamp = 100 + i;
        ctrl.processEvent(event, dirs);
    }
    CHECK(ctrl.flowsCompleted() == 5u);
    CHECK(ctrl.totalViolations() > 0u);
}

// ═════════════════════════════════════════════════════════════════
// SECTION 2: BuildLogRouter
// ═════════════════════════════════════════════════════════════════

// ── BuildLogClass helpers ────────────────────────────────────────

TEST_CASE("buildLogClassName returns correct strings", "[BuildLogRouter][class]") {
    CHECK(std::string(buildLogClassName(BuildLogClass::CompileError))   == "CompileError");
    CHECK(std::string(buildLogClassName(BuildLogClass::CompileWarning)) == "CompileWarning");
    CHECK(std::string(buildLogClassName(BuildLogClass::LinkerError))    == "LinkerError");
    CHECK(std::string(buildLogClassName(BuildLogClass::LinkerWarning))  == "LinkerWarning");
    CHECK(std::string(buildLogClassName(BuildLogClass::ConfigError))    == "ConfigError");
    CHECK(std::string(buildLogClassName(BuildLogClass::TestFailure))    == "TestFailure");
    CHECK(std::string(buildLogClassName(BuildLogClass::BuildSuccess))   == "BuildSuccess");
    CHECK(std::string(buildLogClassName(BuildLogClass::Other))          == "Other");
}

// ── BuildLogRouter — classify ────────────────────────────────────

TEST_CASE("BuildLogRouter classify detects compile error", "[BuildLogRouter][classify]") {
    LogEntry entry;
    entry.level = LogLevel::Error;
    entry.message = "src/main.cpp:42: error: undeclared identifier";
    CHECK(BuildLogRouter::classify(entry) == BuildLogClass::CompileError);
}

TEST_CASE("BuildLogRouter classify detects linker error", "[BuildLogRouter][classify]") {
    LogEntry entry;
    entry.level = LogLevel::Error;
    entry.message = "undefined reference to `foo::bar()`";
    CHECK(BuildLogRouter::classify(entry) == BuildLogClass::LinkerError);
}

TEST_CASE("BuildLogRouter classify detects test failure", "[BuildLogRouter][classify]") {
    LogEntry entry;
    entry.level = LogLevel::Error;
    entry.message = "test case FAILED: assertion failed";
    CHECK(BuildLogRouter::classify(entry) == BuildLogClass::TestFailure);
}

TEST_CASE("BuildLogRouter classify detects config error", "[BuildLogRouter][classify]") {
    LogEntry entry;
    entry.level = LogLevel::Error;
    entry.message = "cmake config failed";
    CHECK(BuildLogRouter::classify(entry) == BuildLogClass::ConfigError);
}

TEST_CASE("BuildLogRouter classify detects compile warning", "[BuildLogRouter][classify]") {
    LogEntry entry;
    entry.level = LogLevel::Warn;
    entry.message = "unused variable 'x'";
    CHECK(BuildLogRouter::classify(entry) == BuildLogClass::CompileWarning);
}

TEST_CASE("BuildLogRouter classify detects linker warning", "[BuildLogRouter][classify]") {
    LogEntry entry;
    entry.level = LogLevel::Warn;
    entry.message = "link warning: duplicate symbol";
    CHECK(BuildLogRouter::classify(entry) == BuildLogClass::LinkerWarning);
}

TEST_CASE("BuildLogRouter classify detects build success", "[BuildLogRouter][classify]") {
    LogEntry entry;
    entry.level = LogLevel::Info;
    entry.message = "Build complete: all targets succeeded";
    CHECK(BuildLogRouter::classify(entry) == BuildLogClass::BuildSuccess);
}

// ── BuildLogRouter — extract ─────────────────────────────────────

TEST_CASE("BuildLogRouter extractFilePath from compiler message", "[BuildLogRouter][extract]") {
    CHECK(BuildLogRouter::extractFilePath("src/main.cpp:42: error: bad") == "src/main.cpp");
}

TEST_CASE("BuildLogRouter extractFilePath returns empty for non-path", "[BuildLogRouter][extract]") {
    CHECK(BuildLogRouter::extractFilePath("some random message").empty());
}

TEST_CASE("BuildLogRouter extractLineNumber from compiler message", "[BuildLogRouter][extract]") {
    CHECK(BuildLogRouter::extractLineNumber("src/main.cpp:42: error: bad") == 42);
}

TEST_CASE("BuildLogRouter extractLineNumber returns 0 for non-match", "[BuildLogRouter][extract]") {
    CHECK(BuildLogRouter::extractLineNumber("no line info here") == 0);
}

// ── BuildLogRouter — install/capture ─────────────────────────────

TEST_CASE("BuildLogRouter default state is not installed", "[BuildLogRouter]") {
    BuildLogRouter router;
    CHECK_FALSE(router.isInstalled());
    CHECK(router.capturedCount() == 0u);
}

TEST_CASE("BuildLogRouter install without binding fails", "[BuildLogRouter]") {
    BuildLogRouter router;
    CHECK_FALSE(router.install());
}

TEST_CASE("BuildLogRouter install succeeds with bound logger", "[BuildLogRouter]") {
    LoggingRouteV1 logger;
    BuildLogRouter router;
    router.bind(&logger);
    CHECK(router.install());
    CHECK(router.isInstalled());
}

TEST_CASE("BuildLogRouter double install fails", "[BuildLogRouter]") {
    LoggingRouteV1 logger;
    BuildLogRouter router;
    router.bind(&logger);
    CHECK(router.install());
    CHECK_FALSE(router.install());
}

TEST_CASE("BuildLogRouter captures build errors", "[BuildLogRouter]") {
    LoggingRouteV1 logger;
    BuildLogRouter router;
    router.bind(&logger);
    router.install();

    logger.error("build", "src/foo.cpp:10: error: syntax error");
    CHECK(router.capturedCount() == 1u);
    CHECK(router.errorCount() == 1u);
    CHECK(router.warningCount() == 0u);
}

TEST_CASE("BuildLogRouter captures build warnings", "[BuildLogRouter]") {
    LoggingRouteV1 logger;
    BuildLogRouter router;
    router.bind(&logger);
    router.install();

    logger.warn("build", "unused variable");
    CHECK(router.capturedCount() == 1u);
    CHECK(router.warningCount() == 1u);
    CHECK(router.errorCount() == 0u);
}

TEST_CASE("BuildLogRouter ignores non-build tags", "[BuildLogRouter]") {
    LoggingRouteV1 logger;
    BuildLogRouter router;
    router.bind(&logger);
    router.install();

    logger.error("rendering", "GPU driver error");
    CHECK(router.capturedCount() == 0u);
}

TEST_CASE("BuildLogRouter routes errors to AtlasAI", "[BuildLogRouter]") {
    LoggingRouteV1 logger;
    AtlasAIIntegration ai;
    ai.init();

    BuildLogRouter router;
    router.bind(&logger, &ai);
    router.install();

    CHECK(ai.totalEvents() == 0u);
    logger.error("build", "src/bar.cpp:5: error: missing semicolon");
    CHECK(router.aiRoutedCount() == 1u);
    CHECK(ai.totalEvents() > 0u);
}

TEST_CASE("BuildLogRouter does not route warnings to AtlasAI by default", "[BuildLogRouter]") {
    LoggingRouteV1 logger;
    AtlasAIIntegration ai;
    ai.init();

    BuildLogRouter router;
    router.bind(&logger, &ai);
    router.install();

    logger.warn("build", "unused parameter");
    CHECK(router.aiRoutedCount() == 0u);
}

TEST_CASE("BuildLogRouter entries list is accessible", "[BuildLogRouter]") {
    LoggingRouteV1 logger;
    BuildLogRouter router;
    router.bind(&logger);
    router.install();

    logger.error("build", "error one");
    logger.warn("build", "warning one");
    CHECK(router.entries().size() == 2u);
}

TEST_CASE("BuildLogRouter errors() filters correctly", "[BuildLogRouter]") {
    LoggingRouteV1 logger;
    BuildLogRouter router;
    router.bind(&logger);
    router.install();

    logger.error("build", "error one");
    logger.warn("build", "warning one");
    logger.error("build", "error two");
    CHECK(router.errors().size() == 2u);
    CHECK(router.warnings().size() == 1u);
}

TEST_CASE("BuildLogRouter clearEntries resets all counters", "[BuildLogRouter]") {
    LoggingRouteV1 logger;
    BuildLogRouter router;
    router.bind(&logger);
    router.install();

    logger.error("build", "error");
    logger.warn("build", "warn");
    router.clearEntries();
    CHECK(router.capturedCount() == 0u);
    CHECK(router.errorCount() == 0u);
    CHECK(router.warningCount() == 0u);
    CHECK(router.entries().empty());
}

TEST_CASE("BuildLogRouter uninstall works", "[BuildLogRouter]") {
    LoggingRouteV1 logger;
    BuildLogRouter router;
    router.bind(&logger);
    router.install();
    CHECK(router.uninstall());
    CHECK_FALSE(router.isInstalled());

    // After uninstall, new logs should not be captured
    logger.error("build", "should not capture");
    CHECK(router.capturedCount() == 0u);
}

TEST_CASE("BuildLogRouter ring buffer enforces maxEntries", "[BuildLogRouter]") {
    LoggingRouteV1 logger;
    BuildLogRouter router;
    BuildLogRouterConfig cfg;
    cfg.maxEntries = 3;
    router.setConfig(cfg);
    router.bind(&logger);
    router.install();

    logger.error("build", "err1");
    logger.error("build", "err2");
    logger.error("build", "err3");
    logger.error("build", "err4");
    CHECK(router.entries().size() == 3u);
    CHECK(router.entries().front().message == "err2"); // oldest evicted
}

// ═════════════════════════════════════════════════════════════════
// SECTION 3: SnippetPromotionRules — Validation
// ═════════════════════════════════════════════════════════════════

TEST_CASE("snippetValidationStatusName returns correct strings", "[SnippetPromotion][validation]") {
    CHECK(std::string(snippetValidationStatusName(SnippetValidationStatus::Valid))            == "Valid");
    CHECK(std::string(snippetValidationStatusName(SnippetValidationStatus::EmptyBody))        == "EmptyBody");
    CHECK(std::string(snippetValidationStatusName(SnippetValidationStatus::BodyTooLong))      == "BodyTooLong");
    CHECK(std::string(snippetValidationStatusName(SnippetValidationStatus::TitleTooLong))     == "TitleTooLong");
    CHECK(std::string(snippetValidationStatusName(SnippetValidationStatus::InvalidLanguage))  == "InvalidLanguage");
    CHECK(std::string(snippetValidationStatusName(SnippetValidationStatus::DuplicateContent)) == "DuplicateContent");
    CHECK(std::string(snippetValidationStatusName(SnippetValidationStatus::TooManyTags))      == "TooManyTags");
}

TEST_CASE("SnippetPromotionRules validates a good snippet", "[SnippetPromotion][validation]") {
    SnippetPromotionRules rules;
    CodexSnippet s;
    s.id = 1;
    s.title = "Hello World";
    s.body = "std::cout << \"Hello World\";";
    s.language = SnippetLanguage::Cpp;
    auto result = rules.validate(s);
    CHECK(result.isValid());
}

TEST_CASE("SnippetPromotionRules rejects empty body", "[SnippetPromotion][validation]") {
    SnippetPromotionRules rules;
    CodexSnippet s;
    s.id = 1;
    s.title = "Empty";
    s.body = "";
    auto result = rules.validate(s);
    CHECK_FALSE(result.isValid());
    CHECK(result.status == SnippetValidationStatus::EmptyBody);
}

TEST_CASE("SnippetPromotionRules rejects body too long", "[SnippetPromotion][validation]") {
    SnippetPromotionRules rules;
    SnippetValidationLimits limits;
    limits.maxBodyLength = 10;
    rules.setLimits(limits);

    CodexSnippet s;
    s.id = 1;
    s.title = "Long";
    s.body = "12345678901"; // 11 chars
    auto result = rules.validate(s);
    CHECK_FALSE(result.isValid());
    CHECK(result.status == SnippetValidationStatus::BodyTooLong);
}

TEST_CASE("SnippetPromotionRules rejects title too long", "[SnippetPromotion][validation]") {
    SnippetPromotionRules rules;
    SnippetValidationLimits limits;
    limits.maxTitleLength = 5;
    rules.setLimits(limits);

    CodexSnippet s;
    s.id = 1;
    s.title = "Too Long Title";
    s.body = "body";
    auto result = rules.validate(s);
    CHECK_FALSE(result.isValid());
    CHECK(result.status == SnippetValidationStatus::TitleTooLong);
}

TEST_CASE("SnippetPromotionRules rejects too many tags", "[SnippetPromotion][validation]") {
    SnippetPromotionRules rules;
    SnippetValidationLimits limits;
    limits.maxTags = 2;
    rules.setLimits(limits);

    CodexSnippet s;
    s.id = 1;
    s.title = "Tags";
    s.body = "body";
    s.tags = {"a", "b", "c"};
    auto result = rules.validate(s);
    CHECK_FALSE(result.isValid());
    CHECK(result.status == SnippetValidationStatus::TooManyTags);
}

// ═════════════════════════════════════════════════════════════════
// SECTION 4: SnippetPromotionRules — Deduplication
// ═════════════════════════════════════════════════════════════════

TEST_CASE("fnv1aHash produces consistent results", "[SnippetPromotion][dedup]") {
    CHECK(fnv1aHash("hello") == fnv1aHash("hello"));
    CHECK(fnv1aHash("hello") != fnv1aHash("world"));
}

TEST_CASE("fnv1aHash empty string has known hash", "[SnippetPromotion][dedup]") {
    CHECK(fnv1aHash("") == 14695981039346656037ULL);
}

TEST_CASE("SnippetPromotionRules tracks and finds duplicates", "[SnippetPromotion][dedup]") {
    SnippetPromotionRules rules;
    rules.trackSnippet(1, "int x = 42;");
    CHECK(rules.findDuplicate("int x = 42;") == 1u);
    CHECK(rules.findDuplicate("int y = 99;") == 0u);
    CHECK(rules.trackedCount() == 1u);
}

TEST_CASE("SnippetPromotionRules untrack removes entry", "[SnippetPromotion][dedup]") {
    SnippetPromotionRules rules;
    rules.trackSnippet(1, "code");
    rules.untrackSnippet(1, "code");
    CHECK(rules.findDuplicate("code") == 0u);
    CHECK(rules.trackedCount() == 0u);
}

TEST_CASE("SnippetPromotionRules validate detects duplicate content", "[SnippetPromotion][dedup]") {
    SnippetPromotionRules rules;
    rules.trackSnippet(1, "shared code");

    CodexSnippet s;
    s.id = 2; // different id
    s.title = "Copy";
    s.body = "shared code";
    auto result = rules.validate(s);
    CHECK_FALSE(result.isValid());
    CHECK(result.status == SnippetValidationStatus::DuplicateContent);
    CHECK(result.duplicateOfId == 1u);
}

TEST_CASE("SnippetPromotionRules same id is not a duplicate of itself", "[SnippetPromotion][dedup]") {
    SnippetPromotionRules rules;
    rules.trackSnippet(1, "self code");

    CodexSnippet s;
    s.id = 1;
    s.title = "Self";
    s.body = "self code";
    auto result = rules.validate(s);
    CHECK(result.isValid()); // same id, not duplicate
}

// ═════════════════════════════════════════════════════════════════
// SECTION 5: SnippetPromotionRules — Rules
// ═════════════════════════════════════════════════════════════════

TEST_CASE("promotionTriggerName returns correct strings", "[SnippetPromotion][rules]") {
    CHECK(std::string(promotionTriggerName(PromotionTrigger::Manual))       == "Manual");
    CHECK(std::string(promotionTriggerName(PromotionTrigger::AutoOnSave))   == "AutoOnSave");
    CHECK(std::string(promotionTriggerName(PromotionTrigger::AutoOnUse))    == "AutoOnUse");
    CHECK(std::string(promotionTriggerName(PromotionTrigger::AutoOnReview)) == "AutoOnReview");
}

TEST_CASE("SnippetPromotionRules addRule and findRule", "[SnippetPromotion][rules]") {
    SnippetPromotionRules rules;
    PromotionRule r;
    r.id = 1;
    r.name = "Test rule";
    r.trigger = PromotionTrigger::Manual;
    CHECK(rules.addRule(r));
    CHECK(rules.ruleCount() == 1u);
    CHECK(rules.findRule(1) != nullptr);
    CHECK(rules.findRule(1)->name == "Test rule");
}

TEST_CASE("SnippetPromotionRules rejects duplicate rule id", "[SnippetPromotion][rules]") {
    SnippetPromotionRules rules;
    PromotionRule r;
    r.id = 1;
    r.name = "Rule A";
    CHECK(rules.addRule(r));
    r.name = "Rule B";
    CHECK_FALSE(rules.addRule(r));
    CHECK(rules.ruleCount() == 1u);
}

TEST_CASE("SnippetPromotionRules rejects invalid rule", "[SnippetPromotion][rules]") {
    SnippetPromotionRules rules;
    PromotionRule r; // id=0, name=""
    CHECK_FALSE(rules.addRule(r));
}

TEST_CASE("SnippetPromotionRules removeRule", "[SnippetPromotion][rules]") {
    SnippetPromotionRules rules;
    PromotionRule r;
    r.id = 1;
    r.name = "Remove me";
    rules.addRule(r);
    CHECK(rules.removeRule(1));
    CHECK(rules.ruleCount() == 0u);
    CHECK_FALSE(rules.removeRule(1)); // already removed
}

TEST_CASE("SnippetPromotionRules loadDefaults loads 4 rules", "[SnippetPromotion][rules]") {
    SnippetPromotionRules rules;
    rules.loadDefaults();
    CHECK(rules.ruleCount() == 4u);
}

// ── Evaluate promotion ───────────────────────────────────────────

TEST_CASE("SnippetPromotionRules evaluatePromotion matches Manual trigger", "[SnippetPromotion][evaluate]") {
    SnippetPromotionRules rules;
    rules.loadDefaults();

    CodexSnippet s;
    s.id = 1;
    s.title = "Test";
    s.body = "code";
    s.language = SnippetLanguage::Cpp;

    auto matched = rules.evaluatePromotion(s, PromotionTrigger::Manual);
    CHECK(!matched.empty());
    // Default rule 1 is Manual + Any
    bool hasRule1 = false;
    for (auto id : matched) if (id == 1) hasRule1 = true;
    CHECK(hasRule1);
}

TEST_CASE("SnippetPromotionRules evaluatePromotion matches AutoOnSave for C++", "[SnippetPromotion][evaluate]") {
    SnippetPromotionRules rules;
    rules.loadDefaults();

    CodexSnippet s;
    s.id = 1;
    s.title = "Cpp snippet";
    s.body = "int x = 0;";
    s.language = SnippetLanguage::Cpp;

    auto matched = rules.evaluatePromotion(s, PromotionTrigger::AutoOnSave);
    CHECK(!matched.empty());
}

TEST_CASE("SnippetPromotionRules evaluatePromotion AutoOnSave skips non-Cpp", "[SnippetPromotion][evaluate]") {
    SnippetPromotionRules rules;
    rules.loadDefaults();

    CodexSnippet s;
    s.id = 1;
    s.title = "Lua snippet";
    s.body = "x = 0";
    s.language = SnippetLanguage::Lua;

    auto matched = rules.evaluatePromotion(s, PromotionTrigger::AutoOnSave);
    CHECK(matched.empty()); // default rule 2 only matches Cpp
}

TEST_CASE("SnippetPromotionRules evaluatePromotion AutoOnUse requires min uses", "[SnippetPromotion][evaluate]") {
    SnippetPromotionRules rules;
    rules.loadDefaults();

    CodexSnippet s;
    s.id = 1;
    s.title = "Reusable";
    s.body = "code";
    s.language = SnippetLanguage::Cpp;

    auto matched2 = rules.evaluatePromotion(s, PromotionTrigger::AutoOnUse, 2);
    CHECK(matched2.empty()); // needs 3 uses

    auto matched3 = rules.evaluatePromotion(s, PromotionTrigger::AutoOnUse, 3);
    CHECK(!matched3.empty());
}

TEST_CASE("SnippetPromotionRules evaluatePromotion AutoOnReview requires pinned", "[SnippetPromotion][evaluate]") {
    SnippetPromotionRules rules;
    rules.loadDefaults();

    CodexSnippet unpinned;
    unpinned.id = 1;
    unpinned.title = "Not pinned";
    unpinned.body = "code";
    unpinned.pinned = false;

    auto m1 = rules.evaluatePromotion(unpinned, PromotionTrigger::AutoOnReview);
    CHECK(m1.empty());

    CodexSnippet pinned;
    pinned.id = 2;
    pinned.title = "Pinned";
    pinned.body = "code";
    pinned.pinned = true;

    auto m2 = rules.evaluatePromotion(pinned, PromotionTrigger::AutoOnReview);
    CHECK(!m2.empty());
}

// ═════════════════════════════════════════════════════════════════
// SECTION 6: SnippetPromotionRules — Promote
// ═════════════════════════════════════════════════════════════════

TEST_CASE("SnippetPromotionRules promote transitions Local to Pending", "[SnippetPromotion][promote]") {
    CodexSnippetMirror mirror;
    CodexSnippet s;
    s.id = 1;
    s.title = "Promote me";
    s.body = "int x = 0;";
    s.syncState = SnippetSyncState::Local;
    mirror.addSnippet(s);

    SnippetPromotionRules rules;
    auto entry = rules.promote(mirror, 1);
    CHECK(entry.success);
    CHECK(entry.fromState == SnippetSyncState::Local);
    CHECK(entry.toState == SnippetSyncState::Pending);
    CHECK(mirror.findSnippet(1)->syncState == SnippetSyncState::Pending);
    CHECK(rules.promotionCount() == 1u);
}

TEST_CASE("SnippetPromotionRules promote transitions Modified to Pending", "[SnippetPromotion][promote]") {
    CodexSnippetMirror mirror;
    CodexSnippet s;
    s.id = 1;
    s.title = "Modified";
    s.body = "updated code";
    s.syncState = SnippetSyncState::Modified;
    mirror.addSnippet(s);

    SnippetPromotionRules rules;
    auto entry = rules.promote(mirror, 1);
    CHECK(entry.success);
    CHECK(entry.toState == SnippetSyncState::Pending);
}

TEST_CASE("SnippetPromotionRules promote fails for Synced snippet", "[SnippetPromotion][promote]") {
    CodexSnippetMirror mirror;
    CodexSnippet s;
    s.id = 1;
    s.title = "Synced";
    s.body = "synced code";
    s.syncState = SnippetSyncState::Synced;
    mirror.addSnippet(s);

    SnippetPromotionRules rules;
    auto entry = rules.promote(mirror, 1);
    CHECK_FALSE(entry.success);
}

TEST_CASE("SnippetPromotionRules promote fails for unknown snippet", "[SnippetPromotion][promote]") {
    CodexSnippetMirror mirror;
    SnippetPromotionRules rules;
    auto entry = rules.promote(mirror, 999);
    CHECK_FALSE(entry.success);
    CHECK(entry.detail.find("not found") != std::string::npos);
}

TEST_CASE("SnippetPromotionRules promote fails validation for empty body", "[SnippetPromotion][promote]") {
    CodexSnippetMirror mirror;
    CodexSnippet s;
    s.id = 1;
    s.title = "Empty";
    s.body = "x"; // needs to pass addSnippet, but we can update body
    s.syncState = SnippetSyncState::Local;
    mirror.addSnippet(s);

    // Now update body to empty — but CodexSnippetMirror::updateBody rejects empty
    // So this snippet should still be valid. Let's test with a too-long body instead.
    SnippetPromotionRules rules;
    SnippetValidationLimits limits;
    limits.maxBodyLength = 0; // nothing is valid
    rules.setLimits(limits);

    auto entry = rules.promote(mirror, 1);
    CHECK_FALSE(entry.success);
    CHECK(entry.detail.find("Validation failed") != std::string::npos);
}

TEST_CASE("SnippetPromotionRules promotionLog records entries", "[SnippetPromotion][promote]") {
    CodexSnippetMirror mirror;
    CodexSnippet s;
    s.id = 1;
    s.title = "Log test";
    s.body = "code";
    s.syncState = SnippetSyncState::Local;
    mirror.addSnippet(s);

    SnippetPromotionRules rules;
    rules.promote(mirror, 1);
    CHECK(rules.promotionLog().size() == 1u);
    CHECK(rules.promotionLog()[0].success);

    rules.clearLog();
    CHECK(rules.promotionLog().empty());
}

// ═════════════════════════════════════════════════════════════════
// SECTION 7: Integration — CodexSnippetMirror with PromotionRules
// ═════════════════════════════════════════════════════════════════

TEST_CASE("Full snippet lifecycle: add, validate, track, promote", "[SnippetPromotion][integration]") {
    CodexSnippetMirror mirror;
    SnippetPromotionRules rules;
    rules.loadDefaults();

    // Add a C++ snippet
    CodexSnippet s;
    s.id = 1;
    s.title = "Matrix multiply";
    s.body = "void matmul(float* A, float* B, float* C, int N) { /* ... */ }";
    s.language = SnippetLanguage::Cpp;
    s.syncState = SnippetSyncState::Local;
    CHECK(mirror.addSnippet(s));

    // Track for dedup
    rules.trackSnippet(s.id, s.body);

    // Validate
    auto validation = rules.validate(s);
    CHECK(validation.isValid());

    // Evaluate promotion rules
    auto matchedRules = rules.evaluatePromotion(s, PromotionTrigger::AutoOnSave);
    CHECK(!matchedRules.empty());

    // Promote
    auto entry = rules.promote(mirror, s.id, matchedRules.front());
    CHECK(entry.success);
    CHECK(mirror.findSnippet(1)->syncState == SnippetSyncState::Pending);

    // Now adding a duplicate should be detected
    CodexSnippet dup;
    dup.id = 2;
    dup.title = "Duplicate matrix";
    dup.body = s.body;
    auto dupValidation = rules.validate(dup);
    CHECK_FALSE(dupValidation.isValid());
    CHECK(dupValidation.status == SnippetValidationStatus::DuplicateContent);
    CHECK(dupValidation.duplicateOfId == 1u);
}

TEST_CASE("Full flow: multiple snippets with dedup tracking", "[SnippetPromotion][integration]") {
    CodexSnippetMirror mirror;
    SnippetPromotionRules rules;

    // Add and track 3 unique snippets
    for (uint32_t i = 1; i <= 3; ++i) {
        CodexSnippet s;
        s.id = i;
        s.title = "Snippet " + std::to_string(i);
        s.body = "unique body " + std::to_string(i);
        s.language = SnippetLanguage::Cpp;
        s.syncState = SnippetSyncState::Local;
        mirror.addSnippet(s);
        rules.trackSnippet(s.id, s.body);
    }

    CHECK(rules.trackedCount() == 3u);
    CHECK(mirror.snippetCount() == 3u);

    // All should validate OK
    for (uint32_t i = 1; i <= 3; ++i) {
        auto* s = mirror.findSnippet(i);
        CHECK(rules.validate(*s).isValid());
    }

    // Remove and untrack snippet 2
    rules.untrackSnippet(2, "unique body 2");
    mirror.removeSnippet(2);
    CHECK(rules.trackedCount() == 2u);
    CHECK(mirror.snippetCount() == 2u);
}
