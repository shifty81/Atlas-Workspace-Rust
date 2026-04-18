// S120 editor tests: BenchmarkSuiteEditor, StressTestEditor, LoadTestEditor
#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/LoadTestEditor.h"
#include "NF/Editor/StressTestEditor.h"
#include "NF/Editor/BenchmarkSuiteEditor.h"

using namespace NF;

// ── BenchmarkSuiteEditor ──────────────────────────────────────────────────────

TEST_CASE("BenchmarkCategory names", "[Editor][S120]") {
    REQUIRE(std::string(benchmarkCategoryName(BenchmarkCategory::CPU))       == "CPU");
    REQUIRE(std::string(benchmarkCategoryName(BenchmarkCategory::GPU))       == "GPU");
    REQUIRE(std::string(benchmarkCategoryName(BenchmarkCategory::Memory))    == "Memory");
    REQUIRE(std::string(benchmarkCategoryName(BenchmarkCategory::IO))        == "IO");
    REQUIRE(std::string(benchmarkCategoryName(BenchmarkCategory::Network))   == "Network");
    REQUIRE(std::string(benchmarkCategoryName(BenchmarkCategory::Physics))   == "Physics");
    REQUIRE(std::string(benchmarkCategoryName(BenchmarkCategory::Rendering)) == "Rendering");
    REQUIRE(std::string(benchmarkCategoryName(BenchmarkCategory::Audio))     == "Audio");
    REQUIRE(std::string(benchmarkCategoryName(BenchmarkCategory::Scripting)) == "Scripting");
    REQUIRE(std::string(benchmarkCategoryName(BenchmarkCategory::Custom))    == "Custom");
}

TEST_CASE("BenchmarkRunState names", "[Editor][S120]") {
    REQUIRE(std::string(benchmarkRunStateName(BenchmarkRunState::Pending))   == "Pending");
    REQUIRE(std::string(benchmarkRunStateName(BenchmarkRunState::Running))   == "Running");
    REQUIRE(std::string(benchmarkRunStateName(BenchmarkRunState::Completed)) == "Completed");
    REQUIRE(std::string(benchmarkRunStateName(BenchmarkRunState::Failed))    == "Failed");
    REQUIRE(std::string(benchmarkRunStateName(BenchmarkRunState::Skipped))   == "Skipped");
    REQUIRE(std::string(benchmarkRunStateName(BenchmarkRunState::Baseline))  == "Baseline");
}

TEST_CASE("BenchmarkMetric names", "[Editor][S120]") {
    REQUIRE(std::string(benchmarkMetricName(BenchmarkMetric::FPS))         == "FPS");
    REQUIRE(std::string(benchmarkMetricName(BenchmarkMetric::FrameTimeMs)) == "FrameTimeMs");
    REQUIRE(std::string(benchmarkMetricName(BenchmarkMetric::MemoryMB))    == "MemoryMB");
    REQUIRE(std::string(benchmarkMetricName(BenchmarkMetric::CPUPercent))  == "CPUPercent");
    REQUIRE(std::string(benchmarkMetricName(BenchmarkMetric::GPUPercent))  == "GPUPercent");
    REQUIRE(std::string(benchmarkMetricName(BenchmarkMetric::DrawCalls))   == "DrawCalls");
    REQUIRE(std::string(benchmarkMetricName(BenchmarkMetric::Triangles))   == "Triangles");
    REQUIRE(std::string(benchmarkMetricName(BenchmarkMetric::Latency))     == "Latency");
}

TEST_CASE("BenchmarkCase defaults", "[Editor][S120]") {
    BenchmarkCase bc(1, "render_scene_a", BenchmarkCategory::GPU);
    REQUIRE(bc.id()            == 1u);
    REQUIRE(bc.name()          == "render_scene_a");
    REQUIRE(bc.category()      == BenchmarkCategory::GPU);
    REQUIRE(bc.state()         == BenchmarkRunState::Pending);
    REQUIRE(bc.primaryMetric() == BenchmarkMetric::FPS);
    REQUIRE(bc.isEnabled());
    REQUIRE(bc.durationSec()   == 10.0f);
    REQUIRE(bc.warmupSec()     == 2.0f);
}

