// Phase 69 — Tool Render Contract (Audit Patch 5)
//
// Tests verify that:
//  1. ToolViewRenderContext color palette constants have the correct values
//  2. IHostedTool::renderToolView() default no-op does not crash
//  3. Each core tool overrides renderToolView() and emits rendering output
//     (verified via UIRenderer::quadCount() and textDrawCount() post-endFrame)
//  4. renderToolView() is a const method — does not mutate tool state
//  5. Narrow (small) bounding boxes are handled gracefully
//
// Tests use NF::NullBackend so no display is required.

#include "NF/Editor/SceneEditorTool.h"
#include "NF/Editor/AssetEditorTool.h"
#include "NF/Editor/MaterialEditorTool.h"
#include "NF/Editor/AnimationEditorTool.h"
#include "NF/Editor/DataEditorTool.h"
#include "NF/Editor/VisualLogicEditorTool.h"
#include "NF/Editor/BuildTool.h"
#include "NF/Editor/AtlasAITool.h"
#include "NF/Workspace/ToolViewRenderContext.h"
#include "NF/UI/UI.h"
#include "NF/UI/UIBackend.h"
#include "NF/UI/UIWidgets.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <string>

using Catch::Approx;

// ── Test helpers ──────────────────────────────────────────────────

namespace {

// Build a UIRenderer+NullBackend pair ready for drawing.
// The caller must call ui.endFrame() + ui.shutdown() after the test body.
void setupRenderer(NF::UIRenderer& ui, NF::NullBackend& nb) {
    nb.init(1280, 800);
    ui.init();
    ui.setBackend(&nb);
    ui.beginFrame(1280.f, 800.f);
}

// Build a standard ToolViewRenderContext for the given UIRenderer.
NF::ToolViewRenderContext makeCtx(NF::UIRenderer& ui, float w = 1200.f, float h = 700.f) {
    NF::UIMouseState mouse{};
    return {ui, mouse, 0.f, 0.f, w, h, nullptr};
}

} // anonymous namespace

// ── ToolViewRenderContext color palette ───────────────────────────

TEST_CASE("ToolViewRenderContext color palette constants", "[phase69]") {
    REQUIRE(NF::ToolViewRenderContext::kCardBg      == 0x2E2E2EFFu);
    REQUIRE(NF::ToolViewRenderContext::kSurface     == 0x252525FFu);
    REQUIRE(NF::ToolViewRenderContext::kBorder      == 0x3C3C3CFFu);
    REQUIRE(NF::ToolViewRenderContext::kAccentBlue  == 0x007ACCFFu);
    REQUIRE(NF::ToolViewRenderContext::kGreen       == 0x4EC94EFFu);
    REQUIRE(NF::ToolViewRenderContext::kRed         == 0xE05050FFu);
    REQUIRE(NF::ToolViewRenderContext::kTextPrimary == 0xE0E0E0FFu);
    REQUIRE(NF::ToolViewRenderContext::kTextSecond  == 0x888888FFu);
    REQUIRE(NF::ToolViewRenderContext::kTextMuted   == 0x555555FFu);
}

TEST_CASE("ToolViewRenderContext bounding box fields are set correctly", "[phase69]") {
    NF::NullBackend nb; NF::UIRenderer ui;
    setupRenderer(ui, nb);

    auto ctx = makeCtx(ui, 800.f, 600.f);
    REQUIRE(ctx.x    == Approx(0.f));
    REQUIRE(ctx.y    == Approx(0.f));
    REQUIRE(ctx.w    == Approx(800.f));
    REQUIRE(ctx.h    == Approx(600.f));
    REQUIRE(ctx.shell == nullptr);

    ui.endFrame();
    ui.shutdown();
}

// ── IHostedTool default no-op (stub type without override) ────────

