// Tests/Workspace/test_phase14_plugin_system.cpp
// Phase 14 — Workspace Plugin System
//
// Tests for:
//   1. PluginState / PluginCapability / PluginVersion — enums, version ops
//   2. PluginDescriptor  — identity, dependencies, capabilities
//   3. PluginInstance     — lifecycle state machine, handlers
//   4. PluginSandbox      — capability-based permission model
//   5. PluginRegistry     — registration, dependency resolution, lifecycle
//   6. Integration        — full plugin pipeline

#include <catch2/catch_test_macros.hpp>
#include "NF/Workspace/WorkspacePluginSystem.h"
#include <string>
#include <vector>

using namespace NF;

// ═════════════════════════════════════════════════════════════════
// SECTION 1 — Enums and PluginVersion
// ═════════════════════════════════════════════════════════════════

TEST_CASE("pluginStateName returns correct strings", "[Phase14][State]") {
    CHECK(std::string(pluginStateName(PluginState::Unloaded))    == "Unloaded");
    CHECK(std::string(pluginStateName(PluginState::Discovered))  == "Discovered");
    CHECK(std::string(pluginStateName(PluginState::Loaded))      == "Loaded");
    CHECK(std::string(pluginStateName(PluginState::Activated))   == "Activated");
    CHECK(std::string(pluginStateName(PluginState::Deactivated)) == "Deactivated");
    CHECK(std::string(pluginStateName(PluginState::Error))       == "Error");
}

TEST_CASE("pluginCapabilityName returns correct strings", "[Phase14][Capability]") {
    CHECK(std::string(pluginCapabilityName(PluginCapability::ReadSettings))   == "ReadSettings");
    CHECK(std::string(pluginCapabilityName(PluginCapability::WriteSettings))  == "WriteSettings");
    CHECK(std::string(pluginCapabilityName(PluginCapability::RegisterTools))  == "RegisterTools");
    CHECK(std::string(pluginCapabilityName(PluginCapability::RegisterPanels)) == "RegisterPanels");
    CHECK(std::string(pluginCapabilityName(PluginCapability::FileSystem))     == "FileSystem");
    CHECK(std::string(pluginCapabilityName(PluginCapability::Network))        == "Network");
    CHECK(std::string(pluginCapabilityName(PluginCapability::EventBus))       == "EventBus");
    CHECK(std::string(pluginCapabilityName(PluginCapability::Commands))       == "Commands");
}

TEST_CASE("PluginVersion make and toString", "[Phase14][Version]") {
    auto v = PluginVersion::make(1, 2, 3);
    CHECK(v.toString() == "1.2.3");
    CHECK(v.isValid());
}

TEST_CASE("PluginVersion zero is not valid", "[Phase14][Version]") {
    PluginVersion v{0, 0, 0};
    CHECK_FALSE(v.isValid());
}

TEST_CASE("PluginVersion comparison operators", "[Phase14][Version]") {
    auto v1 = PluginVersion::make(1, 0, 0);
    auto v2 = PluginVersion::make(1, 1, 0);
    auto v3 = PluginVersion::make(1, 1, 0);
    auto v4 = PluginVersion::make(2, 0, 0);

    CHECK(v1 < v2);
    CHECK(v2 == v3);
    CHECK(v2 != v1);
    CHECK(v4 > v2);
    CHECK(v1 <= v2);
    CHECK(v2 >= v1);
}

TEST_CASE("PluginVersion parse", "[Phase14][Version]") {
    PluginVersion v;
    CHECK(PluginVersion::parse("1.2.3", v));
    CHECK(v.major == 1);
    CHECK(v.minor == 2);
    CHECK(v.patch == 3);

    CHECK_FALSE(PluginVersion::parse("abc", v));
    CHECK_FALSE(PluginVersion::parse("1.2", v));
}

// ═════════════════════════════════════════════════════════════════
// SECTION 2 — PluginDescriptor
// ═════════════════════════════════════════════════════════════════

