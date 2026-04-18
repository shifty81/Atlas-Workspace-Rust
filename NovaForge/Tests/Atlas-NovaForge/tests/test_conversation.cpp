#include <cassert>
#include <iostream>
#include <memory>
#include "../engine/conversation/ConversationGraph.h"
#include "../engine/conversation/ConversationNodes.h"

using namespace atlas::conversation;

void test_conversation_add_nodes() {
    ConversationGraph graph;
    auto node = std::make_unique<DialogueNode>();
    node->dialogueText = "Hello, commander.";
    ConversationNodeID id = graph.AddNode(std::move(node));
    assert(id > 0);
    assert(graph.NodeCount() == 1);
    std::cout << "[PASS] test_conversation_add_nodes" << std::endl;
}

void test_conversation_remove_node() {
    ConversationGraph graph;
    auto node = std::make_unique<DialogueNode>();
    ConversationNodeID id = graph.AddNode(std::move(node));
    graph.RemoveNode(id);
    assert(graph.NodeCount() == 0);
    std::cout << "[PASS] test_conversation_remove_node" << std::endl;
}

void test_conversation_compile_empty() {
    ConversationGraph graph;
    assert(graph.Compile() == true);
    std::cout << "[PASS] test_conversation_compile_empty" << std::endl;
}

void test_conversation_dialogue_node() {
    ConversationGraph graph;

    auto dlg = std::make_unique<DialogueNode>();
    dlg->dialogueText = "We need to move out.";
    ConversationNodeID n1 = graph.AddNode(std::move(dlg));

    assert(graph.Compile());

    ConversationContext ctx;
    ctx.speakerID = 1;
    ctx.listenerID = 2;
    assert(graph.Execute(ctx));

    const ConversationValue* out = graph.GetOutput(n1, 0);
    assert(out != nullptr);
    assert(out->type == ConversationPinType::Dialogue);
    assert(out->text == "We need to move out.");

    std::cout << "[PASS] test_conversation_dialogue_node" << std::endl;
}

void test_conversation_chain() {
    ConversationGraph graph;

    auto dlg = std::make_unique<DialogueNode>();
    dlg->dialogueText = "Stand ready.";
    ConversationNodeID n1 = graph.AddNode(std::move(dlg));

    auto mem = std::make_unique<MemoryWriteNode>();
    mem->memoryKey = "last_order";
    mem->memoryValue = "stand_ready";
    ConversationNodeID n2 = graph.AddNode(std::move(mem));

    graph.AddEdge({ n1, 0, n2, 0 });

    assert(graph.Compile());

    ConversationContext ctx;
    assert(graph.Execute(ctx));

    const ConversationValue* out = graph.GetOutput(n2, 0);
    assert(out != nullptr);
    assert(out->type == ConversationPinType::Memory);
    assert(out->metadata.count("last_order") > 0);
    assert(out->metadata.at("last_order") == "stand_ready");

    std::cout << "[PASS] test_conversation_chain" << std::endl;
}

void test_conversation_relationship_delta() {
    ConversationGraph graph;

    auto dlg = std::make_unique<DialogueNode>();
    dlg->dialogueText = "Good work out there.";
    ConversationNodeID n1 = graph.AddNode(std::move(dlg));

    auto rel = std::make_unique<RelationshipDeltaNode>();
    rel->delta = 5.0f;
    ConversationNodeID n2 = graph.AddNode(std::move(rel));

    graph.AddEdge({ n1, 0, n2, 0 });

    assert(graph.Compile());

    ConversationContext ctx;
    ctx.speakerID = 100;
    ctx.listenerID = 200;
    assert(graph.Execute(ctx));

    const ConversationValue* out = graph.GetOutput(n2, 0);
    assert(out != nullptr);
    assert(out->type == ConversationPinType::Relationship);
    assert(out->numericValue >= 4.99f && out->numericValue <= 5.01f);
    assert(out->metadata.count("speaker") > 0);
    assert(out->metadata.at("speaker") == "100");

    std::cout << "[PASS] test_conversation_relationship_delta" << std::endl;
}

void test_conversation_player_choice() {
    ConversationGraph graph;

    auto choice = std::make_unique<PlayerChoiceNode>();
    choice->choices = { "Accept", "Decline", "Ask for more info" };
    choice->selectedIndex = 1;
    ConversationNodeID n1 = graph.AddNode(std::move(choice));

    assert(graph.Compile());

    ConversationContext ctx;
    assert(graph.Execute(ctx));

    const ConversationValue* out = graph.GetOutput(n1, 0);
    assert(out != nullptr);
    assert(out->type == ConversationPinType::Choice);
    assert(out->text == "Decline");
    assert(out->numericValue >= 0.99f && out->numericValue <= 1.01f);

    std::cout << "[PASS] test_conversation_player_choice" << std::endl;
}
