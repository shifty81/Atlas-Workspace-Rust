#include "systems/incursion_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

IncursionSystem::IncursionSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void IncursionSystem::updateComponent(ecs::Entity& entity,
    components::IncursionState& comp, float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;

    // Transition from Spawning → Active on first tick
    if (comp.lifecycle == components::IncursionState::Lifecycle::Spawning) {
        comp.lifecycle = components::IncursionState::Lifecycle::Active;
    }

    // Slow influence decay over time
    if (comp.lifecycle == components::IncursionState::Lifecycle::Active) {
        comp.influence -= comp.influence_decay_rate * delta_time;
        comp.influence = (std::max)(comp.influence, 0.0f);

        // Withdrawal check
        if (comp.influence <= 0.0f) {
            comp.lifecycle = components::IncursionState::Lifecycle::Withdrawn;
            comp.active = false;
        }
    }

    // Also tick wave-based Incursion component on same entity
    tickIncursion(entity);
}

// Helper to update the Incursion (wave-based) component state each tick
static void tickIncursion(ecs::Entity& entity) {
    using IS = components::Incursion::IncursionState;
    auto* inc = entity.getComponent<components::Incursion>();
    if (!inc) return;
    if (inc->state == IS::Defeated) return;

    // All waves defeated → Withdrawing
    if (inc->state == IS::Active &&
        inc->total_waves_defeated >= inc->max_waves) {
        inc->state = IS::Withdrawing;
    }
    // Influence zero → Defeated
    if (inc->influence <= 0.0f) {
        inc->state = IS::Defeated;
    }
}

bool IncursionSystem::initialize(const std::string& entity_id,
    const std::string& constellation_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::IncursionState>();
    comp->constellation_id = constellation_id;
    entity->addComponent(std::move(comp));
    return true;
}

bool IncursionSystem::spawnSite(const std::string& entity_id,
    const std::string& site_id, components::IncursionState::Tier tier) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (static_cast<int>(comp->sites.size()) >= comp->max_sites) return false;

    components::IncursionState::IncursionSite site;
    site.site_id = site_id;
    site.tier = tier;
    // Wave count scales with tier
    switch (tier) {
        case components::IncursionState::Tier::Vanguard:
            site.max_waves = 3;
            break;
        case components::IncursionState::Tier::Assault:
            site.max_waves = 5;
            break;
        case components::IncursionState::Tier::Headquarters:
            site.max_waves = 7;
            break;
    }
    comp->sites.push_back(site);
    return true;
}

bool IncursionSystem::completeSite(const std::string& entity_id,
    const std::string& site_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;

    for (auto& site : comp->sites) {
        if (site.site_id == site_id && !site.completed) {
            site.completed = true;
            comp->completed_sites++;

            // Count fleet members in this site for reward scaling
            int fleet_in_site = 0;
            for (const auto& fm : comp->fleet_members) {
                if (fm.site_id == site_id) fleet_in_site++;
            }
            int fleet_bonus = (std::max)(fleet_in_site, 1);

            int base_lp = components::IncursionState::baseLPForTier(site.tier);
            int payout = base_lp * fleet_bonus;
            site.lp_reward = payout;
            comp->total_lp_paid += static_cast<float>(payout);

            // Reduce influence based on tier
            float influence_reduction = 0.0f;
            switch (site.tier) {
                case components::IncursionState::Tier::Vanguard:
                    influence_reduction = 1.0f;
                    break;
                case components::IncursionState::Tier::Assault:
                    influence_reduction = 3.0f;
                    break;
                case components::IncursionState::Tier::Headquarters:
                    influence_reduction = 7.0f;
                    break;
            }
            comp->influence = (std::max)(comp->influence - influence_reduction, 0.0f);

            if (comp->influence <= 0.0f) {
                comp->lifecycle = components::IncursionState::Lifecycle::Withdrawn;
                comp->active = false;
            }
            return true;
        }
    }
    return false;
}

bool IncursionSystem::registerFleetMember(const std::string& entity_id,
    const std::string& site_id, const std::string& pilot_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;

    components::IncursionState::FleetMember fm;
    fm.pilot_id = pilot_id;
    fm.site_id = site_id;
    comp->fleet_members.push_back(fm);
    return true;
}

bool IncursionSystem::applyInfluenceDelta(const std::string& entity_id,
    float delta) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->influence = (std::min)((std::max)(comp->influence + delta, 0.0f), 100.0f);

    if (comp->influence <= 0.0f) {
        comp->lifecycle = components::IncursionState::Lifecycle::Withdrawn;
        comp->active = false;
    }
    return true;
}

int IncursionSystem::getSiteCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->sites.size()) : 0;
}

