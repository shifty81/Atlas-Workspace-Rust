#include "systems/directional_scan_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"

namespace atlas {
namespace systems {

DirectionalScanSystem::DirectionalScanSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void DirectionalScanSystem::updateComponent(
        ecs::Entity& /*entity*/,
        components::DirectionalScanState& comp,
        float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;

    // Advance cooldown
    if (comp.cooldown_remaining > 0.0f) {
        comp.cooldown_remaining -= delta_time;
        if (comp.cooldown_remaining < 0.0f) comp.cooldown_remaining = 0.0f;
    }

    // Advance scan timer
    if (comp.status == components::DirectionalScanState::ScanStatus::Scanning) {
        comp.scan_timer += delta_time;
        if (comp.scan_timer >= comp.scan_duration) {
            comp.status = components::DirectionalScanState::ScanStatus::Complete;
            comp.scan_timer = comp.scan_duration;
            ++comp.total_scans;
            comp.cooldown_remaining = comp.cooldown_duration;
        }
    }
}

bool DirectionalScanSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::DirectionalScanState>();
    entity->addComponent(std::move(comp));
    return true;
}

// --- Scan control ---

bool DirectionalScanSystem::startScan(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (comp->status == components::DirectionalScanState::ScanStatus::Scanning)
        return false;
    if (comp->cooldown_remaining > 0.0f) return false;
    comp->status = components::DirectionalScanState::ScanStatus::Scanning;
    comp->scan_timer = 0.0f;
    comp->results.clear();
    return true;
}

bool DirectionalScanSystem::cancelScan(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (comp->status != components::DirectionalScanState::ScanStatus::Scanning)
        return false;
    comp->status = components::DirectionalScanState::ScanStatus::Idle;
    comp->scan_timer = 0.0f;
    return true;
}

// --- Configuration ---

bool DirectionalScanSystem::setScanAngle(const std::string& entity_id,
                                         float angle_degrees) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (angle_degrees < 5.0f || angle_degrees > 360.0f) return false;
    comp->scan_angle = angle_degrees;
    return true;
}

bool DirectionalScanSystem::setScanRange(const std::string& entity_id,
                                         float range_au) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (range_au <= 0.0f) return false;
    comp->scan_range = range_au;
    return true;
}

bool DirectionalScanSystem::setScanDuration(const std::string& entity_id,
                                            float duration) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (duration <= 0.0f) return false;
    comp->scan_duration = duration;
    return true;
}

bool DirectionalScanSystem::setCooldownDuration(const std::string& entity_id,
                                                float duration) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (duration < 0.0f) return false;
    comp->cooldown_duration = duration;
    return true;
}

bool DirectionalScanSystem::setMaxResults(const std::string& entity_id,
                                          int max_results) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (max_results <= 0) return false;
    comp->max_results = max_results;
    return true;
}

// --- Result management ---

bool DirectionalScanSystem::addResult(
        const std::string& entity_id,
        const std::string& result_id,
        const std::string& object_name,
        components::DirectionalScanState::ObjectType type,
        float distance,
        float bearing) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (result_id.empty()) return false;
    if (distance < 0.0f) return false;
    if (bearing < 0.0f || bearing > 360.0f) return false;

    // Duplicate prevention
    for (const auto& r : comp->results) {
        if (r.result_id == result_id) return false;
    }
    // Capacity check
    if (static_cast<int>(comp->results.size()) >= comp->max_results)
        return false;

    components::DirectionalScanState::ScanResult result;
    result.result_id   = result_id;
    result.object_name = object_name;
    result.object_type = type;
    result.distance    = distance;
    result.bearing     = bearing;
    comp->results.push_back(result);
    ++comp->total_objects_found;
    return true;
}

bool DirectionalScanSystem::removeResult(const std::string& entity_id,
                                         const std::string& result_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto it = comp->results.begin(); it != comp->results.end(); ++it) {
        if (it->result_id == result_id) {
            comp->results.erase(it);
            return true;
        }
    }
    return false;
}

bool DirectionalScanSystem::clearResults(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->results.clear();
    return true;
}

// --- Queries ---

int DirectionalScanSystem::getResultCount(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return static_cast<int>(comp->results.size());
}

bool DirectionalScanSystem::hasResult(const std::string& entity_id,
                                      const std::string& result_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& r : comp->results) {
        if (r.result_id == result_id) return true;
    }
    return false;
}

float DirectionalScanSystem::getScanAngle(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    return comp->scan_angle;
}

float DirectionalScanSystem::getScanRange(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    return comp->scan_range;
}

float DirectionalScanSystem::getScanProgress(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    if (comp->scan_duration <= 0.0f) return 0.0f;
    if (comp->status == components::DirectionalScanState::ScanStatus::Complete)
        return 1.0f;
    if (comp->status != components::DirectionalScanState::ScanStatus::Scanning)
        return 0.0f;
    return comp->scan_timer / comp->scan_duration;
}

float DirectionalScanSystem::getCooldownRemaining(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    return comp->cooldown_remaining;
}

bool DirectionalScanSystem::isScanning(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    return comp->status == components::DirectionalScanState::ScanStatus::Scanning;
}

bool DirectionalScanSystem::isScanComplete(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    return comp->status == components::DirectionalScanState::ScanStatus::Complete;
}

bool DirectionalScanSystem::isOnCooldown(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    return comp->cooldown_remaining > 0.0f;
}

int DirectionalScanSystem::getTotalScans(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return comp->total_scans;
}

int DirectionalScanSystem::getTotalObjectsFound(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return comp->total_objects_found;
}

float DirectionalScanSystem::getResultDistance(
        const std::string& entity_id,
        const std::string& result_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    for (const auto& r : comp->results) {
        if (r.result_id == result_id) return r.distance;
    }
    return 0.0f;
}

float DirectionalScanSystem::getResultBearing(
        const std::string& entity_id,
        const std::string& result_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    for (const auto& r : comp->results) {
        if (r.result_id == result_id) return r.bearing;
    }
    return 0.0f;
}

int DirectionalScanSystem::getCountByType(
        const std::string& entity_id,
        components::DirectionalScanState::ObjectType type) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    int count = 0;
    for (const auto& r : comp->results) {
        if (r.object_type == type) ++count;
    }
    return count;
}

} // namespace systems
} // namespace atlas
