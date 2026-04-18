#ifndef NOVAFORGE_SYSTEMS_FLEET_ENGAGEMENT_RULES_SYSTEM_H
#define NOVAFORGE_SYSTEMS_FLEET_ENGAGEMENT_RULES_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/fleet_components.h"
#include <string>

namespace atlas {
namespace systems {

class FleetEngagementRulesSystem
    : public ecs::SingleComponentSystem<components::FleetEngagementRulesState> {
public:
    explicit FleetEngagementRulesSystem(ecs::World* world);
    ~FleetEngagementRulesSystem() override = default;

    std::string getName() const override { return "FleetEngagementRulesSystem"; }

    bool initialize(const std::string& entity_id);

    bool setRoe(const std::string& entity_id, components::FleetEngagementRulesState::RoeProfile roe);
    bool setPrimaryTarget(const std::string& entity_id, components::FleetEngagementRulesState::TargetPriority priority);
    bool setAutoEngageHostiles(const std::string& entity_id, bool value);
    bool setAutoEngageNeutrals(const std::string& entity_id, bool value);
    bool setRangeLimit(const std::string& entity_id, float range);
    bool broadcastTarget(const std::string& entity_id, const std::string& target_id);
    bool allHandsFire(const std::string& entity_id);
    bool ceaseFire(const std::string& entity_id);
    bool setInCombat(const std::string& entity_id, bool in_combat);
    bool setFleetId(const std::string& entity_id, const std::string& fleet_id);

    components::FleetEngagementRulesState::RoeProfile getRoe(const std::string& entity_id) const;
    components::FleetEngagementRulesState::TargetPriority getPrimaryTarget(const std::string& entity_id) const;
    bool        isAutoEngageHostiles(const std::string& entity_id) const;
    bool        isAutoEngageNeutrals(const std::string& entity_id) const;
    float       getRangeLimit(const std::string& entity_id) const;
    std::string getBroadcastTarget(const std::string& entity_id) const;
    bool        isAllHandsFire(const std::string& entity_id) const;
    bool        isCeaseFire(const std::string& entity_id) const;
    bool        isInCombat(const std::string& entity_id) const;
    int         getTotalEngagements(const std::string& entity_id) const;
    int         getTotalDisengages(const std::string& entity_id) const;
    float       getTimeSinceLastEngagement(const std::string& entity_id) const;
    std::string getFleetId(const std::string& entity_id) const;
    std::string getRoeString(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::FleetEngagementRulesState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_FLEET_ENGAGEMENT_RULES_SYSTEM_H
