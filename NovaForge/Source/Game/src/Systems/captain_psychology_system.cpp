#include "systems/captain_psychology_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"

#include <algorithm>

namespace atlas {
namespace systems {

static float clamp01(float v) {
    return std::max(0.0f, std::min(1.0f, v));
}

CaptainPsychologySystem::CaptainPsychologySystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void CaptainPsychologySystem::updateComponent(
        ecs::Entity& /*entity*/,
        components::CaptainPsychologyState& comp,
        float delta_time) {
    if (!comp.active) return;

    comp.elapsed += delta_time;

    // Drift each axis toward its baseline
    auto drift = [&](components::CaptainPsychologyState::PersonalityAxis& axis) {
        float diff = axis.baseline - axis.current;
        if (diff > 0.0f) {
            axis.current = std::min(axis.current + comp.drift_rate * delta_time, axis.baseline);
        } else if (diff < 0.0f) {
            axis.current = std::max(axis.current + comp.drift_rate * delta_time * -1.0f,
                                    axis.baseline);
        }
        // If diff == 0, already at baseline
    };

    drift(comp.aggression);
    drift(comp.caution);
    drift(comp.loyalty);
    drift(comp.greed);

    // Decay stress
    comp.stress = std::max(0.0f, comp.stress - comp.stress_decay * delta_time);

    // Recompute mood = average of 4 axes minus stress, clamped [0,1]
    float avg = (comp.aggression.current + comp.caution.current +
                 comp.loyalty.current + comp.greed.current) / 4.0f;
    comp.mood = clamp01(avg - comp.stress);
}

bool CaptainPsychologySystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto comp = std::make_unique<components::CaptainPsychologyState>();
    entity->addComponent(std::move(comp));
    return true;
}

bool CaptainPsychologySystem::setBaseline(const std::string& entity_id,
                                           float aggression, float caution,
                                           float loyalty, float greed) {
    if (aggression < 0.0f || aggression > 1.0f) return false;
    if (caution < 0.0f || caution > 1.0f) return false;
    if (loyalty < 0.0f || loyalty > 1.0f) return false;
    if (greed < 0.0f || greed > 1.0f) return false;

    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;

    comp->aggression.baseline = aggression;
    comp->caution.baseline = caution;
    comp->loyalty.baseline = loyalty;
    comp->greed.baseline = greed;
    return true;
}

bool CaptainPsychologySystem::setCurrent(const std::string& entity_id,
                                          float aggression, float caution,
                                          float loyalty, float greed) {
    if (aggression < 0.0f || aggression > 1.0f) return false;
    if (caution < 0.0f || caution > 1.0f) return false;
    if (loyalty < 0.0f || loyalty > 1.0f) return false;
    if (greed < 0.0f || greed > 1.0f) return false;

    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;

    comp->aggression.current = aggression;
    comp->caution.current = caution;
    comp->loyalty.current = loyalty;
    comp->greed.current = greed;
    return true;
}

bool CaptainPsychologySystem::setDriftRate(const std::string& entity_id, float rate) {
    if (rate <= 0.0f || rate > 1.0f) return false;

    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;

    comp->drift_rate = rate;
    return true;
}

bool CaptainPsychologySystem::setStressDecay(const std::string& entity_id, float rate) {
    if (rate <= 0.0f || rate > 1.0f) return false;

    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;

    comp->stress_decay = rate;
    return true;
}

bool CaptainPsychologySystem::setCaptainId(const std::string& entity_id,
                                            const std::string& captain_id) {
    if (captain_id.empty()) return false;

    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;

    comp->captain_id = captain_id;
    return true;
}

bool CaptainPsychologySystem::processEvent(
        const std::string& entity_id,
        components::CaptainPsychologyState::EventType event) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;

    using ET = components::CaptainPsychologyState::EventType;

    switch (event) {
        case ET::Victory:
            comp->aggression.current = clamp01(comp->aggression.current + 0.05f);
            comp->caution.current    = clamp01(comp->caution.current - 0.03f);
            comp->mood               = clamp01(comp->mood + 0.1f);
            break;
        case ET::Defeat:
            comp->aggression.current = clamp01(comp->aggression.current - 0.05f);
            comp->caution.current    = clamp01(comp->caution.current + 0.08f);
            comp->stress             = clamp01(comp->stress + 0.15f);
            comp->mood               = clamp01(comp->mood - 0.1f);
            break;
        case ET::LootGained:
            comp->greed.current = clamp01(comp->greed.current + 0.05f);
            comp->mood          = clamp01(comp->mood + 0.05f);
            break;
        case ET::AllyLost:
            comp->loyalty.current = clamp01(comp->loyalty.current + 0.05f);
            comp->caution.current = clamp01(comp->caution.current + 0.05f);
            comp->stress          = clamp01(comp->stress + 0.1f);
            comp->mood            = clamp01(comp->mood - 0.15f);
            break;
        case ET::OrderGiven:
            comp->loyalty.current = clamp01(comp->loyalty.current + 0.02f);
            break;
        case ET::LongIdle:
            comp->stress             = clamp01(comp->stress - 0.05f);
            comp->caution.current    = clamp01(comp->caution.current + 0.02f);
            comp->aggression.current = clamp01(comp->aggression.current - 0.02f);
            break;
    }

    comp->events_processed++;
    comp->total_shifts++;
    return true;
}

bool CaptainPsychologySystem::applyStress(const std::string& entity_id, float amount) {
    if (amount <= 0.0f) return false;

    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;

    comp->stress = clamp01(comp->stress + amount);
    return true;
}

bool CaptainPsychologySystem::resetStress(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;

    comp->stress = 0.0f;
    return true;
}

bool CaptainPsychologySystem::resetToBaseline(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;

    comp->aggression.current = comp->aggression.baseline;
    comp->caution.current    = comp->caution.baseline;
    comp->loyalty.current    = comp->loyalty.baseline;
    comp->greed.current      = comp->greed.baseline;
    comp->stress = 0.0f;
    comp->mood   = 0.5f;
    return true;
}

// --- Queries ---

float CaptainPsychologySystem::getAggression(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->aggression.current : 0.0f;
}

float CaptainPsychologySystem::getCaution(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->caution.current : 0.0f;
}

float CaptainPsychologySystem::getLoyalty(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->loyalty.current : 0.0f;
}

float CaptainPsychologySystem::getGreed(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->greed.current : 0.0f;
}

float CaptainPsychologySystem::getMood(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->mood : 0.0f;
}

float CaptainPsychologySystem::getStress(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->stress : 0.0f;
}

int CaptainPsychologySystem::getEventsProcessed(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->events_processed : 0;
}

int CaptainPsychologySystem::getTotalShifts(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_shifts : 0;
}

bool CaptainPsychologySystem::isAggressive(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->aggression.current > 0.7f : false;
}

bool CaptainPsychologySystem::isCautious(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->caution.current > 0.7f : false;
}

bool CaptainPsychologySystem::isLoyal(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->loyalty.current > 0.7f : false;
}

bool CaptainPsychologySystem::isGreedy(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->greed.current > 0.7f : false;
}

bool CaptainPsychologySystem::isStressed(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->stress > 0.7f : false;
}

std::string CaptainPsychologySystem::getCaptainId(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->captain_id : "";
}

} // namespace systems
} // namespace atlas
