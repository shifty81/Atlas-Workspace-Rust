#pragma once
#include "../../../engine/module/IGameModule.h"
#include "../../../modules/atlas_gameplay/FactionSystem.h"
#include "../../../modules/atlas_gameplay/CombatFramework.h"
#include "../../../modules/atlas_gameplay/EconomySystem.h"

namespace arena2d {

// ECS component type tags for replication
enum ComponentTag : uint32_t {
    Tag_Position    = 200,
    Tag_Health      = 201,
    Tag_SpriteInfo  = 202,
};

// ECS components registered by this module
struct Position {
    float x = 0.0f, y = 0.0f;
};

struct Health {
    float hp = 100.0f;
    float maxHp = 100.0f;
};

struct SpriteInfo {
    uint32_t spriteID = 0;
    uint32_t frameIndex = 0;
};

class Arena2DModule : public atlas::module::IGameModule {
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
    atlas::gameplay::CombatFramework& Combat() { return m_combat; }
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

} // namespace arena2d
