#ifndef NOVAFORGE_SYSTEMS_FLEET_NORM_SYSTEM_H
#define NOVAFORGE_SYSTEMS_FLEET_NORM_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/fleet_components.h"
#include <string>

namespace atlas {
namespace systems {

class FleetNormSystem
    : public ecs::SingleComponentSystem<components::FleetNormState> {
public:
    explicit FleetNormSystem(ecs::World* world);
    ~FleetNormSystem() override = default;

    std::string getName() const override { return "FleetNormSystem"; }

    // --- Lifecycle ---
    bool initialize(const std::string& entity_id);

    // --- Norm management ---
    bool addNorm(const std::string& entity_id,
                 const std::string& norm_id,
                 const std::string& name,
                 const std::string& trigger_action);
    bool removeNorm(const std::string& entity_id,
                    const std::string& norm_id);
    bool clearNorms(const std::string& entity_id);

    // --- Action recording ---
    bool recordAction(const std::string& entity_id,
                      const std::string& trigger_action);

    // --- Norm activation ---
    bool activateNorm(const std::string& entity_id,
                      const std::string& norm_id);
    bool deactivateNorm(const std::string& entity_id,
                        const std::string& norm_id);

    // --- Configuration ---
    bool setActivationThreshold(const std::string& entity_id, int threshold);
    bool setMaxNorms(const std::string& entity_id, int max_norms);
    bool setFleetId(const std::string& entity_id, const std::string& fleet_id);

    // --- Queries ---
    int   getNormCount(const std::string& entity_id) const;
    int   getActiveNormCount(const std::string& entity_id) const;
    bool  isNormActive(const std::string& entity_id,
                       const std::string& norm_id) const;
    float getNormStrength(const std::string& entity_id,
                          const std::string& norm_id) const;
    bool  hasNorm(const std::string& entity_id,
                  const std::string& norm_id) const;
    int   getTotalNormsFormed(const std::string& entity_id) const;
    int   getCountByTrigger(const std::string& entity_id,
                            const std::string& trigger_action) const;
    int   getActivationThreshold(const std::string& entity_id) const;
    std::string getFleetId(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::FleetNormState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_FLEET_NORM_SYSTEM_H
