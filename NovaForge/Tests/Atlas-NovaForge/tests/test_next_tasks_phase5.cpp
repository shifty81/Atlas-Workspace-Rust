#include <iostream>
#include <cassert>
#include <cstdio>
#include <string>
#include <vector>

#include "../engine/render/VulkanRenderer.h"
#include "../editor/panels/QuestEditorPanel.h"
#include "../editor/panels/InventoryEditorPanel.h"
#include "../editor/ai/AIAggregator.h"

// ============================================================
// Task 1: VulkanRenderer draw command recording
// ============================================================

void test_vulkan_begin_end_frame() {
    atlas::render::VulkanRenderer renderer;
    assert(!renderer.IsFrameActive());
    assert(renderer.FrameCount() == 0);
    assert(renderer.DrawCommandCount() == 0);

    renderer.BeginFrame();
    assert(renderer.IsFrameActive());

    renderer.EndFrame();
    assert(!renderer.IsFrameActive());
    assert(renderer.FrameCount() == 1);

    std::cout << "[PASS] test_vulkan_begin_end_frame" << std::endl;
}

void test_vulkan_draw_rect() {
    atlas::render::VulkanRenderer renderer;
    renderer.BeginFrame();
    renderer.DrawRect({10, 20, 100, 50}, {255, 0, 0, 255});
    assert(renderer.DrawCommandCount() == 1);
    assert(renderer.DrawCommands()[0].kind == atlas::render::VkDrawCommand::Kind::Rect);
    assert(renderer.DrawCommands()[0].rect.x == 10);
    assert(renderer.DrawCommands()[0].color.r == 255);
    renderer.EndFrame();

    std::cout << "[PASS] test_vulkan_draw_rect" << std::endl;
}

void test_vulkan_draw_text() {
    atlas::render::VulkanRenderer renderer;
    renderer.BeginFrame();
    renderer.DrawText({0, 0, 200, 20}, "Hello", {200, 200, 200, 255});
    assert(renderer.DrawCommandCount() == 1);
    assert(renderer.DrawCommands()[0].kind == atlas::render::VkDrawCommand::Kind::Text);
    assert(renderer.DrawCommands()[0].text == "Hello");
    renderer.EndFrame();

    std::cout << "[PASS] test_vulkan_draw_text" << std::endl;
}

void test_vulkan_draw_icon() {
    atlas::render::VulkanRenderer renderer;
    renderer.BeginFrame();
    renderer.DrawIcon({0, 0, 32, 32}, 42, {255, 255, 255, 255});
    assert(renderer.DrawCommandCount() == 1);
    assert(renderer.DrawCommands()[0].kind == atlas::render::VkDrawCommand::Kind::Icon);
    assert(renderer.DrawCommands()[0].resourceId == 42);
    renderer.EndFrame();

    std::cout << "[PASS] test_vulkan_draw_icon" << std::endl;
}

void test_vulkan_draw_border() {
    atlas::render::VulkanRenderer renderer;
    renderer.BeginFrame();
    renderer.DrawBorder({5, 5, 200, 100}, 2, {128, 128, 128, 255});
    assert(renderer.DrawCommandCount() == 1);
    assert(renderer.DrawCommands()[0].kind == atlas::render::VkDrawCommand::Kind::Border);
    assert(renderer.DrawCommands()[0].thickness == 2);
    renderer.EndFrame();

    std::cout << "[PASS] test_vulkan_draw_border" << std::endl;
}

void test_vulkan_draw_image() {
    atlas::render::VulkanRenderer renderer;
    renderer.BeginFrame();
    renderer.DrawImage({0, 0, 128, 128}, 7, {255, 255, 255, 128});
    assert(renderer.DrawCommandCount() == 1);
    assert(renderer.DrawCommands()[0].kind == atlas::render::VkDrawCommand::Kind::Image);
    assert(renderer.DrawCommands()[0].resourceId == 7);
    assert(renderer.DrawCommands()[0].color.a == 128);
    renderer.EndFrame();

    std::cout << "[PASS] test_vulkan_draw_image" << std::endl;
}

