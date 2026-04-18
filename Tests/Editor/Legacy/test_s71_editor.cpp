#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"

using namespace NF;

// ── S71: PluginSystem ────────────────────────────────────────────

TEST_CASE("PluginState names are correct", "[Editor][S71]") {
    REQUIRE(std::string(pluginStateName(PluginState::Unloaded))  == "Unloaded");
    REQUIRE(std::string(pluginStateName(PluginState::Loading))   == "Loading");
    REQUIRE(std::string(pluginStateName(PluginState::Loaded))    == "Loaded");
    REQUIRE(std::string(pluginStateName(PluginState::Active))    == "Active");
    REQUIRE(std::string(pluginStateName(PluginState::Suspended)) == "Suspended");
    REQUIRE(std::string(pluginStateName(PluginState::Error))     == "Error");
    REQUIRE(std::string(pluginStateName(PluginState::Disabled))  == "Disabled");
    REQUIRE(std::string(pluginStateName(PluginState::Unloading)) == "Unloading");
}

TEST_CASE("PluginManifest defaults are invalid", "[Editor][S71]") {
    PluginManifest m;
    REQUIRE_FALSE(m.isValid());
}

TEST_CASE("PluginManifest valid when id, name and version set", "[Editor][S71]") {
    PluginManifest m;
    m.id = "my.plugin";
    m.name = "My Plugin";
    m.version = "1.0.0";
    REQUIRE(m.isValid());
}

TEST_CASE("PluginManifest invalid without version", "[Editor][S71]") {
    PluginManifest m;
    m.id = "p";
    m.name = "Plugin";
    REQUIRE_FALSE(m.isValid());
}

TEST_CASE("PluginInstance default state is Unloaded", "[Editor][S71]") {
    PluginInstance inst;
    REQUIRE(inst.state == PluginState::Unloaded);
}

TEST_CASE("PluginInstance isLoaded false for Unloaded", "[Editor][S71]") {
    PluginInstance inst;
    REQUIRE_FALSE(inst.isLoaded());
}

TEST_CASE("PluginInstance isLoaded true for Active", "[Editor][S71]") {
    PluginInstance inst;
    inst.state = PluginState::Active;
    REQUIRE(inst.isLoaded());
    REQUIRE(inst.isActive());
}

TEST_CASE("PluginInstance activate from Loaded succeeds", "[Editor][S71]") {
    PluginInstance inst;
    inst.state = PluginState::Loaded;
    REQUIRE(inst.activate());
    REQUIRE(inst.isActive());
}

TEST_CASE("PluginInstance activate from Unloaded fails", "[Editor][S71]") {
    PluginInstance inst;
    REQUIRE_FALSE(inst.activate());
}

TEST_CASE("PluginInstance suspend from Active succeeds", "[Editor][S71]") {
    PluginInstance inst;
    inst.state = PluginState::Active;
    REQUIRE(inst.suspend());
    REQUIRE(inst.state == PluginState::Suspended);
}

TEST_CASE("PluginInstance suspend from Loaded fails", "[Editor][S71]") {
    PluginInstance inst;
    inst.state = PluginState::Loaded;
    REQUIRE_FALSE(inst.suspend());
}

TEST_CASE("PluginInstance disable from Loaded succeeds", "[Editor][S71]") {
    PluginInstance inst;
    inst.state = PluginState::Loaded;
    REQUIRE(inst.disable());
    REQUIRE(inst.isDisabled());
}

TEST_CASE("PluginInstance disable from Unloaded fails", "[Editor][S71]") {
    PluginInstance inst;
    REQUIRE_FALSE(inst.disable());
}

TEST_CASE("PluginInstance setError sets Error state", "[Editor][S71]") {
    PluginInstance inst;
    inst.state = PluginState::Loaded;
    inst.setError("missing dependency");
    REQUIRE(inst.hasError());
    REQUIRE(inst.errorMessage == "missing dependency");
}

TEST_CASE("PluginRegistry starts empty", "[Editor][S71]") {
    PluginRegistry reg;
    REQUIRE(reg.pluginCount() == 0);
}

TEST_CASE("PluginRegistry registerPlugin succeeds for valid manifest", "[Editor][S71]") {
    PluginRegistry reg;
    PluginManifest m;
    m.id = "plugin.a";
    m.name = "Plugin A";
    m.version = "1.0";
    REQUIRE(reg.registerPlugin(m));
    REQUIRE(reg.pluginCount() == 1);
}

