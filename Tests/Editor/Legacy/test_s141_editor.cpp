// S141 editor tests: AtlasAIPanelHost, AIPanelSession, AIActionSurface
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"

using namespace NF;
using Catch::Approx;

// ── AtlasAIPanelHost ──────────────────────────────────────────────────────────

TEST_CASE("AIPanelVisibility names", "[Editor][S141]") {
    REQUIRE(std::string(aiPanelVisibilityName(AIPanelVisibility::Hidden))     == "Hidden");
    REQUIRE(std::string(aiPanelVisibilityName(AIPanelVisibility::Slideout))   == "Slideout");
    REQUIRE(std::string(aiPanelVisibilityName(AIPanelVisibility::Docked))     == "Docked");
    REQUIRE(std::string(aiPanelVisibilityName(AIPanelVisibility::Floating))   == "Floating");
    REQUIRE(std::string(aiPanelVisibilityName(AIPanelVisibility::Fullscreen)) == "Fullscreen");
}

TEST_CASE("AIPanelEdge names", "[Editor][S141]") {
    REQUIRE(std::string(aiPanelEdgeName(AIPanelEdge::Right))  == "Right");
    REQUIRE(std::string(aiPanelEdgeName(AIPanelEdge::Bottom)) == "Bottom");
    REQUIRE(std::string(aiPanelEdgeName(AIPanelEdge::Left))   == "Left");
}

TEST_CASE("AtlasAIPanelHost initialize", "[Editor][S141]") {
    AtlasAIPanelHost host;
    REQUIRE(!host.isInitialized());
    host.initialize();
    REQUIRE(host.isInitialized());
    REQUIRE(host.visibility() == AIPanelVisibility::Hidden);
    REQUIRE(!host.isVisible());
}

TEST_CASE("AtlasAIPanelHost show and hide", "[Editor][S141]") {
    AtlasAIPanelHost host;
    host.initialize();
    host.show(AIPanelVisibility::Slideout);
    REQUIRE(host.isVisible());
    REQUIRE(host.visibility() == AIPanelVisibility::Slideout);
    REQUIRE(host.showCount() == 1u);

    host.hide();
    REQUIRE(!host.isVisible());
    REQUIRE(host.hideCount() == 1u);
}

TEST_CASE("AtlasAIPanelHost toggle", "[Editor][S141]") {
    AtlasAIPanelHost host;
    host.initialize();
    REQUIRE(!host.isVisible());
    host.toggle();
    REQUIRE(host.isVisible());
    host.toggle();
    REQUIRE(!host.isVisible());
}

TEST_CASE("AtlasAIPanelHost dock edge and dimensions", "[Editor][S141]") {
    AtlasAIPanelHost host;
    host.initialize();
    REQUIRE(host.dockEdge() == AIPanelEdge::Right);
    host.setDockEdge(AIPanelEdge::Bottom);
    REQUIRE(host.dockEdge() == AIPanelEdge::Bottom);

    host.setWidthPx(420.f);
    REQUIRE(host.widthPx() == Catch::Approx(420.f));
    host.setHeightPx(500.f);
    REQUIRE(host.heightPx() == Catch::Approx(500.f));
}

TEST_CASE("AtlasAIPanelHost minimum dimension enforcement", "[Editor][S141]") {
    AtlasAIPanelHost host;
    host.initialize();
    host.setWidthPx(10.f);   // below minimum 80px
    REQUIRE(host.widthPx() == Catch::Approx(80.f));
    host.setHeightPx(20.f);
    REQUIRE(host.heightPx() == Catch::Approx(80.f));
}

TEST_CASE("AtlasAIPanelHost attach panel", "[Editor][S141]") {
    AtlasAIPanelHost host;
    host.initialize();
    REQUIRE(host.panel() == nullptr);

    AIAssistantPanel panel;
    host.attachPanel(&panel);
    REQUIRE(host.panel() == &panel);
}

