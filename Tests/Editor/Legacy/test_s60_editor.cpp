#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"

using namespace NF;

// ── ConsoleMessageLevel ──────────────────────────────────────────

TEST_CASE("ConsoleMessageLevel enum values exist", "[Editor][S60]") {
    REQUIRE(ConsoleMessageLevel::Info    != ConsoleMessageLevel::Warning);
    REQUIRE(ConsoleMessageLevel::Warning != ConsoleMessageLevel::Error);
    REQUIRE(ConsoleMessageLevel::Info    != ConsoleMessageLevel::Error);
}

// ── ConsoleMessage ───────────────────────────────────────────────

TEST_CASE("ConsoleMessage defaults", "[Editor][S60]") {
    ConsoleMessage msg;
    REQUIRE(msg.text.empty());
    REQUIRE(msg.level == ConsoleMessageLevel::Info);
    REQUIRE(msg.timestamp == 0.f);
}

TEST_CASE("ConsoleMessage fields assignable", "[Editor][S60]") {
    ConsoleMessage msg;
    msg.text = "Test message";
    msg.level = ConsoleMessageLevel::Warning;
    msg.timestamp = 1.5f;
    REQUIRE(msg.text == "Test message");
    REQUIRE(msg.level == ConsoleMessageLevel::Warning);
    REQUIRE(msg.timestamp == 1.5f);
}

// ── ConsolePanel ─────────────────────────────────────────────────

TEST_CASE("ConsolePanel name is Console", "[Editor][S60]") {
    ConsolePanel panel;
    REQUIRE(panel.name() == "Console");
}

TEST_CASE("ConsolePanel slot is Bottom", "[Editor][S60]") {
    ConsolePanel panel;
    REQUIRE(panel.slot() == DockSlot::Bottom);
}

TEST_CASE("ConsolePanel starts empty", "[Editor][S60]") {
    ConsolePanel panel;
    REQUIRE(panel.messageCount() == 0);
}

TEST_CASE("ConsolePanel addMessage increases count", "[Editor][S60]") {
    ConsolePanel panel;
    panel.addMessage("Hello", ConsoleMessageLevel::Info, 0.f);
    REQUIRE(panel.messageCount() == 1);
}

TEST_CASE("ConsolePanel addMessage stores level", "[Editor][S60]") {
    ConsolePanel panel;
    panel.addMessage("warn", ConsoleMessageLevel::Warning, 1.f);
    REQUIRE(panel.messages()[0].level == ConsoleMessageLevel::Warning);
    REQUIRE(panel.messages()[0].text == "warn");
}

TEST_CASE("ConsolePanel clearMessages empties buffer", "[Editor][S60]") {
    ConsolePanel panel;
    panel.addMessage("a", ConsoleMessageLevel::Info, 0.f);
    panel.addMessage("b", ConsoleMessageLevel::Error, 0.f);
    panel.clearMessages();
    REQUIRE(panel.messageCount() == 0);
}

TEST_CASE("ConsolePanel addLogMessage info level", "[Editor][S60]") {
    ConsolePanel panel;
    panel.addLogMessage(LogLevel::Info, "Core", "Initialized");
    REQUIRE(panel.messageCount() == 1);
    REQUIRE(panel.messages()[0].level == ConsoleMessageLevel::Info);
    REQUIRE(panel.messages()[0].text.find("Core") != std::string::npos);
}

TEST_CASE("ConsolePanel addLogMessage warn level maps to Warning", "[Editor][S60]") {
    ConsolePanel panel;
    panel.addLogMessage(LogLevel::Warn, "Editor", "Missing asset");
    REQUIRE(panel.messages()[0].level == ConsoleMessageLevel::Warning);
}

TEST_CASE("ConsolePanel addLogMessage error level maps to Error", "[Editor][S60]") {
    ConsolePanel panel;
    panel.addLogMessage(LogLevel::Error, "Renderer", "Shader failed");
    REQUIRE(panel.messages()[0].level == ConsoleMessageLevel::Error);
}

TEST_CASE("ConsolePanel kMaxMessages limits buffer", "[Editor][S60]") {
    ConsolePanel panel;
    for (size_t i = 0; i <= ConsolePanel::kMaxMessages + 5; ++i) {
        panel.addMessage("msg", ConsoleMessageLevel::Info, 0.f);
    }
    REQUIRE(panel.messageCount() == ConsolePanel::kMaxMessages);
}

TEST_CASE("ConsolePanel is an EditorPanel", "[Editor][S60]") {
    ConsolePanel panel;
    EditorPanel* base = &panel;
    REQUIRE(base->name() == "Console");
}

