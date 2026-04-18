#include <catch2/catch_test_macros.hpp>

#include "NF/UI/AtlasUI/DrawPrimitives.h"
#include "NF/UI/AtlasUI/Interfaces.h"
#include "NF/UI/AtlasUI/PanelHost.h"
#include "NF/UI/AtlasUI/WidgetBase.h"
#include "NF/UI/AtlasUI/PanelBase.h"

#include "NF/UI/AtlasUI/Theme/Theme.h"
#include "NF/UI/AtlasUI/Theme/ThemeDefaults.h"
#include "NF/UI/AtlasUI/Theme/ThemeManager.h"
#include "NF/UI/AtlasUI/Theme/ThemeTokens.h"

#include "NF/UI/AtlasUI/Widgets/Button.h"
#include "NF/UI/AtlasUI/Widgets/Panel.h"
#include "NF/UI/AtlasUI/Widgets/TabBar.h"
#include "NF/UI/AtlasUI/Widgets/Splitter.h"
#include "NF/UI/AtlasUI/Widgets/Toolbar.h"
#include "NF/UI/AtlasUI/Widgets/Tooltip.h"
#include "NF/UI/AtlasUI/Widgets/TextInput.h"
#include "NF/UI/AtlasUI/Widgets/Dropdown.h"
#include "NF/UI/AtlasUI/Widgets/PropertyRow.h"
#include "NF/UI/AtlasUI/Widgets/NotificationCard.h"

#include "NF/UI/AtlasUI/Services/FocusService.h"
#include "NF/UI/AtlasUI/Services/PopupHost.h"
#include "NF/UI/AtlasUI/Services/TooltipService.h"
#include "NF/UI/AtlasUI/Services/NotificationHost.h"

#include "NF/UI/AtlasUI/Commands/CommandTypes.h"
#include "NF/UI/AtlasUI/Commands/CommandRegistry.h"
#include "NF/UI/AtlasUI/Commands/CommandManager.h"
#include "NF/UI/AtlasUI/Commands/ShortcutRouter.h"

using namespace NF::UI::AtlasUI;

// ── Concrete test panel (resolves diamond inheritance) ───────────

class TestPanel final : public IPanel {
public:
    TestPanel(std::string id, std::string title)
        : m_id(std::move(id)), m_title(std::move(title)) {}

    const char* panelId() const override { return m_id.c_str(); }
    const char* title() const override { return m_title.c_str(); }
    void initialize() override {}
    void activate() override {}
    void deactivate() override {}
    PanelState saveState() const override { return {m_id, m_title, m_visible, false, m_bounds}; }
    void loadState(const PanelState&) override {}

    void measure(ILayoutContext&) override {}
    void arrange(const NF::Rect& b) override { m_bounds = b; }
    void paint(IPaintContext&) override {}
    bool handleInput(IInputContext&) override { return false; }
    void setVisible(bool v) override { m_visible = v; }
    bool isVisible() const override { return m_visible; }
    NF::Rect bounds() const override { return m_bounds; }

private:
    std::string m_id, m_title;
    NF::Rect m_bounds{};
    bool m_visible = true;
};

// ── Test mock contexts ───────────────────────────────────────────

struct TestLayoutContext : ILayoutContext {
    float dpiScale() const override { return 1.0f; }
    NF::Vec2 availableSize() const override { return {800.f, 600.f}; }
    NF::Vec2 measureText(std::string_view text, float) const override {
        return {static_cast<float>(text.size()) * 8.f, 16.f};
    }
    void invalidateLayout() override {}
};

struct TestPaintContext : IPaintContext {
    DrawList dl;
    void drawRect(const NF::Rect& r, Color c) override { dl.push(DrawRectCmd{r, c}); }
    void fillRect(const NF::Rect& r, Color c) override { dl.push(FillRectCmd{r, c}); }
    void drawText(const NF::Rect& r, std::string_view t, FontId f, Color c) override {
        dl.push(DrawTextCmd{r, std::string(t), f, c});
    }
    void drawImage(const NF::Rect& r, uint32_t textureId, Color tint) override {
        dl.push(DrawImageCmd{r, textureId, tint});
    }
    void pushClip(const NF::Rect&) override {}
    void popClip() override {}
    DrawList& drawList() override { return dl; }
};

struct TestInputContext : IInputContext {
    NF::Vec2 pos{};
    bool primary = false;
    std::string typed;
    NF::Vec2 mousePosition() const override { return pos; }
    bool primaryDown() const override { return primary; }
    bool secondaryDown() const override { return false; }
    bool keyDown(int) const override { return false; }
    std::string_view typedText() const override { return typed; }
    void requestFocus(IWidget*) override {}
    void capturePointer(IWidget*) override {}
    void releasePointer(IWidget*) override {}
};

// ── Theme Tests ──────────────────────────────────────────────────

TEST_CASE("ThemeManager singleton returns same instance", "[AtlasUI][Theme]") {
    auto& a = ThemeManager::Get();
    auto& b = ThemeManager::Get();
    REQUIRE(&a == &b);
}

