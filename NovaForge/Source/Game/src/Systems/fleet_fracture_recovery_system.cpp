#include "systems/fleet_fracture_recovery_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

namespace {

void applyPhaseTransitions(components::FleetFractureRecoveryState& comp) {
    using P = components::FleetFractureRecoveryState::RecoveryPhase;
    bool changed = true;
    while (changed) {
        changed = false;
        switch (comp.phase) {
            case P::Stable:
                if (comp.fracture_score >= comp.cracking_threshold) {
                    comp.phase = P::Cracking;
                    changed = true;
                }
                break;
            case P::Cracking:
                if (comp.fracture_score >= comp.fracture_threshold) {
                    comp.phase = P::Fractured;
                    ++comp.total_fractures;
                    changed = true;
                } else if (comp.fracture_score < comp.cracking_threshold) {
                    comp.phase = P::Stable;
                    changed = true;
                }
                break;
            case P::Fractured:
                if (comp.fracture_score < comp.fracture_threshold) {
                    comp.phase = P::Recovering;
                    changed = true;
                }
                break;
            case P::Recovering:
                if (comp.fracture_score <= comp.recovery_threshold) {
                    comp.phase = P::Rebuilt;
                    ++comp.total_recoveries;
                    changed = true;
                }
                break;
            case P::Rebuilt:
                if (comp.fracture_score == 0.0f) {
                    comp.phase = P::Stable;
                    changed = true;
                }
                break;
        }
    }
}

} // anonymous namespace

FleetFractureRecoverySystem::FleetFractureRecoverySystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void FleetFractureRecoverySystem::updateComponent(
        ecs::Entity& /*entity*/,
        components::FleetFractureRecoveryState& comp,
        float delta_time) {
    if (!comp.active) return;
    using P = components::FleetFractureRecoveryState::RecoveryPhase;
    if (comp.recovery_momentum > 0.0f &&
        (comp.phase == P::Recovering || comp.phase == P::Cracking)) {
        comp.fracture_score -= comp.recovery_momentum * delta_time;
        if (comp.fracture_score < 0.0f) comp.fracture_score = 0.0f;
        applyPhaseTransitions(comp);
    }
    comp.elapsed += delta_time;
}

bool FleetFractureRecoverySystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::FleetFractureRecoveryState>();
    entity->addComponent(std::move(comp));
    return true;
}

bool FleetFractureRecoverySystem::recordFractureEvent(
        const std::string& entity_id,
        const std::string& event_id,
        const std::string& description,
        int ships_lost,
        int captains_departed,
        float morale_delta,
        float severity) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (event_id.empty()) return false;
    if (description.empty()) return false;
    if (ships_lost < 0) return false;
    if (captains_departed < 0) return false;
    if (severity < 0.0f || severity > 1.0f) return false;

    // Auto-purge oldest if at capacity
    if (static_cast<int>(comp->fracture_log.size()) >= comp->max_log) {
        comp->fracture_log.erase(comp->fracture_log.begin());
    }

    components::FleetFractureRecoveryState::FractureEvent ev;
    ev.event_id          = event_id;
    ev.description       = description;
    ev.ships_lost        = ships_lost;
    ev.captains_departed = captains_departed;
    ev.morale_delta      = morale_delta;
    ev.severity          = severity;
    comp->fracture_log.push_back(ev);

    comp->fracture_score += severity * 40.0f * (1.0f + comp->fragility);
    if (comp->fracture_score > 100.0f) comp->fracture_score = 100.0f;
    if (comp->fracture_score < 0.0f)   comp->fracture_score = 0.0f;

    comp->total_captains_lost += captains_departed;
    comp->total_ships_lost    += ships_lost;

    applyPhaseTransitions(*comp);
    return true;
}

bool FleetFractureRecoverySystem::applyRecovery(const std::string& entity_id,
                                                 float amount) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (amount <= 0.0f) return false;
    comp->fracture_score -= amount;
    if (comp->fracture_score < 0.0f) comp->fracture_score = 0.0f;
    applyPhaseTransitions(*comp);
    return true;
}

bool FleetFractureRecoverySystem::setRecoveryMomentum(
        const std::string& entity_id, float rate) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (rate < 0.0f || rate > 1.0f) return false;
    comp->recovery_momentum = rate;
    return true;
}

bool FleetFractureRecoverySystem::setFragility(const std::string& entity_id,
                                                float fragility) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (fragility < 0.0f || fragility > 1.0f) return false;
    comp->fragility = fragility;
    return true;
}

