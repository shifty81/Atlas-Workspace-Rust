#include <iostream>
#include <cassert>
#include <string>
#include <vector>
#include <algorithm>

#include "../engine/render/VulkanRenderer.h"
#include "../engine/assets/SocketHttpClient.h"
#include "../editor/ui/EditorAttachProtocol.h"
#include "../engine/rules/ServerRules.h"

// ============================================================
// Task 1: VulkanRenderer Pipeline Stages
// ============================================================

void test_vulkan_create_render_pass() {
    atlas::render::VulkanRenderer renderer;
    atlas::render::VkRenderPassDesc desc;
    desc.name = "MainPass";
    desc.colorAttachmentCount = 2;
    desc.hasDepth = true;
    desc.clear = true;

    uint32_t id = renderer.CreateRenderPass(desc);
    assert(id >= 1);
    assert(renderer.RenderPassCount() == 1);

    const auto* pass = renderer.GetRenderPass(id);
    assert(pass != nullptr);
    assert(pass->name == "MainPass");
    assert(pass->colorAttachmentCount == 2);
    assert(pass->hasDepth == true);

    // Create a second pass
    desc.name = "ShadowPass";
    uint32_t id2 = renderer.CreateRenderPass(desc);
    assert(id2 == id + 1);
    assert(renderer.RenderPassCount() == 2);

    // Invalid id returns nullptr
    assert(renderer.GetRenderPass(0) == nullptr);
    assert(renderer.GetRenderPass(999) == nullptr);

    std::cout << "[PASS] test_vulkan_create_render_pass" << std::endl;
}

void test_vulkan_begin_end_render_pass() {
    atlas::render::VulkanRenderer renderer;
    atlas::render::VkRenderPassDesc desc;
    desc.name = "ForwardPass";
    uint32_t id = renderer.CreateRenderPass(desc);

    assert(!renderer.IsRenderPassActive());

    renderer.BeginRenderPass(id);
    assert(renderer.IsRenderPassActive());

    renderer.EndRenderPass();
    assert(!renderer.IsRenderPassActive());

    // Begin with invalid id should not activate
    renderer.BeginRenderPass(0);
    assert(!renderer.IsRenderPassActive());
    renderer.BeginRenderPass(999);
    assert(!renderer.IsRenderPassActive());

    std::cout << "[PASS] test_vulkan_begin_end_render_pass" << std::endl;
}

void test_vulkan_create_pipeline_state() {
    atlas::render::VulkanRenderer renderer;
    atlas::render::VkPipelineStateDesc desc;
    desc.vertexShader = "basic.vert";
    desc.fragmentShader = "basic.frag";
    desc.depthTest = true;
    desc.blending = false;

    uint32_t id = renderer.CreatePipelineState(desc);
    assert(id >= 1);
    assert(renderer.PipelineStateCount() == 1);

    const auto* state = renderer.GetPipelineState(id);
    assert(state != nullptr);
    assert(state->vertexShader == "basic.vert");
    assert(state->fragmentShader == "basic.frag");
    assert(state->depthTest == true);
    assert(state->id == id);

    // Invalid id
    assert(renderer.GetPipelineState(0) == nullptr);
    assert(renderer.GetPipelineState(999) == nullptr);

    std::cout << "[PASS] test_vulkan_create_pipeline_state" << std::endl;
}

void test_vulkan_bind_pipeline() {
    atlas::render::VulkanRenderer renderer;
    assert(renderer.BoundPipelineId() == 0);

    atlas::render::VkPipelineStateDesc desc;
    desc.vertexShader = "v.vert";
    desc.fragmentShader = "f.frag";
    uint32_t id = renderer.CreatePipelineState(desc);

    renderer.BindPipeline(id);
    assert(renderer.BoundPipelineId() == id);

    // Bind invalid should not change
    renderer.BindPipeline(0);
    assert(renderer.BoundPipelineId() == id);

    std::cout << "[PASS] test_vulkan_bind_pipeline" << std::endl;
}

