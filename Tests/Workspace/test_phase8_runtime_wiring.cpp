// Tests/Workspace/test_phase8_runtime_wiring.cpp
// Phase 8 — Runtime Wiring and First Real Tool Loop
//
// Tests for:
//   1. WorkspaceBootstrap     — config validation, launch modes, backend choice, run sequence
//   2. WorkspaceFrameController — frame pacing, dt clamping, EMA, budget, statistics
//   3. WorkspaceAppRegistry   — descriptor CRUD, filtering, project-scoped apps
//   4. WorkspaceLaunchContract — launch context toArgs, NullLaunchService, status names
//   5. ConsoleCommandBus      — command registration, execute, scope/hidden/enabled counts
//   6. SelectionService       — select/deselect/toggle/multi/exclusive/version
//   7. EditorEventBus         — subscribe, post, flush, priority routing, suspend/resume

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Workspace/WorkspaceBootstrap.h"
#include "NF/Workspace/WorkspaceFrameController.h"
#include "NF/Workspace/WorkspaceAppRegistry.h"
#include "NF/Workspace/WorkspaceLaunchContract.h"
#include "NF/Workspace/ConsoleCommandBus.h"
#include "NF/Workspace/SelectionService.h"
#include "NF/Workspace/EditorEventBus.h"

using namespace NF;

// ═════════════════════════════════════════════════════════════════
// SECTION 1 — WorkspaceBootstrap
// ═════════════════════════════════════════════════════════════════

TEST_CASE("workspaceStartupModeName returns correct strings", "[Phase8][Bootstrap]") {
    CHECK(std::string(workspaceStartupModeName(WorkspaceStartupMode::Hosted))   == "Hosted");
    CHECK(std::string(workspaceStartupModeName(WorkspaceStartupMode::Headless)) == "Headless");
    CHECK(std::string(workspaceStartupModeName(WorkspaceStartupMode::Preview))  == "Preview");
}

TEST_CASE("workspaceBackendChoiceName returns correct strings", "[Phase8][Bootstrap]") {
    CHECK(std::string(workspaceBackendChoiceName(WorkspaceBackendChoice::Auto))   == "Auto");
    CHECK(std::string(workspaceBackendChoiceName(WorkspaceBackendChoice::D3D11))  == "D3D11");
    CHECK(std::string(workspaceBackendChoiceName(WorkspaceBackendChoice::OpenGL)) == "OpenGL");
    CHECK(std::string(workspaceBackendChoiceName(WorkspaceBackendChoice::GDI))    == "GDI");
    CHECK(std::string(workspaceBackendChoiceName(WorkspaceBackendChoice::Null))   == "Null");
}

TEST_CASE("workspaceBootstrapErrorName returns correct strings", "[Phase8][Bootstrap]") {
    CHECK(std::string(workspaceBootstrapErrorName(WorkspaceBootstrapError::None))                 == "None");
    CHECK(std::string(workspaceBootstrapErrorName(WorkspaceBootstrapError::InvalidConfig))        == "InvalidConfig");
    CHECK(std::string(workspaceBootstrapErrorName(WorkspaceBootstrapError::AlreadyInitialized))   == "AlreadyInitialized");
    CHECK(std::string(workspaceBootstrapErrorName(WorkspaceBootstrapError::ShellInitFailed))      == "ShellInitFailed");
    CHECK(std::string(workspaceBootstrapErrorName(WorkspaceBootstrapError::ToolRegistrationFailed)) == "ToolRegistrationFailed");
}

TEST_CASE("WorkspaceWindowConfig default values are valid", "[Phase8][Bootstrap]") {
    WorkspaceWindowConfig wc;
    CHECK(wc.isValid());
    CHECK(wc.width  == 1280);
    CHECK(wc.height == 800);
    CHECK(wc.aspectRatio() == Catch::Approx(1280.f / 800.f));
    CHECK_FALSE(wc.fullscreen);
    CHECK(wc.resizable);
}

TEST_CASE("WorkspaceWindowConfig invalid when empty title or zero dimensions",
          "[Phase8][Bootstrap]") {
    WorkspaceWindowConfig wc;
    wc.title = "";
    CHECK_FALSE(wc.isValid());
    wc.title = "Test";
    wc.width = 0;
    CHECK_FALSE(wc.isValid());
}

TEST_CASE("WorkspaceBootstrapConfig valid for headless mode without window",
          "[Phase8][Bootstrap]") {
    WorkspaceBootstrapConfig cfg;
    cfg.launchMode = WorkspaceStartupMode::Headless;
    CHECK(cfg.isValid()); // headless does not need a window
}

TEST_CASE("WorkspaceBootstrapConfig invalid when Hosted mode has bad window config",
          "[Phase8][Bootstrap]") {
    WorkspaceBootstrapConfig cfg;
    cfg.launchMode = WorkspaceStartupMode::Hosted;
    cfg.windowConfig.title = ""; // invalid
    CHECK_FALSE(cfg.isValid());
}

TEST_CASE("WorkspaceBootstrap: headless run succeeds with no tools", "[Phase8][Bootstrap]") {
    WorkspaceBootstrapConfig cfg;
    cfg.launchMode = WorkspaceStartupMode::Headless;

    WorkspaceShell shell;
    WorkspaceBootstrap boot;
    auto result = boot.run(cfg, shell);

    CHECK(result.succeeded());
    CHECK(result.error == WorkspaceBootstrapError::None);
    CHECK(result.toolsRegistered == 0);
    CHECK(shell.phase() == ShellPhase::Ready);
    CHECK(boot.runCount() == 1);

    shell.shutdown();
}