bool FleetFractureRecoverySystem::addMilestone(const std::string& entity_id,
                                                const std::string& milestone_id,
                                                const std::string& description,
                                                float required_value) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (milestone_id.empty()) return false;
    if (description.empty()) return false;
    if (required_value <= 0.0f) return false;
    if (static_cast<int>(comp->milestones.size()) >= comp->max_milestones) return false;
    for (const auto& m : comp->milestones) {
        if (m.milestone_id == milestone_id) return false;
    }
    components::FleetFractureRecoveryState::RecoveryMilestone ms;
    ms.milestone_id    = milestone_id;
    ms.description     = description;
    ms.required_value  = required_value;
    ms.current_value   = 0.0f;
    ms.is_achieved     = false;
    comp->milestones.push_back(ms);
    return true;
}

bool FleetFractureRecoverySystem::progressMilestone(const std::string& entity_id,
                                                     const std::string& milestone_id,
                                                     float current_value) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto& m : comp->milestones) {
        if (m.milestone_id == milestone_id) {
            m.current_value = current_value;
            if (m.current_value >= m.required_value) {
                m.is_achieved = true;
            }
            return true;
        }
    }
    return false;
}

bool FleetFractureRecoverySystem::clearFractureLog(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->fracture_log.clear();
    return true;
}

bool FleetFractureRecoverySystem::setFleetId(const std::string& entity_id,
                                              const std::string& fleet_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (fleet_id.empty()) return false;
    comp->fleet_id = fleet_id;
    return true;
}

bool FleetFractureRecoverySystem::setMaxLog(const std::string& entity_id,
                                             int max) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (max < 1) return false;
    comp->max_log = max;
    return true;
}

bool FleetFractureRecoverySystem::setFractureThreshold(
        const std::string& entity_id, float value) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (value <= 0.0f || value >= 100.0f) return false;
    comp->fracture_threshold = value;
    return true;
}

bool FleetFractureRecoverySystem::setRecoveryThreshold(
        const std::string& entity_id, float value) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (value < 0.0f) return false;
    comp->recovery_threshold = value;
    return true;
}

components::FleetFractureRecoveryState::RecoveryPhase
FleetFractureRecoverySystem::getPhase(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return components::FleetFractureRecoveryState::RecoveryPhase::Stable;
    return comp->phase;
}

float FleetFractureRecoverySystem::getFractureScore(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    return comp->fracture_score;
}

float FleetFractureRecoverySystem::getRecoveryMomentum(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    return comp->recovery_momentum;
}

float FleetFractureRecoverySystem::getFragility(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    return comp->fragility;
}

int FleetFractureRecoverySystem::getFractureLogCount(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return static_cast<int>(comp->fracture_log.size());
}

int FleetFractureRecoverySystem::getMilestoneCount(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return static_cast<int>(comp->milestones.size());
}

int FleetFractureRecoverySystem::getMilestoneAchievedCount(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    int count = 0;
    for (const auto& m : comp->milestones) {
        if (m.is_achieved) ++count;
    }
    return count;
}

bool FleetFractureRecoverySystem::isMilestoneAchieved(
        const std::string& entity_id,
        const std::string& milestone_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& m : comp->milestones) {
        if (m.milestone_id == milestone_id) return m.is_achieved;
    }
    return false;
}

int FleetFractureRecoverySystem::getTotalFractures(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return comp->total_fractures;
}

int FleetFractureRecoverySystem::getTotalRecoveries(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return comp->total_recoveries;
}

int FleetFractureRecoverySystem::getTotalCaptainsLost(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return comp->total_captains_lost;
}

int FleetFractureRecoverySystem::getTotalShipsLost(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return comp->total_ships_lost;
}

std::string FleetFractureRecoverySystem::getFleetId(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return "";
    return comp->fleet_id;
}

bool FleetFractureRecoverySystem::isFractured(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    return comp->phase == components::FleetFractureRecoveryState::RecoveryPhase::Fractured;
}

bool FleetFractureRecoverySystem::isRecovering(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    using P = components::FleetFractureRecoveryState::RecoveryPhase;
    return comp->phase == P::Recovering || comp->phase == P::Fractured;
}

std::string FleetFractureRecoverySystem::getPhaseString(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return "";
    using P = components::FleetFractureRecoveryState::RecoveryPhase;
    switch (comp->phase) {
        case P::Stable:     return "Stable";
        case P::Cracking:   return "Cracking";
        case P::Fractured:  return "Fractured";
        case P::Recovering: return "Recovering";
        case P::Rebuilt:    return "Rebuilt";
    }
    return "";
}

} // namespace systems
} // namespace atlas