void test_vulkan_create_buffer() {
    atlas::render::VulkanRenderer renderer;
    assert(renderer.BufferCount() == 0);

    uint32_t vb = renderer.CreateBuffer(atlas::render::VkGPUResource::Type::VertexBuffer, 1024);
    assert(vb >= 1);
    assert(renderer.BufferCount() == 1);

    const auto* buf = renderer.GetBuffer(vb);
    assert(buf != nullptr);
    assert(buf->type == atlas::render::VkGPUResource::Type::VertexBuffer);
    assert(buf->sizeBytes == 1024);
    assert(!buf->mapped);

    uint32_t ib = renderer.CreateBuffer(atlas::render::VkGPUResource::Type::IndexBuffer, 512);
    assert(renderer.BufferCount() == 2);
    assert(renderer.GetBuffer(ib)->type == atlas::render::VkGPUResource::Type::IndexBuffer);

    uint32_t ub = renderer.CreateBuffer(atlas::render::VkGPUResource::Type::UniformBuffer, 256);
    assert(renderer.BufferCount() == 3);

    // Invalid id
    assert(renderer.GetBuffer(0) == nullptr);
    assert(renderer.GetBuffer(999) == nullptr);

    std::cout << "[PASS] test_vulkan_create_buffer" << std::endl;
}

void test_vulkan_map_unmap_buffer() {
    atlas::render::VulkanRenderer renderer;
    uint32_t id = renderer.CreateBuffer(atlas::render::VkGPUResource::Type::UniformBuffer, 128);

    assert(!renderer.GetBuffer(id)->mapped);

    assert(renderer.MapBuffer(id));
    assert(renderer.GetBuffer(id)->mapped);

    // Double map should fail
    assert(!renderer.MapBuffer(id));

    assert(renderer.UnmapBuffer(id));
    assert(!renderer.GetBuffer(id)->mapped);

    // Double unmap should fail
    assert(!renderer.UnmapBuffer(id));

    // Map/Unmap invalid buffer
    assert(!renderer.MapBuffer(999));
    assert(!renderer.UnmapBuffer(999));

    std::cout << "[PASS] test_vulkan_map_unmap_buffer" << std::endl;
}

void test_vulkan_destroy_buffer() {
    atlas::render::VulkanRenderer renderer;
    uint32_t id1 = renderer.CreateBuffer(atlas::render::VkGPUResource::Type::VertexBuffer, 100);
    uint32_t id2 = renderer.CreateBuffer(atlas::render::VkGPUResource::Type::IndexBuffer, 200);
    assert(renderer.BufferCount() == 2);

    assert(renderer.DestroyBuffer(id1));
    assert(renderer.BufferCount() == 1);
    assert(renderer.GetBuffer(id1) == nullptr);
    assert(renderer.GetBuffer(id2) != nullptr);

    // Destroy same buffer again should fail
    assert(!renderer.DestroyBuffer(id1));

    // Destroy invalid
    assert(!renderer.DestroyBuffer(0));
    assert(!renderer.DestroyBuffer(999));

    std::cout << "[PASS] test_vulkan_destroy_buffer" << std::endl;
}

// ============================================================
// Task 2: SocketHttpClient
// ============================================================

void test_socket_http_parse_url_valid() {
    std::string host, path;
    uint16_t port = 0;

    bool ok = atlas::asset::SocketHttpClient::ParseURL(
        "http://example.com/api/v1/data", host, port, path);
    assert(ok);
    assert(host == "example.com");
    assert(port == 80);
    assert(path == "/api/v1/data");

    std::cout << "[PASS] test_socket_http_parse_url_valid" << std::endl;
}

void test_socket_http_parse_url_with_port() {
    std::string host, path;
    uint16_t port = 0;

    bool ok = atlas::asset::SocketHttpClient::ParseURL(
        "http://localhost:8080/test", host, port, path);
    assert(ok);
    assert(host == "localhost");
    assert(port == 8080);
    assert(path == "/test");

    std::cout << "[PASS] test_socket_http_parse_url_with_port" << std::endl;
}

void test_socket_http_parse_url_no_path() {
    std::string host, path;
    uint16_t port = 0;

    bool ok = atlas::asset::SocketHttpClient::ParseURL(
        "http://myserver.com", host, port, path);
    assert(ok);
    assert(host == "myserver.com");
    assert(port == 80);
    assert(path == "/");

    std::cout << "[PASS] test_socket_http_parse_url_no_path" << std::endl;
}