TEST_CASE("WorkspaceBootstrap: fails with InvalidConfig when window title empty",
          "[Phase8][Bootstrap]") {
    WorkspaceBootstrapConfig cfg;
    cfg.launchMode = WorkspaceStartupMode::Hosted;
    cfg.windowConfig.title = "";

    WorkspaceShell shell;
    WorkspaceBootstrap boot;
    auto result = boot.run(cfg, shell);

    CHECK(result.failed());
    CHECK(result.error == WorkspaceBootstrapError::InvalidConfig);
    CHECK(shell.phase() == ShellPhase::Created); // shell was not changed
}

TEST_CASE("WorkspaceBootstrap: fails AlreadyInitialized if shell not in Created",
          "[Phase8][Bootstrap]") {
    WorkspaceBootstrapConfig cfg;
    cfg.launchMode = WorkspaceStartupMode::Headless;

    WorkspaceShell shell;
    shell.initialize(); // put into Ready state

    WorkspaceBootstrap boot;
    auto result = boot.run(cfg, shell);

    CHECK(result.failed());
    CHECK(result.error == WorkspaceBootstrapError::AlreadyInitialized);

    shell.shutdown();
}

TEST_CASE("WorkspaceBootstrap: tool factories are invoked during run", "[Phase8][Bootstrap]") {
    WorkspaceBootstrapConfig cfg;
    cfg.launchMode = WorkspaceStartupMode::Headless;

    bool factory1Called = false;
    bool factory2Called = false;

    cfg.toolFactories.push_back([&]() -> std::unique_ptr<IHostedTool> {
        factory1Called = true;
        return nullptr; // null factories are filtered by registerToolFactory
    });

    WorkspaceShell shell;
    WorkspaceBootstrap boot;
    auto result = boot.run(cfg, shell);
    // A null factory pointer passed directly is caught; a non-null factory returning null
    // is handled by the shell (registerToolFactory accepts null returns silently).
    // The config has a non-null factory but it returns nullptr — the shell gracefully ignores it.
    // The bootstrap itself only fails on a null function object, not a null returned tool.
    CHECK(factory1Called);

    shell.shutdown();
}

TEST_CASE("WorkspaceBootstrap: startup messages are posted to shell", "[Phase8][Bootstrap]") {
    WorkspaceBootstrapConfig cfg;
    cfg.launchMode = WorkspaceStartupMode::Headless;
    cfg.startupMessages.push_back("System initialized");
    cfg.startupMessages.push_back("Ready for input");

    WorkspaceShell shell;
    WorkspaceBootstrap boot;
    auto result = boot.run(cfg, shell);

    CHECK(result.succeeded());
    // Notifications are posted — check notification count from shell contract
    CHECK(shell.shellContract().notificationCount() >= 2);

    shell.shutdown();
}

TEST_CASE("WorkspaceBootstrap: runCount increments on success", "[Phase8][Bootstrap]") {
    WorkspaceBootstrap boot;

    for (int i = 0; i < 3; ++i) {
        WorkspaceBootstrapConfig cfg;
        cfg.launchMode = WorkspaceStartupMode::Headless;
        WorkspaceShell shell;
        (void)boot.run(cfg, shell);
        shell.shutdown();
    }
    CHECK(boot.runCount() == 3);
}

TEST_CASE("WorkspaceBootstrapResult errorName returns string", "[Phase8][Bootstrap]") {
    WorkspaceBootstrapResult r;
    r.error = WorkspaceBootstrapError::ShellInitFailed;
    CHECK(std::string(r.errorName()) == "ShellInitFailed");
    CHECK(r.failed());
}

// ═════════════════════════════════════════════════════════════════
// SECTION 2 — WorkspaceFrameController
// ═════════════════════════════════════════════════════════════════

TEST_CASE("WorkspaceFrameController default state", "[Phase8][FrameController]") {
    WorkspaceFrameController fc;
    CHECK(fc.targetFPS()   == Catch::Approx(60.f));
    CHECK(fc.maxDtSec()    == Catch::Approx(0.1f));
    CHECK(fc.emaAlpha()    == Catch::Approx(0.1f));
    CHECK(fc.frameNumber() == 0);
    CHECK(fc.stats().totalFrames == 0);
}

TEST_CASE("WorkspaceFrameController setTargetFPS updates budget", "[Phase8][FrameController]") {
    WorkspaceFrameController fc;
    fc.setTargetFPS(30.f);
    CHECK(fc.targetFPS() == Catch::Approx(30.f));
    CHECK(fc.budget().totalMs == Catch::Approx(1000.f / 30.f));
}

TEST_CASE("WorkspaceFrameController setTargetFPS ignores non-positive",
          "[Phase8][FrameController]") {
    WorkspaceFrameController fc;
    fc.setTargetFPS(0.f);
    CHECK(fc.targetFPS() == Catch::Approx(60.f)); // unchanged
    fc.setTargetFPS(-10.f);
    CHECK(fc.targetFPS() == Catch::Approx(60.f));
}

TEST_CASE("WorkspaceFrameController setMaxDeltaTime works", "[Phase8][FrameController]") {
    WorkspaceFrameController fc;
    fc.setMaxDeltaTime(0.05f);
    CHECK(fc.maxDtSec() == Catch::Approx(0.05f));
    fc.setMaxDeltaTime(0.f); // invalid, no change
    CHECK(fc.maxDtSec() == Catch::Approx(0.05f));
}

