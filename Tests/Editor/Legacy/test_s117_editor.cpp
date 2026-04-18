// S117 editor tests: TelemetryEditor, AnalyticsDashboard, PlaytestRecorder
#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/PlaytestRecorder.h"
#include "NF/Editor/AnalyticsDashboard.h"
#include "NF/Editor/TelemetryEditor.h"

using namespace NF;

// ── TelemetryEditor ───────────────────────────────────────────────────────────

TEST_CASE("TelemetryEventType names", "[Editor][S117]") {
    REQUIRE(std::string(telemetryEventTypeName(TelemetryEventType::PlayerAction))  == "PlayerAction");
    REQUIRE(std::string(telemetryEventTypeName(TelemetryEventType::SessionStart))  == "SessionStart");
    REQUIRE(std::string(telemetryEventTypeName(TelemetryEventType::SessionEnd))    == "SessionEnd");
    REQUIRE(std::string(telemetryEventTypeName(TelemetryEventType::LevelLoad))     == "LevelLoad");
    REQUIRE(std::string(telemetryEventTypeName(TelemetryEventType::LevelComplete)) == "LevelComplete");
    REQUIRE(std::string(telemetryEventTypeName(TelemetryEventType::Purchase))      == "Purchase");
    REQUIRE(std::string(telemetryEventTypeName(TelemetryEventType::Error))         == "Error");
    REQUIRE(std::string(telemetryEventTypeName(TelemetryEventType::Performance))   == "Performance");
    REQUIRE(std::string(telemetryEventTypeName(TelemetryEventType::Custom))        == "Custom");
}

TEST_CASE("TelemetrySamplingRate names", "[Editor][S117]") {
    REQUIRE(std::string(telemetrySamplingRateName(TelemetrySamplingRate::Every))       == "Every");
    REQUIRE(std::string(telemetrySamplingRateName(TelemetrySamplingRate::EverySecond)) == "EverySecond");
    REQUIRE(std::string(telemetrySamplingRateName(TelemetrySamplingRate::EveryMinute)) == "EveryMinute");
    REQUIRE(std::string(telemetrySamplingRateName(TelemetrySamplingRate::OnChange))    == "OnChange");
    REQUIRE(std::string(telemetrySamplingRateName(TelemetrySamplingRate::Manual))      == "Manual");
}

TEST_CASE("TelemetryPrivacyLevel names", "[Editor][S117]") {
    REQUIRE(std::string(telemetryPrivacyLevelName(TelemetryPrivacyLevel::Anonymous))    == "Anonymous");
    REQUIRE(std::string(telemetryPrivacyLevelName(TelemetryPrivacyLevel::Pseudonymous)) == "Pseudonymous");
    REQUIRE(std::string(telemetryPrivacyLevelName(TelemetryPrivacyLevel::Identified))   == "Identified");
    REQUIRE(std::string(telemetryPrivacyLevelName(TelemetryPrivacyLevel::Restricted))   == "Restricted");
}

TEST_CASE("TelemetryEvent defaults", "[Editor][S117]") {
    TelemetryEvent te(1, "player_jump", TelemetryEventType::PlayerAction);
    REQUIRE(te.id()           == 1u);
    REQUIRE(te.name()         == "player_jump");
    REQUIRE(te.type()         == TelemetryEventType::PlayerAction);
    REQUIRE(te.samplingRate() == TelemetrySamplingRate::OnChange);
    REQUIRE(te.privacyLevel() == TelemetryPrivacyLevel::Anonymous);
    REQUIRE(te.isEnabled());
    REQUIRE(te.bufferSize()   == 256u);
}

TEST_CASE("TelemetryEvent mutation", "[Editor][S117]") {
    TelemetryEvent te(2, "frame_time", TelemetryEventType::Performance);
    te.setSamplingRate(TelemetrySamplingRate::EverySecond);
    te.setPrivacyLevel(TelemetryPrivacyLevel::Pseudonymous);
    te.setIsEnabled(false);
    te.setBufferSize(1024u);
    REQUIRE(te.samplingRate() == TelemetrySamplingRate::EverySecond);
    REQUIRE(te.privacyLevel() == TelemetryPrivacyLevel::Pseudonymous);
    REQUIRE(!te.isEnabled());
    REQUIRE(te.bufferSize()   == 1024u);
}