TEST_CASE("BenchmarkCase mutation", "[Editor][S120]") {
    BenchmarkCase bc(2, "physics_sim", BenchmarkCategory::Physics);
    bc.setState(BenchmarkRunState::Completed);
    bc.setPrimaryMetric(BenchmarkMetric::FrameTimeMs);
    bc.setIsEnabled(false);
    bc.setDurationSec(30.0f);
    bc.setWarmupSec(5.0f);
    REQUIRE(bc.state()         == BenchmarkRunState::Completed);
    REQUIRE(bc.primaryMetric() == BenchmarkMetric::FrameTimeMs);
    REQUIRE(!bc.isEnabled());
    REQUIRE(bc.durationSec()   == 30.0f);
    REQUIRE(bc.warmupSec()     == 5.0f);
}

TEST_CASE("BenchmarkSuiteEditor defaults", "[Editor][S120]") {
    BenchmarkSuiteEditor ed;
    REQUIRE(ed.isShowCompleted());
    REQUIRE(!ed.isAutoBaseline());
    REQUIRE(ed.repeatCount() == 3u);
    REQUIRE(ed.caseCount()   == 0u);
}

TEST_CASE("BenchmarkSuiteEditor add/remove cases", "[Editor][S120]") {
    BenchmarkSuiteEditor ed;
    REQUIRE(ed.addCase(BenchmarkCase(1, "cpu_a",     BenchmarkCategory::CPU)));
    REQUIRE(ed.addCase(BenchmarkCase(2, "gpu_a",     BenchmarkCategory::GPU)));
    REQUIRE(ed.addCase(BenchmarkCase(3, "memory_a",  BenchmarkCategory::Memory)));
    REQUIRE(!ed.addCase(BenchmarkCase(1, "cpu_a",    BenchmarkCategory::CPU)));
    REQUIRE(ed.caseCount() == 3u);
    REQUIRE(ed.removeCase(2));
    REQUIRE(ed.caseCount() == 2u);
    REQUIRE(!ed.removeCase(99));
}

TEST_CASE("BenchmarkSuiteEditor counts and find", "[Editor][S120]") {
    BenchmarkSuiteEditor ed;
    BenchmarkCase b1(1, "cpu_a",    BenchmarkCategory::CPU);
    BenchmarkCase b2(2, "cpu_b",    BenchmarkCategory::CPU);    b2.setState(BenchmarkRunState::Completed);
    BenchmarkCase b3(3, "gpu_a",    BenchmarkCategory::GPU);    b3.setIsEnabled(false);
    BenchmarkCase b4(4, "render_a", BenchmarkCategory::Rendering); b4.setState(BenchmarkRunState::Completed); b4.setIsEnabled(false);
    ed.addCase(b1); ed.addCase(b2); ed.addCase(b3); ed.addCase(b4);
    REQUIRE(ed.countByCategory(BenchmarkCategory::CPU)       == 2u);
    REQUIRE(ed.countByCategory(BenchmarkCategory::GPU)       == 1u);
    REQUIRE(ed.countByCategory(BenchmarkCategory::Network)   == 0u);
    REQUIRE(ed.countByState(BenchmarkRunState::Pending)      == 2u);
    REQUIRE(ed.countByState(BenchmarkRunState::Completed)    == 2u);
    REQUIRE(ed.countEnabled()                                == 2u);
    auto* found = ed.findCase(3);
    REQUIRE(found != nullptr);
    REQUIRE(found->category() == BenchmarkCategory::GPU);
    REQUIRE(ed.findCase(99) == nullptr);
}

TEST_CASE("BenchmarkSuiteEditor settings mutation", "[Editor][S120]") {
    BenchmarkSuiteEditor ed;
    ed.setShowCompleted(false);
    ed.setAutoBaseline(true);
    ed.setRepeatCount(5u);
    REQUIRE(!ed.isShowCompleted());
    REQUIRE(ed.isAutoBaseline());
    REQUIRE(ed.repeatCount() == 5u);
}

// ── StressTestEditor ──────────────────────────────────────────────────────────

TEST_CASE("StressTestScenario names", "[Editor][S120]") {
    REQUIRE(std::string(stressTestScenarioName(StressTestScenario::MaxEntities))           == "MaxEntities");
    REQUIRE(std::string(stressTestScenarioName(StressTestScenario::MaxParticles))          == "MaxParticles");
    REQUIRE(std::string(stressTestScenarioName(StressTestScenario::MaxLights))             == "MaxLights");
    REQUIRE(std::string(stressTestScenarioName(StressTestScenario::MaxAudioSources))       == "MaxAudioSources");
    REQUIRE(std::string(stressTestScenarioName(StressTestScenario::MaxNetworkPeers))       == "MaxNetworkPeers");
    REQUIRE(std::string(stressTestScenarioName(StressTestScenario::MaxPhysicsObjects))     == "MaxPhysicsObjects");
    REQUIRE(std::string(stressTestScenarioName(StressTestScenario::MaxAnimatedCharacters)) == "MaxAnimatedCharacters");
    REQUIRE(std::string(stressTestScenarioName(StressTestScenario::Custom))                == "Custom");
}