TEST_CASE("WorkspaceFrameController setEMAAlpha clamps to valid range",
          "[Phase8][FrameController]") {
    WorkspaceFrameController fc;
    fc.setEMAAlpha(0.5f);
    CHECK(fc.emaAlpha() == Catch::Approx(0.5f));
    fc.setEMAAlpha(0.f); // invalid
    CHECK(fc.emaAlpha() == Catch::Approx(0.5f));
    fc.setEMAAlpha(1.f); // valid (alpha=1 means no smoothing)
    CHECK(fc.emaAlpha() == Catch::Approx(1.f));
}

TEST_CASE("WorkspaceFrameController beginFrame returns incrementing frame number",
          "[Phase8][FrameController]") {
    WorkspaceFrameController fc;
    auto fr1 = fc.beginFrame(0.016f);
    auto fr2 = fc.beginFrame(0.016f);
    auto fr3 = fc.beginFrame(0.016f);
    CHECK(fr1.frameNumber == 1);
    CHECK(fr2.frameNumber == 2);
    CHECK(fr3.frameNumber == 3);
}

TEST_CASE("WorkspaceFrameController clamps large dt to maxDt", "[Phase8][FrameController]") {
    WorkspaceFrameController fc;
    fc.setMaxDeltaTime(0.05f);
    fc.setEMAAlpha(1.0f); // no smoothing — raw == smoothed
    auto fr = fc.beginFrame(1.0f); // 1 second — way too large
    CHECK(fr.rawDt  == Catch::Approx(0.05f));
    CHECK(fr.dt     == Catch::Approx(0.05f));
}

TEST_CASE("WorkspaceFrameController clamps zero/negative dt", "[Phase8][FrameController]") {
    WorkspaceFrameController fc;
    fc.setEMAAlpha(1.0f);
    auto fr = fc.beginFrame(0.f);
    CHECK(fr.rawDt > 0.f);
}

TEST_CASE("WorkspaceFrameController EMA smoothing reduces spikes",
          "[Phase8][FrameController]") {
    WorkspaceFrameController fc;
    fc.setEMAAlpha(0.1f);
    // Run 10 steady frames at 16ms
    for (int i = 0; i < 10; ++i) fc.beginFrame(0.016f);
    float steadyEma = fc.emaDtSec();
    // One spike frame — EMA should move only a little
    fc.beginFrame(0.1f);
    float afterSpike = fc.emaDtSec();
    CHECK(afterSpike > steadyEma);
    // EMA blends: 0.1 * 0.1 + 0.9 * ~0.016 ≈ 0.0244 — well below the spike (0.1)
    CHECK(afterSpike < 0.05f);
}

TEST_CASE("WorkspaceFrameController endFrame updates statistics",
          "[Phase8][FrameController]") {
    WorkspaceFrameController fc;
    fc.beginFrame(0.016f);
    fc.markUpdateDone(3.f);  // 3ms update
    fc.markRenderDone(10.f); // 10ms render
    fc.endFrame(13.f);

    CHECK(fc.stats().totalFrames == 1);
    CHECK(fc.stats().lastUpdateMs == Catch::Approx(3.f));
    CHECK(fc.stats().lastRenderMs == Catch::Approx(10.f));
    CHECK(fc.stats().minDtMs > 0.f);
    CHECK(fc.stats().maxDtMs > 0.f);
}

TEST_CASE("WorkspaceFrameController tracks FPS from smoothed dt",
          "[Phase8][FrameController]") {
    WorkspaceFrameController fc;
    fc.setEMAAlpha(1.0f); // no smoothing
    fc.beginFrame(0.016f);
    fc.endFrame();
    CHECK(fc.stats().fps == Catch::Approx(1000.f / (0.016f * 1000.f)).margin(1.0f));
}

TEST_CASE("WorkspaceFrameController detects over-budget frames",
          "[Phase8][FrameController]") {
    WorkspaceFrameController fc;
    fc.setTargetFPS(60.f); // budget: ~16.67ms
    fc.beginFrame(0.016f);
    fc.endFrame(30.f); // 30ms — over budget
    CHECK(fc.stats().skippedFrames == 1);

    auto fr2 = fc.beginFrame(0.016f);
    CHECK(fr2.wasSkipped); // previous frame was over budget
}

TEST_CASE("WorkspaceFrameController shouldSleep and sleepMs", "[Phase8][FrameController]") {
    WorkspaceFrameController fc;
    fc.setTargetFPS(60.f); // ~16.67ms budget
    CHECK(fc.shouldSleep(10.f));   // 10ms < 16.67ms → should sleep
    CHECK_FALSE(fc.shouldSleep(20.f)); // 20ms > 16.67ms → no sleep
    CHECK(fc.sleepMs(10.f) > 0.f);
    CHECK(fc.sleepMs(20.f) == Catch::Approx(0.f));
}

TEST_CASE("WorkspaceFrameController resetStats clears all counters",
          "[Phase8][FrameController]") {
    WorkspaceFrameController fc;
    fc.beginFrame(0.016f);
    fc.endFrame(30.f);
    REQUIRE(fc.stats().totalFrames == 1);

    fc.resetStats();
    CHECK(fc.stats().totalFrames   == 0);
    CHECK(fc.stats().skippedFrames == 0);
    CHECK(fc.frameNumber()         == 0);
    CHECK(fc.emaDtSec()            == Catch::Approx(0.f));
}

TEST_CASE("FrameBudget validity", "[Phase8][FrameController]") {
    FrameBudget b;
    CHECK(b.isValid());
    b.totalMs = 0.f;
    CHECK_FALSE(b.isValid());
    b.totalMs = 16.67f;
    b.updateMs = 0.f;
    CHECK_FALSE(b.isValid());
}

