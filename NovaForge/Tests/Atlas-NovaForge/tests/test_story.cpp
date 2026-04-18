#include "../engine/story/StoryGraph.h"
#include <iostream>
#include <cassert>

using namespace atlas::story;

void test_story_add_node() {
    StoryGraph graph;
    graph.Init();

    uint32_t id = graph.AddNode(StoryNodeType::Character, "hero", "The main character");
    assert(id > 0);
    assert(graph.GetNode(id) != nullptr);
    assert(graph.GetNode(id)->type == StoryNodeType::Character);
    assert(graph.GetNode(id)->name == "hero");
    assert(graph.GetNode(id)->description == "The main character");
    assert(graph.NodeCount() == 1);

    graph.RemoveNode(id);
    assert(graph.GetNode(id) == nullptr);
    assert(graph.NodeCount() == 0);

    std::cout << "[PASS] test_story_add_node" << std::endl;
}

void test_story_connections() {
    StoryGraph graph;
    graph.Init();

    uint32_t hero = graph.AddNode(StoryNodeType::Character, "hero", "Protagonist");
    uint32_t villain = graph.AddNode(StoryNodeType::Character, "villain", "Antagonist");
    uint32_t conflict = graph.AddNode(StoryNodeType::Conflict, "war", "A great war");

    graph.Connect(hero, conflict);
    graph.Connect(villain, conflict);

    auto conns = graph.GetConnections(hero);
    assert(conns.size() == 1);
    assert(conns[0] == conflict);

    auto conns2 = graph.GetConnections(villain);
    assert(conns2.size() == 1);
    assert(conns2[0] == conflict);

    std::cout << "[PASS] test_story_connections" << std::endl;
}

void test_story_tags() {
    StoryGraph graph;
    graph.Init();

    uint32_t id = graph.AddNode(StoryNodeType::LoreFact, "dragon_lore", "Dragons once ruled");
    graph.AddTag(id, "lore");
    graph.AddTag(id, "dragons");

    uint32_t id2 = graph.AddNode(StoryNodeType::Event, "dragon_attack", "A dragon attacks");
    graph.AddTag(id2, "dragons");

    auto dragonNodes = graph.GetNodesByTag("dragons");
    assert(dragonNodes.size() == 2);

    auto loreNodes = graph.GetNodesByTag("lore");
    assert(loreNodes.size() == 1);

    auto emptyNodes = graph.GetNodesByTag("unknown");
    assert(emptyNodes.size() == 0);

    std::cout << "[PASS] test_story_tags" << std::endl;
}

void test_story_get_by_type() {
    StoryGraph graph;
    graph.Init();

    graph.AddNode(StoryNodeType::Character, "hero", "Protagonist");
    graph.AddNode(StoryNodeType::Character, "sidekick", "Helper");
    graph.AddNode(StoryNodeType::Goal, "save_world", "Save the world");

    auto characters = graph.GetNodesByType(StoryNodeType::Character);
    assert(characters.size() == 2);

    auto goals = graph.GetNodesByType(StoryNodeType::Goal);
    assert(goals.size() == 1);

    auto events = graph.GetNodesByType(StoryNodeType::Event);
    assert(events.size() == 0);

    std::cout << "[PASS] test_story_get_by_type" << std::endl;
}