void test_socket_http_parse_url_https_rejected() {
    std::string host, path;
    uint16_t port = 0;

    bool ok = atlas::asset::SocketHttpClient::ParseURL(
        "https://secure.example.com/api", host, port, path);
    assert(!ok);

    std::cout << "[PASS] test_socket_http_parse_url_https_rejected" << std::endl;
}

void test_socket_http_parse_url_invalid() {
    std::string host, path;
    uint16_t port = 0;

    assert(!atlas::asset::SocketHttpClient::ParseURL("", host, port, path));
    assert(!atlas::asset::SocketHttpClient::ParseURL("ftp://files.com", host, port, path));
    assert(!atlas::asset::SocketHttpClient::ParseURL("not a url", host, port, path));
    assert(!atlas::asset::SocketHttpClient::ParseURL("http://", host, port, path));

    std::cout << "[PASS] test_socket_http_parse_url_invalid" << std::endl;
}

void test_socket_http_config_defaults() {
    atlas::asset::SocketHttpClient client;
    const auto& config = client.GetConfig();

    assert(config.connectTimeoutMs == 5000);
    assert(config.readTimeoutMs == 10000);
    assert(config.userAgent == "AtlasEngine/1.0");
    assert(config.maxRedirects == 5);
    assert(config.maxResponseSizeBytes == 50 * 1024 * 1024);

    // Test SetConfig
    atlas::asset::SocketHttpConfig custom;
    custom.connectTimeoutMs = 1000;
    custom.userAgent = "TestAgent";
    client.SetConfig(custom);
    assert(client.GetConfig().connectTimeoutMs == 1000);
    assert(client.GetConfig().userAgent == "TestAgent");

    std::cout << "[PASS] test_socket_http_config_defaults" << std::endl;
}

void test_socket_http_request_count() {
    atlas::asset::SocketHttpConfig cfg;
    cfg.connectTimeoutMs = 100;   // very short timeout for test
    cfg.readTimeoutMs = 100;
    atlas::asset::SocketHttpClient client(cfg);
    assert(client.TotalRequestCount() == 0);

    // GET to invalid host — will fail but should still increment count
    auto resp = client.Get("http://127.0.0.1:1/test");
    assert(client.TotalRequestCount() == 1);
    assert(resp.IsError());
    assert(!resp.errorMessage.empty());

    // Invalid URL should not increment (ParseURL fails before increment)
    auto resp2 = client.Get("https://secure.com");
    assert(client.TotalRequestCount() == 1);

    std::cout << "[PASS] test_socket_http_request_count" << std::endl;
}

// ============================================================
// Task 3: Editor Permission Enforcement
// ============================================================

void test_attach_default_permission_tier() {
    atlas::editor::EditorAttachProtocol protocol;
    protocol.Init();
    assert(protocol.GetPermissionTier() == atlas::PermissionTier::Developer);

    std::cout << "[PASS] test_attach_default_permission_tier" << std::endl;
}

void test_attach_set_permission_tier() {
    atlas::editor::EditorAttachProtocol protocol;
    protocol.Init();

    protocol.SetPermissionTier(atlas::PermissionTier::Admin);
    assert(protocol.GetPermissionTier() == atlas::PermissionTier::Admin);

    protocol.SetPermissionTier(atlas::PermissionTier::ViewOnly);
    assert(protocol.GetPermissionTier() == atlas::PermissionTier::ViewOnly);

    std::cout << "[PASS] test_attach_set_permission_tier" << std::endl;
}

void test_attach_viewonly_permissions() {
    atlas::editor::EditorAttachProtocol protocol;
    protocol.Init();
    protocol.SetPermissionTier(atlas::PermissionTier::ViewOnly);

    assert(protocol.IsOperationAllowed(atlas::editor::EditorOperation::ViewState));
    assert(protocol.IsOperationAllowed(atlas::editor::EditorOperation::InspectEntities));
    assert(!protocol.IsOperationAllowed(atlas::editor::EditorOperation::ModifyState));
    assert(!protocol.IsOperationAllowed(atlas::editor::EditorOperation::InjectInput));
    assert(!protocol.IsOperationAllowed(atlas::editor::EditorOperation::StepSimulation));
    assert(!protocol.IsOperationAllowed(atlas::editor::EditorOperation::RecordReplay));
    assert(!protocol.IsOperationAllowed(atlas::editor::EditorOperation::EditAssets));
    assert(!protocol.IsOperationAllowed(atlas::editor::EditorOperation::RunCI));

    auto ops = protocol.AllowedOperations();
    assert(ops.size() == 2);

    std::cout << "[PASS] test_attach_viewonly_permissions" << std::endl;
}

