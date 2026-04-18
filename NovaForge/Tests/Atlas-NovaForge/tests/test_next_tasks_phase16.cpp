// Phase 16: AI assistant LLM wiring + permission enforcement tests

#include "../editor/assistant/EditorAssistant.h"
#include "../editor/assistant/AssetGraphAssistant.h"
#include "../editor/ui/EditorAttachProtocol.h"
#include "../engine/ai/LLMBackend.h"
#include "../engine/asset_graph/AssetGraphExecutor.h"
#include <cassert>
#include <iostream>
#include <memory>

using namespace atlas::editor;
using namespace atlas::interaction;
using namespace atlas::ai;
using namespace atlas::asset;

// ---- Helper: test asset node ----

class Phase16TestNode : public AssetNode {
public:
    explicit Phase16TestNode(const std::string& name) : m_name(name) {}
    void Evaluate(const AssetContext&) override {}
    std::string Name() const override { return m_name; }
private:
    std::string m_name;
};

// ============================================================
// EditorAssistant + LLM tests
// ============================================================

void test_editor_assistant_llm_null_by_default() {
    EditorAssistant assistant;
    assert(assistant.GetLLMBackend() == nullptr);
    std::cout << "  [PASS] test_editor_assistant_llm_null_by_default" << std::endl;
}

void test_editor_assistant_set_llm_backend() {
    EditorAssistant assistant;
    LLMBackendRegistry registry;
    assistant.SetLLMBackend(&registry);
    assert(assistant.GetLLMBackend() == &registry);
    std::cout << "  [PASS] test_editor_assistant_set_llm_backend" << std::endl;
}

void test_editor_assistant_known_intents_unchanged() {
    // Known intents still return hardcoded responses even with LLM set
    EditorAssistant assistant;
    LLMBackendRegistry registry;
    assistant.SetLLMBackend(&registry);

    Intent intent{"ExplainPerformance", 1.0f};
    Utterance source{"why?", 0, 0, {InteractionMode::Debug, true}};
    auto result = assistant.Route(intent, source);
    assert(result.text.find("noise warp node") != std::string::npos);
    std::cout << "  [PASS] test_editor_assistant_known_intents_unchanged" << std::endl;
}

void test_editor_assistant_unknown_falls_back_without_llm() {
    EditorAssistant assistant;
    // No LLM backend set
    Intent intent{"SomeUnknownIntent", 1.0f};
    Utterance source{"what?", 0, 0, {InteractionMode::Debug, true}};
    auto result = assistant.Route(intent, source);
    assert(result.text.find("don't have") != std::string::npos);
    std::cout << "  [PASS] test_editor_assistant_unknown_falls_back_without_llm" << std::endl;
}

void test_editor_assistant_unknown_uses_llm() {
    EditorAssistant assistant;
    LLMBackendRegistry registry;

    // Register a canned response in the offline stub
    auto& offlineStub = dynamic_cast<OfflineLLMBackend&>(registry.GetBackend());
    offlineStub.RegisterResponse("UnknownIntent:", "This is an AI-powered response.");

    assistant.SetLLMBackend(&registry);

    Intent intent{"UnknownIntent", 1.0f};
    Utterance source{"explain this", 0, 0, {InteractionMode::Debug, true}};
    auto result = assistant.Route(intent, source);
    assert(result.text.find("AI-powered response") != std::string::npos);
    std::cout << "  [PASS] test_editor_assistant_unknown_uses_llm" << std::endl;
}

void test_editor_assistant_llm_offline_fallback() {
    EditorAssistant assistant;
    LLMBackendRegistry registry;
    // Default offline stub has no registered responses
    assistant.SetLLMBackend(&registry);

    Intent intent{"NoMatchIntent", 1.0f};
    Utterance source{"?", 0, 0, {InteractionMode::Debug, true}};
    auto result = assistant.Route(intent, source);
    // Offline stub returns "[offline]..." which is non-empty,
    // so the LLM response should be used
    assert(!result.text.empty());
    std::cout << "  [PASS] test_editor_assistant_llm_offline_fallback" << std::endl;
}