void test_vulkan_multiple_commands() {
    atlas::render::VulkanRenderer renderer;
    renderer.BeginFrame();
    renderer.DrawRect({0, 0, 100, 100}, {255, 0, 0, 255});
    renderer.DrawText({0, 0, 100, 20}, "test", {200, 200, 200, 255});
    renderer.DrawIcon({0, 0, 16, 16}, 1, {255, 255, 255, 255});
    renderer.DrawBorder({0, 0, 100, 100}, 1, {100, 100, 100, 255});
    renderer.DrawImage({0, 0, 64, 64}, 2, {255, 255, 255, 255});
    assert(renderer.DrawCommandCount() == 5);
    renderer.EndFrame();

    std::cout << "[PASS] test_vulkan_multiple_commands" << std::endl;
}

void test_vulkan_frame_clears_commands() {
    atlas::render::VulkanRenderer renderer;
    renderer.BeginFrame();
    renderer.DrawRect({0, 0, 10, 10}, {255, 0, 0, 255});
    assert(renderer.DrawCommandCount() == 1);
    renderer.EndFrame();

    renderer.BeginFrame();
    assert(renderer.DrawCommandCount() == 0);
    renderer.EndFrame();
    assert(renderer.FrameCount() == 2);

    std::cout << "[PASS] test_vulkan_frame_clears_commands" << std::endl;
}

// ============================================================
// Task 2: QuestEditorPanel
// ============================================================

void test_quest_panel_name() {
    atlas::editor::QuestEditorPanel panel;
    assert(std::string(panel.Name()) == "Quest Editor");
    std::cout << "[PASS] test_quest_panel_name" << std::endl;
}

void test_quest_panel_add_node() {
    atlas::editor::QuestEditorPanel panel;
    assert(panel.NodeCount() == 0);

    atlas::editor::QuestNode node;
    node.name = "Find the key";
    node.nodeType = "objective";
    uint64_t id = panel.AddNode(node);
    assert(id > 0);
    assert(panel.NodeCount() == 1);

    auto* n = panel.GetNode(id);
    assert(n != nullptr);
    assert(n->name == "Find the key");
    assert(n->nodeType == "objective");

    std::cout << "[PASS] test_quest_panel_add_node" << std::endl;
}

void test_quest_panel_remove_node() {
    atlas::editor::QuestEditorPanel panel;
    atlas::editor::QuestNode node;
    node.name = "Test";
    node.nodeType = "dialogue";
    uint64_t id = panel.AddNode(node);

    assert(panel.RemoveNode(id));
    assert(panel.NodeCount() == 0);
    assert(!panel.RemoveNode(9999));

    std::cout << "[PASS] test_quest_panel_remove_node" << std::endl;
}

void test_quest_panel_connections() {
    atlas::editor::QuestEditorPanel panel;

    atlas::editor::QuestNode n1;
    n1.name = "Start";
    n1.nodeType = "dialogue";
    uint64_t id1 = panel.AddNode(n1);

    atlas::editor::QuestNode n2;
    n2.name = "End";
    n2.nodeType = "reward";
    uint64_t id2 = panel.AddNode(n2);

    assert(panel.ConnectNodes(id1, id2));
    auto conns = panel.GetConnections(id1);
    assert(conns.size() == 1);
    assert(conns[0] == id2);

    // Duplicate connection should fail
    assert(!panel.ConnectNodes(id1, id2));

    // Connect to non-existent node should fail
    assert(!panel.ConnectNodes(id1, 9999));

    assert(panel.DisconnectNodes(id1, id2));
    assert(panel.GetConnections(id1).empty());

    // Disconnect non-existent should fail
    assert(!panel.DisconnectNodes(id1, id2));

    std::cout << "[PASS] test_quest_panel_connections" << std::endl;
}

void test_quest_panel_remove_cleans_connections() {
    atlas::editor::QuestEditorPanel panel;

    atlas::editor::QuestNode n1, n2, n3;
    n1.name = "A"; n1.nodeType = "objective";
    n2.name = "B"; n2.nodeType = "branch";
    n3.name = "C"; n3.nodeType = "reward";
    uint64_t id1 = panel.AddNode(n1);
    uint64_t id2 = panel.AddNode(n2);
    uint64_t id3 = panel.AddNode(n3);

    panel.ConnectNodes(id1, id2);
    panel.ConnectNodes(id1, id3);
    panel.ConnectNodes(id2, id3);

    // Removing node 3 should remove references from nodes 1 and 2
    panel.RemoveNode(id3);
    assert(panel.GetConnections(id1).size() == 1);
    assert(panel.GetConnections(id1)[0] == id2);
    assert(panel.GetConnections(id2).empty());

    std::cout << "[PASS] test_quest_panel_remove_cleans_connections" << std::endl;
}

