#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"

using namespace NF;

// ── S14 Tests: Plugin System ─────────────────────────────────────────────────

TEST_CASE("PluginState names", "[Editor][S14]") {
    REQUIRE(std::string(pluginStateName(PluginState::Unloaded))  == "Unloaded");
    REQUIRE(std::string(pluginStateName(PluginState::Loading))   == "Loading");
    REQUIRE(std::string(pluginStateName(PluginState::Loaded))    == "Loaded");
    REQUIRE(std::string(pluginStateName(PluginState::Active))    == "Active");
    REQUIRE(std::string(pluginStateName(PluginState::Suspended)) == "Suspended");
    REQUIRE(std::string(pluginStateName(PluginState::Error))     == "Error");
    REQUIRE(std::string(pluginStateName(PluginState::Disabled))  == "Disabled");
    REQUIRE(std::string(pluginStateName(PluginState::Unloading)) == "Unloading");
}

TEST_CASE("PluginManifest isValid", "[Editor][S14]") {
    PluginManifest m;
    REQUIRE_FALSE(m.isValid());

    m.id = "my.plugin";
    REQUIRE_FALSE(m.isValid()); // missing name

    m.name = "My Plugin";
    REQUIRE_FALSE(m.isValid()); // missing version

    m.version = "1.0.0";
    REQUIRE(m.isValid());
}

TEST_CASE("PluginInstance state transitions", "[Editor][S14]") {
    PluginInstance inst;
    inst.manifest.id      = "test.plugin";
    inst.manifest.name    = "Test";
    inst.manifest.version = "1.0";
    inst.state = PluginState::Loaded;

    REQUIRE(inst.isLoaded());
    REQUIRE_FALSE(inst.isActive());
    REQUIRE_FALSE(inst.hasError());

    REQUIRE(inst.activate());
    REQUIRE(inst.isActive());

    REQUIRE(inst.suspend());
    REQUIRE_FALSE(inst.isActive());
    REQUIRE(inst.isLoaded()); // Suspended counts as loaded

    REQUIRE(inst.activate()); // Can re-activate from Suspended
    REQUIRE(inst.isActive());
}

TEST_CASE("PluginInstance setError", "[Editor][S14]") {
    PluginInstance inst;
    inst.state = PluginState::Loaded;
    inst.setError("Failed to initialize");
    REQUIRE(inst.hasError());
    REQUIRE(inst.errorMessage == "Failed to initialize");
    REQUIRE_FALSE(inst.isLoaded());
}

TEST_CASE("PluginInstance disable", "[Editor][S14]") {
    PluginInstance inst;
    inst.state = PluginState::Active;
    REQUIRE(inst.disable());
    REQUIRE(inst.isDisabled());

    // Cannot disable an unloaded plugin
    PluginInstance inst2;
    inst2.state = PluginState::Unloaded;
    REQUIRE_FALSE(inst2.disable());
}

TEST_CASE("PluginRegistry registerPlugin + duplicate rejection", "[Editor][S14]") {
    PluginRegistry reg;
    PluginManifest m; m.id = "plugin.a"; m.name = "Plugin A"; m.version = "1.0";
    REQUIRE(reg.registerPlugin(m));
    REQUIRE(reg.pluginCount() == 1);
    REQUIRE_FALSE(reg.registerPlugin(m)); // duplicate id
    REQUIRE(reg.pluginCount() == 1);
}

TEST_CASE("PluginRegistry unregisterPlugin", "[Editor][S14]") {
    PluginRegistry reg;
    PluginManifest m; m.id = "plugin.a"; m.name = "Plugin A"; m.version = "1.0";
    reg.registerPlugin(m);
    REQUIRE(reg.unregisterPlugin("plugin.a"));
    REQUIRE(reg.pluginCount() == 0);
    REQUIRE_FALSE(reg.unregisterPlugin("nonexistent"));
}

TEST_CASE("PluginRegistry findPlugin", "[Editor][S14]") {
    PluginRegistry reg;
    PluginManifest m; m.id = "plugin.a"; m.name = "Plugin A"; m.version = "1.0";
    reg.registerPlugin(m);
    REQUIRE(reg.findPlugin("plugin.a") != nullptr);
    REQUIRE(reg.findPlugin("plugin.a")->manifest.id == "plugin.a");
    REQUIRE(reg.findPlugin("plugin.b") == nullptr);
}

