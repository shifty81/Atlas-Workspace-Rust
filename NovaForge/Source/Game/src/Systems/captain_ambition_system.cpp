#include "systems/captain_ambition_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

CaptainAmbitionSystem::CaptainAmbitionSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void CaptainAmbitionSystem::updateComponent(
        ecs::Entity& /*entity*/,
        components::CaptainAmbitionState& comp,
        float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;
    // Per-blocked ambition: accumulate frustration; decay naturally
    for (auto& a : comp.ambitions) {
        if (!a.is_achieved) {
            if (a.is_blocked) {
                a.frustration_level = std::min(1.0f,
                    a.frustration_level + 0.01f * delta_time);
            } else {
                a.frustration_level = std::max(0.0f,
                    a.frustration_level - comp.frustration_decay_rate * delta_time);
            }
        }
    }
    recomputeDepartureRisk(comp);
}

void CaptainAmbitionSystem::recomputeDepartureRisk(components::CaptainAmbitionState& comp) {
    float risk = 0.0f;
    for (const auto& a : comp.ambitions) {
        if (!a.is_achieved) {
            risk += a.frustration_level;
        }
    }
    comp.departure_risk_contrib = std::min(1.0f, risk);
}

bool CaptainAmbitionSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::CaptainAmbitionState>();
    entity->addComponent(std::move(comp));
    return true;
}

bool CaptainAmbitionSystem::addAmbition(const std::string& entity_id,
                                         const std::string& ambition_id,
                                         components::AmbitionType type,
                                         const std::string& description,
                                         float target_value) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (ambition_id.empty()) return false;
    if (target_value <= 0.0f) return false;
    // No duplicate ids
    for (const auto& a : comp->ambitions) {
        if (a.ambition_id == ambition_id) return false;
    }
    // Enforce capacity
    if (static_cast<int>(comp->ambitions.size()) >= comp->max_ambitions) return false;
    components::CaptainAmbition ambition;
    ambition.ambition_id  = ambition_id;
    ambition.type         = type;
    ambition.description  = description;
    ambition.target_value = target_value;
    comp->ambitions.push_back(ambition);
    ++comp->total_ambitions_set;
    return true;
}

bool CaptainAmbitionSystem::removeAmbition(const std::string& entity_id,
                                             const std::string& ambition_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    auto it = std::find_if(comp->ambitions.begin(), comp->ambitions.end(),
        [&](const components::CaptainAmbition& a){ return a.ambition_id == ambition_id; });
    if (it == comp->ambitions.end()) return false;
    comp->ambitions.erase(it);
    recomputeDepartureRisk(*comp);
    return true;
}

bool CaptainAmbitionSystem::clearAmbitions(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->ambitions.clear();
    comp->departure_risk_contrib = 0.0f;
    return true;
}

bool CaptainAmbitionSystem::advanceProgress(const std::string& entity_id,
                                              const std::string& ambition_id,
                                              float amount) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (amount <= 0.0f) return false;
    for (auto& a : comp->ambitions) {
        if (a.ambition_id == ambition_id && !a.is_achieved) {
            a.current_value = std::min(a.target_value, a.current_value + amount);
            a.progress = a.current_value / a.target_value;
            return true;
        }
    }
    return false;
}

bool CaptainAmbitionSystem::markBlocked(const std::string& entity_id,
                                         const std::string& ambition_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto& a : comp->ambitions) {
        if (a.ambition_id == ambition_id && !a.is_achieved) {
            if (a.is_blocked) return false; // already blocked
            a.is_blocked = true;
            ++comp->total_blocked;
            return true;
        }
    }
    return false;
}

bool CaptainAmbitionSystem::markUnblocked(const std::string& entity_id,
                                           const std::string& ambition_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto& a : comp->ambitions) {
        if (a.ambition_id == ambition_id) {
            if (!a.is_blocked) return false;
            a.is_blocked = false;
            return true;
        }
    }
    return false;
}

