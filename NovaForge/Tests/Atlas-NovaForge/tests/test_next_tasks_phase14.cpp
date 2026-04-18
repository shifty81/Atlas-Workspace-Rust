#include "../engine/render/VulkanRenderer.h"
#include "../engine/ai/LLMBackend.h"
#include "../engine/ui/FontBootstrap.h"
#include "../engine/assets/HttpClient.h"
#include <iostream>
#include <cassert>
#include <cstring>
#include <filesystem>
#include <fstream>

using namespace atlas::render;
using namespace atlas::ai;
using namespace atlas::ui;

// ---------------------------------------------------------------
// Vulkan Device Abstraction tests
// ---------------------------------------------------------------

void test_vk_device_not_initialized_by_default() {
    VulkanRenderer r;
    assert(!r.IsDeviceInitialized());
    assert(!r.HasSwapChain());
    assert(r.GetQueueFamilies().empty());
    assert(r.EnumerateDevices().empty());
    std::cout << "[PASS] test_vk_device_not_initialized_by_default" << std::endl;
}

void test_vk_init_device_basic() {
    VulkanRenderer r;
    VkDeviceConfig config;
    config.applicationName = "TestApp";
    config.applicationVersion = 42;
    config.enableValidationLayers = true;

    bool ok = r.InitDevice(config);
    assert(ok);
    assert(r.IsDeviceInitialized());
    assert(r.GetDeviceConfig().applicationName == "TestApp");
    assert(r.GetDeviceConfig().applicationVersion == 42);
    assert(r.GetDeviceConfig().enableValidationLayers);
    std::cout << "[PASS] test_vk_init_device_basic" << std::endl;
}

void test_vk_physical_device_info() {
    VulkanRenderer r;
    VkDeviceConfig config;
    r.InitDevice(config);

    const auto& info = r.GetPhysicalDeviceInfo();
    assert(!info.deviceName.empty());
    assert(info.vendorId != 0);
    assert(info.deviceType == VkPhysicalDeviceInfo::DeviceType::DiscreteGPU);
    assert(info.totalMemoryBytes > 0);
    assert(info.supportsGeometryShader);
    assert(info.supportsTessellation);
    assert(info.supportsCompute);
    std::cout << "[PASS] test_vk_physical_device_info" << std::endl;
}

void test_vk_queue_families() {
    VulkanRenderer r;
    VkDeviceConfig config;
    r.InitDevice(config);

    const auto& families = r.GetQueueFamilies();
    assert(families.size() >= 2);

    // Graphics queue family
    uint32_t gfxIdx = r.GetGraphicsQueueFamily();
    assert(gfxIdx != UINT32_MAX);
    assert(families[gfxIdx].supportsGraphics);

    // Dedicated transfer queue
    uint32_t xferIdx = r.GetTransferQueueFamily();
    assert(xferIdx != UINT32_MAX);

    // Compute queue
    uint32_t computeIdx = r.GetComputeQueueFamily();
    assert(computeIdx != UINT32_MAX);

    std::cout << "[PASS] test_vk_queue_families" << std::endl;
}

void test_vk_swap_chain_create() {
    VulkanRenderer r;
    VkDeviceConfig config;
    r.InitDevice(config);

    VkSwapChainDesc sc;
    sc.width = 1920;
    sc.height = 1080;
    sc.imageCount = 3;
    sc.presentMode = VkSwapChainDesc::PresentMode::Mailbox;

    bool ok = r.CreateSwapChain(sc);
    assert(ok);
    assert(r.HasSwapChain());
    assert(r.GetSwapChain().width == 1920);
    assert(r.GetSwapChain().height == 1080);
    assert(r.GetSwapChain().imageCount == 3);
    assert(r.GetSwapChain().valid);
    std::cout << "[PASS] test_vk_swap_chain_create" << std::endl;
}

void test_vk_swap_chain_no_device() {
    VulkanRenderer r;
    VkSwapChainDesc sc;
    sc.width = 1920;
    sc.height = 1080;
    bool ok = r.CreateSwapChain(sc);
    assert(!ok);
    assert(!r.HasSwapChain());
    std::cout << "[PASS] test_vk_swap_chain_no_device" << std::endl;
}

