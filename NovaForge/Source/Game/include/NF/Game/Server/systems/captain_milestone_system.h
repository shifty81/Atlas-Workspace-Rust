#ifndef NOVAFORGE_SYSTEMS_CAPTAIN_MILESTONE_SYSTEM_H
#define NOVAFORGE_SYSTEMS_CAPTAIN_MILESTONE_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/fleet_components.h"
#include <string>

namespace atlas {
namespace systems {

class CaptainMilestoneSystem
    : public ecs::SingleComponentSystem<components::CaptainMilestoneState> {
public:
    explicit CaptainMilestoneSystem(ecs::World* world);
    ~CaptainMilestoneSystem() override = default;

    std::string getName() const override { return "CaptainMilestoneSystem"; }

    bool initialize(const std::string& entity_id);

    bool addMilestone(const std::string& entity_id,
                      const std::string& milestone_id,
                      components::CaptainMilestoneState::MilestoneType type,
                      const std::string& description,
                      int career_points);
    bool achieveMilestone(const std::string& entity_id, const std::string& milestone_id);
    bool resetMilestone(const std::string& entity_id, const std::string& milestone_id);
    bool removeMilestone(const std::string& entity_id, const std::string& milestone_id);
    bool clearMilestones(const std::string& entity_id);
    bool setCaptainId(const std::string& entity_id, const std::string& captain_id);

    bool        isMilestoneAchieved(const std::string& entity_id, const std::string& milestone_id) const;
    int         getMilestoneCount(const std::string& entity_id) const;
    int         getAchievedCount(const std::string& entity_id) const;
    int         getCareerPoints(const std::string& entity_id) const;
    std::string getCaptainRank(const std::string& entity_id) const;
    bool        hasMilestone(const std::string& entity_id, const std::string& milestone_id) const;
    int         getMilestoneCareerPoints(const std::string& entity_id, const std::string& milestone_id) const;
    std::string getCaptainId(const std::string& entity_id) const;
    int         getCountByType(const std::string& entity_id, components::CaptainMilestoneState::MilestoneType type) const;
    int         getTotalAchieved(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::CaptainMilestoneState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_CAPTAIN_MILESTONE_SYSTEM_H