TEST_CASE("PluginDescriptor default is not valid", "[Phase14][Descriptor]") {
    PluginDescriptor d;
    CHECK_FALSE(d.isValid());
}

TEST_CASE("PluginDescriptor with id, name, version is valid", "[Phase14][Descriptor]") {
    PluginDescriptor d;
    d.id = "my.plugin";
    d.displayName = "My Plugin";
    d.version = PluginVersion::make(1, 0, 0);
    CHECK(d.isValid());
}

TEST_CASE("PluginDescriptor dependsOn", "[Phase14][Descriptor]") {
    PluginDescriptor d;
    d.id = "child";
    d.displayName = "Child";
    d.version = PluginVersion::make(1, 0, 0);
    d.dependencies = {"parent_a", "parent_b"};

    CHECK(d.dependsOn("parent_a"));
    CHECK(d.dependsOn("parent_b"));
    CHECK_FALSE(d.dependsOn("parent_c"));
}

TEST_CASE("PluginDescriptor requiresCapability", "[Phase14][Descriptor]") {
    PluginDescriptor d;
    d.id = "plugin";
    d.displayName = "Plugin";
    d.version = PluginVersion::make(1, 0, 0);
    d.requiredCapabilities = {PluginCapability::ReadSettings, PluginCapability::EventBus};

    CHECK(d.requiresCapability(PluginCapability::ReadSettings));
    CHECK(d.requiresCapability(PluginCapability::EventBus));
    CHECK_FALSE(d.requiresCapability(PluginCapability::Network));
}

// ═════════════════════════════════════════════════════════════════
// SECTION 3 — PluginInstance
// ═════════════════════════════════════════════════════════════════

TEST_CASE("PluginInstance starts in Discovered state", "[Phase14][Instance]") {
    PluginDescriptor d;
    d.id = "plugin"; d.displayName = "P"; d.version = PluginVersion::make(1, 0, 0);
    PluginInstance inst(d);

    CHECK(inst.state() == PluginState::Discovered);
    CHECK(inst.id()    == "plugin");
    CHECK_FALSE(inst.isActive());
}

TEST_CASE("PluginInstance lifecycle: load → activate → deactivate → unload", "[Phase14][Instance]") {
    PluginDescriptor d;
    d.id = "plugin"; d.displayName = "P"; d.version = PluginVersion::make(1, 0, 0);
    PluginInstance inst(d);

    CHECK(inst.load());
    CHECK(inst.state() == PluginState::Loaded);

    CHECK(inst.activate());
    CHECK(inst.state() == PluginState::Activated);
    CHECK(inst.isActive());

    CHECK(inst.deactivate());
    CHECK(inst.state() == PluginState::Deactivated);
    CHECK_FALSE(inst.isActive());

    CHECK(inst.unload());
    CHECK(inst.state() == PluginState::Unloaded);
}

TEST_CASE("PluginInstance cannot activate without load", "[Phase14][Instance]") {
    PluginDescriptor d;
    d.id = "plugin"; d.displayName = "P"; d.version = PluginVersion::make(1, 0, 0);
    PluginInstance inst(d);

    CHECK_FALSE(inst.activate());
    CHECK(inst.state() == PluginState::Discovered);
}

TEST_CASE("PluginInstance activate handler failure sets Error", "[Phase14][Instance]") {
    PluginDescriptor d;
    d.id = "plugin"; d.displayName = "P"; d.version = PluginVersion::make(1, 0, 0);
    PluginInstance inst(d);
    inst.setActivateHandler([]() { return false; });

    inst.load();
    CHECK_FALSE(inst.activate());
    CHECK(inst.state() == PluginState::Error);
    CHECK(inst.errorMessage() == "activate failed");
}