void test_vk_swap_chain_zero_size() {
    VulkanRenderer r;
    VkDeviceConfig config;
    r.InitDevice(config);

    VkSwapChainDesc sc;
    sc.width = 0;
    sc.height = 0;
    bool ok = r.CreateSwapChain(sc);
    assert(!ok);
    assert(!r.HasSwapChain());
    std::cout << "[PASS] test_vk_swap_chain_zero_size" << std::endl;
}

void test_vk_swap_chain_resize() {
    VulkanRenderer r;
    VkDeviceConfig config;
    r.InitDevice(config);

    VkSwapChainDesc sc;
    sc.width = 1280;
    sc.height = 720;
    r.CreateSwapChain(sc);

    bool ok = r.ResizeSwapChain(1920, 1080);
    assert(ok);
    assert(r.GetSwapChain().width == 1920);
    assert(r.GetSwapChain().height == 1080);
    std::cout << "[PASS] test_vk_swap_chain_resize" << std::endl;
}

void test_vk_swap_chain_resize_no_chain() {
    VulkanRenderer r;
    VkDeviceConfig config;
    r.InitDevice(config);

    bool ok = r.ResizeSwapChain(1920, 1080);
    assert(!ok);
    std::cout << "[PASS] test_vk_swap_chain_resize_no_chain" << std::endl;
}

void test_vk_enumerate_devices() {
    VulkanRenderer r;
    VkDeviceConfig config;
    r.InitDevice(config);

    const auto& devices = r.EnumerateDevices();
    assert(!devices.empty());
    assert(!devices[0].deviceName.empty());
    std::cout << "[PASS] test_vk_enumerate_devices" << std::endl;
}

void test_vk_select_device() {
    VulkanRenderer r;
    VkDeviceConfig config;
    r.InitDevice(config);

    // Can't change device while initialized
    bool blocked = r.SelectDevice(0);
    assert(!blocked);

    // After shutdown, can select device
    r.ShutdownDevice();

    // First re-init to populate available devices
    r.InitDevice(config);
    r.ShutdownDevice();

    // Now no devices in list after shutdown
    bool bad = r.SelectDevice(999);
    assert(!bad);
    std::cout << "[PASS] test_vk_select_device" << std::endl;
}

void test_vk_shutdown_device() {
    VulkanRenderer r;
    VkDeviceConfig config;
    r.InitDevice(config);

    VkSwapChainDesc sc;
    sc.width = 1280;
    sc.height = 720;
    r.CreateSwapChain(sc);

    r.ShutdownDevice();
    assert(!r.IsDeviceInitialized());
    assert(!r.HasSwapChain());
    assert(r.GetQueueFamilies().empty());
    assert(r.EnumerateDevices().empty());
    std::cout << "[PASS] test_vk_shutdown_device" << std::endl;
}

void test_vk_double_init() {
    VulkanRenderer r;
    VkDeviceConfig config;
    config.applicationName = "FirstInit";
    r.InitDevice(config);
    assert(r.IsDeviceInitialized());

    // Second init should succeed (idempotent)
    config.applicationName = "SecondInit";
    bool ok = r.InitDevice(config);
    assert(ok);
    // Config should remain from first init
    assert(r.GetDeviceConfig().applicationName == "FirstInit");
    std::cout << "[PASS] test_vk_double_init" << std::endl;
}

void test_vk_header_version_stub() {
    // VK_HEADER_VERSION_STUB should encode Vulkan 1.3.0
    assert(VK_HEADER_VERSION_STUB == ((1u << 22) | (3u << 12) | 0u));
    std::cout << "[PASS] test_vk_header_version_stub" << std::endl;
}

// ---------------------------------------------------------------
// HttpLLMBackend tests
// ---------------------------------------------------------------

/// Mock HTTP client for testing
class LLMMockHttpClient : public atlas::asset::IHttpClient {
public:
    std::string lastUrl;
    std::string lastPostBody;
    std::vector<std::pair<std::string, std::string>> lastHeaders;
    int32_t mockStatusCode = 200;
    std::string mockBody;
    std::string mockError;

    atlas::asset::HttpResponse Get(
        const std::string& url,
        const std::vector<std::pair<std::string, std::string>>& headers = {}) override {
        lastUrl = url;
        lastHeaders = headers;
        atlas::asset::HttpResponse resp;
        resp.statusCode = mockStatusCode;
        resp.body = mockBody;
        resp.errorMessage = mockError;
        return resp;
    }

