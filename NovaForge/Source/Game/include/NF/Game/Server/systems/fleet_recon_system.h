#ifndef NOVAFORGE_SYSTEMS_FLEET_RECON_SYSTEM_H
#define NOVAFORGE_SYSTEMS_FLEET_RECON_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/fleet_components.h"
#include <string>

namespace atlas {
namespace systems {

// FleetReconSystem — Phase C/E (Fleet-as-Civilization / Living Galaxy)
// Fleet scouting operations dispatched to adjacent systems. Each recon mission
// has a duration, a scout identifier, and returns an intel payload (threat level,
// ship count, anomaly count). Missions auto-transition through states per tick:
// Deployed → Returning (at duration) → Intel_Ready (when recalled or arrived).
// Scouts that exceed scout_loss_timeout are marked Scout_Lost.
class FleetReconSystem
    : public ecs::SingleComponentSystem<components::FleetReconState> {
public:
    explicit FleetReconSystem(ecs::World* world);
    ~FleetReconSystem() override = default;

    std::string getName() const override { return "FleetReconSystem"; }

    bool initialize(const std::string& entity_id);

    // Mission lifecycle
    bool deployScout(const std::string& entity_id,
                     const std::string& mission_id,
                     const std::string& target_system,
                     const std::string& scout_id,
                     float              duration);
    bool recallScout(const std::string& entity_id,
                     const std::string& mission_id);
    bool setMissionIntel(const std::string& entity_id,
                         const std::string& mission_id,
                         float              threat,
                         int                ships,
                         int                anomalies);
    bool consumeIntel(const std::string& entity_id,
                      const std::string& mission_id);
    bool removeMission(const std::string& entity_id,
                       const std::string& mission_id);
    bool clearMissions(const std::string& entity_id);

    // Configuration
    bool setMaxMissions(const std::string& entity_id, int max);
    bool setScoutLossTimeout(const std::string& entity_id, float timeout);
    bool setFleetId(const std::string& entity_id, const std::string& fleet_id);

    // Queries
    int          getMissionCount(const std::string& entity_id) const;
    bool         hasMission(const std::string& entity_id,
                            const std::string& mission_id) const;
    components::ReconStatus getMissionStatus(const std::string& entity_id,
                                             const std::string& mission_id) const;
    float        getMissionElapsed(const std::string& entity_id,
                                   const std::string& mission_id) const;
    float        getMissionDuration(const std::string& entity_id,
                                    const std::string& mission_id) const;
    float        getIntelThreat(const std::string& entity_id,
                                const std::string& mission_id) const;
    int          getIntelShips(const std::string& entity_id,
                               const std::string& mission_id) const;
    int          getIntelAnomalies(const std::string& entity_id,
                                   const std::string& mission_id) const;
    bool         isIntelReady(const std::string& entity_id,
                              const std::string& mission_id) const;
    int          getDeployedCount(const std::string& entity_id) const;
    int          getTotalMissionsSent(const std::string& entity_id) const;
    int          getTotalMissionsReturned(const std::string& entity_id) const;
    int          getTotalScoutsLost(const std::string& entity_id) const;
    std::string  getFleetId(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::FleetReconState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_FLEET_RECON_SYSTEM_H
