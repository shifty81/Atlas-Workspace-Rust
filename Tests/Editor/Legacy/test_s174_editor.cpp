// S174 editor tests: TelemetryEditorV1, AnalyticsDashboardV1, ExperimentEditorV1
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/TelemetryEditorV1.h"
#include "NF/Editor/AnalyticsDashboardV1.h"
#include "NF/Editor/ExperimentEditorV1.h"

using namespace NF;
using Catch::Approx;

// ── TelemetryEditorV1 ────────────────────────────────────────────────────────

TEST_CASE("Telv1EventRecord validity", "[Editor][S174]") {
    Telv1EventRecord e;
    REQUIRE(!e.isValid());
    e.id = 1; e.name = "AppLaunch";
    REQUIRE(e.isValid());
}

TEST_CASE("TelemetryEditorV1 addEvent and eventCount", "[Editor][S174]") {
    TelemetryEditorV1 tel;
    REQUIRE(tel.eventCount() == 0);
    Telv1EventRecord e; e.id = 1; e.name = "E1";
    REQUIRE(tel.addEvent(e));
    REQUIRE(tel.eventCount() == 1);
}

TEST_CASE("TelemetryEditorV1 addEvent invalid fails", "[Editor][S174]") {
    TelemetryEditorV1 tel;
    REQUIRE(!tel.addEvent(Telv1EventRecord{}));
}

TEST_CASE("TelemetryEditorV1 addEvent duplicate fails", "[Editor][S174]") {
    TelemetryEditorV1 tel;
    Telv1EventRecord e; e.id = 1; e.name = "A";
    tel.addEvent(e);
    REQUIRE(!tel.addEvent(e));
}

TEST_CASE("TelemetryEditorV1 removeEvent", "[Editor][S174]") {
    TelemetryEditorV1 tel;
    Telv1EventRecord e; e.id = 2; e.name = "B";
    tel.addEvent(e);
    REQUIRE(tel.removeEvent(2));
    REQUIRE(tel.eventCount() == 0);
    REQUIRE(!tel.removeEvent(2));
}

TEST_CASE("TelemetryEditorV1 countBySeverity", "[Editor][S174]") {
    TelemetryEditorV1 tel;
    Telv1EventRecord e1; e1.id = 1; e1.name = "A"; e1.severity = Telv1EventSeverity::Critical;
    Telv1EventRecord e2; e2.id = 2; e2.name = "B"; e2.severity = Telv1EventSeverity::Info;
    tel.addEvent(e1); tel.addEvent(e2);
    REQUIRE(tel.countBySeverity(Telv1EventSeverity::Critical) == 1);
    REQUIRE(tel.countBySeverity(Telv1EventSeverity::Info)     == 1);
}

TEST_CASE("TelemetryEditorV1 addSession and sessionCount", "[Editor][S174]") {
    TelemetryEditorV1 tel;
    REQUIRE(tel.sessionCount() == 0);
    Telv1Session s; s.id = 1; s.name = "Session1";
    REQUIRE(tel.addSession(s));
    REQUIRE(tel.sessionCount() == 1);
}

TEST_CASE("TelemetryEditorV1 addSession invalid fails", "[Editor][S174]") {
    TelemetryEditorV1 tel;
    REQUIRE(!tel.addSession(Telv1Session{}));
}

TEST_CASE("TelemetryEditorV1 removeSession", "[Editor][S174]") {
    TelemetryEditorV1 tel;
    Telv1Session s; s.id = 2; s.name = "B";
    tel.addSession(s);
    REQUIRE(tel.removeSession(2));
    REQUIRE(tel.sessionCount() == 0);
    REQUIRE(!tel.removeSession(2));
}

TEST_CASE("TelemetryEditorV1 setSessionState activeSessionCount", "[Editor][S174]") {
    TelemetryEditorV1 tel;
    Telv1Session s; s.id = 1; s.name = "A";
    tel.addSession(s);
    REQUIRE(tel.setSessionState(1, Telv1SessionState::Active));
    REQUIRE(tel.activeSessionCount() == 1);
    REQUIRE(tel.findSession(1)->isActive());
}