TEST_CASE("PluginRegistry registerPlugin rejects invalid manifest", "[Editor][S71]") {
    PluginRegistry reg;
    PluginManifest m;
    REQUIRE_FALSE(reg.registerPlugin(m));
}

TEST_CASE("PluginRegistry registerPlugin rejects duplicate id", "[Editor][S71]") {
    PluginRegistry reg;
    PluginManifest m;
    m.id = "p";
    m.name = "P";
    m.version = "1";
    reg.registerPlugin(m);
    REQUIRE_FALSE(reg.registerPlugin(m));
}

TEST_CASE("PluginRegistry unregisterPlugin removes entry", "[Editor][S71]") {
    PluginRegistry reg;
    PluginManifest m;
    m.id = "p";
    m.name = "P";
    m.version = "1";
    reg.registerPlugin(m);
    REQUIRE(reg.unregisterPlugin("p"));
    REQUIRE(reg.pluginCount() == 0);
}

TEST_CASE("PluginRegistry unregisterPlugin returns false for missing", "[Editor][S71]") {
    PluginRegistry reg;
    REQUIRE_FALSE(reg.unregisterPlugin("nonexistent"));
}

TEST_CASE("PluginRegistry findPlugin returns correct instance", "[Editor][S71]") {
    PluginRegistry reg;
    PluginManifest m;
    m.id = "plugin.b";
    m.name = "B";
    m.version = "2";
    reg.registerPlugin(m);
    REQUIRE(reg.findPlugin("plugin.b") != nullptr);
    REQUIRE(reg.findPlugin("plugin.b")->manifest.id == "plugin.b");
}

TEST_CASE("PluginRegistry findPlugin returns nullptr for missing", "[Editor][S71]") {
    PluginRegistry reg;
    REQUIRE(reg.findPlugin("none") == nullptr);
}

TEST_CASE("PluginRegistry enabledCount excludes Disabled and Error plugins", "[Editor][S71]") {
    PluginRegistry reg;
    auto addPlugin = [&](const std::string& id) {
        PluginManifest m;
        m.id = id; m.name = id; m.version = "1";
        reg.registerPlugin(m);
    };
    addPlugin("a"); addPlugin("b"); addPlugin("c");
    reg.findPlugin("b")->state = PluginState::Disabled;
    reg.findPlugin("c")->state = PluginState::Error;
    REQUIRE(reg.enabledCount() == 1);
}

TEST_CASE("PluginRegistry pluginsByState filters correctly", "[Editor][S71]") {
    PluginRegistry reg;
    PluginManifest m1; m1.id = "p1"; m1.name = "P1"; m1.version = "1";
    PluginManifest m2; m2.id = "p2"; m2.name = "P2"; m2.version = "1";
    reg.registerPlugin(m1);
    reg.registerPlugin(m2);
    reg.findPlugin("p1")->state = PluginState::Active;
    auto active = reg.pluginsByState(PluginState::Active);
    REQUIRE(active.size() == 1);
}

TEST_CASE("PluginRegistry kMaxPlugins is 64", "[Editor][S71]") {
    REQUIRE(PluginRegistry::kMaxPlugins == 64);
}

TEST_CASE("PluginLoader load transitions state to Loaded", "[Editor][S71]") {
    PluginRegistry reg;
    PluginManifest m; m.id = "p"; m.name = "P"; m.version = "1";
    reg.registerPlugin(m);
    PluginLoader loader;
    REQUIRE(loader.load(reg, "p"));
    REQUIRE(reg.findPlugin("p")->state == PluginState::Loaded);
    REQUIRE(loader.loadCount() == 1);
}

TEST_CASE("PluginLoader load fails for missing plugin", "[Editor][S71]") {
    PluginRegistry reg;
    PluginLoader loader;
    REQUIRE_FALSE(loader.load(reg, "ghost"));
}

TEST_CASE("PluginLoader load fails for already loaded plugin", "[Editor][S71]") {
    PluginRegistry reg;
    PluginManifest m; m.id = "p"; m.name = "P"; m.version = "1";
    reg.registerPlugin(m);
    PluginLoader loader;
    loader.load(reg, "p");
    REQUIRE_FALSE(loader.load(reg, "p"));
}

TEST_CASE("PluginLoader unload transitions state to Unloaded", "[Editor][S71]") {
    PluginRegistry reg;
    PluginManifest m; m.id = "p"; m.name = "P"; m.version = "1";
    reg.registerPlugin(m);
    PluginLoader loader;
    loader.load(reg, "p");
    REQUIRE(loader.unload(reg, "p"));
    REQUIRE(reg.findPlugin("p")->state == PluginState::Unloaded);
    REQUIRE(loader.unloadCount() == 1);
}