    atlas::asset::HttpResponse Post(
        const std::string& url,
        const std::string& body,
        const std::vector<std::pair<std::string, std::string>>& headers = {}) override {
        lastUrl = url;
        lastPostBody = body;
        lastHeaders = headers;
        atlas::asset::HttpResponse resp;
        resp.statusCode = mockStatusCode;
        resp.body = mockBody;
        resp.errorMessage = mockError;
        return resp;
    }

    atlas::asset::HttpResponse DownloadFile(
        const std::string& url,
        const std::string& outputPath,
        const std::vector<std::pair<std::string, std::string>>& headers = {}) override {
        atlas::asset::HttpResponse resp;
        resp.statusCode = mockStatusCode;
        resp.body = mockBody;
        resp.errorMessage = mockError;
        return resp;
    }
};

void test_http_llm_construction() {
    LLMMockHttpClient client;
    HttpLLMBackend backend(&client, "https://api.example.com/v1/chat", "test-model");

    assert(backend.GetEndpoint() == "https://api.example.com/v1/chat");
    assert(backend.GetModel() == "test-model");
    assert(!backend.HasApiKey());
    assert(!backend.IsAvailable());
    assert(backend.Name() == "HttpLLM(test-model)");
    std::cout << "[PASS] test_http_llm_construction" << std::endl;
}

void test_http_llm_api_key() {
    LLMMockHttpClient client;
    HttpLLMBackend backend(&client, "https://api.example.com", "gpt-4");

    assert(!backend.HasApiKey());
    backend.SetApiKey("sk-test-key-123");
    assert(backend.HasApiKey());
    assert(backend.IsAvailable());
    std::cout << "[PASS] test_http_llm_api_key" << std::endl;
}

void test_http_llm_timeout() {
    LLMMockHttpClient client;
    HttpLLMBackend backend(&client, "https://api.example.com", "gpt-4");

    assert(backend.GetTimeoutMs() == 30000); // default
    backend.SetTimeoutMs(60000);
    assert(backend.GetTimeoutMs() == 60000);
    std::cout << "[PASS] test_http_llm_timeout" << std::endl;
}

void test_http_llm_capabilities() {
    LLMMockHttpClient client;
    HttpLLMBackend backend(&client, "https://api.example.com", "gpt-4");

    uint8_t caps = backend.Capabilities();
    assert(caps & static_cast<uint8_t>(LLMCapability::TextGeneration));
    assert(caps & static_cast<uint8_t>(LLMCapability::Streaming));
    std::cout << "[PASS] test_http_llm_capabilities" << std::endl;
}

void test_http_llm_no_api_key_error() {
    LLMMockHttpClient client;
    HttpLLMBackend backend(&client, "https://api.example.com", "gpt-4");
    // No API key set

    LLMRequest req;
    req.prompt = "Hello";
    LLMResponse resp = backend.Complete(req);

    assert(!resp.success);
    assert(!resp.errorMessage.empty());
    assert(backend.FailureCount() == 1);
    assert(backend.SuccessCount() == 0);
    std::cout << "[PASS] test_http_llm_no_api_key_error" << std::endl;
}

void test_http_llm_no_client_error() {
    HttpLLMBackend backend(nullptr, "https://api.example.com", "gpt-4");
    backend.SetApiKey("sk-test");

    LLMRequest req;
    req.prompt = "Hello";
    LLMResponse resp = backend.Complete(req);

    assert(!resp.success);
    assert(!resp.errorMessage.empty());
    assert(backend.FailureCount() == 1);
    std::cout << "[PASS] test_http_llm_no_client_error" << std::endl;
}

void test_http_llm_successful_completion() {
    LLMMockHttpClient client;
    client.mockStatusCode = 200;
    client.mockBody = R"({"choices":[{"message":{"role":"assistant","content":"Hello, world!"}}]})";

    HttpLLMBackend backend(&client, "https://api.example.com/v1/chat", "gpt-4");
    backend.SetApiKey("sk-test");

    LLMRequest req;
    req.prompt = "Say hello";
    LLMResponse resp = backend.Complete(req);

    assert(resp.success);
    assert(resp.text == "Hello, world!");
    assert(resp.tokensUsed > 0);
    assert(backend.SuccessCount() == 1);
    assert(backend.FailureCount() == 0);

    // Verify auth header was sent
    bool hasAuth = false;
    for (const auto& [key, value] : client.lastHeaders) {
        if (key == "Authorization" && value == "Bearer sk-test") {
            hasAuth = true;
        }
    }
    assert(hasAuth);
    std::cout << "[PASS] test_http_llm_successful_completion" << std::endl;
}

