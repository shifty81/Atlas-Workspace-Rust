// Workspace wiring tests: DrawListDispatcher, WorkspaceInputBridge,
// WorkspacePanelHost, and the six new widgets (MenuBar, StatusBar,
// Checkbox, RadioButton, Slider, ProgressBar).
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include "NF/UI/AtlasUI/DrawPrimitives.h"
#include "NF/UI/AtlasUI/Interfaces.h"
#include "NF/UI/AtlasUI/Contexts.h"
#include "NF/UI/AtlasUI/DrawListDispatcher.h"
#include "NF/UI/AtlasUI/WidgetTheme.h"

#include "NF/UI/AtlasUI/Widgets/MenuBar.h"
#include "NF/UI/AtlasUI/Widgets/StatusBar.h"
#include "NF/UI/AtlasUI/Widgets/Checkbox.h"
#include "NF/UI/AtlasUI/Widgets/RadioButton.h"
#include "NF/UI/AtlasUI/Widgets/Slider.h"
#include "NF/UI/AtlasUI/Widgets/ProgressBar.h"

#include "NF/UI/UI.h"
#include "NF/UI/UIBackend.h"

#include "NF/Input/Input.h"

using namespace NF;
using namespace NF::UI::AtlasUI;
using Catch::Approx;

// ── Test helpers ─────────────────────────────────────────────────

struct WireTestPaint : IPaintContext {
    DrawList dl;
    void drawRect(const NF::Rect& r, NF::UI::AtlasUI::Color c)  override { dl.push(DrawRectCmd{r, c}); }
    void fillRect(const NF::Rect& r, NF::UI::AtlasUI::Color c)  override { dl.push(FillRectCmd{r, c}); }
    void drawText(const NF::Rect& r, std::string_view t, FontId f, NF::UI::AtlasUI::Color c) override {
        dl.push(DrawTextCmd{r, std::string(t), f, c});
    }
    void drawImage(const NF::Rect& r, uint32_t textureId, NF::UI::AtlasUI::Color tint) override {
        dl.push(DrawImageCmd{r, textureId, tint});
    }
    void pushClip(const NF::Rect&) override {}
    void popClip() override {}
    DrawList& drawList() override { return dl; }
};

struct WireTestLayout : ILayoutContext {
    float dpiScale() const override { return 1.0f; }
    NF::Vec2 availableSize() const override { return {800.f, 600.f}; }
    NF::Vec2 measureText(std::string_view text, float) const override {
        return {static_cast<float>(text.size()) * 8.f, 16.f};
    }
    void invalidateLayout() override {}
};

struct WireTestInput : IInputContext {
    NF::Vec2 pos{};
    bool primary = false;
    bool secondary = false;
    std::string typed;
    NF::Vec2 mousePosition() const override { return pos; }
    bool primaryDown() const override { return primary; }
    bool secondaryDown() const override { return secondary; }
    bool keyDown(int) const override { return false; }
    std::string_view typedText() const override { return typed; }
    void requestFocus(IWidget*) override {}
    void capturePointer(IWidget*) override {}
    void releasePointer(IWidget*) override {}
};

// ── DrawListDispatcher ────────────────────────────────────────────

// Minimal UIBackend stub that counts drawTextNative calls.
class TextTrackingBackend : public NF::UIBackend {
public:
    bool init(int, int) override { return true; }
    void shutdown() override {}
    void beginFrame(int, int) override {}
    void flush(const NF::UIVertex*, size_t, const uint32_t*, size_t) override {}
    void endFrame() override {}
    [[nodiscard]] const char* backendName() const override { return "Test"; }
    [[nodiscard]] bool isGPUAccelerated() const override { return false; }
    void drawTextNative(float, float, std::string_view text, uint32_t) override {
        lastText = std::string(text);
        ++textCallCount;
    }
    std::string lastText;
    int textCallCount = 0;
};

TEST_CASE("DrawListDispatcher noop without renderer", "[Workspace][Dispatcher]") {
    DrawListDispatcher disp;
    BasicPaintContext ctx;
    ctx.fillRect({0.f, 0.f, 100.f, 100.f}, 0xFF202020u);
    // Must not crash
    disp.dispatch(ctx.drawList());
}