TEST_CASE("ThemeManager::Current() returns valid theme", "[AtlasUI][Theme]") {
    const auto& theme = ThemeManager::Get().Current();
    REQUIRE(theme.Colors.TextPrimary.r > 0.f);
}

TEST_CASE("ThemeManager::SetTheme() changes current theme", "[AtlasUI][Theme]") {
    auto& mgr = ThemeManager::Get();
    auto original = mgr.Current();

    AtlasTheme custom{};
    custom.Colors.Accent = ColorRGBA{1.0f, 0.0f, 0.0f, 1.0f};
    mgr.SetTheme(custom);
    REQUIRE(mgr.Current().Colors.Accent.r == 1.0f);
    REQUIRE(mgr.Current().Colors.Accent.g == 0.0f);

    mgr.SetTheme(original);
}

TEST_CASE("Atlas Dark theme has non-zero color values for key tokens", "[AtlasUI][Theme]") {
    const auto theme = MakeAtlasDarkTheme();
    REQUIRE(theme.Colors.WindowBg.r > 0.f);
    REQUIRE(theme.Colors.PanelBg.r > 0.f);
    REQUIRE(theme.Colors.TextPrimary.r > 0.f);
    REQUIRE(theme.Colors.Accent.r > 0.f);
    REQUIRE(theme.Colors.Accent.b > 0.f);
    REQUIRE(theme.Colors.Error.r > 0.f);
    REQUIRE(theme.Colors.Warning.r > 0.f);
    REQUIRE(theme.Colors.Success.g > 0.f);
}

TEST_CASE("ThemeSpacing defaults are positive", "[AtlasUI][Theme]") {
    ThemeSpacing sp{};
    REQUIRE(sp.XXS > 0.f);
    REQUIRE(sp.XS > 0.f);
    REQUIRE(sp.SM > 0.f);
    REQUIRE(sp.MD > 0.f);
    REQUIRE(sp.LG > 0.f);
    REQUIRE(sp.XL > 0.f);
    REQUIRE(sp.XXL > 0.f);
    REQUIRE(sp.PanelPadding > 0.f);
}

TEST_CASE("ThemeSizes defaults are positive", "[AtlasUI][Theme]") {
    ThemeSizes sz{};
    REQUIRE(sz.TitleBarHeight > 0.f);
    REQUIRE(sz.TabHeight > 0.f);
    REQUIRE(sz.ToolbarHeight > 0.f);
    REQUIRE(sz.ButtonHeight > 0.f);
    REQUIRE(sz.InputHeight > 0.f);
    REQUIRE(sz.RowHeight > 0.f);
    REQUIRE(sz.IconSM > 0.f);
    REQUIRE(sz.Scrollbar > 0.f);
    REQUIRE(sz.Splitter > 0.f);
}

TEST_CASE("ThemeRadii has expected range values", "[AtlasUI][Theme]") {
    ThemeRadii radii{};
    REQUIRE(radii.None == 0.f);
    REQUIRE(radii.SM > 0.f);
    REQUIRE(radii.SM < radii.MD);
    REQUIRE(radii.MD < radii.XL);
    REQUIRE(radii.Button > 0.f);
    REQUIRE(radii.Panel > 0.f);
}

// ── Widget Tests ─────────────────────────────────────────────────

TEST_CASE("Button creates with text", "[AtlasUI][Widget]") {
    Button btn("Click me");
    REQUIRE(btn.isVisible());
}

TEST_CASE("Button measure produces non-zero size", "[AtlasUI][Widget]") {
    Button btn("Hello");
    TestLayoutContext ctx;
    btn.measure(ctx);
    REQUIRE(btn.desiredSize().x > 0.f);
    REQUIRE(btn.desiredSize().y > 0.f);
}

TEST_CASE("Button handleInput detects hover", "[AtlasUI][Widget]") {
    Button btn("Hover");
    btn.arrange({10.f, 10.f, 100.f, 30.f});
    TestInputContext input;
    input.pos = {50.f, 20.f};
    bool result = btn.handleInput(input);
    REQUIRE(result);
}

TEST_CASE("Panel creates with title", "[AtlasUI][Widget]") {
    TestPanel p("panel.test", "Test Panel");
    REQUIRE(std::string(p.panelId()) == "panel.test");
    REQUIRE(std::string(p.title()) == "Test Panel");
}

TEST_CASE("TabBar adds tabs", "[AtlasUI][Widget]") {
    TabBar tb;
    auto w1 = std::make_shared<WidgetBase>();
    auto w2 = std::make_shared<WidgetBase>();
    tb.addTab("Tab1", w1);
    tb.addTab("Tab2", w2);
    REQUIRE(tb.children().size() == 2);
    REQUIRE(tb.activeIndex() == 0);
}