void test_attach_qa_permissions() {
    atlas::editor::EditorAttachProtocol protocol;
    protocol.Init();
    protocol.SetPermissionTier(atlas::PermissionTier::QA);

    assert(protocol.IsOperationAllowed(atlas::editor::EditorOperation::ViewState));
    assert(protocol.IsOperationAllowed(atlas::editor::EditorOperation::InspectEntities));
    assert(protocol.IsOperationAllowed(atlas::editor::EditorOperation::RecordReplay));
    assert(!protocol.IsOperationAllowed(atlas::editor::EditorOperation::ModifyState));
    assert(!protocol.IsOperationAllowed(atlas::editor::EditorOperation::EditAssets));
    assert(!protocol.IsOperationAllowed(atlas::editor::EditorOperation::RunCI));

    auto ops = protocol.AllowedOperations();
    assert(ops.size() == 3);

    std::cout << "[PASS] test_attach_qa_permissions" << std::endl;
}

void test_attach_developer_permissions() {
    atlas::editor::EditorAttachProtocol protocol;
    protocol.Init();
    protocol.SetPermissionTier(atlas::PermissionTier::Developer);

    assert(protocol.IsOperationAllowed(atlas::editor::EditorOperation::ViewState));
    assert(protocol.IsOperationAllowed(atlas::editor::EditorOperation::InspectEntities));
    assert(protocol.IsOperationAllowed(atlas::editor::EditorOperation::ModifyState));
    assert(protocol.IsOperationAllowed(atlas::editor::EditorOperation::InjectInput));
    assert(protocol.IsOperationAllowed(atlas::editor::EditorOperation::StepSimulation));
    assert(protocol.IsOperationAllowed(atlas::editor::EditorOperation::RecordReplay));
    assert(protocol.IsOperationAllowed(atlas::editor::EditorOperation::EditAssets));
    assert(!protocol.IsOperationAllowed(atlas::editor::EditorOperation::RunCI));

    auto ops = protocol.AllowedOperations();
    assert(ops.size() == 7);

    std::cout << "[PASS] test_attach_developer_permissions" << std::endl;
}

void test_attach_admin_permissions() {
    atlas::editor::EditorAttachProtocol protocol;
    protocol.Init();
    protocol.SetPermissionTier(atlas::PermissionTier::Admin);

    assert(protocol.IsOperationAllowed(atlas::editor::EditorOperation::ViewState));
    assert(protocol.IsOperationAllowed(atlas::editor::EditorOperation::InspectEntities));
    assert(protocol.IsOperationAllowed(atlas::editor::EditorOperation::ModifyState));
    assert(protocol.IsOperationAllowed(atlas::editor::EditorOperation::InjectInput));
    assert(protocol.IsOperationAllowed(atlas::editor::EditorOperation::StepSimulation));
    assert(protocol.IsOperationAllowed(atlas::editor::EditorOperation::RecordReplay));
    assert(protocol.IsOperationAllowed(atlas::editor::EditorOperation::EditAssets));
    assert(protocol.IsOperationAllowed(atlas::editor::EditorOperation::RunCI));

    auto ops = protocol.AllowedOperations();
    assert(ops.size() == 8);

    std::cout << "[PASS] test_attach_admin_permissions" << std::endl;
}

void test_attach_ci_permissions() {
    atlas::editor::EditorAttachProtocol protocol;
    protocol.Init();
    protocol.SetPermissionTier(atlas::PermissionTier::CI);

    assert(protocol.IsOperationAllowed(atlas::editor::EditorOperation::ViewState));
    assert(protocol.IsOperationAllowed(atlas::editor::EditorOperation::InspectEntities));
    assert(protocol.IsOperationAllowed(atlas::editor::EditorOperation::RunCI));
    assert(!protocol.IsOperationAllowed(atlas::editor::EditorOperation::ModifyState));
    assert(!protocol.IsOperationAllowed(atlas::editor::EditorOperation::EditAssets));
    assert(!protocol.IsOperationAllowed(atlas::editor::EditorOperation::RecordReplay));

    auto ops = protocol.AllowedOperations();
    assert(ops.size() == 3);

    std::cout << "[PASS] test_attach_ci_permissions" << std::endl;
}

