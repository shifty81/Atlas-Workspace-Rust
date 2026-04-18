#include <iostream>
#include <cassert>
#include <string>
#include <vector>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <filesystem>

#include "../engine/render/VulkanRenderer.h"
#include "../engine/ai/LLMBackend.h"
#include "../engine/production/BuildAuditLog.h"
#include "../engine/tools/ProjectNamespaceRewriter.h"

// ============================================================
// Task 1: VulkanRenderer — Push Constants & Shader Uniforms
// ============================================================

void test_vulkan_register_push_constant_range() {
    atlas::render::VulkanRenderer renderer;
    assert(renderer.PushConstantRangeCount() == 0);

    atlas::render::VkPushConstantRange range;
    range.name   = "ModelMatrix";
    range.offset = 0;
    range.size   = 64; // 4x4 float matrix

    uint32_t id = renderer.RegisterPushConstantRange(range);
    assert(id >= 1);
    assert(renderer.PushConstantRangeCount() == 1);

    const auto* r = renderer.GetPushConstantRange(id);
    assert(r != nullptr);
    assert(r->name   == "ModelMatrix");
    assert(r->offset == 0);
    assert(r->size   == 64);
    assert(r->id     == id);

    // Second range
    range.name   = "LightColor";
    range.offset = 64;
    range.size   = 16;
    uint32_t id2 = renderer.RegisterPushConstantRange(range);
    assert(id2 == id + 1);
    assert(renderer.PushConstantRangeCount() == 2);

    // Invalid id returns nullptr
    assert(renderer.GetPushConstantRange(0) == nullptr);
    assert(renderer.GetPushConstantRange(999) == nullptr);

    std::cout << "[PASS] test_vulkan_register_push_constant_range" << std::endl;
}

void test_vulkan_push_constants_data() {
    atlas::render::VulkanRenderer renderer;

    atlas::render::VkPushConstantRange range;
    range.name   = "Transform";
    range.offset = 0;
    range.size   = 16;
    uint32_t id = renderer.RegisterPushConstantRange(range);

    float data[4] = {1.0f, 2.0f, 3.0f, 4.0f};
    assert(renderer.PushConstants(id, data, 16));

    const auto& buf = renderer.PushConstantData(id);
    assert(buf.size() == 16);
    float out[4];
    std::memcpy(out, buf.data(), 16);
    assert(out[0] == 1.0f);
    assert(out[3] == 4.0f);

    std::cout << "[PASS] test_vulkan_push_constants_data" << std::endl;
}

void test_vulkan_push_constants_invalid() {
    atlas::render::VulkanRenderer renderer;

    atlas::render::VkPushConstantRange range;
    range.name   = "Small";
    range.offset = 0;
    range.size   = 8;
    uint32_t id = renderer.RegisterPushConstantRange(range);

    // Pushing too many bytes should fail
    float big[4] = {1, 2, 3, 4};
    assert(!renderer.PushConstants(id, big, 16)); // 16 > 8

    // Invalid id
    assert(!renderer.PushConstants(0, big, 4));
    assert(!renderer.PushConstants(999, big, 4));

    // Unregistered range returns empty data
    const auto& empty = renderer.PushConstantData(0);
    assert(empty.empty());

    std::cout << "[PASS] test_vulkan_push_constants_invalid" << std::endl;
}

void test_vulkan_bind_shader_uniform() {
    atlas::render::VulkanRenderer renderer;
    assert(renderer.UniformCount() == 0);

    atlas::render::VkShaderUniform u;
    u.name      = "u_ViewProj";
    u.binding   = 0;
    u.set       = 0;
    u.sizeBytes = 64;

    uint32_t id = renderer.BindUniform(u);
    assert(id >= 1);
    assert(renderer.UniformCount() == 1);

    const auto* fetched = renderer.GetUniform(id);
    assert(fetched != nullptr);
    assert(fetched->name    == "u_ViewProj");
    assert(fetched->binding == 0);
    assert(fetched->set     == 0);
    assert(fetched->id      == id);

    // Lookup by name
    const auto* byName = renderer.GetUniformByName("u_ViewProj");
    assert(byName != nullptr);
    assert(byName->id == id);

    // Unknown name returns nullptr
    assert(renderer.GetUniformByName("u_Nonexistent") == nullptr);

    std::cout << "[PASS] test_vulkan_bind_shader_uniform" << std::endl;
}

