#ifndef NOVAFORGE_SYSTEMS_CAPTAIN_STRESS_SYSTEM_H
#define NOVAFORGE_SYSTEMS_CAPTAIN_STRESS_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/fleet_components.h"
#include <string>

namespace atlas {
namespace systems {

// CaptainStressSystem — Phase B/G (Fleet Personality & Operational Wear)
// Tracks per-captain accumulated stress from combat, near-deaths, and long
// deployments. Stress passively recovers between engagements; relief events
// (rest, shore-leave, mission success) reduce it faster.
// High stress (> threshold) triggers morale penalties and escalates departure
// risk. Critical stress (> critical_level) can force departure requests.
class CaptainStressSystem
    : public ecs::SingleComponentSystem<components::CaptainStressState> {
public:
    explicit CaptainStressSystem(ecs::World* world);
    ~CaptainStressSystem() override = default;

    std::string getName() const override { return "CaptainStressSystem"; }

    bool initialize(const std::string& entity_id);

    // Stressor events
    bool recordCombat(const std::string& entity_id, float intensity);
    bool recordNearDeath(const std::string& entity_id);
    bool recordMissionFailure(const std::string& entity_id);
    bool recordLongDeployment(const std::string& entity_id, float hours);

    // Relief events
    bool applyRest(const std::string& entity_id, float hours);
    bool applyRelief(const std::string& entity_id, float amount);
    bool applyShoreleave(const std::string& entity_id);

    // Configuration
    bool setRecoveryRate(const std::string& entity_id, float rate);
    bool setStressThreshold(const std::string& entity_id, float val);
    bool setCriticalLevel(const std::string& entity_id, float val);
    bool setCaptainId(const std::string& entity_id, const std::string& captain_id);

    // Queries
    float       getStressLevel(const std::string& entity_id) const;
    float       getRecoveryRate(const std::string& entity_id) const;
    float       getStressThreshold(const std::string& entity_id) const;
    float       getCriticalLevel(const std::string& entity_id) const;
    bool        isHighStress(const std::string& entity_id) const;
    bool        isCriticalStress(const std::string& entity_id) const;
    float       getStressPercent(const std::string& entity_id) const;
    int         getTotalStressorsApplied(const std::string& entity_id) const;
    int         getTotalReliefEvents(const std::string& entity_id) const;
    std::string getCaptainId(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::CaptainStressState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_CAPTAIN_STRESS_SYSTEM_H