// ============================================================
// AssetGraphAssistant + LLM tests
// ============================================================

void test_asset_assistant_llm_null_by_default() {
    AssetGraphAssistant assistant;
    assert(assistant.GetLLMBackend() == nullptr);
    std::cout << "  [PASS] test_asset_assistant_llm_null_by_default" << std::endl;
}

void test_asset_assistant_set_llm_backend() {
    AssetGraphAssistant assistant;
    LLMBackendRegistry registry;
    assistant.SetLLMBackend(&registry);
    assert(assistant.GetLLMBackend() == &registry);
    std::cout << "  [PASS] test_asset_assistant_set_llm_backend" << std::endl;
}

void test_asset_assistant_suggest_empty_with_llm() {
    AssetGraphAssistant assistant;
    LLMBackendRegistry registry;

    auto& offlineStub = dynamic_cast<OfflineLLMBackend&>(registry.GetBackend());
    offlineStub.RegisterResponse("suggest:empty_graph", "NoiseGenerator");

    assistant.SetLLMBackend(&registry);

    auto suggestions = assistant.SuggestNodes();
    assert(!suggestions.empty());
    assert(suggestions[0].name == "NoiseGenerator");
    assert(suggestions[0].reason.find("AI-suggested") != std::string::npos);
    std::cout << "  [PASS] test_asset_assistant_suggest_empty_with_llm" << std::endl;
}

void test_asset_assistant_suggest_with_nodes_and_llm() {
    AssetGraphExecutor executor;
    executor.AddNode(std::make_unique<Phase16TestNode>("A"));
    executor.AddNode(std::make_unique<Phase16TestNode>("B"));

    AssetGraphAssistant assistant;
    assistant.SetExecutor(&executor);

    LLMBackendRegistry registry;
    auto& offlineStub = dynamic_cast<OfflineLLMBackend&>(registry.GetBackend());
    offlineStub.RegisterResponse("suggest:graph_with_2_nodes", "MeshDecimator");

    assistant.SetLLMBackend(&registry);

    auto suggestions = assistant.SuggestNodes();
    // Should contain AI suggestion + template suggestions
    assert(!suggestions.empty());
    bool foundAI = false;
    for (auto& s : suggestions) {
        if (s.name == "MeshDecimator") foundAI = true;
    }
    assert(foundAI);
    std::cout << "  [PASS] test_asset_assistant_suggest_with_nodes_and_llm" << std::endl;
}

void test_asset_assistant_explain_with_llm() {
    AssetGraphExecutor executor;
    executor.AddNode(std::make_unique<Phase16TestNode>("A"));
    executor.AddNode(std::make_unique<Phase16TestNode>("B"));
    executor.AddNode(std::make_unique<Phase16TestNode>("C"));

    AssetGraphAssistant assistant;
    assistant.SetExecutor(&executor);

    LLMBackendRegistry registry;
    auto& offlineStub = dynamic_cast<OfflineLLMBackend&>(registry.GetBackend());
    offlineStub.RegisterResponse("explain:graph_with_3_nodes",
                                  "A multi-stage asset generation pipeline.");

    assistant.SetLLMBackend(&registry);

    auto explanation = assistant.ExplainGraph();
    assert(explanation.find("3 node(s)") != std::string::npos);
    assert(explanation.find("multi-stage") != std::string::npos);
    std::cout << "  [PASS] test_asset_assistant_explain_with_llm" << std::endl;
}

