#include "systems/fleet_advertisement_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"

namespace atlas {
namespace systems {

FleetAdvertisementSystem::FleetAdvertisementSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void FleetAdvertisementSystem::updateComponent(
        ecs::Entity& /*entity*/,
        components::FleetAdvertisementState& comp,
        float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;

    // Countdown TTL for listed ads
    if (comp.is_listed && comp.time_remaining > 0.0f) {
        comp.time_remaining -= delta_time;
        if (comp.time_remaining <= 0.0f) {
            comp.time_remaining = 0.0f;
            comp.is_listed = false;
        }
    }
}

bool FleetAdvertisementSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::FleetAdvertisementState>();
    entity->addComponent(std::move(comp));
    return true;
}

// --- Ad management ---

bool FleetAdvertisementSystem::postAd(
        const std::string& entity_id,
        const std::string& title,
        const std::string& description,
        components::FleetAdvertisementState::FleetType type) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (title.empty()) return false;
    if (comp->is_listed) return false;  // already listed
    comp->title        = title;
    comp->description  = description;
    comp->fleet_type   = type;
    comp->is_listed    = true;
    comp->time_remaining = comp->ttl;
    ++comp->total_ads_posted;
    return true;
}

bool FleetAdvertisementSystem::delistAd(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (!comp->is_listed) return false;
    comp->is_listed = false;
    comp->time_remaining = 0.0f;
    return true;
}

// --- Configuration ---

bool FleetAdvertisementSystem::setFleetId(const std::string& entity_id,
                                          const std::string& fleet_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->fleet_id = fleet_id;
    return true;
}

bool FleetAdvertisementSystem::setBossName(const std::string& entity_id,
                                           const std::string& boss_name) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->boss_name = boss_name;
    return true;
}

bool FleetAdvertisementSystem::setTtl(const std::string& entity_id,
                                      float ttl) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (ttl <= 0.0f) return false;
    comp->ttl = ttl;
    return true;
}

bool FleetAdvertisementSystem::setMinMembers(const std::string& entity_id,
                                             int min_members) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (min_members < 1) return false;
    comp->min_members = min_members;
    return true;
}

bool FleetAdvertisementSystem::setMaxMembers(const std::string& entity_id,
                                             int max_members) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (max_members < 1) return false;
    comp->max_members = max_members;
    return true;
}

bool FleetAdvertisementSystem::setMaxApplications(
        const std::string& entity_id, int max_apps) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (max_apps <= 0) return false;
    comp->max_applications = max_apps;
    return true;
}

bool FleetAdvertisementSystem::setCurrentMembers(
        const std::string& entity_id, int count) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (count < 0) return false;
    comp->current_members = count;
    return true;
}

// --- Application management ---

bool FleetAdvertisementSystem::applyToFleet(
        const std::string& entity_id,
        const std::string& app_id,
        const std::string& pilot_name,
        const std::string& ship_type) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (app_id.empty()) return false;
    if (pilot_name.empty()) return false;
    if (!comp->is_listed) return false;

    // Duplicate prevention
    for (const auto& a : comp->applications) {
        if (a.app_id == app_id) return false;
    }
    // Capacity check
    if (static_cast<int>(comp->applications.size()) >= comp->max_applications)
        return false;

    components::FleetAdvertisementState::Application app;
    app.app_id     = app_id;
    app.pilot_name = pilot_name;
    app.ship_type  = ship_type;
    app.status     = components::FleetAdvertisementState::AppStatus::Pending;
    comp->applications.push_back(app);
    ++comp->total_applications_received;
    return true;
}

bool FleetAdvertisementSystem::acceptApplication(
        const std::string& entity_id,
        const std::string& app_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto& a : comp->applications) {
        if (a.app_id == app_id) {
            if (a.status != components::FleetAdvertisementState::AppStatus::Pending)
                return false;
            a.status = components::FleetAdvertisementState::AppStatus::Accepted;
            ++comp->total_accepted;
            ++comp->current_members;
            return true;
        }
    }
    return false;
}

bool FleetAdvertisementSystem::rejectApplication(
        const std::string& entity_id,
        const std::string& app_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto& a : comp->applications) {
        if (a.app_id == app_id) {
            if (a.status != components::FleetAdvertisementState::AppStatus::Pending)
                return false;
            a.status = components::FleetAdvertisementState::AppStatus::Rejected;
            ++comp->total_rejected;
            return true;
        }
    }
    return false;
}

bool FleetAdvertisementSystem::removeApplication(
        const std::string& entity_id,
        const std::string& app_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto it = comp->applications.begin();
         it != comp->applications.end(); ++it) {
        if (it->app_id == app_id) {
            comp->applications.erase(it);
            return true;
        }
    }
    return false;
}

bool FleetAdvertisementSystem::clearApplications(
        const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->applications.clear();
    return true;
}

// --- Queries ---

int FleetAdvertisementSystem::getApplicationCount(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return static_cast<int>(comp->applications.size());
}

int FleetAdvertisementSystem::getPendingCount(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    int count = 0;
    for (const auto& a : comp->applications) {
        if (a.status == components::FleetAdvertisementState::AppStatus::Pending)
            ++count;
    }
    return count;
}

bool FleetAdvertisementSystem::hasApplication(
        const std::string& entity_id,
        const std::string& app_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& a : comp->applications) {
        if (a.app_id == app_id) return true;
    }
    return false;
}

bool FleetAdvertisementSystem::isListed(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    return comp->is_listed;
}

float FleetAdvertisementSystem::getTimeRemaining(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    return comp->time_remaining;
}

std::string FleetAdvertisementSystem::getTitle(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return "";
    return comp->title;
}

std::string FleetAdvertisementSystem::getBossName(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return "";
    return comp->boss_name;
}

std::string FleetAdvertisementSystem::getFleetId(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return "";
    return comp->fleet_id;
}

int FleetAdvertisementSystem::getCurrentMembers(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return comp->current_members;
}

int FleetAdvertisementSystem::getMaxMembers(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return comp->max_members;
}

int FleetAdvertisementSystem::getTotalAdsPosted(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return comp->total_ads_posted;
}

int FleetAdvertisementSystem::getTotalApplicationsReceived(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return comp->total_applications_received;
}

int FleetAdvertisementSystem::getTotalAccepted(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return comp->total_accepted;
}

int FleetAdvertisementSystem::getTotalRejected(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return comp->total_rejected;
}

components::FleetAdvertisementState::FleetType
FleetAdvertisementSystem::getFleetType(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return components::FleetAdvertisementState::FleetType::PvE;
    return comp->fleet_type;
}

components::FleetAdvertisementState::AppStatus
FleetAdvertisementSystem::getApplicationStatus(
        const std::string& entity_id,
        const std::string& app_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return components::FleetAdvertisementState::AppStatus::Pending;
    for (const auto& a : comp->applications) {
        if (a.app_id == app_id) return a.status;
    }
    return components::FleetAdvertisementState::AppStatus::Pending;
}

} // namespace systems
} // namespace atlas
