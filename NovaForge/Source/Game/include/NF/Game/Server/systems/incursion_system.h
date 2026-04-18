#ifndef NOVAFORGE_SYSTEMS_INCURSION_SYSTEM_H
#define NOVAFORGE_SYSTEMS_INCURSION_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/combat_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Dynamic incursion spawning and fleet coordination system
 *
 * Manages constellation-wide incursion encounters.  Sites are spawned at
 * different difficulty tiers (Vanguard, Assault, Headquarters) and
 * completing them reduces the incursion's influence level.  When influence
 * reaches zero the incursion withdraws.  Fleet coordination is rewarded
 * with loyalty point payouts scaled by fleet size and site tier.
 */
class IncursionSystem : public ecs::SingleComponentSystem<components::IncursionState> {
public:
    explicit IncursionSystem(ecs::World* world);
    ~IncursionSystem() override = default;

    std::string getName() const override { return "IncursionSystem"; }

    void update(float delta_time) override;

    // --- public API: mutators ---
    bool initialize(const std::string& entity_id,
                    const std::string& constellation_id = "");
    // Extended initialize: incursion_id, system_id, max_waves
    bool initialize(const std::string& entity_id,
                    const std::string& incursion_id,
                    const std::string& system_id,
                    int max_waves);
    bool spawnSite(const std::string& entity_id, const std::string& site_id,
                   components::IncursionState::Tier tier =
                       components::IncursionState::Tier::Vanguard);
    bool completeSite(const std::string& entity_id, const std::string& site_id);
    bool registerFleetMember(const std::string& entity_id,
                             const std::string& site_id,
                             const std::string& pilot_id);
    bool applyInfluenceDelta(const std::string& entity_id, float delta);

    // --- Wave-based incursion API (operates on Incursion component) ---
    bool  addWave(const std::string& entity_id, int wave_number,
                  const std::string& ship_type, int count);
    bool  spawnWave(const std::string& entity_id, int wave_number);
    bool  defeatWave(const std::string& entity_id, int wave_number);
    bool  joinIncursion(const std::string& entity_id, const std::string& player_id);
    bool  leaveIncursion(const std::string& entity_id, const std::string& player_id);
    bool  contributeRewards(const std::string& entity_id, float amount);
    int   getParticipantCount(const std::string& entity_id) const;
    int   getState(const std::string& entity_id) const;
    float getRewardPool(const std::string& entity_id) const;
    int   getTotalWavesDefeated(const std::string& entity_id) const;

    // --- public API: queries ---
    int   getSiteCount(const std::string& entity_id) const;
    float getInfluence(const std::string& entity_id) const;
    bool  isWithdrawn(const std::string& entity_id) const;
    int   getCompletedSiteCount(const std::string& entity_id) const;
    float getTotalLPPaid(const std::string& entity_id) const;
    int   getFleetSize(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::IncursionState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_INCURSION_SYSTEM_H