TEST_CASE("DrawListDispatcher FillRectCmd calls UIRenderer::drawRect", "[Workspace][Dispatcher]") {
    NF::UIRenderer renderer;
    renderer.init();
    renderer.beginFrame(800.f, 600.f);

    DrawListDispatcher disp;
    disp.setRenderer(&renderer);

    BasicPaintContext ctx;
    ctx.fillRect({10.f, 20.f, 100.f, 50.f}, Theme::ColorToken::Surface);

    size_t vertsBefore = renderer.vertices().size();
    disp.dispatch(ctx.drawList());
    REQUIRE(renderer.vertices().size() > vertsBefore);

    renderer.endFrame();
    renderer.shutdown();
}

TEST_CASE("DrawListDispatcher DrawRectCmd emits outline quads", "[Workspace][Dispatcher]") {
    NF::UIRenderer renderer;
    renderer.init();
    renderer.beginFrame(800.f, 600.f);

    DrawListDispatcher disp;
    disp.setRenderer(&renderer);

    BasicPaintContext ctx;
    ctx.drawRect({0.f, 0.f, 200.f, 100.f}, Theme::ColorToken::Border);
    disp.dispatch(ctx.drawList());

    renderer.endFrame();
    // drawRectOutline emits 4 quads (top/bottom/left/right bars)
    REQUIRE(renderer.quadCount() == 4u);

    renderer.shutdown();
}

TEST_CASE("DrawListDispatcher DrawTextCmd routes via backend drawTextNative", "[Workspace][Dispatcher]") {
    NF::UIRenderer renderer;
    renderer.init();
    renderer.beginFrame(800.f, 600.f);

    TextTrackingBackend backend;
    backend.init(800, 600);
    renderer.setBackend(&backend);

    DrawListDispatcher disp;
    disp.setRenderer(&renderer);

    BasicPaintContext ctx;
    ctx.drawText({5.f, 10.f, 200.f, 20.f}, "Hello Workspace", 0, Theme::ColorToken::Text);
    disp.dispatch(ctx.drawList());

    REQUIRE(backend.textCallCount == 1);
    REQUIRE(backend.lastText == "Hello Workspace");

    renderer.setBackend(nullptr);
    renderer.endFrame();
    renderer.shutdown();
}

TEST_CASE("DrawListDispatcher color conversion AARRGGBB to RRGGBBAA", "[Workspace][Dispatcher]") {
    // Verify that AtlasUI accent color (AARRGGBB 0xFF3A7AFE) dispatches as blue
    // quad via UIRenderer (RRGGBBAA 0x3A7AFEFF).
    NF::UIRenderer renderer;
    renderer.init();
    renderer.beginFrame(800.f, 600.f);

    DrawListDispatcher disp;
    disp.setRenderer(&renderer);

    BasicPaintContext ctx;
    ctx.fillRect({0.f, 0.f, 10.f, 10.f}, Theme::ColorToken::Accent); // 0xFF3A7AFE
    disp.dispatch(ctx.drawList());

    REQUIRE_FALSE(renderer.vertices().empty());
    uint32_t col = renderer.vertices()[0].color;
    // RRGGBBAA: R=0x3A, G=0x7A, B=0xFE, A=0xFF
    REQUIRE(((col >> 24) & 0xFF) == 0x3A); // R
    REQUIRE(((col >> 16) & 0xFF) == 0x7A); // G
    REQUIRE(((col >>  8) & 0xFF) == 0xFE); // B

    renderer.endFrame();
    renderer.shutdown();
}

TEST_CASE("DrawListDispatcher PushClip PopClip are silent no-ops", "[Workspace][Dispatcher]") {
    NF::UIRenderer renderer;
    renderer.init();
    renderer.beginFrame(800.f, 600.f);

    DrawListDispatcher disp;
    disp.setRenderer(&renderer);

    BasicPaintContext ctx;
    ctx.pushClip({0.f, 0.f, 800.f, 600.f});
    ctx.fillRect({10.f, 10.f, 40.f, 40.f}, 0xFF202020u);
    ctx.popClip();
    // Must not crash and must produce the one fill quad
    disp.dispatch(ctx.drawList());
    renderer.endFrame();
    REQUIRE(renderer.quadCount() == 1u);

    renderer.shutdown();
}

