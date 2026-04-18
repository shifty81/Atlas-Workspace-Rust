#include "systems/fleet_engagement_rules_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"

namespace atlas {
namespace systems {

FleetEngagementRulesSystem::FleetEngagementRulesSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void FleetEngagementRulesSystem::updateComponent(
        ecs::Entity& /*entity*/,
        components::FleetEngagementRulesState& comp,
        float delta_time) {
    if (!comp.active) return;
    if (!comp.in_combat) {
        comp.time_since_last_engagement += delta_time;
    }
    comp.elapsed += delta_time;
}

bool FleetEngagementRulesSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::FleetEngagementRulesState>();
    entity->addComponent(std::move(comp));
    return true;
}

bool FleetEngagementRulesSystem::setRoe(
        const std::string& entity_id,
        components::FleetEngagementRulesState::RoeProfile roe) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->roe = roe;
    if (roe == components::FleetEngagementRulesState::RoeProfile::Passive) {
        comp->cease_fire = true;
        comp->all_hands_fire = false;
    }
    return true;
}

bool FleetEngagementRulesSystem::setPrimaryTarget(
        const std::string& entity_id,
        components::FleetEngagementRulesState::TargetPriority priority) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->primary_target = priority;
    return true;
}

bool FleetEngagementRulesSystem::setAutoEngageHostiles(
        const std::string& entity_id, bool value) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->auto_engage_hostiles = value;
    return true;
}

bool FleetEngagementRulesSystem::setAutoEngageNeutrals(
        const std::string& entity_id, bool value) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (value && comp->roe == components::FleetEngagementRulesState::RoeProfile::Passive)
        return false;
    comp->auto_engage_neutrals = value;
    return true;
}

bool FleetEngagementRulesSystem::setRangeLimit(
        const std::string& entity_id, float range) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (range < 0.0f) return false;
    comp->range_limit = range;
    return true;
}

bool FleetEngagementRulesSystem::broadcastTarget(
        const std::string& entity_id, const std::string& target_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->broadcast_target = target_id;
    return true;
}

bool FleetEngagementRulesSystem::allHandsFire(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->all_hands_fire = true;
    comp->cease_fire = false;
    comp->in_combat = true;
    ++comp->total_engagements;
    return true;
}

bool FleetEngagementRulesSystem::ceaseFire(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->cease_fire = true;
    comp->all_hands_fire = false;
    return true;
}

bool FleetEngagementRulesSystem::setInCombat(
        const std::string& entity_id, bool in_combat) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->in_combat = in_combat;
    if (in_combat) {
        ++comp->total_engagements;
        comp->time_since_last_engagement = 0.0f;
    } else {
        ++comp->total_disengages;
    }
    return true;
}

bool FleetEngagementRulesSystem::setFleetId(
        const std::string& entity_id, const std::string& fleet_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (fleet_id.empty()) return false;
    comp->fleet_id = fleet_id;
    return true;
}

components::FleetEngagementRulesState::RoeProfile
FleetEngagementRulesSystem::getRoe(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return components::FleetEngagementRulesState::RoeProfile::Defensive;
    return comp->roe;
}

components::FleetEngagementRulesState::TargetPriority
FleetEngagementRulesSystem::getPrimaryTarget(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return components::FleetEngagementRulesState::TargetPriority::Any;
    return comp->primary_target;
}

bool FleetEngagementRulesSystem::isAutoEngageHostiles(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    return comp->auto_engage_hostiles;
}

bool FleetEngagementRulesSystem::isAutoEngageNeutrals(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    return comp->auto_engage_neutrals;
}

float FleetEngagementRulesSystem::getRangeLimit(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    return comp->range_limit;
}

std::string FleetEngagementRulesSystem::getBroadcastTarget(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return "";
    return comp->broadcast_target;
}

bool FleetEngagementRulesSystem::isAllHandsFire(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    return comp->all_hands_fire;
}

bool FleetEngagementRulesSystem::isCeaseFire(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    return comp->cease_fire;
}

bool FleetEngagementRulesSystem::isInCombat(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    return comp->in_combat;
}

int FleetEngagementRulesSystem::getTotalEngagements(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return comp->total_engagements;
}

int FleetEngagementRulesSystem::getTotalDisengages(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return comp->total_disengages;
}

float FleetEngagementRulesSystem::getTimeSinceLastEngagement(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    return comp->time_since_last_engagement;
}

std::string FleetEngagementRulesSystem::getFleetId(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return "";
    return comp->fleet_id;
}

std::string FleetEngagementRulesSystem::getRoeString(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return "";
    using R = components::FleetEngagementRulesState::RoeProfile;
    switch (comp->roe) {
        case R::Passive:    return "Passive";
        case R::Defensive:  return "Defensive";
        case R::Aggressive: return "Aggressive";
        case R::Skirmish:   return "Skirmish";
        case R::Doctrine:   return "Doctrine";
    }
    return "";
}

} // namespace systems
} // namespace atlas
