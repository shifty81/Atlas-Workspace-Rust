#ifndef NOVAFORGE_SYSTEMS_FACTION_BEHAVIOR_MODIFIER_SYSTEM_H
#define NOVAFORGE_SYSTEMS_FACTION_BEHAVIOR_MODIFIER_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/fleet_components.h"
#include <string>

namespace atlas {
namespace systems {

class FactionBehaviorModifierSystem
    : public ecs::SingleComponentSystem<components::FactionBehaviorState> {
public:
    explicit FactionBehaviorModifierSystem(ecs::World* world);
    ~FactionBehaviorModifierSystem() override = default;

    std::string getName() const override { return "FactionBehaviorModifierSystem"; }

    // --- Lifecycle ---
    bool initialize(const std::string& entity_id);

    // --- Profile ---
    bool setProfile(const std::string& entity_id,
                    components::FactionBehaviorState::FactionProfile profile);

    bool setFactionId(const std::string& entity_id, const std::string& faction_id);

    // --- Fine-grained configuration ---
    bool setMoraleBias(const std::string& entity_id, float bias);
    bool setChatterRateMult(const std::string& entity_id, float mult);
    bool setCombatPreference(const std::string& entity_id, float pref);
    bool setMiningPreference(const std::string& entity_id, float pref);
    bool setExplorationPreference(const std::string& entity_id, float pref);
    bool setTradePreference(const std::string& entity_id, float pref);
    bool setDepartureThreshold(const std::string& entity_id, float threshold);

    // --- Derived computations ---
    float applyMoraleModifier(const std::string& entity_id,
                              float base_morale) const;

    std::string getDominantActivity(const std::string& entity_id) const;

    bool isDepartureRisk(const std::string& entity_id, float morale) const;

    // --- Queries ---
    components::FactionBehaviorState::FactionProfile
        getProfile(const std::string& entity_id) const;
    components::FactionBehaviorState::MoraleDriver
        getMoraleDriver(const std::string& entity_id) const;
    float       getMoraleBias(const std::string& entity_id) const;
    float       getChatterRateMult(const std::string& entity_id) const;
    float       getCombatPreference(const std::string& entity_id) const;
    float       getMiningPreference(const std::string& entity_id) const;
    float       getExplorationPreference(const std::string& entity_id) const;
    float       getTradePreference(const std::string& entity_id) const;
    float       getDepartureThreshold(const std::string& entity_id) const;
    std::string getFactionId(const std::string& entity_id) const;
    int         getTotalProfileChanges(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::FactionBehaviorState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_FACTION_BEHAVIOR_MODIFIER_SYSTEM_H
