#include "Arena2DModule.h"
#include "../../../engine/ecs/ECS.h"
#include "../../../engine/net/Replication.h"
#include "../../../engine/rules/ServerRules.h"

namespace arena2d {

atlas::module::GameModuleDesc Arena2DModule::Describe() const {
    return {"Arena2D", 1};
}

void Arena2DModule::RegisterTypes(atlas::module::GameModuleContext& ctx) {
    // Register factions for the arena game
    auto heroes = m_factions.RegisterFaction("Heroes", "H");
    auto monsters = m_factions.RegisterFaction("Monsters", "M");
    auto neutrals = m_factions.RegisterFaction("Neutrals", "N");

    // Heroes vs monsters, neutrals are neutral to both
    m_factions.SetRelation(heroes, monsters, atlas::gameplay::FactionRelation::Hostile);
    m_factions.SetRelation(heroes, neutrals, atlas::gameplay::FactionRelation::Friendly);
    m_factions.SetRelation(monsters, neutrals, atlas::gameplay::FactionRelation::Neutral);

    // Register economy resources
    m_economy.RegisterResource("Gold", 0.0);
    m_economy.RegisterResource("XP", 0.0);

    (void)ctx;
}

void Arena2DModule::ConfigureReplication(atlas::module::GameModuleContext& ctx) {
    // Position replicates every tick to all clients (movement is frequent)
    atlas::net::ReplicationRule posRule;
    posRule.typeTag = Tag_Position;
    posRule.componentName = "Position";
    posRule.frequency = atlas::net::ReplicateFrequency::EveryTick;
    posRule.direction = atlas::net::ReplicateDirection::ServerToClient;
    posRule.reliable = false;
    posRule.priority = 200;
    ctx.replication.AddRule(posRule);

    // Health replicates on change
    atlas::net::ReplicationRule healthRule;
    healthRule.typeTag = Tag_Health;
    healthRule.componentName = "Health";
    healthRule.frequency = atlas::net::ReplicateFrequency::OnChange;
    healthRule.direction = atlas::net::ReplicateDirection::ServerToClient;
    healthRule.reliable = true;
    healthRule.priority = 180;
    ctx.replication.AddRule(healthRule);

    // Sprite info replicates on change
    atlas::net::ReplicationRule spriteRule;
    spriteRule.typeTag = Tag_SpriteInfo;
    spriteRule.componentName = "SpriteInfo";
    spriteRule.frequency = atlas::net::ReplicateFrequency::OnChange;
    spriteRule.direction = atlas::net::ReplicateDirection::ServerToClient;
    spriteRule.reliable = true;
    spriteRule.priority = 100;
    ctx.replication.AddRule(spriteRule);
}

void Arena2DModule::ConfigureServerRules(atlas::module::GameModuleContext& ctx) {
    ctx.rules.RegisterRule({"enemySpawnRate", 1.0f, 1.0f, 0.1f, 5.0f, true, false});
    ctx.rules.RegisterRule({"playerDamageMultiplier", 1.0f, 1.0f, 0.5f, 3.0f, true, false});
    ctx.rules.RegisterRule({"xpMultiplier", 1.0f, 1.0f, 0.5f, 5.0f, true, false});
}

void Arena2DModule::OnStart(atlas::module::GameModuleContext& ctx) {
    (void)ctx;
    m_started = true;
}

void Arena2DModule::OnTick(atlas::module::GameModuleContext& ctx, float dt) {
    (void)ctx;
    (void)dt;
    ++m_tickCount;
}

void Arena2DModule::OnShutdown(atlas::module::GameModuleContext& ctx) {
    (void)ctx;
    m_factions.Clear();
    m_combat.Clear();
    m_economy.Clear();
    m_started = false;
    m_tickCount = 0;
}

} // namespace arena2d

// Factory function for dynamic loading
extern "C" atlas::module::IGameModule* CreateArena2DModule() {
    return new arena2d::Arena2DModule();
}
