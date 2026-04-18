#include "systems/faction_behavior_modifier_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

FactionBehaviorModifierSystem::FactionBehaviorModifierSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void FactionBehaviorModifierSystem::updateComponent(
        ecs::Entity& /*entity*/,
        components::FactionBehaviorState& comp,
        float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;
}

bool FactionBehaviorModifierSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::FactionBehaviorState>();
    entity->addComponent(std::move(comp));
    return true;
}

bool FactionBehaviorModifierSystem::setProfile(
        const std::string& entity_id,
        components::FactionBehaviorState::FactionProfile profile) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;

    using FP = components::FactionBehaviorState::FactionProfile;
    using MD = components::FactionBehaviorState::MoraleDriver;

    comp->profile = profile;
    ++comp->total_profile_changes;

    switch (profile) {
        case FP::Industrial:
            comp->chatter_rate_mult       = 0.5f;
            comp->combat_preference       = 0.2f;
            comp->mining_preference       = 0.9f;
            comp->exploration_preference  = 0.4f;
            comp->trade_preference        = 0.8f;
            comp->departure_threshold     = -30.0f;
            comp->morale_bias             = 5.0f;
            comp->morale_driver           = MD::Efficiency;
            break;
        case FP::Militaristic:
            comp->chatter_rate_mult       = 1.5f;
            comp->combat_preference       = 0.9f;
            comp->mining_preference       = 0.2f;
            comp->exploration_preference  = 0.3f;
            comp->trade_preference        = 0.5f;
            comp->departure_threshold     = -60.0f;
            comp->morale_bias             = 8.0f;
            comp->morale_driver           = MD::Victory;
            break;
        case FP::Nomadic:
            comp->chatter_rate_mult       = 1.2f;
            comp->combat_preference       = 0.4f;
            comp->mining_preference       = 0.3f;
            comp->exploration_preference  = 0.95f;
            comp->trade_preference        = 0.4f;
            comp->departure_threshold     = -40.0f;
            comp->morale_bias             = 3.0f;
            comp->morale_driver           = MD::Exploration;
            break;
        case FP::Corporate:
            comp->chatter_rate_mult       = 0.6f;
            comp->combat_preference       = 0.5f;
            comp->mining_preference       = 0.4f;
            comp->exploration_preference  = 0.3f;
            comp->trade_preference        = 0.9f;
            comp->departure_threshold     = -20.0f;
            comp->morale_bias             = 0.0f;
            comp->morale_driver           = MD::Success;
            break;
        case FP::Neutral:
        default:
            comp->chatter_rate_mult       = 1.0f;
            comp->combat_preference       = 0.5f;
            comp->mining_preference       = 0.5f;
            comp->exploration_preference  = 0.5f;
            comp->trade_preference        = 0.5f;
            comp->departure_threshold     = -50.0f;
            comp->morale_bias             = 0.0f;
            comp->morale_driver           = MD::None;
            break;
    }
    return true;
}

bool FactionBehaviorModifierSystem::setFactionId(
        const std::string& entity_id, const std::string& faction_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (faction_id.empty()) return false;
    comp->faction_id = faction_id;
    return true;
}

bool FactionBehaviorModifierSystem::setMoraleBias(
        const std::string& entity_id, float bias) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->morale_bias = std::max(-100.0f, std::min(100.0f, bias));
    return true;
}

bool FactionBehaviorModifierSystem::setChatterRateMult(
        const std::string& entity_id, float mult) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (mult <= 0.0f) return false;
    comp->chatter_rate_mult = mult;
    return true;
}

bool FactionBehaviorModifierSystem::setCombatPreference(
        const std::string& entity_id, float pref) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->combat_preference = std::max(0.0f, std::min(1.0f, pref));
    return true;
}

bool FactionBehaviorModifierSystem::setMiningPreference(
        const std::string& entity_id, float pref) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->mining_preference = std::max(0.0f, std::min(1.0f, pref));
    return true;
}

bool FactionBehaviorModifierSystem::setExplorationPreference(
        const std::string& entity_id, float pref) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->exploration_preference = std::max(0.0f, std::min(1.0f, pref));
    return true;
}

bool FactionBehaviorModifierSystem::setTradePreference(
        const std::string& entity_id, float pref) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->trade_preference = std::max(0.0f, std::min(1.0f, pref));
    return true;
}

bool FactionBehaviorModifierSystem::setDepartureThreshold(
        const std::string& entity_id, float threshold) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->departure_threshold = threshold;
    return true;
}

float FactionBehaviorModifierSystem::applyMoraleModifier(
        const std::string& entity_id, float base_morale) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return base_morale;
    float result = base_morale + comp->morale_bias;
    return std::max(-100.0f, std::min(100.0f, result));
}

std::string FactionBehaviorModifierSystem::getDominantActivity(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return "None";

    // Ties broken by order: combat > mining > exploration > trade
    float best = comp->combat_preference;
    std::string activity = "combat";

    if (comp->mining_preference > best) {
        best = comp->mining_preference;
        activity = "mining";
    }
    if (comp->exploration_preference > best) {
        best = comp->exploration_preference;
        activity = "exploration";
    }
    if (comp->trade_preference > best) {
        activity = "trade";
    }
    return activity;
}

bool FactionBehaviorModifierSystem::isDepartureRisk(
        const std::string& entity_id, float morale) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    return morale <= comp->departure_threshold;
}

components::FactionBehaviorState::FactionProfile
FactionBehaviorModifierSystem::getProfile(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return components::FactionBehaviorState::FactionProfile::Neutral;
    return comp->profile;
}

components::FactionBehaviorState::MoraleDriver
FactionBehaviorModifierSystem::getMoraleDriver(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return components::FactionBehaviorState::MoraleDriver::None;
    return comp->morale_driver;
}

float FactionBehaviorModifierSystem::getMoraleBias(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    return comp->morale_bias;
}

float FactionBehaviorModifierSystem::getChatterRateMult(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    return comp->chatter_rate_mult;
}

float FactionBehaviorModifierSystem::getCombatPreference(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    return comp->combat_preference;
}

float FactionBehaviorModifierSystem::getMiningPreference(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    return comp->mining_preference;
}

float FactionBehaviorModifierSystem::getExplorationPreference(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    return comp->exploration_preference;
}

float FactionBehaviorModifierSystem::getTradePreference(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    return comp->trade_preference;
}

float FactionBehaviorModifierSystem::getDepartureThreshold(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    return comp->departure_threshold;
}

std::string FactionBehaviorModifierSystem::getFactionId(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return "";
    return comp->faction_id;
}

int FactionBehaviorModifierSystem::getTotalProfileChanges(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return comp->total_profile_changes;
}

} // namespace systems
} // namespace atlas