TEST_CASE("StressTestStatus names", "[Editor][S120]") {
    REQUIRE(std::string(stressTestStatusName(StressTestStatus::Idle))    == "Idle");
    REQUIRE(std::string(stressTestStatusName(StressTestStatus::Warming)) == "Warming");
    REQUIRE(std::string(stressTestStatusName(StressTestStatus::Running)) == "Running");
    REQUIRE(std::string(stressTestStatusName(StressTestStatus::Peaked))  == "Peaked");
    REQUIRE(std::string(stressTestStatusName(StressTestStatus::Crashed)) == "Crashed");
    REQUIRE(std::string(stressTestStatusName(StressTestStatus::Passed))  == "Passed");
    REQUIRE(std::string(stressTestStatusName(StressTestStatus::Failed))  == "Failed");
}

TEST_CASE("StressTestRampMode names", "[Editor][S120]") {
    REQUIRE(std::string(stressTestRampModeName(StressTestRampMode::Linear))      == "Linear");
    REQUIRE(std::string(stressTestRampModeName(StressTestRampMode::Exponential)) == "Exponential");
    REQUIRE(std::string(stressTestRampModeName(StressTestRampMode::Step))        == "Step");
    REQUIRE(std::string(stressTestRampModeName(StressTestRampMode::Burst))       == "Burst");
    REQUIRE(std::string(stressTestRampModeName(StressTestRampMode::Sustained))   == "Sustained");
}

TEST_CASE("StressTestJob defaults", "[Editor][S120]") {
    StressTestJob j(1, "max_entities_test", StressTestScenario::MaxEntities);
    REQUIRE(j.id()         == 1u);
    REQUIRE(j.name()       == "max_entities_test");
    REQUIRE(j.scenario()   == StressTestScenario::MaxEntities);
    REQUIRE(j.status()     == StressTestStatus::Idle);
    REQUIRE(j.rampMode()   == StressTestRampMode::Linear);
    REQUIRE(j.targetLoad() == 1000u);
    REQUIRE(j.isEnabled());
}

TEST_CASE("StressTestJob mutation", "[Editor][S120]") {
    StressTestJob j(2, "max_particles_test", StressTestScenario::MaxParticles);
    j.setStatus(StressTestStatus::Passed);
    j.setRampMode(StressTestRampMode::Exponential);
    j.setTargetLoad(50000u);
    j.setIsEnabled(false);
    REQUIRE(j.status()     == StressTestStatus::Passed);
    REQUIRE(j.rampMode()   == StressTestRampMode::Exponential);
    REQUIRE(j.targetLoad() == 50000u);
    REQUIRE(!j.isEnabled());
}

TEST_CASE("StressTestEditor defaults", "[Editor][S120]") {
    StressTestEditor ed;
    REQUIRE(ed.isShowPassed());
    REQUIRE(ed.isAbortOnCrash());
    REQUIRE(ed.timeoutSec() == 60.0f);
    REQUIRE(ed.jobCount()   == 0u);
}

TEST_CASE("StressTestEditor add/remove jobs", "[Editor][S120]") {
    StressTestEditor ed;
    REQUIRE(ed.addJob(StressTestJob(1, "entities_a", StressTestScenario::MaxEntities)));
    REQUIRE(ed.addJob(StressTestJob(2, "particles_a",StressTestScenario::MaxParticles)));
    REQUIRE(ed.addJob(StressTestJob(3, "lights_a",   StressTestScenario::MaxLights)));
    REQUIRE(!ed.addJob(StressTestJob(1, "entities_a",StressTestScenario::MaxEntities)));
    REQUIRE(ed.jobCount() == 3u);
    REQUIRE(ed.removeJob(2));
    REQUIRE(ed.jobCount() == 2u);
    REQUIRE(!ed.removeJob(99));
}

