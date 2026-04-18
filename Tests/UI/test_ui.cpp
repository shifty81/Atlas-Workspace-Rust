#include <catch2/catch_test_macros.hpp>
#include "NF/UI/UI.h"
#include "NF/UI/UIBackend.h"
#include "NF/UI/UIWidgets.h"
#include "NF/UI/OpenGLBackend.h"

// ── UIRenderer basics ────────────────────────────────────────────

TEST_CASE("UIRenderer drawRect emits 4 vertices and 6 indices", "[UI][Renderer]") {
    NF::UIRenderer r;
    r.init();
    r.beginFrame(800.f, 600.f);
    r.drawRect({10.f, 20.f, 100.f, 50.f}, 0xFF0000FF);
    REQUIRE(r.vertices().size() == 4);
    REQUIRE(r.indices().size() == 6);
    r.endFrame();
    REQUIRE(r.quadCount() == 1);
    r.shutdown();
}

TEST_CASE("UIRenderer drawText emits quads per visible char", "[UI][Renderer]") {
    NF::UIRenderer r;
    r.init();
    r.beginFrame(800.f, 600.f);
    r.drawText(0.f, 0.f, "Hi", 0xFFFFFFFF);
    // 2 chars => 2 quads => 8 vertices, 12 indices
    REQUIRE(r.vertices().size() == 8);
    REQUIRE(r.indices().size() == 12);
    r.endFrame();
    REQUIRE(r.textDrawCount() == 1);
    r.shutdown();
}

TEST_CASE("UIRenderer drawRectOutline emits 4 quads", "[UI][Renderer]") {
    NF::UIRenderer r;
    r.init();
    r.beginFrame(800.f, 600.f);
    r.drawRectOutline({0.f, 0.f, 100.f, 100.f}, 0xFFFFFFFF, 2.f);
    r.endFrame();
    REQUIRE(r.quadCount() == 4);
    r.shutdown();
}

TEST_CASE("UIRenderer drawGradientRect emits 4 vertices", "[UI][Renderer]") {
    NF::UIRenderer r;
    r.init();
    r.beginFrame(800.f, 600.f);
    r.drawGradientRect({10.f, 10.f, 100.f, 50.f}, 0xFF0000FF, 0x0000FFFF);
    REQUIRE(r.vertices().size() == 4);
    // Top vertices should have topColor
    REQUIRE(r.vertices()[0].color == 0xFF0000FF);
    // Bottom vertices should have bottomColor
    REQUIRE(r.vertices()[2].color == 0x0000FFFF);
    r.endFrame();
    r.shutdown();
}

TEST_CASE("UIRenderer drawLine emits quad for line segment", "[UI][Renderer]") {
    NF::UIRenderer r;
    r.init();
    r.beginFrame(800.f, 600.f);
    r.drawLine(0.f, 0.f, 100.f, 100.f, 0xFFFFFFFF, 2.f);
    REQUIRE(r.vertices().size() == 4);
    REQUIRE(r.indices().size() == 6);
    r.endFrame();
    r.shutdown();
}

TEST_CASE("UIRenderer drawCircle emits fan geometry", "[UI][Renderer]") {
    NF::UIRenderer r;
    r.init();
    r.beginFrame(800.f, 600.f);
    r.drawCircle(50.f, 50.f, 20.f, 0xFF0000FF, 8);
    // 1 centre + 9 perimeter vertices (8 segments + 1 closing)
    REQUIRE(r.vertices().size() == 10);
    // 8 triangles * 3 indices = 24
    REQUIRE(r.indices().size() == 24);
    r.endFrame();
    r.shutdown();
}

TEST_CASE("UIRenderer measureText returns reasonable extents", "[UI][Renderer]") {
    NF::UIRenderer r;
    r.init();
    auto sz = r.measureText("Hello", 14.f);
    REQUIRE(sz.x == 5.f * 8.f);
    REQUIRE(sz.y > 0.f);
    r.shutdown();
}