TEST_CASE("Splitter default ratio is reasonable", "[AtlasUI][Widget]") {
    Splitter sp(SplitOrientation::Horizontal);
    REQUIRE(sp.ratio() >= 0.1f);
    REQUIRE(sp.ratio() <= 0.9f);
}

TEST_CASE("Toolbar adds items", "[AtlasUI][Widget]") {
    Toolbar tb;
    bool called = false;
    tb.addItem("Build", [&]() { called = true; });
    tb.addSeparator();
    tb.addItem("Run", nullptr);

    TestLayoutContext lctx;
    tb.measure(lctx);
    REQUIRE(tb.bounds().w > 0.f);
}

TEST_CASE("Tooltip creates with text", "[AtlasUI][Widget]") {
    Tooltip tt;
    tt.hide();
    REQUIRE_FALSE(tt.isShowing());
    tt.show("Help text", {100.f, 200.f});
    REQUIRE(tt.isShowing());
    tt.hide();
    REQUIRE_FALSE(tt.isShowing());
}

TEST_CASE("TextInput creates empty", "[AtlasUI][Widget]") {
    TextInput ti;
    REQUIRE(ti.text().empty());
}

TEST_CASE("TextInput accepts character input via setText", "[AtlasUI][Widget]") {
    TextInput ti;
    ti.setText("Hello");
    REQUIRE(ti.text() == "Hello");
}

TEST_CASE("Dropdown adds options", "[AtlasUI][Widget]") {
    Dropdown dd({"Alpha", "Beta", "Gamma"});
    REQUIRE(dd.selectedIndex() == 0);
    REQUIRE(dd.selectedText() == "Alpha");
    dd.setSelectedIndex(2);
    REQUIRE(dd.selectedText() == "Gamma");
}

TEST_CASE("PropertyRow creates with label", "[AtlasUI][Widget]") {
    auto input = std::make_shared<TextInput>();
    PropertyRow row("Color", input);
    REQUIRE(row.label() == "Color");
}

TEST_CASE("NotificationCard creates with severity", "[AtlasUI][Widget]") {
    NotificationCard card("Build succeeded", NotificationLevel::Info);
    REQUIRE_FALSE(card.isExpired());
    card.dismiss();
    REQUIRE(card.isExpired());
}

// ── Service Tests ────────────────────────────────────────────────

TEST_CASE("FocusService sets and clears focus", "[AtlasUI][Service]") {
    auto& fs = FocusService::Get();
    WidgetBase w;
    fs.setFocus(&w);
    REQUIRE(fs.focusedWidget() == &w);
    fs.clearFocus();
    REQUIRE(fs.focusedWidget() == nullptr);
}

TEST_CASE("FocusService reports HasFocus correctly", "[AtlasUI][Service]") {
    auto& fs = FocusService::Get();
    WidgetBase a, b;
    fs.setFocus(&a);
    REQUIRE(fs.hasFocus(&a));
    REQUIRE_FALSE(fs.hasFocus(&b));
    fs.clearFocus();
}

TEST_CASE("PopupHost open/close popup", "[AtlasUI][Service]") {
    auto& ph = PopupHost::Get();
    ph.closePopup();
    REQUIRE_FALSE(ph.isPopupOpen());
    auto widget = std::make_shared<WidgetBase>();
    ph.openPopup({10.f, 10.f, 200.f, 100.f}, widget);
    REQUIRE(ph.isPopupOpen());
    ph.closePopup();
    REQUIRE_FALSE(ph.isPopupOpen());
}

TEST_CASE("TooltipService show/hide tooltip", "[AtlasUI][Service]") {
    auto& ts = TooltipService::Get();
    ts.hide();
    REQUIRE_FALSE(ts.isShowing());
    ts.show("Helpful tip", {50.f, 50.f});
    REQUIRE(ts.isShowing());
    ts.hide();
    REQUIRE_FALSE(ts.isShowing());
}

TEST_CASE("NotificationHost push/dismiss notification", "[AtlasUI][Service]") {
    auto& nh = NotificationHost::Get();
    nh.dismissAll();
    REQUIRE(nh.count() == 0);

    nh.post("Build finished");
    REQUIRE(nh.count() == 1);
    nh.post("Warning: deprecated API", NotificationLevel::Warning);
    REQUIRE(nh.count() == 2);

    nh.dismissAll();
    REQUIRE(nh.count() == 0);
}

// ── Command Tests ────────────────────────────────────────────────

TEST_CASE("CommandRegistry register and find", "[AtlasUI][Command]") {
    CommandRegistry reg;
    bool executed = false;
    CommandSpec spec;
    spec.Id = "test.save";
    spec.Label = "Save";
    spec.Scope = CommandScope::Global;
    spec.Binding.Primary = {true, false, false, false, 'S'};
    spec.Execute = [&](const CommandContext&) { executed = true; };

    REQUIRE(reg.Register(std::move(spec)));
    const auto* found = reg.Find("test.save");
    REQUIRE(found != nullptr);
    REQUIRE(found->Label == "Save");

    found->Execute(CommandContext{});
    REQUIRE(executed);

    REQUIRE(reg.Find("nonexistent") == nullptr);
}