float IncursionSystem::getInfluence(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->influence : 0.0f;
}

bool IncursionSystem::isWithdrawn(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? (comp->lifecycle == components::IncursionState::Lifecycle::Withdrawn) : false;
}

int IncursionSystem::getCompletedSiteCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->completed_sites : 0;
}

float IncursionSystem::getTotalLPPaid(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_lp_paid : 0.0f;
}

int IncursionSystem::getFleetSize(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->fleet_members.size()) : 0;
}

// ── Extended wave-based incursion API ────────────────────────────────────

// Helper: get or null Incursion component
static components::Incursion* getIncursion(ecs::World* world,
                                            const std::string& entity_id) {
    auto* entity = world->getEntity(entity_id);
    return entity ? entity->getComponent<components::Incursion>() : nullptr;
}

bool IncursionSystem::initialize(const std::string& entity_id,
                                  const std::string& incursion_id,
                                  const std::string& system_id,
                                  int max_waves) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::Incursion>();
    comp->incursion_id = incursion_id;
    comp->system_id = system_id;
    comp->max_waves = max_waves;
    comp->influence = 1.0f;
    entity->addComponent(std::move(comp));
    return true;
}

bool IncursionSystem::addWave(const std::string& entity_id, int wave_number,
                               const std::string& ship_type, int count) {
    auto* inc = getIncursion(world_, entity_id);
    if (!inc) return false;
    if (static_cast<int>(inc->waves.size()) >= inc->max_waves) return false;
    for (const auto& w : inc->waves) {
        if (w.wave_number == wave_number) return false; // duplicate
    }
    components::Incursion::Wave wave;
    wave.wave_number = wave_number;
    wave.ship_type = ship_type;
    wave.count = count;
    inc->waves.push_back(wave);
    return true;
}

bool IncursionSystem::spawnWave(const std::string& entity_id, int wave_number) {
    auto* inc = getIncursion(world_, entity_id);
    if (!inc) return false;
    for (auto& w : inc->waves) {
        if (w.wave_number == wave_number) {
            if (w.spawned) return false; // already spawned
            w.spawned = true;
            if (inc->state == components::Incursion::IncursionState::Pending) {
                inc->state = components::Incursion::IncursionState::Active;
            }
            return true;
        }
    }
    return false; // wave not found
}

bool IncursionSystem::defeatWave(const std::string& entity_id, int wave_number) {
    auto* inc = getIncursion(world_, entity_id);
    if (!inc) return false;
    for (auto& w : inc->waves) {
        if (w.wave_number == wave_number) {
            if (w.defeated) return false; // already defeated
            w.defeated = true;
            inc->total_waves_defeated++;
            inc->influence = std::max(0.0f, inc->influence - 0.1f);
            return true;
        }
    }
    return false;
}

bool IncursionSystem::joinIncursion(const std::string& entity_id,
                                     const std::string& player_id) {
    auto* inc = getIncursion(world_, entity_id);
    if (!inc) return false;
    if (static_cast<int>(inc->participants.size()) >= inc->max_participants) return false;
    for (const auto& p : inc->participants) {
        if (p == player_id) return false; // duplicate
    }
    inc->participants.push_back(player_id);
    return true;
}

bool IncursionSystem::leaveIncursion(const std::string& entity_id,
                                      const std::string& player_id) {
    auto* inc = getIncursion(world_, entity_id);
    if (!inc) return false;
    auto it = std::find(inc->participants.begin(), inc->participants.end(), player_id);
    if (it == inc->participants.end()) return false;
    inc->participants.erase(it);
    return true;
}

bool IncursionSystem::contributeRewards(const std::string& entity_id, float amount) {
    auto* inc = getIncursion(world_, entity_id);
    if (!inc) return false;
    inc->reward_pool += amount;
    return true;
}

int IncursionSystem::getParticipantCount(const std::string& entity_id) const {
    auto* inc = getIncursion(world_, entity_id);
    return inc ? static_cast<int>(inc->participants.size()) : 0;
}

int IncursionSystem::getState(const std::string& entity_id) const {
    auto* inc = getIncursion(world_, entity_id);
    return inc ? static_cast<int>(inc->state) : 0;
}

float IncursionSystem::getRewardPool(const std::string& entity_id) const {
    auto* inc = getIncursion(world_, entity_id);
    return inc ? inc->reward_pool : 0.0f;
}

int IncursionSystem::getTotalWavesDefeated(const std::string& entity_id) const {
    auto* inc = getIncursion(world_, entity_id);
    return inc ? inc->total_waves_defeated : 0;
}

} // namespace systems
} // namespace atlas