TEST_CASE("DrawListDispatcher mixed command batch", "[Workspace][Dispatcher]") {
    NF::UIRenderer renderer;
    renderer.init();
    renderer.beginFrame(800.f, 600.f);

    TextTrackingBackend backend;
    backend.init(800, 600);
    renderer.setBackend(&backend);

    DrawListDispatcher disp;
    disp.setRenderer(&renderer);

    BasicPaintContext ctx;
    ctx.fillRect({0.f, 0.f, 200.f, 100.f}, Theme::ColorToken::Surface);    // 1 quad
    ctx.drawRect({0.f, 0.f, 200.f, 100.f}, Theme::ColorToken::Border);     // 4 quads
    ctx.drawText({4.f, 4.f, 100.f, 16.f}, "Panel", 0, Theme::ColorToken::Text); // 1 text
    disp.dispatch(ctx.drawList());

    REQUIRE(backend.textCallCount == 1);

    renderer.setBackend(nullptr);
    renderer.endFrame();
    // 1 fill + 4 outline
    REQUIRE(renderer.quadCount() == 5u);
    renderer.shutdown();
}

// ── WorkspaceInputBridge ─────────────────────────────────────────

#include "NF/Workspace/WorkspaceInputBridge.h"

TEST_CASE("WorkspaceInputBridge syncs mouse position", "[Workspace][InputBridge]") {
    NF::InputSystem input;
    input.init();
    input.setMousePosition(250.f, 375.f);

    BasicInputContext ctx;
    NF::WorkspaceInputBridge{}.sync(input, ctx);

    REQUIRE(ctx.mousePosition().x == Approx(250.f));
    REQUIRE(ctx.mousePosition().y == Approx(375.f));
    input.shutdown();
}

TEST_CASE("WorkspaceInputBridge syncs mouse buttons", "[Workspace][InputBridge]") {
    NF::InputSystem input;
    input.init();
    input.setKeyDown(NF::KeyCode::Mouse1);

    BasicInputContext ctx;
    NF::WorkspaceInputBridge{}.sync(input, ctx);

    REQUIRE(ctx.primaryDown());
    REQUIRE_FALSE(ctx.secondaryDown());
    input.shutdown();
}

TEST_CASE("WorkspaceInputBridge syncs right mouse button", "[Workspace][InputBridge]") {
    NF::InputSystem input;
    input.init();
    input.setKeyDown(NF::KeyCode::Mouse2);

    BasicInputContext ctx;
    NF::WorkspaceInputBridge{}.sync(input, ctx);

    REQUIRE_FALSE(ctx.primaryDown());
    REQUIRE(ctx.secondaryDown());
    input.shutdown();
}

TEST_CASE("WorkspaceInputBridge no buttons down", "[Workspace][InputBridge]") {
    NF::InputSystem input;
    input.init();

    BasicInputContext ctx;
    NF::WorkspaceInputBridge{}.sync(input, ctx);

    REQUIRE_FALSE(ctx.primaryDown());
    REQUIRE_FALSE(ctx.secondaryDown());
    input.shutdown();
}

// ── MenuBar ──────────────────────────────────────────────────────

TEST_CASE("MenuBar default state", "[Workspace][MenuBar]") {
    MenuBar bar;
    REQUIRE(bar.categoryCount() == 0u);
    REQUIRE(bar.openCategoryIndex() == -1);
}

TEST_CASE("MenuBar addCategory", "[Workspace][MenuBar]") {
    MenuBar bar;
    auto& file = bar.addCategory("File");
    file.addItem("New", []{});
    file.addItem("Open", []{});
    file.addSeparator();
    file.addItem("Exit", []{});

    REQUIRE(bar.categoryCount() == 1u);
    REQUIRE(bar.categories()[0].label == "File");
    REQUIRE(bar.categories()[0].entryCount() == 4u); // New, Open, sep, Exit
}

TEST_CASE("MenuBar multiple categories", "[Workspace][MenuBar]") {
    MenuBar bar;
    bar.addCategory("File");
    bar.addCategory("Edit");
    bar.addCategory("View");
    REQUIRE(bar.categoryCount() == 3u);
}

TEST_CASE("MenuBar findCategory", "[Workspace][MenuBar]") {
    MenuBar bar;
    bar.addCategory("File");
    bar.addCategory("Edit");

    REQUIRE(bar.findCategory("File") != nullptr);
    REQUIRE(bar.findCategory("Edit") != nullptr);
    REQUIRE(bar.findCategory("Tools") == nullptr);
}