TEST_CASE("CommandRegistry unregister", "[AtlasUI][Command]") {
    CommandRegistry reg;
    CommandSpec spec;
    spec.Id = "test.remove";
    spec.Label = "Remove";
    spec.Execute = [](const CommandContext&) {};

    REQUIRE(reg.Register(std::move(spec)));
    REQUIRE(reg.Find("test.remove") != nullptr);
    REQUIRE(reg.Unregister("test.remove"));
    REQUIRE(reg.Find("test.remove") == nullptr);
    REQUIRE_FALSE(reg.Unregister("test.remove"));
}

TEST_CASE("CommandManager singleton", "[AtlasUI][Command]") {
    auto& a = CommandManager::Get();
    auto& b = CommandManager::Get();
    REQUIRE(&a == &b);
}

TEST_CASE("ShortcutRouter routes to correct command", "[AtlasUI][Command]") {
    CommandRegistry reg;
    bool saveCalled = false;
    bool openCalled = false;

    CommandSpec saveSpec;
    saveSpec.Id = "router.save";
    saveSpec.Label = "Save";
    saveSpec.Scope = CommandScope::Global;
    saveSpec.Binding.Primary = {true, false, false, false, 'S'};
    saveSpec.Execute = [&](const CommandContext&) { saveCalled = true; };
    reg.Register(std::move(saveSpec));

    CommandSpec openSpec;
    openSpec.Id = "router.open";
    openSpec.Label = "Open";
    openSpec.Scope = CommandScope::Global;
    openSpec.Binding.Primary = {true, false, false, false, 'O'};
    openSpec.Execute = [&](const CommandContext&) { openCalled = true; };
    reg.Register(std::move(openSpec));

    ShortcutRouter router(reg);
    CommandContext ctx;
    ctx.Scope = CommandScope::Global;

    REQUIRE(router.TryExecute({true, false, false, false, 'S'}, ctx));
    REQUIRE(saveCalled);
    REQUIRE_FALSE(openCalled);

    REQUIRE(router.TryExecute({true, false, false, false, 'O'}, ctx));
    REQUIRE(openCalled);

    REQUIRE_FALSE(router.TryExecute({true, false, false, false, 'X'}, ctx));
}

TEST_CASE("CommandTypes KeyChord creation and comparison", "[AtlasUI][Command]") {
    KeyChord a{true, false, true, false, 'Z'};
    KeyChord b{true, false, true, false, 'Z'};
    KeyChord c{false, false, true, false, 'Z'};

    REQUIRE(a == b);
    REQUIRE_FALSE(a == c);

    REQUIRE(a.IsValid());
    KeyChord empty{};
    REQUIRE_FALSE(empty.IsValid());

    REQUIRE(a.ToString() == "Ctrl+Shift+Z");
}

// ── PanelHost Tests ──────────────────────────────────────────────

TEST_CASE("PanelHost attach/detach", "[AtlasUI][PanelHost]") {
    PanelHost host;
    auto p1 = std::make_shared<TestPanel>("p1", "First");
    auto p2 = std::make_shared<TestPanel>("p2", "Second");

    host.attachPanel(p1);
    host.attachPanel(p2);
    REQUIRE(host.panels().size() == 2);

    host.detachPanel("p1");
    REQUIRE(host.panels().size() == 1);
    REQUIRE(host.findPanel("p1") == nullptr);
}

TEST_CASE("PanelHost find panel", "[AtlasUI][PanelHost]") {
    PanelHost host;
    auto p = std::make_shared<TestPanel>("viewport", "Viewport");
    host.attachPanel(p);

    auto found = host.findPanel("viewport");
    REQUIRE(found != nullptr);
    REQUIRE(std::string(found->title()) == "Viewport");

    REQUIRE(host.findPanel("missing") == nullptr);
}

TEST_CASE("PanelHost no duplicate attachment", "[AtlasUI][PanelHost]") {
    PanelHost host;
    auto p = std::make_shared<TestPanel>("dup", "Duplicate");
    host.attachPanel(p);
    host.attachPanel(p);
    REQUIRE(host.panels().size() == 1);
}

// ── DrawList Tests ───────────────────────────────────────────────

TEST_CASE("DrawList starts empty", "[AtlasUI][DrawList]") {
    DrawList dl;
    REQUIRE(dl.empty());
    REQUIRE(dl.size() == 0);
}

TEST_CASE("DrawList push commands", "[AtlasUI][DrawList]") {
    DrawList dl;
    dl.push(FillRectCmd{{0.f, 0.f, 100.f, 50.f}, 0xFF0000FF});
    dl.push(DrawRectCmd{{10.f, 10.f, 80.f, 30.f}, 0xFFFFFFFF});
    dl.push(DrawTextCmd{{20.f, 20.f, 60.f, 16.f}, "Hello", 0, 0xFFFFFFFF});
    REQUIRE(dl.size() == 3);
    REQUIRE_FALSE(dl.empty());
}