TEST_CASE("FrameStatistics budgetUtilization", "[Phase8][FrameController]") {
    FrameStatistics s;
    s.lastUpdateMs = 4.f;
    s.lastRenderMs = 10.f;
    CHECK(s.budgetUtilization(16.67f) == Catch::Approx(14.f / 16.67f));
    CHECK(s.budgetUtilization(0.f) == Catch::Approx(0.f)); // divide-by-zero guard
}

// ═════════════════════════════════════════════════════════════════
// SECTION 3 — WorkspaceAppRegistry
// ═════════════════════════════════════════════════════════════════

TEST_CASE("workspaceAppName returns correct strings", "[Phase8][AppRegistry]") {
    CHECK(std::string(workspaceAppName(WorkspaceAppId::NovaForgeEditor)) == "NovaForgeEditor");
    CHECK(std::string(workspaceAppName(WorkspaceAppId::NovaForgeGame))   == "NovaForgeGame");
    CHECK(std::string(workspaceAppName(WorkspaceAppId::NovaForgeServer)) == "NovaForgeServer");
    CHECK(std::string(workspaceAppName(WorkspaceAppId::TileEditor))      == "TileEditor");
}

TEST_CASE("WorkspaceAppDescriptor validity", "[Phase8][AppRegistry]") {
    WorkspaceAppDescriptor d;
    CHECK_FALSE(d.isValid()); // id=Unknown, no name/path
    d.id = WorkspaceAppId::TileEditor;
    CHECK_FALSE(d.isValid()); // still missing name
    d.name = "TileEditor";
    CHECK_FALSE(d.isValid()); // still missing path
    d.executablePath = "TileEditor.exe";
    CHECK(d.isValid());
}

TEST_CASE("WorkspaceAppDescriptor displayLabel", "[Phase8][AppRegistry]") {
    WorkspaceAppDescriptor d;
    d.id = WorkspaceAppId::TileEditor;
    d.name = "TileEditor";
    d.executablePath = "TileEditor.exe";
    CHECK(d.displayLabel() == "TileEditor (TileEditor.exe)");
}

TEST_CASE("WorkspaceAppRegistry registerApp and find", "[Phase8][AppRegistry]") {
    WorkspaceAppRegistry reg;
    WorkspaceAppDescriptor d;
    d.id             = WorkspaceAppId::TileEditor;
    d.name           = "TileEditor";
    d.executablePath = "TileEditor.exe";
    CHECK(reg.registerApp(d));
    CHECK(reg.count() == 1);
    CHECK(reg.isRegistered(WorkspaceAppId::TileEditor));
    const auto* found = reg.find(WorkspaceAppId::TileEditor);
    REQUIRE(found != nullptr);
    CHECK(found->name == "TileEditor");
}

TEST_CASE("WorkspaceAppRegistry registerApp rejects duplicates", "[Phase8][AppRegistry]") {
    WorkspaceAppRegistry reg;
    WorkspaceAppDescriptor d;
    d.id = WorkspaceAppId::TileEditor; d.name = "TileEditor"; d.executablePath = "x.exe";
    CHECK(reg.registerApp(d));
    CHECK_FALSE(reg.registerApp(d));
    CHECK(reg.count() == 1);
}

TEST_CASE("WorkspaceAppRegistry registerApp rejects invalid", "[Phase8][AppRegistry]") {
    WorkspaceAppRegistry reg;
    WorkspaceAppDescriptor bad; // id=Unknown
    CHECK_FALSE(reg.registerApp(bad));
    CHECK(reg.count() == 0);
}

TEST_CASE("WorkspaceAppRegistry unregisterApp works", "[Phase8][AppRegistry]") {
    WorkspaceAppRegistry reg;
    WorkspaceAppDescriptor d;
    d.id = WorkspaceAppId::TileEditor; d.name = "TileEditor"; d.executablePath = "x.exe";
    reg.registerApp(d);
    CHECK(reg.unregisterApp(WorkspaceAppId::TileEditor));
    CHECK(reg.count() == 0);
    CHECK_FALSE(reg.unregisterApp(WorkspaceAppId::TileEditor)); // already removed
}

TEST_CASE("WorkspaceAppRegistry findByName works", "[Phase8][AppRegistry]") {
    WorkspaceAppRegistry reg;
    WorkspaceAppDescriptor d;
    d.id = WorkspaceAppId::NovaForgeEditor; d.name = "NovaForgeEditor"; d.executablePath = "x.exe";
    reg.registerApp(d);
    const auto* found = reg.findByName("NovaForgeEditor");
    REQUIRE(found != nullptr);
    CHECK(found->id == WorkspaceAppId::NovaForgeEditor);
    CHECK(reg.findByName("missing") == nullptr);
}

TEST_CASE("WorkspaceAppRegistry projectScopedApps filtering", "[Phase8][AppRegistry]") {
    WorkspaceAppRegistry reg;

    WorkspaceAppDescriptor scoped;
    scoped.id = WorkspaceAppId::NovaForgeEditor; scoped.name = "A"; scoped.executablePath = "a.exe";
    scoped.isProjectScoped = true;
    reg.registerApp(scoped);

    WorkspaceAppDescriptor notScoped;
    notScoped.id = WorkspaceAppId::TileEditor; notScoped.name = "B"; notScoped.executablePath = "b.exe";
    notScoped.isProjectScoped = false;
    reg.registerApp(notScoped);

    auto scoped_list = reg.projectScopedApps();
    CHECK(scoped_list.size() == 1);
    CHECK(scoped_list[0]->id == WorkspaceAppId::NovaForgeEditor);
}

