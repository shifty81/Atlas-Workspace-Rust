#include "../editor/assistant/EditorAssistant.h"
#include "../editor/panels/InteractionDebugger.h"
#include <iostream>
#include <cassert>

using namespace atlas::editor;
using namespace atlas::interaction;

void test_editor_assistant_explain_performance() {
    EditorAssistant assistant;
    Intent intent{ "ExplainPerformance", 1.0f };
    Utterance source{ "why is this slow?", 0, 0, { InteractionMode::Debug, true } };

    auto result = assistant.Route(intent, source);
    assert(result.text.find("noise warp node") != std::string::npos);
    assert(result.ctx.mode == InteractionMode::Debug);
    assert(result.ctx.authoritative);
    std::cout << "[PASS] test_editor_assistant_explain_performance" << std::endl;
}

void test_editor_assistant_explain_node() {
    EditorAssistant assistant;
    Intent intent{ "ExplainGraphNode", 1.0f };
    Utterance source{ "what does this node do?", 0, 0, { InteractionMode::Debug, true } };

    auto result = assistant.Route(intent, source);
    assert(result.text.find("domain coordinates") != std::string::npos);
    std::cout << "[PASS] test_editor_assistant_explain_node" << std::endl;
}

void test_editor_assistant_unknown() {
    EditorAssistant assistant;
    Intent intent{ "SomethingUnknown", 1.0f };
    Utterance source{ "???", 0, 0, { InteractionMode::Debug, true } };

    auto result = assistant.Route(intent, source);
    assert(result.text.find("don't have") != std::string::npos);
    std::cout << "[PASS] test_editor_assistant_unknown" << std::endl;
}

void test_interaction_debugger_record() {
    InteractionDebugger debugger;
    assert(debugger.EntryCount() == 0);

    DebuggerLogEntry entry;
    entry.utterance = { "hello", 1, 2, { InteractionMode::Command, true } };
    entry.intent = { "Greet", 1.0f };

    debugger.Record(entry);
    assert(debugger.EntryCount() == 1);
    assert(debugger.GetEntry(0).intent.name == "Greet");
    assert(debugger.GetEntry(0).utterance.text == "hello");

    std::cout << "[PASS] test_interaction_debugger_record" << std::endl;
}

void test_interaction_debugger_clear() {
    InteractionDebugger debugger;
    DebuggerLogEntry entry;
    entry.utterance = { "test", 0, 0, { InteractionMode::Debug, true } };
    entry.intent = { "Test", 1.0f };

    debugger.Record(entry);
    debugger.Record(entry);
    assert(debugger.EntryCount() == 2);

    debugger.Clear();
    assert(debugger.EntryCount() == 0);
    std::cout << "[PASS] test_interaction_debugger_clear" << std::endl;
}