TEST_CASE("MenuBar measure does not crash", "[Workspace][MenuBar]") {
    MenuBar bar;
    bar.addCategory("File");
    bar.addCategory("Edit");

    WireTestLayout layout;
    bar.arrange({0.f, 0.f, 800.f, MenuBar::kBarHeight});
    bar.measure(layout);
    REQUIRE(bar.bounds().h == Approx(MenuBar::kBarHeight));
}

TEST_CASE("MenuBar paint produces draw commands", "[Workspace][MenuBar]") {
    MenuBar bar;
    bar.addCategory("File");
    bar.addCategory("Edit");

    WireTestLayout layout;
    bar.arrange({0.f, 0.f, 800.f, MenuBar::kBarHeight});
    bar.measure(layout);

    WireTestPaint paint;
    bar.paint(paint);
    REQUIRE_FALSE(paint.dl.empty());
}

TEST_CASE("MenuBar closeMenu resets open index", "[Workspace][MenuBar]") {
    MenuBar bar;
    bar.addCategory("File");
    bar.closeMenu();
    REQUIRE(bar.openCategoryIndex() == -1);
}

TEST_CASE("MenuBar hidden produces no draw commands", "[Workspace][MenuBar]") {
    MenuBar bar;
    bar.addCategory("File");
    bar.setVisible(false);
    bar.arrange({0.f, 0.f, 800.f, MenuBar::kBarHeight});

    WireTestPaint paint;
    bar.paint(paint);
    REQUIRE(paint.dl.empty());
}

// ── StatusBar ────────────────────────────────────────────────────

TEST_CASE("StatusBar default state", "[Workspace][StatusBar]") {
    StatusBar bar;
    REQUIRE(bar.sectionCount() == 0u);
}

TEST_CASE("StatusBar setText", "[Workspace][StatusBar]") {
    StatusBar bar;
    bar.setText("Ready");
    REQUIRE(bar.sectionCount() == 1u);
    REQUIRE(bar.sections()[0] == "Ready");

    bar.setText("Loading");
    REQUIRE(bar.sectionCount() == 1u);
    REQUIRE(bar.sections()[0] == "Loading");
}

TEST_CASE("StatusBar addSection", "[Workspace][StatusBar]") {
    StatusBar bar;
    bar.addSection("Editor");
    bar.addSection("Ln 1, Col 1");
    bar.addSection("UTF-8");
    REQUIRE(bar.sectionCount() == 3u);
}

TEST_CASE("StatusBar clearSections", "[Workspace][StatusBar]") {
    StatusBar bar;
    bar.addSection("A");
    bar.addSection("B");
    bar.clearSections();
    REQUIRE(bar.sectionCount() == 0u);
}

TEST_CASE("StatusBar paint produces draw commands", "[Workspace][StatusBar]") {
    StatusBar bar;
    bar.addSection("Editor");
    bar.addSection("60fps");
    bar.arrange({0.f, 576.f, 800.f, StatusBar::kBarHeight});

    WireTestPaint paint;
    bar.paint(paint);
    REQUIRE_FALSE(paint.dl.empty());
}

TEST_CASE("StatusBar hidden produces no draw commands", "[Workspace][StatusBar]") {
    StatusBar bar;
    bar.addSection("Ready");
    bar.setVisible(false);
    bar.arrange({0.f, 576.f, 800.f, StatusBar::kBarHeight});

    WireTestPaint paint;
    bar.paint(paint);
    REQUIRE(paint.dl.empty());
}

TEST_CASE("StatusBar measure sets bar height", "[Workspace][StatusBar]") {
    StatusBar bar;
    WireTestLayout layout;
    bar.arrange({0.f, 0.f, 800.f, StatusBar::kBarHeight});
    bar.measure(layout);
    REQUIRE(bar.bounds().h == Approx(StatusBar::kBarHeight));
}

// ── Checkbox ─────────────────────────────────────────────────────

TEST_CASE("Checkbox default state", "[Workspace][Checkbox]") {
    Checkbox cb;
    REQUIRE_FALSE(cb.isChecked());
    REQUIRE(cb.isVisible());
}

TEST_CASE("Checkbox explicit label and checked state", "[Workspace][Checkbox]") {
    Checkbox cb("Enable VSync", true);
    REQUIRE(cb.isChecked());
}