void test_asset_assistant_mutate_with_llm() {
    AssetGraphExecutor executor;
    executor.AddNode(std::make_unique<Phase16TestNode>("A"));

    AssetGraphAssistant assistant;
    assistant.SetExecutor(&executor);

    LLMBackendRegistry registry;
    auto& offlineStub = dynamic_cast<OfflineLLMBackend&>(registry.GetBackend());
    offlineStub.RegisterResponse("mutate:intensity_",
                                  "AI-adjusted noise frequency and material roughness.");

    assistant.SetLLMBackend(&registry);

    auto result = assistant.MutateGraph(0.5f);
    assert(result.find("AI-adjusted") != std::string::npos);
    std::cout << "  [PASS] test_asset_assistant_mutate_with_llm" << std::endl;
}

void test_asset_assistant_no_llm_fallback() {
    // Without LLM, template responses should be used
    AssetGraphExecutor executor;
    executor.AddNode(std::make_unique<Phase16TestNode>("A"));

    AssetGraphAssistant assistant;
    assistant.SetExecutor(&executor);

    auto suggestions = assistant.SuggestNodes();
    assert(!suggestions.empty());
    // Should have template suggestions (TextureVariation since count < 3)
    bool foundTemplate = false;
    for (auto& s : suggestions) {
        if (s.name == "TextureVariation") foundTemplate = true;
    }
    assert(foundTemplate);
    std::cout << "  [PASS] test_asset_assistant_no_llm_fallback" << std::endl;
}

// ============================================================
// EditorAttachProtocol permission enforcement tests
// ============================================================

void test_attach_request_operation_requires_connection() {
    EditorAttachProtocol protocol;
    protocol.Init();
    // Not connected
    assert(!protocol.RequestOperation(EditorOperation::ViewState));
    std::cout << "  [PASS] test_attach_request_operation_requires_connection" << std::endl;
}

void test_attach_request_operation_standalone() {
    EditorAttachProtocol protocol;
    protocol.Init();
    protocol.SetPermissionTier(atlas::PermissionTier::Developer);
    protocol.Connect({AttachMode::Standalone});

    assert(protocol.RequestOperation(EditorOperation::ViewState));
    assert(protocol.RequestOperation(EditorOperation::ModifyState));
    assert(protocol.RequestOperation(EditorOperation::EditAssets));
    assert(protocol.RequestOperation(EditorOperation::StepSimulation));
    std::cout << "  [PASS] test_attach_request_operation_standalone" << std::endl;
}

void test_attach_request_operation_replay_readonly() {
    EditorAttachProtocol protocol;
    protocol.Init();
    protocol.SetPermissionTier(atlas::PermissionTier::Admin);
    protocol.Connect({AttachMode::Replay, "", 0, "/tmp/test.replay"});

    // Replay is read-only regardless of tier
    assert(protocol.RequestOperation(EditorOperation::ViewState));
    assert(protocol.RequestOperation(EditorOperation::InspectEntities));
    assert(protocol.RequestOperation(EditorOperation::StepSimulation));
    assert(!protocol.RequestOperation(EditorOperation::ModifyState));
    assert(!protocol.RequestOperation(EditorOperation::InjectInput));
    assert(!protocol.RequestOperation(EditorOperation::EditAssets));
    assert(!protocol.RequestOperation(EditorOperation::RunCI));
    std::cout << "  [PASS] test_attach_request_operation_replay_readonly" << std::endl;
}

void test_attach_request_operation_headless_no_step() {
    EditorAttachProtocol protocol;
    protocol.Init();
    protocol.SetPermissionTier(atlas::PermissionTier::Developer);
    protocol.Connect({AttachMode::HeadlessServer, "localhost", 9999});

    assert(protocol.RequestOperation(EditorOperation::ViewState));
    assert(protocol.RequestOperation(EditorOperation::ModifyState));
    assert(!protocol.RequestOperation(EditorOperation::StepSimulation));
    std::cout << "  [PASS] test_attach_request_operation_headless_no_step" << std::endl;
}

