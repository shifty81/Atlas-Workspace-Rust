#include "../projects/eveoffline/module/EveOfflineModule.h"
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

extern "C" atlas::module::IGameModule* CreateGameModule();

static GameModuleContext MakeTestContext(atlas::ecs::World& world,
                                        atlas::net::NetContext& net,
                                        atlas::net::ReplicationManager& repl,
                                        atlas::asset::AssetRegistry& assets) {
    return {world, net, repl, atlas::rules::ServerRules::Get(), assets,
            atlas::project::ProjectManager::Get().Descriptor()};
}

void test_eveoffline_describe() {
    eveoffline::EveOfflineModule mod;
    auto desc = mod.Describe();
    assert(std::string(desc.name) == "EveOffline");
    assert(desc.version == 1);
    std::cout << "[PASS] test_eveoffline_describe" << std::endl;
}

void test_eveoffline_register_types() {
    eveoffline::EveOfflineModule mod;
    atlas::ecs::World world;
    atlas::net::NetContext net;
    atlas::net::ReplicationManager repl;
    atlas::asset::AssetRegistry assets;
    auto ctx = MakeTestContext(world, net, repl, assets);

    mod.RegisterTypes(ctx);

    // Should have registered 5 factions
    assert(mod.Factions().FactionCount() == 5);
    // Economy should have 5 resources
    assert(mod.Economy().ResourceCount() == 5);

    std::cout << "[PASS] test_eveoffline_register_types" << std::endl;
}

void test_eveoffline_replication_rules() {
    eveoffline::EveOfflineModule mod;
    atlas::ecs::World world;
    atlas::net::NetContext net;
    atlas::net::ReplicationManager repl;
    atlas::asset::AssetRegistry assets;
    repl.SetWorld(&world);
    auto ctx = MakeTestContext(world, net, repl, assets);

    mod.ConfigureReplication(ctx);

    assert(repl.RuleCount() == 3);
    assert(repl.HasRule(eveoffline::Tag_ShipPosition));
    assert(repl.HasRule(eveoffline::Tag_ShipHealth));
    assert(repl.HasRule(eveoffline::Tag_Inventory));

    std::cout << "[PASS] test_eveoffline_replication_rules" << std::endl;
}

void test_eveoffline_server_rules() {
    eveoffline::EveOfflineModule mod;
    atlas::ecs::World world;
    atlas::net::NetContext net;
    atlas::net::ReplicationManager repl;
    atlas::asset::AssetRegistry assets;

    auto& rules = atlas::rules::ServerRules::Get();
    rules.Clear();

    auto ctx = MakeTestContext(world, net, repl, assets);
    mod.ConfigureServerRules(ctx);

    assert(rules.HasRule("miningYieldMultiplier"));
    assert(rules.HasRule("npcSpawnRate"));
    assert(rules.HasRule("marketUpdateInterval"));
    assert(rules.HasRule("pirateAggressionLevel"));
    assert(rules.HasRule("warpSpeedMultiplier"));

    std::cout << "[PASS] test_eveoffline_server_rules" << std::endl;
}

void test_eveoffline_lifecycle() {
    eveoffline::EveOfflineModule mod;
    atlas::ecs::World world;
    atlas::net::NetContext net;
    atlas::net::ReplicationManager repl;
    atlas::asset::AssetRegistry assets;
    auto ctx = MakeTestContext(world, net, repl, assets);

    assert(!mod.IsStarted());
    assert(mod.TickCount() == 0);

    mod.RegisterTypes(ctx);
    mod.OnStart(ctx);
    assert(mod.IsStarted());

    mod.OnTick(ctx, 0.033f);
    mod.OnTick(ctx, 0.033f);
    assert(mod.TickCount() == 2);

    mod.OnShutdown(ctx);
    assert(!mod.IsStarted());
    assert(mod.TickCount() == 0);

    std::cout << "[PASS] test_eveoffline_lifecycle" << std::endl;
}

void test_eveoffline_factory() {
    std::unique_ptr<atlas::module::IGameModule> mod(CreateGameModule());
    assert(mod != nullptr);
    auto desc = mod->Describe();
    assert(std::string(desc.name) == "EveOffline");

    std::cout << "[PASS] test_eveoffline_factory" << std::endl;
}