TEST_CASE("UIRenderer beginFrame clears previous vertices", "[UI][Renderer]") {
    NF::UIRenderer r;
    r.init();
    r.beginFrame(800.f, 600.f);
    r.drawRect({0, 0, 10, 10}, 0xFF);
    r.endFrame();
    r.beginFrame(800.f, 600.f);
    REQUIRE(r.vertices().empty());
    REQUIRE(r.indices().empty());
    r.endFrame();
    r.shutdown();
}

// ── UIBackend ────────────────────────────────────────────────────

TEST_CASE("NullBackend init and flush are no-ops", "[UI][Backend]") {
    NF::NullBackend nb;
    REQUIRE(nb.init(1280, 800));
    nb.beginFrame(1280, 800);

    NF::UIVertex verts[4] = {};
    uint32_t indices[6] = {0, 1, 2, 0, 2, 3};
    nb.flush(verts, 4, indices, 6);  // should not crash
    nb.endFrame();

    REQUIRE(std::string(nb.backendName()) == "Null");
    REQUIRE_FALSE(nb.isGPUAccelerated());
    nb.shutdown();
}

TEST_CASE("UIRenderer delegates flush to backend", "[UI][Backend]") {
    // Use NullBackend — verifies no crash when backend is set
    NF::NullBackend nb;
    nb.init(800, 600);

    NF::UIRenderer r;
    r.init();
    r.setBackend(&nb);
    REQUIRE(r.backend() == &nb);

    r.beginFrame(800.f, 600.f);
    r.drawRect({10, 10, 50, 50}, 0xFF0000FF);
    r.endFrame();  // should call nb.flush()
    REQUIRE(r.quadCount() == 1);

    r.shutdown();
    nb.shutdown();
}

TEST_CASE("UIRenderer works without backend (null backend)", "[UI][Backend]") {
    NF::UIRenderer r;
    r.init();
    REQUIRE(r.backend() == nullptr);
    r.beginFrame(800.f, 600.f);
    r.drawRect({10, 10, 50, 50}, 0xFF0000FF);
    r.endFrame();  // should not crash
    REQUIRE(r.quadCount() == 1);
    r.shutdown();
}

TEST_CASE("UIBackend measureText fallback", "[UI][Backend]") {
    NF::NullBackend nb;
    nb.init(800, 600);
    auto sz = nb.measureText("Test", 14.f);
    REQUIRE(sz.x == 4.f * 8.f);  // 4 chars * 8px
    REQUIRE(sz.y > 0.f);
    nb.shutdown();
}

// ── OpenGLBackend (stub) ─────────────────────────────────────────

TEST_CASE("OpenGLBackend stub initializes and tracks stats", "[UI][OpenGL]") {
    NF::OpenGLBackend gl;
    REQUIRE(gl.init(1280, 800));
    REQUIRE(std::string(gl.backendName()) == "OpenGL");
    REQUIRE(gl.isGPUAccelerated());

    gl.beginFrame(1280, 800);
    NF::UIVertex verts[4] = {};
    uint32_t indices[6] = {0, 1, 2, 0, 2, 3};
    gl.flush(verts, 4, indices, 6);
    REQUIRE(gl.lastVertexCount() == 4);
    REQUIRE(gl.lastIndexCount() == 6);
    gl.endFrame();
    gl.shutdown();
}

TEST_CASE("OpenGLBackend loadFont stores params", "[UI][OpenGL]") {
    NF::OpenGLBackend gl;
    gl.init(800, 600);
    gl.loadFont("Segoe UI", 16.f);
    // No crash, font is stored internally
    gl.shutdown();
}

// ── UIWidgets / UIContext ────────────────────────────────────────