TEST_CASE("AtlasAIPanelHost escalate error notification to AI", "[Editor][S141]") {
    AtlasAIPanelHost host;
    host.initialize();
    AIAssistantPanel panel;
    host.attachPanel(&panel);

    Notification n;
    n.id = "n1"; n.title = "Build failed"; n.message = "Linker error";
    n.severity = NotificationSeverity::Error;

    REQUIRE(host.escalateToAI(n));
    REQUIRE(host.escalatedCount() == 1u);
    REQUIRE(panel.session().messageCount() == 1u);
    REQUIRE(host.isVisible());  // auto-showed
}

TEST_CASE("AtlasAIPanelHost does not escalate info notification", "[Editor][S141]") {
    AtlasAIPanelHost host;
    host.initialize();
    AIAssistantPanel panel;
    host.attachPanel(&panel);

    Notification n;
    n.id = "n2"; n.title = "Build started"; n.severity = NotificationSeverity::Info;
    REQUIRE(!host.escalateToAI(n));
    REQUIRE(host.escalatedCount() == 0u);
}

TEST_CASE("AtlasAIPanelHost escalate without panel returns false", "[Editor][S141]") {
    AtlasAIPanelHost host;
    host.initialize();
    Notification n;
    n.id = "n3"; n.title = "Error"; n.severity = NotificationSeverity::Error;
    REQUIRE(!host.escalateToAI(n));
}

// ── AIPanelSession ────────────────────────────────────────────────────────────

TEST_CASE("AISessionContextType names", "[Editor][S141]") {
    REQUIRE(std::string(aiSessionContextTypeName(AISessionContextType::None))         == "None");
    REQUIRE(std::string(aiSessionContextTypeName(AISessionContextType::File))         == "File");
    REQUIRE(std::string(aiSessionContextTypeName(AISessionContextType::Selection))    == "Selection");
    REQUIRE(std::string(aiSessionContextTypeName(AISessionContextType::Error))        == "Error");
    REQUIRE(std::string(aiSessionContextTypeName(AISessionContextType::Notification)) == "Notification");
    REQUIRE(std::string(aiSessionContextTypeName(AISessionContextType::Diff))         == "Diff");
    REQUIRE(std::string(aiSessionContextTypeName(AISessionContextType::Log))          == "Log");
}

TEST_CASE("AISessionContext validity", "[Editor][S141]") {
    AISessionContext ctx;
    REQUIRE(!ctx.isValid());
    ctx.type = AISessionContextType::File;
    ctx.content = "some content";
    REQUIRE(ctx.isValid());
}

TEST_CASE("AIPanelSession initial state", "[Editor][S141]") {
    AIPanelSession session("sess_1");
    REQUIRE(session.sessionId() == "sess_1");
    REQUIRE(session.isEmpty());
    REQUIRE(session.turnCount()   == 0u);
    REQUIRE(session.contextCount() == 0u);
}

TEST_CASE("AIPanelSession addContext and findContext", "[Editor][S141]") {
    AIPanelSession session;
    AISessionContext ctx;
    ctx.type = AISessionContextType::File;
    ctx.label = "main.cpp";
    ctx.content = "int main() {}";
    REQUIRE(session.addContext(ctx));
    REQUIRE(session.contextCount() == 1u);

    const auto* found = session.findContext("main.cpp");
    REQUIRE(found != nullptr);
    REQUIRE(found->type == AISessionContextType::File);
}

TEST_CASE("AIPanelSession addContext rejects invalid", "[Editor][S141]") {
    AIPanelSession session;
    AISessionContext invalid; // type=None, content empty
    REQUIRE(!session.addContext(invalid));
}

TEST_CASE("AIPanelSession removeContext", "[Editor][S141]") {
    AIPanelSession session;
    AISessionContext ctx;
    ctx.type = AISessionContextType::Error; ctx.label = "err1";
    ctx.content = "error text";
    session.addContext(ctx);
    REQUIRE(session.removeContext("err1"));
    REQUIRE(session.contextCount() == 0u);
    REQUIRE(!session.removeContext("err1")); // not found
}

