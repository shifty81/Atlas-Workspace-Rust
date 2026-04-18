#include <iostream>
#include <cassert>
#include <cstdio>
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>

#include "../engine/render/VulkanRenderer.h"
#include "../editor/ai/AIAggregator.h"
#include "../editor/ai/TemplateAIBackend.h"
#include "../engine/ui/FontBootstrap.h"
#include "../engine/assets/HttpClient.h"
#include "../engine/assets/MarketplaceImporter.h"

// ============================================================
// Task 1: VulkanRenderer GPU submission pipeline
// ============================================================

void test_vulkan_submit_command_buffer() {
    atlas::render::VulkanRenderer renderer;
    renderer.BeginFrame();
    renderer.DrawRect({0, 0, 100, 100}, {255, 0, 0, 255});
    renderer.DrawText({0, 0, 200, 20}, "Hello", {200, 200, 200, 255});

    assert(renderer.HasPendingCommands());
    renderer.SubmitCommandBuffer();
    assert(renderer.SubmittedBufferCount() == 1);

    const auto& buf = renderer.LastSubmittedBuffer();
    assert(buf.submitted);
    assert(buf.commands.size() == 2);
    assert(buf.frameIndex == 0);
    assert(buf.submitTimestamp == 0);

    renderer.EndFrame();
    std::cout << "[PASS] test_vulkan_submit_command_buffer" << std::endl;
}

void test_vulkan_auto_submit_on_end_frame() {
    atlas::render::VulkanRenderer renderer;
    renderer.BeginFrame();
    renderer.DrawRect({0, 0, 50, 50}, {0, 255, 0, 255});
    renderer.EndFrame();

    // EndFrame should have auto-submitted
    assert(renderer.SubmittedBufferCount() == 1);
    const auto& buf = renderer.LastSubmittedBuffer();
    assert(buf.submitted);
    assert(buf.commands.size() == 1);

    std::cout << "[PASS] test_vulkan_auto_submit_on_end_frame" << std::endl;
}

void test_vulkan_no_submit_empty_frame() {
    atlas::render::VulkanRenderer renderer;
    renderer.BeginFrame();
    // No draw commands
    renderer.EndFrame();

    assert(renderer.SubmittedBufferCount() == 0);
    std::cout << "[PASS] test_vulkan_no_submit_empty_frame" << std::endl;
}

void test_vulkan_triple_buffering() {
    atlas::render::VulkanRenderer renderer;

    // Submit 4 frames; SubmittedBufferCount tracks total submissions,
    // while the internal ring buffer keeps only the last MAX_BUFFERED_FRAMES
    for (uint32_t i = 0; i < 4; ++i) {
        renderer.BeginFrame();
        renderer.DrawRect({0, 0, 10, 10}, {255, 255, 255, 255});
        renderer.EndFrame();
    }

    assert(renderer.SubmittedBufferCount() == 4); // total submissions
    assert(renderer.FrameCount() == 4);

    // Last submitted buffer should be from frame 3
    const auto& buf = renderer.LastSubmittedBuffer();
    assert(buf.frameIndex == 3);
    assert(buf.submitTimestamp == 3);

    std::cout << "[PASS] test_vulkan_triple_buffering" << std::endl;
}

void test_vulkan_submit_preserves_commands() {
    atlas::render::VulkanRenderer renderer;
    renderer.BeginFrame();
    renderer.DrawRect({10, 20, 30, 40}, {100, 150, 200, 255});
    renderer.DrawIcon({5, 5, 16, 16}, 42, {255, 255, 255, 255});
    renderer.DrawBorder({0, 0, 100, 100}, 2, {128, 128, 128, 255});
    renderer.DrawImage({0, 0, 64, 64}, 7, {200, 200, 200, 255});
    renderer.EndFrame();

    const auto& buf = renderer.LastSubmittedBuffer();
    assert(buf.commands.size() == 4);
    assert(buf.commands[0].kind == atlas::render::VkDrawCommand::Kind::Rect);
    assert(buf.commands[1].kind == atlas::render::VkDrawCommand::Kind::Icon);
    assert(buf.commands[1].resourceId == 42);
    assert(buf.commands[2].kind == atlas::render::VkDrawCommand::Kind::Border);
    assert(buf.commands[2].thickness == 2);
    assert(buf.commands[3].kind == atlas::render::VkDrawCommand::Kind::Image);
    assert(buf.commands[3].resourceId == 7);

    std::cout << "[PASS] test_vulkan_submit_preserves_commands" << std::endl;
}