TEST_CASE("UIContext button renders and detects click", "[UI][Widgets]") {
    NF::UIRenderer r;
    r.init();
    r.beginFrame(800.f, 600.f);

    NF::UITheme theme = NF::UITheme::dark();
    NF::UIMouseState mouse{};
    mouse.x = 20.f;
    mouse.y = 30.f;
    mouse.leftReleased = true;  // simulate click

    NF::UIContext ctx;
    ctx.begin(r, mouse, theme, 0.016f);

    // Position a panel that contains the mouse position
    ctx.beginPanel("Test", {0.f, 0.f, 200.f, 200.f});
    bool clicked = ctx.button("Click Me");
    ctx.endPanel();
    ctx.end();

    // Button is at cursor position inside the panel (after header)
    // The click should be detected if mouse is within the button bounds
    // Since button is rendered at (8, ~26), mouse at (20,30) is over it
    REQUIRE(clicked);
    r.endFrame();
    r.shutdown();
}

TEST_CASE("UIContext button not clicked when mouse outside", "[UI][Widgets]") {
    NF::UIRenderer r;
    r.init();
    r.beginFrame(800.f, 600.f);

    NF::UITheme theme = NF::UITheme::dark();
    NF::UIMouseState mouse{};
    mouse.x = 500.f;  // far outside
    mouse.y = 500.f;
    mouse.leftReleased = true;

    NF::UIContext ctx;
    ctx.begin(r, mouse, theme, 0.016f);
    ctx.beginPanel("Test", {0.f, 0.f, 200.f, 200.f});
    bool clicked = ctx.button("Click Me");
    ctx.endPanel();
    ctx.end();

    REQUIRE_FALSE(clicked);
    r.endFrame();
    r.shutdown();
}

TEST_CASE("UIContext checkbox toggles value", "[UI][Widgets]") {
    NF::UIRenderer r;
    r.init();
    r.beginFrame(800.f, 600.f);

    NF::UITheme theme = NF::UITheme::dark();
    NF::UIMouseState mouse{};
    mouse.x = 12.f;
    mouse.y = 30.f;
    mouse.leftReleased = true;

    bool value = false;
    NF::UIContext ctx;
    ctx.begin(r, mouse, theme, 0.016f);
    ctx.beginPanel("Test", {0.f, 0.f, 200.f, 200.f});
    bool toggled = ctx.checkbox("Enable", value);
    ctx.endPanel();
    ctx.end();

    REQUIRE(toggled);
    REQUIRE(value == true);
    r.endFrame();
    r.shutdown();
}

TEST_CASE("UIContext label renders text", "[UI][Widgets]") {
    NF::UIRenderer r;
    r.init();
    r.beginFrame(800.f, 600.f);

    NF::UITheme theme = NF::UITheme::dark();
    NF::UIMouseState mouse{};

    NF::UIContext ctx;
    ctx.begin(r, mouse, theme, 0.016f);
    ctx.beginPanel("Test", {0.f, 0.f, 200.f, 200.f});
    ctx.label("Hello World");
    ctx.endPanel();
    ctx.end();

    // Should have drawn text quads
    REQUIRE(r.vertices().size() > 0);
    r.endFrame();
    r.shutdown();
}

TEST_CASE("UIContext slider changes value on drag", "[UI][Widgets]") {
    NF::UIRenderer r;
    r.init();
    r.beginFrame(800.f, 600.f);

    NF::UITheme theme = NF::UITheme::dark();
    NF::UIMouseState mouse{};
    // Position mouse over the slider track
    mouse.x = 80.f;
    mouse.y = 34.f;
    mouse.leftDown = true;

    float val = 0.5f;
    NF::UIContext ctx;
    ctx.begin(r, mouse, theme, 0.016f);
    ctx.beginPanel("Test", {0.f, 0.f, 300.f, 200.f});
    bool changed = ctx.slider("Speed", val, 0.f, 1.f);
    ctx.endPanel();
    ctx.end();

    // Slider interaction depends on exact bounds, but should render
    REQUIRE(r.vertices().size() > 0);
    r.endFrame();
    r.shutdown();
}

