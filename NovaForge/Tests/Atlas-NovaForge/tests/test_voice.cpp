#include "../engine/voice/VoiceCommand.h"
#include <iostream>
#include <cassert>

void test_voice_register_command() {
    atlas::voice::VoiceCommandRegistry registry;
    registry.Register({"select all", "select_units", "game"});
    assert(registry.CommandCount() == 1);
    std::cout << "[PASS] test_voice_register_command" << std::endl;
}

void test_voice_match_command() {
    atlas::voice::VoiceCommandRegistry registry;
    registry.Register({"select all fighters", "select_units", "game"});
    registry.Register({"pause", "pause_game", "both"});

    auto* match = registry.Match("select all fighters");
    assert(match != nullptr);
    assert(match->intentName == "select_units");
    std::cout << "[PASS] test_voice_match_command" << std::endl;
}

void test_voice_no_match() {
    atlas::voice::VoiceCommandRegistry registry;
    registry.Register({"attack", "attack_target", "game"});
    auto* match = registry.Match("retreat");
    assert(match == nullptr);
    std::cout << "[PASS] test_voice_no_match" << std::endl;
}

void test_voice_get_by_context() {
    atlas::voice::VoiceCommandRegistry registry;
    registry.Register({"select", "select_units", "game"});
    registry.Register({"compile", "compile_graph", "editor"});
    registry.Register({"pause", "pause_sim", "both"});

    auto game = registry.GetByContext("game");
    assert(game.size() == 2); // "game" + "both"

    auto editor = registry.GetByContext("editor");
    assert(editor.size() == 2); // "editor" + "both"
    std::cout << "[PASS] test_voice_get_by_context" << std::endl;
}