void test_vulkan_update_shader_uniform() {
    atlas::render::VulkanRenderer renderer;

    atlas::render::VkShaderUniform u;
    u.name      = "u_Time";
    u.binding   = 1;
    u.set       = 0;
    u.sizeBytes = 4;

    uint32_t id = renderer.BindUniform(u);

    float t = 1.5f;
    assert(renderer.UpdateUniform(id, &t, 4));

    const auto* updated = renderer.GetUniform(id);
    assert(updated != nullptr);
    assert(updated->data.size() == 4);
    float out = 0.0f;
    std::memcpy(&out, updated->data.data(), 4);
    assert(out == 1.5f);

    // Invalid id
    assert(!renderer.UpdateUniform(0, &t, 4));
    assert(!renderer.UpdateUniform(999, &t, 4));

    std::cout << "[PASS] test_vulkan_update_shader_uniform" << std::endl;
}

void test_vulkan_clear_uniforms() {
    atlas::render::VulkanRenderer renderer;

    atlas::render::VkShaderUniform u;
    u.name = "u_A"; u.binding = 0; u.set = 0; u.sizeBytes = 4;
    renderer.BindUniform(u);
    u.name = "u_B"; u.binding = 1;
    renderer.BindUniform(u);
    assert(renderer.UniformCount() == 2);

    renderer.ClearUniforms();
    assert(renderer.UniformCount() == 0);
    assert(renderer.GetUniformByName("u_A") == nullptr);

    std::cout << "[PASS] test_vulkan_clear_uniforms" << std::endl;
}

// ============================================================
// Task 2: LLM Backend (offline stub & registry)
// ============================================================

void test_llm_offline_backend_available() {
    atlas::ai::OfflineLLMBackend backend;
    assert(backend.IsAvailable());
    assert(backend.Name() == "OfflineStub");
    assert(backend.Capabilities() != 0);
    assert(backend.CallCount() == 0);
    assert(backend.ResponseCount() == 0);

    std::cout << "[PASS] test_llm_offline_backend_available" << std::endl;
}

void test_llm_offline_default_response() {
    atlas::ai::OfflineLLMBackend backend;

    atlas::ai::LLMRequest req;
    req.prompt    = "What is the meaning of life?";
    req.requestId = 42;

    auto resp = backend.Complete(req);
    assert(resp.success);
    assert(resp.requestId == 42);
    assert(!resp.text.empty());
    assert(backend.CallCount() == 1);

    std::cout << "[PASS] test_llm_offline_default_response" << std::endl;
}

void test_llm_offline_canned_response() {
    atlas::ai::OfflineLLMBackend backend;
    backend.RegisterResponse("generate code", "// generated code placeholder");
    backend.RegisterResponse("explain",       "This is an explanation.");

    assert(backend.ResponseCount() == 2);

    {
        atlas::ai::LLMRequest req;
        req.prompt = "generate code for sorting";
        auto resp = backend.Complete(req);
        assert(resp.success);
        assert(resp.text == "// generated code placeholder");
    }
    {
        atlas::ai::LLMRequest req;
        req.prompt = "explain the ECS system";
        auto resp = backend.Complete(req);
        assert(resp.success);
        assert(resp.text == "This is an explanation.");
    }

    // No match → default response
    {
        atlas::ai::LLMRequest req;
        req.prompt = "unknown prompt xyz";
        auto resp = backend.Complete(req);
        assert(resp.success);
        assert(resp.text.find("[offline]") != std::string::npos);
    }

    assert(backend.CallCount() == 3);

    backend.ClearResponses();
    assert(backend.ResponseCount() == 0);

    std::cout << "[PASS] test_llm_offline_canned_response" << std::endl;
}

void test_llm_registry_no_backend() {
    atlas::ai::LLMBackendRegistry registry;
    assert(!registry.HasRealBackend());
    assert(registry.GetBackend().Name() == "OfflineStub");
    assert(registry.RequestCount() == 0);

    std::cout << "[PASS] test_llm_registry_no_backend" << std::endl;
}

