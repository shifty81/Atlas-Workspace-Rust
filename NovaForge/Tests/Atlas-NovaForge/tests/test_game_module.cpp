#include "../engine/module/IGameModule.h"
#include "../engine/module/ModuleLoader.h"
#include "../engine/ecs/ECS.h"
#include "../engine/net/NetContext.h"
#include "../engine/net/Replication.h"
#include "../engine/rules/ServerRules.h"
#include "../engine/assets/AssetRegistry.h"
#include "../engine/project/ProjectManager.h"
#include <iostream>
#include <cassert>
#include <memory>

using namespace atlas::module;

// A test game module for static linking
class TestGameModule : public IGameModule {
public:
    bool registerCalled = false;
    bool startCalled = false;
    bool tickCalled = false;
    bool shutdownCalled = false;
    float lastDt = 0.0f;

    GameModuleDesc Describe() const override {
        return {"TestGame", 1};
    }

    void RegisterTypes(GameModuleContext& ctx) override {
        (void)ctx;
        registerCalled = true;
    }

    void OnStart(GameModuleContext& ctx) override {
        (void)ctx;
        startCalled = true;
    }

    void OnTick(GameModuleContext& ctx, float dt) override {
        (void)ctx;
        tickCalled = true;
        lastDt = dt;
    }

    void OnShutdown(GameModuleContext& ctx) override {
        (void)ctx;
        shutdownCalled = true;
    }
};

void test_game_module_describe() {
    TestGameModule mod;
    auto desc = mod.Describe();
    assert(std::string(desc.name) == "TestGame");
    assert(desc.version == 1);
    std::cout << "[PASS] test_game_module_describe" << std::endl;
}

void test_game_module_lifecycle() {
    TestGameModule mod;
    atlas::ecs::World world;
    atlas::net::NetContext net;
    atlas::net::ReplicationManager replication;
    atlas::asset::AssetRegistry assets;

    auto& rules = atlas::rules::ServerRules::Get();
    const auto& project = atlas::project::ProjectManager::Get().Descriptor();

    GameModuleContext ctx{world, net, replication, rules, assets, project};

    assert(!mod.registerCalled);
    assert(!mod.startCalled);

    mod.RegisterTypes(ctx);
    assert(mod.registerCalled);

    mod.ConfigureReplication(ctx);
    mod.ConfigureServerRules(ctx);

    mod.OnStart(ctx);
    assert(mod.startCalled);

    mod.OnTick(ctx, 0.016f);
    assert(mod.tickCalled);
    assert(mod.lastDt > 0.015f && mod.lastDt < 0.017f);

    mod.OnShutdown(ctx);
    assert(mod.shutdownCalled);

    std::cout << "[PASS] test_game_module_lifecycle" << std::endl;
}

void test_module_loader_static() {
    ModuleLoader loader;
    assert(!loader.IsLoaded());
    assert(loader.GetModule() == nullptr);

    auto mod = std::make_unique<TestGameModule>();
    auto* rawPtr = mod.get();
    loader.SetStaticModule(std::move(mod));

    assert(loader.IsLoaded());
    assert(loader.GetModule() == rawPtr);

    loader.Unload();
    assert(!loader.IsLoaded());
    assert(loader.GetModule() == nullptr);

    std::cout << "[PASS] test_module_loader_static" << std::endl;
}

void test_module_loader_already_loaded() {
    ModuleLoader loader;
    loader.SetStaticModule(std::make_unique<TestGameModule>());

    auto result = loader.Load("nonexistent.so");
    assert(result == ModuleLoadResult::AlreadyLoaded);

    std::cout << "[PASS] test_module_loader_already_loaded" << std::endl;
}

void test_module_loader_not_found() {
    ModuleLoader loader;
    auto result = loader.Load("/tmp/no_such_module.so");
    assert(result == ModuleLoadResult::NotFound);
    assert(!loader.IsLoaded());

    std::cout << "[PASS] test_module_loader_not_found" << std::endl;
}
