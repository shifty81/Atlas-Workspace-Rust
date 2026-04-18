#ifndef NOVAFORGE_SYSTEMS_FLEET_MORALE_SYSTEM_H
#define NOVAFORGE_SYSTEMS_FLEET_MORALE_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/fleet_components.h"
#include <string>

namespace atlas {
namespace systems {

class FleetMoraleSystem
    : public ecs::SingleComponentSystem<components::FleetMoraleState> {
public:
    explicit FleetMoraleSystem(ecs::World* world);
    ~FleetMoraleSystem() override = default;

    std::string getName() const override { return "FleetMoraleSystem"; }

    // --- Lifecycle ---
    bool initialize(const std::string& entity_id);

    // --- Event recording ---
    bool recordEvent(const std::string& entity_id,
                     components::FleetMoraleState::MoraleEvent event);
    bool clearEventLog(const std::string& entity_id);

    // --- Morale manipulation ---
    bool boostMorale(const std::string& entity_id, float amount);
    bool reduceMorale(const std::string& entity_id, float amount);
    bool resetMorale(const std::string& entity_id);

    // --- Cohesion manipulation ---
    bool boostCohesion(const std::string& entity_id, float amount);
    bool reduceCohesion(const std::string& entity_id, float amount);

    // --- Configuration ---
    bool setFleetId(const std::string& entity_id, const std::string& fleet_id);
    bool setMoraleDecay(const std::string& entity_id, float rate);
    bool setCohesionDecay(const std::string& entity_id, float rate);
    bool setMoraleBaseline(const std::string& entity_id, float value);
    bool setCohesionBaseline(const std::string& entity_id, float value);
    bool setMaxEventLog(const std::string& entity_id, int max_size);

    // --- Queries ---
    float getMorale(const std::string& entity_id) const;
    float getCohesion(const std::string& entity_id) const;
    int   getEventCount(const std::string& entity_id) const;
    int   getTotalEvents(const std::string& entity_id) const;
    int   getVictories(const std::string& entity_id) const;
    int   getDefeats(const std::string& entity_id) const;
    bool  isHighMorale(const std::string& entity_id) const;
    bool  isLowMorale(const std::string& entity_id) const;
    bool  isHighCohesion(const std::string& entity_id) const;
    bool  isLowCohesion(const std::string& entity_id) const;
    std::string getFleetId(const std::string& entity_id) const;

    // --- FleetMorale convenience API (operates on FleetMorale component) ---
    bool  recordWin(const std::string& entity_id);
    bool  recordLoss(const std::string& entity_id);
    bool  recordShipLost(const std::string& entity_id);
    bool  recordSavedByPlayer(const std::string& entity_id);
    bool  recordMissionTogether(const std::string& entity_id);
    float getMoraleScore(const std::string& entity_id) const;
    std::string getMoraleState(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::FleetMoraleState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_FLEET_MORALE_SYSTEM_H