TEST_CASE("PluginInstance activate/deactivate handlers called", "[Phase14][Instance]") {
    PluginDescriptor d;
    d.id = "plugin"; d.displayName = "P"; d.version = PluginVersion::make(1, 0, 0);
    PluginInstance inst(d);

    int activated = 0, deactivated = 0;
    inst.setActivateHandler([&]() { ++activated; return true; });
    inst.setDeactivateHandler([&]() { ++deactivated; });

    inst.load();
    inst.activate();
    CHECK(activated == 1);

    inst.deactivate();
    CHECK(deactivated == 1);
}

TEST_CASE("PluginInstance unload from Activated deactivates first", "[Phase14][Instance]") {
    PluginDescriptor d;
    d.id = "plugin"; d.displayName = "P"; d.version = PluginVersion::make(1, 0, 0);
    PluginInstance inst(d);

    int deactivated = 0;
    inst.setDeactivateHandler([&]() { ++deactivated; });

    inst.load();
    inst.activate();
    inst.unload();
    CHECK(deactivated == 1);
    CHECK(inst.state() == PluginState::Unloaded);
}

TEST_CASE("PluginInstance can reactivate after deactivation", "[Phase14][Instance]") {
    PluginDescriptor d;
    d.id = "plugin"; d.displayName = "P"; d.version = PluginVersion::make(1, 0, 0);
    PluginInstance inst(d);

    inst.load();
    inst.activate();
    inst.deactivate();
    CHECK(inst.activate());
    CHECK(inst.isActive());
}

TEST_CASE("PluginInstance setError transitions to Error", "[Phase14][Instance]") {
    PluginDescriptor d;
    d.id = "plugin"; d.displayName = "P"; d.version = PluginVersion::make(1, 0, 0);
    PluginInstance inst(d);

    inst.setError("crashed");
    CHECK(inst.state() == PluginState::Error);
    CHECK(inst.errorMessage() == "crashed");
}

// ═════════════════════════════════════════════════════════════════
// SECTION 4 — PluginSandbox
// ═════════════════════════════════════════════════════════════════

TEST_CASE("PluginSandbox empty on construction", "[Phase14][Sandbox]") {
    PluginSandbox sandbox;
    CHECK(sandbox.totalGrants() == 0);
}

TEST_CASE("PluginSandbox grant and check capability", "[Phase14][Sandbox]") {
    PluginSandbox sandbox;
    CHECK(sandbox.grant("plugin_a", PluginCapability::ReadSettings));
    CHECK(sandbox.hasCapability("plugin_a", PluginCapability::ReadSettings));
    CHECK_FALSE(sandbox.hasCapability("plugin_a", PluginCapability::Network));
    CHECK_FALSE(sandbox.hasCapability("plugin_b", PluginCapability::ReadSettings));
}

TEST_CASE("PluginSandbox rejects duplicate grant", "[Phase14][Sandbox]") {
    PluginSandbox sandbox;
    sandbox.grant("p", PluginCapability::EventBus);
    CHECK_FALSE(sandbox.grant("p", PluginCapability::EventBus));
}

TEST_CASE("PluginSandbox revoke", "[Phase14][Sandbox]") {
    PluginSandbox sandbox;
    sandbox.grant("p", PluginCapability::FileSystem);
    CHECK(sandbox.revoke("p", PluginCapability::FileSystem));
    CHECK_FALSE(sandbox.hasCapability("p", PluginCapability::FileSystem));
    CHECK_FALSE(sandbox.revoke("p", PluginCapability::FileSystem)); // already revoked
}

TEST_CASE("PluginSandbox grantRequired from descriptor", "[Phase14][Sandbox]") {
    PluginDescriptor d;
    d.id = "p"; d.displayName = "P"; d.version = PluginVersion::make(1, 0, 0);
    d.requiredCapabilities = {PluginCapability::ReadSettings, PluginCapability::EventBus, PluginCapability::Commands};

    PluginSandbox sandbox;
    size_t granted = sandbox.grantRequired(d);
    CHECK(granted == 3);
    CHECK(sandbox.hasCapability("p", PluginCapability::ReadSettings));
    CHECK(sandbox.hasCapability("p", PluginCapability::EventBus));
    CHECK(sandbox.hasCapability("p", PluginCapability::Commands));
}

