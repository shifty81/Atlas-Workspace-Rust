#include "systems/sector_tension_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

SectorTensionSystem::SectorTensionSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void SectorTensionSystem::updateComponent(
        ecs::Entity& /*entity*/,
        components::SectorTensionState& comp,
        float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;

    // Advance event timers; sum per-event decay
    float event_decay = 0.0f;
    for (auto& ev : comp.events) {
        ev.time_remaining -= delta_time;
        if (ev.time_remaining > 0.0f)
            event_decay += ev.decay_rate * delta_time;
    }

    // Remove expired events
    comp.events.erase(
        std::remove_if(comp.events.begin(), comp.events.end(),
            [](const auto& ev) { return ev.time_remaining <= 0.0f; }),
        comp.events.end());

    // Apply passive decay and event-driven decay
    float total_decay = (comp.passive_decay_rate + event_decay);
    comp.tension_level -= total_decay;
    if (comp.tension_level < 0.0f) comp.tension_level = 0.0f;
}

bool SectorTensionSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::SectorTensionState>();
    entity->addComponent(std::move(comp));
    return true;
}

bool SectorTensionSystem::addEvent(
        const std::string& entity_id,
        const std::string& event_id,
        components::SectorTensionState::TensionType type,
        float magnitude,
        float decay_rate,
        float duration) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (event_id.empty()) return false;
    if (magnitude <= 0.0f) return false;
    if (decay_rate < 0.0f) return false;
    if (duration <= 0.0f) return false;
    if (static_cast<int>(comp->events.size()) >= comp->max_events) return false;
    for (const auto& ev : comp->events)
        if (ev.event_id == event_id) return false;

    components::SectorTensionState::TensionEvent ev;
    ev.event_id       = event_id;
    ev.type           = type;
    ev.magnitude      = magnitude;
    ev.decay_rate     = decay_rate;
    ev.time_remaining = duration;
    comp->events.push_back(ev);

    comp->tension_level = std::min(comp->max_tension,
                                   comp->tension_level + magnitude);
    ++comp->total_events_recorded;
    return true;
}

bool SectorTensionSystem::removeEvent(const std::string& entity_id,
                                       const std::string& event_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    auto it = std::find_if(comp->events.begin(), comp->events.end(),
        [&](const auto& ev) { return ev.event_id == event_id; });
    if (it == comp->events.end()) return false;
    comp->events.erase(it);
    return true;
}

bool SectorTensionSystem::clearEvents(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->events.clear();
    return true;
}

bool SectorTensionSystem::setSectorId(const std::string& entity_id,
                                       const std::string& sector_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (sector_id.empty()) return false;
    comp->sector_id = sector_id;
    return true;
}

bool SectorTensionSystem::setPassiveDecayRate(const std::string& entity_id,
                                               float rate) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (rate < 0.0f) return false;
    comp->passive_decay_rate = rate;
    return true;
}

bool SectorTensionSystem::setMaxTension(const std::string& entity_id,
                                         float max) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (max <= 0.0f) return false;
    comp->max_tension = max;
    return true;
}

bool SectorTensionSystem::setMaxEvents(const std::string& entity_id,
                                        int max_events) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (max_events < 1) return false;
    comp->max_events = max_events;
    return true;
}

float SectorTensionSystem::getTensionLevel(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    return comp->tension_level;
}

bool SectorTensionSystem::isHighTension(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    return comp->tension_level >= 75.0f;
}

bool SectorTensionSystem::isCriticalTension(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    return comp->tension_level >= 90.0f;
}

int SectorTensionSystem::getEventCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return static_cast<int>(comp->events.size());
}

bool SectorTensionSystem::hasEvent(const std::string& entity_id,
                                    const std::string& event_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& ev : comp->events)
        if (ev.event_id == event_id) return true;
    return false;
}

std::string SectorTensionSystem::getSectorId(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return "";
    return comp->sector_id;
}

int SectorTensionSystem::getTotalEventsRecorded(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return comp->total_events_recorded;
}

int SectorTensionSystem::getCountByType(
        const std::string& entity_id,
        components::SectorTensionState::TensionType type) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    int count = 0;
    for (const auto& ev : comp->events)
        if (ev.type == type) ++count;
    return count;
}

float SectorTensionSystem::getPassiveDecayRate(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    return comp->passive_decay_rate;
}

} // namespace systems
} // namespace atlas