void test_vulkan_has_pending_commands() {
    atlas::render::VulkanRenderer renderer;
    assert(!renderer.HasPendingCommands());

    renderer.BeginFrame();
    assert(!renderer.HasPendingCommands());

    renderer.DrawRect({0, 0, 10, 10}, {255, 0, 0, 255});
    assert(renderer.HasPendingCommands());

    renderer.EndFrame();
    std::cout << "[PASS] test_vulkan_has_pending_commands" << std::endl;
}

// ============================================================
// Task 2: TemplateAIBackend
// ============================================================

void test_template_backend_default_count() {
    atlas::ai::TemplateAIBackend backend;
    assert(backend.TemplateCount() == 12); // 3 per type × 4 types
    std::cout << "[PASS] test_template_backend_default_count" << std::endl;
}

void test_template_backend_query_graph() {
    atlas::ai::TemplateAIBackend backend;
    atlas::ai::AIContext ctx;
    ctx.projectName = "TestProject";

    auto resp = backend.Query("[GraphGeneration] create graph", ctx);
    assert(!resp.content.empty());
    assert(resp.confidence > 0.0f);
    assert(resp.confidence <= 0.4f);
    assert(resp.content.find("TestProject") != std::string::npos);

    std::cout << "[PASS] test_template_backend_query_graph" << std::endl;
}

void test_template_backend_query_world() {
    atlas::ai::TemplateAIBackend backend;
    atlas::ai::AIContext ctx;
    ctx.projectName = "MyGame";

    auto resp = backend.Query("[WorldGeneration] generate world", ctx);
    assert(!resp.content.empty());
    assert(resp.content.find("MyGame") != std::string::npos);

    std::cout << "[PASS] test_template_backend_query_world" << std::endl;
}

void test_template_backend_query_code() {
    atlas::ai::TemplateAIBackend backend;
    atlas::ai::AIContext ctx;
    ctx.selectedAsset = "PlayerShip";

    auto resp = backend.Query("[CodeAssist] explain this asset", ctx);
    assert(!resp.content.empty());
    assert(resp.content.find("PlayerShip") != std::string::npos);

    std::cout << "[PASS] test_template_backend_query_code" << std::endl;
}

void test_template_backend_query_analysis() {
    atlas::ai::TemplateAIBackend backend;
    atlas::ai::AIContext ctx;

    auto resp = backend.Query("[Analysis] validate schema", ctx);
    assert(!resp.content.empty());

    std::cout << "[PASS] test_template_backend_query_analysis" << std::endl;
}

void test_template_backend_empty_prompt() {
    atlas::ai::TemplateAIBackend backend;
    atlas::ai::AIContext ctx;

    auto resp = backend.Query("", ctx);
    assert(resp.content.empty());
    assert(resp.confidence == 0.0f);

    std::cout << "[PASS] test_template_backend_empty_prompt" << std::endl;
}

void test_template_backend_no_match() {
    atlas::ai::TemplateAIBackend backend;
    atlas::ai::AIContext ctx;

    auto resp = backend.Query("zzzzunknownzzzzquery", ctx);
    // Should still find some partial match or empty
    // Even if no words match, confidence should be 0
    assert(resp.confidence <= 0.1f);

    std::cout << "[PASS] test_template_backend_no_match" << std::endl;
}

void test_template_backend_add_custom() {
    atlas::ai::TemplateAIBackend backend;
    size_t initial = backend.TemplateCount();

    backend.AddTemplate("custom pattern", "Custom response for ${project}");
    assert(backend.TemplateCount() == initial + 1);

    atlas::ai::AIContext ctx;
    ctx.projectName = "AtlasDemo";
    auto resp = backend.Query("custom pattern test", ctx);
    assert(!resp.content.empty());
    assert(resp.content.find("AtlasDemo") != std::string::npos);

    std::cout << "[PASS] test_template_backend_add_custom" << std::endl;
}

void test_template_backend_clear() {
    atlas::ai::TemplateAIBackend backend;
    assert(backend.TemplateCount() > 0);

    backend.ClearTemplates();
    assert(backend.TemplateCount() == 0);

    atlas::ai::AIContext ctx;
    auto resp = backend.Query("create graph", ctx);
    assert(resp.content.empty());

    std::cout << "[PASS] test_template_backend_clear" << std::endl;
}