void test_http_llm_http_error() {
    LLMMockHttpClient client;
    client.mockStatusCode = 500;
    client.mockError = "Internal Server Error";

    HttpLLMBackend backend(&client, "https://api.example.com", "gpt-4");
    backend.SetApiKey("sk-test");

    LLMRequest req;
    req.prompt = "Test error";
    LLMResponse resp = backend.Complete(req);

    assert(!resp.success);
    assert(resp.errorMessage.find("500") != std::string::npos);
    assert(backend.FailureCount() == 1);
    std::cout << "[PASS] test_http_llm_http_error" << std::endl;
}

void test_http_llm_parse_error() {
    LLMMockHttpClient client;
    client.mockStatusCode = 200;
    client.mockBody = "not valid json at all";

    HttpLLMBackend backend(&client, "https://api.example.com", "gpt-4");
    backend.SetApiKey("sk-test");

    LLMRequest req;
    req.prompt = "Test parse error";
    LLMResponse resp = backend.Complete(req);

    assert(!resp.success);
    assert(!resp.errorMessage.empty());
    assert(backend.FailureCount() == 1);
    std::cout << "[PASS] test_http_llm_parse_error" << std::endl;
}

void test_http_llm_escaped_content() {
    LLMMockHttpClient client;
    client.mockStatusCode = 200;
    client.mockBody = R"({"choices":[{"message":{"content":"Line 1\nLine 2\ttab"}}]})";

    HttpLLMBackend backend(&client, "https://api.example.com", "gpt-4");
    backend.SetApiKey("sk-test");

    LLMRequest req;
    req.prompt = "Test escapes";
    LLMResponse resp = backend.Complete(req);

    assert(resp.success);
    assert(resp.text.find('\n') != std::string::npos);
    assert(resp.text.find('\t') != std::string::npos);
    std::cout << "[PASS] test_http_llm_escaped_content" << std::endl;
}

void test_http_llm_registry_integration() {
    LLMMockHttpClient client;
    client.mockStatusCode = 200;
    client.mockBody = R"({"choices":[{"message":{"content":"registry test"}}]})";

    auto backend = std::make_shared<HttpLLMBackend>(&client, "https://api.example.com", "gpt-4");
    backend->SetApiKey("sk-test");

    LLMBackendRegistry registry;
    assert(!registry.HasRealBackend());

    registry.SetBackend(backend);
    assert(registry.HasRealBackend());

    LLMResponse resp = registry.Complete("Test prompt");
    assert(resp.success);
    assert(resp.text == "registry test");
    assert(registry.RequestCount() == 1);
    std::cout << "[PASS] test_http_llm_registry_integration" << std::endl;
}

// ---------------------------------------------------------------
// FontBootstrap enhancement tests
// ---------------------------------------------------------------

void test_font_search_paths_empty_default() {
    FontBootstrap fb;
    assert(fb.GetFontSearchPaths().empty());
    assert(fb.GetLoadedFontPath().empty());
    std::cout << "[PASS] test_font_search_paths_empty_default" << std::endl;
}

void test_font_add_search_path() {
    FontBootstrap fb;
    fb.AddFontSearchPath("/usr/share/fonts");
    fb.AddFontSearchPath("/home/user/.fonts");
    assert(fb.GetFontSearchPaths().size() == 2);
    assert(fb.GetFontSearchPaths()[0] == "/usr/share/fonts");
    assert(fb.GetFontSearchPaths()[1] == "/home/user/.fonts");
    std::cout << "[PASS] test_font_add_search_path" << std::endl;
}

void test_font_add_search_path_dedup() {
    FontBootstrap fb;
    fb.AddFontSearchPath("/usr/share/fonts");
    fb.AddFontSearchPath("/usr/share/fonts");
    assert(fb.GetFontSearchPaths().size() == 1);
    std::cout << "[PASS] test_font_add_search_path_dedup" << std::endl;
}

void test_font_add_search_path_empty() {
    FontBootstrap fb;
    fb.AddFontSearchPath("");
    assert(fb.GetFontSearchPaths().empty());
    std::cout << "[PASS] test_font_add_search_path_empty" << std::endl;
}

