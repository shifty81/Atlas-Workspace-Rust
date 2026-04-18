#ifndef NOVAFORGE_SYSTEMS_BEHAVIORAL_REPUTATION_SYSTEM_H
#define NOVAFORGE_SYSTEMS_BEHAVIORAL_REPUTATION_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

class BehavioralReputationSystem
    : public ecs::SingleComponentSystem<components::BehavioralReputationState> {
public:
    explicit BehavioralReputationSystem(ecs::World* world);
    ~BehavioralReputationSystem() override = default;

    std::string getName() const override { return "BehavioralReputationSystem"; }

    // --- Lifecycle ---
    bool initialize(const std::string& entity_id);

    // --- Record management ---
    bool recordBehavior(const std::string& entity_id,
                        const std::string& record_id,
                        components::BehavioralReputationState::BehaviorType type,
                        float impact);

    bool removeBehavior(const std::string& entity_id,
                        const std::string& record_id);

    bool clearRecords(const std::string& entity_id);

    // --- Configuration ---
    bool setPlayerId(const std::string& entity_id, const std::string& player_id);
    bool setMaxRecords(const std::string& entity_id, int max);

    // --- Queries ---
    float       getGenerosityScore(const std::string& entity_id) const;
    float       getLoyaltyScore(const std::string& entity_id) const;
    float       getSalvageScore(const std::string& entity_id) const;
    float       getDistressScore(const std::string& entity_id) const;
    float       getOverallReputation(const std::string& entity_id) const;
    int         getRecordCount(const std::string& entity_id) const;
    bool        hasRecord(const std::string& entity_id,
                          const std::string& record_id) const;
    int         getOccurrenceCount(const std::string& entity_id,
                                   const std::string& record_id) const;
    float       getImpact(const std::string& entity_id,
                          const std::string& record_id) const;
    int         getTotalRecordsEver(const std::string& entity_id) const;
    int         getCountByType(const std::string& entity_id,
                               components::BehavioralReputationState::BehaviorType type) const;
    std::string getDominantBehaviorType(const std::string& entity_id) const;
    std::string getPlayerId(const std::string& entity_id) const;
    int         getMaxRecords(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::BehavioralReputationState& comp,
                         float delta_time) override;

private:
    static void applyImpactToScores(
        components::BehavioralReputationState& comp,
        components::BehavioralReputationState::BehaviorType type,
        float impact,
        float sign);

    static std::string behaviorTypeToString(
        components::BehavioralReputationState::BehaviorType type);
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_BEHAVIORAL_REPUTATION_SYSTEM_H