TEST_CASE("AIPanelSession clearUnpinnedContexts", "[Editor][S141]") {
    AIPanelSession session;
    auto mk = [](AISessionContextType t, const char* label, const char* content, bool pinned) {
        AISessionContext c; c.type = t; c.label = label;
        c.content = content; c.pinned = pinned; return c;
    };
    session.addContext(mk(AISessionContextType::File, "f1", "code", true));
    session.addContext(mk(AISessionContextType::Error, "e1", "err", false));
    session.addContext(mk(AISessionContextType::Log, "l1", "log", false));
    REQUIRE(session.contextCount() == 3u);
    session.clearUnpinnedContexts();
    REQUIRE(session.contextCount() == 1u);
    REQUIRE(session.findContext("f1") != nullptr);
}

TEST_CASE("AIPanelSession user and assistant turns", "[Editor][S141]") {
    AIPanelSession session;
    session.submitUserTurn("Fix the shader error");
    session.receiveAssistantResponse("Here is the fix: ...");
    REQUIRE(session.turnCount()    == 1u);
    REQUIRE(session.messageCount() == 2u);
}

TEST_CASE("AIPanelSession reset clears everything", "[Editor][S141]") {
    AIPanelSession session;
    AISessionContext ctx;
    ctx.type = AISessionContextType::File; ctx.label = "f"; ctx.content = "x";
    session.addContext(ctx);
    session.submitUserTurn("Hello");
    session.reset();
    REQUIRE(session.isEmpty());
    REQUIRE(session.turnCount()   == 0u);
    REQUIRE(session.contextCount() == 0u);
}

TEST_CASE("AIPanelSession title", "[Editor][S141]") {
    AIPanelSession session;
    REQUIRE(session.title().empty());
    session.setTitle("Debug session");
    REQUIRE(session.title() == "Debug session");
}

// ── AIActionSurface ───────────────────────────────────────────────────────────

TEST_CASE("AIActionType names", "[Editor][S141]") {
    REQUIRE(std::string(aiActionTypeName(AIActionType::ApplyDiff))       == "ApplyDiff");
    REQUIRE(std::string(aiActionTypeName(AIActionType::RejectDiff))      == "RejectDiff");
    REQUIRE(std::string(aiActionTypeName(AIActionType::RunCommand))      == "RunCommand");
    REQUIRE(std::string(aiActionTypeName(AIActionType::OpenFile))        == "OpenFile");
    REQUIRE(std::string(aiActionTypeName(AIActionType::InsertSnippet))   == "InsertSnippet");
    REQUIRE(std::string(aiActionTypeName(AIActionType::ShowExplanation)) == "ShowExplanation");
}

TEST_CASE("AIActionStatus names", "[Editor][S141]") {
    REQUIRE(std::string(aiActionStatusName(AIActionStatus::Pending))  == "Pending");
    REQUIRE(std::string(aiActionStatusName(AIActionStatus::Applied))  == "Applied");
    REQUIRE(std::string(aiActionStatusName(AIActionStatus::Rejected)) == "Rejected");
    REQUIRE(std::string(aiActionStatusName(AIActionStatus::Failed))   == "Failed");
}

TEST_CASE("AIActionItem validity", "[Editor][S141]") {
    AIActionItem item;
    REQUIRE(!item.isValid());
    item.id = 1; item.payload = "diff content";
    REQUIRE(item.isValid());
}

TEST_CASE("AIActionSurface propose action", "[Editor][S141]") {
    AIActionSurface surface;
    uint32_t id = surface.propose(AIActionType::ApplyDiff, "Fix shader",
                                   "--- a/shader.hlsl\n+++ b/shader.hlsl\n@@ ...@@",
                                   "shader.hlsl");
    REQUIRE(id != 0u);
    REQUIRE(surface.pendingCount() == 1u);
    REQUIRE(surface.totalProposed() == 1u);

    const auto* item = surface.find(id);
    REQUIRE(item != nullptr);
    REQUIRE(item->type == AIActionType::ApplyDiff);
    REQUIRE(item->isPending());
    REQUIRE(item->targetPath == "shader.hlsl");
}