void test_font_discover_no_paths() {
    FontBootstrap fb;
    auto fonts = fb.DiscoverFonts();
    assert(fonts.empty());
    std::cout << "[PASS] test_font_discover_no_paths" << std::endl;
}

void test_font_discover_nonexistent_path() {
    FontBootstrap fb;
    fb.AddFontSearchPath("/nonexistent/path/fonts/12345");
    auto fonts = fb.DiscoverFonts();
    assert(fonts.empty());
    std::cout << "[PASS] test_font_discover_nonexistent_path" << std::endl;
}

void test_font_discover_with_files() {
    // Create a temp directory with font-like files
    std::string tmpDir = (std::filesystem::temp_directory_path() / ("atlas_font_test_" + std::to_string(getpid()))).string();
    std::error_code ec;
    std::filesystem::create_directories(tmpDir, ec);
    assert(!ec);

    // Create test files
    std::ofstream(tmpDir + "/test.ttf").put('X');
    std::ofstream(tmpDir + "/test.otf").put('X');
    std::ofstream(tmpDir + "/test.txt").put('X');
    std::ofstream(tmpDir + "/test.png").put('X');

    FontBootstrap fb;
    fb.AddFontSearchPath(tmpDir);
    auto fonts = fb.DiscoverFonts();

    // Should find .ttf and .otf but not .txt or .png
    assert(fonts.size() == 2);

    // Clean up
    std::filesystem::remove_all(tmpDir, ec);
    std::cout << "[PASS] test_font_discover_with_files" << std::endl;
}

void test_font_load_nonexistent() {
    FontBootstrap fb;
    bool ok = fb.LoadFont("/nonexistent/font.ttf");
    assert(!ok);
    std::cout << "[PASS] test_font_load_nonexistent" << std::endl;
}

void test_font_load_empty_path() {
    FontBootstrap fb;
    bool ok = fb.LoadFont("");
    assert(!ok);
    std::cout << "[PASS] test_font_load_empty_path" << std::endl;
}

void test_font_init_adds_search_path() {
    // Init should add the assets/fonts path to search paths
    std::string tmpDir = (std::filesystem::temp_directory_path() / ("atlas_font_init_" + std::to_string(getpid()))).string();
    std::error_code ec;
    std::filesystem::create_directories(tmpDir + "/fonts", ec);

    FontBootstrap fb;
    fb.Init(tmpDir);

    const auto& paths = fb.GetFontSearchPaths();
    assert(!paths.empty());
    assert(paths[0] == tmpDir + "/fonts");

    // Clean up
    std::filesystem::remove_all(tmpDir, ec);
    std::cout << "[PASS] test_font_init_adds_search_path" << std::endl;
}

// ---------------------------------------------------------------
// Post() method tests
// ---------------------------------------------------------------

void test_http_llm_uses_post() {
    LLMMockHttpClient client;
    client.mockStatusCode = 200;
    client.mockBody = R"({"choices":[{"message":{"content":"post reply"}}]})";

    HttpLLMBackend backend(&client, "https://api.example.com/v1/chat", "gpt-4");
    backend.SetApiKey("sk-test");

    LLMRequest req;
    req.prompt = "Hello via POST";
    LLMResponse resp = backend.Complete(req);

    assert(resp.success);
    assert(resp.text == "post reply");
    // Verify POST body was sent (not via URL query string)
    assert(!client.lastPostBody.empty());
    assert(client.lastPostBody.find("\"model\":\"gpt-4\"") != std::string::npos);
    assert(client.lastPostBody.find("Hello via POST") != std::string::npos);
    // URL should be the endpoint itself, not appended with query
    assert(client.lastUrl == "https://api.example.com/v1/chat");
    std::cout << "[PASS] test_http_llm_uses_post" << std::endl;
}

void test_null_http_client_post() {
    atlas::asset::NullHttpClient nullClient;
    auto resp = nullClient.Post("http://example.com", "{}", {});
    assert(resp.IsError());
    assert(!resp.errorMessage.empty());
    std::cout << "[PASS] test_null_http_client_post" << std::endl;
}

// ---------------------------------------------------------------
// LLMBackendFactory tests
// ---------------------------------------------------------------