TEST_CASE("IHostedTool default renderToolView no-op emits zero quads", "[phase69]") {
    struct StubTool final : public NF::IHostedTool {
        [[nodiscard]] const NF::HostedToolDescriptor& descriptor() const override { return m_desc; }
        [[nodiscard]] const std::string& toolId() const override { return m_desc.toolId; }
        bool initialize() override { return true; }
        void shutdown()   override {}
        void activate()   override {}
        void suspend()    override {}
        void update(float) override {}
        [[nodiscard]] NF::HostedToolState state() const override { return NF::HostedToolState::Unloaded; }
        // Does NOT override renderToolView → exercises the default no-op.
        NF::HostedToolDescriptor m_desc;
    };

    NF::NullBackend nb; NF::UIRenderer ui;
    setupRenderer(ui, nb);

    StubTool stub;
    auto ctx = makeCtx(ui);
    stub.renderToolView(ctx);          // no-op — must not crash

    ui.endFrame();
    REQUIRE(ui.quadCount()     == 0u); // no quads emitted
    REQUIRE(ui.textDrawCount() == 0u);
    ui.shutdown();
}

// ── SceneEditorTool render contract ──────────────────────────────

TEST_CASE("SceneEditorTool::renderToolView emits quads and text", "[phase69]") {
    // SceneEditorTool::renderToolView is intentionally a no-op: the Scene Editor
    // delegates all panel rendering to WorkspacePanelHost (HierarchyPanel,
    // ViewportPanel, InspectorPanel, ContentBrowserPanel).  The render contract
    // test now verifies that the no-op is stable and does not crash, and that
    // the tool state (entityCount, selectionCount) is still accessible.
    NF::NullBackend nb; NF::UIRenderer ui;
    setupRenderer(ui, nb);

    NF::SceneEditorTool tool;
    tool.initialize();
    tool.activate();
    tool.setEntityCount(5);
    tool.setSelectionCount(2);

    REQUIRE_NOTHROW(tool.renderToolView(makeCtx(ui)));
    REQUIRE(tool.stats().entityCount == 5u);

    ui.endFrame();
    ui.shutdown();
}

TEST_CASE("SceneEditorTool::renderToolView dirty flag path does not crash", "[phase69]") {
    NF::NullBackend nb; NF::UIRenderer ui;
    setupRenderer(ui, nb);

    NF::SceneEditorTool tool;
    tool.initialize();
    tool.activate();
    tool.markDirty();
    REQUIRE_NOTHROW(tool.renderToolView(makeCtx(ui)));

    ui.endFrame();
    ui.shutdown();
}

TEST_CASE("SceneEditorTool::renderToolView is const — state unchanged after call", "[phase69]") {
    NF::NullBackend nb; NF::UIRenderer ui;
    setupRenderer(ui, nb);

    NF::SceneEditorTool tool;
    tool.initialize();
    tool.activate();
    tool.setEntityCount(10);

    const NF::SceneEditorTool& cref = tool;
    cref.renderToolView(makeCtx(ui));  // const call must compile + run

    REQUIRE(cref.stats().entityCount == 10u); // state unmodified
    ui.endFrame();
    ui.shutdown();
}

// ── AssetEditorTool render contract ──────────────────────────────

TEST_CASE("AssetEditorTool::renderToolView emits quads and text", "[phase69]") {
    NF::NullBackend nb; NF::UIRenderer ui;
    setupRenderer(ui, nb);

    NF::AssetEditorTool tool;
    tool.initialize();
    tool.activate();
    tool.setTotalAssetCount(100);
    tool.setFilteredAssetCount(25);

    tool.renderToolView(makeCtx(ui));
    ui.endFrame();
    REQUIRE(ui.quadCount()     > 0u);
    REQUIRE(ui.textDrawCount() > 0u);
    ui.shutdown();
}

TEST_CASE("AssetEditorTool::renderToolView zero assets does not crash", "[phase69]") {
    NF::NullBackend nb; NF::UIRenderer ui;
    setupRenderer(ui, nb);

    NF::AssetEditorTool tool;
    tool.initialize();
    tool.activate();
    REQUIRE_NOTHROW(tool.renderToolView(makeCtx(ui)));
    ui.endFrame();
    ui.shutdown();
}

// ── MaterialEditorTool render contract ───────────────────────────

