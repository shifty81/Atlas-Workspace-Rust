#ifndef NOVAFORGE_SYSTEMS_FLEET_CARGO_AGGREGATOR_SYSTEM_H
#define NOVAFORGE_SYSTEMS_FLEET_CARGO_AGGREGATOR_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/fleet_components.h"
#include <string>

namespace atlas {
namespace systems {

class FleetCargoAggregatorSystem
    : public ecs::SingleComponentSystem<components::FleetCargoAggregatorState> {
public:
    explicit FleetCargoAggregatorSystem(ecs::World* world);
    ~FleetCargoAggregatorSystem() override = default;

    std::string getName() const override { return "FleetCargoAggregatorSystem"; }

    bool initialize(const std::string& entity_id);

    // Contribution management
    bool addContribution(const std::string& entity_id,
                         const std::string& ship_id,
                         const std::string& ship_name,
                         components::FleetCargoAggregatorState::ResourceType resource_type,
                         float quantity,
                         float capacity);
    bool removeContribution(const std::string& entity_id, const std::string& ship_id);
    bool clearContributions(const std::string& entity_id);
    bool updateContributionQuantity(const std::string& entity_id,
                                     const std::string& ship_id,
                                     float new_quantity);

    // Pool management
    bool addPool(const std::string& entity_id,
                 const std::string& pool_id,
                 components::FleetCargoAggregatorState::ResourceType resource_type,
                 float capacity);
    bool removePool(const std::string& entity_id, const std::string& pool_id);
    bool clearPools(const std::string& entity_id);

    // Transfer between ships (contribution to pool)
    bool transferToPool(const std::string& entity_id,
                        const std::string& ship_id,
                        const std::string& pool_id,
                        float amount);

    // Configuration
    bool setFleetId(const std::string& entity_id, const std::string& fleet_id);
    bool setMaxContributions(const std::string& entity_id, int max);
    bool setMaxPools(const std::string& entity_id, int max);

    // Queries
    int         getContributionCount(const std::string& entity_id) const;
    int         getPoolCount(const std::string& entity_id) const;
    bool        hasContribution(const std::string& entity_id, const std::string& ship_id) const;
    bool        hasPool(const std::string& entity_id, const std::string& pool_id) const;
    float       getContributionQuantity(const std::string& entity_id, const std::string& ship_id) const;
    float       getContributionCapacity(const std::string& entity_id, const std::string& ship_id) const;
    float       getPoolQuantity(const std::string& entity_id, const std::string& pool_id) const;
    float       getPoolCapacity(const std::string& entity_id, const std::string& pool_id) const;
    float       getTotalFleetQuantity(const std::string& entity_id) const;
    float       getTotalFleetCapacity(const std::string& entity_id) const;
    float       getQuantityByType(const std::string& entity_id,
                                  components::FleetCargoAggregatorState::ResourceType type) const;
    int         getCountByType(const std::string& entity_id,
                               components::FleetCargoAggregatorState::ResourceType type) const;
    std::string getFleetId(const std::string& entity_id) const;
    int         getTotalTransfers(const std::string& entity_id) const;
    float       getTotalQuantityTransferred(const std::string& entity_id) const;
    int         getTotalContributionsAdded(const std::string& entity_id) const;
    int         getMaxContributions(const std::string& entity_id) const;
    int         getMaxPools(const std::string& entity_id) const;
    std::string getShipName(const std::string& entity_id, const std::string& ship_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::FleetCargoAggregatorState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_FLEET_CARGO_AGGREGATOR_SYSTEM_H