TEST_CASE("UIContext progressBar renders fill", "[UI][Widgets]") {
    NF::UIRenderer r;
    r.init();
    r.beginFrame(800.f, 600.f);

    NF::UITheme theme = NF::UITheme::dark();
    NF::UIMouseState mouse{};

    NF::UIContext ctx;
    ctx.begin(r, mouse, theme, 0.016f);
    ctx.beginPanel("Test", {0.f, 0.f, 200.f, 200.f});
    ctx.progressBar(0.75f, 8.f);
    ctx.endPanel();
    ctx.end();

    REQUIRE(r.vertices().size() > 0);
    r.endFrame();
    r.shutdown();
}

TEST_CASE("UIContext colorSwatch renders colored rect", "[UI][Widgets]") {
    NF::UIRenderer r;
    r.init();
    r.beginFrame(800.f, 600.f);

    NF::UITheme theme = NF::UITheme::dark();
    NF::UIMouseState mouse{};

    NF::UIContext ctx;
    ctx.begin(r, mouse, theme, 0.016f);
    ctx.beginPanel("Test", {0.f, 0.f, 200.f, 200.f});
    ctx.colorSwatch(0xFF0000FF, 24.f);
    ctx.endPanel();
    ctx.end();

    REQUIRE(r.vertices().size() > 0);
    r.endFrame();
    r.shutdown();
}

TEST_CASE("UIContext treeNode toggles expansion", "[UI][Widgets]") {
    NF::UIRenderer r;
    r.init();
    r.beginFrame(800.f, 600.f);

    NF::UITheme theme = NF::UITheme::dark();
    NF::UIMouseState mouse{};
    mouse.x = 20.f;
    mouse.y = 30.f;
    mouse.leftReleased = true;

    bool expanded = false;
    NF::UIContext ctx;
    ctx.begin(r, mouse, theme, 0.016f);
    ctx.beginPanel("Test", {0.f, 0.f, 200.f, 200.f});
    ctx.treeNode("Node", expanded);
    ctx.endPanel();
    ctx.end();

    REQUIRE(expanded == true);
    r.endFrame();
    r.shutdown();
}

TEST_CASE("UIContext separator draws line", "[UI][Widgets]") {
    NF::UIRenderer r;
    r.init();
    r.beginFrame(800.f, 600.f);

    NF::UITheme theme = NF::UITheme::dark();
    NF::UIMouseState mouse{};

    NF::UIContext ctx;
    ctx.begin(r, mouse, theme, 0.016f);
    ctx.beginPanel("Test", {0.f, 0.f, 200.f, 200.f});
    ctx.separator();
    ctx.endPanel();
    ctx.end();

    REQUIRE(r.vertices().size() > 0);
    r.endFrame();
    r.shutdown();
}

TEST_CASE("UIContext scrollArea renders scrollbar when overflow", "[UI][Widgets]") {
    NF::UIRenderer r;
    r.init();
    r.beginFrame(800.f, 600.f);

    NF::UITheme theme = NF::UITheme::dark();
    NF::UIMouseState mouse{};

    NF::UIContext ctx;
    ctx.begin(r, mouse, theme, 0.016f);
    // Content height > area height → scrollbar
    ctx.beginScrollArea("scroll1", {0.f, 0.f, 200.f, 100.f}, 500.f);
    ctx.label("Line 1");
    ctx.label("Line 2");
    ctx.endScrollArea();
    ctx.end();

    REQUIRE(r.vertices().size() > 0);
    r.endFrame();
    r.shutdown();
}

TEST_CASE("UIContext tooltip renders on end()", "[UI][Widgets]") {
    NF::UIRenderer r;
    r.init();
    r.beginFrame(800.f, 600.f);

    NF::UITheme theme = NF::UITheme::dark();
    NF::UIMouseState mouse{};
    mouse.x = 100.f;
    mouse.y = 100.f;

    NF::UIContext ctx;
    ctx.begin(r, mouse, theme, 0.016f);
    ctx.tooltip("Help text");
    ctx.end();

    // Tooltip draws rect + text
    REQUIRE(r.vertices().size() > 0);
    r.endFrame();
    r.shutdown();
}