// ═════════════════════════════════════════════════════════════════
// SECTION 4 — WorkspaceLaunchContract
// ═════════════════════════════════════════════════════════════════

TEST_CASE("workspaceLaunchModeName returns correct strings", "[Phase8][LaunchContract]") {
    CHECK(std::string(workspaceLaunchModeName(WorkspaceLaunchMode::Hosted))   == "hosted");
    CHECK(std::string(workspaceLaunchModeName(WorkspaceLaunchMode::Headless)) == "headless");
    CHECK(std::string(workspaceLaunchModeName(WorkspaceLaunchMode::Preview))  == "preview");
}

TEST_CASE("workspaceLaunchStatusName returns correct strings", "[Phase8][LaunchContract]") {
    CHECK(std::string(workspaceLaunchStatusName(WorkspaceLaunchStatus::Success))            == "Success");
    CHECK(std::string(workspaceLaunchStatusName(WorkspaceLaunchStatus::AppNotFound))        == "AppNotFound");
    CHECK(std::string(workspaceLaunchStatusName(WorkspaceLaunchStatus::ExecutableNotFound)) == "ExecutableNotFound");
    CHECK(std::string(workspaceLaunchStatusName(WorkspaceLaunchStatus::InvalidContext))     == "InvalidContext");
    CHECK(std::string(workspaceLaunchStatusName(WorkspaceLaunchStatus::AlreadyRunning))     == "AlreadyRunning");
    CHECK(std::string(workspaceLaunchStatusName(WorkspaceLaunchStatus::SpawnFailed))        == "SpawnFailed");
}

TEST_CASE("WorkspaceLaunchContext validity", "[Phase8][LaunchContract]") {
    WorkspaceLaunchContext ctx;
    CHECK_FALSE(ctx.isValid());
    ctx.workspaceRoot = "/ws";
    CHECK_FALSE(ctx.isValid());
    ctx.projectPath = "/ws/project.atlas.json";
    CHECK_FALSE(ctx.isValid());
    ctx.sessionId = "session-001";
    CHECK(ctx.isValid());
}

TEST_CASE("WorkspaceLaunchContext toArgs generates proper CLI args", "[Phase8][LaunchContract]") {
    WorkspaceLaunchContext ctx;
    ctx.workspaceRoot = "/ws";
    ctx.projectPath   = "/ws/proj.json";
    ctx.sessionId     = "s1";
    ctx.ipcEndpoint   = "/tmp/ipc.sock";
    ctx.logSinkPath   = "/tmp/log.txt";
    ctx.mode          = WorkspaceLaunchMode::Headless;

    auto args = ctx.toArgs();
    REQUIRE(args.size() >= 4);
    CHECK(args[0] == "--headless");
    CHECK(args[1] == "--workspace-root=/ws");
    CHECK(args[2] == "--project=/ws/proj.json");
    CHECK(args[3] == "--session-id=s1");
    // ipc and log args also present
    bool hasIpc = false, hasLog = false;
    for (const auto& a : args) {
        if (a.find("--ipc=") == 0) hasIpc = true;
        if (a.find("--log=") == 0) hasLog = true;
    }
    CHECK(hasIpc);
    CHECK(hasLog);
}

TEST_CASE("WorkspaceLaunchContext toArgs omits optional args when empty",
          "[Phase8][LaunchContract]") {
    WorkspaceLaunchContext ctx;
    ctx.workspaceRoot = "/ws";
    ctx.projectPath   = "/ws/proj.json";
    ctx.sessionId     = "s1";
    // no ipc or log
    auto args = ctx.toArgs();
    for (const auto& a : args) {
        CHECK(a.find("--ipc=") == std::string::npos);
        CHECK(a.find("--log=") == std::string::npos);
    }
}

TEST_CASE("WorkspaceLaunchResult succeeded/isRunning", "[Phase8][LaunchContract]") {
    WorkspaceLaunchResult r;
    r.status = WorkspaceLaunchStatus::Success;
    r.pid    = 1234;
    CHECK(r.succeeded());
    CHECK(r.isRunning());

    r.pid = 0;
    CHECK(r.succeeded());
    CHECK_FALSE(r.isRunning()); // succeeded but no pid

    r.status = WorkspaceLaunchStatus::SpawnFailed;
    CHECK_FALSE(r.succeeded());
}

TEST_CASE("NullLaunchService launches valid app successfully", "[Phase8][LaunchContract]") {
    NullLaunchService svc;
    WorkspaceAppDescriptor d;
    d.id = WorkspaceAppId::TileEditor; d.name = "TileEditor"; d.executablePath = "x.exe";

    WorkspaceLaunchContext ctx;
    ctx.workspaceRoot = "/ws"; ctx.projectPath = "/ws/p.json"; ctx.sessionId = "s";

    auto result = svc.launchApp(d, ctx);
    CHECK(result.succeeded());
    CHECK(result.pid == 1);
    CHECK(svc.isRunning(WorkspaceAppId::TileEditor));
    CHECK(svc.pidOf(WorkspaceAppId::TileEditor) == 1u);
}

TEST_CASE("NullLaunchService returns AppNotFound for invalid descriptor",
          "[Phase8][LaunchContract]") {
    NullLaunchService svc;
    WorkspaceAppDescriptor bad; // invalid
    WorkspaceLaunchContext ctx;
    ctx.workspaceRoot = "/ws"; ctx.projectPath = "/ws/p.json"; ctx.sessionId = "s";

    auto result = svc.launchApp(bad, ctx);
    CHECK(result.status == WorkspaceLaunchStatus::AppNotFound);
    CHECK_FALSE(result.succeeded());
}