void test_attach_permission_description() {
    atlas::editor::EditorAttachProtocol protocol;
    protocol.Init();

    protocol.SetPermissionTier(atlas::PermissionTier::ViewOnly);
    assert(!protocol.PermissionDescription().empty());
    assert(protocol.PermissionDescription().find("ViewOnly") != std::string::npos);

    protocol.SetPermissionTier(atlas::PermissionTier::Admin);
    assert(protocol.PermissionDescription().find("Admin") != std::string::npos);

    protocol.SetPermissionTier(atlas::PermissionTier::Developer);
    assert(protocol.PermissionDescription().find("Developer") != std::string::npos);

    std::cout << "[PASS] test_attach_permission_description" << std::endl;
}

// ============================================================
// Task 4: Live Edit Rules — Config Hot-Reload
// ============================================================

void test_rules_load_from_config() {
    auto& rules = atlas::rules::ServerRules::Get();
    rules.Clear();

    std::string config = "speed=2.5\ndamage=1.0\nhealth=3.0\n";
    size_t count = rules.LoadFromConfig(config);
    assert(count == 3);
    assert(rules.RuleCount() == 3);
    assert(rules.HasRule("speed"));
    assert(rules.HasRule("damage"));
    assert(rules.HasRule("health"));

    // Check multiplier values
    assert(rules.GetMultiplier("speed") > 2.4f && rules.GetMultiplier("speed") < 2.6f);
    assert(rules.GetMultiplier("damage") > 0.9f && rules.GetMultiplier("damage") < 1.1f);

    rules.Clear();
    std::cout << "[PASS] test_rules_load_from_config" << std::endl;
}

void test_rules_export_to_config() {
    auto& rules = atlas::rules::ServerRules::Get();
    rules.Clear();

    atlas::rules::RuleDescriptor desc;
    desc.name = "gravity";
    desc.multiplier = 1.5f;
    rules.RegisterRule(desc);

    std::string exported = rules.ExportToConfig();
    assert(!exported.empty());
    assert(exported.find("gravity=") != std::string::npos);

    rules.Clear();
    std::cout << "[PASS] test_rules_export_to_config" << std::endl;
}

void test_rules_hot_reload_only_reloadable() {
    auto& rules = atlas::rules::ServerRules::Get();
    rules.Clear();

    atlas::rules::RuleDescriptor speedRule;
    speedRule.name = "speed";
    speedRule.multiplier = 1.0f;
    speedRule.hotReloadable = true;
    rules.RegisterRule(speedRule);

    atlas::rules::RuleDescriptor damageRule;
    damageRule.name = "damage";
    damageRule.multiplier = 1.0f;
    damageRule.hotReloadable = false;
    rules.RegisterRule(damageRule);

    std::string config = "speed=3.0\ndamage=5.0\n";
    size_t updated = rules.HotReloadFromConfig(config);
    assert(updated == 1); // only speed should be updated

    assert(rules.GetMultiplier("speed") > 2.9f && rules.GetMultiplier("speed") < 3.1f);
    assert(rules.GetMultiplier("damage") > 0.9f && rules.GetMultiplier("damage") < 1.1f);

    rules.Clear();
    std::cout << "[PASS] test_rules_hot_reload_only_reloadable" << std::endl;
}

void test_rules_hot_reload_skips_non_reloadable() {
    auto& rules = atlas::rules::ServerRules::Get();
    rules.Clear();

    atlas::rules::RuleDescriptor desc;
    desc.name = "armor";
    desc.multiplier = 2.0f;
    desc.hotReloadable = false;
    rules.RegisterRule(desc);

    std::string config = "armor=9.0\n";
    size_t updated = rules.HotReloadFromConfig(config);
    assert(updated == 0);
    assert(rules.GetMultiplier("armor") > 1.9f && rules.GetMultiplier("armor") < 2.1f);

    rules.Clear();
    std::cout << "[PASS] test_rules_hot_reload_skips_non_reloadable" << std::endl;
}