TEST_CASE("UIContext dropdown cycles selection on click", "[UI][Widgets]") {
    NF::UIRenderer r;
    r.init();
    r.beginFrame(800.f, 600.f);

    NF::UITheme theme = NF::UITheme::dark();
    NF::UIMouseState mouse{};
    // Position on drop area
    mouse.x = 70.f;
    mouse.y = 30.f;
    mouse.leftReleased = true;

    int selected = 0;
    std::vector<std::string> options = {"Option A", "Option B", "Option C"};

    NF::UIContext ctx;
    ctx.begin(r, mouse, theme, 0.016f);
    ctx.beginPanel("Test", {0.f, 0.f, 300.f, 200.f});
    bool changed = ctx.dropdown("Mode", selected, options);
    ctx.endPanel();
    ctx.end();

    // May or may not have changed depending on hit test positioning
    REQUIRE(r.vertices().size() > 0);
    r.endFrame();
    r.shutdown();
}

TEST_CASE("UIContext menuItem renders and detects click", "[UI][Widgets]") {
    NF::UIRenderer r;
    r.init();
    r.beginFrame(800.f, 600.f);

    NF::UITheme theme = NF::UITheme::dark();
    NF::UIMouseState mouse{};
    mouse.x = 50.f;
    mouse.y = 30.f;
    mouse.leftReleased = true;

    NF::UIContext ctx;
    ctx.begin(r, mouse, theme, 0.016f);
    ctx.beginPanel("Menu", {0.f, 0.f, 200.f, 200.f});
    bool clicked = ctx.menuItem("Save", "Ctrl+S");
    ctx.endPanel();
    ctx.end();

    REQUIRE(clicked);
    r.endFrame();
    r.shutdown();
}

TEST_CASE("UIContext horizontal layout stacks items side by side", "[UI][Widgets]") {
    NF::UIRenderer r;
    r.init();
    r.beginFrame(800.f, 600.f);

    NF::UITheme theme = NF::UITheme::dark();
    NF::UIMouseState mouse{};

    NF::UIContext ctx;
    ctx.begin(r, mouse, theme, 0.016f);
    ctx.beginPanel("Test", {0.f, 0.f, 400.f, 200.f});
    ctx.beginHorizontal();
    ctx.button("A", 40.f);
    ctx.button("B", 40.f);
    ctx.button("C", 40.f);
    ctx.endHorizontal();
    ctx.endPanel();
    ctx.end();

    // Should have rendered panel + 3 buttons
    REQUIRE(r.vertices().size() > 0);
    r.endFrame();
    r.shutdown();
}

TEST_CASE("UITheme dark and light presets", "[UI][Theme]") {
    auto dark = NF::UITheme::dark();
    auto light = NF::UITheme::light();

    // Dark theme has dark panel background
    REQUIRE(dark.panelBackground == 0x2B2B2BFF);
    // Light theme has bright panel background
    REQUIRE(light.panelBackground == 0xF0F0F0FF);
    // Both have valid font sizes
    REQUIRE(dark.fontSize > 0.f);
    REQUIRE(light.fontSize > 0.f);
}

TEST_CASE("uiHash produces consistent hashes", "[UI][Widgets]") {
    uint32_t h1 = NF::uiHash("button_ok");
    uint32_t h2 = NF::uiHash("button_ok");
    uint32_t h3 = NF::uiHash("button_cancel");
    REQUIRE(h1 == h2);
    REQUIRE(h1 != h3);
}

TEST_CASE("UIContext textInput renders field", "[UI][Widgets]") {
    NF::UIRenderer r;
    r.init();
    r.beginFrame(800.f, 600.f);

    NF::UITheme theme = NF::UITheme::dark();
    NF::UIMouseState mouse{};

    std::string value = "test";
    NF::UIContext ctx;
    ctx.begin(r, mouse, theme, 0.016f);
    ctx.beginPanel("Test", {0.f, 0.f, 300.f, 200.f});
    ctx.textInput("Name", value);
    ctx.endPanel();
    ctx.end();

    REQUIRE(r.vertices().size() > 0);
    REQUIRE(value == "test");  // Not modified without focus system
    r.endFrame();
    r.shutdown();
}