TEST_CASE("Checkbox setChecked", "[Workspace][Checkbox]") {
    Checkbox cb("Test");
    cb.setChecked(true);
    REQUIRE(cb.isChecked());
    cb.setChecked(false);
    REQUIRE_FALSE(cb.isChecked());
}

TEST_CASE("Checkbox onChange fires on handleInput click", "[Workspace][Checkbox]") {
    Checkbox cb("Click me");
    cb.arrange({0.f, 0.f, 120.f, 24.f});

    bool changed = false;
    bool newVal  = false;
    cb.setOnChange([&](bool v) { changed = true; newVal = v; });

    WireTestInput inp;
    inp.pos = {10.f, 10.f};  // inside bounds
    inp.primary = true;
    REQUIRE(cb.handleInput(inp));
    REQUIRE(changed);
    REQUIRE(newVal == true);  // toggled from false → true
}

TEST_CASE("Checkbox click outside does not fire onChange", "[Workspace][Checkbox]") {
    Checkbox cb("Test");
    cb.arrange({0.f, 0.f, 120.f, 24.f});

    bool changed = false;
    cb.setOnChange([&](bool) { changed = true; });

    WireTestInput inp;
    inp.pos = {200.f, 200.f};  // outside bounds
    inp.primary = true;
    cb.handleInput(inp);
    REQUIRE_FALSE(changed);
}

TEST_CASE("Checkbox paint produces draw commands", "[Workspace][Checkbox]") {
    Checkbox cb("Sample");
    cb.arrange({0.f, 0.f, 120.f, 24.f});

    WireTestPaint paint;
    cb.paint(paint);
    REQUIRE_FALSE(paint.dl.empty());
}

TEST_CASE("Checkbox checked paint has more commands than unchecked", "[Workspace][Checkbox]") {
    Checkbox cb("Test");
    cb.arrange({0.f, 0.f, 120.f, 24.f});

    WireTestPaint uncheckedPaint;
    cb.paint(uncheckedPaint);

    cb.setChecked(true);
    WireTestPaint checkedPaint;
    cb.paint(checkedPaint);

    REQUIRE(checkedPaint.dl.size() > uncheckedPaint.dl.size()); // check mark adds commands
}

TEST_CASE("Checkbox hidden paints nothing", "[Workspace][Checkbox]") {
    Checkbox cb("Hidden");
    cb.setVisible(false);
    cb.arrange({0.f, 0.f, 100.f, 24.f});

    WireTestPaint paint;
    cb.paint(paint);
    REQUIRE(paint.dl.empty());
}

// ── RadioButton ──────────────────────────────────────────────────

TEST_CASE("RadioButton default state", "[Workspace][RadioButton]") {
    RadioButton rb;
    REQUIRE_FALSE(rb.isSelected());
    REQUIRE(rb.value() == 0);
}

TEST_CASE("RadioButton label and value", "[Workspace][RadioButton]") {
    RadioButton rb("Option A", 1);
    REQUIRE(rb.value() == 1);
    REQUIRE_FALSE(rb.isSelected());
}

TEST_CASE("RadioButton setSelected", "[Workspace][RadioButton]") {
    RadioButton rb("B", 2);
    rb.setSelected(true);
    REQUIRE(rb.isSelected());
    rb.setSelected(false);
    REQUIRE_FALSE(rb.isSelected());
}

TEST_CASE("RadioButton onSelect fires on click", "[Workspace][RadioButton]") {
    RadioButton rb("Choice", 42);
    rb.arrange({0.f, 0.f, 120.f, 24.f});

    int selected = -1;
    rb.setOnSelect([&](int v) { selected = v; });

    WireTestInput inp;
    inp.pos = {10.f, 10.f};
    inp.primary = true;
    REQUIRE(rb.handleInput(inp));
    REQUIRE(selected == 42);
    REQUIRE(rb.isSelected());
}

TEST_CASE("RadioButton does not fire onSelect when already selected", "[Workspace][RadioButton]") {
    RadioButton rb("Choice", 1);
    rb.setSelected(true);
    rb.arrange({0.f, 0.f, 120.f, 24.f});

    int calls = 0;
    rb.setOnSelect([&](int) { ++calls; });

    WireTestInput inp;
    inp.pos = {10.f, 10.f};
    inp.primary = true;
    rb.handleInput(inp);
    REQUIRE(calls == 0);  // already selected, no re-fire
}