void test_rules_pending_changes() {
    auto& rules = atlas::rules::ServerRules::Get();
    rules.Clear();

    assert(!rules.HasPendingChanges());

    atlas::rules::RuleDescriptor desc;
    desc.name = "regen";
    desc.multiplier = 1.0f;
    desc.hotReloadable = true;
    rules.RegisterRule(desc);

    rules.HotReloadFromConfig("regen=2.0\n");
    assert(rules.HasPendingChanges());

    rules.AcknowledgeChanges();
    assert(!rules.HasPendingChanges());

    rules.Clear();
    std::cout << "[PASS] test_rules_pending_changes" << std::endl;
}

void test_rules_last_modified_rules() {
    auto& rules = atlas::rules::ServerRules::Get();
    rules.Clear();

    atlas::rules::RuleDescriptor a;
    a.name = "alpha";
    a.multiplier = 1.0f;
    a.hotReloadable = true;
    rules.RegisterRule(a);

    atlas::rules::RuleDescriptor b;
    b.name = "beta";
    b.multiplier = 1.0f;
    b.hotReloadable = true;
    rules.RegisterRule(b);

    atlas::rules::RuleDescriptor c;
    c.name = "gamma";
    c.multiplier = 1.0f;
    c.hotReloadable = false;
    rules.RegisterRule(c);

    rules.HotReloadFromConfig("alpha=2.0\nbeta=3.0\ngamma=4.0\n");
    auto modified = rules.LastModifiedRules();
    assert(modified.size() == 2);

    // alpha and beta should be in the list
    assert(std::find(modified.begin(), modified.end(), "alpha") != modified.end());
    assert(std::find(modified.begin(), modified.end(), "beta") != modified.end());

    rules.AcknowledgeChanges();
    assert(rules.LastModifiedRules().empty());

    rules.Clear();
    std::cout << "[PASS] test_rules_last_modified_rules" << std::endl;
}

void test_rules_hot_reload_empty_config() {
    auto& rules = atlas::rules::ServerRules::Get();
    rules.Clear();

    atlas::rules::RuleDescriptor desc;
    desc.name = "test";
    desc.multiplier = 1.0f;
    desc.hotReloadable = true;
    rules.RegisterRule(desc);

    size_t updated = rules.HotReloadFromConfig("");
    assert(updated == 0);
    assert(!rules.HasPendingChanges());

    // Also test with non-existent rule names
    updated = rules.HotReloadFromConfig("nonexistent=5.0\n");
    assert(updated == 0);

    rules.Clear();
    std::cout << "[PASS] test_rules_hot_reload_empty_config" << std::endl;
}

// ============================================================
// Runner function — called from main.cpp
// ============================================================

void run_next_tasks_phase7_tests() {
    std::cout << "\n--- Phase 7: VulkanRenderer Pipeline Stages ---" << std::endl;
    test_vulkan_create_render_pass();
    test_vulkan_begin_end_render_pass();
    test_vulkan_create_pipeline_state();
    test_vulkan_bind_pipeline();
    test_vulkan_create_buffer();
    test_vulkan_map_unmap_buffer();
    test_vulkan_destroy_buffer();

    std::cout << "\n--- Phase 7: SocketHttpClient Tests ---" << std::endl;
    test_socket_http_parse_url_valid();
    test_socket_http_parse_url_with_port();
    test_socket_http_parse_url_no_path();
    test_socket_http_parse_url_https_rejected();
    test_socket_http_parse_url_invalid();
    test_socket_http_config_defaults();
    test_socket_http_request_count();

    std::cout << "\n--- Phase 7: Editor Permission Enforcement ---" << std::endl;
    test_attach_default_permission_tier();
    test_attach_set_permission_tier();
    test_attach_viewonly_permissions();
    test_attach_qa_permissions();
    test_attach_developer_permissions();
    test_attach_admin_permissions();
    test_attach_ci_permissions();
    test_attach_permission_description();

    std::cout << "\n--- Phase 7: Live Edit Rules Hot-Reload ---" << std::endl;
    test_rules_load_from_config();
    test_rules_export_to_config();
    test_rules_hot_reload_only_reloadable();
    test_rules_hot_reload_skips_non_reloadable();
    test_rules_pending_changes();
    test_rules_last_modified_rules();
    test_rules_hot_reload_empty_config();
}
