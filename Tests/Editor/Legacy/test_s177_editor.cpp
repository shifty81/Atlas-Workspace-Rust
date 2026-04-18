// S177 editor tests: BenchmarkSuiteEditorV1, LoadTestEditorV1, StressTestEditorV1
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/BenchmarkSuiteEditorV1.h"
#include "NF/Editor/LoadTestEditorV1.h"
#include "NF/Editor/StressTestEditorV1.h"

using namespace NF;
using Catch::Approx;

// ── BenchmarkSuiteEditorV1 ───────────────────────────────────────────────────

TEST_CASE("Bchv1Suite validity", "[Editor][S177]") {
    Bchv1Suite s;
    REQUIRE(!s.isValid());
    s.id = 1; s.name = "CPUSuite";
    REQUIRE(s.isValid());
}

TEST_CASE("BenchmarkSuiteEditorV1 addSuite and suiteCount", "[Editor][S177]") {
    BenchmarkSuiteEditorV1 bch;
    REQUIRE(bch.suiteCount() == 0);
    Bchv1Suite s; s.id = 1; s.name = "S1";
    REQUIRE(bch.addSuite(s));
    REQUIRE(bch.suiteCount() == 1);
}

TEST_CASE("BenchmarkSuiteEditorV1 addSuite invalid fails", "[Editor][S177]") {
    BenchmarkSuiteEditorV1 bch;
    REQUIRE(!bch.addSuite(Bchv1Suite{}));
}

TEST_CASE("BenchmarkSuiteEditorV1 addSuite duplicate fails", "[Editor][S177]") {
    BenchmarkSuiteEditorV1 bch;
    Bchv1Suite s; s.id = 1; s.name = "A";
    bch.addSuite(s);
    REQUIRE(!bch.addSuite(s));
}

TEST_CASE("BenchmarkSuiteEditorV1 removeSuite", "[Editor][S177]") {
    BenchmarkSuiteEditorV1 bch;
    Bchv1Suite s; s.id = 2; s.name = "B";
    bch.addSuite(s);
    REQUIRE(bch.removeSuite(2));
    REQUIRE(bch.suiteCount() == 0);
    REQUIRE(!bch.removeSuite(2));
}

TEST_CASE("BenchmarkSuiteEditorV1 addRun and runCount", "[Editor][S177]") {
    BenchmarkSuiteEditorV1 bch;
    Bchv1Run r; r.id = 1; r.suiteId = 10; r.name = "Run1";
    REQUIRE(bch.addRun(r));
    REQUIRE(bch.runCount() == 1);
}

TEST_CASE("BenchmarkSuiteEditorV1 addRun invalid fails", "[Editor][S177]") {
    BenchmarkSuiteEditorV1 bch;
    REQUIRE(!bch.addRun(Bchv1Run{}));
}

TEST_CASE("BenchmarkSuiteEditorV1 removeRun", "[Editor][S177]") {
    BenchmarkSuiteEditorV1 bch;
    Bchv1Run r; r.id = 1; r.suiteId = 10; r.name = "Run1";
    bch.addRun(r);
    REQUIRE(bch.removeRun(1));
    REQUIRE(bch.runCount() == 0);
    REQUIRE(!bch.removeRun(1));
}

TEST_CASE("BenchmarkSuiteEditorV1 setRunState completeCount failedCount", "[Editor][S177]") {
    BenchmarkSuiteEditorV1 bch;
    Bchv1Run r1; r1.id = 1; r1.suiteId = 1; r1.name = "A";
    Bchv1Run r2; r2.id = 2; r2.suiteId = 1; r2.name = "B";
    bch.addRun(r1); bch.addRun(r2);
    bch.setRunState(1, Bchv1RunState::Complete);
    bch.setRunState(2, Bchv1RunState::Failed);
    REQUIRE(bch.completeCount() == 1);
    REQUIRE(bch.failedCount()   == 1);
}

TEST_CASE("BenchmarkSuiteEditorV1 countByCategory", "[Editor][S177]") {
    BenchmarkSuiteEditorV1 bch;
    Bchv1Run r1; r1.id = 1; r1.suiteId = 1; r1.name = "A"; r1.category = Bchv1BenchmarkCategory::GPU;
    Bchv1Run r2; r2.id = 2; r2.suiteId = 1; r2.name = "B"; r2.category = Bchv1BenchmarkCategory::Memory;
    bch.addRun(r1); bch.addRun(r2);
    REQUIRE(bch.countByCategory(Bchv1BenchmarkCategory::GPU)    == 1);
    REQUIRE(bch.countByCategory(Bchv1BenchmarkCategory::Memory) == 1);
}