void test_template_backend_with_aggregator() {
    atlas::ai::AIAggregator aggregator;
    atlas::ai::TemplateAIBackend backend;

    aggregator.RegisterBackend(&backend);
    assert(aggregator.BackendCount() == 1);

    atlas::ai::AIContext ctx;
    ctx.projectName = "IntegrationTest";

    auto resp = aggregator.Execute(
        atlas::ai::AIRequestType::GraphGeneration,
        "create graph",
        ctx);
    assert(!resp.content.empty());
    assert(resp.content.find("IntegrationTest") != std::string::npos);

    std::cout << "[PASS] test_template_backend_with_aggregator" << std::endl;
}

void test_template_backend_variable_expansion() {
    atlas::ai::TemplateAIBackend backend;
    atlas::ai::AIContext ctx;
    ctx.projectName = "ProjectX";
    ctx.selectedAsset = "AssetY";
    ctx.networkMode = "server";

    // Query something that hits the "explain" template (contains ${asset} and ${project})
    auto resp = backend.Query("explain this asset", ctx);
    assert(!resp.content.empty());
    assert(resp.content.find("AssetY") != std::string::npos);
    assert(resp.content.find("ProjectX") != std::string::npos);

    std::cout << "[PASS] test_template_backend_variable_expansion" << std::endl;
}

// ============================================================
// Task 3: FontBootstrap TTF parsing
// ============================================================

void test_font_bootstrap_fallback_name() {
    atlas::ui::FontBootstrap fb;
    std::string testAssetRoot = "/tmp/atlas_test_font_phase6";
    std::filesystem::create_directories(testAssetRoot);

    bool ok = fb.Init(testAssetRoot);
    assert(ok);
    assert(fb.IsUsingFallback());
    assert(fb.GetFontName() == "builtin-fallback");

    fb.Shutdown();
    std::filesystem::remove_all(testAssetRoot);
    std::cout << "[PASS] test_font_bootstrap_fallback_name" << std::endl;
}

void test_font_bootstrap_ttf_loading() {
    // Create a minimal valid TTF-like file to test ParseTTFHeader
    std::string testAssetRoot = "/tmp/atlas_test_font_ttf_phase6";
    std::string fontsDir = testAssetRoot + "/fonts";
    std::filesystem::create_directories(fontsDir);

    std::string fontPath = fontsDir + "/Inter-Regular.ttf";

    // Create a minimal TTF with valid header, table directory, and name table
    std::ofstream out(fontPath, std::ios::binary);

    // TTF offset table: sfVersion=0x00010000, numTables=1
    uint8_t header[12] = {0x00, 0x01, 0x00, 0x00,  // sfVersion (TrueType)
                          0x00, 0x01,                // numTables = 1
                          0x00, 0x10,                // searchRange
                          0x00, 0x00,                // entrySelector
                          0x00, 0x10};               // rangeShift
    out.write(reinterpret_cast<char*>(header), 12);

    // Table directory entry: "name", checksum=0, offset=28, length=TBD
    uint8_t tableEntry[16] = {'n', 'a', 'm', 'e',  // tag
                              0x00, 0x00, 0x00, 0x00, // checksum
                              0x00, 0x00, 0x00, 0x1C, // offset = 28
                              0x00, 0x00, 0x00, 0x00}; // length (filled below)

    // Name table: format=0, count=1, stringOffset=18 (6 + 12*1)
    // Name record: platformID=1 (Mac), encodingID=0, languageID=0, nameID=1, length=5, offset=0
    // String: "Inter"
    std::string fontName = "Inter";
    uint16_t nameTableSize = 6 + 12 + static_cast<uint16_t>(fontName.size());

    // Fill length in table directory
    tableEntry[12] = 0;
    tableEntry[13] = 0;
    tableEntry[14] = static_cast<uint8_t>((nameTableSize >> 8) & 0xFF);
    tableEntry[15] = static_cast<uint8_t>(nameTableSize & 0xFF);

    out.write(reinterpret_cast<char*>(tableEntry), 16);

    // Name table header: format=0, count=1, stringOffset=18
    uint8_t nameHeader[6] = {0x00, 0x00,  // format
                             0x00, 0x01,   // count = 1
                             0x00, 0x12};  // stringOffset = 18 (6 + 12)
    out.write(reinterpret_cast<char*>(nameHeader), 6);

    // Name record: platformID=1, encodingID=0, languageID=0, nameID=1, length=5, offset=0
    uint8_t nameRecord[12] = {0x00, 0x01,  // platformID = 1 (Macintosh)
                              0x00, 0x00,  // encodingID = 0 (Roman)
                              0x00, 0x00,  // languageID = 0
                              0x00, 0x01,  // nameID = 1 (font family)
                              0x00, 0x05,  // length = 5
                              0x00, 0x00}; // offset = 0
    out.write(reinterpret_cast<char*>(nameRecord), 12);

    // String data: "Inter"
    out.write(fontName.c_str(), static_cast<std::streamsize>(fontName.size()));
    out.close();

    atlas::ui::FontBootstrap fb;
    bool ok = fb.Init(testAssetRoot);
    assert(ok);
    assert(!fb.IsUsingFallback());
    assert(fb.GetFontName() == "Inter");
    assert(fb.IsReady());

    fb.Shutdown();
    std::filesystem::remove_all(testAssetRoot);
    std::cout << "[PASS] test_font_bootstrap_ttf_loading" << std::endl;
}