void test_quest_panel_selection() {
    atlas::editor::QuestEditorPanel panel;
    assert(panel.SelectedNodeID() == 0);

    atlas::editor::QuestNode node;
    node.name = "Test";
    node.nodeType = "objective";
    uint64_t id = panel.AddNode(node);

    panel.SelectNode(id);
    assert(panel.SelectedNodeID() == id);

    // Removing selected node resets selection
    panel.RemoveNode(id);
    assert(panel.SelectedNodeID() == 0);

    std::cout << "[PASS] test_quest_panel_selection" << std::endl;
}

void test_quest_panel_preview_mode() {
    atlas::editor::QuestEditorPanel panel;
    assert(!panel.IsPreviewMode());
    panel.SetPreviewMode(true);
    assert(panel.IsPreviewMode());
    panel.SetPreviewMode(false);
    assert(!panel.IsPreviewMode());

    std::cout << "[PASS] test_quest_panel_preview_mode" << std::endl;
}

void test_quest_panel_export() {
    atlas::editor::QuestEditorPanel panel;
    atlas::editor::QuestNode node;
    node.name = "FindKey";
    node.nodeType = "objective";
    node.completed = true;
    panel.AddNode(node);

    auto json = panel.ExportQuest();
    assert(json.find("FindKey") != std::string::npos);
    assert(json.find("objective") != std::string::npos);
    assert(json.find("true") != std::string::npos);

    std::cout << "[PASS] test_quest_panel_export" << std::endl;
}

void test_quest_panel_import() {
    atlas::editor::QuestEditorPanel panel;
    assert(panel.ImportQuest("{\"nodes\":[]}"));
    assert(!panel.ImportQuest(""));
    assert(!panel.ImportQuest("{\"invalid\":true}"));

    std::cout << "[PASS] test_quest_panel_import" << std::endl;
}

void test_quest_panel_clear() {
    atlas::editor::QuestEditorPanel panel;
    atlas::editor::QuestNode node;
    node.name = "Test";
    node.nodeType = "objective";
    panel.AddNode(node);
    panel.SetPreviewMode(true);
    panel.SelectNode(1);

    panel.Clear();
    assert(panel.NodeCount() == 0);
    assert(panel.SelectedNodeID() == 0);
    assert(!panel.IsPreviewMode());

    std::cout << "[PASS] test_quest_panel_clear" << std::endl;
}

void test_quest_panel_draw() {
    atlas::editor::QuestEditorPanel panel;

    atlas::editor::QuestNode n1;
    n1.name = "Start";
    n1.nodeType = "dialogue";
    n1.completed = true;
    panel.AddNode(n1);

    atlas::editor::QuestNode n2;
    n2.name = "End";
    n2.nodeType = "reward";
    panel.AddNode(n2);

    panel.Draw();
    assert(panel.GetDrawList().CommandCount() > 0);

    std::cout << "[PASS] test_quest_panel_draw" << std::endl;
}

void test_quest_panel_draw_preview() {
    atlas::editor::QuestEditorPanel panel;
    panel.SetPreviewMode(true);
    panel.Draw();
    // Should have at least the background + title
    assert(panel.GetDrawList().CommandCount() >= 2);

    std::cout << "[PASS] test_quest_panel_draw_preview" << std::endl;
}

// ============================================================
// Task 3: InventoryEditorPanel
// ============================================================

void test_inventory_panel_name() {
    atlas::editor::InventoryEditorPanel panel;
    assert(std::string(panel.Name()) == "Inventory Editor");
    std::cout << "[PASS] test_inventory_panel_name" << std::endl;
}

void test_inventory_panel_add_item() {
    atlas::editor::InventoryEditorPanel panel;
    assert(panel.ItemCount() == 0);

    atlas::editor::InventoryItem item;
    item.name = "Iron Sword";
    item.category = "weapon";
    item.stackSize = 1;
    item.maxStack = 1;
    item.weight = 3.5f;
    uint64_t id = panel.AddItem(item);
    assert(id > 0);
    assert(panel.ItemCount() == 1);

    auto* i = panel.GetItem(id);
    assert(i != nullptr);
    assert(i->name == "Iron Sword");
    assert(i->category == "weapon");
    assert(i->weight == 3.5f);

    std::cout << "[PASS] test_inventory_panel_add_item" << std::endl;
}