void test_llm_registry_complete() {
    atlas::ai::LLMBackendRegistry registry;

    auto resp = registry.Complete("Hello, Atlas AI");
    assert(resp.success);
    assert(!resp.text.empty());
    assert(registry.RequestCount() == 1);
    assert(registry.ResponseHistory().size() == 1);

    std::cout << "[PASS] test_llm_registry_complete" << std::endl;
}

void test_llm_registry_set_backend() {
    atlas::ai::LLMBackendRegistry registry;
    assert(!registry.HasRealBackend());

    // Provide a custom offline backend with a canned response.
    auto stub = std::make_shared<atlas::ai::OfflineLLMBackend>();
    stub->RegisterResponse("ping", "pong");
    registry.SetBackend(stub);
    assert(registry.HasRealBackend());
    assert(registry.GetBackend().Name() == "OfflineStub");

    auto resp = registry.Complete("ping the server");
    assert(resp.success);
    assert(resp.text == "pong");

    // Remove backend — falls back to internal offline stub.
    registry.SetBackend(nullptr);
    assert(!registry.HasRealBackend());

    std::cout << "[PASS] test_llm_registry_set_backend" << std::endl;
}

void test_llm_registry_history() {
    atlas::ai::LLMBackendRegistry registry;
    registry.Complete("one");
    registry.Complete("two");
    registry.Complete("three");

    assert(registry.RequestCount() == 3);
    const auto& hist = registry.ResponseHistory();
    assert(hist.size() == 3);

    registry.ClearHistory();
    assert(registry.RequestCount() == 0);

    std::cout << "[PASS] test_llm_registry_history" << std::endl;
}

// ============================================================
// Task 3: Build Audit Log
// ============================================================

void test_audit_log_open_close() {
    atlas::production::BuildAuditLog log;
    assert(!log.IsOpen());
    assert(log.EntryCount() == 0);

    log.Open("build-001");
    assert(log.IsOpen());
    assert(log.BuildId() == "build-001");
    assert(log.EntryCount() == 0);

    log.Close(5000);
    assert(!log.IsOpen());

    std::cout << "[PASS] test_audit_log_open_close" << std::endl;
}

void test_audit_log_append_entries() {
    atlas::production::BuildAuditLog log;
    log.Open("build-002");

    log.Info   ("CMake",       "Configuration complete");
    log.Warning("AssetCooker", "Asset 'mesh_tree.fbx' has no LOD");
    log.Error  ("Packager",    "Bundle failed", "disk full");

    assert(log.EntryCount() == 3);

    const auto& entries = log.Entries();
    assert(entries[0].severity == atlas::production::AuditSeverity::Info);
    assert(entries[0].actor    == "CMake");
    assert(entries[1].severity == atlas::production::AuditSeverity::Warning);
    assert(entries[2].severity == atlas::production::AuditSeverity::Error);
    assert(entries[2].detail   == "disk full");

    std::cout << "[PASS] test_audit_log_append_entries" << std::endl;
}

void test_audit_log_closed_no_append() {
    atlas::production::BuildAuditLog log;
    // Do NOT open — entries should be ignored.
    log.Info("Actor", "Should be ignored");
    assert(log.EntryCount() == 0);

    log.Open("build-003");
    log.Info("Actor", "Written");
    log.Close();
    assert(log.EntryCount() == 1);

    // After close, more entries are ignored.
    log.Info("Actor", "After close");
    assert(log.EntryCount() == 1);

    std::cout << "[PASS] test_audit_log_closed_no_append" << std::endl;
}

void test_audit_log_filter() {
    atlas::production::BuildAuditLog log;
    log.Open("build-004");

    log.Log(atlas::production::AuditSeverity::Info,    atlas::production::AuditCategory::Build,       "CMake",       "step1");
    log.Log(atlas::production::AuditSeverity::Warning, atlas::production::AuditCategory::AssetCook,   "Cooker",      "step2");
    log.Log(atlas::production::AuditSeverity::Error,   atlas::production::AuditCategory::Packaging,   "Packager",    "step3");
    log.Log(atlas::production::AuditSeverity::Info,    atlas::production::AuditCategory::Verification,"Verifier",    "step4");
    log.Log(atlas::production::AuditSeverity::Info,    atlas::production::AuditCategory::Build,       "CMake",       "step5");

    auto infos = log.FilterBySeverity(atlas::production::AuditSeverity::Info);
    assert(infos.size() == 3);

    auto warnings = log.FilterBySeverity(atlas::production::AuditSeverity::Warning);
    assert(warnings.size() == 1);

    auto buildCat = log.FilterByCategory(atlas::production::AuditCategory::Build);
    assert(buildCat.size() == 2);

    auto cmakeActor = log.FilterByActor("CMake");
    assert(cmakeActor.size() == 2);

    std::cout << "[PASS] test_audit_log_filter" << std::endl;
}

