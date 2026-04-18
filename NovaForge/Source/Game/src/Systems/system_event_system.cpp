#include "systems/system_event_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

SystemEventSystem::SystemEventSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void SystemEventSystem::updateComponent(
        ecs::Entity& /*entity*/,
        components::SystemEventState& comp,
        float delta_time) {
    if (!comp.active) return;

    // Tick down time_remaining for active events; auto-resolve expired ones
    for (auto it = comp.events.begin(); it != comp.events.end(); ) {
        if (it->is_active) {
            it->time_remaining -= delta_time;
            if (it->time_remaining <= 0.0f) {
                it = comp.events.erase(it);
                ++comp.total_events_resolved;
                continue;
            }
        }
        ++it;
    }

    // Auto-fire threshold events
    using ET = components::SystemEventState::SystemEventType;
    auto hasId = [&](const std::string& id) -> bool {
        for (const auto& e : comp.events) {
            if (e.event_id == id) return true;
        }
        return false;
    };
    int sz = static_cast<int>(comp.events.size());

    if (comp.threat_level >= comp.pirate_surge_threshold && !hasId("auto_pirate_surge") && sz < comp.max_events) {
        components::SystemEventState::SystemEvent ev;
        ev.event_id      = "auto_pirate_surge";
        ev.event_type    = ET::PirateSurge;
        ev.severity      = comp.threat_level;
        ev.duration      = 300.0f;
        ev.time_remaining = 300.0f;
        ev.is_active     = true;
        ev.trigger_value = comp.threat_level;
        comp.events.push_back(ev);
        ++comp.total_events_fired;
        ++sz;
    }
    if (comp.economy_health <= comp.shortage_threshold && !hasId("auto_trade_shortage") && sz < comp.max_events) {
        components::SystemEventState::SystemEvent ev;
        ev.event_id      = "auto_trade_shortage";
        ev.event_type    = ET::TradeShortage;
        ev.severity      = 1.0f - comp.economy_health;
        ev.duration      = 300.0f;
        ev.time_remaining = 300.0f;
        ev.is_active     = true;
        ev.trigger_value = comp.economy_health;
        comp.events.push_back(ev);
        ++comp.total_events_fired;
        ++sz;
    }
    if (comp.security_level <= comp.lockdown_threshold && !hasId("auto_security_lockdown") && sz < comp.max_events) {
        components::SystemEventState::SystemEvent ev;
        ev.event_id      = "auto_security_lockdown";
        ev.event_type    = ET::SecurityLockdown;
        ev.severity      = 1.0f - comp.security_level;
        ev.duration      = 300.0f;
        ev.time_remaining = 300.0f;
        ev.is_active     = true;
        ev.trigger_value = comp.security_level;
        comp.events.push_back(ev);
        ++comp.total_events_fired;
        ++sz;
    }
    if (comp.economy_health >= comp.boom_threshold && !hasId("auto_resource_boom") && sz < comp.max_events) {
        components::SystemEventState::SystemEvent ev;
        ev.event_id      = "auto_resource_boom";
        ev.event_type    = ET::ResourceBoom;
        ev.severity      = comp.economy_health;
        ev.duration      = 300.0f;
        ev.time_remaining = 300.0f;
        ev.is_active     = true;
        ev.trigger_value = comp.economy_health;
        comp.events.push_back(ev);
        ++comp.total_events_fired;
        ++sz;
    }

    comp.elapsed += delta_time;
}

bool SystemEventSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::SystemEventState>();
    entity->addComponent(std::move(comp));
    return true;
}

bool SystemEventSystem::fireEvent(const std::string& entity_id,
                                   const std::string& event_id,
                                   components::SystemEventState::SystemEventType event_type,
                                   float severity,
                                   float duration) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (event_id.empty()) return false;
    if (severity < 0.0f || severity > 1.0f) return false;
    if (duration <= 0.0f) return false;
    if (static_cast<int>(comp->events.size()) >= comp->max_events) return false;
    for (const auto& e : comp->events) {
        if (e.event_id == event_id) return false;
    }
    components::SystemEventState::SystemEvent ev;
    ev.event_id       = event_id;
    ev.event_type     = event_type;
    ev.severity       = severity;
    ev.duration       = duration;
    ev.time_remaining = duration;
    ev.is_active      = true;
    ev.trigger_value  = 0.0f;
    comp->events.push_back(ev);
    ++comp->total_events_fired;
    return true;
}