TEST_CASE("DrawList clear", "[AtlasUI][DrawList]") {
    DrawList dl;
    dl.push(FillRectCmd{{0.f, 0.f, 100.f, 50.f}, 0xFF0000FF});
    dl.push(DrawRectCmd{{0.f, 0.f, 50.f, 50.f}, 0xFFFFFFFF});
    REQUIRE(dl.size() == 2);
    dl.clear();
    REQUIRE(dl.empty());
    REQUIRE(dl.size() == 0);
}

// ── AtlasUI Panel Tests (U1–U8) ────────────────────────────────

#include "NF/UI/AtlasUI/Panels/InspectorPanel.h"
#include "NF/UI/AtlasUI/Panels/HierarchyPanel.h"
#include "NF/UI/AtlasUI/Panels/ContentBrowserPanel.h"
#include "NF/UI/AtlasUI/Panels/ConsolePanel.h"
#include "NF/UI/AtlasUI/Panels/IDEPanel.h"
#include "NF/UI/AtlasUI/Panels/GraphEditorPanel.h"
#include "NF/UI/AtlasUI/Panels/ViewportPanel.h"
#include "NF/UI/AtlasUI/Panels/PipelineMonitorPanel.h"

// ── InspectorPanel (U1) ─────────────────────────────────────────

TEST_CASE("InspectorPanel has correct panel ID", "[AtlasUI][Panel][Inspector]") {
    InspectorPanel panel;
    REQUIRE(std::string(panel.panelId()) == "atlas.inspector");
    REQUIRE(std::string(panel.title()) == "Inspector");
}

TEST_CASE("InspectorPanel entity selection", "[AtlasUI][Panel][Inspector]") {
    InspectorPanel panel;
    REQUIRE(panel.selectedEntityId() == -1);
    panel.setSelectedEntityId(42);
    REQUIRE(panel.selectedEntityId() == 42);
}

TEST_CASE("InspectorPanel transform data", "[AtlasUI][Panel][Inspector]") {
    InspectorPanel panel;
    panel.setTransform(1.f, 2.f, 3.f);
    REQUIRE(panel.transformX() == 1.f);
    REQUIRE(panel.transformY() == 2.f);
    REQUIRE(panel.transformZ() == 3.f);
}

TEST_CASE("InspectorPanel properties", "[AtlasUI][Panel][Inspector]") {
    InspectorPanel panel;
    panel.addProperty("Health", "100");
    panel.addProperty("Name", "Player");
    REQUIRE(panel.properties().size() == 2);
    REQUIRE(panel.properties()[0].label == "Health");
    REQUIRE(panel.properties()[1].value == "Player");
    panel.clearProperties();
    REQUIRE(panel.properties().empty());
}

TEST_CASE("InspectorPanel paint produces draw commands", "[AtlasUI][Panel][Inspector]") {
    InspectorPanel panel;
    panel.arrange({0.f, 0.f, 300.f, 400.f});
    panel.setSelectedEntityId(7);
    panel.setTransform(10.f, 20.f, 30.f);
    TestPaintContext ctx;
    panel.paint(ctx);
    REQUIRE(ctx.dl.size() > 0);
}

TEST_CASE("InspectorPanel paint with no selection", "[AtlasUI][Panel][Inspector]") {
    InspectorPanel panel;
    panel.arrange({0.f, 0.f, 300.f, 400.f});
    TestPaintContext ctx;
    panel.paint(ctx);
    REQUIRE(ctx.dl.size() > 0);
}

TEST_CASE("InspectorPanel state save/load", "[AtlasUI][Panel][Inspector]") {
    InspectorPanel panel;
    panel.arrange({10.f, 20.f, 300.f, 400.f});
    panel.setVisible(false);
    auto state = panel.saveState();
    REQUIRE(state.panelId == "atlas.inspector");
    REQUIRE(state.visible == false);

    InspectorPanel panel2;
    panel2.loadState(state);
    REQUIRE(panel2.isVisible() == false);
}

// ── HierarchyPanel (U2) ────────────────────────────────────────

TEST_CASE("HierarchyPanel has correct panel ID", "[AtlasUI][Panel][Hierarchy]") {
    HierarchyPanel panel;
    REQUIRE(std::string(panel.panelId()) == "atlas.hierarchy");
    REQUIRE(std::string(panel.title()) == "Hierarchy");
}

TEST_CASE("HierarchyPanel entity management", "[AtlasUI][Panel][Hierarchy]") {
    HierarchyPanel panel;
    panel.addEntity(1, "Player", true);
    panel.addEntity(2, "Enemy");
    panel.addEntity(3, "World", false, 0);
    REQUIRE(panel.entityCount() == 3);
    REQUIRE(panel.entities()[0].selected == true);
    REQUIRE(panel.entities()[1].name == "Enemy");
    panel.clearEntities();
    REQUIRE(panel.entityCount() == 0);
}