TEST_CASE("PluginSandbox revokeAll removes all for plugin", "[Phase14][Sandbox]") {
    PluginSandbox sandbox;
    sandbox.grant("p", PluginCapability::ReadSettings);
    sandbox.grant("p", PluginCapability::Network);
    sandbox.grant("other", PluginCapability::FileSystem);

    size_t removed = sandbox.revokeAll("p");
    CHECK(removed == 2);
    CHECK(sandbox.countFor("p") == 0);
    CHECK(sandbox.countFor("other") == 1); // untouched
}

TEST_CASE("PluginSandbox countFor", "[Phase14][Sandbox]") {
    PluginSandbox sandbox;
    sandbox.grant("p", PluginCapability::ReadSettings);
    sandbox.grant("p", PluginCapability::WriteSettings);
    CHECK(sandbox.countFor("p") == 2);
}

TEST_CASE("PluginSandbox clear", "[Phase14][Sandbox]") {
    PluginSandbox sandbox;
    sandbox.grant("p", PluginCapability::Network);
    sandbox.clear();
    CHECK(sandbox.totalGrants() == 0);
}

// ═════════════════════════════════════════════════════════════════
// SECTION 5 — PluginRegistry
// ═════════════════════════════════════════════════════════════════

TEST_CASE("PluginRegistry empty on construction", "[Phase14][Registry]") {
    PluginRegistry reg;
    CHECK(reg.empty());
    CHECK(reg.count() == 0);
}

TEST_CASE("PluginRegistry register and find", "[Phase14][Registry]") {
    PluginRegistry reg;
    PluginDescriptor d;
    d.id = "plugin_a"; d.displayName = "Plugin A"; d.version = PluginVersion::make(1, 0, 0);

    CHECK(reg.registerPlugin(d));
    CHECK(reg.count() == 1);
    CHECK(reg.isRegistered("plugin_a"));

    auto* inst = reg.find("plugin_a");
    REQUIRE(inst != nullptr);
    CHECK(inst->state() == PluginState::Discovered);
}

TEST_CASE("PluginRegistry rejects duplicate", "[Phase14][Registry]") {
    PluginRegistry reg;
    PluginDescriptor d;
    d.id = "p"; d.displayName = "P"; d.version = PluginVersion::make(1, 0, 0);

    reg.registerPlugin(d);
    CHECK_FALSE(reg.registerPlugin(d));
}

TEST_CASE("PluginRegistry rejects invalid descriptor", "[Phase14][Registry]") {
    PluginRegistry reg;
    PluginDescriptor d; // empty
    CHECK_FALSE(reg.registerPlugin(d));
}

TEST_CASE("PluginRegistry load and activate", "[Phase14][Registry]") {
    PluginRegistry reg;
    PluginDescriptor d;
    d.id = "p"; d.displayName = "P"; d.version = PluginVersion::make(1, 0, 0);
    reg.registerPlugin(d);

    CHECK(reg.loadPlugin("p"));
    CHECK(reg.find("p")->state() == PluginState::Loaded);

    CHECK(reg.activatePlugin("p"));
    CHECK(reg.find("p")->isActive());
    CHECK(reg.activeCount() == 1);
}

TEST_CASE("PluginRegistry activate checks dependencies", "[Phase14][Registry]") {
    PluginRegistry reg;

    PluginDescriptor parent;
    parent.id = "parent"; parent.displayName = "Parent"; parent.version = PluginVersion::make(1, 0, 0);
    reg.registerPlugin(parent);

    PluginDescriptor child;
    child.id = "child"; child.displayName = "Child"; child.version = PluginVersion::make(1, 0, 0);
    child.dependencies = {"parent"};
    reg.registerPlugin(child);

    reg.loadPlugin("child");
    // Parent not activated → dependency not met
    CHECK_FALSE(reg.activatePlugin("child"));

    // Activate parent first
    reg.loadPlugin("parent");
    reg.activatePlugin("parent");
    CHECK(reg.activatePlugin("child"));
}