bool SystemEventSystem::resolveEvent(const std::string& entity_id,
                                      const std::string& event_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    auto it = std::find_if(comp->events.begin(), comp->events.end(),
        [&](const components::SystemEventState::SystemEvent& e) {
            return e.event_id == event_id;
        });
    if (it == comp->events.end()) return false;
    comp->events.erase(it);
    ++comp->total_events_resolved;
    return true;
}

bool SystemEventSystem::clearEvents(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->events.clear();
    return true;
}

bool SystemEventSystem::setThreatLevel(const std::string& entity_id, float value) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (value < 0.0f || value > 1.0f) return false;
    comp->threat_level = value;
    return true;
}

bool SystemEventSystem::setEconomyHealth(const std::string& entity_id, float value) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (value < 0.0f || value > 1.0f) return false;
    comp->economy_health = value;
    return true;
}

bool SystemEventSystem::setSecurityLevel(const std::string& entity_id, float value) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (value < 0.0f || value > 1.0f) return false;
    comp->security_level = value;
    return true;
}

bool SystemEventSystem::setTradeVolume(const std::string& entity_id, float value) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (value < 0.0f || value > 1.0f) return false;
    comp->trade_volume = value;
    return true;
}

bool SystemEventSystem::setPirateSurgeThreshold(const std::string& entity_id, float value) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (value < 0.0f || value > 1.0f) return false;
    comp->pirate_surge_threshold = value;
    return true;
}

bool SystemEventSystem::setShortageThreshold(const std::string& entity_id, float value) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (value < 0.0f || value > 1.0f) return false;
    comp->shortage_threshold = value;
    return true;
}

bool SystemEventSystem::setLockdownThreshold(const std::string& entity_id, float value) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (value < 0.0f || value > 1.0f) return false;
    comp->lockdown_threshold = value;
    return true;
}

bool SystemEventSystem::setBoomThreshold(const std::string& entity_id, float value) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (value < 0.0f || value > 1.0f) return false;
    comp->boom_threshold = value;
    return true;
}

bool SystemEventSystem::setSystemId(const std::string& entity_id,
                                     const std::string& system_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (system_id.empty()) return false;
    comp->system_id = system_id;
    return true;
}

bool SystemEventSystem::setMaxEvents(const std::string& entity_id, int max) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (max < 1) return false;
    comp->max_events = max;
    return true;
}

int SystemEventSystem::getEventCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return static_cast<int>(comp->events.size());
}

bool SystemEventSystem::hasEvent(const std::string& entity_id,
                                  const std::string& event_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& e : comp->events) {
        if (e.event_id == event_id) return true;
    }
    return false;
}

float SystemEventSystem::getEventSeverity(const std::string& entity_id,
                                           const std::string& event_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    for (const auto& e : comp->events) {
        if (e.event_id == event_id) return e.severity;
    }
    return 0.0f;
}

float SystemEventSystem::getTimeRemaining(const std::string& entity_id,
                                           const std::string& event_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    for (const auto& e : comp->events) {
        if (e.event_id == event_id) return e.time_remaining;
    }
    return 0.0f;
}

bool SystemEventSystem::isEventActive(const std::string& entity_id,
                                       const std::string& event_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& e : comp->events) {
        if (e.event_id == event_id) return e.is_active;
    }
    return false;
}

int SystemEventSystem::getTotalEventsFired(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return comp->total_events_fired;
}

int SystemEventSystem::getTotalEventsResolved(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return comp->total_events_resolved;
}

int SystemEventSystem::getActiveEventCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    int count = 0;
    for (const auto& e : comp->events) {
        if (e.is_active) ++count;
    }
    return count;
}

float SystemEventSystem::getThreatLevel(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    return comp->threat_level;
}

float SystemEventSystem::getEconomyHealth(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    return comp->economy_health;
}

float SystemEventSystem::getSecurityLevel(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    return comp->security_level;
}

float SystemEventSystem::getTradeVolume(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    return comp->trade_volume;
}

std::string SystemEventSystem::getSystemId(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return "";
    return comp->system_id;
}

int SystemEventSystem::getCountByType(
        const std::string& entity_id,
        components::SystemEventState::SystemEventType type) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    int count = 0;
    for (const auto& e : comp->events) {
        if (e.event_type == type) ++count;
    }
    return count;
}

} // namespace systems
} // namespace atlas