TEST_CASE("BenchmarkSuiteEditorV1 findSuite and findRun", "[Editor][S177]") {
    BenchmarkSuiteEditorV1 bch;
    Bchv1Suite s; s.id = 5; s.name = "S5";
    bch.addSuite(s);
    Bchv1Run r; r.id = 9; r.suiteId = 5; r.name = "R9";
    bch.addRun(r);
    REQUIRE(bch.findSuite(5) != nullptr);
    REQUIRE(bch.findSuite(5)->name == "S5");
    REQUIRE(bch.findRun(9) != nullptr);
    REQUIRE(bch.findRun(9)->name == "R9");
    REQUIRE(bch.findSuite(99) == nullptr);
    REQUIRE(bch.findRun(99)   == nullptr);
}

TEST_CASE("bchv1RunStateName covers all values", "[Editor][S177]") {
    REQUIRE(std::string(bchv1RunStateName(Bchv1RunState::Pending))  == "Pending");
    REQUIRE(std::string(bchv1RunStateName(Bchv1RunState::Complete)) == "Complete");
    REQUIRE(std::string(bchv1RunStateName(Bchv1RunState::Skipped))  == "Skipped");
}

TEST_CASE("bchv1BenchmarkCategoryName covers all values", "[Editor][S177]") {
    REQUIRE(std::string(bchv1BenchmarkCategoryName(Bchv1BenchmarkCategory::CPU))     == "CPU");
    REQUIRE(std::string(bchv1BenchmarkCategoryName(Bchv1BenchmarkCategory::Physics)) == "Physics");
}

TEST_CASE("Bchv1Run helpers", "[Editor][S177]") {
    Bchv1Run r; r.id = 1; r.suiteId = 1; r.name = "R";
    REQUIRE(r.isValid());
    r.state = Bchv1RunState::Complete;
    REQUIRE(r.isComplete());
    r.state = Bchv1RunState::Failed;
    REQUIRE(r.isFailed());
}

TEST_CASE("BenchmarkSuiteEditorV1 onChange callback", "[Editor][S177]") {
    BenchmarkSuiteEditorV1 bch;
    uint64_t notified = 0;
    bch.setOnChange([&](uint64_t id) { notified = id; });
    Bchv1Run r; r.id = 4; r.suiteId = 20; r.name = "D";
    bch.addRun(r);
    REQUIRE(notified == 20);
}

// ── LoadTestEditorV1 ─────────────────────────────────────────────────────────

TEST_CASE("Ldtv1Scenario validity", "[Editor][S177]") {
    Ldtv1Scenario s;
    REQUIRE(!s.isValid());
    s.id = 1; s.name = "SteadyLoad";
    REQUIRE(s.isValid());
}

TEST_CASE("LoadTestEditorV1 addScenario and scenarioCount", "[Editor][S177]") {
    LoadTestEditorV1 ldt;
    REQUIRE(ldt.scenarioCount() == 0);
    Ldtv1Scenario s; s.id = 1; s.name = "S1";
    REQUIRE(ldt.addScenario(s));
    REQUIRE(ldt.scenarioCount() == 1);
}

TEST_CASE("LoadTestEditorV1 addScenario invalid fails", "[Editor][S177]") {
    LoadTestEditorV1 ldt;
    REQUIRE(!ldt.addScenario(Ldtv1Scenario{}));
}

TEST_CASE("LoadTestEditorV1 addScenario duplicate fails", "[Editor][S177]") {
    LoadTestEditorV1 ldt;
    Ldtv1Scenario s; s.id = 1; s.name = "A";
    ldt.addScenario(s);
    REQUIRE(!ldt.addScenario(s));
}

TEST_CASE("LoadTestEditorV1 removeScenario", "[Editor][S177]") {
    LoadTestEditorV1 ldt;
    Ldtv1Scenario s; s.id = 2; s.name = "B";
    ldt.addScenario(s);
    REQUIRE(ldt.removeScenario(2));
    REQUIRE(ldt.scenarioCount() == 0);
    REQUIRE(!ldt.removeScenario(2));
}

TEST_CASE("LoadTestEditorV1 addResult", "[Editor][S177]") {
    LoadTestEditorV1 ldt;
    Ldtv1Result r; r.id = 1; r.scenarioId = 10; r.passed = true;
    REQUIRE(ldt.addResult(r));
}

TEST_CASE("LoadTestEditorV1 addResult invalid fails", "[Editor][S177]") {
    LoadTestEditorV1 ldt;
    REQUIRE(!ldt.addResult(Ldtv1Result{}));
}

