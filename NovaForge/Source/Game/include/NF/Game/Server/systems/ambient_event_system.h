#ifndef NOVAFORGE_SYSTEMS_AMBIENT_EVENT_SYSTEM_H
#define NOVAFORGE_SYSTEMS_AMBIENT_EVENT_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/exploration_components.h"
#include <string>

namespace atlas {
namespace systems {

// AmbientEventSystem — Phase E (Living Galaxy Simulation)
// Non-combat ambient events for a solar system.
// Events (NavBeaconMalfunction, StationLockdown, RadiationStorm, DistressBeacon,
// SalvageFieldAppearance, TrafficJam) have a TTL; per-tick countdown auto-resolves
// expired events.
class AmbientEventSystem
    : public ecs::SingleComponentSystem<components::AmbientEventState> {
public:
    explicit AmbientEventSystem(ecs::World* world);
    ~AmbientEventSystem() override = default;

    std::string getName() const override { return "AmbientEventSystem"; }

    bool initialize(const std::string& entity_id);

    // Event management
    bool fireEvent(const std::string& entity_id,
                   const std::string& event_id,
                   components::AmbientEventState::AmbientEventType event_type,
                   float intensity,
                   float duration);
    bool resolveEvent(const std::string& entity_id, const std::string& event_id);
    bool clearEvents(const std::string& entity_id);
    bool removeEvent(const std::string& entity_id, const std::string& event_id);

    // Configuration
    bool setSystemId(const std::string& entity_id, const std::string& system_id);
    bool setMaxEvents(const std::string& entity_id, int max);

    // Queries
    int         getEventCount(const std::string& entity_id) const;
    int         getActiveEventCount(const std::string& entity_id) const;
    bool        hasEvent(const std::string& entity_id, const std::string& event_id) const;
    bool        isEventActive(const std::string& entity_id, const std::string& event_id) const;
    float       getEventIntensity(const std::string& entity_id, const std::string& event_id) const;
    float       getTimeRemaining(const std::string& entity_id, const std::string& event_id) const;
    components::AmbientEventState::AmbientEventType
                getEventType(const std::string& entity_id, const std::string& event_id) const;
    int         getCountByType(const std::string& entity_id,
                               components::AmbientEventState::AmbientEventType type) const;
    int         getTotalEventsFired(const std::string& entity_id) const;
    int         getTotalEventsResolved(const std::string& entity_id) const;
    std::string getSystemId(const std::string& entity_id) const;
    int         getMaxEvents(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::AmbientEventState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_AMBIENT_EVENT_SYSTEM_H