TEST_CASE("StressTestEditor counts and find", "[Editor][S120]") {
    StressTestEditor ed;
    StressTestJob j1(1, "ent_a",  StressTestScenario::MaxEntities);
    StressTestJob j2(2, "ent_b",  StressTestScenario::MaxEntities);  j2.setStatus(StressTestStatus::Passed);
    StressTestJob j3(3, "part_a", StressTestScenario::MaxParticles); j3.setIsEnabled(false);
    StressTestJob j4(4, "light_a",StressTestScenario::MaxLights);    j4.setStatus(StressTestStatus::Passed); j4.setIsEnabled(false);
    ed.addJob(j1); ed.addJob(j2); ed.addJob(j3); ed.addJob(j4);
    REQUIRE(ed.countByScenario(StressTestScenario::MaxEntities)  == 2u);
    REQUIRE(ed.countByScenario(StressTestScenario::MaxParticles) == 1u);
    REQUIRE(ed.countByScenario(StressTestScenario::Custom)       == 0u);
    REQUIRE(ed.countByStatus(StressTestStatus::Idle)             == 2u);
    REQUIRE(ed.countByStatus(StressTestStatus::Passed)           == 2u);
    REQUIRE(ed.countEnabled()                                    == 2u);
    auto* found = ed.findJob(3);
    REQUIRE(found != nullptr);
    REQUIRE(found->scenario() == StressTestScenario::MaxParticles);
    REQUIRE(ed.findJob(99) == nullptr);
}

TEST_CASE("StressTestEditor settings mutation", "[Editor][S120]") {
    StressTestEditor ed;
    ed.setShowPassed(false);
    ed.setAbortOnCrash(false);
    ed.setTimeoutSec(120.0f);
    REQUIRE(!ed.isShowPassed());
    REQUIRE(!ed.isAbortOnCrash());
    REQUIRE(ed.timeoutSec() == 120.0f);
}

// ── LoadTestEditor ────────────────────────────────────────────────────────────

TEST_CASE("LoadTestProfile names", "[Editor][S120]") {
    REQUIRE(std::string(loadTestProfileName(LoadTestProfile::Constant))   == "Constant");
    REQUIRE(std::string(loadTestProfileName(LoadTestProfile::Ramp))       == "Ramp");
    REQUIRE(std::string(loadTestProfileName(LoadTestProfile::Spike))      == "Spike");
    REQUIRE(std::string(loadTestProfileName(LoadTestProfile::Soak))       == "Soak");
    REQUIRE(std::string(loadTestProfileName(LoadTestProfile::Breakpoint)) == "Breakpoint");
    REQUIRE(std::string(loadTestProfileName(LoadTestProfile::Endurance))  == "Endurance");
}

TEST_CASE("LoadTestTarget names", "[Editor][S120]") {
    REQUIRE(std::string(loadTestTargetName(LoadTestTarget::Matchmaking))    == "Matchmaking");
    REQUIRE(std::string(loadTestTargetName(LoadTestTarget::Login))          == "Login");
    REQUIRE(std::string(loadTestTargetName(LoadTestTarget::AssetStreaming)) == "AssetStreaming");
    REQUIRE(std::string(loadTestTargetName(LoadTestTarget::SaveLoad))       == "SaveLoad");
    REQUIRE(std::string(loadTestTargetName(LoadTestTarget::Leaderboard))    == "Leaderboard");
    REQUIRE(std::string(loadTestTargetName(LoadTestTarget::Chat))           == "Chat");
    REQUIRE(std::string(loadTestTargetName(LoadTestTarget::Custom))         == "Custom");
}

TEST_CASE("LoadTestResult names", "[Editor][S120]") {
    REQUIRE(std::string(loadTestResultName(LoadTestResult::NotRun))   == "NotRun");
    REQUIRE(std::string(loadTestResultName(LoadTestResult::Pass))     == "Pass");
    REQUIRE(std::string(loadTestResultName(LoadTestResult::Degraded)) == "Degraded");
    REQUIRE(std::string(loadTestResultName(LoadTestResult::Fail))     == "Fail");
    REQUIRE(std::string(loadTestResultName(LoadTestResult::Timeout))  == "Timeout");
    REQUIRE(std::string(loadTestResultName(LoadTestResult::Aborted))  == "Aborted");
}