void test_audit_log_summary() {
    atlas::production::BuildAuditLog log;
    log.Open("build-005");

    log.Info   ("A", "msg");
    log.Warning("B", "msg");
    log.Error  ("C", "msg");
    log.Fatal  ("D", "msg");

    log.Close(12345);

    auto s = log.Summary();
    assert(s.buildId      == "build-005");
    assert(s.totalEntries == 4);
    assert(s.infoCount    == 1);
    assert(s.warningCount == 1);
    assert(s.errorCount   == 1);
    assert(s.fatalCount   == 1);
    assert(s.hasErrors    == true);
    assert(s.durationMs   == 12345);

    std::cout << "[PASS] test_audit_log_summary" << std::endl;
}

void test_audit_log_export_text() {
    atlas::production::BuildAuditLog log;
    log.Open("build-006");
    log.Info   ("Step1", "Build started");
    log.Warning("Step2", "Minor issue");

    std::string text = log.ExportText();
    assert(text.find("build-006")    != std::string::npos);
    assert(text.find("Build started") != std::string::npos);
    assert(text.find("WARN")         != std::string::npos);

    std::cout << "[PASS] test_audit_log_export_text" << std::endl;
}

void test_audit_log_export_json() {
    atlas::production::BuildAuditLog log;
    log.Open("build-007");
    log.Info("Builder", "Hello JSON");

    std::string json = log.ExportJSON();
    assert(json.find("\"build_id\"")  != std::string::npos);
    assert(json.find("build-007")     != std::string::npos);
    assert(json.find("Hello JSON")    != std::string::npos);

    std::cout << "[PASS] test_audit_log_export_json" << std::endl;
}

void test_audit_log_save_file() {
    const std::string path = "/tmp/atlas_audit_log_test.txt";
    std::filesystem::remove(path);

    atlas::production::BuildAuditLog log;
    log.Open("build-008");
    log.Info("CI", "All steps completed");
    log.Close(999);

    assert(log.SaveToFile(path));
    assert(std::filesystem::exists(path));

    std::ifstream in(path);
    std::string content((std::istreambuf_iterator<char>(in)),
                         std::istreambuf_iterator<char>());
    assert(content.find("build-008") != std::string::npos);
    assert(content.find("All steps completed") != std::string::npos);

    std::filesystem::remove(path);
    std::cout << "[PASS] test_audit_log_save_file" << std::endl;
}

void test_audit_log_callback() {
    atlas::production::BuildAuditLog log;
    log.Open("build-009");

    std::vector<atlas::production::AuditEntry> received;
    log.SetEntryCallback([&](const atlas::production::AuditEntry& e) {
        received.push_back(e);
    });

    log.Info("A", "first");
    log.Error("B", "second");

    assert(received.size() == 2);
    assert(received[0].message == "first");
    assert(received[1].severity == atlas::production::AuditSeverity::Error);

    std::cout << "[PASS] test_audit_log_callback" << std::endl;
}

void test_audit_log_reset() {
    atlas::production::BuildAuditLog log;
    log.Open("build-010");
    log.Info("X", "msg");
    assert(log.EntryCount() == 1);

    log.Reset();
    assert(!log.IsOpen());
    assert(log.EntryCount() == 0);
    assert(log.BuildId().empty());

    std::cout << "[PASS] test_audit_log_reset" << std::endl;
}

// ============================================================
// Task 4: Project Namespace Rewriter
// ============================================================