TEST_CASE("NullLaunchService returns InvalidContext for bad context",
          "[Phase8][LaunchContract]") {
    NullLaunchService svc;
    WorkspaceAppDescriptor d;
    d.id = WorkspaceAppId::TileEditor; d.name = "TileEditor"; d.executablePath = "x.exe";
    WorkspaceLaunchContext ctx; // invalid — missing required fields

    auto result = svc.launchApp(d, ctx);
    CHECK(result.status == WorkspaceLaunchStatus::InvalidContext);
}

TEST_CASE("NullLaunchService shutdownApp stops running app", "[Phase8][LaunchContract]") {
    NullLaunchService svc;
    WorkspaceAppDescriptor d;
    d.id = WorkspaceAppId::TileEditor; d.name = "TileEditor"; d.executablePath = "x.exe";
    WorkspaceLaunchContext ctx;
    ctx.workspaceRoot = "/ws"; ctx.projectPath = "/ws/p.json"; ctx.sessionId = "s";

    svc.launchApp(d, ctx);
    REQUIRE(svc.isRunning(WorkspaceAppId::TileEditor));
    svc.shutdownApp(WorkspaceAppId::TileEditor);
    CHECK_FALSE(svc.isRunning(WorkspaceAppId::TileEditor));
    CHECK(svc.pidOf(WorkspaceAppId::TileEditor) == 0u);
}

// ═════════════════════════════════════════════════════════════════
// SECTION 5 — ConsoleCommandBus
// ═════════════════════════════════════════════════════════════════

TEST_CASE("consoleCmdScopeName returns correct strings", "[Phase8][ConsoleCmd]") {
    CHECK(std::string(consoleCmdScopeName(ConsoleCmdScope::Global)) == "Global");
    CHECK(std::string(consoleCmdScopeName(ConsoleCmdScope::Editor)) == "Editor");
    CHECK(std::string(consoleCmdScopeName(ConsoleCmdScope::Game))   == "Game");
    CHECK(std::string(consoleCmdScopeName(ConsoleCmdScope::Server)) == "Server");
    CHECK(std::string(consoleCmdScopeName(ConsoleCmdScope::Client)) == "Client");
    CHECK(std::string(consoleCmdScopeName(ConsoleCmdScope::Plugin)) == "Plugin");
}

TEST_CASE("consoleCmdArgTypeName returns correct strings", "[Phase8][ConsoleCmd]") {
    CHECK(std::string(consoleCmdArgTypeName(ConsoleCmdArgType::None))   == "None");
    CHECK(std::string(consoleCmdArgTypeName(ConsoleCmdArgType::Bool))   == "Bool");
    CHECK(std::string(consoleCmdArgTypeName(ConsoleCmdArgType::Int))    == "Int");
    CHECK(std::string(consoleCmdArgTypeName(ConsoleCmdArgType::Float))  == "Float");
    CHECK(std::string(consoleCmdArgTypeName(ConsoleCmdArgType::String)) == "String");
    CHECK(std::string(consoleCmdArgTypeName(ConsoleCmdArgType::Enum))   == "Enum");
}

TEST_CASE("consoleCmdExecResultName returns correct strings", "[Phase8][ConsoleCmd]") {
    CHECK(std::string(consoleCmdExecResultName(ConsoleCmdExecResult::Ok))               == "Ok");
    CHECK(std::string(consoleCmdExecResultName(ConsoleCmdExecResult::NotFound))         == "NotFound");
    CHECK(std::string(consoleCmdExecResultName(ConsoleCmdExecResult::InvalidArgs))      == "InvalidArgs");
    CHECK(std::string(consoleCmdExecResultName(ConsoleCmdExecResult::PermissionDenied)) == "PermissionDenied");
    CHECK(std::string(consoleCmdExecResultName(ConsoleCmdExecResult::Error))            == "Error");
}

TEST_CASE("ConsoleCommand accessors", "[Phase8][ConsoleCmd]") {
    ConsoleCommand cmd("r.vsync", ConsoleCmdScope::Editor, ConsoleCmdArgType::Bool);
    cmd.setDescription("Toggle vsync");
    CHECK(cmd.name()        == "r.vsync");
    CHECK(cmd.scope()       == ConsoleCmdScope::Editor);
    CHECK(cmd.argType()     == ConsoleCmdArgType::Bool);
    CHECK(cmd.description() == "Toggle vsync");
    CHECK(cmd.isEnabled());
    CHECK_FALSE(cmd.isHidden());
}

TEST_CASE("ConsoleCommandBus registerCommand and execute", "[Phase8][ConsoleCmd]") {
    ConsoleCommandBus bus;
    ConsoleCommand cmd("reload", ConsoleCmdScope::Global, ConsoleCmdArgType::None);
    CHECK(bus.registerCommand(cmd));
    CHECK(bus.commandCount() == 1);

    auto result = bus.execute("reload");
    CHECK(result == ConsoleCmdExecResult::Ok);
    CHECK(bus.lastExec() == "reload");
}

TEST_CASE("ConsoleCommandBus rejects duplicate command names", "[Phase8][ConsoleCmd]") {
    ConsoleCommandBus bus;
    ConsoleCommand cmd("dup", ConsoleCmdScope::Global, ConsoleCmdArgType::None);
    CHECK(bus.registerCommand(cmd));
    CHECK_FALSE(bus.registerCommand(cmd));
    CHECK(bus.commandCount() == 1);
}

TEST_CASE("ConsoleCommandBus execute returns NotFound for unknown command",
          "[Phase8][ConsoleCmd]") {
    ConsoleCommandBus bus;
    CHECK(bus.execute("nonexistent") == ConsoleCmdExecResult::NotFound);
}

