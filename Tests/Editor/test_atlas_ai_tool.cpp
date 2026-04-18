// Tests/Editor/test_atlas_ai_tool.cpp
// Tests for Phase 3 NF::AtlasAITool — eighth real IHostedTool.
#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/AtlasAITool.h"

using namespace NF;

// ─────────────────────────────────────────────────────────────────
// AIAssistMode helpers
// ─────────────────────────────────────────────────────────────────

TEST_CASE("aiAssistModeName returns correct strings", "[AIAssistMode]") {
    CHECK(std::string(aiAssistModeName(AIAssistMode::Chat))        == "Chat");
    CHECK(std::string(aiAssistModeName(AIAssistMode::Suggestions)) == "Suggestions");
    CHECK(std::string(aiAssistModeName(AIAssistMode::Codex))       == "Codex");
}

// ─────────────────────────────────────────────────────────────────
// AtlasAITool — descriptor
// ─────────────────────────────────────────────────────────────────

TEST_CASE("AtlasAITool descriptor is valid at construction", "[AtlasAITool][descriptor]") {
    AtlasAITool tool;
    const auto& d = tool.descriptor();
    CHECK(d.isValid());
    CHECK(d.toolId      == AtlasAITool::kToolId);
    CHECK(d.displayName == "AtlasAI");
    CHECK(d.category    == HostedToolCategory::AIAssistant);
    CHECK(d.isPrimary   == true);
    CHECK(d.acceptsProjectExtensions == false);
}

TEST_CASE("AtlasAITool toolId matches kToolId", "[AtlasAITool][descriptor]") {
    AtlasAITool tool;
    CHECK(tool.toolId() == std::string(AtlasAITool::kToolId));
    CHECK(tool.toolId() == "workspace.atlas_ai");
}

TEST_CASE("AtlasAITool declares expected shared panels", "[AtlasAITool][descriptor]") {
    AtlasAITool tool;
    const auto& panels = tool.descriptor().supportedPanels;
    REQUIRE(panels.size() >= 3);
    auto has = [&](const std::string& id) {
        for (const auto& p : panels) if (p == id) return true;
        return false;
    };
    CHECK(has("panel.ai_chat"));
    CHECK(has("panel.ai_suggestions"));
    CHECK(has("panel.console"));
}

TEST_CASE("AtlasAITool declares expected commands", "[AtlasAITool][descriptor]") {
    AtlasAITool tool;
    const auto& cmds = tool.descriptor().commands;
    REQUIRE(cmds.size() >= 5);
    auto has = [&](const std::string& id) {
        for (const auto& c : cmds) if (c == id) return true;
        return false;
    };
    CHECK(has("ai.query"));
    CHECK(has("ai.apply_suggestion"));
    CHECK(has("ai.clear_history"));
    CHECK(has("ai.toggle_inline"));
    CHECK(has("ai.promote_snippet"));
}

// ─────────────────────────────────────────────────────────────────
// AtlasAITool — lifecycle
// ─────────────────────────────────────────────────────────────────

TEST_CASE("AtlasAITool initial state is Unloaded", "[AtlasAITool][lifecycle]") {
    AtlasAITool tool;
    CHECK(tool.state() == HostedToolState::Unloaded);
}

TEST_CASE("AtlasAITool initialize transitions to Ready", "[AtlasAITool][lifecycle]") {
    AtlasAITool tool;
    CHECK(tool.initialize());
    CHECK(tool.state() == HostedToolState::Ready);
}

TEST_CASE("AtlasAITool double initialize returns false", "[AtlasAITool][lifecycle]") {
    AtlasAITool tool;
    CHECK(tool.initialize());
    CHECK_FALSE(tool.initialize());
    CHECK(tool.state() == HostedToolState::Ready);
}

TEST_CASE("AtlasAITool activate from Ready transitions to Active", "[AtlasAITool][lifecycle]") {
    AtlasAITool tool;
    tool.initialize();
    tool.activate();
    CHECK(tool.state() == HostedToolState::Active);
}

TEST_CASE("AtlasAITool suspend from Active transitions to Suspended", "[AtlasAITool][lifecycle]") {
    AtlasAITool tool;
    tool.initialize();
    tool.activate();
    tool.suspend();
    CHECK(tool.state() == HostedToolState::Suspended);
}

TEST_CASE("AtlasAITool suspend clears isProcessing flag", "[AtlasAITool][lifecycle]") {
    AtlasAITool tool;
    tool.initialize();
    tool.activate();
    tool.setProcessing(true);
    tool.suspend();
    CHECK_FALSE(tool.isProcessing());
}