void test_inventory_panel_remove_item() {
    atlas::editor::InventoryEditorPanel panel;
    atlas::editor::InventoryItem item;
    item.name = "Potion";
    item.category = "consumable";
    uint64_t id = panel.AddItem(item);

    assert(panel.RemoveItem(id));
    assert(panel.ItemCount() == 0);
    assert(!panel.RemoveItem(9999));

    std::cout << "[PASS] test_inventory_panel_remove_item" << std::endl;
}

void test_inventory_panel_selection() {
    atlas::editor::InventoryEditorPanel panel;
    assert(panel.SelectedItemID() == 0);

    atlas::editor::InventoryItem item;
    item.name = "Shield";
    item.category = "armor";
    uint64_t id = panel.AddItem(item);

    panel.SelectItem(id);
    assert(panel.SelectedItemID() == id);

    panel.RemoveItem(id);
    assert(panel.SelectedItemID() == 0);

    std::cout << "[PASS] test_inventory_panel_selection" << std::endl;
}

void test_inventory_panel_filter() {
    atlas::editor::InventoryEditorPanel panel;

    atlas::editor::InventoryItem i1;
    i1.name = "Sword"; i1.category = "weapon";
    panel.AddItem(i1);

    atlas::editor::InventoryItem i2;
    i2.name = "Potion"; i2.category = "consumable";
    panel.AddItem(i2);

    atlas::editor::InventoryItem i3;
    i3.name = "Axe"; i3.category = "weapon";
    panel.AddItem(i3);

    assert(panel.FilterCategory().empty());
    assert(panel.FilteredItems().size() == 3);

    panel.SetFilterCategory("weapon");
    assert(panel.FilterCategory() == "weapon");
    assert(panel.FilteredItems().size() == 2);

    panel.SetFilterCategory("consumable");
    assert(panel.FilteredItems().size() == 1);
    assert(panel.FilteredItems()[0].name == "Potion");

    panel.SetFilterCategory("");
    assert(panel.FilteredItems().size() == 3);

    std::cout << "[PASS] test_inventory_panel_filter" << std::endl;
}

void test_inventory_panel_sort_by_name() {
    atlas::editor::InventoryEditorPanel panel;

    atlas::editor::InventoryItem i1, i2, i3;
    i1.name = "Zephyr"; i1.category = "weapon";
    i2.name = "Axe"; i2.category = "weapon";
    i3.name = "Mace"; i3.category = "weapon";
    panel.AddItem(i1);
    panel.AddItem(i2);
    panel.AddItem(i3);

    panel.SortByName();
    auto items = panel.ListItems();
    assert(items[0].name == "Axe");
    assert(items[1].name == "Mace");
    assert(items[2].name == "Zephyr");

    std::cout << "[PASS] test_inventory_panel_sort_by_name" << std::endl;
}

void test_inventory_panel_sort_by_category() {
    atlas::editor::InventoryEditorPanel panel;

    atlas::editor::InventoryItem i1, i2, i3;
    i1.name = "Sword"; i1.category = "weapon";
    i2.name = "Potion"; i2.category = "consumable";
    i3.name = "Shield"; i3.category = "armor";
    panel.AddItem(i1);
    panel.AddItem(i2);
    panel.AddItem(i3);

    panel.SortByCategory();
    auto items = panel.ListItems();
    assert(items[0].category == "armor");
    assert(items[1].category == "consumable");
    assert(items[2].category == "weapon");

    std::cout << "[PASS] test_inventory_panel_sort_by_category" << std::endl;
}

void test_inventory_panel_export() {
    atlas::editor::InventoryEditorPanel panel;
    atlas::editor::InventoryItem item;
    item.name = "Gem";
    item.category = "material";
    item.stackSize = 5;
    item.maxStack = 50;
    panel.AddItem(item);

    auto json = panel.ExportInventory();
    assert(json.find("Gem") != std::string::npos);
    assert(json.find("material") != std::string::npos);

    std::cout << "[PASS] test_inventory_panel_export" << std::endl;
}

