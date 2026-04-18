#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"

using namespace NF;

// ── WorkspaceAppId ───────────────────────────────────────────────

TEST_CASE("WorkspaceAppId names are distinct", "[Editor][S64]") {
    REQUIRE(WorkspaceAppId::NovaForgeEditor != WorkspaceAppId::NovaForgeGame);
    REQUIRE(WorkspaceAppId::NovaForgeGame   != WorkspaceAppId::NovaForgeServer);
    REQUIRE(WorkspaceAppId::NovaForgeEditor != WorkspaceAppId::Unknown);
    REQUIRE(WorkspaceAppId::TileEditor      != WorkspaceAppId::Unknown);
}

TEST_CASE("workspaceAppName returns correct strings", "[Editor][S64]") {
    REQUIRE(std::string(workspaceAppName(WorkspaceAppId::NovaForgeEditor)) == "NovaForgeEditor");
    REQUIRE(std::string(workspaceAppName(WorkspaceAppId::NovaForgeGame))   == "NovaForgeGame");
    REQUIRE(std::string(workspaceAppName(WorkspaceAppId::NovaForgeServer)) == "NovaForgeServer");
    REQUIRE(std::string(workspaceAppName(WorkspaceAppId::TileEditor))      == "TileEditor");
    REQUIRE(std::string(workspaceAppName(WorkspaceAppId::Unknown))         == "Unknown");
}

// ── WorkspaceAppDescriptor ───────────────────────────────────────

TEST_CASE("WorkspaceAppDescriptor defaults are invalid", "[Editor][S64]") {
    WorkspaceAppDescriptor d;
    REQUIRE_FALSE(d.isValid());
}

TEST_CASE("WorkspaceAppDescriptor valid when fully populated", "[Editor][S64]") {
    WorkspaceAppDescriptor d;
    d.id             = WorkspaceAppId::NovaForgeEditor;
    d.name           = "NovaForgeEditor";
    d.executablePath = "NovaForgeEditor.exe";
    REQUIRE(d.isValid());
}

TEST_CASE("WorkspaceAppDescriptor invalid without name", "[Editor][S64]") {
    WorkspaceAppDescriptor d;
    d.id             = WorkspaceAppId::NovaForgeEditor;
    d.executablePath = "NovaForgeEditor.exe";
    REQUIRE_FALSE(d.isValid());
}

TEST_CASE("WorkspaceAppDescriptor invalid without path", "[Editor][S64]") {
    WorkspaceAppDescriptor d;
    d.id   = WorkspaceAppId::NovaForgeEditor;
    d.name = "NovaForgeEditor";
    REQUIRE_FALSE(d.isValid());
}

TEST_CASE("WorkspaceAppDescriptor displayLabel includes name and path", "[Editor][S64]") {
    WorkspaceAppDescriptor d;
    d.id             = WorkspaceAppId::NovaForgeGame;
    d.name           = "NovaForgeGame";
    d.executablePath = "NovaForgeGame.exe";
    std::string label = d.displayLabel();
    REQUIRE(label.find("NovaForgeGame") != std::string::npos);
    REQUIRE(label.find("NovaForgeGame.exe") != std::string::npos);
}

// ── WorkspaceAppRegistry ─────────────────────────────────────────

TEST_CASE("WorkspaceAppRegistry starts empty", "[Editor][S64]") {
    WorkspaceAppRegistry reg;
    REQUIRE(reg.count() == 0);
    REQUIRE(reg.empty());
}

TEST_CASE("WorkspaceAppRegistry registerApp succeeds", "[Editor][S64]") {
    WorkspaceAppRegistry reg;
    WorkspaceAppDescriptor d;
    d.id = WorkspaceAppId::NovaForgeEditor;
    d.name = "NovaForgeEditor";
    d.executablePath = "NovaForgeEditor.exe";
    REQUIRE(reg.registerApp(d));
    REQUIRE(reg.count() == 1);
    REQUIRE(reg.isRegistered(WorkspaceAppId::NovaForgeEditor));
}

TEST_CASE("WorkspaceAppRegistry registerApp rejects invalid descriptor", "[Editor][S64]") {
    WorkspaceAppRegistry reg;
    WorkspaceAppDescriptor d; // default = invalid
    REQUIRE_FALSE(reg.registerApp(d));
    REQUIRE(reg.empty());
}

