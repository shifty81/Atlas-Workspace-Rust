#ifndef NOVAFORGE_SYSTEMS_SECTOR_TENSION_SYSTEM_H
#define NOVAFORGE_SYSTEMS_SECTOR_TENSION_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/exploration_components.h"
#include <string>

namespace atlas {
namespace systems {

class SectorTensionSystem
    : public ecs::SingleComponentSystem<components::SectorTensionState> {
public:
    explicit SectorTensionSystem(ecs::World* world);
    ~SectorTensionSystem() override = default;

    std::string getName() const override { return "SectorTensionSystem"; }

    // --- Lifecycle ---
    bool initialize(const std::string& entity_id);

    // --- Event management ---
    bool addEvent(const std::string& entity_id,
                  const std::string& event_id,
                  components::SectorTensionState::TensionType type,
                  float magnitude,
                  float decay_rate,
                  float duration);
    bool removeEvent(const std::string& entity_id,
                     const std::string& event_id);
    bool clearEvents(const std::string& entity_id);

    // --- Configuration ---
    bool setSectorId(const std::string& entity_id,
                     const std::string& sector_id);
    bool setPassiveDecayRate(const std::string& entity_id, float rate);
    bool setMaxTension(const std::string& entity_id, float max);
    bool setMaxEvents(const std::string& entity_id, int max_events);

    // --- Queries ---
    float getTensionLevel(const std::string& entity_id) const;
    bool  isHighTension(const std::string& entity_id) const;
    bool  isCriticalTension(const std::string& entity_id) const;
    int   getEventCount(const std::string& entity_id) const;
    bool  hasEvent(const std::string& entity_id,
                   const std::string& event_id) const;
    std::string getSectorId(const std::string& entity_id) const;
    int   getTotalEventsRecorded(const std::string& entity_id) const;
    int   getCountByType(
              const std::string& entity_id,
              components::SectorTensionState::TensionType type) const;
    float getPassiveDecayRate(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::SectorTensionState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_SECTOR_TENSION_SYSTEM_H