void test_font_bootstrap_invalid_ttf() {
    std::string testAssetRoot = "/tmp/atlas_test_font_invalid_phase6";
    std::string fontsDir = testAssetRoot + "/fonts";
    std::filesystem::create_directories(fontsDir);

    std::string fontPath = fontsDir + "/Inter-Regular.ttf";

    // Write garbage data
    std::ofstream out(fontPath, std::ios::binary);
    out << "not a font file";
    out.close();

    atlas::ui::FontBootstrap fb;
    bool ok = fb.Init(testAssetRoot);
    assert(ok); // Should still succeed but with default name
    assert(!fb.IsUsingFallback());
    assert(fb.GetFontName() == "Inter-Regular"); // Fallback name when parse fails

    fb.Shutdown();
    std::filesystem::remove_all(testAssetRoot);
    std::cout << "[PASS] test_font_bootstrap_invalid_ttf" << std::endl;
}

// ============================================================
// Task 4: HTTP Client & Marketplace API
// ============================================================

void test_null_http_client_get() {
    atlas::asset::NullHttpClient client;
    auto resp = client.Get("https://example.com/test");
    assert(resp.IsError());
    assert(resp.statusCode == 0);
    assert(!resp.errorMessage.empty());

    std::cout << "[PASS] test_null_http_client_get" << std::endl;
}

void test_null_http_client_download() {
    atlas::asset::NullHttpClient client;
    auto resp = client.DownloadFile("https://example.com/file.png", "/tmp/test.png");
    assert(resp.IsError());
    assert(!resp.errorMessage.empty());

    std::cout << "[PASS] test_null_http_client_download" << std::endl;
}

void test_http_response_status_codes() {
    atlas::asset::HttpResponse resp;

    resp.statusCode = 200;
    assert(resp.IsSuccess());
    assert(!resp.IsError());

    resp.statusCode = 404;
    assert(!resp.IsSuccess());
    assert(resp.IsError());

    resp.statusCode = 0;
    assert(!resp.IsSuccess());
    assert(resp.IsError());

    resp.statusCode = 201;
    assert(resp.IsSuccess());

    std::cout << "[PASS] test_http_response_status_codes" << std::endl;
}

// Mock HTTP client for testing marketplace download path
class MockHttpClient : public atlas::asset::IHttpClient {
public:
    atlas::asset::HttpResponse Get(
        const std::string& url,
        const std::vector<std::pair<std::string, std::string>>& headers) override {
        atlas::asset::HttpResponse resp;
        resp.statusCode = 200;
        resp.body = R"({"id": "test", "name": "Test Asset"})";
        lastGetUrl = url;
        return resp;
    }

    atlas::asset::HttpResponse Post(
        const std::string& url,
        const std::string& body,
        const std::vector<std::pair<std::string, std::string>>& headers) override {
        atlas::asset::HttpResponse resp;
        resp.statusCode = 200;
        resp.body = R"({"id": "test", "name": "Test Asset"})";
        lastPostUrl = url;
        return resp;
    }

    atlas::asset::HttpResponse DownloadFile(
        const std::string& url,
        const std::string& outputPath,
        const std::vector<std::pair<std::string, std::string>>& headers) override {
        // Create a dummy file at the output path
        std::ofstream out(outputPath);
        out << "mock asset data";
        out.close();

        atlas::asset::HttpResponse resp;
        resp.statusCode = 200;
        lastDownloadUrl = url;
        lastDownloadPath = outputPath;
        return resp;
    }

    std::string lastGetUrl;
    std::string lastPostUrl;
    std::string lastDownloadUrl;
    std::string lastDownloadPath;
};

