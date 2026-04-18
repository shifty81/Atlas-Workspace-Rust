#include "../engine/ai/AtlasAICore.h"
#include <iostream>
#include <cassert>

void test_ai_core_default_permissions() {
    atlas::ai::AtlasAICore core;
    assert(core.GetPermissions() == 0);
    assert(!core.HasPermission(atlas::ai::AIPermission::ReadState));
    std::cout << "[PASS] test_ai_core_default_permissions" << std::endl;
}

void test_ai_core_set_permissions() {
    atlas::ai::AtlasAICore core;
    core.SetPermissions(static_cast<uint8_t>(atlas::ai::AIPermission::ReadState) |
                        static_cast<uint8_t>(atlas::ai::AIPermission::SuggestChanges));
    assert(core.GetPermissions() == 3);
    std::cout << "[PASS] test_ai_core_set_permissions" << std::endl;
}

void test_ai_core_has_permission() {
    atlas::ai::AtlasAICore core;
    core.SetPermissions(static_cast<uint8_t>(atlas::ai::AIPermission::ReadState) |
                        static_cast<uint8_t>(atlas::ai::AIPermission::ExecuteCommands));
    assert(core.HasPermission(atlas::ai::AIPermission::ReadState));
    assert(!core.HasPermission(atlas::ai::AIPermission::SuggestChanges));
    assert(core.HasPermission(atlas::ai::AIPermission::ExecuteCommands));
    std::cout << "[PASS] test_ai_core_has_permission" << std::endl;
}

void test_ai_core_register_intent() {
    atlas::ai::AtlasAICore core;
    atlas::ai::AIIntentHandler handler;
    handler.name = "test_intent";
    handler.description = "A test intent";
    handler.requiredPermissions = static_cast<uint8_t>(atlas::ai::AIPermission::ReadState);
    handler.handler = [](const atlas::ai::AIRequest&) {
        atlas::ai::AIResponse resp;
        resp.success = true;
        resp.result = "ok";
        return resp;
    };
    core.RegisterIntent(handler);
    assert(core.IntentCount() == 1);
    assert(core.GetIntent("test_intent") != nullptr);
    assert(core.GetIntent("test_intent")->description == "A test intent");
    std::cout << "[PASS] test_ai_core_register_intent" << std::endl;
}

void test_ai_core_unregister_intent() {
    atlas::ai::AtlasAICore core;
    atlas::ai::AIIntentHandler handler;
    handler.name = "removable";
    handler.handler = [](const atlas::ai::AIRequest&) { return atlas::ai::AIResponse{}; };
    core.RegisterIntent(handler);
    assert(core.IntentCount() == 1);
    core.UnregisterIntent("removable");
    assert(core.IntentCount() == 0);
    assert(core.GetIntent("removable") == nullptr);
    std::cout << "[PASS] test_ai_core_unregister_intent" << std::endl;
}

void test_ai_core_list_intents() {
    atlas::ai::AtlasAICore core;
    atlas::ai::AIIntentHandler h1;
    h1.name = "alpha";
    h1.handler = [](const atlas::ai::AIRequest&) { return atlas::ai::AIResponse{}; };
    atlas::ai::AIIntentHandler h2;
    h2.name = "beta";
    h2.handler = [](const atlas::ai::AIRequest&) { return atlas::ai::AIResponse{}; };
    core.RegisterIntent(h1);
    core.RegisterIntent(h2);
    auto intents = core.ListIntents();
    assert(intents.size() == 2);
    std::cout << "[PASS] test_ai_core_list_intents" << std::endl;
}

void test_ai_core_process_request() {
    atlas::ai::AtlasAICore core;
    core.SetPermissions(static_cast<uint8_t>(atlas::ai::AIPermission::All));
    atlas::ai::AIIntentHandler handler;
    handler.name = "greet";
    handler.requiredPermissions = static_cast<uint8_t>(atlas::ai::AIPermission::ReadState);
    handler.handler = [](const atlas::ai::AIRequest& req) {
        atlas::ai::AIResponse resp;
        resp.success = true;
        resp.result = "Hello!";
        resp.confidence = 0.95;
        return resp;
    };
    core.RegisterIntent(handler);

    atlas::ai::AIRequest req;
    req.intentName = "greet";
    auto resp = core.ProcessRequest(req);
    assert(resp.success);
    assert(resp.result == "Hello!");
    assert(resp.confidence == 0.95);
    assert(resp.requestID > 0);
    std::cout << "[PASS] test_ai_core_process_request" << std::endl;
}

void test_ai_core_permission_denied() {
    atlas::ai::AtlasAICore core;
    core.SetPermissions(static_cast<uint8_t>(atlas::ai::AIPermission::ReadState));
    atlas::ai::AIIntentHandler handler;
    handler.name = "modify";
    handler.requiredPermissions = static_cast<uint8_t>(atlas::ai::AIPermission::ApplyChanges);
    handler.handler = [](const atlas::ai::AIRequest&) {
        atlas::ai::AIResponse resp;
        resp.success = true;
        return resp;
    };
    core.RegisterIntent(handler);

    atlas::ai::AIRequest req;
    req.intentName = "modify";
    auto resp = core.ProcessRequest(req);
    assert(!resp.success);
    assert(!resp.errorMessage.empty());
    std::cout << "[PASS] test_ai_core_permission_denied" << std::endl;
}

void test_ai_core_unknown_intent() {
    atlas::ai::AtlasAICore core;
    core.SetPermissions(static_cast<uint8_t>(atlas::ai::AIPermission::All));
    atlas::ai::AIRequest req;
    req.intentName = "nonexistent";
    auto resp = core.ProcessRequest(req);
    assert(!resp.success);
    assert(resp.errorMessage.find("Unknown intent") != std::string::npos);
    std::cout << "[PASS] test_ai_core_unknown_intent" << std::endl;
}

void test_ai_core_history() {
    atlas::ai::AtlasAICore core;
    core.SetPermissions(static_cast<uint8_t>(atlas::ai::AIPermission::All));
    atlas::ai::AIIntentHandler handler;
    handler.name = "echo";
    handler.handler = [](const atlas::ai::AIRequest& req) {
        atlas::ai::AIResponse resp;
        resp.success = true;
        resp.result = req.context;
        return resp;
    };
    core.RegisterIntent(handler);

    atlas::ai::AIRequest req;
    req.intentName = "echo";
    req.context = "test1";
    core.ProcessRequest(req);
    req.context = "test2";
    core.ProcessRequest(req);

    assert(core.RequestCount() == 2);
    assert(core.RequestHistory().size() == 2);
    assert(core.ResponseHistory().size() == 2);
    assert(core.ResponseHistory()[0].result == "test1");
    assert(core.ResponseHistory()[1].result == "test2");
    std::cout << "[PASS] test_ai_core_history" << std::endl;
}

void test_ai_core_clear() {
    atlas::ai::AtlasAICore core;
    core.SetPermissions(static_cast<uint8_t>(atlas::ai::AIPermission::All));
    atlas::ai::AIIntentHandler handler;
    handler.name = "test";
    handler.handler = [](const atlas::ai::AIRequest&) { return atlas::ai::AIResponse{}; };
    core.RegisterIntent(handler);
    atlas::ai::AIRequest req;
    req.intentName = "test";
    core.ProcessRequest(req);

    core.Clear();
    assert(core.GetPermissions() == 0);
    assert(core.IntentCount() == 0);
    assert(core.RequestCount() == 0);
    assert(core.RequestHistory().empty());
    assert(core.ResponseHistory().empty());
    std::cout << "[PASS] test_ai_core_clear" << std::endl;
}
