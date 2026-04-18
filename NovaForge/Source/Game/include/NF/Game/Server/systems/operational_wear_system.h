#ifndef NOVAFORGE_SYSTEMS_OPERATIONAL_WEAR_SYSTEM_H
#define NOVAFORGE_SYSTEMS_OPERATIONAL_WEAR_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/fleet_components.h"
#include <string>

namespace atlas {
namespace systems {

class OperationalWearSystem
    : public ecs::SingleComponentSystem<components::OperationalWearState> {
public:
    explicit OperationalWearSystem(ecs::World* world);
    ~OperationalWearSystem() override = default;

    std::string getName() const override { return "OperationalWearSystem"; }

    // --- Lifecycle ---
    bool initialize(const std::string& entity_id);

    // --- Wear mutation ---
    bool recordWear(const std::string& entity_id,
                    const std::string& event_id,
                    const std::string& event_type,
                    float amount);

    bool fieldRepair(const std::string& entity_id, float amount);
    bool dockRepair(const std::string& entity_id);

    // --- Configuration ---
    bool setDocked(const std::string& entity_id, bool docked);
    bool setShipId(const std::string& entity_id, const std::string& ship_id);
    bool setRotationThreshold(const std::string& entity_id, float secs);
    bool setPassiveWearRate(const std::string& entity_id, float rate);
    bool setRecoveryRate(const std::string& entity_id, float rate);
    bool setMaxEvents(const std::string& entity_id, int max);

    // --- Queries ---
    float       getWearLevel(const std::string& entity_id) const;
    float       getFuelInefficiency(const std::string& entity_id) const;
    float       getRepairDelayMult(const std::string& entity_id) const;
    float       getCrewStress(const std::string& entity_id) const;
    bool        isWorn(const std::string& entity_id) const;
    bool        isCritical(const std::string& entity_id) const;
    bool        hasHiddenPenalties(const std::string& entity_id) const;
    float       getDeploymentDuration(const std::string& entity_id) const;
    int         getTotalFieldRepairs(const std::string& entity_id) const;
    int         getTotalDockRepairs(const std::string& entity_id) const;
    std::string getShipId(const std::string& entity_id) const;
    float       getRotationThreshold(const std::string& entity_id) const;
    float       getPassiveWearRate(const std::string& entity_id) const;
    float       getRecoveryRate(const std::string& entity_id) const;
    int         getMaxEvents(const std::string& entity_id) const;
    int         getEventCount(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::OperationalWearState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_OPERATIONAL_WEAR_SYSTEM_H