void test_inventory_panel_import() {
    atlas::editor::InventoryEditorPanel panel;
    assert(panel.ImportInventory("{\"items\":[]}"));
    assert(!panel.ImportInventory(""));
    assert(!panel.ImportInventory("{\"invalid\":true}"));

    std::cout << "[PASS] test_inventory_panel_import" << std::endl;
}

void test_inventory_panel_clear() {
    atlas::editor::InventoryEditorPanel panel;
    atlas::editor::InventoryItem item;
    item.name = "Test";
    item.category = "quest";
    panel.AddItem(item);
    panel.SelectItem(1);
    panel.SetFilterCategory("quest");

    panel.Clear();
    assert(panel.ItemCount() == 0);
    assert(panel.SelectedItemID() == 0);
    assert(panel.FilterCategory().empty());

    std::cout << "[PASS] test_inventory_panel_clear" << std::endl;
}

void test_inventory_panel_draw() {
    atlas::editor::InventoryEditorPanel panel;

    atlas::editor::InventoryItem item;
    item.name = "Sword";
    item.category = "weapon";
    item.stackSize = 1;
    item.maxStack = 1;
    panel.AddItem(item);

    panel.Draw();
    assert(panel.GetDrawList().CommandCount() > 0);

    std::cout << "[PASS] test_inventory_panel_draw" << std::endl;
}

void test_inventory_panel_draw_with_filter() {
    atlas::editor::InventoryEditorPanel panel;
    panel.SetFilterCategory("armor");
    panel.Draw();
    assert(panel.GetDrawList().CommandCount() >= 2);

    std::cout << "[PASS] test_inventory_panel_draw_with_filter" << std::endl;
}

// ============================================================
// Task 4: AIAggregator
// ============================================================

namespace {

class MockAIBackend : public atlas::ai::AIBackend {
public:
    explicit MockAIBackend(float conf, const std::string& resp)
        : m_confidence(conf), m_response(resp) {}

    atlas::ai::AggregatorResponse Query(
        const std::string& prompt,
        const atlas::ai::AIContext& /*context*/) override {
        m_lastPrompt = prompt;
        return atlas::ai::AggregatorResponse{m_response, m_confidence};
    }

    std::string LastPrompt() const { return m_lastPrompt; }

private:
    float m_confidence;
    std::string m_response;
    std::string m_lastPrompt;
};

class EmptyAIBackend : public atlas::ai::AIBackend {
public:
    atlas::ai::AggregatorResponse Query(
        const std::string& /*prompt*/,
        const atlas::ai::AIContext& /*context*/) override {
        return atlas::ai::AggregatorResponse{"", 0.0f};
    }
};

} // anonymous namespace

void test_ai_aggregator_empty() {
    atlas::ai::AIAggregator agg;
    assert(agg.BackendCount() == 0);

    atlas::ai::AIContext ctx;
    auto resp = agg.Execute(atlas::ai::AIRequestType::Analysis, "test", ctx);
    assert(resp.content.empty());
    assert(resp.confidence == 0.0f);

    std::cout << "[PASS] test_ai_aggregator_empty" << std::endl;
}

void test_ai_aggregator_register() {
    atlas::ai::AIAggregator agg;
    MockAIBackend backend(0.9f, "hello");
    agg.RegisterBackend(&backend);
    assert(agg.BackendCount() == 1);

    agg.RegisterBackend(nullptr);
    assert(agg.BackendCount() == 1);

    std::cout << "[PASS] test_ai_aggregator_register" << std::endl;
}

void test_ai_aggregator_execute_single() {
    atlas::ai::AIAggregator agg;
    MockAIBackend backend(0.8f, "generated graph");
    agg.RegisterBackend(&backend);

    atlas::ai::AIContext ctx;
    ctx.projectName = "TestProject";
    auto resp = agg.Execute(atlas::ai::AIRequestType::GraphGeneration,
                            "create a node", ctx);

    assert(resp.content == "generated graph");
    assert(resp.confidence == 0.8f);
    assert(backend.LastPrompt().find("[GraphGeneration]") != std::string::npos);
    assert(backend.LastPrompt().find("create a node") != std::string::npos);

    std::cout << "[PASS] test_ai_aggregator_execute_single" << std::endl;
}

