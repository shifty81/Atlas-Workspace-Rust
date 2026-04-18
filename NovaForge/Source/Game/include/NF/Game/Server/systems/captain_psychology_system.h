#ifndef NOVAFORGE_SYSTEMS_CAPTAIN_PSYCHOLOGY_SYSTEM_H
#define NOVAFORGE_SYSTEMS_CAPTAIN_PSYCHOLOGY_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/fleet_components.h"
#include <string>

namespace atlas {
namespace systems {

class CaptainPsychologySystem
    : public ecs::SingleComponentSystem<components::CaptainPsychologyState> {
public:
    explicit CaptainPsychologySystem(ecs::World* world);
    ~CaptainPsychologySystem() override = default;

    std::string getName() const override { return "CaptainPsychologySystem"; }

    // --- Lifecycle ---
    bool initialize(const std::string& entity_id);

    // --- Personality configuration ---
    bool setBaseline(const std::string& entity_id,
                     float aggression, float caution, float loyalty, float greed);
    bool setCurrent(const std::string& entity_id,
                    float aggression, float caution, float loyalty, float greed);
    bool setDriftRate(const std::string& entity_id, float rate);
    bool setStressDecay(const std::string& entity_id, float rate);
    bool setCaptainId(const std::string& entity_id, const std::string& captain_id);

    // --- Event processing ---
    bool processEvent(const std::string& entity_id,
                      components::CaptainPsychologyState::EventType event);
    bool applyStress(const std::string& entity_id, float amount);
    bool resetStress(const std::string& entity_id);
    bool resetToBaseline(const std::string& entity_id);

    // --- Queries ---
    float getAggression(const std::string& entity_id) const;
    float getCaution(const std::string& entity_id) const;
    float getLoyalty(const std::string& entity_id) const;
    float getGreed(const std::string& entity_id) const;
    float getMood(const std::string& entity_id) const;
    float getStress(const std::string& entity_id) const;
    int   getEventsProcessed(const std::string& entity_id) const;
    int   getTotalShifts(const std::string& entity_id) const;
    bool  isAggressive(const std::string& entity_id) const;   // aggression > 0.7
    bool  isCautious(const std::string& entity_id) const;     // caution > 0.7
    bool  isLoyal(const std::string& entity_id) const;        // loyalty > 0.7
    bool  isGreedy(const std::string& entity_id) const;       // greed > 0.7
    bool  isStressed(const std::string& entity_id) const;     // stress > 0.7
    std::string getCaptainId(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::CaptainPsychologyState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_CAPTAIN_PSYCHOLOGY_SYSTEM_H