TEST_CASE("HierarchyPanel search filter", "[AtlasUI][Panel][Hierarchy]") {
    HierarchyPanel panel;
    panel.setSearchFilter("Player");
    REQUIRE(panel.searchFilter() == "Player");
}

TEST_CASE("HierarchyPanel paint produces draw commands", "[AtlasUI][Panel][Hierarchy]") {
    HierarchyPanel panel;
    panel.arrange({0.f, 0.f, 250.f, 400.f});
    panel.addEntity(1, "Player", true);
    panel.addEntity(2, "Enemy");
    TestPaintContext ctx;
    panel.paint(ctx);
    REQUIRE(ctx.dl.size() > 0);
}

// ── ContentBrowserPanel (U3) ────────────────────────────────────

TEST_CASE("ContentBrowserPanel has correct panel ID", "[AtlasUI][Panel][ContentBrowser]") {
    ContentBrowserPanel panel;
    REQUIRE(std::string(panel.panelId()) == "atlas.content_browser");
    REQUIRE(std::string(panel.title()) == "Content Browser");
}

TEST_CASE("ContentBrowserPanel entry management", "[AtlasUI][Panel][ContentBrowser]") {
    ContentBrowserPanel panel;
    panel.addEntry("Models", true);
    panel.addEntry("player.obj", false);
    REQUIRE(panel.entryCount() == 2);
    REQUIRE(panel.entries()[0].isDirectory == true);
    REQUIRE(panel.entries()[1].name == "player.obj");
    panel.clearEntries();
    REQUIRE(panel.entryCount() == 0);
}

TEST_CASE("ContentBrowserPanel path navigation", "[AtlasUI][Panel][ContentBrowser]") {
    ContentBrowserPanel panel;
    REQUIRE(panel.currentPath() == "/");
    panel.setCurrentPath("/Content/Models");
    REQUIRE(panel.currentPath() == "/Content/Models");
}

TEST_CASE("ContentBrowserPanel paint produces draw commands", "[AtlasUI][Panel][ContentBrowser]") {
    ContentBrowserPanel panel;
    panel.arrange({0.f, 0.f, 250.f, 400.f});
    panel.setCurrentPath("/Content");
    panel.addEntry("Textures", true);
    panel.addEntry("readme.txt", false);
    TestPaintContext ctx;
    panel.paint(ctx);
    REQUIRE(ctx.dl.size() > 0);
}

// ── ConsolePanel (U4) ───────────────────────────────────────────

TEST_CASE("ConsolePanel has correct panel ID", "[AtlasUI][Panel][Console]") {
    ConsolePanel panel;
    REQUIRE(std::string(panel.panelId()) == "atlas.console");
    REQUIRE(std::string(panel.title()) == "Console");
}

TEST_CASE("ConsolePanel message management", "[AtlasUI][Panel][Console]") {
    ConsolePanel panel;
    panel.addMessage("Hello", MessageLevel::Info, 0.f);
    panel.addMessage("Warning!", MessageLevel::Warning, 1.f);
    panel.addMessage("Error!", MessageLevel::Error, 2.f);
    REQUIRE(panel.messageCount() == 3);
    REQUIRE(panel.messages()[0].text == "Hello");
    REQUIRE(panel.messages()[1].level == MessageLevel::Warning);
    REQUIRE(panel.messages()[2].level == MessageLevel::Error);
    panel.clearMessages();
    REQUIRE(panel.messageCount() == 0);
}

TEST_CASE("ConsolePanel enforces max message limit", "[AtlasUI][Panel][Console]") {
    ConsolePanel panel;
    for (size_t i = 0; i <= ConsolePanel::kMaxMessages; ++i) {
        panel.addMessage("msg " + std::to_string(i), MessageLevel::Info);
    }
    REQUIRE(panel.messageCount() == ConsolePanel::kMaxMessages);
}

TEST_CASE("ConsolePanel paint produces draw commands", "[AtlasUI][Panel][Console]") {
    ConsolePanel panel;
    panel.arrange({0.f, 0.f, 400.f, 200.f});
    panel.addMessage("Test info", MessageLevel::Info);
    panel.addMessage("Test warning", MessageLevel::Warning);
    panel.addMessage("Test error", MessageLevel::Error);
    TestPaintContext ctx;
    panel.paint(ctx);
    REQUIRE(ctx.dl.size() > 0);
}

// ── IDEPanel (U5) ───────────────────────────────────────────────

TEST_CASE("IDEPanel has correct panel ID", "[AtlasUI][Panel][IDE]") {
    IDEPanel panel;
    REQUIRE(std::string(panel.panelId()) == "atlas.ide");
    REQUIRE(std::string(panel.title()) == "IDE");
}