TEST_CASE("WorkspaceAppRegistry registerApp rejects duplicate id", "[Editor][S64]") {
    WorkspaceAppRegistry reg;
    WorkspaceAppDescriptor d;
    d.id = WorkspaceAppId::NovaForgeEditor;
    d.name = "NovaForgeEditor";
    d.executablePath = "NovaForgeEditor.exe";
    REQUIRE(reg.registerApp(d));
    REQUIRE_FALSE(reg.registerApp(d));
    REQUIRE(reg.count() == 1);
}

TEST_CASE("WorkspaceAppRegistry find returns correct descriptor", "[Editor][S64]") {
    WorkspaceAppRegistry reg;
    WorkspaceAppDescriptor d;
    d.id = WorkspaceAppId::NovaForgeGame;
    d.name = "NovaForgeGame";
    d.executablePath = "NovaForgeGame.exe";
    reg.registerApp(d);
    const WorkspaceAppDescriptor* found = reg.find(WorkspaceAppId::NovaForgeGame);
    REQUIRE(found != nullptr);
    REQUIRE(found->name == "NovaForgeGame");
}

TEST_CASE("WorkspaceAppRegistry find returns nullptr for unknown id", "[Editor][S64]") {
    WorkspaceAppRegistry reg;
    REQUIRE(reg.find(WorkspaceAppId::NovaForgeEditor) == nullptr);
}

TEST_CASE("WorkspaceAppRegistry findByName works", "[Editor][S64]") {
    WorkspaceAppRegistry reg;
    WorkspaceAppDescriptor d;
    d.id = WorkspaceAppId::TileEditor;
    d.name = "TileEditor";
    d.executablePath = "TileEditor.exe";
    reg.registerApp(d);
    REQUIRE(reg.findByName("TileEditor") != nullptr);
    REQUIRE(reg.findByName("Unknown") == nullptr);
}

TEST_CASE("WorkspaceAppRegistry unregisterApp removes entry", "[Editor][S64]") {
    WorkspaceAppRegistry reg;
    WorkspaceAppDescriptor d;
    d.id = WorkspaceAppId::NovaForgeServer;
    d.name = "NovaForgeServer";
    d.executablePath = "NovaForgeServer.exe";
    reg.registerApp(d);
    REQUIRE(reg.isRegistered(WorkspaceAppId::NovaForgeServer));
    REQUIRE(reg.unregisterApp(WorkspaceAppId::NovaForgeServer));
    REQUIRE_FALSE(reg.isRegistered(WorkspaceAppId::NovaForgeServer));
}

TEST_CASE("WorkspaceAppRegistry unregisterApp returns false for unknown id", "[Editor][S64]") {
    WorkspaceAppRegistry reg;
    REQUIRE_FALSE(reg.unregisterApp(WorkspaceAppId::TileEditor));
}

TEST_CASE("WorkspaceAppRegistry projectScopedApps filters correctly", "[Editor][S64]") {
    WorkspaceAppRegistry reg;
    WorkspaceAppDescriptor scoped;
    scoped.id = WorkspaceAppId::NovaForgeEditor;
    scoped.name = "NovaForgeEditor";
    scoped.executablePath = "NovaForgeEditor.exe";
    scoped.isProjectScoped = true;

    WorkspaceAppDescriptor notScoped;
    notScoped.id = WorkspaceAppId::TileEditor;
    notScoped.name = "TileEditor";
    notScoped.executablePath = "TileEditor.exe";
    notScoped.isProjectScoped = false;

    reg.registerApp(scoped);
    reg.registerApp(notScoped);
    REQUIRE(reg.count() == 2);
    auto ps = reg.projectScopedApps();
    REQUIRE(ps.size() == 1);
    REQUIRE(ps[0]->name == "NovaForgeEditor");
}

// ── WorkspaceLaunchMode ───────────────────────────────────────────────────

TEST_CASE("WorkspaceLaunchMode names are correct", "[Editor][S64]") {
    REQUIRE(std::string(workspaceLaunchModeName(WorkspaceLaunchMode::Hosted))   == "hosted");
    REQUIRE(std::string(workspaceLaunchModeName(WorkspaceLaunchMode::Headless)) == "headless");
    REQUIRE(std::string(workspaceLaunchModeName(WorkspaceLaunchMode::Preview))  == "preview");
}

