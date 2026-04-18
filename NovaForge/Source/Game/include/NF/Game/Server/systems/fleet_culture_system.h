#ifndef NOVAFORGE_SYSTEMS_FLEET_CULTURE_SYSTEM_H
#define NOVAFORGE_SYSTEMS_FLEET_CULTURE_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/social_components.h"
#include <string>

namespace atlas {
namespace systems {

// FleetCultureSystem — Phase B/G (Fleet Personality / Additional Features)
// Tracks emergent cultural elements of a fleet: traditions, taboos, mottos,
// and rituals. Elements form through repeated patterns and gain strength with
// each reinforcement. Violating a culture element raises fleet tension and
// can damage cohesion. Reinforcing it builds cohesion_bonus over time.
// Per-tick: tension decays toward 0; cohesion_bonus is recomputed from active
// element strengths.
class FleetCultureSystem
    : public ecs::SingleComponentSystem<components::FleetCultureState> {
public:
    explicit FleetCultureSystem(ecs::World* world);
    ~FleetCultureSystem() override = default;

    std::string getName() const override { return "FleetCultureSystem"; }

    bool initialize(const std::string& entity_id);

    // Element management
    bool addElement(const std::string& entity_id,
                    const std::string& element_id,
                    const std::string& name,
                    components::CultureElementType type,
                    const std::string& description);
    bool removeElement(const std::string& entity_id,
                       const std::string& element_id);
    bool clearElements(const std::string& entity_id);

    // Culture events
    bool reinforce(const std::string& entity_id,
                   const std::string& element_id);
    bool violate(const std::string& entity_id,
                 const std::string& element_id);
    bool deactivateElement(const std::string& entity_id,
                           const std::string& element_id);

    // Configuration
    bool setFleetId(const std::string& entity_id,
                    const std::string& fleet_id);
    bool setMaxElements(const std::string& entity_id, int max);
    bool setTensionDecayRate(const std::string& entity_id, float rate);

    // Queries
    int         getElementCount(const std::string& entity_id) const;
    int         getActiveElementCount(const std::string& entity_id) const;
    bool        hasElement(const std::string& entity_id,
                           const std::string& element_id) const;
    bool        isElementActive(const std::string& entity_id,
                                const std::string& element_id) const;
    float       getElementStrength(const std::string& entity_id,
                                   const std::string& element_id) const;
    int         getReinforcementCount(const std::string& entity_id,
                                      const std::string& element_id) const;
    int         getViolationCount(const std::string& entity_id,
                                  const std::string& element_id) const;
    std::string getElementName(const std::string& entity_id,
                               const std::string& element_id) const;
    components::CultureElementType
                getElementType(const std::string& entity_id,
                               const std::string& element_id) const;
    float       getCohesionBonus(const std::string& entity_id) const;
    float       getTensionLevel(const std::string& entity_id) const;
    bool        isHighTension(const std::string& entity_id) const;
    int         getTotalElementsFormed(const std::string& entity_id) const;
    int         getTotalReinforcements(const std::string& entity_id) const;
    int         getTotalViolations(const std::string& entity_id) const;
    std::string getFleetId(const std::string& entity_id) const;
    int         getCountByType(const std::string& entity_id,
                               components::CultureElementType type) const;
    int         getMaxElements(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::FleetCultureState& comp,
                         float delta_time) override;

private:
    void recomputeCohesionBonus(components::FleetCultureState& comp);
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_FLEET_CULTURE_SYSTEM_H
