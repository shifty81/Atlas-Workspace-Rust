#ifndef NOVAFORGE_SYSTEMS_OFFICER_SPAWN_SYSTEM_H
#define NOVAFORGE_SYSTEMS_OFFICER_SPAWN_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/combat_components.h"
#include <string>

namespace atlas {
namespace systems {

class OfficerSpawnSystem
    : public ecs::SingleComponentSystem<components::OfficerSpawnState> {
public:
    explicit OfficerSpawnSystem(ecs::World* world);
    ~OfficerSpawnSystem() override = default;

    std::string getName() const override { return "OfficerSpawnSystem"; }

    // --- Lifecycle ---
    bool initialize(const std::string& entity_id);

    // --- Officer management ---
    bool spawnOfficer(const std::string& entity_id,
                      const std::string& officer_id,
                      components::OfficerSpawnState::OfficerRank rank,
                      components::OfficerSpawnState::OfficerFaction faction,
                      const std::string& loot_table_id);
    bool defeatOfficer(const std::string& entity_id,
                       const std::string& officer_id);
    bool despawnOfficer(const std::string& entity_id,
                        const std::string& officer_id);
    bool removeOfficer(const std::string& entity_id,
                       const std::string& officer_id);
    bool clearOfficers(const std::string& entity_id);

    // --- Configuration ---
    bool setSectorId(const std::string& entity_id,
                     const std::string& sector_id);
    bool setSpawnInterval(const std::string& entity_id, float interval);
    bool setMaxOfficers(const std::string& entity_id, int max);
    bool setBaseBounty(const std::string& entity_id, float bounty);
    bool setDifficultyModifier(const std::string& entity_id, float modifier);

    // --- Queries ---
    int   getOfficerCount(const std::string& entity_id) const;
    bool  hasOfficer(const std::string& entity_id,
                     const std::string& officer_id) const;
    float getOfficerBounty(const std::string& entity_id,
                           const std::string& officer_id) const;
    bool  isOfficerActive(const std::string& entity_id,
                          const std::string& officer_id) const;
    std::string getSectorId(const std::string& entity_id) const;
    int   getTotalSpawned(const std::string& entity_id) const;
    int   getTotalDefeated(const std::string& entity_id) const;
    int   getCountByRank(
              const std::string& entity_id,
              components::OfficerSpawnState::OfficerRank rank) const;
    int   getCountByFaction(
              const std::string& entity_id,
              components::OfficerSpawnState::OfficerFaction faction) const;
    float getBaseBounty(const std::string& entity_id) const;
    float getDifficultyModifier(const std::string& entity_id) const;
    float getSpawnInterval(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::OfficerSpawnState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_OFFICER_SPAWN_SYSTEM_H