TEST_CASE("ConsoleCommandBus disabled command returns PermissionDenied",
          "[Phase8][ConsoleCmd]") {
    ConsoleCommandBus bus;
    ConsoleCommand cmd("locked", ConsoleCmdScope::Global, ConsoleCmdArgType::None);
    (void)bus.registerCommand(cmd);
    bus.findCommand("locked")->setEnabled(false);
    CHECK(bus.execute("locked") == ConsoleCmdExecResult::PermissionDenied);
}

TEST_CASE("ConsoleCommandBus unregisterCommand works", "[Phase8][ConsoleCmd]") {
    ConsoleCommandBus bus;
    ConsoleCommand cmd("temp", ConsoleCmdScope::Global, ConsoleCmdArgType::None);
    (void)bus.registerCommand(cmd);
    CHECK(bus.unregisterCommand("temp"));
    CHECK(bus.commandCount() == 0);
    CHECK_FALSE(bus.unregisterCommand("temp"));
}

TEST_CASE("ConsoleCommandBus countByScope and hiddenCount and enabledCount",
          "[Phase8][ConsoleCmd]") {
    ConsoleCommandBus bus;
    ConsoleCommand g1("g1", ConsoleCmdScope::Global, ConsoleCmdArgType::None);
    ConsoleCommand g2("g2", ConsoleCmdScope::Global, ConsoleCmdArgType::None);
    ConsoleCommand e1("e1", ConsoleCmdScope::Editor, ConsoleCmdArgType::None);

    g2.setHidden(true);
    e1.setEnabled(false);

    (void)bus.registerCommand(g1);
    (void)bus.registerCommand(g2);
    (void)bus.registerCommand(e1);

    CHECK(bus.countByScope(ConsoleCmdScope::Global) == 2);
    CHECK(bus.countByScope(ConsoleCmdScope::Editor) == 1);
    CHECK(bus.hiddenCount()  == 1);
    CHECK(bus.enabledCount() == 2); // g1 + g2 enabled, e1 disabled
}

// ═════════════════════════════════════════════════════════════════
// SECTION 6 — SelectionService
// ═════════════════════════════════════════════════════════════════

TEST_CASE("SelectionService default state is empty", "[Phase8][Selection]") {
    SelectionService svc;
    CHECK_FALSE(svc.hasSelection());
    CHECK(svc.selectionCount() == 0);
    CHECK_FALSE(svc.hasSelection());
    CHECK(svc.primarySelection() == INVALID_ENTITY);
}

TEST_CASE("SelectionService select and deselect", "[Phase8][Selection]") {
    SelectionService svc;
    svc.select(1);
    CHECK(svc.isSelected(1));
    CHECK(svc.selectionCount() == 1);
    CHECK(svc.primarySelection() == 1);

    svc.deselect(1);
    CHECK_FALSE(svc.isSelected(1));
    CHECK_FALSE(svc.hasSelection());
}

TEST_CASE("SelectionService toggleSelect works", "[Phase8][Selection]") {
    SelectionService svc;
    svc.toggleSelect(5);
    CHECK(svc.isSelected(5));
    svc.toggleSelect(5);
    CHECK_FALSE(svc.isSelected(5));
}

TEST_CASE("SelectionService multi-select accumulates", "[Phase8][Selection]") {
    SelectionService svc;
    svc.select(1);
    svc.select(2);
    svc.select(3);
    CHECK(svc.selectionCount() == 3);
    CHECK(svc.isSelected(1));
    CHECK(svc.isSelected(2));
    CHECK(svc.isSelected(3));
}

TEST_CASE("SelectionService clearSelection empties selection", "[Phase8][Selection]") {
    SelectionService svc;
    svc.select(1);
    svc.select(2);
    svc.clearSelection();
    CHECK_FALSE(svc.hasSelection());
    CHECK(svc.primarySelection() == INVALID_ENTITY);
}

TEST_CASE("SelectionService selectExclusive replaces all", "[Phase8][Selection]") {
    SelectionService svc;
    svc.select(1);
    svc.select(2);
    svc.selectExclusive(7);
    CHECK(svc.selectionCount() == 1);
    CHECK(svc.isSelected(7));
    CHECK_FALSE(svc.isSelected(1));
    CHECK(svc.primarySelection() == 7);
}

TEST_CASE("SelectionService version increments on change", "[Phase8][Selection]") {
    SelectionService svc;
    uint32_t v0 = svc.version();
    svc.select(1);
    CHECK(svc.version() > v0);
    uint32_t v1 = svc.version();
    svc.deselect(1);
    CHECK(svc.version() > v1);
}

TEST_CASE("SelectionService primary tracks last selected when deselected",
          "[Phase8][Selection]") {
    SelectionService svc;
    svc.select(10);
    svc.select(20);
    // primary is 20 (last selected)
    svc.deselect(20);
    // primary should fall back to something in the set
    CHECK(svc.primarySelection() != INVALID_ENTITY);
}

// ═════════════════════════════════════════════════════════════════
// SECTION 7 — EditorEventBus
// ═════════════════════════════════════════════════════════════════

TEST_CASE("editorEventPriorityName returns correct strings", "[Phase8][EventBus]") {
    CHECK(std::string(editorEventPriorityName(EditorEventPriority::Lowest))   == "Lowest");
    CHECK(std::string(editorEventPriorityName(EditorEventPriority::Normal))   == "Normal");
    CHECK(std::string(editorEventPriorityName(EditorEventPriority::High))     == "High");
    CHECK(std::string(editorEventPriorityName(EditorEventPriority::Critical)) == "Critical");
    CHECK(std::string(editorEventPriorityName(EditorEventPriority::Realtime)) == "Realtime");
}

