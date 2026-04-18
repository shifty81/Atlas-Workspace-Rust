#include <iostream>
#include <cassert>
#include <string>
#include <vector>
#include <algorithm>

#include "../engine/render/VulkanRenderer.h"
#include "../engine/net/NetHardening.h"
#include "../engine/assets/AssetRegistry.h"
#include "../editor/panels/MaterialEditorPanel.h"

// ============================================================
// Task 1: VulkanRenderer Descriptor Sets & Texture Management
// ============================================================

void test_vulkan_create_descriptor_set_layout() {
    atlas::render::VulkanRenderer renderer;
    atlas::render::VkDescriptorSetLayoutDesc desc;
    desc.name = "GlobalLayout";
    desc.bindingCount = 3;

    uint32_t id = renderer.CreateDescriptorSetLayout(desc);
    assert(id >= 1);
    assert(renderer.DescriptorSetLayoutCount() == 1);

    const auto* layout = renderer.GetDescriptorSetLayout(id);
    assert(layout != nullptr);
    assert(layout->name == "GlobalLayout");
    assert(layout->bindingCount == 3);
    assert(layout->id == id);

    // Create a second layout
    desc.name = "MaterialLayout";
    desc.bindingCount = 5;
    uint32_t id2 = renderer.CreateDescriptorSetLayout(desc);
    assert(id2 == id + 1);
    assert(renderer.DescriptorSetLayoutCount() == 2);

    // Invalid id returns nullptr
    assert(renderer.GetDescriptorSetLayout(0) == nullptr);
    assert(renderer.GetDescriptorSetLayout(999) == nullptr);

    std::cout << "[PASS] test_vulkan_create_descriptor_set_layout" << std::endl;
}

void test_vulkan_bind_descriptor_set() {
    atlas::render::VulkanRenderer renderer;
    assert(renderer.BoundDescriptorSetId() == 0);

    atlas::render::VkDescriptorSetLayoutDesc desc;
    desc.name = "TestLayout";
    desc.bindingCount = 2;
    uint32_t id = renderer.CreateDescriptorSetLayout(desc);

    renderer.BindDescriptorSet(id);
    assert(renderer.BoundDescriptorSetId() == id);

    // Bind invalid should not change
    renderer.BindDescriptorSet(0);
    assert(renderer.BoundDescriptorSetId() == id);
    renderer.BindDescriptorSet(999);
    assert(renderer.BoundDescriptorSetId() == id);

    std::cout << "[PASS] test_vulkan_bind_descriptor_set" << std::endl;
}

void test_vulkan_create_texture() {
    atlas::render::VulkanRenderer renderer;
    assert(renderer.TextureCount() == 0);

    atlas::render::VkTextureDesc desc;
    desc.name = "Diffuse";
    desc.width = 1024;
    desc.height = 1024;
    desc.mipLevels = 4;

    uint32_t id = renderer.CreateTexture(desc);
    assert(id >= 1);
    assert(renderer.TextureCount() == 1);

    const auto* tex = renderer.GetTexture(id);
    assert(tex != nullptr);
    assert(tex->name == "Diffuse");
    assert(tex->width == 1024);
    assert(tex->height == 1024);
    assert(tex->mipLevels == 4);
    assert(tex->id == id);

    // Invalid id
    assert(renderer.GetTexture(0) == nullptr);
    assert(renderer.GetTexture(999) == nullptr);

    std::cout << "[PASS] test_vulkan_create_texture" << std::endl;
}

void test_vulkan_destroy_texture() {
    atlas::render::VulkanRenderer renderer;
    atlas::render::VkTextureDesc desc;
    desc.name = "Normal";
    desc.width = 512;
    desc.height = 512;

    uint32_t id1 = renderer.CreateTexture(desc);
    desc.name = "Specular";
    uint32_t id2 = renderer.CreateTexture(desc);
    assert(renderer.TextureCount() == 2);

    assert(renderer.DestroyTexture(id1));
    assert(renderer.TextureCount() == 1);
    assert(renderer.GetTexture(id1) == nullptr);
    assert(renderer.GetTexture(id2) != nullptr);

    // Destroy same texture again should fail
    assert(!renderer.DestroyTexture(id1));

    // Destroy invalid
    assert(!renderer.DestroyTexture(0));
    assert(!renderer.DestroyTexture(999));

    std::cout << "[PASS] test_vulkan_destroy_texture" << std::endl;
}