TEST_CASE("LoadTestEditorV1 setState doneCount", "[Editor][S177]") {
    LoadTestEditorV1 ldt;
    Ldtv1Scenario s; s.id = 1; s.name = "A";
    ldt.addScenario(s);
    REQUIRE(ldt.setState(1, Ldtv1ScenarioState::Done));
    REQUIRE(ldt.doneCount() == 1);
    REQUIRE(ldt.findScenario(1)->isDone());
}

TEST_CASE("LoadTestEditorV1 failedCount (aborted)", "[Editor][S177]") {
    LoadTestEditorV1 ldt;
    Ldtv1Scenario s; s.id = 1; s.name = "A";
    ldt.addScenario(s);
    ldt.setState(1, Ldtv1ScenarioState::Aborted);
    REQUIRE(ldt.failedCount() == 1);
    REQUIRE(ldt.findScenario(1)->isAborted());
}

TEST_CASE("LoadTestEditorV1 countByProfile", "[Editor][S177]") {
    LoadTestEditorV1 ldt;
    Ldtv1Scenario s1; s1.id = 1; s1.name = "A"; s1.profile = Ldtv1LoadProfile::Ramp;
    Ldtv1Scenario s2; s2.id = 2; s2.name = "B"; s2.profile = Ldtv1LoadProfile::Spike;
    ldt.addScenario(s1); ldt.addScenario(s2);
    REQUIRE(ldt.countByProfile(Ldtv1LoadProfile::Ramp)  == 1);
    REQUIRE(ldt.countByProfile(Ldtv1LoadProfile::Spike) == 1);
}

TEST_CASE("LoadTestEditorV1 findScenario returns ptr", "[Editor][S177]") {
    LoadTestEditorV1 ldt;
    Ldtv1Scenario s; s.id = 7; s.name = "G";
    ldt.addScenario(s);
    REQUIRE(ldt.findScenario(7) != nullptr);
    REQUIRE(ldt.findScenario(7)->name == "G");
    REQUIRE(ldt.findScenario(99) == nullptr);
}

TEST_CASE("ldtv1ScenarioStateName covers all values", "[Editor][S177]") {
    REQUIRE(std::string(ldtv1ScenarioStateName(Ldtv1ScenarioState::Draft))   == "Draft");
    REQUIRE(std::string(ldtv1ScenarioStateName(Ldtv1ScenarioState::Aborted)) == "Aborted");
}

TEST_CASE("ldtv1LoadProfileName covers all values", "[Editor][S177]") {
    REQUIRE(std::string(ldtv1LoadProfileName(Ldtv1LoadProfile::Steady)) == "Steady");
    REQUIRE(std::string(ldtv1LoadProfileName(Ldtv1LoadProfile::Step))   == "Step");
}

TEST_CASE("LoadTestEditorV1 onChange callback", "[Editor][S177]") {
    LoadTestEditorV1 ldt;
    uint64_t notified = 0;
    ldt.setOnChange([&](uint64_t id) { notified = id; });
    Ldtv1Scenario s; s.id = 3; s.name = "C";
    ldt.addScenario(s);
    ldt.setState(3, Ldtv1ScenarioState::Running);
    REQUIRE(notified == 3);
}

// ── StressTestEditorV1 ───────────────────────────────────────────────────────

TEST_CASE("Sttv1TestCase validity", "[Editor][S177]") {
    Sttv1TestCase tc;
    REQUIRE(!tc.isValid());
    tc.id = 1; tc.name = "CPUStress";
    REQUIRE(tc.isValid());
}

TEST_CASE("StressTestEditorV1 addCase and caseCount", "[Editor][S177]") {
    StressTestEditorV1 stt;
    REQUIRE(stt.caseCount() == 0);
    Sttv1TestCase tc; tc.id = 1; tc.name = "C1";
    REQUIRE(stt.addCase(tc));
    REQUIRE(stt.caseCount() == 1);
}

TEST_CASE("StressTestEditorV1 addCase invalid fails", "[Editor][S177]") {
    StressTestEditorV1 stt;
    REQUIRE(!stt.addCase(Sttv1TestCase{}));
}

TEST_CASE("StressTestEditorV1 addCase duplicate fails", "[Editor][S177]") {
    StressTestEditorV1 stt;
    Sttv1TestCase tc; tc.id = 1; tc.name = "A";
    stt.addCase(tc);
    REQUIRE(!stt.addCase(tc));
}

