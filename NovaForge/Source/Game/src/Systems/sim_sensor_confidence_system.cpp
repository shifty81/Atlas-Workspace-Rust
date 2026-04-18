#include "systems/sim_sensor_confidence_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

SimSensorConfidenceSystem::SimSensorConfidenceSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void SimSensorConfidenceSystem::updateComponent(
        ecs::Entity& /*entity*/,
        components::SensorConfidenceState& comp,
        float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;

    for (auto& entry : comp.entries) {
        entry.age_seconds += delta_time;
        entry.confidence -= comp.base_decay_rate * delta_time;
        if (entry.confidence < 0.0f) entry.confidence = 0.0f;
        if (entry.confidence > 1.0f) entry.confidence = 1.0f;
        if (entry.confidence < 0.05f) entry.is_decayed = true;
    }
}

bool SimSensorConfidenceSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::SensorConfidenceState>();
    entity->addComponent(std::move(comp));
    return true;
}

bool SimSensorConfidenceSystem::addEntry(const std::string& entity_id,
                                         const std::string& entry_id,
                                         const std::string& target_id,
                                         const std::string& ship_class_estimate,
                                         float confidence,
                                         float dist_min,
                                         float dist_max) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (entry_id.empty()) return false;
    if (target_id.empty()) return false;
    if (confidence < 0.0f || confidence > 1.0f) return false;
    if (dist_min < 0.0f || dist_max < 0.0f) return false;
    if (dist_min > dist_max) return false;
    if (static_cast<int>(comp->entries.size()) >= comp->max_entries) return false;

    // Reject duplicate entry_id
    for (const auto& e : comp->entries) {
        if (e.entry_id == entry_id) return false;
    }

    components::SensorConfidenceState::SensorEntry entry;
    entry.entry_id             = entry_id;
    entry.target_id            = target_id;
    entry.ship_class_estimate  = ship_class_estimate;
    entry.confidence           = confidence;
    entry.distance_min         = dist_min;
    entry.distance_max         = dist_max;
    entry.age_seconds          = 0.0f;
    entry.is_decayed           = false;

    if (confidence >= 0.9f) ++comp->total_high_confidence;
    comp->entries.push_back(entry);
    ++comp->total_entries_recorded;
    return true;
}

bool SimSensorConfidenceSystem::removeEntry(const std::string& entity_id,
                                            const std::string& entry_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    auto it = std::find_if(comp->entries.begin(), comp->entries.end(),
        [&](const components::SensorConfidenceState::SensorEntry& e) {
            return e.entry_id == entry_id;
        });
    if (it == comp->entries.end()) return false;
    comp->entries.erase(it);
    return true;
}

bool SimSensorConfidenceSystem::clearEntries(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->entries.clear();
    return true;
}

bool SimSensorConfidenceSystem::refreshEntry(const std::string& entity_id,
                                             const std::string& entry_id,
                                             float new_confidence) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (new_confidence < 0.0f || new_confidence > 1.0f) return false;
    for (auto& e : comp->entries) {
        if (e.entry_id == entry_id) {
            e.confidence  = new_confidence;
            e.age_seconds = 0.0f;
            e.is_decayed  = false;
            return true;
        }
    }
    return false;
}

bool SimSensorConfidenceSystem::setDecayRate(const std::string& entity_id,
                                             float rate) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (rate < 0.0f) return false;
    comp->base_decay_rate = rate;
    return true;
}

bool SimSensorConfidenceSystem::setMaxEntries(const std::string& entity_id,
                                              int max) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (max < 1) return false;
    comp->max_entries = max;
    return true;
}

bool SimSensorConfidenceSystem::setScannerID(const std::string& entity_id,
                                             const std::string& scanner_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (scanner_id.empty()) return false;
    comp->scanner_id = scanner_id;
    return true;
}

int SimSensorConfidenceSystem::getEntryCount(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return static_cast<int>(comp->entries.size());
}

bool SimSensorConfidenceSystem::hasEntry(const std::string& entity_id,
                                         const std::string& entry_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& e : comp->entries) {
        if (e.entry_id == entry_id) return true;
    }
    return false;
}

float SimSensorConfidenceSystem::getConfidence(
        const std::string& entity_id,
        const std::string& entry_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    for (const auto& e : comp->entries) {
        if (e.entry_id == entry_id) return e.confidence;
    }
    return 0.0f;
}

float SimSensorConfidenceSystem::getEntryAge(
        const std::string& entity_id,
        const std::string& entry_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    for (const auto& e : comp->entries) {
        if (e.entry_id == entry_id) return e.age_seconds;
    }
    return 0.0f;
}

bool SimSensorConfidenceSystem::isDecayed(
        const std::string& entity_id,
        const std::string& entry_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& e : comp->entries) {
        if (e.entry_id == entry_id) return e.is_decayed;
    }
    return false;
}

std::string SimSensorConfidenceSystem::getShipClassEstimate(
        const std::string& entity_id,
        const std::string& entry_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return "";
    for (const auto& e : comp->entries) {
        if (e.entry_id == entry_id) return e.ship_class_estimate;
    }
    return "";
}

float SimSensorConfidenceSystem::getDistanceMin(
        const std::string& entity_id,
        const std::string& entry_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    for (const auto& e : comp->entries) {
        if (e.entry_id == entry_id) return e.distance_min;
    }
    return 0.0f;
}

float SimSensorConfidenceSystem::getDistanceMax(
        const std::string& entity_id,
        const std::string& entry_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    for (const auto& e : comp->entries) {
        if (e.entry_id == entry_id) return e.distance_max;
    }
    return 0.0f;
}

int SimSensorConfidenceSystem::getActiveEntryCount(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    int count = 0;
    for (const auto& e : comp->entries) {
        if (!e.is_decayed) ++count;
    }
    return count;
}

int SimSensorConfidenceSystem::getTotalEntriesRecorded(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return comp->total_entries_recorded;
}

int SimSensorConfidenceSystem::getTotalHighConfidence(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return comp->total_high_confidence;
}

std::string SimSensorConfidenceSystem::getScannerID(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return "";
    return comp->scanner_id;
}

float SimSensorConfidenceSystem::getDecayRate(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    return comp->base_decay_rate;
}

int SimSensorConfidenceSystem::getMaxEntries(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return comp->max_entries;
}

} // namespace systems
} // namespace atlas