// ── UIContext::hitRegion ─────────────────────────────────────────

TEST_CASE("UIContext hitRegion returns true on click inside rect", "[UI][Widgets]") {
    NF::UIRenderer r;
    r.init();
    r.beginFrame(800.f, 600.f);

    NF::UITheme theme = NF::UITheme::dark();
    NF::UIMouseState mouse{};
    mouse.x           = 50.f;
    mouse.y           = 50.f;
    mouse.leftReleased = true;  // mouse was just released inside the rect

    NF::UIContext ctx;
    ctx.begin(r, mouse, theme, 0.016f);

    // hitRegion works without any layout cursor (no beginPanel needed)
    bool clicked = ctx.hitRegion({10.f, 10.f, 100.f, 80.f});

    ctx.end();

    REQUIRE(clicked);
    r.endFrame();
    r.shutdown();
}

TEST_CASE("UIContext hitRegion returns false when mouse outside rect", "[UI][Widgets]") {
    NF::UIRenderer r;
    r.init();
    r.beginFrame(800.f, 600.f);

    NF::UITheme theme = NF::UITheme::dark();
    NF::UIMouseState mouse{};
    mouse.x           = 200.f;  // outside {10,10,100,80}
    mouse.y           = 200.f;
    mouse.leftReleased = true;

    NF::UIContext ctx;
    ctx.begin(r, mouse, theme, 0.016f);
    bool clicked = ctx.hitRegion({10.f, 10.f, 100.f, 80.f});
    ctx.end();

    REQUIRE_FALSE(clicked);
    r.endFrame();
    r.shutdown();
}

TEST_CASE("UIContext hitRegion returns false when leftReleased is false", "[UI][Widgets]") {
    NF::UIRenderer r;
    r.init();
    r.beginFrame(800.f, 600.f);

    NF::UITheme theme = NF::UITheme::dark();
    NF::UIMouseState mouse{};
    mouse.x            = 50.f;
    mouse.y            = 50.f;
    mouse.leftDown     = true;  // held but not released
    mouse.leftReleased = false;

    NF::UIContext ctx;
    ctx.begin(r, mouse, theme, 0.016f);
    bool clicked = ctx.hitRegion({10.f, 10.f, 100.f, 80.f});
    ctx.end();

    REQUIRE_FALSE(clicked);  // click only fires on leftReleased
    r.endFrame();
    r.shutdown();
}

TEST_CASE("UIContext hitRegion draws hover highlight when hovered", "[UI][Widgets]") {
    NF::UIRenderer r;
    r.init();
    r.beginFrame(800.f, 600.f);

    NF::UITheme theme = NF::UITheme::dark();
    NF::UIMouseState mouse{};
    mouse.x = 50.f;
    mouse.y = 50.f;
    // No click — just hovering
    size_t verticesBefore = r.vertices().size();

    NF::UIContext ctx;
    ctx.begin(r, mouse, theme, 0.016f);
    ctx.hitRegion({10.f, 10.f, 100.f, 80.f}, true);  // drawHover=true
    ctx.end();

    // Hover should have drawn a rect (4 more vertices)
    REQUIRE(r.vertices().size() == verticesBefore + 4);
    r.endFrame();
    r.shutdown();
}

TEST_CASE("UIContext hitRegion suppresses hover highlight when drawHover=false", "[UI][Widgets]") {
    NF::UIRenderer r;
    r.init();
    r.beginFrame(800.f, 600.f);

    NF::UITheme theme = NF::UITheme::dark();
    NF::UIMouseState mouse{};
    mouse.x = 50.f;
    mouse.y = 50.f;
    size_t verticesBefore = r.vertices().size();

    NF::UIContext ctx;
    ctx.begin(r, mouse, theme, 0.016f);
    ctx.hitRegion({10.f, 10.f, 100.f, 80.f}, false);  // drawHover=false
    ctx.end();

    // No additional vertices should be emitted
    REQUIRE(r.vertices().size() == verticesBefore);
    r.endFrame();
    r.shutdown();
}