TEST_CASE("PluginRegistry deactivate cascades to dependents", "[Phase14][Registry]") {
    PluginRegistry reg;

    PluginDescriptor parent;
    parent.id = "parent"; parent.displayName = "Parent"; parent.version = PluginVersion::make(1, 0, 0);
    reg.registerPlugin(parent);

    PluginDescriptor child;
    child.id = "child"; child.displayName = "Child"; child.version = PluginVersion::make(1, 0, 0);
    child.dependencies = {"parent"};
    reg.registerPlugin(child);

    reg.loadPlugin("parent");
    reg.activatePlugin("parent");
    reg.loadPlugin("child");
    reg.activatePlugin("child");
    CHECK(reg.activeCount() == 2);

    // Deactivating parent should cascade to child
    reg.deactivatePlugin("parent");
    CHECK_FALSE(reg.find("parent")->isActive());
    CHECK_FALSE(reg.find("child")->isActive());
    CHECK(reg.activeCount() == 0);
}

TEST_CASE("PluginRegistry unregister active plugin fails", "[Phase14][Registry]") {
    PluginRegistry reg;
    PluginDescriptor d;
    d.id = "p"; d.displayName = "P"; d.version = PluginVersion::make(1, 0, 0);
    reg.registerPlugin(d);
    reg.loadPlugin("p");
    reg.activatePlugin("p");

    CHECK_FALSE(reg.unregisterPlugin("p"));
}

TEST_CASE("PluginRegistry unregister inactive plugin succeeds", "[Phase14][Registry]") {
    PluginRegistry reg;
    PluginDescriptor d;
    d.id = "p"; d.displayName = "P"; d.version = PluginVersion::make(1, 0, 0);
    reg.registerPlugin(d);
    reg.loadPlugin("p");
    reg.activatePlugin("p");
    reg.deactivatePlugin("p");

    CHECK(reg.unregisterPlugin("p"));
    CHECK(reg.count() == 0);
}

TEST_CASE("PluginRegistry findByState", "[Phase14][Registry]") {
    PluginRegistry reg;

    PluginDescriptor d1;
    d1.id = "a"; d1.displayName = "A"; d1.version = PluginVersion::make(1, 0, 0);
    reg.registerPlugin(d1);

    PluginDescriptor d2;
    d2.id = "b"; d2.displayName = "B"; d2.version = PluginVersion::make(1, 0, 0);
    reg.registerPlugin(d2);

    reg.loadPlugin("a");
    reg.activatePlugin("a");

    auto active = reg.findByState(PluginState::Activated);
    CHECK(active.size() == 1);
    CHECK(active[0]->id() == "a");

    auto discovered = reg.findByState(PluginState::Discovered);
    CHECK(discovered.size() == 1);
    CHECK(discovered[0]->id() == "b");
}

TEST_CASE("PluginRegistry areDependenciesMet", "[Phase14][Registry]") {
    PluginRegistry reg;

    PluginDescriptor d;
    d.id = "solo"; d.displayName = "Solo"; d.version = PluginVersion::make(1, 0, 0);
    reg.registerPlugin(d);
    CHECK(reg.areDependenciesMet("solo")); // no deps

    PluginDescriptor dep;
    dep.id = "dep"; dep.displayName = "Dep"; dep.version = PluginVersion::make(1, 0, 0);
    dep.dependencies = {"missing"};
    reg.registerPlugin(dep);
    CHECK_FALSE(reg.areDependenciesMet("dep")); // missing dep
}

TEST_CASE("PluginRegistry clear", "[Phase14][Registry]") {
    PluginRegistry reg;
    PluginDescriptor d;
    d.id = "p"; d.displayName = "P"; d.version = PluginVersion::make(1, 0, 0);
    reg.registerPlugin(d);
    reg.clear();
    CHECK(reg.empty());
}