TEST_CASE("LoadTestScenario defaults", "[Editor][S120]") {
    LoadTestScenario sc(1, "login_ramp", LoadTestProfile::Ramp, LoadTestTarget::Login);
    REQUIRE(sc.id()              == 1u);
    REQUIRE(sc.name()            == "login_ramp");
    REQUIRE(sc.profile()         == LoadTestProfile::Ramp);
    REQUIRE(sc.target()          == LoadTestTarget::Login);
    REQUIRE(sc.result()          == LoadTestResult::NotRun);
    REQUIRE(sc.concurrentUsers() == 100u);
    REQUIRE(sc.durationSec()     == 60.0f);
    REQUIRE(sc.isEnabled());
}

TEST_CASE("LoadTestScenario mutation", "[Editor][S120]") {
    LoadTestScenario sc(2, "matchmaking_soak", LoadTestProfile::Soak, LoadTestTarget::Matchmaking);
    sc.setResult(LoadTestResult::Pass);
    sc.setConcurrentUsers(500u);
    sc.setDurationSec(300.0f);
    sc.setIsEnabled(false);
    REQUIRE(sc.result()          == LoadTestResult::Pass);
    REQUIRE(sc.concurrentUsers() == 500u);
    REQUIRE(sc.durationSec()     == 300.0f);
    REQUIRE(!sc.isEnabled());
}

TEST_CASE("LoadTestEditor defaults", "[Editor][S120]") {
    LoadTestEditor ed;
    REQUIRE(ed.isShowPassed());
    REQUIRE(!ed.isStopOnFail());
    REQUIRE(ed.globalTimeout()  == 300.0f);
    REQUIRE(ed.scenarioCount()  == 0u);
}

TEST_CASE("LoadTestEditor add/remove scenarios", "[Editor][S120]") {
    LoadTestEditor ed;
    REQUIRE(ed.addScenario(LoadTestScenario(1, "login_ramp",  LoadTestProfile::Ramp,  LoadTestTarget::Login)));
    REQUIRE(ed.addScenario(LoadTestScenario(2, "match_soak",  LoadTestProfile::Soak,  LoadTestTarget::Matchmaking)));
    REQUIRE(ed.addScenario(LoadTestScenario(3, "asset_spike", LoadTestProfile::Spike, LoadTestTarget::AssetStreaming)));
    REQUIRE(!ed.addScenario(LoadTestScenario(1, "login_ramp", LoadTestProfile::Ramp,  LoadTestTarget::Login)));
    REQUIRE(ed.scenarioCount() == 3u);
    REQUIRE(ed.removeScenario(2));
    REQUIRE(ed.scenarioCount() == 2u);
    REQUIRE(!ed.removeScenario(99));
}

TEST_CASE("LoadTestEditor counts and find", "[Editor][S120]") {
    LoadTestEditor ed;
    LoadTestScenario s1(1, "login_a",  LoadTestProfile::Ramp,  LoadTestTarget::Login);
    LoadTestScenario s2(2, "login_b",  LoadTestProfile::Ramp,  LoadTestTarget::Login);    s2.setResult(LoadTestResult::Pass);
    LoadTestScenario s3(3, "match_a",  LoadTestProfile::Soak,  LoadTestTarget::Matchmaking); s3.setIsEnabled(false);
    LoadTestScenario s4(4, "asset_a",  LoadTestProfile::Spike, LoadTestTarget::AssetStreaming); s4.setResult(LoadTestResult::Pass); s4.setIsEnabled(false);
    ed.addScenario(s1); ed.addScenario(s2); ed.addScenario(s3); ed.addScenario(s4);
    REQUIRE(ed.countByProfile(LoadTestProfile::Ramp)          == 2u);
    REQUIRE(ed.countByProfile(LoadTestProfile::Soak)          == 1u);
    REQUIRE(ed.countByProfile(LoadTestProfile::Endurance)     == 0u);
    REQUIRE(ed.countByResult(LoadTestResult::NotRun)          == 2u);
    REQUIRE(ed.countByResult(LoadTestResult::Pass)            == 2u);
    REQUIRE(ed.countEnabled()                                 == 2u);
    auto* found = ed.findScenario(3);
    REQUIRE(found != nullptr);
    REQUIRE(found->target() == LoadTestTarget::Matchmaking);
    REQUIRE(ed.findScenario(99) == nullptr);
}

TEST_CASE("LoadTestEditor settings mutation", "[Editor][S120]") {
    LoadTestEditor ed;
    ed.setShowPassed(false);
    ed.setStopOnFail(true);
    ed.setGlobalTimeout(600.0f);
    REQUIRE(!ed.isShowPassed());
    REQUIRE(ed.isStopOnFail());
    REQUIRE(ed.globalTimeout() == 600.0f);
}
