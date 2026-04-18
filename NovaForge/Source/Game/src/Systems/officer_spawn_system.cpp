#include "systems/officer_spawn_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

OfficerSpawnSystem::OfficerSpawnSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void OfficerSpawnSystem::updateComponent(
        ecs::Entity& /*entity*/,
        components::OfficerSpawnState& comp,
        float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;
    comp.time_since_last_spawn += delta_time;

    // Advance time_alive for active officers; auto-despawn after 1 hour
    for (auto& off : comp.officers) {
        if (off.status == components::OfficerSpawnState::SpawnStatus::Active) {
            off.time_alive += delta_time;
            if (off.time_alive > 3600.0f) {
                off.status = components::OfficerSpawnState::SpawnStatus::Despawned;
            }
        }
    }
}

bool OfficerSpawnSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::OfficerSpawnState>();
    entity->addComponent(std::move(comp));
    return true;
}

static float healthForRank(components::OfficerSpawnState::OfficerRank rank,
                           float difficulty) {
    float base = 500.0f;
    switch (rank) {
        case components::OfficerSpawnState::OfficerRank::Lieutenant: base = 500.0f;   break;
        case components::OfficerSpawnState::OfficerRank::Commander:  base = 1000.0f;  break;
        case components::OfficerSpawnState::OfficerRank::Captain:    base = 2000.0f;  break;
        case components::OfficerSpawnState::OfficerRank::Admiral:    base = 5000.0f;  break;
        case components::OfficerSpawnState::OfficerRank::Overlord:   base = 10000.0f; break;
    }
    return base * difficulty;
}

bool OfficerSpawnSystem::spawnOfficer(
        const std::string& entity_id,
        const std::string& officer_id,
        components::OfficerSpawnState::OfficerRank rank,
        components::OfficerSpawnState::OfficerFaction faction,
        const std::string& loot_table_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (officer_id.empty()) return false;
    if (static_cast<int>(comp->officers.size()) >= comp->max_officers)
        return false;
    for (const auto& off : comp->officers)
        if (off.officer_id == officer_id) return false;

    components::OfficerSpawnState::OfficerEntry entry;
    entry.officer_id       = officer_id;
    entry.rank             = rank;
    entry.faction          = faction;
    entry.status           = components::OfficerSpawnState::SpawnStatus::Active;
    entry.loot_table_id    = loot_table_id;
    entry.bounty_multiplier = 2.0f;
    float hp = healthForRank(rank, comp->difficulty_modifier);
    entry.health           = hp;
    entry.max_health       = hp;
    entry.spawn_time       = comp->elapsed;
    entry.time_alive       = 0.0f;
    comp->officers.push_back(entry);

    comp->time_since_last_spawn = 0.0f;
    ++comp->total_officers_spawned;
    return true;
}

bool OfficerSpawnSystem::defeatOfficer(const std::string& entity_id,
                                        const std::string& officer_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto& off : comp->officers) {
        if (off.officer_id == officer_id) {
            if (off.status != components::OfficerSpawnState::SpawnStatus::Active)
                return false;
            off.status = components::OfficerSpawnState::SpawnStatus::Defeated;
            ++comp->total_officers_defeated;
            return true;
        }
    }
    return false;
}

bool OfficerSpawnSystem::despawnOfficer(const std::string& entity_id,
                                         const std::string& officer_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto& off : comp->officers) {
        if (off.officer_id == officer_id) {
            if (off.status != components::OfficerSpawnState::SpawnStatus::Active)
                return false;
            off.status = components::OfficerSpawnState::SpawnStatus::Despawned;
            return true;
        }
    }
    return false;
}

bool OfficerSpawnSystem::removeOfficer(const std::string& entity_id,
                                        const std::string& officer_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    auto it = std::find_if(comp->officers.begin(), comp->officers.end(),
        [&](const auto& off) { return off.officer_id == officer_id; });
    if (it == comp->officers.end()) return false;
    comp->officers.erase(it);
    return true;
}

bool OfficerSpawnSystem::clearOfficers(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->officers.clear();
    return true;
}

bool OfficerSpawnSystem::setSectorId(const std::string& entity_id,
                                      const std::string& sector_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (sector_id.empty()) return false;
    comp->sector_id = sector_id;
    return true;
}

bool OfficerSpawnSystem::setSpawnInterval(const std::string& entity_id,
                                           float interval) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (interval <= 0.0f) return false;
    comp->spawn_interval = interval;
    return true;
}

bool OfficerSpawnSystem::setMaxOfficers(const std::string& entity_id,
                                         int max) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (max < 1) return false;
    comp->max_officers = max;
    return true;
}

bool OfficerSpawnSystem::setBaseBounty(const std::string& entity_id,
                                        float bounty) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (bounty <= 0.0f) return false;
    comp->base_bounty = bounty;
    return true;
}

bool OfficerSpawnSystem::setDifficultyModifier(const std::string& entity_id,
                                                float modifier) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (modifier <= 0.0f) return false;
    comp->difficulty_modifier = modifier;
    return true;
}

int OfficerSpawnSystem::getOfficerCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return static_cast<int>(comp->officers.size());
}

bool OfficerSpawnSystem::hasOfficer(const std::string& entity_id,
                                     const std::string& officer_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& off : comp->officers)
        if (off.officer_id == officer_id) return true;
    return false;
}

float OfficerSpawnSystem::getOfficerBounty(const std::string& entity_id,
                                            const std::string& officer_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    for (const auto& off : comp->officers)
        if (off.officer_id == officer_id)
            return off.bounty_multiplier * comp->base_bounty * comp->difficulty_modifier;
    return 0.0f;
}

bool OfficerSpawnSystem::isOfficerActive(const std::string& entity_id,
                                          const std::string& officer_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& off : comp->officers)
        if (off.officer_id == officer_id)
            return off.status == components::OfficerSpawnState::SpawnStatus::Active;
    return false;
}

std::string OfficerSpawnSystem::getSectorId(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return "";
    return comp->sector_id;
}

int OfficerSpawnSystem::getTotalSpawned(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return comp->total_officers_spawned;
}

int OfficerSpawnSystem::getTotalDefeated(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return comp->total_officers_defeated;
}

int OfficerSpawnSystem::getCountByRank(
        const std::string& entity_id,
        components::OfficerSpawnState::OfficerRank rank) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    int count = 0;
    for (const auto& off : comp->officers)
        if (off.rank == rank) ++count;
    return count;
}

int OfficerSpawnSystem::getCountByFaction(
        const std::string& entity_id,
        components::OfficerSpawnState::OfficerFaction faction) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    int count = 0;
    for (const auto& off : comp->officers)
        if (off.faction == faction) ++count;
    return count;
}

float OfficerSpawnSystem::getBaseBounty(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    return comp->base_bounty;
}

float OfficerSpawnSystem::getDifficultyModifier(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    return comp->difficulty_modifier;
}

float OfficerSpawnSystem::getSpawnInterval(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    return comp->spawn_interval;
}

} // namespace systems
} // namespace atlas