TEST_CASE("PluginRegistry pluginsByState + enabledCount", "[Editor][S14]") {
    PluginRegistry reg;
    PluginManifest m1; m1.id = "a"; m1.name = "A"; m1.version = "1";
    PluginManifest m2; m2.id = "b"; m2.name = "B"; m2.version = "1";
    PluginManifest m3; m3.id = "c"; m3.name = "C"; m3.version = "1";
    reg.registerPlugin(m1);
    reg.registerPlugin(m2);
    reg.registerPlugin(m3);

    reg.findPlugin("a")->state = PluginState::Active;
    reg.findPlugin("b")->state = PluginState::Active;
    reg.findPlugin("c")->state = PluginState::Disabled;

    REQUIRE(reg.pluginsByState(PluginState::Active).size() == 2);
    REQUIRE(reg.pluginsByState(PluginState::Disabled).size() == 1);
    REQUIRE(reg.enabledCount() == 2);
}

TEST_CASE("PluginLoader load/unload", "[Editor][S14]") {
    PluginRegistry reg;
    PluginManifest m; m.id = "plugin.a"; m.name = "A"; m.version = "1.0";
    reg.registerPlugin(m);

    PluginLoader loader;
    REQUIRE(loader.load(reg, "plugin.a"));
    REQUIRE(loader.loadCount() == 1);

    auto* inst = reg.findPlugin("plugin.a");
    REQUIRE(inst != nullptr);
    REQUIRE(inst->state == PluginState::Loaded);

    REQUIRE(loader.unload(reg, "plugin.a"));
    REQUIRE(loader.unloadCount() == 1);
    REQUIRE(inst->state == PluginState::Unloaded);
}

TEST_CASE("PluginLoader reload", "[Editor][S14]") {
    PluginRegistry reg;
    PluginManifest m; m.id = "plugin.a"; m.name = "A"; m.version = "1.0";
    reg.registerPlugin(m);

    PluginLoader loader;
    loader.load(reg, "plugin.a");
    REQUIRE(loader.reload(reg, "plugin.a"));
    REQUIRE(loader.loadCount() == 2);
    REQUIRE(loader.unloadCount() == 1);
    REQUIRE(reg.findPlugin("plugin.a")->state == PluginState::Loaded);
}

TEST_CASE("PluginSystem init/shutdown", "[Editor][S14]") {
    PluginSystem sys;
    REQUIRE_FALSE(sys.isInitialized());
    sys.init();
    REQUIRE(sys.isInitialized());
    sys.shutdown();
    REQUIRE_FALSE(sys.isInitialized());
}

TEST_CASE("PluginSystem registerPlugin + loadPlugin + activatePlugin", "[Editor][S14]") {
    PluginSystem sys;
    sys.init();

    PluginManifest m; m.id = "plugin.a"; m.name = "A"; m.version = "1.0";
    REQUIRE(sys.registerPlugin(m));
    REQUIRE(sys.loadPlugin("plugin.a"));

    auto* inst = sys.findPlugin("plugin.a");
    REQUIRE(inst != nullptr);
    REQUIRE(inst->state == PluginState::Loaded);

    REQUIRE(sys.activatePlugin("plugin.a"));
    REQUIRE(inst->state == PluginState::Active);
    REQUIRE(sys.activePluginCount() == 1);
}

TEST_CASE("PluginSystem suspendPlugin", "[Editor][S14]") {
    PluginSystem sys;
    sys.init();

    PluginManifest m; m.id = "plugin.a"; m.name = "A"; m.version = "1.0";
    sys.registerPlugin(m);
    sys.loadPlugin("plugin.a");
    sys.activatePlugin("plugin.a");

    REQUIRE(sys.suspendPlugin("plugin.a"));
    REQUIRE(sys.activePluginCount() == 0);
    REQUIRE(sys.findPlugin("plugin.a")->state == PluginState::Suspended);
}

TEST_CASE("PluginSystem autoActivateOnLoad config", "[Editor][S14]") {
    PluginSystem sys;
    PluginSystemConfig config;
    config.autoActivateOnLoad = true;
    sys.init(config);

    PluginManifest m; m.id = "plugin.a"; m.name = "A"; m.version = "1.0";
    sys.registerPlugin(m);
    sys.loadPlugin("plugin.a");

    REQUIRE(sys.findPlugin("plugin.a")->state == PluginState::Active);
    REQUIRE(sys.activePluginCount() == 1);
}

TEST_CASE("PluginSystem tick + not-initialized rejects", "[Editor][S14]") {
    PluginSystem sys;
    PluginManifest m; m.id = "x"; m.name = "X"; m.version = "1";
    REQUIRE_FALSE(sys.registerPlugin(m)); // not initialized

    sys.init();
    sys.tick(0.016f);
    sys.tick(0.016f);
    REQUIRE(sys.tickCount() == 2);
}