TEST_CASE("TelemetryEditor defaults", "[Editor][S117]") {
    TelemetryEditor ed;
    REQUIRE(!ed.isShowDisabled());
    REQUIRE(!ed.isShowPrivate());
    REQUIRE(ed.flushIntervalMs() == 5000u);
    REQUIRE(ed.eventCount()      == 0u);
}

TEST_CASE("TelemetryEditor add/remove events", "[Editor][S117]") {
    TelemetryEditor ed;
    REQUIRE(ed.addEvent(TelemetryEvent(1, "jump",    TelemetryEventType::PlayerAction)));
    REQUIRE(ed.addEvent(TelemetryEvent(2, "session", TelemetryEventType::SessionStart)));
    REQUIRE(ed.addEvent(TelemetryEvent(3, "fps",     TelemetryEventType::Performance)));
    REQUIRE(!ed.addEvent(TelemetryEvent(1, "jump",   TelemetryEventType::PlayerAction)));
    REQUIRE(ed.eventCount() == 3u);
    REQUIRE(ed.removeEvent(2));
    REQUIRE(ed.eventCount() == 2u);
    REQUIRE(!ed.removeEvent(99));
}

TEST_CASE("TelemetryEditor counts and find", "[Editor][S117]") {
    TelemetryEditor ed;
    TelemetryEvent e1(1, "jump_a",    TelemetryEventType::PlayerAction);
    TelemetryEvent e2(2, "jump_b",    TelemetryEventType::PlayerAction); e2.setPrivacyLevel(TelemetryPrivacyLevel::Pseudonymous);
    TelemetryEvent e3(3, "session_a", TelemetryEventType::SessionStart); e3.setIsEnabled(false);
    TelemetryEvent e4(4, "perf_a",    TelemetryEventType::Performance);  e4.setPrivacyLevel(TelemetryPrivacyLevel::Pseudonymous); e4.setIsEnabled(false);
    ed.addEvent(e1); ed.addEvent(e2); ed.addEvent(e3); ed.addEvent(e4);
    REQUIRE(ed.countByType(TelemetryEventType::PlayerAction)            == 2u);
    REQUIRE(ed.countByType(TelemetryEventType::SessionStart)            == 1u);
    REQUIRE(ed.countByType(TelemetryEventType::Purchase)                == 0u);
    REQUIRE(ed.countByPrivacyLevel(TelemetryPrivacyLevel::Anonymous)    == 2u);
    REQUIRE(ed.countByPrivacyLevel(TelemetryPrivacyLevel::Pseudonymous) == 2u);
    REQUIRE(ed.countEnabled()                                           == 2u);
    auto* found = ed.findEvent(3);
    REQUIRE(found != nullptr);
    REQUIRE(found->type() == TelemetryEventType::SessionStart);
    REQUIRE(ed.findEvent(99) == nullptr);
}

TEST_CASE("TelemetryEditor settings mutation", "[Editor][S117]") {
    TelemetryEditor ed;
    ed.setShowDisabled(true);
    ed.setShowPrivate(true);
    ed.setFlushIntervalMs(1000u);
    REQUIRE(ed.isShowDisabled());
    REQUIRE(ed.isShowPrivate());
    REQUIRE(ed.flushIntervalMs() == 1000u);
}

// ── AnalyticsDashboard ────────────────────────────────────────────────────────

TEST_CASE("AnalyticsMetricType names", "[Editor][S117]") {
    REQUIRE(std::string(analyticsMetricTypeName(AnalyticsMetricType::Counter))    == "Counter");
    REQUIRE(std::string(analyticsMetricTypeName(AnalyticsMetricType::Gauge))      == "Gauge");
    REQUIRE(std::string(analyticsMetricTypeName(AnalyticsMetricType::Histogram))  == "Histogram");
    REQUIRE(std::string(analyticsMetricTypeName(AnalyticsMetricType::Summary))    == "Summary");
    REQUIRE(std::string(analyticsMetricTypeName(AnalyticsMetricType::Rate))       == "Rate");
    REQUIRE(std::string(analyticsMetricTypeName(AnalyticsMetricType::Average))    == "Average");
    REQUIRE(std::string(analyticsMetricTypeName(AnalyticsMetricType::Percentile)) == "Percentile");
}

