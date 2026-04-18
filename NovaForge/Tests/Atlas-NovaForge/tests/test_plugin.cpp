#include "../engine/plugin/PluginSystem.h"
#include <iostream>
#include <cassert>

void test_plugin_validate_valid() {
    atlas::plugin::PluginDescriptor desc{"TestPlugin", "1.0.0", "0.1.0", "graph-extension", true};
    auto result = atlas::plugin::PluginValidator::Validate(desc);
    assert(result == atlas::plugin::PluginLoadResult::Success);
    std::cout << "[PASS] test_plugin_validate_valid" << std::endl;
}

void test_plugin_validate_no_name() {
    atlas::plugin::PluginDescriptor desc{"", "1.0.0", "0.1.0", "graph-extension", true};
    auto result = atlas::plugin::PluginValidator::Validate(desc);
    assert(result == atlas::plugin::PluginLoadResult::InvalidDescriptor);
    std::cout << "[PASS] test_plugin_validate_no_name" << std::endl;
}

void test_plugin_validate_nondeterministic() {
    atlas::plugin::PluginDescriptor desc{"Bad", "1.0.0", "0.1.0", "graph-extension", false};
    auto result = atlas::plugin::PluginValidator::Validate(desc);
    assert(result == atlas::plugin::PluginLoadResult::NonDeterministic);
    std::cout << "[PASS] test_plugin_validate_nondeterministic" << std::endl;
}

void test_plugin_validate_incompatible_version() {
    atlas::plugin::PluginDescriptor desc{"Test", "1.0.0", "5.0.0", "graph-extension", true};
    auto result = atlas::plugin::PluginValidator::Validate(desc, "0.1.0");
    assert(result == atlas::plugin::PluginLoadResult::IncompatibleVersion);
    std::cout << "[PASS] test_plugin_validate_incompatible_version" << std::endl;
}

void test_plugin_registry_register() {
    atlas::plugin::PluginRegistry registry;
    atlas::plugin::PluginDescriptor desc{"MyPlugin", "1.0.0", "0.1.0", "graph-extension", true};
    assert(registry.RegisterPlugin(desc));
    assert(registry.PluginCount() == 1);
    std::cout << "[PASS] test_plugin_registry_register" << std::endl;
}

void test_plugin_registry_find() {
    atlas::plugin::PluginRegistry registry;
    registry.RegisterPlugin({"MyPlugin", "1.0.0", "0.1.0", "graph-extension", true});
    auto* found = registry.FindPlugin("MyPlugin");
    assert(found != nullptr);
    assert(found->name == "MyPlugin");
    assert(registry.FindPlugin("Missing") == nullptr);
    std::cout << "[PASS] test_plugin_registry_find" << std::endl;
}

void test_plugin_registry_unregister() {
    atlas::plugin::PluginRegistry registry;
    registry.RegisterPlugin({"ToRemove", "1.0.0", "0.1.0", "graph-extension", true});
    assert(registry.PluginCount() == 1);
    assert(registry.UnregisterPlugin("ToRemove"));
    assert(registry.PluginCount() == 0);
    assert(!registry.UnregisterPlugin("Missing"));
    std::cout << "[PASS] test_plugin_registry_unregister" << std::endl;
}

void test_plugin_registry_get_by_type() {
    atlas::plugin::PluginRegistry registry;
    registry.RegisterPlugin({"A", "1.0.0", "0.1.0", "graph-extension", true});
    registry.RegisterPlugin({"B", "1.0.0", "0.1.0", "editor-panel", true});
    registry.RegisterPlugin({"C", "1.0.0", "0.1.0", "graph-extension", true});

    auto graphPlugins = registry.GetByType("graph-extension");
    assert(graphPlugins.size() == 2);
    std::cout << "[PASS] test_plugin_registry_get_by_type" << std::endl;
}
