#include "EveOfflineModule.h"
#include "../../../engine/ecs/ECS.h"
#include "../../../engine/net/Replication.h"
#include "../../../engine/rules/ServerRules.h"

namespace eveoffline {

atlas::module::GameModuleDesc EveOfflineModule::Describe() const {
    return {"EveOffline", 1};
}

void EveOfflineModule::RegisterTypes(atlas::module::GameModuleContext& ctx) {
    // Register factions from the EveOffline universe
    auto empire = m_factions.RegisterFaction("Amarr Empire", "AE");
    auto state = m_factions.RegisterFaction("Caldari State", "CS");
    auto federation = m_factions.RegisterFaction("Gallente Federation", "GF");
    auto republic = m_factions.RegisterFaction("Minmatar Republic", "MR");
    auto pirates = m_factions.RegisterFaction("Pirate Factions", "PF");

    // Set initial faction relations
    m_factions.SetRelation(empire, state, atlas::gameplay::FactionRelation::Allied);
    m_factions.SetRelation(federation, republic, atlas::gameplay::FactionRelation::Allied);
    m_factions.SetRelation(empire, republic, atlas::gameplay::FactionRelation::Hostile);
    m_factions.SetRelation(state, federation, atlas::gameplay::FactionRelation::Hostile);
    m_factions.SetRelation(pirates, empire, atlas::gameplay::FactionRelation::Hostile);
    m_factions.SetRelation(pirates, state, atlas::gameplay::FactionRelation::Hostile);
    m_factions.SetRelation(pirates, federation, atlas::gameplay::FactionRelation::Hostile);
    m_factions.SetRelation(pirates, republic, atlas::gameplay::FactionRelation::Hostile);

    // Register economy resources
    m_economy.RegisterResource("ISK", 0.0);
    m_economy.RegisterResource("Tritanium", 0.0);
    m_economy.RegisterResource("Pyerite", 0.0);
    m_economy.RegisterResource("Mexallon", 0.0);
    m_economy.RegisterResource("Isogen", 0.0);

    (void)ctx;
}

void EveOfflineModule::ConfigureReplication(atlas::module::GameModuleContext& ctx) {
    // Ships replicate position every tick to all clients
    atlas::net::ReplicationRule posRule;
    posRule.typeTag = Tag_ShipPosition;
    posRule.componentName = "ShipPosition";
    posRule.frequency = atlas::net::ReplicateFrequency::EveryTick;
    posRule.direction = atlas::net::ReplicateDirection::ServerToClient;
    posRule.reliable = false;
    posRule.priority = 200;
    ctx.replication.AddRule(posRule);

    // Health replicates on change
    atlas::net::ReplicationRule healthRule;
    healthRule.typeTag = Tag_ShipHealth;
    healthRule.componentName = "ShipHealth";
    healthRule.frequency = atlas::net::ReplicateFrequency::OnChange;
    healthRule.direction = atlas::net::ReplicateDirection::ServerToClient;
    healthRule.reliable = true;
    healthRule.priority = 180;
    ctx.replication.AddRule(healthRule);

    // Inventory replicates on change, reliable
    atlas::net::ReplicationRule invRule;
    invRule.typeTag = Tag_Inventory;
    invRule.componentName = "Inventory";
    invRule.frequency = atlas::net::ReplicateFrequency::OnChange;
    invRule.direction = atlas::net::ReplicateDirection::ServerToClient;
    invRule.reliable = true;
    invRule.priority = 100;
    ctx.replication.AddRule(invRule);
}

void EveOfflineModule::ConfigureServerRules(atlas::module::GameModuleContext& ctx) {
    ctx.rules.RegisterRule({"miningYieldMultiplier", 1.0f, 1.0f, 0.1f, 5.0f, true, false});
    ctx.rules.RegisterRule({"npcSpawnRate", 1.0f, 1.0f, 0.0f, 3.0f, true, false});
    ctx.rules.RegisterRule({"marketUpdateInterval", 300.0f, 1.0f, 0.5f, 10.0f, true, false});
    ctx.rules.RegisterRule({"pirateAggressionLevel", 1.0f, 1.0f, 0.0f, 5.0f, true, false});
    ctx.rules.RegisterRule({"warpSpeedMultiplier", 1.0f, 1.0f, 0.5f, 3.0f, true, true});
}

void EveOfflineModule::OnStart(atlas::module::GameModuleContext& ctx) {
    (void)ctx;
    m_started = true;
}

void EveOfflineModule::OnTick(atlas::module::GameModuleContext& ctx, float dt) {
    (void)ctx;
    (void)dt;
    ++m_tickCount;
}

void EveOfflineModule::OnShutdown(atlas::module::GameModuleContext& ctx) {
    (void)ctx;
    m_factions.Clear();
    m_combat.Clear();
    m_economy.Clear();
    m_started = false;
    m_tickCount = 0;
}

} // namespace eveoffline

// Factory function for dynamic loading
extern "C" atlas::module::IGameModule* CreateGameModule() {
    return new eveoffline::EveOfflineModule();
}
