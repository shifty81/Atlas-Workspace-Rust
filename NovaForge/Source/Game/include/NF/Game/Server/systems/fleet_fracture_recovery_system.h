#ifndef NOVAFORGE_SYSTEMS_FLEET_FRACTURE_RECOVERY_SYSTEM_H
#define NOVAFORGE_SYSTEMS_FLEET_FRACTURE_RECOVERY_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/fleet_components.h"
#include <string>

namespace atlas {
namespace systems {

class FleetFractureRecoverySystem
    : public ecs::SingleComponentSystem<components::FleetFractureRecoveryState> {
public:
    explicit FleetFractureRecoverySystem(ecs::World* world);
    ~FleetFractureRecoverySystem() override = default;

    std::string getName() const override { return "FleetFractureRecoverySystem"; }

    bool initialize(const std::string& entity_id);

    bool recordFractureEvent(const std::string& entity_id,
                             const std::string& event_id,
                             const std::string& description,
                             int ships_lost,
                             int captains_departed,
                             float morale_delta,
                             float severity);
    bool applyRecovery(const std::string& entity_id, float amount);
    bool setRecoveryMomentum(const std::string& entity_id, float rate);
    bool setFragility(const std::string& entity_id, float fragility);
    bool addMilestone(const std::string& entity_id,
                      const std::string& milestone_id,
                      const std::string& description,
                      float required_value);
    bool progressMilestone(const std::string& entity_id,
                           const std::string& milestone_id,
                           float current_value);
    bool clearFractureLog(const std::string& entity_id);
    bool setFleetId(const std::string& entity_id, const std::string& fleet_id);
    bool setMaxLog(const std::string& entity_id, int max);
    bool setFractureThreshold(const std::string& entity_id, float value);
    bool setRecoveryThreshold(const std::string& entity_id, float value);

    components::FleetFractureRecoveryState::RecoveryPhase getPhase(const std::string& entity_id) const;
    float       getFractureScore(const std::string& entity_id) const;
    float       getRecoveryMomentum(const std::string& entity_id) const;
    float       getFragility(const std::string& entity_id) const;
    int         getFractureLogCount(const std::string& entity_id) const;
    int         getMilestoneCount(const std::string& entity_id) const;
    int         getMilestoneAchievedCount(const std::string& entity_id) const;
    bool        isMilestoneAchieved(const std::string& entity_id, const std::string& milestone_id) const;
    int         getTotalFractures(const std::string& entity_id) const;
    int         getTotalRecoveries(const std::string& entity_id) const;
    int         getTotalCaptainsLost(const std::string& entity_id) const;
    int         getTotalShipsLost(const std::string& entity_id) const;
    std::string getFleetId(const std::string& entity_id) const;
    bool        isFractured(const std::string& entity_id) const;
    bool        isRecovering(const std::string& entity_id) const;
    std::string getPhaseString(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::FleetFractureRecoveryState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_FLEET_FRACTURE_RECOVERY_SYSTEM_H
