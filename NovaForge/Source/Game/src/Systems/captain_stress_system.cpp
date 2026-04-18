#include "systems/captain_stress_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

CaptainStressSystem::CaptainStressSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void CaptainStressSystem::updateComponent(
        ecs::Entity& /*entity*/,
        components::CaptainStressState& comp,
        float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;
    // Passive stress recovery
    if (comp.stress_level > 0.0f) {
        comp.stress_level = std::max(0.0f,
            comp.stress_level - comp.recovery_rate * delta_time);
    }
}

bool CaptainStressSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::CaptainStressState>();
    entity->addComponent(std::move(comp));
    return true;
}

bool CaptainStressSystem::recordCombat(const std::string& entity_id, float intensity) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (intensity < 0.0f) return false;
    float increase = std::clamp(intensity, 0.0f, 1.0f) * 15.0f;
    comp->stress_level = std::min(100.0f, comp->stress_level + increase);
    ++comp->total_stressors_applied;
    return true;
}

bool CaptainStressSystem::recordNearDeath(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->stress_level = std::min(100.0f, comp->stress_level + 25.0f);
    ++comp->total_stressors_applied;
    return true;
}

bool CaptainStressSystem::recordMissionFailure(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->stress_level = std::min(100.0f, comp->stress_level + 10.0f);
    ++comp->total_stressors_applied;
    return true;
}

bool CaptainStressSystem::recordLongDeployment(const std::string& entity_id, float hours) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (hours < 0.0f) return false;
    // 1 stress point per 8 hours of deployment
    float increase = hours / 8.0f;
    comp->stress_level = std::min(100.0f, comp->stress_level + increase);
    ++comp->total_stressors_applied;
    return true;
}

bool CaptainStressSystem::applyRest(const std::string& entity_id, float hours) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (hours < 0.0f) return false;
    float reduction = hours * 3.0f;
    comp->stress_level = std::max(0.0f, comp->stress_level - reduction);
    ++comp->total_relief_events;
    return true;
}

bool CaptainStressSystem::applyRelief(const std::string& entity_id, float amount) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (amount < 0.0f) return false;
    comp->stress_level = std::max(0.0f, comp->stress_level - amount);
    ++comp->total_relief_events;
    return true;
}

bool CaptainStressSystem::applyShoreleave(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->stress_level = 0.0f;
    ++comp->total_relief_events;
    return true;
}

bool CaptainStressSystem::setRecoveryRate(const std::string& entity_id, float rate) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (rate < 0.0f) return false;
    comp->recovery_rate = rate;
    return true;
}

bool CaptainStressSystem::setStressThreshold(const std::string& entity_id, float val) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (val < 0.0f || val > 100.0f) return false;
    comp->stress_threshold = val;
    return true;
}

bool CaptainStressSystem::setCriticalLevel(const std::string& entity_id, float val) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (val < 0.0f || val > 100.0f) return false;
    comp->critical_level = val;
    return true;
}

bool CaptainStressSystem::setCaptainId(const std::string& entity_id,
                                        const std::string& captain_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (captain_id.empty()) return false;
    comp->captain_id = captain_id;
    return true;
}

float CaptainStressSystem::getStressLevel(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->stress_level : 0.0f;
}

float CaptainStressSystem::getRecoveryRate(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->recovery_rate : 0.0f;
}

float CaptainStressSystem::getStressThreshold(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->stress_threshold : 0.0f;
}

float CaptainStressSystem::getCriticalLevel(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->critical_level : 0.0f;
}

bool CaptainStressSystem::isHighStress(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    return comp->stress_level >= comp->stress_threshold;
}

bool CaptainStressSystem::isCriticalStress(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    return comp->stress_level >= comp->critical_level;
}

float CaptainStressSystem::getStressPercent(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    return comp->stress_level / 100.0f;
}

int CaptainStressSystem::getTotalStressorsApplied(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_stressors_applied : 0;
}

int CaptainStressSystem::getTotalReliefEvents(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_relief_events : 0;
}

std::string CaptainStressSystem::getCaptainId(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->captain_id : "";
}

} // namespace systems
} // namespace atlas
