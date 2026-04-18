#include "systems/operational_wear_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

OperationalWearSystem::OperationalWearSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void OperationalWearSystem::updateComponent(
        ecs::Entity& /*entity*/,
        components::OperationalWearState& comp,
        float delta_time) {
    if (!comp.active) return;
    comp.elapsed             += delta_time;
    comp.deployment_duration += delta_time;

    if (comp.is_docked) {
        // Recover wear while docked
        comp.wear_level -= comp.recovery_rate * delta_time;
        if (comp.wear_level < 0.0f) comp.wear_level = 0.0f;
    } else if (comp.deployment_duration > comp.rotation_threshold) {
        // Passive wear accumulates past rotation threshold
        comp.wear_level += comp.passive_wear_rate * delta_time;
        if (comp.wear_level > 100.0f) comp.wear_level = 100.0f;
    }

    // Recompute derived values
    comp.is_worn     = comp.wear_level >= 50.0f;
    comp.is_critical = comp.wear_level >= 85.0f;
    comp.fuel_inefficiency  = comp.wear_level / 200.0f;   // 0–0.5
    comp.repair_delay_mult  = 1.0f + comp.wear_level / 100.0f; // 1.0–2.0
    comp.crew_stress        = comp.wear_level;
}

bool OperationalWearSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::OperationalWearState>();
    entity->addComponent(std::move(comp));
    return true;
}

bool OperationalWearSystem::recordWear(const std::string& entity_id,
                                       const std::string& event_id,
                                       const std::string& event_type,
                                       float amount) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (event_id.empty()) return false;
    if (amount <= 0.0f) return false;

    comp->wear_level += amount;
    if (comp->wear_level > 100.0f) comp->wear_level = 100.0f;

    comp->is_worn     = comp->wear_level >= 50.0f;
    comp->is_critical = comp->wear_level >= 85.0f;
    comp->fuel_inefficiency = comp->wear_level / 200.0f;
    comp->repair_delay_mult = 1.0f + comp->wear_level / 100.0f;
    comp->crew_stress       = comp->wear_level;

    // Record event (auto-purge oldest)
    components::OperationalWearState::WearEvent ev;
    ev.event_id   = event_id;
    ev.event_type = event_type;
    ev.wear_amount = amount;
    if (static_cast<int>(comp->event_log.size()) >= comp->max_events) {
        comp->event_log.erase(comp->event_log.begin());
    }
    comp->event_log.push_back(ev);
    return true;
}

bool OperationalWearSystem::fieldRepair(const std::string& entity_id,
                                        float amount) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (amount <= 0.0f) return false;

    comp->wear_level -= amount;
    if (comp->wear_level < 0.0f) comp->wear_level = 0.0f;
    comp->has_hidden_penalties = true;
    ++comp->total_field_repairs;

    comp->is_worn     = comp->wear_level >= 50.0f;
    comp->is_critical = comp->wear_level >= 85.0f;
    comp->fuel_inefficiency = comp->wear_level / 200.0f;
    comp->repair_delay_mult = 1.0f + comp->wear_level / 100.0f;
    comp->crew_stress       = comp->wear_level;
    return true;
}

bool OperationalWearSystem::dockRepair(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;

    comp->wear_level           = 0.0f;
    comp->has_hidden_penalties = false;
    comp->is_worn              = false;
    comp->is_critical          = false;
    comp->fuel_inefficiency    = 0.0f;
    comp->repair_delay_mult    = 1.0f;
    comp->crew_stress          = 0.0f;
    ++comp->total_dock_repairs;
    return true;
}

bool OperationalWearSystem::setDocked(const std::string& entity_id,
                                      bool docked) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->is_docked = docked;
    return true;
}

bool OperationalWearSystem::setShipId(const std::string& entity_id,
                                      const std::string& ship_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (ship_id.empty()) return false;
    comp->ship_id = ship_id;
    return true;
}

bool OperationalWearSystem::setRotationThreshold(const std::string& entity_id,
                                                 float secs) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (secs < 0.0f) return false;
    comp->rotation_threshold = secs;
    return true;
}

bool OperationalWearSystem::setPassiveWearRate(const std::string& entity_id,
                                               float rate) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (rate < 0.0f) return false;
    comp->passive_wear_rate = rate;
    return true;
}

bool OperationalWearSystem::setRecoveryRate(const std::string& entity_id,
                                            float rate) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (rate < 0.0f) return false;
    comp->recovery_rate = rate;
    return true;
}

bool OperationalWearSystem::setMaxEvents(const std::string& entity_id,
                                         int max) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (max < 1) return false;
    comp->max_events = max;
    return true;
}

float OperationalWearSystem::getWearLevel(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    return comp->wear_level;
}

float OperationalWearSystem::getFuelInefficiency(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    return comp->fuel_inefficiency;
}

float OperationalWearSystem::getRepairDelayMult(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 1.0f;
    return comp->repair_delay_mult;
}

float OperationalWearSystem::getCrewStress(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    return comp->crew_stress;
}

bool OperationalWearSystem::isWorn(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    return comp->is_worn;
}

bool OperationalWearSystem::isCritical(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    return comp->is_critical;
}

bool OperationalWearSystem::hasHiddenPenalties(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    return comp->has_hidden_penalties;
}

float OperationalWearSystem::getDeploymentDuration(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    return comp->deployment_duration;
}

int OperationalWearSystem::getTotalFieldRepairs(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return comp->total_field_repairs;
}

int OperationalWearSystem::getTotalDockRepairs(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return comp->total_dock_repairs;
}

std::string OperationalWearSystem::getShipId(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return "";
    return comp->ship_id;
}

float OperationalWearSystem::getRotationThreshold(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    return comp->rotation_threshold;
}

float OperationalWearSystem::getPassiveWearRate(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    return comp->passive_wear_rate;
}

float OperationalWearSystem::getRecoveryRate(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    return comp->recovery_rate;
}

int OperationalWearSystem::getMaxEvents(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return comp->max_events;
}

int OperationalWearSystem::getEventCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return static_cast<int>(comp->event_log.size());
}

} // namespace systems
} // namespace atlas
