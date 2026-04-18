#include "systems/fleet_departure_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>
#include <cmath>

namespace atlas {
namespace systems {

FleetDepartureSystem::FleetDepartureSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

// ---------------------------------------------------------------------------
// Risk formula:
//   risk = clamp(losing_streak * 0.10 + near_deaths * 0.20
//                + max(0, (-morale - 30) / 100), 0, 1)
// ---------------------------------------------------------------------------
void FleetDepartureSystem::recomputeRisk(
        components::FleetDepartureState& comp) const {
    float r = comp.losing_streak * 0.10f
            + comp.consecutive_near_deaths * 0.20f
            + std::max(0.0f, (-comp.morale - 30.0f) / 100.0f);
    comp.departure_risk = std::clamp(r, 0.0f, 1.0f);
}

void FleetDepartureSystem::updateComponent(
        ecs::Entity& /*entity*/,
        components::FleetDepartureState& comp,
        float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;
    comp.time_in_stage += delta_time;

    // Auto-trigger Arguing when risk exceeds threshold and still Stable
    if (comp.stage == components::FleetDepartureState::DepartureStage::Stable
            && comp.departure_risk >= comp.departure_threshold) {
        comp.stage = components::FleetDepartureState::DepartureStage::Arguing;
        comp.time_in_stage = 0.0f;
    }
}

bool FleetDepartureSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::FleetDepartureState>();
    entity->addComponent(std::move(comp));
    return true;
}

bool FleetDepartureSystem::recordNearDeath(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    ++comp->consecutive_near_deaths;
    recomputeRisk(*comp);
    return true;
}

bool FleetDepartureSystem::recordLoss(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    ++comp->losing_streak;
    recomputeRisk(*comp);
    return true;
}

bool FleetDepartureSystem::recordWin(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->losing_streak = 0;
    comp->consecutive_near_deaths = 0;
    recomputeRisk(*comp);
    return true;
}

bool FleetDepartureSystem::setMorale(const std::string& entity_id, float morale) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->morale = std::clamp(morale, -100.0f, 100.0f);
    recomputeRisk(*comp);
    return true;
}

bool FleetDepartureSystem::requestTransfer(const std::string& entity_id,
                                            const std::string& reason) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    using Stage = components::FleetDepartureState::DepartureStage;
    if (comp->stage != Stage::Arguing) return false;
    comp->stage = Stage::Requesting;
    comp->has_departure_request = true;
    comp->departure_reason = reason;
    comp->time_in_stage = 0.0f;
    return true;
}

bool FleetDepartureSystem::acceptTransfer(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    using Stage = components::FleetDepartureState::DepartureStage;
    if (comp->stage != Stage::Requesting) return false;
    comp->stage = Stage::Departed;
    ++comp->total_transfers;
    comp->time_in_stage = 0.0f;
    return true;
}

bool FleetDepartureSystem::resolveConflict(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    using Stage = components::FleetDepartureState::DepartureStage;
    if (comp->stage != Stage::Arguing && comp->stage != Stage::Requesting) return false;
    comp->stage = Stage::Stable;
    comp->has_departure_request = false;
    comp->departure_reason.clear();
    comp->departure_risk = 0.0f;
    comp->losing_streak = 0;
    comp->consecutive_near_deaths = 0;
    comp->morale = std::max(comp->morale, 0.0f);
    comp->time_in_stage = 0.0f;
    return true;
}

bool FleetDepartureSystem::resetStreak(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->losing_streak = 0;
    comp->consecutive_near_deaths = 0;
    recomputeRisk(*comp);
    return true;
}

bool FleetDepartureSystem::setDepartureThreshold(const std::string& entity_id,
                                                   float threshold) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (threshold < 0.0f || threshold > 1.0f) return false;
    comp->departure_threshold = threshold;
    return true;
}

bool FleetDepartureSystem::setCaptainId(const std::string& entity_id,
                                         const std::string& captain_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (captain_id.empty()) return false;
    comp->captain_id = captain_id;
    return true;
}

bool FleetDepartureSystem::setFleetId(const std::string& entity_id,
                                       const std::string& fleet_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (fleet_id.empty()) return false;
    comp->fleet_id = fleet_id;
    return true;
}

components::FleetDepartureState::DepartureStage
FleetDepartureSystem::getStage(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return components::FleetDepartureState::DepartureStage::Stable;
    return comp->stage;
}

float FleetDepartureSystem::getDepartureRisk(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->departure_risk : 0.0f;
}

float FleetDepartureSystem::getMorale(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->morale : 0.0f;
}

int FleetDepartureSystem::getLosingStreak(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->losing_streak : 0;
}

int FleetDepartureSystem::getConsecutiveNearDeaths(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->consecutive_near_deaths : 0;
}

bool FleetDepartureSystem::hasDepartureRequest(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->has_departure_request : false;
}

std::string FleetDepartureSystem::getDepartureReason(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->departure_reason : "";
}

float FleetDepartureSystem::getTimeInStage(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->time_in_stage : 0.0f;
}

int FleetDepartureSystem::getTotalDepartures(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_departures : 0;
}

int FleetDepartureSystem::getTotalTransfers(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_transfers : 0;
}

bool FleetDepartureSystem::isDepartureRisk(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    using Stage = components::FleetDepartureState::DepartureStage;
    return comp->stage == Stage::Arguing
        || comp->stage == Stage::Requesting
        || comp->stage == Stage::Transferring;
}

bool FleetDepartureSystem::isGone(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    return comp->stage == components::FleetDepartureState::DepartureStage::Departed;
}

std::string FleetDepartureSystem::getStageString(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return "";
    using Stage = components::FleetDepartureState::DepartureStage;
    switch (comp->stage) {
        case Stage::Stable:       return "Stable";
        case Stage::Arguing:      return "Arguing";
        case Stage::Requesting:   return "Requesting";
        case Stage::Transferring: return "Transferring";
        case Stage::Departed:     return "Departed";
    }
    return "Unknown";
}

std::string FleetDepartureSystem::getCaptainId(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->captain_id : "";
}

std::string FleetDepartureSystem::getFleetId(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->fleet_id : "";
}

} // namespace systems
} // namespace atlas