// ── ToolbarItem ──────────────────────────────────────────────────

TEST_CASE("ToolbarItem defaults", "[Editor][S60]") {
    ToolbarItem item;
    REQUIRE(item.name.empty());
    REQUIRE(item.icon.empty());
    REQUIRE(item.enabled);
    REQUIRE_FALSE(item.isSeparator);
}

// ── EditorToolbar ────────────────────────────────────────────────

TEST_CASE("EditorToolbar starts empty", "[Editor][S60]") {
    EditorToolbar tb;
    REQUIRE(tb.itemCount() == 0);
}

TEST_CASE("EditorToolbar addItem increases count", "[Editor][S60]") {
    EditorToolbar tb;
    tb.addItem("Play", "play_icon", "Play the game", [] {});
    REQUIRE(tb.itemCount() == 1);
}

TEST_CASE("EditorToolbar addItem stores name and icon", "[Editor][S60]") {
    EditorToolbar tb;
    tb.addItem("Stop", "stop_icon", "Stop", [] {});
    REQUIRE(tb.items()[0].name == "Stop");
    REQUIRE(tb.items()[0].icon == "stop_icon");
    REQUIRE(tb.items()[0].tooltip == "Stop");
    REQUIRE_FALSE(tb.items()[0].isSeparator);
}

TEST_CASE("EditorToolbar addItem disabled flag", "[Editor][S60]") {
    EditorToolbar tb;
    tb.addItem("Pause", "pause_icon", "Pause", [] {}, false);
    REQUIRE_FALSE(tb.items()[0].enabled);
}

TEST_CASE("EditorToolbar addSeparator marks item as separator", "[Editor][S60]") {
    EditorToolbar tb;
    tb.addItem("Play", "play_icon", "Play", [] {});
    tb.addSeparator();
    tb.addItem("Stop", "stop_icon", "Stop", [] {});
    REQUIRE(tb.itemCount() == 3);
    REQUIRE(tb.items()[1].isSeparator);
    REQUIRE_FALSE(tb.items()[0].isSeparator);
    REQUIRE_FALSE(tb.items()[2].isSeparator);
}

TEST_CASE("EditorToolbar action is callable", "[Editor][S60]") {
    EditorToolbar tb;
    int callCount = 0;
    tb.addItem("Test", "", "", [&callCount] { ++callCount; });
    tb.items()[0].action();
    REQUIRE(callCount == 1);
}

// ── ContentViewMode ──────────────────────────────────────────────

TEST_CASE("ContentViewMode enum values exist", "[Editor][S60]") {
    REQUIRE(ContentViewMode::Grid != ContentViewMode::List);
}

// ── ContentBrowserPanel ──────────────────────────────────────────

TEST_CASE("ContentBrowserPanel name is ContentBrowser", "[Editor][S60]") {
    ContentBrowserPanel panel;
    REQUIRE(panel.name() == "ContentBrowser");
}

TEST_CASE("ContentBrowserPanel slot is Left", "[Editor][S60]") {
    ContentBrowserPanel panel;
    REQUIRE(panel.slot() == DockSlot::Left);
}

TEST_CASE("ContentBrowserPanel default view mode is Grid", "[Editor][S60]") {
    ContentBrowserPanel panel;
    REQUIRE(panel.viewMode() == ContentViewMode::Grid);
}

TEST_CASE("ContentBrowserPanel setViewMode updates mode", "[Editor][S60]") {
    ContentBrowserPanel panel;
    panel.setViewMode(ContentViewMode::List);
    REQUIRE(panel.viewMode() == ContentViewMode::List);
}

TEST_CASE("ContentBrowserPanel starts with no content browser", "[Editor][S60]") {
    ContentBrowserPanel panel;
    REQUIRE(panel.contentBrowser() == nullptr);
}

TEST_CASE("ContentBrowserPanel setContentBrowser stores pointer", "[Editor][S60]") {
    ContentBrowserPanel panel;
    ContentBrowser cb;
    panel.setContentBrowser(&cb);
    REQUIRE(panel.contentBrowser() == &cb);
}

TEST_CASE("ContentBrowserPanel constructor with browser pointer", "[Editor][S60]") {
    ContentBrowser cb;
    ContentBrowserPanel panel(&cb);
    REQUIRE(panel.contentBrowser() == &cb);
}

TEST_CASE("ContentBrowserPanel is an EditorPanel", "[Editor][S60]") {
    ContentBrowserPanel panel;
    EditorPanel* base = &panel;
    REQUIRE(base->name() == "ContentBrowser");
}