TEST_CASE("RadioButton paint produces draw commands", "[Workspace][RadioButton]") {
    RadioButton rb("Option");
    rb.arrange({0.f, 0.f, 120.f, 24.f});

    WireTestPaint paint;
    rb.paint(paint);
    REQUIRE_FALSE(paint.dl.empty());
}

TEST_CASE("RadioButton selected state produces inner fill", "[Workspace][RadioButton]") {
    RadioButton rb("Test");
    rb.arrange({0.f, 0.f, 120.f, 24.f});

    WireTestPaint unselPaint;
    rb.paint(unselPaint);

    rb.setSelected(true);
    WireTestPaint selPaint;
    rb.paint(selPaint);

    REQUIRE(selPaint.dl.size() > unselPaint.dl.size());
}

// ── Slider ───────────────────────────────────────────────────────

TEST_CASE("Slider default state", "[Workspace][Slider]") {
    Slider slider;
    REQUIRE(slider.value() == Approx(0.f));
    REQUIRE(slider.minValue() == Approx(0.f));
    REQUIRE(slider.maxValue() == Approx(1.f));
    REQUIRE(slider.fraction() == Approx(0.f));
}

TEST_CASE("Slider custom range and initial value", "[Workspace][Slider]") {
    Slider slider(0.f, 100.f, 50.f);
    REQUIRE(slider.value() == Approx(50.f));
    REQUIRE(slider.fraction() == Approx(0.5f));
}

TEST_CASE("Slider setValue clamped to range", "[Workspace][Slider]") {
    Slider slider(0.f, 10.f);
    slider.setValue(15.f);
    REQUIRE(slider.value() == Approx(10.f));
    slider.setValue(-5.f);
    REQUIRE(slider.value() == Approx(0.f));
}

TEST_CASE("Slider setRange adjusts value", "[Workspace][Slider]") {
    Slider slider(0.f, 100.f, 80.f);
    slider.setRange(0.f, 50.f);
    REQUIRE(slider.value() == Approx(50.f)); // clamped
}

TEST_CASE("Slider onChange fires on drag", "[Workspace][Slider]") {
    Slider slider(0.f, 1.f);
    slider.arrange({0.f, 0.f, 200.f, 32.f});

    float lastVal = -1.f;
    slider.setOnChange([&](float v) { lastVal = v; });

    // Click on track: track is at y≈5, height=4; click mid x ≈ 100, y=7
    WireTestInput inp;
    inp.pos = {100.f, 7.f};
    inp.primary = true;
    slider.handleInput(inp);
    REQUIRE(lastVal >= 0.f);
}

TEST_CASE("Slider fraction is 0 when min == max", "[Workspace][Slider]") {
    Slider slider(5.f, 5.f, 5.f);
    REQUIRE(slider.fraction() == Approx(0.f));
}

TEST_CASE("Slider paint produces draw commands", "[Workspace][Slider]") {
    Slider slider(0.f, 1.f, 0.6f);
    slider.arrange({0.f, 0.f, 200.f, 32.f});

    WireTestPaint paint;
    slider.paint(paint);
    REQUIRE_FALSE(paint.dl.empty());
}

TEST_CASE("Slider paint with label includes label commands", "[Workspace][Slider]") {
    Slider slider(0.f, 100.f, 50.f);
    slider.setLabel("Volume");
    slider.arrange({0.f, 0.f, 200.f, 48.f});

    WireTestPaint paintNoLabel;
    {
        Slider s(0.f, 100.f, 50.f);
        s.arrange({0.f, 0.f, 200.f, 48.f});
        s.paint(paintNoLabel);
    }

    WireTestPaint paintWithLabel;
    slider.paint(paintWithLabel);

    REQUIRE(paintWithLabel.dl.size() > paintNoLabel.dl.size());
}

TEST_CASE("Slider hidden paints nothing", "[Workspace][Slider]") {
    Slider slider;
    slider.setVisible(false);
    slider.arrange({0.f, 0.f, 200.f, 32.f});

    WireTestPaint paint;
    slider.paint(paint);
    REQUIRE(paint.dl.empty());
}

// ── ProgressBar ──────────────────────────────────────────────────

TEST_CASE("ProgressBar default state", "[Workspace][ProgressBar]") {
    ProgressBar bar;
    REQUIRE(bar.value() == Approx(0.f));
    REQUIRE(bar.label().empty());
}

