#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"

using namespace NF;

// 1. ToolStatus all 8 names
TEST_CASE("ToolStatus names cover all 8 values", "[Editor][S8]") {
    CHECK(std::string(toolStatusName(ToolStatus::Stopped))   == "Stopped");
    CHECK(std::string(toolStatusName(ToolStatus::Starting))  == "Starting");
    CHECK(std::string(toolStatusName(ToolStatus::Running))   == "Running");
    CHECK(std::string(toolStatusName(ToolStatus::Unhealthy)) == "Unhealthy");
    CHECK(std::string(toolStatusName(ToolStatus::Stopping))  == "Stopping");
    CHECK(std::string(toolStatusName(ToolStatus::Crashed))   == "Crashed");
    CHECK(std::string(toolStatusName(ToolStatus::Unknown))   == "Unknown");
    CHECK(std::string(toolStatusName(ToolStatus::Disabled))  == "Disabled");
}

// 2. ToolInstanceInfo defaults
TEST_CASE("ToolInstanceInfo has sensible defaults", "[Editor][S8]") {
    ToolInstanceInfo info;
    CHECK(info.name.empty());
    CHECK(info.executablePath.empty());
    CHECK(info.status == ToolStatus::Stopped);
    CHECK(info.pid == -1);
    CHECK(info.uptimeSeconds == 0.f);
    CHECK(info.eventsHandled == 0);
    CHECK(info.lastHeartbeatAge == 0.f);
}

// 3. ToolEcosystemConfig defaults
TEST_CASE("ToolEcosystemConfig defaults", "[Editor][S8]") {
    ToolEcosystemConfig cfg;
    CHECK(cfg.pipelineDir == ".atlas/pipeline");
    CHECK(cfg.heartbeatIntervalSec == 5.f);
    CHECK(cfg.unhealthyThresholdSec == 15.f);
    CHECK(cfg.crashThresholdSec == 30.f);
    CHECK(cfg.maxEventsPerTick == 16);
    CHECK(cfg.autoRestart == true);
}

// 4. StandaloneToolRunner start lifecycle
TEST_CASE("StandaloneToolRunner start lifecycle", "[Editor][S8]") {
    StandaloneToolRunner r;
    r.setName("TestTool");
    r.setExecutablePath("/usr/bin/test");
    CHECK(r.status() == ToolStatus::Stopped);
    CHECK(r.start());
    CHECK(r.status() == ToolStatus::Running);
    CHECK(r.info().pid >= 0);
    CHECK(r.isAlive());
}

// 5. StandaloneToolRunner stop lifecycle
TEST_CASE("StandaloneToolRunner stop lifecycle", "[Editor][S8]") {
    StandaloneToolRunner r;
    r.setName("TestTool");
    r.start();
    CHECK(r.stop());
    CHECK(r.status() == ToolStatus::Stopped);
    CHECK(r.info().pid == -1);
    CHECK_FALSE(r.isAlive());
}

// 6. StandaloneToolRunner cannot start when already running
TEST_CASE("StandaloneToolRunner cannot start when already running", "[Editor][S8]") {
    StandaloneToolRunner r;
    r.setName("TestTool");
    r.start();
    CHECK_FALSE(r.start());
    CHECK(r.status() == ToolStatus::Running);
}

// 7. StandaloneToolRunner cannot stop when not running
TEST_CASE("StandaloneToolRunner cannot stop when not running", "[Editor][S8]") {
    StandaloneToolRunner r;
    r.setName("TestTool");
    CHECK_FALSE(r.stop());
    CHECK(r.status() == ToolStatus::Stopped);
}

// 8. StandaloneToolRunner uptime tracking
TEST_CASE("StandaloneToolRunner uptime tracking", "[Editor][S8]") {
    StandaloneToolRunner r;
    r.setName("TestTool");
    r.start();
    r.tickUptime(1.5f);
    r.tickUptime(2.0f);
    CHECK(r.uptimeSeconds() == Catch::Approx(3.5f));
}

// 9. StandaloneToolRunner event recording
TEST_CASE("StandaloneToolRunner event recording", "[Editor][S8]") {
    StandaloneToolRunner r;
    r.setName("TestTool");
    r.start();
    r.recordEvent();
    r.recordEvent();
    r.recordEvent();
    CHECK(r.eventsHandled() == 3);
}

// 10. StandaloneToolRunner heartbeat + markUnhealthy
TEST_CASE("StandaloneToolRunner heartbeat and markUnhealthy", "[Editor][S8]") {
    StandaloneToolRunner r;
    r.setName("TestTool");
    r.start();
    r.tickUptime(10.f);
    CHECK(r.info().lastHeartbeatAge == Catch::Approx(10.f));
    r.recordHeartbeat();
    CHECK(r.info().lastHeartbeatAge == 0.f);
    r.markUnhealthy();
    CHECK(r.status() == ToolStatus::Unhealthy);
    CHECK(r.isAlive());
}

// 11. StandaloneToolRunner markCrashed
TEST_CASE("StandaloneToolRunner markCrashed", "[Editor][S8]") {
    StandaloneToolRunner r;
    r.setName("TestTool");
    r.start();
    r.markCrashed();
    CHECK(r.status() == ToolStatus::Crashed);
    CHECK(r.info().pid == -1);
    CHECK_FALSE(r.isAlive());
}