void test_vulkan_create_sampler() {
    atlas::render::VulkanRenderer renderer;
    assert(renderer.SamplerCount() == 0);

    atlas::render::VkSamplerDesc desc;
    desc.name = "LinearClamp";
    desc.linearFilter = true;
    desc.clampToEdge = true;

    uint32_t id = renderer.CreateSampler(desc);
    assert(id >= 1);
    assert(renderer.SamplerCount() == 1);

    const auto* sampler = renderer.GetSampler(id);
    assert(sampler != nullptr);
    assert(sampler->name == "LinearClamp");
    assert(sampler->linearFilter == true);
    assert(sampler->clampToEdge == true);
    assert(sampler->id == id);

    // Invalid id
    assert(renderer.GetSampler(0) == nullptr);
    assert(renderer.GetSampler(999) == nullptr);

    std::cout << "[PASS] test_vulkan_create_sampler" << std::endl;
}

void test_vulkan_destroy_sampler() {
    atlas::render::VulkanRenderer renderer;
    atlas::render::VkSamplerDesc desc;
    desc.name = "NearestRepeat";
    desc.linearFilter = false;
    desc.clampToEdge = false;

    uint32_t id1 = renderer.CreateSampler(desc);
    desc.name = "LinearRepeat";
    uint32_t id2 = renderer.CreateSampler(desc);
    assert(renderer.SamplerCount() == 2);

    assert(renderer.DestroySampler(id1));
    assert(renderer.SamplerCount() == 1);
    assert(renderer.GetSampler(id1) == nullptr);
    assert(renderer.GetSampler(id2) != nullptr);

    // Destroy same sampler again should fail
    assert(!renderer.DestroySampler(id1));

    // Destroy invalid
    assert(!renderer.DestroySampler(0));
    assert(!renderer.DestroySampler(999));

    std::cout << "[PASS] test_vulkan_destroy_sampler" << std::endl;
}

void test_vulkan_descriptor_texture_sampler_counts() {
    atlas::render::VulkanRenderer renderer;
    assert(renderer.DescriptorSetLayoutCount() == 0);
    assert(renderer.TextureCount() == 0);
    assert(renderer.SamplerCount() == 0);

    atlas::render::VkDescriptorSetLayoutDesc dslDesc;
    dslDesc.name = "L1";
    renderer.CreateDescriptorSetLayout(dslDesc);
    dslDesc.name = "L2";
    renderer.CreateDescriptorSetLayout(dslDesc);

    atlas::render::VkTextureDesc texDesc;
    texDesc.name = "T1";
    texDesc.width = 256;
    texDesc.height = 256;
    renderer.CreateTexture(texDesc);

    atlas::render::VkSamplerDesc sampDesc;
    sampDesc.name = "S1";
    renderer.CreateSampler(sampDesc);
    sampDesc.name = "S2";
    renderer.CreateSampler(sampDesc);
    sampDesc.name = "S3";
    renderer.CreateSampler(sampDesc);

    assert(renderer.DescriptorSetLayoutCount() == 2);
    assert(renderer.TextureCount() == 1);
    assert(renderer.SamplerCount() == 3);

    std::cout << "[PASS] test_vulkan_descriptor_texture_sampler_counts" << std::endl;
}

// ============================================================
// Task 2: NetContext Packet Loss Simulation & Connection Quality
// ============================================================

void test_net_packet_loss_config() {
    atlas::net::NetHardening net;
    atlas::net::PacketLossSimConfig config;
    config.lossPercent = 10.0f;
    config.latencyMs = 50.0f;
    config.jitterMs = 5.0f;
    config.enabled = true;

    net.SetPacketLossSimulation(config);
    const auto& c = net.PacketLossSimulation();
    assert(c.enabled == true);
    assert(c.lossPercent > 9.9f && c.lossPercent < 10.1f);
    assert(c.latencyMs > 49.9f && c.latencyMs < 50.1f);
    assert(c.jitterMs > 4.9f && c.jitterMs < 5.1f);

    std::cout << "[PASS] test_net_packet_loss_config" << std::endl;
}

void test_net_should_drop_packet() {
    atlas::net::NetHardening net;
    atlas::net::PacketLossSimConfig config;
    config.lossPercent = 10.0f;
    config.enabled = true;
    net.SetPacketLossSimulation(config);

    // With 10% loss, first 10 of every 100 should drop
    int dropped = 0;
    for (int i = 0; i < 100; ++i) {
        if (net.ShouldDropPacket()) dropped++;
    }
    assert(dropped == 10);

    // Disabled should not drop
    config.enabled = false;
    net.SetPacketLossSimulation(config);
    assert(!net.ShouldDropPacket());

    std::cout << "[PASS] test_net_should_drop_packet" << std::endl;
}

