#ifndef NOVAFORGE_SYSTEMS_FACTION_DOCTRINE_SYSTEM_H
#define NOVAFORGE_SYSTEMS_FACTION_DOCTRINE_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/fleet_components.h"
#include <string>

namespace atlas {
namespace systems {

// FactionDoctrineSystem — Phase F (Pirate Titan Meta-Threat)
// Pirate coalition AI doctrine state machine driven by Titan completion %,
// discovery risk, resource scarcity, and player proximity.
// Doctrine phases: Accumulate → Conceal → Disrupt → Defend → PrepareLaunch
// Per-tick: recomputes aggression/stealth/raid_frequency from current phase.
class FactionDoctrineSystem
    : public ecs::SingleComponentSystem<components::FactionDoctrineState> {
public:
    explicit FactionDoctrineSystem(ecs::World* world);
    ~FactionDoctrineSystem() override = default;

    std::string getName() const override { return "FactionDoctrineSystem"; }

    bool initialize(const std::string& entity_id);

    // Driver mutators
    bool setTitanCompletion(const std::string& entity_id, float val);
    bool setDiscoveryRisk(const std::string& entity_id, float val);
    bool setResourceScarcity(const std::string& entity_id, float val);
    bool setPlayerProximity(const std::string& entity_id, float val);

    // Threshold configuration
    bool setConcealThreshold(const std::string& entity_id, float val);
    bool setDisruptThreshold(const std::string& entity_id, float val);
    bool setDefendThreshold(const std::string& entity_id, float val);
    bool setLaunchThreshold(const std::string& entity_id, float val);

    // Configuration
    bool setFactionId(const std::string& entity_id, const std::string& faction_id);

    // Force a manual phase advance (for testing / scripted events)
    bool advancePhase(const std::string& entity_id);
    bool resetToAccumulate(const std::string& entity_id);

    // Queries
    components::FactionDoctrineState::DoctrinePhase
                getDoctrinePhase(const std::string& entity_id) const;
    std::string getDoctrinePhaseString(const std::string& entity_id) const;
    float       getTitanCompletion(const std::string& entity_id) const;
    float       getDiscoveryRisk(const std::string& entity_id) const;
    float       getResourceScarcity(const std::string& entity_id) const;
    float       getPlayerProximity(const std::string& entity_id) const;
    float       getAggressionMult(const std::string& entity_id) const;
    float       getStealthBias(const std::string& entity_id) const;
    float       getRaidFrequency(const std::string& entity_id) const;
    int         getTotalPhaseShifts(const std::string& entity_id) const;
    std::string getFactionId(const std::string& entity_id) const;
    bool        isLaunchImminent(const std::string& entity_id) const;
    bool        isActive(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::FactionDoctrineState& comp,
                         float delta_time) override;

private:
    void recomputeProfile(components::FactionDoctrineState& comp) const;
    void applyPhaseTransitions(components::FactionDoctrineState& comp) const;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_FACTION_DOCTRINE_SYSTEM_H