TEST_CASE("ProgressBar explicit initial value", "[Workspace][ProgressBar]") {
    ProgressBar bar(0.75f);
    REQUIRE(bar.value() == Approx(0.75f));
}

TEST_CASE("ProgressBar setValue clamped to 0..1", "[Workspace][ProgressBar]") {
    ProgressBar bar;
    bar.setValue(1.5f);
    REQUIRE(bar.value() == Approx(1.f));
    bar.setValue(-0.3f);
    REQUIRE(bar.value() == Approx(0.f));
}

TEST_CASE("ProgressBar setLabel", "[Workspace][ProgressBar]") {
    ProgressBar bar;
    bar.setLabel("Loading assets...");
    REQUIRE(bar.label() == "Loading assets...");
}

TEST_CASE("ProgressBar paint produces draw commands", "[Workspace][ProgressBar]") {
    ProgressBar bar(0.4f);
    bar.arrange({0.f, 0.f, 300.f, 24.f});

    WireTestPaint paint;
    bar.paint(paint);
    REQUIRE_FALSE(paint.dl.empty());
}

TEST_CASE("ProgressBar at 0 has no fill command", "[Workspace][ProgressBar]") {
    ProgressBar bar(0.f);
    bar.arrange({0.f, 0.f, 300.f, 8.f});

    WireTestPaint fullPaint;
    ProgressBar barFull(1.f);
    barFull.arrange({0.f, 0.f, 300.f, 8.f});
    barFull.paint(fullPaint);

    WireTestPaint zeroPaint;
    bar.paint(zeroPaint);

    REQUIRE(fullPaint.dl.size() > zeroPaint.dl.size());
}

TEST_CASE("ProgressBar with label has more draw commands", "[Workspace][ProgressBar]") {
    ProgressBar barNoLabel(0.5f);
    barNoLabel.arrange({0.f, 0.f, 300.f, 30.f});
    WireTestPaint paintNoLabel;
    barNoLabel.paint(paintNoLabel);

    ProgressBar barWithLabel(0.5f);
    barWithLabel.setLabel("Compiling");
    barWithLabel.arrange({0.f, 0.f, 300.f, 30.f});
    WireTestPaint paintWithLabel;
    barWithLabel.paint(paintWithLabel);

    REQUIRE(paintWithLabel.dl.size() > paintNoLabel.dl.size());
}

TEST_CASE("ProgressBar hidden paints nothing", "[Workspace][ProgressBar]") {
    ProgressBar bar(0.9f);
    bar.setVisible(false);
    bar.arrange({0.f, 0.f, 300.f, 24.f});

    WireTestPaint paint;
    bar.paint(paint);
    REQUIRE(paint.dl.empty());
}

// ── BasicInputContext direct tests ───────────────────────────────

TEST_CASE("BasicInputContext setters work correctly", "[Workspace][InputContext]") {
    BasicInputContext ctx;
    ctx.setMousePosition({123.f, 456.f});
    ctx.setPrimaryDown(true);
    ctx.setSecondaryDown(false);
    ctx.setKeyDown(65, true);  // 'A'

    REQUIRE(ctx.mousePosition().x == Approx(123.f));
    REQUIRE(ctx.mousePosition().y == Approx(456.f));
    REQUIRE(ctx.primaryDown());
    REQUIRE_FALSE(ctx.secondaryDown());
    REQUIRE(ctx.keyDown(65));
    REQUIRE_FALSE(ctx.keyDown(66));

    ctx.setKeyDown(65, false);
    REQUIRE_FALSE(ctx.keyDown(65));
}

TEST_CASE("BasicPaintContext captures all command types", "[Workspace][PaintContext]") {
    BasicPaintContext ctx;
    ctx.fillRect({0.f, 0.f, 10.f, 10.f}, 0xFF0000FFu);
    ctx.drawRect({0.f, 0.f, 10.f, 10.f}, 0x00FF00FFu);
    ctx.drawText({0.f, 0.f, 100.f, 20.f}, "test", 0, 0xFFFFFFFFu);
    ctx.pushClip({0.f, 0.f, 800.f, 600.f});
    ctx.popClip();

    REQUIRE(ctx.drawList().size() == 5u);
}

// ── WorkspaceRenderer interaction wiring ─────────────────────────
// Tests that validate mouse input → UIContext hit regions → launch service
// path introduced for Option B clickable workspace chrome.