void test_net_connection_quality_excellent() {
    atlas::net::NetHardening net;
    net.RecordRtt(10.0f);
    // No packets dropped, packetsSent=0, so loss=0%
    assert(net.GetConnectionQuality() == atlas::net::ConnectionQuality::Excellent);

    std::cout << "[PASS] test_net_connection_quality_excellent" << std::endl;
}

void test_net_connection_quality_poor() {
    atlas::net::NetHardening net;
    net.RecordRtt(200.0f);
    // RTT >= 150, but < 300 and loss < 15%
    assert(net.GetConnectionQuality() == atlas::net::ConnectionQuality::Poor);

    std::cout << "[PASS] test_net_connection_quality_poor" << std::endl;
}

void test_net_connection_quality_string() {
    atlas::net::NetHardening net;
    net.RecordRtt(10.0f);
    assert(net.ConnectionQualityString() == "Excellent");

    atlas::net::NetHardening net2;
    net2.RecordRtt(200.0f);
    assert(net2.ConnectionQualityString() == "Poor");

    atlas::net::NetHardening net3;
    net3.RecordRtt(500.0f);
    assert(net3.ConnectionQualityString() == "Critical");

    std::cout << "[PASS] test_net_connection_quality_string" << std::endl;
}

void test_net_packet_loss_percent() {
    atlas::net::NetHardening net;
    // Initially 0%
    assert(net.PacketLossPercent() < 0.01f);

    net.RecordPacketSent();
    net.RecordPacketSent();
    net.RecordPacketSent();
    net.RecordPacketSent();
    net.RecordPacketDropped();
    // total = 4 sent + 0 received = 4, dropped = 1
    // loss = 1/4 * 100 = 25%
    float loss = net.PacketLossPercent();
    assert(loss > 24.0f && loss < 26.0f);

    std::cout << "[PASS] test_net_packet_loss_percent" << std::endl;
}

void test_net_average_bandwidth() {
    atlas::net::NetHardening net;
    // Initially 0
    assert(net.AverageBandwidthBytesPerSec() < 0.01f);

    // Send 1000 bytes over 1000ms
    net.RecordBytesSent(1000);
    net.Update(1000.0f);

    float bw = net.AverageBandwidthBytesPerSec();
    assert(bw > 900.0f && bw < 1100.0f);

    std::cout << "[PASS] test_net_average_bandwidth" << std::endl;
}

// ============================================================
// Task 3: AssetRegistry Dependency Tracking
// ============================================================

void test_asset_add_dependency() {
    atlas::asset::AssetRegistry registry;
    registry.AddDependency("meshA", "textureB");
    registry.AddDependency("meshA", "shaderC");

    auto deps = registry.GetDependencies("meshA");
    assert(deps.size() == 2);
    assert(std::find(deps.begin(), deps.end(), "textureB") != deps.end());
    assert(std::find(deps.begin(), deps.end(), "shaderC") != deps.end());

    // Adding duplicate should not increase count
    registry.AddDependency("meshA", "textureB");
    assert(registry.GetDependencies("meshA").size() == 2);

    registry.ClearDependencies();
    std::cout << "[PASS] test_asset_add_dependency" << std::endl;
}

void test_asset_remove_dependency() {
    atlas::asset::AssetRegistry registry;
    registry.AddDependency("meshA", "textureB");
    registry.AddDependency("meshA", "shaderC");
    assert(registry.GetDependencies("meshA").size() == 2);

    registry.RemoveDependency("meshA", "textureB");
    auto deps = registry.GetDependencies("meshA");
    assert(deps.size() == 1);
    assert(deps[0] == "shaderC");

    // Remove non-existent should be safe
    registry.RemoveDependency("meshA", "nonexistent");
    assert(registry.GetDependencies("meshA").size() == 1);

    registry.ClearDependencies();
    std::cout << "[PASS] test_asset_remove_dependency" << std::endl;
}

void test_asset_get_dependents() {
    atlas::asset::AssetRegistry registry;
    registry.AddDependency("meshA", "textureX");
    registry.AddDependency("meshB", "textureX");
    registry.AddDependency("meshC", "textureX");

    auto dependents = registry.GetDependents("textureX");
    assert(dependents.size() == 3);
    assert(std::find(dependents.begin(), dependents.end(), "meshA") != dependents.end());
    assert(std::find(dependents.begin(), dependents.end(), "meshB") != dependents.end());
    assert(std::find(dependents.begin(), dependents.end(), "meshC") != dependents.end());

    // No dependents for unknown asset
    assert(registry.GetDependents("unknown").empty());

    registry.ClearDependencies();
    std::cout << "[PASS] test_asset_get_dependents" << std::endl;
}