// ── WorkspaceLaunchContext ────────────────────────────────────────────────

TEST_CASE("WorkspaceLaunchContext default is invalid", "[Editor][S64]") {
    WorkspaceLaunchContext ctx;
    REQUIRE_FALSE(ctx.isValid());
}

TEST_CASE("WorkspaceLaunchContext valid when required fields set", "[Editor][S64]") {
    WorkspaceLaunchContext ctx;
    ctx.workspaceRoot = "/workspace";
    ctx.projectPath   = "/workspace/Project/project.atlas.json";
    ctx.sessionId     = "sess-001";
    REQUIRE(ctx.isValid());
}

TEST_CASE("WorkspaceLaunchContext invalid without workspaceRoot", "[Editor][S64]") {
    WorkspaceLaunchContext ctx;
    ctx.projectPath = "/workspace/Project/project.atlas.json";
    ctx.sessionId   = "sess-001";
    REQUIRE_FALSE(ctx.isValid());
}

TEST_CASE("WorkspaceLaunchContext toArgs builds correct argument list", "[Editor][S64]") {
    WorkspaceLaunchContext ctx;
    ctx.workspaceRoot = "/workspace";
    ctx.projectPath   = "/workspace/Project/project.atlas.json";
    ctx.sessionId     = "sess-001";
    ctx.ipcEndpoint   = "atlas.pipe.1234";
    ctx.logSinkPath   = "/workspace/Logs/editor.log";
    ctx.mode          = WorkspaceLaunchMode::Hosted;

    auto args = ctx.toArgs();
    REQUIRE(args.size() >= 5);

    bool hasMode     = false, hasRoot = false, hasProject = false;
    bool hasSession  = false, hasIpc  = false, hasLog     = false;
    for (const auto& a : args) {
        if (a == "--hosted")                                     hasMode    = true;
        if (a.find("--workspace-root=") != std::string::npos)   hasRoot    = true;
        if (a.find("--project=") != std::string::npos)          hasProject = true;
        if (a.find("--session-id=") != std::string::npos)       hasSession = true;
        if (a.find("--ipc=") != std::string::npos)              hasIpc     = true;
        if (a.find("--log=") != std::string::npos)              hasLog     = true;
    }
    REQUIRE(hasMode);
    REQUIRE(hasRoot);
    REQUIRE(hasProject);
    REQUIRE(hasSession);
    REQUIRE(hasIpc);
    REQUIRE(hasLog);
}

TEST_CASE("WorkspaceLaunchContext toArgs omits ipc when empty", "[Editor][S64]") {
    WorkspaceLaunchContext ctx;
    ctx.workspaceRoot = "/workspace";
    ctx.projectPath   = "/workspace/Project/project.atlas.json";
    ctx.sessionId     = "sess-002";
    // no ipcEndpoint
    auto args = ctx.toArgs();
    for (const auto& a : args)
        REQUIRE(a.find("--ipc=") == std::string::npos);
}

// ── WorkspaceLaunchStatus ─────────────────────────────────────────────────

TEST_CASE("WorkspaceLaunchStatus names are correct", "[Editor][S64]") {
    REQUIRE(std::string(workspaceLaunchStatusName(WorkspaceLaunchStatus::Success))            == "Success");
    REQUIRE(std::string(workspaceLaunchStatusName(WorkspaceLaunchStatus::AppNotFound))        == "AppNotFound");
    REQUIRE(std::string(workspaceLaunchStatusName(WorkspaceLaunchStatus::ExecutableNotFound)) == "ExecutableNotFound");
    REQUIRE(std::string(workspaceLaunchStatusName(WorkspaceLaunchStatus::InvalidContext))     == "InvalidContext");
    REQUIRE(std::string(workspaceLaunchStatusName(WorkspaceLaunchStatus::AlreadyRunning))     == "AlreadyRunning");
    REQUIRE(std::string(workspaceLaunchStatusName(WorkspaceLaunchStatus::SpawnFailed))        == "SpawnFailed");
}

// ── WorkspaceLaunchResult ─────────────────────────────────────────────────