#include "NF/UI/UIWidgets.h"
#include "NF/Workspace/WorkspaceRenderer.h"
#include "NF/Workspace/WorkspaceShell.h"
#include "NF/Workspace/WorkspaceLaunchContract.h"
#include "NF/Workspace/WorkspaceBootstrap.h"

TEST_CASE("WorkspaceRenderer renders with no mouse input without crashing", "[Workspace][Renderer]") {
    NF::UIRenderer r;
    r.init();

    NF::NullBackend nb;
    nb.init(1280, 800);
    r.setBackend(&nb);

    NF::WorkspaceShell shell;
    NF::WorkspaceBootstrap bootstrap;
    NF::WorkspaceBootstrapConfig cfg;
    cfg.launchMode = NF::WorkspaceStartupMode::Hosted;
    auto result = bootstrap.run(cfg, shell);
    REQUIRE_FALSE(result.failed());

    NF::WorkspaceRenderer renderer;
    // No mouse, no launch service — renders visuals only
    NF::UIMouseState noMouse{};
    REQUIRE_NOTHROW(renderer.render(r, 1280.f, 800.f, shell, noMouse, nullptr));

    r.shutdown();
    shell.shutdown();
}

TEST_CASE("WorkspaceRenderer renders with UIMouseState and NullLaunchService", "[Workspace][Renderer]") {
    NF::UIRenderer r;
    r.init();

    NF::NullBackend nb;
    nb.init(1280, 800);
    r.setBackend(&nb);

    NF::WorkspaceShell shell;
    NF::WorkspaceBootstrap bootstrap;
    NF::WorkspaceBootstrapConfig cfg;
    cfg.launchMode = NF::WorkspaceStartupMode::Hosted;
    (void)bootstrap.run(cfg, shell);

    // Register a test app
    NF::WorkspaceAppDescriptor desc;
    desc.id              = NF::WorkspaceAppId::TileEditor;
    desc.name            = "Test App";
    desc.executablePath  = "TestApp.exe";
    desc.isProjectScoped = false;
    desc.allowDirectLaunch = true;
    shell.appRegistry().registerApp(desc);

    NF::NullLaunchService launchSvc;
    NF::UIMouseState mouse{};
    mouse.x = 100.f;
    mouse.y = 100.f;

    NF::WorkspaceRenderer renderer;
    // Should not crash with mouse state and launch service provided
    REQUIRE_NOTHROW(renderer.render(r, 1280.f, 800.f, shell, mouse, &launchSvc));

    r.shutdown();
    shell.shutdown();
}

TEST_CASE("NullLaunchService records sidebar app launch via WorkspaceRenderer click", "[Workspace][Renderer][Interaction]") {
    NF::UIRenderer r;
    r.init();

    NF::NullBackend nb;
    nb.init(1280, 800);
    r.setBackend(&nb);

    NF::WorkspaceShell shell;
    NF::WorkspaceBootstrap bootstrap;
    NF::WorkspaceBootstrapConfig cfg;
    cfg.launchMode = NF::WorkspaceStartupMode::Hosted;
    (void)bootstrap.run(cfg, shell);

    // Register a non-project-scoped app (simpler launch path)
    NF::WorkspaceAppDescriptor desc;
    desc.id              = NF::WorkspaceAppId::TileEditor;
    desc.name            = "Tile Editor";
    desc.executablePath  = "TileEditor.exe";
    desc.isProjectScoped = false;
    desc.allowDirectLaunch = true;
    shell.appRegistry().registerApp(desc);

    NF::NullLaunchService launchSvc;

    // Simulate a click on the first sidebar card.
    // Sidebar cards start at y = kContentY + 30 = 28 + 30 = 58
    // x range: 4 to 4 + (224 - 8) = 220, so center at x=112, y=58+19=77
    NF::UIMouseState mouse{};
    mouse.x            = 112.f;   // inside first sidebar card
    mouse.y            = 77.f;    // y = kContentY(28) + 30 + 19 = 77
    mouse.leftReleased = true;    // simulate a click release

    NF::WorkspaceRenderer renderer;
    renderer.render(r, 1280.f, 800.f, shell, mouse, &launchSvc);

    // The NullLaunchService should have recorded the launch
    REQUIRE(launchSvc.isRunning(NF::WorkspaceAppId::TileEditor));

    r.shutdown();
    shell.shutdown();
}
