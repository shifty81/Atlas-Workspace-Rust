#ifndef NOVAFORGE_SYSTEMS_CAPTAIN_AMBITION_SYSTEM_H
#define NOVAFORGE_SYSTEMS_CAPTAIN_AMBITION_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/social_components.h"
#include <string>

namespace atlas {
namespace systems {

// CaptainAmbitionSystem — Phase B (Fleet Personality)
// Tracks per-captain long-term goals (ambitions). Progress is driven by
// external event calls (advanceProgress, markBlocked, markAchieved). When
// an ambition is achieved, departure_risk_contrib drops and a morale bonus
// fires. When blocked for long periods frustration_level rises, increasing
// departure_risk_contrib. Per-tick: frustration decays naturally; blocked
// ambitions accumulate frustration; departure_risk_contrib is recomputed.
class CaptainAmbitionSystem
    : public ecs::SingleComponentSystem<components::CaptainAmbitionState> {
public:
    explicit CaptainAmbitionSystem(ecs::World* world);
    ~CaptainAmbitionSystem() override = default;

    std::string getName() const override { return "CaptainAmbitionSystem"; }

    bool initialize(const std::string& entity_id);

    // Ambition management
    bool addAmbition(const std::string& entity_id,
                     const std::string& ambition_id,
                     components::AmbitionType type,
                     const std::string& description,
                     float target_value);
    bool removeAmbition(const std::string& entity_id,
                        const std::string& ambition_id);
    bool clearAmbitions(const std::string& entity_id);

    // Progress events
    bool advanceProgress(const std::string& entity_id,
                         const std::string& ambition_id,
                         float amount);
    bool markBlocked(const std::string& entity_id,
                     const std::string& ambition_id);
    bool markUnblocked(const std::string& entity_id,
                       const std::string& ambition_id);
    bool markAchieved(const std::string& entity_id,
                      const std::string& ambition_id);

    // Configuration
    bool setCaptainId(const std::string& entity_id,
                      const std::string& captain_id);
    bool setFrustrationDecayRate(const std::string& entity_id, float rate);
    bool setMaxAmbitions(const std::string& entity_id, int max);

    // Queries
    int         getAmbitionCount(const std::string& entity_id) const;
    bool        hasAmbition(const std::string& entity_id,
                            const std::string& ambition_id) const;
    float       getProgress(const std::string& entity_id,
                            const std::string& ambition_id) const;
    float       getFrustrationLevel(const std::string& entity_id,
                                    const std::string& ambition_id) const;
    bool        isAchieved(const std::string& entity_id,
                           const std::string& ambition_id) const;
    bool        isBlocked(const std::string& entity_id,
                          const std::string& ambition_id) const;
    components::AmbitionType
                getAmbitionType(const std::string& entity_id,
                                const std::string& ambition_id) const;
    float       getDepartureRiskContrib(const std::string& entity_id) const;
    int         getAchievedCount(const std::string& entity_id) const;
    int         getBlockedCount(const std::string& entity_id) const;
    int         getTotalAmbitionsSet(const std::string& entity_id) const;
    int         getTotalAchieved(const std::string& entity_id) const;
    int         getTotalBlocked(const std::string& entity_id) const;
    std::string getCaptainId(const std::string& entity_id) const;
    float       getFrustrationDecayRate(const std::string& entity_id) const;
    int         getMaxAmbitions(const std::string& entity_id) const;
    int         getCountByType(const std::string& entity_id,
                               components::AmbitionType type) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::CaptainAmbitionState& comp,
                         float delta_time) override;

private:
    void recomputeDepartureRisk(components::CaptainAmbitionState& comp);
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_CAPTAIN_AMBITION_SYSTEM_H