void test_ai_aggregator_selects_best() {
    atlas::ai::AIAggregator agg;
    MockAIBackend low(0.3f, "low quality");
    MockAIBackend high(0.95f, "high quality");
    MockAIBackend mid(0.6f, "mid quality");
    agg.RegisterBackend(&low);
    agg.RegisterBackend(&high);
    agg.RegisterBackend(&mid);

    atlas::ai::AIContext ctx;
    auto resp = agg.Execute(atlas::ai::AIRequestType::CodeAssist,
                            "fix bug", ctx);

    assert(resp.content == "high quality");
    assert(resp.confidence == 0.95f);

    std::cout << "[PASS] test_ai_aggregator_selects_best" << std::endl;
}

void test_ai_aggregator_skips_empty_responses() {
    atlas::ai::AIAggregator agg;
    EmptyAIBackend empty;
    MockAIBackend valid(0.5f, "valid response");
    agg.RegisterBackend(&empty);
    agg.RegisterBackend(&valid);

    atlas::ai::AIContext ctx;
    auto resp = agg.Execute(atlas::ai::AIRequestType::WorldGeneration,
                            "generate terrain", ctx);

    assert(resp.content == "valid response");
    assert(resp.confidence == 0.5f);

    std::cout << "[PASS] test_ai_aggregator_skips_empty_responses" << std::endl;
}

void test_ai_aggregator_all_empty_responses() {
    atlas::ai::AIAggregator agg;
    EmptyAIBackend e1, e2;
    agg.RegisterBackend(&e1);
    agg.RegisterBackend(&e2);

    atlas::ai::AIContext ctx;
    auto resp = agg.Execute(atlas::ai::AIRequestType::Analysis,
                            "analyze", ctx);

    assert(resp.content.empty());
    assert(resp.confidence == 0.0f);

    std::cout << "[PASS] test_ai_aggregator_all_empty_responses" << std::endl;
}

void test_ai_aggregator_request_type_prefix() {
    atlas::ai::AIAggregator agg;
    MockAIBackend backend(1.0f, "ok");
    agg.RegisterBackend(&backend);

    atlas::ai::AIContext ctx;

    agg.Execute(atlas::ai::AIRequestType::WorldGeneration, "test", ctx);
    assert(backend.LastPrompt().find("[WorldGeneration]") != std::string::npos);

    agg.Execute(atlas::ai::AIRequestType::CodeAssist, "test", ctx);
    assert(backend.LastPrompt().find("[CodeAssist]") != std::string::npos);

    agg.Execute(atlas::ai::AIRequestType::Analysis, "test", ctx);
    assert(backend.LastPrompt().find("[Analysis]") != std::string::npos);

    std::cout << "[PASS] test_ai_aggregator_request_type_prefix" << std::endl;
}

// ============================================================
// Runner
// ============================================================

void run_next_tasks_phase5_tests() {
    std::cout << "\n--- Next Tasks Phase 5: Vulkan + Quest + Inventory + AIAggregator ---" << std::endl;

    // Vulkan renderer
    test_vulkan_begin_end_frame();
    test_vulkan_draw_rect();
    test_vulkan_draw_text();
    test_vulkan_draw_icon();
    test_vulkan_draw_border();
    test_vulkan_draw_image();
    test_vulkan_multiple_commands();
    test_vulkan_frame_clears_commands();

    // Quest editor panel
    test_quest_panel_name();
    test_quest_panel_add_node();
    test_quest_panel_remove_node();
    test_quest_panel_connections();
    test_quest_panel_remove_cleans_connections();
    test_quest_panel_selection();
    test_quest_panel_preview_mode();
    test_quest_panel_export();
    test_quest_panel_import();
    test_quest_panel_clear();
    test_quest_panel_draw();
    test_quest_panel_draw_preview();

    // Inventory editor panel
    test_inventory_panel_name();
    test_inventory_panel_add_item();
    test_inventory_panel_remove_item();
    test_inventory_panel_selection();
    test_inventory_panel_filter();
    test_inventory_panel_sort_by_name();
    test_inventory_panel_sort_by_category();
    test_inventory_panel_export();
    test_inventory_panel_import();
    test_inventory_panel_clear();
    test_inventory_panel_draw();
    test_inventory_panel_draw_with_filter();

    // AI aggregator
    test_ai_aggregator_empty();
    test_ai_aggregator_register();
    test_ai_aggregator_execute_single();
    test_ai_aggregator_selects_best();
    test_ai_aggregator_skips_empty_responses();
    test_ai_aggregator_all_empty_responses();
    test_ai_aggregator_request_type_prefix();
}