TEST_CASE("AnalyticsTimeRange names", "[Editor][S117]") {
    REQUIRE(std::string(analyticsTimeRangeName(AnalyticsTimeRange::LastHour))  == "LastHour");
    REQUIRE(std::string(analyticsTimeRangeName(AnalyticsTimeRange::LastDay))   == "LastDay");
    REQUIRE(std::string(analyticsTimeRangeName(AnalyticsTimeRange::LastWeek))  == "LastWeek");
    REQUIRE(std::string(analyticsTimeRangeName(AnalyticsTimeRange::LastMonth)) == "LastMonth");
    REQUIRE(std::string(analyticsTimeRangeName(AnalyticsTimeRange::AllTime))   == "AllTime");
    REQUIRE(std::string(analyticsTimeRangeName(AnalyticsTimeRange::Custom))    == "Custom");
}

TEST_CASE("AnalyticsChartType names", "[Editor][S117]") {
    REQUIRE(std::string(analyticsChartTypeName(AnalyticsChartType::Line))    == "Line");
    REQUIRE(std::string(analyticsChartTypeName(AnalyticsChartType::Bar))     == "Bar");
    REQUIRE(std::string(analyticsChartTypeName(AnalyticsChartType::Pie))     == "Pie");
    REQUIRE(std::string(analyticsChartTypeName(AnalyticsChartType::Scatter)) == "Scatter");
    REQUIRE(std::string(analyticsChartTypeName(AnalyticsChartType::Heatmap)) == "Heatmap");
    REQUIRE(std::string(analyticsChartTypeName(AnalyticsChartType::Funnel))  == "Funnel");
}

TEST_CASE("AnalyticsWidget defaults", "[Editor][S117]") {
    AnalyticsWidget w(1, "dau_counter", AnalyticsMetricType::Counter);
    REQUIRE(w.id()          == 1u);
    REQUIRE(w.name()        == "dau_counter");
    REQUIRE(w.metricType()  == AnalyticsMetricType::Counter);
    REQUIRE(w.chartType()   == AnalyticsChartType::Line);
    REQUIRE(w.timeRange()   == AnalyticsTimeRange::LastDay);
    REQUIRE(w.isVisible());
    REQUIRE(w.refreshHz()   == 1.0f);
}

TEST_CASE("AnalyticsWidget mutation", "[Editor][S117]") {
    AnalyticsWidget w(2, "fps_histogram", AnalyticsMetricType::Histogram);
    w.setChartType(AnalyticsChartType::Bar);
    w.setTimeRange(AnalyticsTimeRange::LastWeek);
    w.setIsVisible(false);
    w.setRefreshHz(0.5f);
    REQUIRE(w.chartType()   == AnalyticsChartType::Bar);
    REQUIRE(w.timeRange()   == AnalyticsTimeRange::LastWeek);
    REQUIRE(!w.isVisible());
    REQUIRE(w.refreshHz()   == 0.5f);
}

TEST_CASE("AnalyticsDashboard defaults", "[Editor][S117]") {
    AnalyticsDashboard db;
    REQUIRE(db.activeTimeRange()     == AnalyticsTimeRange::LastDay);
    REQUIRE(db.isShowGrid());
    REQUIRE(db.isAutoRefresh());
    REQUIRE(db.refreshIntervalSec()  == 30.0f);
    REQUIRE(db.widgetCount()         == 0u);
}

TEST_CASE("AnalyticsDashboard add/remove widgets", "[Editor][S117]") {
    AnalyticsDashboard db;
    REQUIRE(db.addWidget(AnalyticsWidget(1, "dau",    AnalyticsMetricType::Counter)));
    REQUIRE(db.addWidget(AnalyticsWidget(2, "fps",    AnalyticsMetricType::Gauge)));
    REQUIRE(db.addWidget(AnalyticsWidget(3, "errors", AnalyticsMetricType::Rate)));
    REQUIRE(!db.addWidget(AnalyticsWidget(1, "dau",   AnalyticsMetricType::Counter)));
    REQUIRE(db.widgetCount() == 3u);
    REQUIRE(db.removeWidget(2));
    REQUIRE(db.widgetCount() == 2u);
    REQUIRE(!db.removeWidget(99));
}