void test_attach_request_operation_viewonly_restricted() {
    EditorAttachProtocol protocol;
    protocol.Init();
    protocol.SetPermissionTier(atlas::PermissionTier::ViewOnly);
    protocol.Connect({AttachMode::Standalone});

    assert(protocol.RequestOperation(EditorOperation::ViewState));
    assert(protocol.RequestOperation(EditorOperation::InspectEntities));
    assert(!protocol.RequestOperation(EditorOperation::ModifyState));
    assert(!protocol.RequestOperation(EditorOperation::EditAssets));
    assert(!protocol.RequestOperation(EditorOperation::InjectInput));
    std::cout << "  [PASS] test_attach_request_operation_viewonly_restricted" << std::endl;
}

void test_attach_request_operation_qa_can_record() {
    EditorAttachProtocol protocol;
    protocol.Init();
    protocol.SetPermissionTier(atlas::PermissionTier::QA);
    protocol.Connect({AttachMode::LiveClient, "localhost", 8080});

    assert(protocol.RequestOperation(EditorOperation::ViewState));
    assert(protocol.RequestOperation(EditorOperation::RecordReplay));
    assert(!protocol.RequestOperation(EditorOperation::ModifyState));
    assert(!protocol.RequestOperation(EditorOperation::EditAssets));
    std::cout << "  [PASS] test_attach_request_operation_qa_can_record" << std::endl;
}

void test_attach_request_operation_ci_can_run_ci() {
    EditorAttachProtocol protocol;
    protocol.Init();
    protocol.SetPermissionTier(atlas::PermissionTier::CI);
    protocol.Connect({AttachMode::Standalone});

    assert(protocol.RequestOperation(EditorOperation::ViewState));
    assert(protocol.RequestOperation(EditorOperation::RunCI));
    assert(!protocol.RequestOperation(EditorOperation::ModifyState));
    assert(!protocol.RequestOperation(EditorOperation::EditAssets));
    std::cout << "  [PASS] test_attach_request_operation_ci_can_run_ci" << std::endl;
}

void test_attach_request_operation_disconnected_after_disconnect() {
    EditorAttachProtocol protocol;
    protocol.Init();
    protocol.SetPermissionTier(atlas::PermissionTier::Admin);
    protocol.Connect({AttachMode::Standalone});
    assert(protocol.RequestOperation(EditorOperation::ViewState));

    protocol.Disconnect();
    assert(!protocol.RequestOperation(EditorOperation::ViewState));
    std::cout << "  [PASS] test_attach_request_operation_disconnected_after_disconnect" << std::endl;
}

// ---------------------------------------------------------------
// Runner
// ---------------------------------------------------------------

void register_next_tasks_phase16() {
    std::cout << "\n--- Phase 16: EditorAssistant + LLM ---" << std::endl;
    test_editor_assistant_llm_null_by_default();
    test_editor_assistant_set_llm_backend();
    test_editor_assistant_known_intents_unchanged();
    test_editor_assistant_unknown_falls_back_without_llm();
    test_editor_assistant_unknown_uses_llm();
    test_editor_assistant_llm_offline_fallback();

    std::cout << "\n--- Phase 16: AssetGraphAssistant + LLM ---" << std::endl;
    test_asset_assistant_llm_null_by_default();
    test_asset_assistant_set_llm_backend();
    test_asset_assistant_suggest_empty_with_llm();
    test_asset_assistant_suggest_with_nodes_and_llm();
    test_asset_assistant_explain_with_llm();
    test_asset_assistant_mutate_with_llm();
    test_asset_assistant_no_llm_fallback();

    std::cout << "\n--- Phase 16: EditorAttachProtocol Permission Enforcement ---" << std::endl;
    test_attach_request_operation_requires_connection();
    test_attach_request_operation_standalone();
    test_attach_request_operation_replay_readonly();
    test_attach_request_operation_headless_no_step();
    test_attach_request_operation_viewonly_restricted();
    test_attach_request_operation_qa_can_record();
    test_attach_request_operation_ci_can_run_ci();
    test_attach_request_operation_disconnected_after_disconnect();
}
