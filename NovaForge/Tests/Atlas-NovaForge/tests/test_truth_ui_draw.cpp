#include "../editor/panels/TruthUIPanel.h"
#include <iostream>
#include <cassert>

using namespace atlas::editor;
using namespace atlas::ui;

void test_truth_ui_draw_produces_commands() {
    TruthUIPanel panel;
    panel.SetBounds(0, 0, 400, 300);
    panel.Draw();

    auto& dl = panel.GetDrawList();
    // Should have produced multiple draw commands (background, header, text, etc.)
    assert(dl.CommandCount() > 0);
    std::cout << "[PASS] test_truth_ui_draw_produces_commands" << std::endl;
}

void test_truth_ui_draw_no_pie_shows_message() {
    TruthUIPanel panel;
    panel.SetBounds(0, 0, 400, 300);
    panel.Draw();

    auto& cmds = panel.GetDrawList().Commands();
    // Should have a text command containing "No simulation attached"
    bool found = false;
    for (const auto& cmd : cmds) {
        if (cmd.kind == UIDrawCmd::Kind::Text &&
            cmd.text.find("No simulation") != std::string::npos) {
            found = true;
            break;
        }
    }
    assert(found);
    std::cout << "[PASS] test_truth_ui_draw_no_pie_shows_message" << std::endl;
}

void test_truth_ui_draw_with_pie() {
    TruthUIPanel panel;
    PlayInEditor pie;
    panel.SetPlayInEditor(&pie);
    panel.SetBounds(0, 0, 400, 300);
    panel.Draw();

    auto& cmds = panel.GetDrawList().Commands();
    // Should have a text command containing "Mode: Stopped"
    bool foundMode = false;
    for (const auto& cmd : cmds) {
        if (cmd.kind == UIDrawCmd::Kind::Text &&
            cmd.text.find("Mode: Stopped") != std::string::npos) {
            foundMode = true;
            break;
        }
    }
    assert(foundMode);
    std::cout << "[PASS] test_truth_ui_draw_with_pie" << std::endl;
}

void test_truth_ui_draw_no_divergence() {
    TruthUIPanel panel;
    panel.SetBounds(0, 0, 400, 300);
    panel.Draw();

    auto& cmds = panel.GetDrawList().Commands();
    // Should show "No divergence"
    bool found = false;
    for (const auto& cmd : cmds) {
        if (cmd.kind == UIDrawCmd::Kind::Text &&
            cmd.text.find("No divergence") != std::string::npos) {
            found = true;
            break;
        }
    }
    assert(found);
    std::cout << "[PASS] test_truth_ui_draw_no_divergence" << std::endl;
}

void test_truth_ui_draw_clears_between_frames() {
    TruthUIPanel panel;
    panel.SetBounds(0, 0, 400, 300);

    panel.Draw();
    size_t firstCount = panel.GetDrawList().CommandCount();
    assert(firstCount > 0);

    // Draw again — should clear and regenerate
    panel.Draw();
    size_t secondCount = panel.GetDrawList().CommandCount();
    assert(secondCount == firstCount);
    std::cout << "[PASS] test_truth_ui_draw_clears_between_frames" << std::endl;
}

void test_truth_ui_draw_has_header() {
    TruthUIPanel panel;
    panel.SetBounds(10, 20, 400, 300);
    panel.Draw();

    auto& cmds = panel.GetDrawList().Commands();
    // Should have a text command with "Truth UI" header
    bool found = false;
    for (const auto& cmd : cmds) {
        if (cmd.kind == UIDrawCmd::Kind::Text &&
            cmd.text == "Truth UI") {
            found = true;
            break;
        }
    }
    assert(found);
    std::cout << "[PASS] test_truth_ui_draw_has_header" << std::endl;
}