bool CaptainAmbitionSystem::markAchieved(const std::string& entity_id,
                                          const std::string& ambition_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto& a : comp->ambitions) {
        if (a.ambition_id == ambition_id && !a.is_achieved) {
            a.is_achieved        = true;
            a.is_blocked         = false;
            a.progress           = 1.0f;
            a.frustration_level  = 0.0f;
            ++comp->total_achieved;
            recomputeDepartureRisk(*comp);
            return true;
        }
    }
    return false;
}

bool CaptainAmbitionSystem::setCaptainId(const std::string& entity_id,
                                          const std::string& captain_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (captain_id.empty()) return false;
    comp->captain_id = captain_id;
    return true;
}

bool CaptainAmbitionSystem::setFrustrationDecayRate(const std::string& entity_id, float rate) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (rate < 0.0f) return false;
    comp->frustration_decay_rate = rate;
    return true;
}

bool CaptainAmbitionSystem::setMaxAmbitions(const std::string& entity_id, int max) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (max < 1) return false;
    comp->max_ambitions = max;
    return true;
}

// ── Queries ──────────────────────────────────────────────────────────────────

int CaptainAmbitionSystem::getAmbitionCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->ambitions.size()) : 0;
}

bool CaptainAmbitionSystem::hasAmbition(const std::string& entity_id,
                                         const std::string& ambition_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& a : comp->ambitions) {
        if (a.ambition_id == ambition_id) return true;
    }
    return false;
}

float CaptainAmbitionSystem::getProgress(const std::string& entity_id,
                                          const std::string& ambition_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    for (const auto& a : comp->ambitions) {
        if (a.ambition_id == ambition_id) return a.progress;
    }
    return 0.0f;
}

float CaptainAmbitionSystem::getFrustrationLevel(const std::string& entity_id,
                                                   const std::string& ambition_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    for (const auto& a : comp->ambitions) {
        if (a.ambition_id == ambition_id) return a.frustration_level;
    }
    return 0.0f;
}

bool CaptainAmbitionSystem::isAchieved(const std::string& entity_id,
                                        const std::string& ambition_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& a : comp->ambitions) {
        if (a.ambition_id == ambition_id) return a.is_achieved;
    }
    return false;
}

bool CaptainAmbitionSystem::isBlocked(const std::string& entity_id,
                                       const std::string& ambition_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& a : comp->ambitions) {
        if (a.ambition_id == ambition_id) return a.is_blocked;
    }
    return false;
}

components::AmbitionType
CaptainAmbitionSystem::getAmbitionType(const std::string& entity_id,
                                        const std::string& ambition_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return components::AmbitionType::LeadWing;
    for (const auto& a : comp->ambitions) {
        if (a.ambition_id == ambition_id) return a.type;
    }
    return components::AmbitionType::LeadWing;
}

float CaptainAmbitionSystem::getDepartureRiskContrib(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->departure_risk_contrib : 0.0f;
}

int CaptainAmbitionSystem::getAchievedCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    int cnt = 0;
    for (const auto& a : comp->ambitions) if (a.is_achieved) ++cnt;
    return cnt;
}

int CaptainAmbitionSystem::getBlockedCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    int cnt = 0;
    for (const auto& a : comp->ambitions) if (a.is_blocked && !a.is_achieved) ++cnt;
    return cnt;
}

int CaptainAmbitionSystem::getTotalAmbitionsSet(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_ambitions_set : 0;
}

int CaptainAmbitionSystem::getTotalAchieved(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_achieved : 0;
}

int CaptainAmbitionSystem::getTotalBlocked(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_blocked : 0;
}

std::string CaptainAmbitionSystem::getCaptainId(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->captain_id : "";
}

float CaptainAmbitionSystem::getFrustrationDecayRate(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->frustration_decay_rate : 0.0f;
}

int CaptainAmbitionSystem::getMaxAmbitions(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->max_ambitions : 0;
}

int CaptainAmbitionSystem::getCountByType(const std::string& entity_id,
                                           components::AmbitionType type) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    int cnt = 0;
    for (const auto& a : comp->ambitions) {
        if (a.type == type) ++cnt;
    }
    return cnt;
}

} // namespace systems
} // namespace atlas