TEST_CASE("StressTestEditorV1 removeCase", "[Editor][S177]") {
    StressTestEditorV1 stt;
    Sttv1TestCase tc; tc.id = 2; tc.name = "B";
    stt.addCase(tc);
    REQUIRE(stt.removeCase(2));
    REQUIRE(stt.caseCount() == 0);
    REQUIRE(!stt.removeCase(2));
}

TEST_CASE("StressTestEditorV1 addThreshold and thresholdCount", "[Editor][S177]") {
    StressTestEditorV1 stt;
    Sttv1Threshold th; th.id = 1; th.caseId = 10; th.metricName = "FPS"; th.limitValue = 60.f;
    REQUIRE(stt.addThreshold(th));
    REQUIRE(stt.thresholdCount() == 1);
}

TEST_CASE("StressTestEditorV1 addThreshold invalid fails", "[Editor][S177]") {
    StressTestEditorV1 stt;
    REQUIRE(!stt.addThreshold(Sttv1Threshold{}));
}

TEST_CASE("StressTestEditorV1 removeThreshold", "[Editor][S177]") {
    StressTestEditorV1 stt;
    Sttv1Threshold th; th.id = 1; th.caseId = 5; th.metricName = "Mem";
    stt.addThreshold(th);
    REQUIRE(stt.removeThreshold(1));
    REQUIRE(stt.thresholdCount() == 0);
    REQUIRE(!stt.removeThreshold(1));
}

TEST_CASE("StressTestEditorV1 setState passedCount failedCount", "[Editor][S177]") {
    StressTestEditorV1 stt;
    Sttv1TestCase tc1; tc1.id = 1; tc1.name = "A";
    Sttv1TestCase tc2; tc2.id = 2; tc2.name = "B";
    stt.addCase(tc1); stt.addCase(tc2);
    stt.setState(1, Sttv1CaseState::Passed);
    stt.setState(2, Sttv1CaseState::Failed);
    REQUIRE(stt.passedCount() == 1);
    REQUIRE(stt.failedCount() == 1);
}

TEST_CASE("StressTestEditorV1 countByTarget", "[Editor][S177]") {
    StressTestEditorV1 stt;
    Sttv1TestCase tc1; tc1.id = 1; tc1.name = "A"; tc1.target = Sttv1StressTarget::GPU;
    Sttv1TestCase tc2; tc2.id = 2; tc2.name = "B"; tc2.target = Sttv1StressTarget::Storage;
    stt.addCase(tc1); stt.addCase(tc2);
    REQUIRE(stt.countByTarget(Sttv1StressTarget::GPU)     == 1);
    REQUIRE(stt.countByTarget(Sttv1StressTarget::Storage) == 1);
}

TEST_CASE("StressTestEditorV1 findCase returns ptr", "[Editor][S177]") {
    StressTestEditorV1 stt;
    Sttv1TestCase tc; tc.id = 6; tc.name = "F";
    stt.addCase(tc);
    REQUIRE(stt.findCase(6) != nullptr);
    REQUIRE(stt.findCase(6)->name == "F");
    REQUIRE(stt.findCase(99) == nullptr);
}

TEST_CASE("sttv1CaseStateName covers all values", "[Editor][S177]") {
    REQUIRE(std::string(sttv1CaseStateName(Sttv1CaseState::Idle))    == "Idle");
    REQUIRE(std::string(sttv1CaseStateName(Sttv1CaseState::Skipped)) == "Skipped");
}

TEST_CASE("sttv1StressTargetName covers all values", "[Editor][S177]") {
    REQUIRE(std::string(sttv1StressTargetName(Sttv1StressTarget::CPU))      == "CPU");
    REQUIRE(std::string(sttv1StressTargetName(Sttv1StressTarget::Combined)) == "Combined");
}

TEST_CASE("Sttv1TestCase state helpers", "[Editor][S177]") {
    Sttv1TestCase tc; tc.id = 1; tc.name = "X";
    tc.state = Sttv1CaseState::Active;
    REQUIRE(tc.isActive());
    tc.state = Sttv1CaseState::Passed;
    REQUIRE(tc.isPassed());
    tc.state = Sttv1CaseState::Failed;
    REQUIRE(tc.isFailed());
}

TEST_CASE("StressTestEditorV1 onChange callback", "[Editor][S177]") {
    StressTestEditorV1 stt;
    uint64_t notified = 0;
    stt.setOnChange([&](uint64_t id) { notified = id; });
    Sttv1TestCase tc; tc.id = 5; tc.name = "E";
    stt.addCase(tc);
    stt.setState(5, Sttv1CaseState::Active);
    REQUIRE(notified == 5);
}
