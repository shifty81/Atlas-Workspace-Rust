#include "../projects/arena2d/module/Arena2DModule.h"
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

extern "C" atlas::module::IGameModule* CreateArena2DModule();

static GameModuleContext MakeArena2DTestContext(atlas::ecs::World& world,
                                                atlas::net::NetContext& net,
                                                atlas::net::ReplicationManager& repl,
                                                atlas::asset::AssetRegistry& assets) {
    return {world, net, repl, atlas::rules::ServerRules::Get(), assets,
            atlas::project::ProjectManager::Get().Descriptor()};
}

void test_arena2d_describe() {
    arena2d::Arena2DModule mod;
    auto desc = mod.Describe();
    assert(std::string(desc.name) == "Arena2D");
    assert(desc.version == 1);
    std::cout << "[PASS] test_arena2d_describe" << std::endl;
}

void test_arena2d_register_types() {
    arena2d::Arena2DModule mod;
    atlas::ecs::World world;
    atlas::net::NetContext net;
    atlas::net::ReplicationManager repl;
    atlas::asset::AssetRegistry assets;
    auto ctx = MakeArena2DTestContext(world, net, repl, assets);

    mod.RegisterTypes(ctx);

    // Should have registered 3 factions (Heroes, Monsters, Neutrals)
    assert(mod.Factions().FactionCount() == 3);
    // Economy should have 2 resources (Gold, XP)
    assert(mod.Economy().ResourceCount() == 2);

    std::cout << "[PASS] test_arena2d_register_types" << std::endl;
}

void test_arena2d_replication_rules() {
    arena2d::Arena2DModule mod;
    atlas::ecs::World world;
    atlas::net::NetContext net;
    atlas::net::ReplicationManager repl;
    atlas::asset::AssetRegistry assets;
    repl.SetWorld(&world);
    auto ctx = MakeArena2DTestContext(world, net, repl, assets);

    mod.ConfigureReplication(ctx);

    assert(repl.RuleCount() == 3);
    assert(repl.HasRule(arena2d::Tag_Position));
    assert(repl.HasRule(arena2d::Tag_Health));
    assert(repl.HasRule(arena2d::Tag_SpriteInfo));

    std::cout << "[PASS] test_arena2d_replication_rules" << std::endl;
}

void test_arena2d_server_rules() {
    arena2d::Arena2DModule mod;
    atlas::ecs::World world;
    atlas::net::NetContext net;
    atlas::net::ReplicationManager repl;
    atlas::asset::AssetRegistry assets;

    auto& rules = atlas::rules::ServerRules::Get();
    rules.Clear();

    auto ctx = MakeArena2DTestContext(world, net, repl, assets);
    mod.ConfigureServerRules(ctx);

    assert(rules.HasRule("enemySpawnRate"));
    assert(rules.HasRule("playerDamageMultiplier"));
    assert(rules.HasRule("xpMultiplier"));

    std::cout << "[PASS] test_arena2d_server_rules" << std::endl;
}

void test_arena2d_lifecycle() {
    arena2d::Arena2DModule mod;
    atlas::ecs::World world;
    atlas::net::NetContext net;
    atlas::net::ReplicationManager repl;
    atlas::asset::AssetRegistry assets;
    auto ctx = MakeArena2DTestContext(world, net, repl, assets);

    assert(!mod.IsStarted());
    assert(mod.TickCount() == 0);

    mod.RegisterTypes(ctx);
    mod.OnStart(ctx);
    assert(mod.IsStarted());

    mod.OnTick(ctx, 0.016f);
    mod.OnTick(ctx, 0.016f);
    assert(mod.TickCount() == 2);

    mod.OnShutdown(ctx);
    assert(!mod.IsStarted());
    assert(mod.TickCount() == 0);

    std::cout << "[PASS] test_arena2d_lifecycle" << std::endl;
}

void test_arena2d_factory() {
    std::unique_ptr<atlas::module::IGameModule> mod(CreateArena2DModule());
    assert(mod != nullptr);
    auto desc = mod->Describe();
    assert(std::string(desc.name) == "Arena2D");

    std::cout << "[PASS] test_arena2d_factory" << std::endl;
}