TEST_CASE("PluginLoader reload succeeds", "[Editor][S71]") {
    PluginRegistry reg;
    PluginManifest m; m.id = "p"; m.name = "P"; m.version = "1";
    reg.registerPlugin(m);
    PluginLoader loader;
    loader.load(reg, "p");
    REQUIRE(loader.reload(reg, "p"));
    REQUIRE(reg.findPlugin("p")->state == PluginState::Loaded);
}

TEST_CASE("PluginSystem starts uninitialized", "[Editor][S71]") {
    PluginSystem sys;
    REQUIRE_FALSE(sys.isInitialized());
}

TEST_CASE("PluginSystem init sets initialized", "[Editor][S71]") {
    PluginSystem sys;
    sys.init();
    REQUIRE(sys.isInitialized());
}

TEST_CASE("PluginSystem registerPlugin before init fails", "[Editor][S71]") {
    PluginSystem sys;
    PluginManifest m; m.id = "p"; m.name = "P"; m.version = "1";
    REQUIRE_FALSE(sys.registerPlugin(m));
}

TEST_CASE("PluginSystem registerPlugin after init succeeds", "[Editor][S71]") {
    PluginSystem sys;
    sys.init();
    PluginManifest m; m.id = "p"; m.name = "P"; m.version = "1";
    REQUIRE(sys.registerPlugin(m));
    REQUIRE(sys.totalPluginCount() == 1);
}

TEST_CASE("PluginSystem loadPlugin transitions to Loaded", "[Editor][S71]") {
    PluginSystem sys;
    sys.init();
    PluginManifest m; m.id = "p"; m.name = "P"; m.version = "1";
    sys.registerPlugin(m);
    REQUIRE(sys.loadPlugin("p"));
    REQUIRE(sys.findPlugin("p")->state == PluginState::Loaded);
}

TEST_CASE("PluginSystem autoActivateOnLoad activates plugin after load", "[Editor][S71]") {
    PluginSystem sys;
    PluginSystemConfig cfg;
    cfg.autoActivateOnLoad = true;
    sys.init(cfg);
    PluginManifest m; m.id = "p"; m.name = "P"; m.version = "1";
    sys.registerPlugin(m);
    sys.loadPlugin("p");
    REQUIRE(sys.findPlugin("p")->isActive());
    REQUIRE(sys.activePluginCount() == 1);
}

TEST_CASE("PluginSystem activatePlugin succeeds after load", "[Editor][S71]") {
    PluginSystem sys;
    sys.init();
    PluginManifest m; m.id = "p"; m.name = "P"; m.version = "1";
    sys.registerPlugin(m);
    sys.loadPlugin("p");
    REQUIRE(sys.activatePlugin("p"));
    REQUIRE(sys.findPlugin("p")->isActive());
}

TEST_CASE("PluginSystem suspendPlugin suspends active plugin", "[Editor][S71]") {
    PluginSystem sys;
    sys.init();
    PluginManifest m; m.id = "p"; m.name = "P"; m.version = "1";
    sys.registerPlugin(m);
    sys.loadPlugin("p");
    sys.activatePlugin("p");
    REQUIRE(sys.suspendPlugin("p"));
    REQUIRE(sys.findPlugin("p")->state == PluginState::Suspended);
}

TEST_CASE("PluginSystem unloadPlugin unloads loaded plugin", "[Editor][S71]") {
    PluginSystem sys;
    sys.init();
    PluginManifest m; m.id = "p"; m.name = "P"; m.version = "1";
    sys.registerPlugin(m);
    sys.loadPlugin("p");
    REQUIRE(sys.unloadPlugin("p"));
    REQUIRE(sys.findPlugin("p")->state == PluginState::Unloaded);
}

TEST_CASE("PluginSystem tick increments tickCount", "[Editor][S71]") {
    PluginSystem sys;
    sys.init();
    REQUIRE(sys.tickCount() == 0);
    sys.tick(0.016f);
    REQUIRE(sys.tickCount() == 1);
}

TEST_CASE("PluginSystem shutdown clears plugins", "[Editor][S71]") {
    PluginSystem sys;
    sys.init();
    PluginManifest m; m.id = "p"; m.name = "P"; m.version = "1";
    sys.registerPlugin(m);
    sys.shutdown();
    REQUIRE_FALSE(sys.isInitialized());
    REQUIRE(sys.totalPluginCount() == 0);
}