TEST_CASE("TelemetryEditorV1 session crashed state", "[Editor][S174]") {
    TelemetryEditorV1 tel;
    Telv1Session s; s.id = 1; s.name = "A";
    tel.addSession(s);
    tel.setSessionState(1, Telv1SessionState::Crashed);
    REQUIRE(tel.findSession(1)->hasCrashed());
}

TEST_CASE("TelemetryEditorV1 findEvent returns nullptr when missing", "[Editor][S174]") {
    TelemetryEditorV1 tel;
    REQUIRE(tel.findEvent(42) == nullptr);
}

TEST_CASE("Telv1EventRecord severity helpers", "[Editor][S174]") {
    Telv1EventRecord e; e.id = 1; e.name = "X"; e.severity = Telv1EventSeverity::Critical;
    REQUIRE(e.isCritical());
    e.severity = Telv1EventSeverity::Warning;
    REQUIRE(e.isWarning());
}

TEST_CASE("telv1EventSeverityName covers all values", "[Editor][S174]") {
    REQUIRE(std::string(telv1EventSeverityName(Telv1EventSeverity::Trace))    == "Trace");
    REQUIRE(std::string(telv1EventSeverityName(Telv1EventSeverity::Critical)) == "Critical");
}

TEST_CASE("telv1SessionStateName covers all values", "[Editor][S174]") {
    REQUIRE(std::string(telv1SessionStateName(Telv1SessionState::Unknown)) == "Unknown");
    REQUIRE(std::string(telv1SessionStateName(Telv1SessionState::Crashed)) == "Crashed");
}

TEST_CASE("TelemetryEditorV1 onEvent callback", "[Editor][S174]") {
    TelemetryEditorV1 tel;
    uint64_t notified = 0;
    tel.setOnEvent([&](uint64_t id) { notified = id; });
    Telv1Session s; s.id = 5; s.name = "E";
    tel.addSession(s);
    tel.setSessionState(5, Telv1SessionState::Active);
    REQUIRE(notified == 5);
}

// ── AnalyticsDashboardV1 ─────────────────────────────────────────────────────

TEST_CASE("Adv1Widget validity", "[Editor][S174]") {
    Adv1Widget w;
    REQUIRE(!w.isValid());
    w.id = 1; w.name = "DAUCounter";
    REQUIRE(w.isValid());
}

TEST_CASE("AnalyticsDashboardV1 addWidget and widgetCount", "[Editor][S174]") {
    AnalyticsDashboardV1 ad;
    REQUIRE(ad.widgetCount() == 0);
    Adv1Widget w; w.id = 1; w.name = "W1";
    REQUIRE(ad.addWidget(w));
    REQUIRE(ad.widgetCount() == 1);
}

TEST_CASE("AnalyticsDashboardV1 addWidget invalid fails", "[Editor][S174]") {
    AnalyticsDashboardV1 ad;
    REQUIRE(!ad.addWidget(Adv1Widget{}));
}

TEST_CASE("AnalyticsDashboardV1 addWidget duplicate fails", "[Editor][S174]") {
    AnalyticsDashboardV1 ad;
    Adv1Widget w; w.id = 1; w.name = "A";
    ad.addWidget(w);
    REQUIRE(!ad.addWidget(w));
}

TEST_CASE("AnalyticsDashboardV1 removeWidget", "[Editor][S174]") {
    AnalyticsDashboardV1 ad;
    Adv1Widget w; w.id = 2; w.name = "B";
    ad.addWidget(w);
    REQUIRE(ad.removeWidget(2));
    REQUIRE(ad.widgetCount() == 0);
    REQUIRE(!ad.removeWidget(2));
}

TEST_CASE("AnalyticsDashboardV1 addMetric and metricCount", "[Editor][S174]") {
    AnalyticsDashboardV1 ad;
    REQUIRE(ad.metricCount() == 0);
    Adv1Metric m; m.id = 1; m.name = "DAU";
    REQUIRE(ad.addMetric(m));
    REQUIRE(ad.metricCount() == 1);
}

TEST_CASE("AnalyticsDashboardV1 addMetric duplicate fails", "[Editor][S174]") {
    AnalyticsDashboardV1 ad;
    Adv1Metric m; m.id = 1; m.name = "A";
    ad.addMetric(m);
    REQUIRE(!ad.addMetric(m));
}

