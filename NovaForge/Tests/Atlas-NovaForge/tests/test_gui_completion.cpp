#include "../editor/ui/EditorPanel.h"
#include "../editor/ui/EditorLayout.h"
#include "../editor/ui/EditorTheme.h"
#include "../editor/ui/EditorAttachProtocol.h"
#include "../editor/ui/DefaultEditorLayout.h"
#include "../engine/ui/GUIDSLParser.h"
#include <iostream>
#include <cassert>
#include <cstring>

// --- Stub panel for testing ---
class StubPanel : public atlas::editor::EditorPanel {
public:
    explicit StubPanel(const char* name, bool closable = true)
        : m_name(name) { SetClosable(closable); }
    const char* Name() const override { return m_name; }
    void Draw() override { m_drawCount++; }
    int DrawCount() const { return m_drawCount; }
private:
    const char* m_name;
    int m_drawCount = 0;
};

// ============================================================
// Editor Theme Tests
// ============================================================

void test_editor_theme_dark_defaults() {
    auto theme = atlas::editor::EditorTheme::Dark();
    assert(theme.background.r == 30);
    assert(theme.background.g == 31);
    assert(theme.background.b == 34);
    assert(theme.background.a == 255);
    assert(theme.panelBackground.r == 43);
    assert(theme.textPrimary.r == 220);
    assert(theme.accentPrimary.r == 66);
    assert(theme.statusError.r == 230);
    assert(theme.fontSizeBody == 13);
    assert(theme.spacingUnit == 4);
    assert(theme.panelHeaderHeight == 28);
    assert(theme.iconSizeSmall == 16);
    std::cout << "[PASS] test_editor_theme_dark_defaults" << std::endl;
}

void test_editor_theme_custom_values() {
    atlas::editor::EditorTheme theme;
    theme.fontSizeHeader = 20;
    theme.paddingLarge = 24;
    assert(theme.fontSizeHeader == 20);
    assert(theme.paddingLarge == 24);
    std::cout << "[PASS] test_editor_theme_custom_values" << std::endl;
}

// ============================================================
// Non-Closable Panel Tests
// ============================================================

void test_panel_closable_by_default() {
    StubPanel panel("TestPanel");
    assert(panel.IsClosable() == true);
    std::cout << "[PASS] test_panel_closable_by_default" << std::endl;
}

void test_panel_set_non_closable() {
    StubPanel panel("Locked");
    panel.SetClosable(false);
    assert(panel.IsClosable() == false);
    std::cout << "[PASS] test_panel_set_non_closable" << std::endl;
}

void test_layout_close_closable_panel() {
    atlas::editor::EditorLayout layout;
    StubPanel panel("Optional");
    layout.RegisterPanel(&panel);

    assert(panel.IsVisible());
    bool closed = layout.ClosePanel("Optional");
    assert(closed);
    assert(!panel.IsVisible());
    std::cout << "[PASS] test_layout_close_closable_panel" << std::endl;
}

void test_layout_close_non_closable_panel() {
    atlas::editor::EditorLayout layout;
    StubPanel panel("Console", false);
    layout.RegisterPanel(&panel);

    assert(panel.IsVisible());
    bool closed = layout.ClosePanel("Console");
    assert(!closed);
    assert(panel.IsVisible());
    std::cout << "[PASS] test_layout_close_non_closable_panel" << std::endl;
}

void test_layout_close_unknown_panel() {
    atlas::editor::EditorLayout layout;
    bool closed = layout.ClosePanel("DoesNotExist");
    assert(!closed);
    std::cout << "[PASS] test_layout_close_unknown_panel" << std::endl;
}

void test_layout_find_panel() {
    atlas::editor::EditorLayout layout;
    StubPanel panel("Inspector");
    layout.RegisterPanel(&panel);

    auto* found = layout.FindPanel("Inspector");
    assert(found != nullptr);
    assert(std::strcmp(found->Name(), "Inspector") == 0);

    auto* notFound = layout.FindPanel("DoesNotExist");
    assert(notFound == nullptr);
    std::cout << "[PASS] test_layout_find_panel" << std::endl;
}

void test_main_panels_non_closable() {
    atlas::editor::EditorLayout layout;

    StubPanel assetBrowser("AssetBrowser", false);
    StubPanel viewport("Viewport", false);
    StubPanel inspector("Inspector", false);
    StubPanel replay("ReplayTimeline", false);
    StubPanel console("Console", false);
    StubPanel permissions("Permissions", false);
    StubPanel determinism("DeterminismStatus", false);

    layout.RegisterPanel(&assetBrowser);
    layout.RegisterPanel(&viewport);
    layout.RegisterPanel(&inspector);
    layout.RegisterPanel(&replay);
    layout.RegisterPanel(&console);
    layout.RegisterPanel(&permissions);
    layout.RegisterPanel(&determinism);

    // None of the main panels can be closed
    for (auto* p : layout.Panels()) {
        bool closed = layout.ClosePanel(p->Name());
        assert(!closed);
        assert(p->IsVisible());
    }
    std::cout << "[PASS] test_main_panels_non_closable" << std::endl;
}