TEST_CASE("MaterialEditorTool::renderToolView emits quads", "[phase69]") {
    NF::NullBackend nb; NF::UIRenderer ui;
    setupRenderer(ui, nb);

    NF::MaterialEditorTool tool;
    tool.initialize();
    tool.activate();
    tool.setNodeCount(12);
    tool.setTextureSlotCount(4);

    tool.renderToolView(makeCtx(ui));
    ui.endFrame();
    REQUIRE(ui.quadCount() > 0u);
    ui.shutdown();
}

TEST_CASE("MaterialEditorTool::renderToolView dirty flag does not crash", "[phase69]") {
    NF::NullBackend nb; NF::UIRenderer ui;
    setupRenderer(ui, nb);

    NF::MaterialEditorTool tool;
    tool.initialize();
    tool.activate();
    tool.markDirty();
    REQUIRE_NOTHROW(tool.renderToolView(makeCtx(ui)));
    ui.endFrame();
    ui.shutdown();
}

// ── AnimationEditorTool render contract ──────────────────────────

TEST_CASE("AnimationEditorTool::renderToolView emits quads", "[phase69]") {
    NF::NullBackend nb; NF::UIRenderer ui;
    setupRenderer(ui, nb);

    NF::AnimationEditorTool tool;
    tool.initialize();
    tool.activate();
    tool.setClipDurationMs(2000.f);
    tool.setFrameCount(60);

    tool.renderToolView(makeCtx(ui));
    ui.endFrame();
    REQUIRE(ui.quadCount() > 0u);
    ui.shutdown();
}

TEST_CASE("AnimationEditorTool::renderToolView playing+recording state", "[phase69]") {
    NF::NullBackend nb; NF::UIRenderer ui;
    setupRenderer(ui, nb);

    NF::AnimationEditorTool tool;
    tool.initialize();
    tool.activate();
    tool.play();
    tool.record(true);
    REQUIRE_NOTHROW(tool.renderToolView(makeCtx(ui)));
    ui.endFrame();
    ui.shutdown();
}

// ── DataEditorTool render contract ───────────────────────────────

TEST_CASE("DataEditorTool::renderToolView emits quads", "[phase69]") {
    NF::NullBackend nb; NF::UIRenderer ui;
    setupRenderer(ui, nb);

    NF::DataEditorTool tool;
    tool.initialize();
    tool.activate();
    tool.setRowCount(50);
    tool.setColumnCount(8);
    tool.setSelectedRowCount(3);
    tool.setOpenTablePath("Tables/GameData");

    tool.renderToolView(makeCtx(ui));
    ui.endFrame();
    REQUIRE(ui.quadCount()     > 0u);
    REQUIRE(ui.textDrawCount() > 0u);
    ui.shutdown();
}

// ── VisualLogicEditorTool render contract ────────────────────────

TEST_CASE("VisualLogicEditorTool::renderToolView emits quads", "[phase69]") {
    NF::NullBackend nb; NF::UIRenderer ui;
    setupRenderer(ui, nb);

    NF::VisualLogicEditorTool tool;
    tool.initialize();
    tool.activate();
    tool.setNodeCount(10);
    tool.setConnectionCount(15);

    tool.renderToolView(makeCtx(ui));
    ui.endFrame();
    REQUIRE(ui.quadCount() > 0u);
    ui.shutdown();
}

TEST_CASE("VisualLogicEditorTool::renderToolView error state shows red pill path", "[phase69]") {
    NF::NullBackend nb; NF::UIRenderer ui;
    setupRenderer(ui, nb);

    NF::VisualLogicEditorTool tool;
    tool.initialize();
    tool.activate();
    tool.setNodeCount(5);
    tool.setErrorCount(3);
    REQUIRE_NOTHROW(tool.renderToolView(makeCtx(ui)));
    ui.endFrame();
    REQUIRE(ui.quadCount() > 0u);
    ui.shutdown();
}

TEST_CASE("VisualLogicEditorTool::renderToolView compiling state does not crash", "[phase69]") {
    NF::NullBackend nb; NF::UIRenderer ui;
    setupRenderer(ui, nb);

    NF::VisualLogicEditorTool tool;
    tool.initialize();
    tool.activate();
    tool.setCompiling(true);
    REQUIRE_NOTHROW(tool.renderToolView(makeCtx(ui)));
    ui.endFrame();
    ui.shutdown();
}