TEST_CASE("IDEPanel search and results", "[AtlasUI][Panel][IDE]") {
    IDEPanel panel;
    panel.setSearchQuery("Entity");
    REQUIRE(panel.searchQuery() == "Entity");

    panel.addResult("Entity", "Source/Game/Entity.h", 42);
    panel.addResult("EntityManager", "Source/Game/EntityManager.h", 10);
    REQUIRE(panel.resultCount() == 2);
    REQUIRE(panel.results()[0].symbolName == "Entity");
    REQUIRE(panel.results()[0].line == 42);
    panel.clearResults();
    REQUIRE(panel.resultCount() == 0);
}

TEST_CASE("IDEPanel paint produces draw commands", "[AtlasUI][Panel][IDE]") {
    IDEPanel panel;
    panel.arrange({0.f, 0.f, 400.f, 300.f});
    panel.setSearchQuery("Test");
    panel.addResult("TestClass", "test.h");
    TestPaintContext ctx;
    panel.paint(ctx);
    REQUIRE(ctx.dl.size() > 0);
}

// ── GraphEditorPanel (U6) ───────────────────────────────────────

TEST_CASE("GraphEditorPanel has correct panel ID", "[AtlasUI][Panel][GraphEditor]") {
    GraphEditorPanel panel;
    REQUIRE(std::string(panel.panelId()) == "atlas.graph_editor");
    REQUIRE(std::string(panel.title()) == "Graph Editor");
}

TEST_CASE("GraphEditorPanel node management", "[AtlasUI][Panel][GraphEditor]") {
    GraphEditorPanel panel;
    panel.setGraphName("TestGraph");
    REQUIRE(panel.hasOpenGraph());
    REQUIRE(panel.graphName() == "TestGraph");

    int n1 = panel.addNode("Start", 0.f, 0.f);
    int n2 = panel.addNode("End", 200.f, 100.f);
    REQUIRE(panel.nodeCount() == 2);
    REQUIRE(n1 != n2);

    panel.addLink(n1, n2);
    REQUIRE(panel.linkCount() == 1);

    panel.selectNode(n1);
    REQUIRE(panel.selectedNodeId() == n1);
    panel.clearSelection();
    REQUIRE(panel.selectedNodeId() == -1);

    REQUIRE(panel.removeNode(n1));
    REQUIRE(panel.nodeCount() == 1);
    REQUIRE_FALSE(panel.removeNode(999));
}

TEST_CASE("GraphEditorPanel clear graph", "[AtlasUI][Panel][GraphEditor]") {
    GraphEditorPanel panel;
    panel.setGraphName("G");
    panel.addNode("A", 0.f, 0.f);
    panel.addNode("B", 100.f, 0.f);
    panel.addLink(1, 2);
    panel.clearGraph();
    REQUIRE(panel.nodeCount() == 0);
    REQUIRE(panel.linkCount() == 0);
    REQUIRE(panel.selectedNodeId() == -1);
}

TEST_CASE("GraphEditorPanel paint with no graph", "[AtlasUI][Panel][GraphEditor]") {
    GraphEditorPanel panel;
    panel.arrange({0.f, 0.f, 400.f, 300.f});
    TestPaintContext ctx;
    panel.paint(ctx);
    REQUIRE(ctx.dl.size() > 0);
}

TEST_CASE("GraphEditorPanel paint with nodes", "[AtlasUI][Panel][GraphEditor]") {
    GraphEditorPanel panel;
    panel.arrange({0.f, 0.f, 400.f, 300.f});
    panel.setGraphName("MyGraph");
    panel.addNode("Start", 10.f, 20.f);
    panel.addNode("End", 150.f, 80.f);
    panel.addLink(1, 2);
    TestPaintContext ctx;
    panel.paint(ctx);
    REQUIRE(ctx.dl.size() > 0);
}

// ── ViewportPanel (U7) ──────────────────────────────────────────

TEST_CASE("ViewportPanel has correct panel ID", "[AtlasUI][Panel][Viewport]") {
    ViewportPanel panel;
    REQUIRE(std::string(panel.panelId()) == "atlas.viewport");
    REQUIRE(std::string(panel.title()) == "Viewport");
}

TEST_CASE("ViewportPanel camera and settings", "[AtlasUI][Panel][Viewport]") {
    ViewportPanel panel;
    panel.setCameraPosition(10.f, 20.f, 30.f);
    REQUIRE(panel.cameraX() == 10.f);
    REQUIRE(panel.cameraY() == 20.f);
    REQUIRE(panel.cameraZ() == 30.f);

    panel.setGridEnabled(false);
    REQUIRE(panel.gridEnabled() == false);

    panel.setRenderMode(ViewportRenderMode::Wireframe);
    REQUIRE(panel.renderMode() == ViewportRenderMode::Wireframe);

    panel.setToolMode(ViewportToolMode::Rotate);
    REQUIRE(panel.toolMode() == ViewportToolMode::Rotate);
}

TEST_CASE("ViewportPanel paint produces draw commands", "[AtlasUI][Panel][Viewport]") {
    ViewportPanel panel;
    panel.arrange({0.f, 0.f, 800.f, 600.f});
    panel.setCameraPosition(5.f, 10.f, 15.f);
    TestPaintContext ctx;
    panel.paint(ctx);
    REQUIRE(ctx.dl.size() > 0);
}