TEST_CASE("AnalyticsDashboard counts and find", "[Editor][S117]") {
    AnalyticsDashboard db;
    AnalyticsWidget w1(1, "counter_a", AnalyticsMetricType::Counter);
    AnalyticsWidget w2(2, "counter_b", AnalyticsMetricType::Counter); w2.setChartType(AnalyticsChartType::Bar);
    AnalyticsWidget w3(3, "gauge_a",   AnalyticsMetricType::Gauge);   w3.setIsVisible(false);
    AnalyticsWidget w4(4, "rate_a",    AnalyticsMetricType::Rate);    w4.setChartType(AnalyticsChartType::Bar); w4.setIsVisible(false);
    db.addWidget(w1); db.addWidget(w2); db.addWidget(w3); db.addWidget(w4);
    REQUIRE(db.countByMetricType(AnalyticsMetricType::Counter)  == 2u);
    REQUIRE(db.countByMetricType(AnalyticsMetricType::Gauge)    == 1u);
    REQUIRE(db.countByMetricType(AnalyticsMetricType::Summary)  == 0u);
    REQUIRE(db.countByChartType(AnalyticsChartType::Line)       == 2u);
    REQUIRE(db.countByChartType(AnalyticsChartType::Bar)        == 2u);
    REQUIRE(db.countVisible()                                   == 2u);
    auto* found = db.findWidget(3);
    REQUIRE(found != nullptr);
    REQUIRE(found->metricType() == AnalyticsMetricType::Gauge);
    REQUIRE(db.findWidget(99) == nullptr);
}

TEST_CASE("AnalyticsDashboard settings mutation", "[Editor][S117]") {
    AnalyticsDashboard db;
    db.setActiveTimeRange(AnalyticsTimeRange::LastWeek);
    db.setShowGrid(false);
    db.setAutoRefresh(false);
    db.setRefreshIntervalSec(60.0f);
    REQUIRE(db.activeTimeRange()    == AnalyticsTimeRange::LastWeek);
    REQUIRE(!db.isShowGrid());
    REQUIRE(!db.isAutoRefresh());
    REQUIRE(db.refreshIntervalSec() == 60.0f);
}

// ── PlaytestRecorder ──────────────────────────────────────────────────────────

TEST_CASE("PlaytestSessionState names", "[Editor][S117]") {
    REQUIRE(std::string(playtestSessionStateName(PlaytestSessionState::Idle))      == "Idle");
    REQUIRE(std::string(playtestSessionStateName(PlaytestSessionState::Recording)) == "Recording");
    REQUIRE(std::string(playtestSessionStateName(PlaytestSessionState::Paused))    == "Paused");
    REQUIRE(std::string(playtestSessionStateName(PlaytestSessionState::Reviewing)) == "Reviewing");
    REQUIRE(std::string(playtestSessionStateName(PlaytestSessionState::Exported))  == "Exported");
    REQUIRE(std::string(playtestSessionStateName(PlaytestSessionState::Archived))  == "Archived");
}

TEST_CASE("PlaytestCaptureMode names", "[Editor][S117]") {
    REQUIRE(std::string(playtestCaptureModeName(PlaytestCaptureMode::InputOnly))      == "InputOnly");
    REQUIRE(std::string(playtestCaptureModeName(PlaytestCaptureMode::InputAndScreen)) == "InputAndScreen");
    REQUIRE(std::string(playtestCaptureModeName(PlaytestCaptureMode::FullSession))    == "FullSession");
    REQUIRE(std::string(playtestCaptureModeName(PlaytestCaptureMode::EventStream))    == "EventStream");
    REQUIRE(std::string(playtestCaptureModeName(PlaytestCaptureMode::Annotated))      == "Annotated");
}

TEST_CASE("PlaytestAnnotationType names", "[Editor][S117]") {
    REQUIRE(std::string(playtestAnnotationTypeName(PlaytestAnnotationType::Bug))        == "Bug");
    REQUIRE(std::string(playtestAnnotationTypeName(PlaytestAnnotationType::Suggestion)) == "Suggestion");
    REQUIRE(std::string(playtestAnnotationTypeName(PlaytestAnnotationType::Highlight))  == "Highlight");
    REQUIRE(std::string(playtestAnnotationTypeName(PlaytestAnnotationType::Question))   == "Question");
    REQUIRE(std::string(playtestAnnotationTypeName(PlaytestAnnotationType::Blocker))    == "Blocker");
}

TEST_CASE("PlaytestSession defaults", "[Editor][S117]") {
    PlaytestSession ps(1, "session_alpha", PlaytestCaptureMode::InputAndScreen);
    REQUIRE(ps.id()              == 1u);
    REQUIRE(ps.name()            == "session_alpha");
    REQUIRE(ps.captureMode()     == PlaytestCaptureMode::InputAndScreen);
    REQUIRE(ps.state()           == PlaytestSessionState::Idle);
    REQUIRE(ps.durationSeconds() == 0.0f);
    REQUIRE(ps.annotationCount() == 0u);
    REQUIRE(!ps.isFlagged());
    REQUIRE(ps.testerName()      == "");
}