void test_asset_circular_dependency() {
    atlas::asset::AssetRegistry registry;
    registry.AddDependency("A", "B");
    registry.AddDependency("B", "C");
    registry.AddDependency("C", "A");

    assert(registry.HasCircularDependency("A"));
    assert(registry.HasCircularDependency("B"));
    assert(registry.HasCircularDependency("C"));

    registry.ClearDependencies();
    std::cout << "[PASS] test_asset_circular_dependency" << std::endl;
}

void test_asset_no_circular_dependency() {
    atlas::asset::AssetRegistry registry;
    registry.AddDependency("A", "B");
    registry.AddDependency("B", "C");

    assert(!registry.HasCircularDependency("A"));
    assert(!registry.HasCircularDependency("B"));
    assert(!registry.HasCircularDependency("C"));

    registry.ClearDependencies();
    std::cout << "[PASS] test_asset_no_circular_dependency" << std::endl;
}

void test_asset_resolve_build_order() {
    atlas::asset::AssetRegistry registry;
    registry.AddDependency("app", "lib");
    registry.AddDependency("lib", "core");

    auto order = registry.ResolveBuildOrder();
    assert(order.size() == 3);

    // core should come before lib, lib before app
    auto posCore = std::find(order.begin(), order.end(), "core");
    auto posLib = std::find(order.begin(), order.end(), "lib");
    auto posApp = std::find(order.begin(), order.end(), "app");
    assert(posCore != order.end());
    assert(posLib != order.end());
    assert(posApp != order.end());
    assert(posCore < posLib);
    assert(posLib < posApp);

    registry.ClearDependencies();
    std::cout << "[PASS] test_asset_resolve_build_order" << std::endl;
}

void test_asset_clear_dependencies() {
    atlas::asset::AssetRegistry registry;
    registry.AddDependency("A", "B");
    registry.AddDependency("C", "D");
    assert(registry.GetDependencies("A").size() == 1);
    assert(registry.GetDependencies("C").size() == 1);

    registry.ClearDependencies();
    assert(registry.GetDependencies("A").empty());
    assert(registry.GetDependencies("C").empty());

    std::cout << "[PASS] test_asset_clear_dependencies" << std::endl;
}

// ============================================================
// Task 4: MaterialEditorPanel Draw & Shader Parameter Support
// ============================================================

void test_material_editor_draw() {
    atlas::editor::MaterialEditorPanel panel;
    atlas::procedural::MaterialData mat;
    mat.width = 256;
    mat.height = 256;
    panel.LoadMaterial(mat);

    atlas::editor::MaterialParameter param;
    param.name = "roughness";
    param.type = "float";
    param.value = "0.5";
    panel.AddParameter(param);

    panel.Draw();
    const auto& drawList = panel.GetDrawList();
    assert(drawList.CommandCount() > 0);

    std::cout << "[PASS] test_material_editor_draw" << std::endl;
}

void test_material_editor_shader_path() {
    atlas::editor::MaterialEditorPanel panel;
    assert(panel.ShaderPath().empty());

    panel.SetShaderPath("shaders/pbr.glsl");
    assert(panel.ShaderPath() == "shaders/pbr.glsl");

    panel.SetShaderPath("shaders/toon.glsl");
    assert(panel.ShaderPath() == "shaders/toon.glsl");

    std::cout << "[PASS] test_material_editor_shader_path" << std::endl;
}

void test_material_editor_parameter_names() {
    atlas::editor::MaterialEditorPanel panel;
    assert(panel.ParameterNames().empty());

    atlas::editor::MaterialParameter p1;
    p1.name = "albedo";
    p1.type = "color";
    p1.value = "#ffffff";
    panel.AddParameter(p1);

    atlas::editor::MaterialParameter p2;
    p2.name = "metallic";
    p2.type = "float";
    p2.value = "1.0";
    panel.AddParameter(p2);

    auto names = panel.ParameterNames();
    assert(names.size() == 2);
    assert(names[0] == "albedo");
    assert(names[1] == "metallic");

    std::cout << "[PASS] test_material_editor_parameter_names" << std::endl;
}

void test_material_editor_parameter_count() {
    atlas::editor::MaterialEditorPanel panel;
    assert(panel.ParameterCount() == 0);

    atlas::editor::MaterialParameter p;
    p.name = "normal";
    p.type = "texture";
    p.value = "default_normal.png";
    panel.AddParameter(p);
    assert(panel.ParameterCount() == 1);

    p.name = "ao";
    panel.AddParameter(p);
    assert(panel.ParameterCount() == 2);

    panel.RemoveParameter("normal");
    assert(panel.ParameterCount() == 1);

    std::cout << "[PASS] test_material_editor_parameter_count" << std::endl;
}