void test_marketplace_set_http_client() {
    atlas::asset::ItchIOImporter importer;
    MockHttpClient mock;
    importer.SetHttpClient(&mock);

    // Fetch with mock client should attempt download
    atlas::asset::MarketplaceImportOptions opts;
    opts.downloadCache = "/tmp/atlas_test_marketplace_phase6";
    std::filesystem::create_directories(opts.downloadCache);

    auto result = importer.FetchAsset("test-asset.png", opts);
    // Mock client returns success and creates a file
    assert(result.success);
    assert(!mock.lastGetUrl.empty());
    assert(mock.lastGetUrl.find("itch.io") != std::string::npos);

    std::filesystem::remove_all(opts.downloadCache);
    std::cout << "[PASS] test_marketplace_set_http_client" << std::endl;
}

void test_marketplace_registry_set_http_client() {
    atlas::asset::MarketplaceImportRegistry registry;
    registry.RegisterImporter(std::make_unique<atlas::asset::ItchIOImporter>());
    registry.RegisterImporter(std::make_unique<atlas::asset::UnrealMarketplaceImporter>());
    registry.RegisterImporter(std::make_unique<atlas::asset::UnityAssetStoreImporter>());

    MockHttpClient mock;
    registry.SetHttpClient(&mock);

    // All importers should have the client set
    assert(registry.ImporterCount() == 3);

    std::cout << "[PASS] test_marketplace_registry_set_http_client" << std::endl;
}

void test_marketplace_no_http_client_error() {
    atlas::asset::ItchIOImporter importer;
    // No HTTP client set, no cache file
    atlas::asset::MarketplaceImportOptions opts;
    opts.downloadCache = "/tmp/atlas_test_no_http_phase6";
    std::filesystem::create_directories(opts.downloadCache);

    auto result = importer.FetchAsset("nonexistent.png", opts);
    assert(!result.success);
    assert(result.errorMessage.find("no HTTP client") != std::string::npos);

    std::filesystem::remove_all(opts.downloadCache);
    std::cout << "[PASS] test_marketplace_no_http_client_error" << std::endl;
}

void test_marketplace_path_traversal_rejected() {
    atlas::asset::ItchIOImporter importer;
    atlas::asset::MarketplaceImportOptions opts;
    opts.downloadCache = "/tmp/atlas_test_traversal_phase6";

    // Path traversal attempts must be rejected
    auto r1 = importer.FetchAsset("../../etc/passwd", opts);
    assert(!r1.success);
    assert(r1.errorMessage.find("path traversal") != std::string::npos);

    auto r2 = importer.FetchAsset("assets/../secret", opts);
    assert(!r2.success);

    auto r3 = importer.FetchAsset("sub/dir/file.png", opts);
    assert(!r3.success);

    auto r4 = importer.FetchAsset(".hidden", opts);
    assert(!r4.success);

    std::cout << "[PASS] test_marketplace_path_traversal_rejected" << std::endl;
}

// ============================================================
// Runner function — called from main.cpp
// ============================================================

void run_next_tasks_phase6_tests() {
    std::cout << "\n--- Phase 6: VulkanRenderer GPU Submission Tests ---" << std::endl;
    test_vulkan_submit_command_buffer();
    test_vulkan_auto_submit_on_end_frame();
    test_vulkan_no_submit_empty_frame();
    test_vulkan_triple_buffering();
    test_vulkan_submit_preserves_commands();
    test_vulkan_has_pending_commands();

    std::cout << "\n--- Phase 6: TemplateAIBackend Tests ---" << std::endl;
    test_template_backend_default_count();
    test_template_backend_query_graph();
    test_template_backend_query_world();
    test_template_backend_query_code();
    test_template_backend_query_analysis();
    test_template_backend_empty_prompt();
    test_template_backend_no_match();
    test_template_backend_add_custom();
    test_template_backend_clear();
    test_template_backend_with_aggregator();
    test_template_backend_variable_expansion();

    std::cout << "\n--- Phase 6: FontBootstrap TTF Loading Tests ---" << std::endl;
    test_font_bootstrap_fallback_name();
    test_font_bootstrap_ttf_loading();
    test_font_bootstrap_invalid_ttf();

    std::cout << "\n--- Phase 6: HTTP Client & Marketplace API Tests ---" << std::endl;
    test_null_http_client_get();
    test_null_http_client_download();
    test_http_response_status_codes();
    test_marketplace_set_http_client();
    test_marketplace_registry_set_http_client();
    test_marketplace_no_http_client_error();
    test_marketplace_path_traversal_rejected();
}