TEST_CASE("PlaytestSession mutation", "[Editor][S117]") {
    PlaytestSession ps(2, "session_beta", PlaytestCaptureMode::FullSession);
    ps.setState(PlaytestSessionState::Recording);
    ps.setDurationSeconds(1800.0f);
    ps.setAnnotationCount(12u);
    ps.setIsFlagged(true);
    ps.setTesterName("QA_Alice");
    REQUIRE(ps.state()           == PlaytestSessionState::Recording);
    REQUIRE(ps.durationSeconds() == 1800.0f);
    REQUIRE(ps.annotationCount() == 12u);
    REQUIRE(ps.isFlagged());
    REQUIRE(ps.testerName()      == "QA_Alice");
}

TEST_CASE("PlaytestRecorder defaults", "[Editor][S117]") {
    PlaytestRecorder rec;
    REQUIRE(rec.defaultCaptureMode()    == PlaytestCaptureMode::InputAndScreen);
    REQUIRE(rec.isShowTimeline());
    REQUIRE(rec.isShowAnnotations());
    REQUIRE(rec.maxSessionDurationSec() == 3600.0f);
    REQUIRE(rec.sessionCount()          == 0u);
}

TEST_CASE("PlaytestRecorder add/remove sessions", "[Editor][S117]") {
    PlaytestRecorder rec;
    REQUIRE(rec.addSession(PlaytestSession(1, "sess1", PlaytestCaptureMode::InputOnly)));
    REQUIRE(rec.addSession(PlaytestSession(2, "sess2", PlaytestCaptureMode::FullSession)));
    REQUIRE(rec.addSession(PlaytestSession(3, "sess3", PlaytestCaptureMode::Annotated)));
    REQUIRE(!rec.addSession(PlaytestSession(1, "sess1", PlaytestCaptureMode::InputOnly)));
    REQUIRE(rec.sessionCount() == 3u);
    REQUIRE(rec.removeSession(2));
    REQUIRE(rec.sessionCount() == 2u);
    REQUIRE(!rec.removeSession(99));
}

TEST_CASE("PlaytestRecorder counts and find", "[Editor][S117]") {
    PlaytestRecorder rec;
    PlaytestSession s1(1, "s_a", PlaytestCaptureMode::InputOnly);
    PlaytestSession s2(2, "s_b", PlaytestCaptureMode::InputOnly);   s2.setState(PlaytestSessionState::Exported);
    PlaytestSession s3(3, "s_c", PlaytestCaptureMode::FullSession);  s3.setIsFlagged(true);
    PlaytestSession s4(4, "s_d", PlaytestCaptureMode::Annotated);   s4.setState(PlaytestSessionState::Exported); s4.setIsFlagged(true);
    rec.addSession(s1); rec.addSession(s2); rec.addSession(s3); rec.addSession(s4);
    REQUIRE(rec.countByState(PlaytestSessionState::Idle)             == 2u);
    REQUIRE(rec.countByState(PlaytestSessionState::Exported)         == 2u);
    REQUIRE(rec.countByState(PlaytestSessionState::Archived)         == 0u);
    REQUIRE(rec.countByCaptureMode(PlaytestCaptureMode::InputOnly)   == 2u);
    REQUIRE(rec.countByCaptureMode(PlaytestCaptureMode::FullSession) == 1u);
    REQUIRE(rec.countFlagged()                                       == 2u);
    auto* found = rec.findSession(3);
    REQUIRE(found != nullptr);
    REQUIRE(found->captureMode() == PlaytestCaptureMode::FullSession);
    REQUIRE(rec.findSession(99) == nullptr);
}

TEST_CASE("PlaytestRecorder settings mutation", "[Editor][S117]") {
    PlaytestRecorder rec;
    rec.setDefaultCaptureMode(PlaytestCaptureMode::EventStream);
    rec.setShowTimeline(false);
    rec.setShowAnnotations(false);
    rec.setMaxSessionDurationSec(7200.0f);
    REQUIRE(rec.defaultCaptureMode()    == PlaytestCaptureMode::EventStream);
    REQUIRE(!rec.isShowTimeline());
    REQUIRE(!rec.isShowAnnotations());
    REQUIRE(rec.maxSessionDurationSec() == 7200.0f);
}
