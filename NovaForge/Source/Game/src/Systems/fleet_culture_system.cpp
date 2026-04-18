#include "systems/fleet_culture_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

FleetCultureSystem::FleetCultureSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void FleetCultureSystem::updateComponent(
        ecs::Entity& /*entity*/,
        components::FleetCultureState& comp,
        float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;
    // Tension decays naturally
    if (comp.tension_level > 0.0f) {
        comp.tension_level = std::max(0.0f,
            comp.tension_level - comp.tension_decay_rate * delta_time);
    }
    recomputeCohesionBonus(comp);
}

void FleetCultureSystem::recomputeCohesionBonus(components::FleetCultureState& comp) {
    float bonus = 0.0f;
    for (const auto& e : comp.elements) {
        if (e.is_active) {
            bonus += e.strength * 0.1f; // each element contributes up to 0.1
        }
    }
    comp.cohesion_bonus = std::min(1.0f, bonus);
}

bool FleetCultureSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::FleetCultureState>();
    entity->addComponent(std::move(comp));
    return true;
}

bool FleetCultureSystem::addElement(const std::string& entity_id,
                                     const std::string& element_id,
                                     const std::string& name,
                                     components::CultureElementType type,
                                     const std::string& description) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (element_id.empty() || name.empty()) return false;
    // No duplicate ids
    for (const auto& e : comp->elements) {
        if (e.element_id == element_id) return false;
    }
    // Enforce capacity
    if (static_cast<int>(comp->elements.size()) >= comp->max_elements) return false;
    components::CultureElement elem;
    elem.element_id   = element_id;
    elem.name         = name;
    elem.type         = type;
    elem.description  = description;
    elem.is_active    = true;
    comp->elements.push_back(elem);
    ++comp->total_elements_formed;
    return true;
}

bool FleetCultureSystem::removeElement(const std::string& entity_id,
                                        const std::string& element_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    auto it = std::find_if(comp->elements.begin(), comp->elements.end(),
        [&](const components::CultureElement& e){ return e.element_id == element_id; });
    if (it == comp->elements.end()) return false;
    comp->elements.erase(it);
    recomputeCohesionBonus(*comp);
    return true;
}

bool FleetCultureSystem::clearElements(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->elements.clear();
    comp->cohesion_bonus = 0.0f;
    return true;
}

bool FleetCultureSystem::reinforce(const std::string& entity_id,
                                    const std::string& element_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto& e : comp->elements) {
        if (e.element_id == element_id && e.is_active) {
            ++e.reinforcement_count;
            ++comp->total_reinforcements;
            // Strength grows with each reinforcement (asymptotic toward 1)
            e.strength = std::min(1.0f, e.strength + 0.1f);
            recomputeCohesionBonus(*comp);
            return true;
        }
    }
    return false;
}

bool FleetCultureSystem::violate(const std::string& entity_id,
                                  const std::string& element_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto& e : comp->elements) {
        if (e.element_id == element_id && e.is_active) {
            ++e.violation_count;
            ++comp->total_violations;
            // Violation weakens element strength
            e.strength = std::max(0.0f, e.strength - 0.05f);
            // Violations raise fleet tension
            comp->tension_level = std::min(1.0f, comp->tension_level + 0.15f);
            recomputeCohesionBonus(*comp);
            return true;
        }
    }
    return false;
}

bool FleetCultureSystem::deactivateElement(const std::string& entity_id,
                                            const std::string& element_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto& e : comp->elements) {
        if (e.element_id == element_id) {
            if (!e.is_active) return false; // already inactive
            e.is_active = false;
            recomputeCohesionBonus(*comp);
            return true;
        }
    }
    return false;
}

bool FleetCultureSystem::setFleetId(const std::string& entity_id,
                                     const std::string& fleet_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (fleet_id.empty()) return false;
    comp->fleet_id = fleet_id;
    return true;
}

bool FleetCultureSystem::setMaxElements(const std::string& entity_id, int max) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (max < 1) return false;
    comp->max_elements = max;
    return true;
}

bool FleetCultureSystem::setTensionDecayRate(const std::string& entity_id, float rate) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (rate < 0.0f) return false;
    comp->tension_decay_rate = rate;
    return true;
}

// ── Queries ──────────────────────────────────────────────────────────────────

int FleetCultureSystem::getElementCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->elements.size()) : 0;
}

int FleetCultureSystem::getActiveElementCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    int cnt = 0;
    for (const auto& e : comp->elements) if (e.is_active) ++cnt;
    return cnt;
}

bool FleetCultureSystem::hasElement(const std::string& entity_id,
                                     const std::string& element_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& e : comp->elements) {
        if (e.element_id == element_id) return true;
    }
    return false;
}

bool FleetCultureSystem::isElementActive(const std::string& entity_id,
                                          const std::string& element_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& e : comp->elements) {
        if (e.element_id == element_id) return e.is_active;
    }
    return false;
}

float FleetCultureSystem::getElementStrength(const std::string& entity_id,
                                              const std::string& element_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    for (const auto& e : comp->elements) {
        if (e.element_id == element_id) return e.strength;
    }
    return 0.0f;
}

int FleetCultureSystem::getReinforcementCount(const std::string& entity_id,
                                               const std::string& element_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    for (const auto& e : comp->elements) {
        if (e.element_id == element_id) return e.reinforcement_count;
    }
    return 0;
}

int FleetCultureSystem::getViolationCount(const std::string& entity_id,
                                           const std::string& element_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    for (const auto& e : comp->elements) {
        if (e.element_id == element_id) return e.violation_count;
    }
    return 0;
}

std::string FleetCultureSystem::getElementName(const std::string& entity_id,
                                                const std::string& element_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return "";
    for (const auto& e : comp->elements) {
        if (e.element_id == element_id) return e.name;
    }
    return "";
}

components::CultureElementType
FleetCultureSystem::getElementType(const std::string& entity_id,
                                    const std::string& element_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return components::CultureElementType::Tradition;
    for (const auto& e : comp->elements) {
        if (e.element_id == element_id) return e.type;
    }
    return components::CultureElementType::Tradition;
}

float FleetCultureSystem::getCohesionBonus(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->cohesion_bonus : 0.0f;
}

float FleetCultureSystem::getTensionLevel(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->tension_level : 0.0f;
}

bool FleetCultureSystem::isHighTension(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    return comp->tension_level >= 0.7f;
}

int FleetCultureSystem::getTotalElementsFormed(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_elements_formed : 0;
}

int FleetCultureSystem::getTotalReinforcements(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_reinforcements : 0;
}

int FleetCultureSystem::getTotalViolations(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_violations : 0;
}

std::string FleetCultureSystem::getFleetId(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->fleet_id : "";
}

int FleetCultureSystem::getCountByType(const std::string& entity_id,
                                        components::CultureElementType type) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    int cnt = 0;
    for (const auto& e : comp->elements) {
        if (e.type == type) ++cnt;
    }
    return cnt;
}

int FleetCultureSystem::getMaxElements(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->max_elements : 0;
}

} // namespace systems
} // namespace atlas
