#ifndef NOVAFORGE_SYSTEMS_TITAN_COUNTERMEASURE_SYSTEM_H
#define NOVAFORGE_SYSTEMS_TITAN_COUNTERMEASURE_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/fleet_components.h"
#include <string>

namespace atlas {
namespace systems {

// TitanCountermeasureSystem — Phase F (Pirate Titan Meta-Threat)
// Tracks player-side operations designed to slow the Pirate Titan's assembly.
// Each countermeasure operation contributes to titan_delay_score, representing
// how much the assembly has been set back. High pirate_awareness (from too many
// visible ops) triggers retaliation events and reduces op effectiveness.
// Pairs with FactionDoctrineSystem (pirate AI doctrine) and the background
// TitanAssemblySystem (progress tracking).
class TitanCountermeasureSystem
    : public ecs::SingleComponentSystem<components::TitanCountermeasureState> {
public:
    explicit TitanCountermeasureSystem(ecs::World* world);
    ~TitanCountermeasureSystem() override = default;

    std::string getName() const override { return "TitanCountermeasureSystem"; }

    bool initialize(const std::string& entity_id);

    // Op execution
    bool executeOp(const std::string& entity_id,
                   const std::string& op_id,
                   components::CountermeasureType type,
                   const std::string& target_node,
                   float              effectiveness);
    bool removeOp(const std::string& entity_id, const std::string& op_id);
    bool clearOps(const std::string& entity_id);

    // Retaliation / awareness events
    bool recordRetaliation(const std::string& entity_id);
    bool applyAwarenessBoost(const std::string& entity_id, float amount);

    // Configuration
    bool setMaxOperations(const std::string& entity_id, int max);
    bool setAwarenessDecayRate(const std::string& entity_id, float rate);
    bool setAwarenessThreshold(const std::string& entity_id, float threshold);
    bool setPlayerId(const std::string& entity_id, const std::string& player_id);

    // Queries
    int          getOperationCount(const std::string& entity_id) const;
    bool         hasOp(const std::string& entity_id,
                       const std::string& op_id) const;
    float        getTitanDelayScore(const std::string& entity_id) const;
    float        getPirateAwareness(const std::string& entity_id) const;
    bool         isAwarenessHigh(const std::string& entity_id) const;
    float        getOpEffectiveness(const std::string& entity_id,
                                    const std::string& op_id) const;
    float        getOpDelayContributed(const std::string& entity_id,
                                       const std::string& op_id) const;
    int          getTotalOpsExecuted(const std::string& entity_id) const;
    int          getTotalSabotageOps(const std::string& entity_id) const;
    int          getTotalIntelOps(const std::string& entity_id) const;
    int          getRetaliationEvents(const std::string& entity_id) const;
    int          getCountByType(const std::string& entity_id,
                                components::CountermeasureType type) const;
    std::string  getPlayerId(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::TitanCountermeasureState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_TITAN_COUNTERMEASURE_SYSTEM_H
