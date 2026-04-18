#ifndef NOVAFORGE_SYSTEMS_EMOTIONAL_ARC_SYSTEM_H
#define NOVAFORGE_SYSTEMS_EMOTIONAL_ARC_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/social_components.h"
#include "components/fleet_components.h"
#include <string>

namespace atlas {
namespace systems {

// EmotionalArcSystem — Phase B (Fleet Personality & Social Systems)
// Tracks per-captain emotional arc via discrete event calls.
// confidence / trust_in_player / fatigue / hope (all 0–1) drive an
// emergent arc label that is computed from current values.
class EmotionalArcSystem
    : public ecs::SingleComponentSystem<components::EmotionalArcState> {
public:
    explicit EmotionalArcSystem(ecs::World* world);
    ~EmotionalArcSystem() override = default;

    std::string getName() const override { return "EmotionalArcSystem"; }

    bool initialize(const std::string& entity_id);

    // Discrete event mutators (return false if entity/component missing)
    bool applyWin(const std::string& entity_id);
    bool applyLoss(const std::string& entity_id);
    bool applyNearDeath(const std::string& entity_id);
    bool applySave(const std::string& entity_id);
    bool applyRest(const std::string& entity_id, float hours);
    bool applyExploration(const std::string& entity_id);

    // Fine-grained setters — clamp to [0,1]
    bool setConfidence(const std::string& entity_id, float val);
    bool setTrustInPlayer(const std::string& entity_id, float val);
    bool setFatigue(const std::string& entity_id, float val);
    bool setHope(const std::string& entity_id, float val);
    bool setCaptainId(const std::string& entity_id, const std::string& captain_id);

    // Queries — return safe defaults on missing entity
    float       getConfidence(const std::string& entity_id) const;
    float       getTrustInPlayer(const std::string& entity_id) const;
    float       getFatigue(const std::string& entity_id) const;
    float       getHope(const std::string& entity_id) const;
    std::string getArcLabel(const std::string& entity_id) const;
    bool        isWornDown(const std::string& entity_id) const;
    bool        isLoyalToPlayer(const std::string& entity_id) const;
    bool        isOptimistic(const std::string& entity_id) const;
    int         getWins(const std::string& entity_id) const;
    int         getLosses(const std::string& entity_id) const;
    int         getNearDeaths(const std::string& entity_id) const;
    int         getSavesByPlayer(const std::string& entity_id) const;
    int         getTotalArcUpdates(const std::string& entity_id) const;
    std::string getCaptainId(const std::string& entity_id) const;

    // Convenience aliases — operate on EmotionalArcState (create if absent) or EmotionalState
    bool  onCombatVictory(const std::string& entity_id);
    bool  onCombatDefeat(const std::string& entity_id);
    bool  onRest(const std::string& entity_id);
    bool  onPlayerTrust(const std::string& entity_id);
    bool  onPlayerBetray(const std::string& entity_id);
    float getTrust(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::EmotionalArcState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_EMOTIONAL_ARC_SYSTEM_H
