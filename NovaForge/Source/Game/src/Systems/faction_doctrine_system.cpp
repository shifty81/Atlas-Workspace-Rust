#include "systems/faction_doctrine_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

FactionDoctrineSystem::FactionDoctrineSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

// ---------------------------------------------------------------------------
// Derived aggression/stealth/raid_frequency from current phase
// ---------------------------------------------------------------------------
void FactionDoctrineSystem::recomputeProfile(
        components::FactionDoctrineState& comp) const {
    using P = components::FactionDoctrineState::DoctrinePhase;
    switch (comp.doctrine_phase) {
        case P::Accumulate:
            comp.aggression_mult = 0.2f;
            comp.stealth_bias    = 0.7f;
            comp.raid_frequency  = 0.1f;
            break;
        case P::Conceal:
            comp.aggression_mult = 0.1f;
            comp.stealth_bias    = 0.9f;
            comp.raid_frequency  = 0.05f;
            break;
        case P::Disrupt:
            comp.aggression_mult = 0.6f;
            comp.stealth_bias    = 0.4f;
            comp.raid_frequency  = 0.5f;
            break;
        case P::Defend:
            comp.aggression_mult = 0.8f;
            comp.stealth_bias    = 0.3f;
            comp.raid_frequency  = 0.3f;
            break;
        case P::PrepareLaunch:
            comp.aggression_mult = 1.0f;
            comp.stealth_bias    = 0.1f;
            comp.raid_frequency  = 0.9f;
            break;
    }
}

void FactionDoctrineSystem::applyPhaseTransitions(
        components::FactionDoctrineState& comp) const {
    using P = components::FactionDoctrineState::DoctrinePhase;
    P old_phase = comp.doctrine_phase;

    if (comp.titan_completion >= comp.launch_threshold) {
        comp.doctrine_phase = P::PrepareLaunch;
    } else if (comp.titan_completion >= comp.defend_threshold) {
        comp.doctrine_phase = P::Defend;
    } else if (comp.titan_completion >= comp.disrupt_threshold
            || comp.discovery_risk >= 0.6f) {
        comp.doctrine_phase = P::Disrupt;
    } else if (comp.titan_completion >= comp.conceal_threshold
            || comp.discovery_risk >= 0.3f) {
        comp.doctrine_phase = P::Conceal;
    } else {
        comp.doctrine_phase = P::Accumulate;
    }

    if (comp.doctrine_phase != old_phase) {
        ++comp.total_phase_shifts;
        recomputeProfile(comp);
    }
}

void FactionDoctrineSystem::updateComponent(
        ecs::Entity& /*entity*/,
        components::FactionDoctrineState& comp,
        float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;
    applyPhaseTransitions(comp);
}

bool FactionDoctrineSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::FactionDoctrineState>();
    recomputeProfile(*comp);
    entity->addComponent(std::move(comp));
    return true;
}

bool FactionDoctrineSystem::setTitanCompletion(const std::string& entity_id, float val) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->titan_completion = std::clamp(val, 0.0f, 1.0f);
    applyPhaseTransitions(*comp);
    return true;
}

bool FactionDoctrineSystem::setDiscoveryRisk(const std::string& entity_id, float val) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->discovery_risk = std::clamp(val, 0.0f, 1.0f);
    applyPhaseTransitions(*comp);
    return true;
}

bool FactionDoctrineSystem::setResourceScarcity(const std::string& entity_id, float val) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->resource_scarcity = std::clamp(val, 0.0f, 1.0f);
    return true;
}

bool FactionDoctrineSystem::setPlayerProximity(const std::string& entity_id, float val) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->player_proximity = std::clamp(val, 0.0f, 1.0f);
    return true;
}

bool FactionDoctrineSystem::setConcealThreshold(const std::string& entity_id, float val) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (val < 0.0f || val > 1.0f) return false;
    comp->conceal_threshold = val;
    return true;
}

bool FactionDoctrineSystem::setDisruptThreshold(const std::string& entity_id, float val) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (val < 0.0f || val > 1.0f) return false;
    comp->disrupt_threshold = val;
    return true;
}

bool FactionDoctrineSystem::setDefendThreshold(const std::string& entity_id, float val) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (val < 0.0f || val > 1.0f) return false;
    comp->defend_threshold = val;
    return true;
}

bool FactionDoctrineSystem::setLaunchThreshold(const std::string& entity_id, float val) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (val < 0.0f || val > 1.0f) return false;
    comp->launch_threshold = val;
    return true;
}

bool FactionDoctrineSystem::setFactionId(const std::string& entity_id,
                                          const std::string& faction_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (faction_id.empty()) return false;
    comp->faction_id = faction_id;
    return true;
}

bool FactionDoctrineSystem::advancePhase(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    using P = components::FactionDoctrineState::DoctrinePhase;
    if (comp->doctrine_phase == P::PrepareLaunch) return false;
    comp->doctrine_phase = static_cast<P>(static_cast<int>(comp->doctrine_phase) + 1);
    ++comp->total_phase_shifts;
    recomputeProfile(*comp);
    return true;
}

bool FactionDoctrineSystem::resetToAccumulate(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->doctrine_phase    = components::FactionDoctrineState::DoctrinePhase::Accumulate;
    comp->titan_completion  = 0.0f;
    comp->discovery_risk    = 0.0f;
    comp->resource_scarcity = 0.0f;
    comp->player_proximity  = 0.0f;
    recomputeProfile(*comp);
    return true;
}

components::FactionDoctrineState::DoctrinePhase
FactionDoctrineSystem::getDoctrinePhase(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return components::FactionDoctrineState::DoctrinePhase::Accumulate;
    return comp->doctrine_phase;
}

std::string FactionDoctrineSystem::getDoctrinePhaseString(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return "";
    using P = components::FactionDoctrineState::DoctrinePhase;
    switch (comp->doctrine_phase) {
        case P::Accumulate:    return "Accumulate";
        case P::Conceal:       return "Conceal";
        case P::Disrupt:       return "Disrupt";
        case P::Defend:        return "Defend";
        case P::PrepareLaunch: return "PrepareLaunch";
    }
    return "Unknown";
}

float FactionDoctrineSystem::getTitanCompletion(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->titan_completion : 0.0f;
}

float FactionDoctrineSystem::getDiscoveryRisk(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->discovery_risk : 0.0f;
}

float FactionDoctrineSystem::getResourceScarcity(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->resource_scarcity : 0.0f;
}

float FactionDoctrineSystem::getPlayerProximity(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->player_proximity : 0.0f;
}

float FactionDoctrineSystem::getAggressionMult(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->aggression_mult : 0.0f;
}

float FactionDoctrineSystem::getStealthBias(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->stealth_bias : 0.0f;
}

float FactionDoctrineSystem::getRaidFrequency(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->raid_frequency : 0.0f;
}

int FactionDoctrineSystem::getTotalPhaseShifts(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_phase_shifts : 0;
}

std::string FactionDoctrineSystem::getFactionId(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->faction_id : "";
}

bool FactionDoctrineSystem::isLaunchImminent(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    return comp->doctrine_phase
        == components::FactionDoctrineState::DoctrinePhase::PrepareLaunch;
}

bool FactionDoctrineSystem::isActive(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->active : false;
}

} // namespace systems
} // namespace atlas