TEST_CASE("ViewportPanel paint without grid", "[AtlasUI][Panel][Viewport]") {
    ViewportPanel panel;
    panel.arrange({0.f, 0.f, 800.f, 600.f});
    panel.setGridEnabled(false);
    TestPaintContext ctx;
    panel.paint(ctx);
    REQUIRE(ctx.dl.size() > 0);
}

// ── PipelineMonitorPanel ────────────────────────────────────────

TEST_CASE("PipelineMonitorPanel has correct panel ID", "[AtlasUI][Panel][PipelineMonitor]") {
    PipelineMonitorPanel panel;
    REQUIRE(std::string(panel.panelId()) == "atlas.pipeline_monitor");
    REQUIRE(std::string(panel.title()) == "Pipeline Monitor");
}

TEST_CASE("PipelineMonitorPanel event management", "[AtlasUI][Panel][PipelineMonitor]") {
    PipelineMonitorPanel panel;
    panel.addEvent("FileAdded", "blendergen", "model.obj exported", 1.0f);
    panel.addEvent("ContractIssue", "scanner", "null check missing", 2.0f);
    REQUIRE(panel.eventCount() == 2);
    REQUIRE(panel.events()[0].type == "FileAdded");
    REQUIRE(panel.events()[1].source == "scanner");
    panel.clearEvents();
    REQUIRE(panel.eventCount() == 0);
}

TEST_CASE("PipelineMonitorPanel enforces max event limit", "[AtlasUI][Panel][PipelineMonitor]") {
    PipelineMonitorPanel panel;
    for (size_t i = 0; i <= PipelineMonitorPanel::kMaxEvents; ++i) {
        panel.addEvent("T", "S", "D" + std::to_string(i));
    }
    REQUIRE(panel.eventCount() == PipelineMonitorPanel::kMaxEvents);
}

TEST_CASE("PipelineMonitorPanel paint produces draw commands", "[AtlasUI][Panel][PipelineMonitor]") {
    PipelineMonitorPanel panel;
    panel.arrange({0.f, 0.f, 400.f, 200.f});
    panel.addEvent("FileAdded", "test", "test.txt");
    TestPaintContext ctx;
    panel.paint(ctx);
    REQUIRE(ctx.dl.size() > 0);
}

// ── Panel PanelHost integration ─────────────────────────────────

TEST_CASE("AtlasUI panels register with PanelHost", "[AtlasUI][Panel][PanelHost]") {
    PanelHost host;

    auto inspector = std::make_shared<InspectorPanel>();
    auto hierarchy = std::make_shared<HierarchyPanel>();
    auto browser = std::make_shared<ContentBrowserPanel>();
    auto console = std::make_shared<ConsolePanel>();
    auto ide = std::make_shared<IDEPanel>();
    auto graph = std::make_shared<GraphEditorPanel>();
    auto viewport = std::make_shared<ViewportPanel>();
    auto pipeline = std::make_shared<PipelineMonitorPanel>();

    host.attachPanel(inspector);
    host.attachPanel(hierarchy);
    host.attachPanel(browser);
    host.attachPanel(console);
    host.attachPanel(ide);
    host.attachPanel(graph);
    host.attachPanel(viewport);
    host.attachPanel(pipeline);

    REQUIRE(host.panels().size() == 8);
    REQUIRE(host.findPanel("atlas.inspector") != nullptr);
    REQUIRE(host.findPanel("atlas.hierarchy") != nullptr);
    REQUIRE(host.findPanel("atlas.content_browser") != nullptr);
    REQUIRE(host.findPanel("atlas.console") != nullptr);
    REQUIRE(host.findPanel("atlas.ide") != nullptr);
    REQUIRE(host.findPanel("atlas.graph_editor") != nullptr);
    REQUIRE(host.findPanel("atlas.viewport") != nullptr);
    REQUIRE(host.findPanel("atlas.pipeline_monitor") != nullptr);
}

TEST_CASE("All AtlasUI panels hidden paint produces no commands", "[AtlasUI][Panel]") {
    TestPaintContext ctx;
    InspectorPanel p1;
    HierarchyPanel p2;
    ContentBrowserPanel p3;
    ConsolePanel p4;
    IDEPanel p5;
    GraphEditorPanel p6;
    ViewportPanel p7;
    PipelineMonitorPanel p8;

    p1.setVisible(false); p1.paint(ctx);
    p2.setVisible(false); p2.paint(ctx);
    p3.setVisible(false); p3.paint(ctx);
    p4.setVisible(false); p4.paint(ctx);
    p5.setVisible(false); p5.paint(ctx);
    p6.setVisible(false); p6.paint(ctx);
    p7.setVisible(false); p7.paint(ctx);
    p8.setVisible(false); p8.paint(ctx);

    REQUIRE(ctx.dl.empty());
}

