#include "systems/fleet_norm_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

FleetNormSystem::FleetNormSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void FleetNormSystem::updateComponent(
        ecs::Entity& /*entity*/,
        components::FleetNormState& comp,
        float delta_time) {
    comp.elapsed += delta_time;
    // Passive tick: nothing to decay — norms strengthen over time and
    // are only removed by explicit removeNorm / clearNorms calls.
}

bool FleetNormSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::FleetNormState>();
    entity->addComponent(std::move(comp));
    return true;
}

bool FleetNormSystem::addNorm(const std::string& entity_id,
                               const std::string& norm_id,
                               const std::string& name,
                               const std::string& trigger_action) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (norm_id.empty() || name.empty() || trigger_action.empty())
        return false;
    if (static_cast<int>(comp->norms.size()) >= comp->max_norms) return false;
    for (const auto& n : comp->norms)
        if (n.norm_id == norm_id) return false;

    components::FleetNormState::Norm n;
    n.norm_id       = norm_id;
    n.name          = name;
    n.trigger_action = trigger_action;
    comp->norms.push_back(n);
    return true;
}

bool FleetNormSystem::removeNorm(const std::string& entity_id,
                                  const std::string& norm_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    auto it = std::find_if(comp->norms.begin(), comp->norms.end(),
        [&](const auto& n) { return n.norm_id == norm_id; });
    if (it == comp->norms.end()) return false;
    comp->norms.erase(it);
    return true;
}

bool FleetNormSystem::clearNorms(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->norms.clear();
    return true;
}

bool FleetNormSystem::recordAction(const std::string& entity_id,
                                    const std::string& trigger_action) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (trigger_action.empty()) return false;
    bool found = false;
    for (auto& n : comp->norms) {
        if (n.trigger_action == trigger_action) {
            found = true;
            ++n.activation_count;
            // Strength grows as a fraction of threshold up to 1.0
            n.strength = std::min(
                1.0f,
                static_cast<float>(n.activation_count) /
                    static_cast<float>(comp->activation_threshold));
            if (!n.active &&
                n.activation_count >= comp->activation_threshold) {
                n.active = true;
                ++comp->total_norms_formed;
            }
        }
    }
    return found;
}

bool FleetNormSystem::activateNorm(const std::string& entity_id,
                                    const std::string& norm_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto& n : comp->norms) {
        if (n.norm_id == norm_id) {
            if (n.active) return false;
            n.active = true;
            ++comp->total_norms_formed;
            return true;
        }
    }
    return false;
}

bool FleetNormSystem::deactivateNorm(const std::string& entity_id,
                                      const std::string& norm_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto& n : comp->norms) {
        if (n.norm_id == norm_id) {
            if (!n.active) return false;
            n.active = false;
            return true;
        }
    }
    return false;
}

bool FleetNormSystem::setActivationThreshold(const std::string& entity_id,
                                              int threshold) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (threshold < 1) return false;
    comp->activation_threshold = threshold;
    return true;
}

bool FleetNormSystem::setMaxNorms(const std::string& entity_id, int max_norms) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (max_norms < 1) return false;
    comp->max_norms = max_norms;
    return true;
}

bool FleetNormSystem::setFleetId(const std::string& entity_id,
                                  const std::string& fleet_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (fleet_id.empty()) return false;
    comp->fleet_id = fleet_id;
    return true;
}

int FleetNormSystem::getNormCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return static_cast<int>(comp->norms.size());
}

int FleetNormSystem::getActiveNormCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    int count = 0;
    for (const auto& n : comp->norms)
        if (n.active) ++count;
    return count;
}

bool FleetNormSystem::isNormActive(const std::string& entity_id,
                                    const std::string& norm_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& n : comp->norms)
        if (n.norm_id == norm_id) return n.active;
    return false;
}

float FleetNormSystem::getNormStrength(const std::string& entity_id,
                                       const std::string& norm_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    for (const auto& n : comp->norms)
        if (n.norm_id == norm_id) return n.strength;
    return 0.0f;
}

bool FleetNormSystem::hasNorm(const std::string& entity_id,
                               const std::string& norm_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& n : comp->norms)
        if (n.norm_id == norm_id) return true;
    return false;
}

int FleetNormSystem::getTotalNormsFormed(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return comp->total_norms_formed;
}

int FleetNormSystem::getCountByTrigger(
        const std::string& entity_id,
        const std::string& trigger_action) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    int count = 0;
    for (const auto& n : comp->norms)
        if (n.trigger_action == trigger_action) ++count;
    return count;
}

int FleetNormSystem::getActivationThreshold(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return comp->activation_threshold;
}

std::string FleetNormSystem::getFleetId(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return "";
    return comp->fleet_id;
}

} // namespace systems
} // namespace atlas