// ═════════════════════════════════════════════════════════════════
// SECTION 6 — Integration Tests
// ═════════════════════════════════════════════════════════════════

TEST_CASE("Integration: full plugin lifecycle with sandbox", "[Phase14][Integration]") {
    PluginRegistry reg;
    PluginSandbox sandbox;

    PluginDescriptor d;
    d.id = "my.plugin"; d.displayName = "My Plugin"; d.version = PluginVersion::make(1, 0, 0);
    d.author = "Test";
    d.requiredCapabilities = {PluginCapability::ReadSettings, PluginCapability::EventBus};

    // Register and grant capabilities
    reg.registerPlugin(d);
    sandbox.grantRequired(d);
    CHECK(sandbox.countFor("my.plugin") == 2);

    // Lifecycle
    reg.loadPlugin("my.plugin");
    reg.activatePlugin("my.plugin");
    CHECK(reg.find("my.plugin")->isActive());

    // Deactivate and revoke
    reg.deactivatePlugin("my.plugin");
    sandbox.revokeAll("my.plugin");
    CHECK(sandbox.countFor("my.plugin") == 0);

    reg.unloadPlugin("my.plugin");
    CHECK(reg.find("my.plugin")->state() == PluginState::Unloaded);
}

TEST_CASE("Integration: dependency chain — A → B → C", "[Phase14][Integration]") {
    PluginRegistry reg;

    PluginDescriptor a;
    a.id = "a"; a.displayName = "A"; a.version = PluginVersion::make(1, 0, 0);
    reg.registerPlugin(a);

    PluginDescriptor b;
    b.id = "b"; b.displayName = "B"; b.version = PluginVersion::make(1, 0, 0);
    b.dependencies = {"a"};
    reg.registerPlugin(b);

    PluginDescriptor c;
    c.id = "c"; c.displayName = "C"; c.version = PluginVersion::make(1, 0, 0);
    c.dependencies = {"b"};
    reg.registerPlugin(c);

    // Must activate in dependency order
    reg.loadPlugin("a"); reg.loadPlugin("b"); reg.loadPlugin("c");

    CHECK_FALSE(reg.activatePlugin("c")); // b not active
    CHECK_FALSE(reg.activatePlugin("b")); // a not active
    CHECK(reg.activatePlugin("a"));
    CHECK(reg.activatePlugin("b"));
    CHECK(reg.activatePlugin("c"));
    CHECK(reg.activeCount() == 3);

    // Deactivating A cascades to B, and B cascading deactivates C
    reg.deactivatePlugin("a");
    CHECK(reg.activeCount() == 0);
}

TEST_CASE("Integration: plugin with handlers", "[Phase14][Integration]") {
    PluginRegistry reg;

    PluginDescriptor d;
    d.id = "p"; d.displayName = "P"; d.version = PluginVersion::make(1, 0, 0);
    reg.registerPlugin(d);

    int initCount = 0, shutdownCount = 0;
    auto* inst = reg.find("p");
    inst->setActivateHandler([&]() { ++initCount; return true; });
    inst->setDeactivateHandler([&]() { ++shutdownCount; });

    reg.loadPlugin("p");
    reg.activatePlugin("p");
    CHECK(initCount == 1);

    reg.deactivatePlugin("p");
    CHECK(shutdownCount == 1);

    // Reactivate
    reg.activatePlugin("p");
    CHECK(initCount == 2);
}

TEST_CASE("Integration: version comparison for compatibility check", "[Phase14][Integration]") {
    auto required = PluginVersion::make(2, 0, 0);
    auto actual   = PluginVersion::make(2, 1, 3);

    CHECK(actual >= required); // compatible: actual >= required
    CHECK_FALSE(PluginVersion::make(1, 9, 9) >= required); // too old
}