TEST_CASE("AnalyticsDashboardV1 removeMetric", "[Editor][S174]") {
    AnalyticsDashboardV1 ad;
    Adv1Metric m; m.id = 2; m.name = "B";
    ad.addMetric(m);
    REQUIRE(ad.removeMetric(2));
    REQUIRE(ad.metricCount() == 0);
    REQUIRE(!ad.removeMetric(2));
}

TEST_CASE("AnalyticsDashboardV1 countByWidgetType", "[Editor][S174]") {
    AnalyticsDashboardV1 ad;
    Adv1Widget w1; w1.id = 1; w1.name = "A"; w1.type = Adv1WidgetType::LineChart;
    Adv1Widget w2; w2.id = 2; w2.name = "B"; w2.type = Adv1WidgetType::PieChart;
    ad.addWidget(w1); ad.addWidget(w2);
    REQUIRE(ad.countByWidgetType(Adv1WidgetType::LineChart) == 1);
    REQUIRE(ad.countByWidgetType(Adv1WidgetType::PieChart)  == 1);
}

TEST_CASE("AnalyticsDashboardV1 countByAggregation", "[Editor][S174]") {
    AnalyticsDashboardV1 ad;
    Adv1Metric m1; m1.id = 1; m1.name = "A"; m1.aggregation = Adv1MetricAggregation::Sum;
    Adv1Metric m2; m2.id = 2; m2.name = "B"; m2.aggregation = Adv1MetricAggregation::P95;
    ad.addMetric(m1); ad.addMetric(m2);
    REQUIRE(ad.countByAggregation(Adv1MetricAggregation::Sum) == 1);
    REQUIRE(ad.countByAggregation(Adv1MetricAggregation::P95) == 1);
}

TEST_CASE("AnalyticsDashboardV1 setVisible", "[Editor][S174]") {
    AnalyticsDashboardV1 ad;
    Adv1Widget w; w.id = 1; w.name = "A"; w.visible = true;
    ad.addWidget(w);
    REQUIRE(ad.setVisible(1, false));
    REQUIRE(!ad.findWidget(1)->isVisible());
}

TEST_CASE("adv1WidgetTypeName covers all values", "[Editor][S174]") {
    REQUIRE(std::string(adv1WidgetTypeName(Adv1WidgetType::Counter)) == "Counter");
    REQUIRE(std::string(adv1WidgetTypeName(Adv1WidgetType::Table))   == "Table");
}

TEST_CASE("adv1MetricAggregationName covers all values", "[Editor][S174]") {
    REQUIRE(std::string(adv1MetricAggregationName(Adv1MetricAggregation::Sum)) == "Sum");
    REQUIRE(std::string(adv1MetricAggregationName(Adv1MetricAggregation::P95)) == "P95");
}

TEST_CASE("AnalyticsDashboardV1 onRefresh callback", "[Editor][S174]") {
    AnalyticsDashboardV1 ad;
    uint64_t notified = 0;
    ad.setOnRefresh([&](uint64_t id) { notified = id; });
    Adv1Widget w; w.id = 6; w.name = "F";
    ad.addWidget(w);
    ad.setVisible(6, false);
    REQUIRE(notified == 6);
}

// ── ExperimentEditorV1 ───────────────────────────────────────────────────────

TEST_CASE("Expv1Experiment validity", "[Editor][S174]") {
    Expv1Experiment e;
    REQUIRE(!e.isValid());
    e.id = 1; e.name = "ButtonColorTest";
    REQUIRE(e.isValid());
}

TEST_CASE("ExperimentEditorV1 addExperiment and experimentCount", "[Editor][S174]") {
    ExperimentEditorV1 exp;
    REQUIRE(exp.experimentCount() == 0);
    Expv1Experiment e; e.id = 1; e.name = "E1";
    REQUIRE(exp.addExperiment(e));
    REQUIRE(exp.experimentCount() == 1);
}

TEST_CASE("ExperimentEditorV1 addExperiment invalid fails", "[Editor][S174]") {
    ExperimentEditorV1 exp;
    REQUIRE(!exp.addExperiment(Expv1Experiment{}));
}

TEST_CASE("ExperimentEditorV1 addExperiment duplicate fails", "[Editor][S174]") {
    ExperimentEditorV1 exp;
    Expv1Experiment e; e.id = 1; e.name = "A";
    exp.addExperiment(e);
    REQUIRE(!exp.addExperiment(e));
}