TEST_CASE("AtlasAITool activate from Suspended transitions to Active", "[AtlasAITool][lifecycle]") {
    AtlasAITool tool;
    tool.initialize();
    tool.activate();
    tool.suspend();
    tool.activate();
    CHECK(tool.state() == HostedToolState::Active);
}

TEST_CASE("AtlasAITool shutdown from Active transitions to Unloaded", "[AtlasAITool][lifecycle]") {
    AtlasAITool tool;
    tool.initialize();
    tool.activate();
    tool.shutdown();
    CHECK(tool.state() == HostedToolState::Unloaded);
}

TEST_CASE("AtlasAITool update does not change state", "[AtlasAITool][lifecycle]") {
    AtlasAITool tool;
    tool.initialize();
    tool.activate();
    tool.update(0.016f);
    CHECK(tool.state() == HostedToolState::Active);
}

// ─────────────────────────────────────────────────────────────────
// AtlasAITool — assist mode
// ─────────────────────────────────────────────────────────────────

TEST_CASE("AtlasAITool default assist mode is Chat", "[AtlasAITool][mode]") {
    AtlasAITool tool;
    tool.initialize();
    CHECK(tool.assistMode() == AIAssistMode::Chat);
}

TEST_CASE("AtlasAITool setAssistMode roundtrips all modes", "[AtlasAITool][mode]") {
    AtlasAITool tool;
    tool.initialize();
    tool.setAssistMode(AIAssistMode::Suggestions);
    CHECK(tool.assistMode() == AIAssistMode::Suggestions);
    tool.setAssistMode(AIAssistMode::Codex);
    CHECK(tool.assistMode() == AIAssistMode::Codex);
    tool.setAssistMode(AIAssistMode::Chat);
    CHECK(tool.assistMode() == AIAssistMode::Chat);
}

// ─────────────────────────────────────────────────────────────────
// AtlasAITool — stats
// ─────────────────────────────────────────────────────────────────

TEST_CASE("AtlasAITool stats default to zero", "[AtlasAITool][stats]") {
    AtlasAITool tool;
    tool.initialize();
    const auto& s = tool.stats();
    CHECK(s.messageCount           == 0u);
    CHECK(s.pendingSuggestionCount == 0u);
    CHECK(s.codexSnippetCount      == 0u);
    CHECK(s.isProcessing           == false);
}

TEST_CASE("AtlasAITool setProcessing / isProcessing", "[AtlasAITool][stats]") {
    AtlasAITool tool;
    tool.initialize();
    tool.setProcessing(true);
    CHECK(tool.isProcessing());
    tool.setProcessing(false);
    CHECK_FALSE(tool.isProcessing());
}

TEST_CASE("AtlasAITool setMessageCount / messageCount", "[AtlasAITool][stats]") {
    AtlasAITool tool;
    tool.initialize();
    tool.setMessageCount(15);
    CHECK(tool.messageCount() == 15u);
}

TEST_CASE("AtlasAITool setPendingSuggestionCount / pendingSuggestionCount", "[AtlasAITool][stats]") {
    AtlasAITool tool;
    tool.initialize();
    tool.setPendingSuggestionCount(4);
    CHECK(tool.pendingSuggestionCount() == 4u);
}

TEST_CASE("AtlasAITool setCodexSnippetCount / codexSnippetCount", "[AtlasAITool][stats]") {
    AtlasAITool tool;
    tool.initialize();
    tool.setCodexSnippetCount(32);
    CHECK(tool.codexSnippetCount() == 32u);
}

TEST_CASE("AtlasAITool clearSession resets all stats", "[AtlasAITool][stats]") {
    AtlasAITool tool;
    tool.initialize();
    tool.setMessageCount(10);
    tool.setPendingSuggestionCount(3);
    tool.setCodexSnippetCount(8);
    tool.setProcessing(true);
    tool.clearSession();
    CHECK(tool.messageCount()           == 0u);
    CHECK(tool.pendingSuggestionCount() == 0u);
    CHECK(tool.codexSnippetCount()      == 0u);
    CHECK_FALSE(tool.isProcessing());
}

// ─────────────────────────────────────────────────────────────────
// AtlasAITool — project hooks
// ─────────────────────────────────────────────────────────────────

TEST_CASE("AtlasAITool onProjectLoaded clears session", "[AtlasAITool][project]") {
    AtlasAITool tool;
    tool.initialize();
    tool.setMessageCount(10);
    tool.onProjectLoaded("nova_forge");
    CHECK(tool.messageCount() == 0u);
}

TEST_CASE("AtlasAITool onProjectUnloaded clears session", "[AtlasAITool][project]") {
    AtlasAITool tool;
    tool.initialize();
    tool.setMessageCount(10);
    tool.onProjectLoaded("nova_forge");
    tool.onProjectUnloaded();
    CHECK(tool.messageCount() == 0u);
}