void test_material_editor_select_parameter_p8() {
    atlas::editor::MaterialEditorPanel panel;
    atlas::editor::MaterialParameter p1;
    p1.name = "color";
    p1.type = "color";
    p1.value = "#ff0000";
    panel.AddParameter(p1);

    atlas::editor::MaterialParameter p2;
    p2.name = "opacity";
    p2.type = "float";
    p2.value = "0.8";
    panel.AddParameter(p2);

    panel.SelectParameter(0);
    assert(panel.SelectedParameter() == 0);

    panel.SelectParameter(1);
    assert(panel.SelectedParameter() == 1);

    // Invalid index resets
    panel.SelectParameter(99);
    assert(panel.SelectedParameter() == -1);

    std::cout << "[PASS] test_material_editor_select_parameter_p8" << std::endl;
}

void test_material_editor_preview_mode_p8() {
    atlas::editor::MaterialEditorPanel panel;
    assert(!panel.PreviewMode());

    panel.SetPreviewMode(true);
    assert(panel.PreviewMode());

    // Draw with preview mode
    atlas::procedural::MaterialData mat;
    mat.width = 128;
    mat.height = 128;
    panel.LoadMaterial(mat);
    panel.Draw();

    const auto& drawList = panel.GetDrawList();
    bool foundPreviewTitle = false;
    for (size_t i = 0; i < drawList.CommandCount(); ++i) {
        const auto& cmd = drawList.Commands()[i];
        if (cmd.text.find("[Preview]") != std::string::npos) {
            foundPreviewTitle = true;
            break;
        }
    }
    assert(foundPreviewTitle);

    panel.SetPreviewMode(false);
    assert(!panel.PreviewMode());

    std::cout << "[PASS] test_material_editor_preview_mode_p8" << std::endl;
}

void test_material_editor_summary_p8() {
    atlas::editor::MaterialEditorPanel panel;
    assert(panel.Summary() == "No material loaded");

    atlas::procedural::MaterialData mat;
    mat.width = 512;
    mat.height = 512;
    panel.LoadMaterial(mat);

    atlas::editor::MaterialParameter p;
    p.name = "test";
    p.type = "float";
    p.value = "1.0";
    panel.AddParameter(p);

    std::string summary = panel.Summary();
    assert(summary.find("512x512") != std::string::npos);
    assert(summary.find("1 params") != std::string::npos);
    assert(summary.find("(modified)") != std::string::npos);

    panel.MarkClean();
    summary = panel.Summary();
    assert(summary.find("(modified)") == std::string::npos);

    std::cout << "[PASS] test_material_editor_summary_p8" << std::endl;
}

// ============================================================
// Runner function — called from main.cpp
// ============================================================

void run_next_tasks_phase8_tests() {
    std::cout << "\n--- Phase 8: VulkanRenderer Descriptor Sets & Textures ---" << std::endl;
    test_vulkan_create_descriptor_set_layout();
    test_vulkan_bind_descriptor_set();
    test_vulkan_create_texture();
    test_vulkan_destroy_texture();
    test_vulkan_create_sampler();
    test_vulkan_destroy_sampler();
    test_vulkan_descriptor_texture_sampler_counts();

    std::cout << "\n--- Phase 8: NetContext Packet Loss & Connection Quality ---" << std::endl;
    test_net_packet_loss_config();
    test_net_should_drop_packet();
    test_net_connection_quality_excellent();
    test_net_connection_quality_poor();
    test_net_connection_quality_string();
    test_net_packet_loss_percent();
    test_net_average_bandwidth();

    std::cout << "\n--- Phase 8: AssetRegistry Dependency Tracking ---" << std::endl;
    test_asset_add_dependency();
    test_asset_remove_dependency();
    test_asset_get_dependents();
    test_asset_circular_dependency();
    test_asset_no_circular_dependency();
    test_asset_resolve_build_order();
    test_asset_clear_dependencies();

    std::cout << "\n--- Phase 8: MaterialEditorPanel Draw & Shader Parameters ---" << std::endl;
    test_material_editor_draw();
    test_material_editor_shader_path();
    test_material_editor_parameter_names();
    test_material_editor_parameter_count();
    test_material_editor_select_parameter_p8();
    test_material_editor_preview_mode_p8();
    test_material_editor_summary_p8();
}
