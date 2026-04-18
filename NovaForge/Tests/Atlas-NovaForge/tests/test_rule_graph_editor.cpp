#include "../editor/panels/RuleGraphEditorPanel.h"
#include <iostream>
#include <cassert>
#include <cstring>

using namespace atlas::editor;

void test_rule_graph_editor_name() {
    RuleGraphEditorPanel panel;
    assert(std::strcmp(panel.Name(), "Rule Graph Editor") == 0);
    std::cout << "[PASS] test_rule_graph_editor_name" << std::endl;
}

void test_rule_graph_editor_add_node() {
    RuleGraphEditorPanel panel;
    assert(panel.NodeCount() == 0);

    auto id = panel.AddNode("TileInput", 10.0f, 20.0f);
    assert(id > 0);
    assert(panel.NodeCount() == 1);

    auto* node = panel.GetNode(id);
    assert(node != nullptr);
    assert(node->label == "TileInput");
    assert(node->x == 10.0f);
    assert(node->y == 20.0f);
    std::cout << "[PASS] test_rule_graph_editor_add_node" << std::endl;
}

void test_rule_graph_editor_remove_node() {
    RuleGraphEditorPanel panel;
    auto id = panel.AddNode("Test", 0.0f, 0.0f);
    assert(panel.NodeCount() == 1);

    panel.RemoveNode(id);
    assert(panel.NodeCount() == 0);
    assert(panel.GetNode(id) == nullptr);
    std::cout << "[PASS] test_rule_graph_editor_remove_node" << std::endl;
}

void test_rule_graph_editor_add_wire() {
    RuleGraphEditorPanel panel;
    auto id1 = panel.AddNode("A", 0, 0);
    auto id2 = panel.AddNode("B", 200, 0);

    panel.AddWire(id1, 0, id2, 0);
    assert(panel.WireCount() == 1);

    auto& wires = panel.Wires();
    assert(wires[0].fromNode == id1);
    assert(wires[0].toNode == id2);
    std::cout << "[PASS] test_rule_graph_editor_add_wire" << std::endl;
}

void test_rule_graph_editor_remove_wires_for_node() {
    RuleGraphEditorPanel panel;
    auto id1 = panel.AddNode("A", 0, 0);
    auto id2 = panel.AddNode("B", 200, 0);
    auto id3 = panel.AddNode("C", 400, 0);

    panel.AddWire(id1, 0, id2, 0);
    panel.AddWire(id2, 0, id3, 0);
    assert(panel.WireCount() == 2);

    panel.RemoveWiresForNode(id2);
    assert(panel.WireCount() == 0);
    std::cout << "[PASS] test_rule_graph_editor_remove_wires_for_node" << std::endl;
}

void test_rule_graph_editor_remove_node_removes_wires() {
    RuleGraphEditorPanel panel;
    auto id1 = panel.AddNode("A", 0, 0);
    auto id2 = panel.AddNode("B", 200, 0);

    panel.AddWire(id1, 0, id2, 0);
    assert(panel.WireCount() == 1);

    panel.RemoveNode(id1);
    assert(panel.WireCount() == 0);
    std::cout << "[PASS] test_rule_graph_editor_remove_node_removes_wires" << std::endl;
}

void test_rule_graph_editor_selection() {
    RuleGraphEditorPanel panel;
    auto id1 = panel.AddNode("A", 0, 0);
    auto id2 = panel.AddNode("B", 200, 0);

    assert(panel.SelectedNodeId() == 0);
    panel.SelectNode(id1);
    assert(panel.SelectedNodeId() == id1);
    assert(panel.GetNode(id1)->selected);
    assert(!panel.GetNode(id2)->selected);

    panel.SelectNode(id2);
    assert(panel.SelectedNodeId() == id2);
    assert(!panel.GetNode(id1)->selected);
    assert(panel.GetNode(id2)->selected);

    panel.ClearSelection();
    assert(panel.SelectedNodeId() == 0);
    std::cout << "[PASS] test_rule_graph_editor_selection" << std::endl;
}

void test_rule_graph_editor_pan_zoom() {
    RuleGraphEditorPanel panel;
    assert(panel.PanX() == 0.0f);
    assert(panel.PanY() == 0.0f);
    assert(panel.Zoom() == 1.0f);

    panel.SetPan(100.0f, 50.0f);
    assert(panel.PanX() == 100.0f);
    assert(panel.PanY() == 50.0f);

    panel.SetZoom(2.0f);
    assert(panel.Zoom() == 2.0f);

    // Zoom clamp
    panel.SetZoom(0.05f);
    assert(panel.Zoom() == 2.0f);  // unchanged, too small
    panel.SetZoom(11.0f);
    assert(panel.Zoom() == 2.0f);  // unchanged, too large
    std::cout << "[PASS] test_rule_graph_editor_pan_zoom" << std::endl;
}

void test_rule_graph_editor_debug_mode() {
    RuleGraphEditorPanel panel;
    assert(!panel.IsDebugMode());
    panel.SetDebugMode(true);
    assert(panel.IsDebugMode());
    std::cout << "[PASS] test_rule_graph_editor_debug_mode" << std::endl;
}

void test_rule_graph_editor_draw() {
    RuleGraphEditorPanel panel;
    auto id1 = panel.AddNode("A", 0, 0);
    auto id2 = panel.AddNode("B", 200, 0);
    panel.AddWire(id1, 0, id2, 0);

    panel.Draw();
    assert(panel.GetDrawList().CommandCount() > 0);
    std::cout << "[PASS] test_rule_graph_editor_draw" << std::endl;
}

void test_rule_graph_editor_draw_debug() {
    RuleGraphEditorPanel panel;
    panel.AddNode("A", 0, 0);
    panel.SetDebugMode(true);

    panel.Draw();
    // Debug mode should add extra draw commands
    size_t debugCount = panel.GetDrawList().CommandCount();
    assert(debugCount > 0);
    std::cout << "[PASS] test_rule_graph_editor_draw_debug" << std::endl;
}

void test_rule_graph_editor_draw_with_selection() {
    RuleGraphEditorPanel panel;
    auto id = panel.AddNode("Selected", 0, 0);
    panel.SelectNode(id);

    panel.Draw();
    assert(panel.GetDrawList().CommandCount() > 0);
    std::cout << "[PASS] test_rule_graph_editor_draw_with_selection" << std::endl;
}