TEST_CASE("AIActionSurface apply action", "[Editor][S141]") {
    AIActionSurface surface;
    auto id = surface.propose(AIActionType::ApplyDiff, "Fix", "diff_content");
    REQUIRE(surface.apply(id));
    REQUIRE(surface.find(id)->isApplied());
    REQUIRE(surface.totalApplied() == 1u);
    REQUIRE(surface.pendingCount() == 0u);
}

TEST_CASE("AIActionSurface apply with handler success", "[Editor][S141]") {
    AIActionSurface surface;
    bool handlerCalled = false;
    surface.setApplyHandler([&](AIActionItem& item) {
        handlerCalled = true;
        item.payload = item.payload + " [applied]";
        return true;
    });
    auto id = surface.propose(AIActionType::InsertSnippet, "Snippet", "code");
    REQUIRE(surface.apply(id));
    REQUIRE(handlerCalled);
    REQUIRE(surface.find(id)->payload == "code [applied]");
}

TEST_CASE("AIActionSurface apply with handler failure", "[Editor][S141]") {
    AIActionSurface surface;
    surface.setApplyHandler([](AIActionItem&) { return false; });
    auto id = surface.propose(AIActionType::RunCommand, "Run", "build.cmd");
    REQUIRE(!surface.apply(id));
    REQUIRE(surface.find(id)->status == AIActionStatus::Failed);
    REQUIRE(surface.totalFailed() == 1u);
}

TEST_CASE("AIActionSurface reject action", "[Editor][S141]") {
    AIActionSurface surface;
    auto id = surface.propose(AIActionType::RejectDiff, "Reject", "diff");
    REQUIRE(surface.reject(id));
    REQUIRE(surface.find(id)->isRejected());
    REQUIRE(surface.totalRejected() == 1u);
}

TEST_CASE("AIActionSurface cannot apply non-pending", "[Editor][S141]") {
    AIActionSurface surface;
    auto id = surface.propose(AIActionType::ApplyDiff, "Fix", "diff");
    surface.apply(id);  // applied
    REQUIRE(!surface.apply(id));  // cannot apply again
}

TEST_CASE("AIActionSurface confirm action", "[Editor][S141]") {
    AIActionSurface surface;
    auto id = surface.propose(AIActionType::RunCommand, "Build", "make");
    REQUIRE(surface.confirm(id));
    REQUIRE(surface.find(id)->confirmed);
}

TEST_CASE("AIActionSurface clearFinished removes applied and rejected", "[Editor][S141]") {
    AIActionSurface surface;
    auto id1 = surface.propose(AIActionType::ApplyDiff, "A", "diff");
    auto id2 = surface.propose(AIActionType::RejectDiff, "B", "diff");
    auto id3 = surface.propose(AIActionType::OpenFile, "C", "file.cpp");
    surface.apply(id1);
    surface.reject(id2);
    // id3 remains pending
    surface.clearFinished();
    REQUIRE(surface.totalCount() == 1u);
    REQUIRE(surface.find(id3) != nullptr);
}

TEST_CASE("AIActionSurface multiple actions lifecycle", "[Editor][S141]") {
    AIActionSurface surface;
    auto a = surface.propose(AIActionType::ApplyDiff,     "Patch 1", "diff1");
    auto b = surface.propose(AIActionType::InsertSnippet, "Snippet", "code");
    auto c = surface.propose(AIActionType::RunCommand,    "Run",     "cmd");

    REQUIRE(surface.pendingCount() == 3u);
    surface.apply(a);
    surface.reject(b);
    REQUIRE(surface.pendingCount() == 1u);
    REQUIRE(surface.totalApplied()  == 1u);
    REQUIRE(surface.totalRejected() == 1u);
    surface.apply(c);
    REQUIRE(surface.pendingCount() == 0u);
}