void test_rewriter_extensions() {
    atlas::tools::ProjectNamespaceRewriter rw;
    assert(rw.ExtensionCount() == 0);

    rw.RegisterExtension(".cpp");
    rw.RegisterExtension(".h");
    rw.RegisterExtension(".cmake");
    assert(rw.ExtensionCount() == 3);

    // Duplicate registration is ignored.
    rw.RegisterExtension(".cpp");
    assert(rw.ExtensionCount() == 3);

    assert(rw.ShouldRewrite("Engine.cpp"));
    assert(rw.ShouldRewrite("Config.h"));
    assert(rw.ShouldRewrite("Atlas.cmake"));
    assert(!rw.ShouldRewrite("README.md"));
    assert(!rw.ShouldRewrite("image.png"));

    std::cout << "[PASS] test_rewriter_extensions" << std::endl;
}

void test_rewriter_add_rules() {
    atlas::tools::ProjectNamespaceRewriter rw;
    assert(rw.RuleCount() == 0);

    rw.AddRule("AtlasForge", "MyGame");
    rw.AddRule("atlas::",    "mygame::");
    assert(rw.RuleCount() == 2);

    rw.ClearRules();
    assert(rw.RuleCount() == 0);

    std::cout << "[PASS] test_rewriter_add_rules" << std::endl;
}

void test_rewriter_apply_rules() {
    atlas::tools::ProjectNamespaceRewriter rw;
    rw.AddRule("AtlasForge", "MyGame");
    rw.AddRule("atlas::",    "mygame::");

    std::string content = "namespace atlas::core { /* AtlasForge engine */ }\n"
                          "atlas::engine::Init();\n";

    uint32_t count = 0;
    std::string result = rw.ApplyRules(content, &count);

    assert(result.find("AtlasForge") == std::string::npos);
    assert(result.find("MyGame")     != std::string::npos);
    assert(result.find("mygame::")   != std::string::npos);
    assert(count > 0);

    // No rules — content unchanged
    atlas::tools::ProjectNamespaceRewriter rw2;
    uint32_t cnt2 = 99;
    std::string unchanged = rw2.ApplyRules("hello", &cnt2);
    assert(unchanged == "hello");
    assert(cnt2 == 0);

    std::cout << "[PASS] test_rewriter_apply_rules" << std::endl;
}

void test_rewriter_rewrite_file() {
    const std::string srcDir = "/tmp/atlas_rw_test_src";
    const std::string outDir = "/tmp/atlas_rw_test_out";
    std::filesystem::create_directories(srcDir);
    std::filesystem::create_directories(outDir);

    const std::string srcFile = srcDir + "/Engine.h";
    const std::string outFile = outDir + "/Engine.h";

    {
        std::ofstream out(srcFile);
        out << "#pragma once\n"
               "namespace AtlasForge { class Engine {}; }\n";
    }

    atlas::tools::ProjectNamespaceRewriter rw;
    rw.RegisterExtension(".h");
    rw.AddRule("AtlasForge", "MyGame");

    auto result = rw.RewriteFile(srcFile, outFile);
    assert(result.success);
    assert(result.substitutionCount == 1);

    std::ifstream in(outFile);
    std::string content((std::istreambuf_iterator<char>(in)),
                         std::istreambuf_iterator<char>());
    assert(content.find("MyGame")     != std::string::npos);
    assert(content.find("AtlasForge") == std::string::npos);

    std::filesystem::remove_all(srcDir);
    std::filesystem::remove_all(outDir);
    std::cout << "[PASS] test_rewriter_rewrite_file" << std::endl;
}

void test_rewriter_rewrite_file_not_found() {
    atlas::tools::ProjectNamespaceRewriter rw;
    rw.AddRule("A", "B");

    auto result = rw.RewriteFile("/nonexistent/path/Engine.h", "/tmp/out.h");
    assert(!result.success);
    assert(!result.errorMessage.empty());

    std::cout << "[PASS] test_rewriter_rewrite_file_not_found" << std::endl;
}

