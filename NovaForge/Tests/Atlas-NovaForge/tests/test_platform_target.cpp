#include "../engine/production/PlatformTarget.h"
#include <iostream>
#include <cassert>

using namespace atlas::production;

void test_platform_default_windows() {
    PlatformConfig config = PlatformTarget::DefaultFor(Platform::Windows);

    assert(config.platform == Platform::Windows);
    assert(config.architecture == "x86_64");
    assert(config.outputExtension == ".exe");
    assert(config.toolchain == "msvc");
    assert(!config.defines.empty());

    std::cout << "[PASS] test_platform_default_windows" << std::endl;
}

void test_platform_default_linux() {
    PlatformConfig config = PlatformTarget::DefaultFor(Platform::Linux);

    assert(config.platform == Platform::Linux);
    assert(config.architecture == "x86_64");
    assert(config.outputExtension == "");
    assert(config.toolchain == "gcc");

    std::cout << "[PASS] test_platform_default_linux" << std::endl;
}

void test_platform_default_macos() {
    PlatformConfig config = PlatformTarget::DefaultFor(Platform::macOS);

    assert(config.platform == Platform::macOS);
    assert(config.architecture == "arm64");
    assert(config.outputExtension == ".app");
    assert(config.toolchain == "clang");

    std::cout << "[PASS] test_platform_default_macos" << std::endl;
}

void test_platform_default_web() {
    PlatformConfig config = PlatformTarget::DefaultFor(Platform::Web);

    assert(config.platform == Platform::Web);
    assert(config.architecture == "wasm32");
    assert(config.outputExtension == ".html");
    assert(config.toolchain == "emscripten");
    assert(!config.excludedModules.empty());

    std::cout << "[PASS] test_platform_default_web" << std::endl;
}

void test_platform_name() {
    assert(std::string(PlatformTarget::PlatformName(Platform::Windows)) == "Windows");
    assert(std::string(PlatformTarget::PlatformName(Platform::Linux)) == "Linux");
    assert(std::string(PlatformTarget::PlatformName(Platform::macOS)) == "macOS");
    assert(std::string(PlatformTarget::PlatformName(Platform::Web)) == "Web");

    std::cout << "[PASS] test_platform_name" << std::endl;
}

void test_platform_set_target() {
    PlatformTarget target;

    target.SetTarget(Platform::Windows);
    assert(target.ActivePlatform() == Platform::Windows);
    assert(target.ActiveConfig().toolchain == "msvc");

    target.SetTarget(Platform::Linux);
    assert(target.ActivePlatform() == Platform::Linux);
    assert(target.ActiveConfig().toolchain == "gcc");

    std::cout << "[PASS] test_platform_set_target" << std::endl;
}

void test_platform_custom_config() {
    PlatformTarget target;

    PlatformConfig custom;
    custom.platform = Platform::Linux;
    custom.architecture = "aarch64";
    custom.toolchain = "clang";

    target.SetTargetConfig(custom);
    assert(target.ActivePlatform() == Platform::Linux);
    assert(target.ActiveConfig().architecture == "aarch64");
    assert(target.ActiveConfig().toolchain == "clang");

    std::cout << "[PASS] test_platform_custom_config" << std::endl;
}

void test_platform_module_exclusion() {
    PlatformTarget target;

    target.SetTarget(Platform::Web);
    assert(target.IsModuleExcluded("voice") == true);
    assert(target.IsModuleExcluded("server") == true);
    assert(target.IsModuleExcluded("ecs") == false);

    target.SetTarget(Platform::Linux);
    assert(target.IsModuleExcluded("voice") == false);

    std::cout << "[PASS] test_platform_module_exclusion" << std::endl;
}

void test_platform_host() {
    Platform host = PlatformTarget::HostPlatform();
    // On this CI/test machine, should be Linux
    assert(host == Platform::Linux);

    std::cout << "[PASS] test_platform_host" << std::endl;
}

void test_platform_supported_list() {
    auto platforms = PlatformTarget::SupportedPlatforms();
    assert(platforms.size() == 4);

    std::cout << "[PASS] test_platform_supported_list" << std::endl;
}
