#pragma once
#include "../../../engine/module/IGameModule.h"
#include "../../../modules/atlas_gameplay/FactionSystem.h"
#include "../../../modules/atlas_gameplay/CombatFramework.h"
#include "../../../modules/atlas_gameplay/EconomySystem.h"

namespace eveoffline {

// ECS component type tags for replication
enum ComponentTag : uint32_t {
    Tag_ShipPosition = 100,
    Tag_ShipHealth   = 101,
    Tag_Inventory    = 102,
};

// ECS components registered by this module
struct ShipPosition {
    float x = 0.0f, y = 0.0f, z = 0.0f;
};

struct ShipHealth {
    float hull = 100.0f;
    float shield = 100.0f;
    float armor = 50.0f;
};

struct Inventory {
    uint32_t slotCount = 0;
};

class EveOfflineModule : public atlas::module::IGameModule {
public:
    atlas::module::GameModuleDesc Describe() const override;
    void RegisterTypes(atlas::module::GameModuleContext& ctx) override;
    void ConfigureReplication(atlas::module::GameModuleContext& ctx) override;
    void ConfigureServerRules(atlas::module::GameModuleContext& ctx) override;
    void OnStart(atlas::module::GameModuleContext& ctx) override;
    void OnTick(atlas::module::GameModuleContext& ctx, float dt) override;
    void OnShutdown(atlas::module::GameModuleContext& ctx) override;

    // Accessors for testing
    atlas::gameplay::FactionSystem& Factions() { return m_factions; }
    atlas::gameplay::EconomySystem& Economy() { return m_economy; }
    bool IsStarted() const { return m_started; }
    uint32_t TickCount() const { return m_tickCount; }

private:
    atlas::gameplay::FactionSystem m_factions;
    atlas::gameplay::CombatFramework m_combat;
    atlas::gameplay::EconomySystem m_economy;
    bool m_started = false;
    uint32_t m_tickCount = 0;
};

} // namespace eveoffline