// 12. ToolHealthMonitor add/remove runners
TEST_CASE("ToolHealthMonitor add and remove runners", "[Editor][S8]") {
    ToolHealthMonitor mon;
    StandaloneToolRunner a, b;
    a.setName("A");
    b.setName("B");
    mon.addRunner(&a);
    mon.addRunner(&b);
    CHECK(mon.runnerCount() == 2);
    mon.removeRunner("A");
    CHECK(mon.runnerCount() == 1);
}

// 13. ToolHealthMonitor checkHealth marks unhealthy
TEST_CASE("ToolHealthMonitor checkHealth marks unhealthy", "[Editor][S8]") {
    ToolHealthMonitor mon;
    ToolEcosystemConfig cfg;
    cfg.unhealthyThresholdSec = 10.f;
    cfg.crashThresholdSec = 30.f;
    mon.setConfig(cfg);

    StandaloneToolRunner r;
    r.setName("T");
    r.start();
    r.tickUptime(12.f); // heartbeat age > 10 but < 30
    mon.addRunner(&r);
    mon.checkHealth();
    CHECK(r.status() == ToolStatus::Unhealthy);
}

// 14. ToolHealthMonitor checkHealth marks crashed
TEST_CASE("ToolHealthMonitor checkHealth marks crashed", "[Editor][S8]") {
    ToolHealthMonitor mon;
    ToolEcosystemConfig cfg;
    cfg.crashThresholdSec = 5.f;
    mon.setConfig(cfg);

    StandaloneToolRunner r;
    r.setName("T");
    r.start();
    r.tickUptime(6.f);
    mon.addRunner(&r);
    mon.checkHealth();
    CHECK(r.status() == ToolStatus::Crashed);
}

// 15. ToolHealthMonitor counting
TEST_CASE("ToolHealthMonitor counting healthy/unhealthy/crashed", "[Editor][S8]") {
    ToolHealthMonitor mon;
    StandaloneToolRunner a, b, c;
    a.setName("A"); b.setName("B"); c.setName("C");
    a.start(); b.start(); c.start();
    b.markUnhealthy();
    c.markCrashed();
    mon.addRunner(&a);
    mon.addRunner(&b);
    mon.addRunner(&c);
    CHECK(mon.healthyCount() == 1);
    CHECK(mon.unhealthyCount() == 1);
    CHECK(mon.crashedCount() == 1);
}

// 16. ToolOrchestrator startAll/stopAll
TEST_CASE("ToolOrchestrator startAll and stopAll", "[Editor][S8]") {
    ToolOrchestrator orch;
    CHECK(orch.startAll());
    CHECK(orch.runningCount() == 4);
    CHECK(orch.stopAll());
    CHECK(orch.runningCount() == 0);
}

// 17. ToolOrchestrator runner lookup by name
TEST_CASE("ToolOrchestrator runner lookup by name", "[Editor][S8]") {
    ToolOrchestrator orch;
    CHECK(orch.runner("SwissAgent") != nullptr);
    CHECK(orch.runner("AtlasAI") != nullptr);
    CHECK(orch.runner("ContractScanner") != nullptr);
    CHECK(orch.runner("ReplayMinimizer") != nullptr);
    CHECK(orch.runner("NonExistent") == nullptr);
}

// 18. ToolOrchestrator total events
TEST_CASE("ToolOrchestrator totalEventsHandled", "[Editor][S8]") {
    ToolOrchestrator orch;
    orch.startAll();
    orch.runner("SwissAgent")->recordEvent();
    orch.runner("SwissAgent")->recordEvent();
    orch.runner("AtlasAI")->recordEvent();
    CHECK(orch.totalEventsHandled() == 3);
}

// 19. ToolEcosystem init/shutdown
TEST_CASE("ToolEcosystem init and shutdown", "[Editor][S8]") {
    ToolEcosystem eco;
    CHECK_FALSE(eco.isInitialized());
    eco.init();
    CHECK(eco.isInitialized());
    eco.startAll();
    CHECK(eco.orchestrator().runningCount() == 4);
    eco.shutdown();
    CHECK_FALSE(eco.isInitialized());
}

// 20. ToolEcosystem tick with auto-restart
TEST_CASE("ToolEcosystem tick with auto-restart", "[Editor][S8]") {
    ToolEcosystemConfig cfg;
    cfg.autoRestart = true;
    cfg.crashThresholdSec = 5.f;

    ToolEcosystem eco;
    eco.init(cfg);
    eco.startAll();

    // Simulate a crash via large heartbeat gap
    eco.orchestrator().runner("SwissAgent")->tickUptime(100.f);
    eco.tick(0.f); // checkHealth -> crash -> autoRestart
    CHECK(eco.orchestrator().runner("SwissAgent")->status() == ToolStatus::Running);
    CHECK(eco.tickCount() == 1);
}

// 21. ToolEcosystem healthy tool count
TEST_CASE("ToolEcosystem healthyToolCount", "[Editor][S8]") {
    ToolEcosystem eco;
    eco.init();
    eco.startAll();
    CHECK(eco.healthyToolCount() == 4);
    eco.orchestrator().runner("AtlasAI")->markCrashed();
    CHECK(eco.healthyToolCount() == 3);
}