TEST_CASE("EditorBusEvent consume and priority helpers", "[Phase8][EventBus]") {
    EditorBusEvent ev;
    ev.priority = EditorEventPriority::High;
    CHECK(ev.isHighPrio());
    CHECK_FALSE(ev.isCritical());
    CHECK_FALSE(ev.isConsumed());
    ev.consume();
    CHECK(ev.isConsumed());
}

TEST_CASE("EditorEventBus default state", "[Phase8][EventBus]") {
    EditorEventBus bus;
    CHECK(bus.state()             == EditorBusState::Idle);
    CHECK(bus.queueSize()         == 0);
    CHECK(bus.subscriptionCount() == 0);
    CHECK_FALSE(bus.isSuspended());
}

TEST_CASE("EditorEventBus subscribe and flush delivers events", "[Phase8][EventBus]") {
    EditorEventBus bus;

    int callCount = 0;
    std::string lastPayload;
    bus.subscribe("scene.changed", EditorEventPriority::Normal,
        [&](const EditorBusEvent& ev) {
            ++callCount;
            lastPayload = ev.payload;
        });

    EditorBusEvent ev;
    ev.topic   = "scene.changed";
    ev.payload = "obj42";
    ev.priority = EditorEventPriority::Normal;
    bus.post(ev);
    CHECK(bus.queueSize() == 1);

    size_t dispatched = bus.flush();
    CHECK(dispatched == 1);
    CHECK(callCount == 1);
    CHECK(lastPayload == "obj42");
    CHECK(bus.queueSize() == 0);
}

TEST_CASE("EditorEventBus wildcard subscription receives all events",
          "[Phase8][EventBus]") {
    EditorEventBus bus;
    int count = 0;
    bus.subscribe("*", EditorEventPriority::Lowest,
        [&](const EditorBusEvent&) { ++count; });

    EditorBusEvent ev1; ev1.topic = "scene.changed";  ev1.priority = EditorEventPriority::Normal;
    EditorBusEvent ev2; ev2.topic = "asset.imported";  ev2.priority = EditorEventPriority::Normal;
    bus.post(ev1);
    bus.post(ev2);
    bus.flush();
    CHECK(count == 2);
}

TEST_CASE("EditorEventBus priority filter skips low priority events",
          "[Phase8][EventBus]") {
    EditorEventBus bus;
    int highCount = 0;
    bus.subscribe("topic", EditorEventPriority::High,
        [&](const EditorBusEvent&) { ++highCount; });

    EditorBusEvent low;  low.topic  = "topic"; low.priority  = EditorEventPriority::Normal;
    EditorBusEvent high; high.topic = "topic"; high.priority = EditorEventPriority::High;
    bus.post(low);
    bus.post(high);
    bus.flush();
    CHECK(highCount == 1); // only the high-priority event delivered
}

TEST_CASE("EditorEventBus suspend blocks posting and flush", "[Phase8][EventBus]") {
    EditorEventBus bus;
    bus.suspend();
    CHECK(bus.isSuspended());

    EditorBusEvent ev; ev.topic = "t"; ev.priority = EditorEventPriority::Normal;
    CHECK_FALSE(bus.post(ev)); // post fails when suspended
    CHECK(bus.queueSize() == 0);
    CHECK(bus.flush() == 0);  // flush does nothing when suspended
}

TEST_CASE("EditorEventBus resume re-enables posting", "[Phase8][EventBus]") {
    EditorEventBus bus;
    bus.suspend();
    bus.resume();
    CHECK_FALSE(bus.isSuspended());

    EditorBusEvent ev; ev.topic = "t"; ev.priority = EditorEventPriority::Normal;
    CHECK(bus.post(ev));
}

TEST_CASE("EditorEventBus clearQueue empties pending events", "[Phase8][EventBus]") {
    EditorEventBus bus;
    EditorBusEvent ev; ev.topic = "t"; ev.priority = EditorEventPriority::Normal;
    bus.post(ev);
    bus.post(ev);
    CHECK(bus.queueSize() == 2);
    bus.clearQueue();
    CHECK(bus.queueSize() == 0);
}

TEST_CASE("EditorEventSubscription tracks callCount and cancel", "[Phase8][EventBus]") {
    EditorEventBus bus;
    EditorEventSubscription* sub = bus.subscribe("topic", EditorEventPriority::Normal,
        [](const EditorBusEvent&) {});

    REQUIRE(sub != nullptr);
    CHECK(sub->isActive());
    CHECK(sub->callCount() == 0);

    EditorBusEvent ev; ev.topic = "topic"; ev.priority = EditorEventPriority::Normal;
    bus.post(ev);
    bus.flush();
    CHECK(sub->callCount() == 1);

    sub->cancel();
    CHECK_FALSE(sub->isActive());

    // After cancel, further deliveries should not increment
    bus.post(ev);
    bus.flush();
    CHECK(sub->callCount() == 1); // unchanged
}

TEST_CASE("EditorEventBus non-matching topic not delivered", "[Phase8][EventBus]") {
    EditorEventBus bus;
    int count = 0;
    bus.subscribe("scene.saved", EditorEventPriority::Normal,
        [&](const EditorBusEvent&) { ++count; });

    EditorBusEvent ev; ev.topic = "asset.imported"; ev.priority = EditorEventPriority::Normal;
    bus.post(ev);
    bus.flush();
    CHECK(count == 0);
}