// ============================================================
// Editor Attach Protocol Tests
// ============================================================

void test_attach_init() {
    atlas::editor::EditorAttachProtocol proto;
    proto.Init();
    assert(proto.GetState() == atlas::editor::AttachState::Disconnected);
    assert(!proto.IsConnected());
    std::cout << "[PASS] test_attach_init" << std::endl;
}

void test_attach_standalone() {
    atlas::editor::EditorAttachProtocol proto;
    proto.Init();

    atlas::editor::AttachConfig cfg;
    cfg.mode = atlas::editor::AttachMode::Standalone;
    bool ok = proto.Connect(cfg);
    assert(ok);
    assert(proto.IsConnected());
    assert(proto.GetMode() == atlas::editor::AttachMode::Standalone);
    assert(proto.TargetDescription() == "Standalone");
    std::cout << "[PASS] test_attach_standalone" << std::endl;
}

void test_attach_live_client() {
    atlas::editor::EditorAttachProtocol proto;
    proto.Init();

    atlas::editor::AttachConfig cfg;
    cfg.mode = atlas::editor::AttachMode::LiveClient;
    cfg.host = "127.0.0.1";
    cfg.port = 9100;
    bool ok = proto.Connect(cfg);
    assert(ok);
    assert(proto.IsConnected());
    assert(proto.GetMode() == atlas::editor::AttachMode::LiveClient);
    assert(proto.TargetDescription().find("127.0.0.1") != std::string::npos);
    std::cout << "[PASS] test_attach_live_client" << std::endl;
}

void test_attach_live_client_missing_host() {
    atlas::editor::EditorAttachProtocol proto;
    proto.Init();

    atlas::editor::AttachConfig cfg;
    cfg.mode = atlas::editor::AttachMode::LiveClient;
    bool ok = proto.Connect(cfg);
    assert(!ok);
    assert(proto.GetState() == atlas::editor::AttachState::Error);
    std::cout << "[PASS] test_attach_live_client_missing_host" << std::endl;
}

void test_attach_headless_server() {
    atlas::editor::EditorAttachProtocol proto;
    proto.Init();

    atlas::editor::AttachConfig cfg;
    cfg.mode = atlas::editor::AttachMode::HeadlessServer;
    cfg.host = "192.168.1.50";
    cfg.port = 9200;
    bool ok = proto.Connect(cfg);
    assert(ok);
    assert(proto.IsConnected());
    assert(proto.GetMode() == atlas::editor::AttachMode::HeadlessServer);
    std::cout << "[PASS] test_attach_headless_server" << std::endl;
}

void test_attach_replay() {
    atlas::editor::EditorAttachProtocol proto;
    proto.Init();

    atlas::editor::AttachConfig cfg;
    cfg.mode = atlas::editor::AttachMode::Replay;
    cfg.replayPath = "/tmp/replays/golden_01.atlasreplay";
    bool ok = proto.Connect(cfg);
    assert(ok);
    assert(proto.IsConnected());
    assert(proto.GetMode() == atlas::editor::AttachMode::Replay);
    assert(proto.TargetDescription().find("golden_01") != std::string::npos);
    std::cout << "[PASS] test_attach_replay" << std::endl;
}

void test_attach_replay_missing_path() {
    atlas::editor::EditorAttachProtocol proto;
    proto.Init();

    atlas::editor::AttachConfig cfg;
    cfg.mode = atlas::editor::AttachMode::Replay;
    bool ok = proto.Connect(cfg);
    assert(!ok);
    assert(proto.GetState() == atlas::editor::AttachState::Error);
    std::cout << "[PASS] test_attach_replay_missing_path" << std::endl;
}

void test_attach_disconnect() {
    atlas::editor::EditorAttachProtocol proto;
    proto.Init();

    atlas::editor::AttachConfig cfg;
    cfg.mode = atlas::editor::AttachMode::Standalone;
    proto.Connect(cfg);
    assert(proto.IsConnected());

    proto.Disconnect();
    assert(!proto.IsConnected());
    assert(proto.GetState() == atlas::editor::AttachState::Disconnected);
    std::cout << "[PASS] test_attach_disconnect" << std::endl;
}

// ============================================================
// Editor Self-Hosting DSL Tests
// ============================================================

void test_default_editor_dsl_parses() {
    atlas::ui::GUIDSLParser parser;
    auto result = parser.Parse(atlas::editor::DefaultEditorDSL());
    assert(result.success);
    assert(result.root != nullptr);
    assert(result.root->type == atlas::ui::DSLNodeType::Layout);
    assert(result.root->name == "DefaultEditor");
    std::cout << "[PASS] test_default_editor_dsl_parses" << std::endl;
}

void test_default_editor_dsl_has_panels() {
    atlas::ui::GUIDSLParser parser;
    auto result = parser.Parse(atlas::editor::DefaultEditorDSL());
    assert(result.success);
    assert(result.root->children.size() >= 1);
    std::cout << "[PASS] test_default_editor_dsl_has_panels" << std::endl;
}