void test_llm_factory_create_explicit() {
    LLMMockHttpClient client;
    auto backend = LLMBackendFactory::Create(&client, "https://api.example.com", "gpt-4", "sk-key", 5000);
    assert(backend != nullptr);
    assert(backend->GetEndpoint() == "https://api.example.com");
    assert(backend->GetModel() == "gpt-4");
    assert(backend->HasApiKey());
    assert(backend->GetTimeoutMs() == 5000);
    assert(backend->IsAvailable());
    std::cout << "[PASS] test_llm_factory_create_explicit" << std::endl;
}

void test_llm_factory_no_env() {
    // When env vars are not set, CreateFromEnv returns nullptr
    // (we cannot guarantee env state, but at least HasEnvConfig works)
    bool hasConfig = LLMBackendFactory::HasEnvConfig();
    LLMMockHttpClient client;
    auto backend = LLMBackendFactory::CreateFromEnv(&client);
    if (!hasConfig) {
        assert(backend == nullptr);
    }
    std::cout << "[PASS] test_llm_factory_no_env" << std::endl;
}

void test_llm_factory_create_and_complete() {
    LLMMockHttpClient client;
    client.mockStatusCode = 200;
    client.mockBody = R"({"choices":[{"message":{"content":"factory test"}}]})";

    auto backend = LLMBackendFactory::Create(&client, "https://api.example.com", "gpt-4", "sk-test-key");
    LLMRequest req;
    req.prompt = "factory prompt";
    LLMResponse resp = backend->Complete(req);
    assert(resp.success);
    assert(resp.text == "factory test");
    std::cout << "[PASS] test_llm_factory_create_and_complete" << std::endl;
}

// ---------------------------------------------------------------
// Font: Inter-Regular.ttf loading test
// ---------------------------------------------------------------

void test_font_init_with_inter_regular() {
    // Use the real assets directory with Inter-Regular.ttf
    std::string assetDir = std::string(CMAKE_SOURCE_DIR) + "/assets";
    FontBootstrap fb;
    bool ok = fb.Init(assetDir);

    // If the font file exists, it should load successfully
    std::string fontPath = assetDir + "/fonts/Inter-Regular.ttf";
    if (std::filesystem::exists(fontPath)) {
        assert(ok);
        assert(fb.IsReady());
        assert(fb.GetFontName() == "Inter");
        assert(!fb.IsUsingFallback());
    }
    std::cout << "[PASS] test_font_init_with_inter_regular" << std::endl;
}

// ---------------------------------------------------------------
// Registration
// ---------------------------------------------------------------

void register_next_tasks_phase14() {
    std::cout << "\n--- Phase 14: Vulkan Device Abstraction ---" << std::endl;
    test_vk_device_not_initialized_by_default();
    test_vk_init_device_basic();
    test_vk_physical_device_info();
    test_vk_queue_families();
    test_vk_swap_chain_create();
    test_vk_swap_chain_no_device();
    test_vk_swap_chain_zero_size();
    test_vk_swap_chain_resize();
    test_vk_swap_chain_resize_no_chain();
    test_vk_enumerate_devices();
    test_vk_select_device();
    test_vk_shutdown_device();
    test_vk_double_init();
    test_vk_header_version_stub();

    std::cout << "\n--- Phase 14: HttpLLMBackend ---" << std::endl;
    test_http_llm_construction();
    test_http_llm_api_key();
    test_http_llm_timeout();
    test_http_llm_capabilities();
    test_http_llm_no_api_key_error();
    test_http_llm_no_client_error();
    test_http_llm_successful_completion();
    test_http_llm_http_error();
    test_http_llm_parse_error();
    test_http_llm_escaped_content();
    test_http_llm_registry_integration();

    std::cout << "\n--- Phase 14: FontBootstrap Enhancements ---" << std::endl;
    test_font_search_paths_empty_default();
    test_font_add_search_path();
    test_font_add_search_path_dedup();
    test_font_add_search_path_empty();
    test_font_discover_no_paths();
    test_font_discover_nonexistent_path();
    test_font_discover_with_files();
    test_font_load_nonexistent();
    test_font_load_empty_path();
    test_font_init_adds_search_path();

    std::cout << "\n--- Phase 14: HTTP Post & LLM Factory ---" << std::endl;
    test_http_llm_uses_post();
    test_null_http_client_post();
    test_llm_factory_create_explicit();
    test_llm_factory_no_env();
    test_llm_factory_create_and_complete();

    std::cout << "\n--- Phase 14: Font Bundling ---" << std::endl;
    test_font_init_with_inter_regular();
}