TEST_CASE("ExperimentEditorV1 removeExperiment", "[Editor][S174]") {
    ExperimentEditorV1 exp;
    Expv1Experiment e; e.id = 2; e.name = "B";
    exp.addExperiment(e);
    REQUIRE(exp.removeExperiment(2));
    REQUIRE(exp.experimentCount() == 0);
    REQUIRE(!exp.removeExperiment(2));
}

TEST_CASE("ExperimentEditorV1 setState runningCount", "[Editor][S174]") {
    ExperimentEditorV1 exp;
    Expv1Experiment e; e.id = 1; e.name = "A";
    exp.addExperiment(e);
    REQUIRE(exp.setState(1, Expv1ExperimentState::Running));
    REQUIRE(exp.runningCount() == 1);
    REQUIRE(exp.findExperiment(1)->isRunning());
}

TEST_CASE("ExperimentEditorV1 concludedCount", "[Editor][S174]") {
    ExperimentEditorV1 exp;
    Expv1Experiment e; e.id = 1; e.name = "A";
    exp.addExperiment(e);
    exp.setState(1, Expv1ExperimentState::Concluded);
    REQUIRE(exp.concludedCount() == 1);
    REQUIRE(exp.findExperiment(1)->isConcluded());
}

TEST_CASE("ExperimentEditorV1 addVariant and countByVariantType", "[Editor][S174]") {
    ExperimentEditorV1 exp;
    Expv1Experiment e; e.id = 1; e.name = "A";
    exp.addExperiment(e);
    Expv1Variant v1; v1.name = "Control"; v1.type = Expv1VariantType::Control; v1.weight = 0.5f;
    Expv1Variant v2; v2.name = "Treat"; v2.type = Expv1VariantType::Treatment; v2.weight = 0.5f;
    REQUIRE(exp.addVariant(1, v1));
    REQUIRE(exp.addVariant(1, v2));
    REQUIRE(exp.countByVariantType(1, Expv1VariantType::Control)   == 1);
    REQUIRE(exp.countByVariantType(1, Expv1VariantType::Treatment) == 1);
}

TEST_CASE("ExperimentEditorV1 addVariant duplicate fails", "[Editor][S174]") {
    ExperimentEditorV1 exp;
    Expv1Experiment e; e.id = 1; e.name = "A";
    exp.addExperiment(e);
    Expv1Variant v; v.name = "Control"; v.type = Expv1VariantType::Control; v.weight = 0.5f;
    exp.addVariant(1, v);
    REQUIRE(!exp.addVariant(1, v));
}

TEST_CASE("Expv1Variant validity", "[Editor][S174]") {
    Expv1Variant v;
    REQUIRE(!v.isValid()); // weight = 0.5 but name empty
    v.name = "ctrl"; v.weight = 0.5f;
    REQUIRE(v.isValid());
}

TEST_CASE("expv1ExperimentStateName covers all values", "[Editor][S174]") {
    REQUIRE(std::string(expv1ExperimentStateName(Expv1ExperimentState::Draft))     == "Draft");
    REQUIRE(std::string(expv1ExperimentStateName(Expv1ExperimentState::Archived))  == "Archived");
}

TEST_CASE("expv1VariantTypeName covers all values", "[Editor][S174]") {
    REQUIRE(std::string(expv1VariantTypeName(Expv1VariantType::Control))   == "Control");
    REQUIRE(std::string(expv1VariantTypeName(Expv1VariantType::Holdout))   == "Holdout");
}

TEST_CASE("ExperimentEditorV1 findExperiment returns nullptr when missing", "[Editor][S174]") {
    ExperimentEditorV1 exp;
    REQUIRE(exp.findExperiment(99) == nullptr);
}

TEST_CASE("ExperimentEditorV1 onChange callback", "[Editor][S174]") {
    ExperimentEditorV1 exp;
    uint64_t notified = 0;
    exp.setOnChange([&](uint64_t id) { notified = id; });
    Expv1Experiment e; e.id = 3; e.name = "C";
    exp.addExperiment(e);
    exp.setState(3, Expv1ExperimentState::Running);
    REQUIRE(notified == 3);
}