void test_rewriter_rewrite_directory() {
    const std::string srcDir = "/tmp/atlas_rw_dir_src";
    const std::string outDir = "/tmp/atlas_rw_dir_out";
    std::filesystem::create_directories(srcDir + "/sub");

    auto writeFile = [](const std::string& path, const std::string& content) {
        std::ofstream out(path);
        out << content;
    };

    writeFile(srcDir + "/Main.cpp",       "// AtlasForge main\natlasMain();\n");
    writeFile(srcDir + "/Config.h",       "#pragma once // AtlasForge config\n");
    writeFile(srcDir + "/sub/Module.cpp", "namespace AtlasForge {}\n");
    writeFile(srcDir + "/README.md",      "# AtlasForge\nNot a source file.\n");

    atlas::tools::ProjectNamespaceRewriter rw;
    rw.RegisterExtension(".cpp");
    rw.RegisterExtension(".h");
    rw.AddRule("AtlasForge", "MyGame");
    rw.AddRule("atlasMain",  "myMain");

    std::vector<std::string> processed;
    rw.SetProgressCallback([&](const atlas::tools::RewriteResult& r) {
        processed.push_back(r.sourcePath);
    });

    auto run = rw.RewriteDirectory(srcDir, outDir);

    // 4 files scanned (3 eligible + README.md)
    assert(run.filesScanned  >= 4);
    // 3 .cpp/.h files modified, README skipped
    assert(run.filesModified == 3);
    assert(run.filesSkipped  >= 1);
    assert(!run.hasErrors);
    assert(run.totalSubstitutions > 0);
    assert(processed.size() == 3);

    // Verify output content
    {
        std::ifstream in(outDir + "/Main.cpp");
        std::string c((std::istreambuf_iterator<char>(in)),
                       std::istreambuf_iterator<char>());
        assert(c.find("AtlasForge") == std::string::npos);
        assert(c.find("MyGame") != std::string::npos);
        assert(c.find("myMain") != std::string::npos);
    }

    std::filesystem::remove_all(srcDir);
    std::filesystem::remove_all(outDir);
    std::cout << "[PASS] test_rewriter_rewrite_directory" << std::endl;
}

void test_rewriter_validate_rules() {
    atlas::tools::ProjectNamespaceRewriter rw;
    rw.AddRule("Atlas", "MyGame");  // valid
    rw.AddRule("",      "Bad");     // empty from
    rw.AddRule("Good",  "");        // empty to
    rw.AddRule("Same",  "Same");    // from == to

    auto errors = rw.ValidateRules();
    assert(errors.size() == 3);

    // Only the first rule is valid
    atlas::tools::ProjectNamespaceRewriter rw2;
    rw2.AddRule("A", "B");
    rw2.AddRule("C", "D");
    assert(rw2.ValidateRules().empty());

    std::cout << "[PASS] test_rewriter_validate_rules" << std::endl;
}

// ============================================================
// Runner function — called from main.cpp
// ============================================================

void run_next_tasks_phase10_tests() {
    std::cout << "\n--- Phase 10: VulkanRenderer Push Constants & Shader Uniforms ---" << std::endl;
    test_vulkan_register_push_constant_range();
    test_vulkan_push_constants_data();
    test_vulkan_push_constants_invalid();
    test_vulkan_bind_shader_uniform();
    test_vulkan_update_shader_uniform();
    test_vulkan_clear_uniforms();

    std::cout << "\n--- Phase 10: LLM Backend (Offline Stub & Registry) ---" << std::endl;
    test_llm_offline_backend_available();
    test_llm_offline_default_response();
    test_llm_offline_canned_response();
    test_llm_registry_no_backend();
    test_llm_registry_complete();
    test_llm_registry_set_backend();
    test_llm_registry_history();

    std::cout << "\n--- Phase 10: Build Audit Log ---" << std::endl;
    test_audit_log_open_close();
    test_audit_log_append_entries();
    test_audit_log_closed_no_append();
    test_audit_log_filter();
    test_audit_log_summary();
    test_audit_log_export_text();
    test_audit_log_export_json();
    test_audit_log_save_file();
    test_audit_log_callback();
    test_audit_log_reset();

    std::cout << "\n--- Phase 10: Project Namespace Rewriter ---" << std::endl;
    test_rewriter_extensions();
    test_rewriter_add_rules();
    test_rewriter_apply_rules();
    test_rewriter_rewrite_file();
    test_rewriter_rewrite_file_not_found();
    test_rewriter_rewrite_directory();
    test_rewriter_validate_rules();
}
