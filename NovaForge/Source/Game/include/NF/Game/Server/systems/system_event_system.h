#ifndef NOVAFORGE_SYSTEMS_SYSTEM_EVENT_SYSTEM_H
#define NOVAFORGE_SYSTEMS_SYSTEM_EVENT_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/exploration_components.h"
#include <string>

namespace atlas {
namespace systems {

class SystemEventSystem
    : public ecs::SingleComponentSystem<components::SystemEventState> {
public:
    explicit SystemEventSystem(ecs::World* world);
    ~SystemEventSystem() override = default;

    std::string getName() const override { return "SystemEventSystem"; }

    bool initialize(const std::string& entity_id);

    bool fireEvent(const std::string& entity_id,
                   const std::string& event_id,
                   components::SystemEventState::SystemEventType event_type,
                   float severity,
                   float duration);
    bool resolveEvent(const std::string& entity_id, const std::string& event_id);
    bool clearEvents(const std::string& entity_id);

    bool setThreatLevel(const std::string& entity_id, float value);
    bool setEconomyHealth(const std::string& entity_id, float value);
    bool setSecurityLevel(const std::string& entity_id, float value);
    bool setTradeVolume(const std::string& entity_id, float value);
    bool setPirateSurgeThreshold(const std::string& entity_id, float value);
    bool setShortageThreshold(const std::string& entity_id, float value);
    bool setLockdownThreshold(const std::string& entity_id, float value);
    bool setBoomThreshold(const std::string& entity_id, float value);
    bool setSystemId(const std::string& entity_id, const std::string& system_id);
    bool setMaxEvents(const std::string& entity_id, int max);

    int   getEventCount(const std::string& entity_id) const;
    bool  hasEvent(const std::string& entity_id, const std::string& event_id) const;
    float getEventSeverity(const std::string& entity_id, const std::string& event_id) const;
    float getTimeRemaining(const std::string& entity_id, const std::string& event_id) const;
    bool  isEventActive(const std::string& entity_id, const std::string& event_id) const;
    int   getTotalEventsFired(const std::string& entity_id) const;
    int   getTotalEventsResolved(const std::string& entity_id) const;
    int   getActiveEventCount(const std::string& entity_id) const;
    float getThreatLevel(const std::string& entity_id) const;
    float getEconomyHealth(const std::string& entity_id) const;
    float getSecurityLevel(const std::string& entity_id) const;
    float getTradeVolume(const std::string& entity_id) const;
    std::string getSystemId(const std::string& entity_id) const;
    int   getCountByType(const std::string& entity_id,
                         components::SystemEventState::SystemEventType type) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::SystemEventState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_SYSTEM_EVENT_SYSTEM_H