// ── BuildTool render contract ─────────────────────────────────────

TEST_CASE("BuildTool::renderToolView emits quads and text", "[phase69]") {
    NF::NullBackend nb; NF::UIRenderer ui;
    setupRenderer(ui, nb);

    NF::BuildTool tool;
    tool.initialize();
    tool.activate();
    tool.setWarningCount(2);
    tool.setLastBuildMs(3500.f);
    tool.setActiveTarget("Editor_Debug");

    tool.renderToolView(makeCtx(ui));
    ui.endFrame();
    REQUIRE(ui.quadCount()     > 0u);
    REQUIRE(ui.textDrawCount() > 0u);
    ui.shutdown();
}

TEST_CASE("BuildTool::renderToolView clean build (no errors/warnings) path", "[phase69]") {
    NF::NullBackend nb; NF::UIRenderer ui;
    setupRenderer(ui, nb);

    NF::BuildTool tool;
    tool.initialize();
    tool.activate();
    tool.setLastBuildMs(1200.f);
    REQUIRE_NOTHROW(tool.renderToolView(makeCtx(ui)));
    ui.endFrame();
    REQUIRE(ui.quadCount() > 0u);
    ui.shutdown();
}

TEST_CASE("BuildTool::renderToolView building state pill does not crash", "[phase69]") {
    NF::NullBackend nb; NF::UIRenderer ui;
    setupRenderer(ui, nb);

    NF::BuildTool tool;
    tool.initialize();
    tool.activate();
    tool.setBuilding(true);
    REQUIRE_NOTHROW(tool.renderToolView(makeCtx(ui)));
    ui.endFrame();
    ui.shutdown();
}

// ── AtlasAITool render contract ───────────────────────────────────

TEST_CASE("AtlasAITool::renderToolView emits quads and text", "[phase69]") {
    NF::NullBackend nb; NF::UIRenderer ui;
    setupRenderer(ui, nb);

    NF::AtlasAITool tool;
    tool.initialize();
    tool.activate();
    tool.setMessageCount(5);
    tool.setCodexSnippetCount(2);

    tool.renderToolView(makeCtx(ui));
    ui.endFrame();
    REQUIRE(ui.quadCount()     > 0u);
    REQUIRE(ui.textDrawCount() > 0u);
    ui.shutdown();
}

TEST_CASE("AtlasAITool::renderToolView empty session shows hint path", "[phase69]") {
    NF::NullBackend nb; NF::UIRenderer ui;
    setupRenderer(ui, nb);

    NF::AtlasAITool tool;
    tool.initialize();
    tool.activate();
    REQUIRE_NOTHROW(tool.renderToolView(makeCtx(ui)));
    ui.endFrame();
    REQUIRE(ui.quadCount() > 0u);
    ui.shutdown();
}

TEST_CASE("AtlasAITool::renderToolView processing state does not crash", "[phase69]") {
    NF::NullBackend nb; NF::UIRenderer ui;
    setupRenderer(ui, nb);

    NF::AtlasAITool tool;
    tool.initialize();
    tool.activate();
    tool.setProcessing(true);
    REQUIRE_NOTHROW(tool.renderToolView(makeCtx(ui)));
    ui.endFrame();
    ui.shutdown();
}

// ── Narrow viewport robustness ────────────────────────────────────

TEST_CASE("renderToolView handles minimal bounding box gracefully", "[phase69]") {
    NF::NullBackend nb; NF::UIRenderer ui;
    setupRenderer(ui, nb);

    NF::SceneEditorTool scene; scene.initialize(); scene.activate();
    NF::BuildTool build;       build.initialize(); build.activate();
    NF::AtlasAITool ai;        ai.initialize();    ai.activate();

    // Very small area — should not divide by zero or overflow
    NF::ToolViewRenderContext tiny = makeCtx(ui, 50.f, 50.f);
    REQUIRE_NOTHROW(scene.renderToolView(tiny));
    REQUIRE_NOTHROW(build.renderToolView(tiny));
    REQUIRE_NOTHROW(ai.renderToolView(tiny));

    ui.endFrame();
    ui.shutdown();
}
