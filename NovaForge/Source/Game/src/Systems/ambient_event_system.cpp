#include "systems/ambient_event_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

AmbientEventSystem::AmbientEventSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void AmbientEventSystem::updateComponent(
        ecs::Entity& /*entity*/,
        components::AmbientEventState& comp,
        float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;

    // Per-tick countdown; auto-resolve expired events
    for (auto& ev : comp.events) {
        if (ev.is_resolved) continue;
        ev.time_remaining -= delta_time;
        if (ev.time_remaining <= 0.0f) {
            ev.time_remaining = 0.0f;
            ev.is_resolved    = true;
            ++comp.total_events_resolved;
        }
    }

    // Remove resolved events
    comp.events.erase(
        std::remove_if(comp.events.begin(), comp.events.end(),
            [](const components::AmbientEventState::AmbientEvent& e) {
                return e.is_resolved;
            }),
        comp.events.end());
}

bool AmbientEventSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::AmbientEventState>();
    entity->addComponent(std::move(comp));
    return true;
}

bool AmbientEventSystem::fireEvent(
        const std::string& entity_id,
        const std::string& event_id,
        components::AmbientEventState::AmbientEventType event_type,
        float intensity,
        float duration) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (event_id.empty()) return false;
    if (intensity < 0.0f || intensity > 1.0f) return false;
    if (duration <= 0.0f) return false;
    if (static_cast<int>(comp->events.size()) >= comp->max_events) return false;
    for (const auto& ev : comp->events) {
        if (ev.event_id == event_id) return false;
    }
    components::AmbientEventState::AmbientEvent ev;
    ev.event_id      = event_id;
    ev.event_type    = event_type;
    ev.intensity     = intensity;
    ev.duration      = duration;
    ev.time_remaining = duration;
    ev.is_resolved   = false;
    comp->events.push_back(ev);
    ++comp->total_events_fired;
    return true;
}

bool AmbientEventSystem::resolveEvent(const std::string& entity_id,
                                       const std::string& event_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto& ev : comp->events) {
        if (ev.event_id == event_id && !ev.is_resolved) {
            ev.is_resolved = true;
            ++comp->total_events_resolved;
            comp->events.erase(
                std::remove_if(comp->events.begin(), comp->events.end(),
                    [&](const components::AmbientEventState::AmbientEvent& e) {
                        return e.is_resolved;
                    }),
                comp->events.end());
            return true;
        }
    }
    return false;
}

bool AmbientEventSystem::clearEvents(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->events.clear();
    return true;
}

bool AmbientEventSystem::removeEvent(const std::string& entity_id,
                                      const std::string& event_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    auto it = std::find_if(comp->events.begin(), comp->events.end(),
        [&](const components::AmbientEventState::AmbientEvent& e) {
            return e.event_id == event_id;
        });
    if (it == comp->events.end()) return false;
    comp->events.erase(it);
    return true;
}

bool AmbientEventSystem::setSystemId(const std::string& entity_id,
                                      const std::string& system_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (system_id.empty()) return false;
    comp->system_id = system_id;
    return true;
}

bool AmbientEventSystem::setMaxEvents(const std::string& entity_id, int max) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (max < 1) return false;
    comp->max_events = max;
    return true;
}

int AmbientEventSystem::getEventCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->events.size()) : 0;
}

int AmbientEventSystem::getActiveEventCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    int count = 0;
    for (const auto& ev : comp->events) {
        if (!ev.is_resolved) ++count;
    }
    return count;
}

bool AmbientEventSystem::hasEvent(const std::string& entity_id,
                                   const std::string& event_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& ev : comp->events) {
        if (ev.event_id == event_id) return true;
    }
    return false;
}

bool AmbientEventSystem::isEventActive(const std::string& entity_id,
                                        const std::string& event_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& ev : comp->events) {
        if (ev.event_id == event_id) return !ev.is_resolved;
    }
    return false;
}

float AmbientEventSystem::getEventIntensity(const std::string& entity_id,
                                             const std::string& event_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    for (const auto& ev : comp->events) {
        if (ev.event_id == event_id) return ev.intensity;
    }
    return 0.0f;
}

float AmbientEventSystem::getTimeRemaining(const std::string& entity_id,
                                            const std::string& event_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    for (const auto& ev : comp->events) {
        if (ev.event_id == event_id) return ev.time_remaining;
    }
    return 0.0f;
}

components::AmbientEventState::AmbientEventType
AmbientEventSystem::getEventType(const std::string& entity_id,
                                  const std::string& event_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return components::AmbientEventState::AmbientEventType::NavBeaconMalfunction;
    for (const auto& ev : comp->events) {
        if (ev.event_id == event_id) return ev.event_type;
    }
    return components::AmbientEventState::AmbientEventType::NavBeaconMalfunction;
}

int AmbientEventSystem::getCountByType(
        const std::string& entity_id,
        components::AmbientEventState::AmbientEventType type) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    int count = 0;
    for (const auto& ev : comp->events) {
        if (ev.event_type == type) ++count;
    }
    return count;
}

int AmbientEventSystem::getTotalEventsFired(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_events_fired : 0;
}

int AmbientEventSystem::getTotalEventsResolved(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_events_resolved : 0;
}

std::string AmbientEventSystem::getSystemId(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->system_id : "";
}

int AmbientEventSystem::getMaxEvents(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->max_events : 0;
}

} // namespace systems
} // namespace atlas