TEST_CASE("WorkspaceLaunchResult defaults to failed", "[Editor][S64]") {
    WorkspaceLaunchResult r;
    REQUIRE_FALSE(r.succeeded());
    REQUIRE_FALSE(r.isRunning());
    REQUIRE(r.pid == 0);
}

TEST_CASE("WorkspaceLaunchResult succeeded and isRunning when success with pid", "[Editor][S64]") {
    WorkspaceLaunchResult r;
    r.status = WorkspaceLaunchStatus::Success;
    r.pid    = 1234;
    REQUIRE(r.succeeded());
    REQUIRE(r.isRunning());
}

TEST_CASE("WorkspaceLaunchResult succeeded but not running when pid is zero", "[Editor][S64]") {
    WorkspaceLaunchResult r;
    r.status = WorkspaceLaunchStatus::Success;
    r.pid    = 0;
    REQUIRE(r.succeeded());
    REQUIRE_FALSE(r.isRunning());
}

// ── NullLaunchService ────────────────────────────────────────────

TEST_CASE("NullLaunchService launch valid app and context succeeds", "[Editor][S64]") {
    NullLaunchService svc;
    WorkspaceAppDescriptor app;
    app.id             = WorkspaceAppId::NovaForgeEditor;
    app.name           = "NovaForgeEditor";
    app.executablePath = "NovaForgeEditor.exe";

    WorkspaceLaunchContext ctx;
    ctx.workspaceRoot = "/workspace";
    ctx.projectPath   = "/workspace/Project/project.atlas.json";
    ctx.sessionId     = "sess-001";

    WorkspaceLaunchResult r = svc.launchApp(app, ctx);
    REQUIRE(r.succeeded());
    REQUIRE(svc.isRunning(WorkspaceAppId::NovaForgeEditor));
    REQUIRE(svc.pidOf(WorkspaceAppId::NovaForgeEditor) != 0);
}

TEST_CASE("NullLaunchService rejects invalid app descriptor", "[Editor][S64]") {
    NullLaunchService svc;
    WorkspaceAppDescriptor bad; // default = invalid
    WorkspaceLaunchContext ctx;
    ctx.workspaceRoot = "/workspace";
    ctx.projectPath   = "/p/project.atlas.json";
    ctx.sessionId     = "s";
    WorkspaceLaunchResult r = svc.launchApp(bad, ctx);
    REQUIRE(r.status == WorkspaceLaunchStatus::AppNotFound);
    REQUIRE_FALSE(r.succeeded());
}

TEST_CASE("NullLaunchService rejects invalid context", "[Editor][S64]") {
    NullLaunchService svc;
    WorkspaceAppDescriptor app;
    app.id = WorkspaceAppId::NovaForgeGame;
    app.name = "NovaForgeGame";
    app.executablePath = "NovaForgeGame.exe";
    WorkspaceLaunchContext bad; // default = invalid
    WorkspaceLaunchResult r = svc.launchApp(app, bad);
    REQUIRE(r.status == WorkspaceLaunchStatus::InvalidContext);
}

TEST_CASE("NullLaunchService shutdownApp stops running state", "[Editor][S64]") {
    NullLaunchService svc;
    WorkspaceAppDescriptor app;
    app.id = WorkspaceAppId::NovaForgeServer;
    app.name = "NovaForgeServer";
    app.executablePath = "NovaForgeServer.exe";
    WorkspaceLaunchContext ctx;
    ctx.workspaceRoot = "/w"; ctx.projectPath = "/w/p"; ctx.sessionId = "s";

    svc.launchApp(app, ctx);
    REQUIRE(svc.isRunning(WorkspaceAppId::NovaForgeServer));
    svc.shutdownApp(WorkspaceAppId::NovaForgeServer);
    REQUIRE_FALSE(svc.isRunning(WorkspaceAppId::NovaForgeServer));
    REQUIRE(svc.pidOf(WorkspaceAppId::NovaForgeServer) == 0);
}

TEST_CASE("NullLaunchService not running before launch", "[Editor][S64]") {
    NullLaunchService svc;
    REQUIRE_FALSE(svc.isRunning(WorkspaceAppId::NovaForgeEditor));
    REQUIRE(svc.pidOf(WorkspaceAppId::NovaForgeEditor) == 0);
}
