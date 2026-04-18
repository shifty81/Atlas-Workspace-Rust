#ifndef NOVAFORGE_SYSTEMS_FLEET_DEPARTURE_SYSTEM_H
#define NOVAFORGE_SYSTEMS_FLEET_DEPARTURE_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/fleet_components.h"
#include <string>

namespace atlas {
namespace systems {

// FleetDepartureSystem — Phase B (Fleet Personality & Social Systems)
// Tracks a captain's departure/transfer lifecycle.
// Stage progression: Stable → Arguing → Requesting → Transferring → Departed.
// Risk is computed from morale, losing_streak, and consecutive_near_deaths.
// Per-tick: advances time_in_stage; auto-triggers Arguing when risk ≥ threshold.
class FleetDepartureSystem
    : public ecs::SingleComponentSystem<components::FleetDepartureState> {
public:
    explicit FleetDepartureSystem(ecs::World* world);
    ~FleetDepartureSystem() override = default;

    std::string getName() const override { return "FleetDepartureSystem"; }

    bool initialize(const std::string& entity_id);

    // Event mutators
    bool recordNearDeath(const std::string& entity_id);
    bool recordLoss(const std::string& entity_id);
    bool recordWin(const std::string& entity_id);
    bool setMorale(const std::string& entity_id, float morale);

    // Stage progression
    bool requestTransfer(const std::string& entity_id, const std::string& reason);
    bool acceptTransfer(const std::string& entity_id);
    bool resolveConflict(const std::string& entity_id);

    // Configuration
    bool resetStreak(const std::string& entity_id);
    bool setDepartureThreshold(const std::string& entity_id, float threshold);
    bool setCaptainId(const std::string& entity_id, const std::string& captain_id);
    bool setFleetId(const std::string& entity_id, const std::string& fleet_id);

    // Queries
    components::FleetDepartureState::DepartureStage
                getStage(const std::string& entity_id) const;
    float       getDepartureRisk(const std::string& entity_id) const;
    float       getMorale(const std::string& entity_id) const;
    int         getLosingStreak(const std::string& entity_id) const;
    int         getConsecutiveNearDeaths(const std::string& entity_id) const;
    bool        hasDepartureRequest(const std::string& entity_id) const;
    std::string getDepartureReason(const std::string& entity_id) const;
    float       getTimeInStage(const std::string& entity_id) const;
    int         getTotalDepartures(const std::string& entity_id) const;
    int         getTotalTransfers(const std::string& entity_id) const;
    bool        isDepartureRisk(const std::string& entity_id) const;
    bool        isGone(const std::string& entity_id) const;
    std::string getStageString(const std::string& entity_id) const;
    std::string getCaptainId(const std::string& entity_id) const;
    std::string getFleetId(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::FleetDepartureState& comp,
                         float delta_time) override;

private:
    void recomputeRisk(components::FleetDepartureState& comp) const;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_FLEET_DEPARTURE_SYSTEM_H
