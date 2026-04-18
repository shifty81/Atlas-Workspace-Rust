#include "systems/emotional_arc_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>
#include <cmath>

namespace atlas {
namespace systems {

EmotionalArcSystem::EmotionalArcSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void EmotionalArcSystem::updateComponent(
        ecs::Entity& /*entity*/,
        components::EmotionalArcState& comp,
        float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;
}

bool EmotionalArcSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::EmotionalArcState>();
    entity->addComponent(std::move(comp));
    return true;
}

bool EmotionalArcSystem::applyWin(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->confidence = std::min(1.0f, comp->confidence + 0.05f);
    comp->hope       = std::min(1.0f, comp->hope       + 0.03f);
    comp->fatigue    = std::min(1.0f, comp->fatigue    + 0.02f);
    ++comp->wins;
    ++comp->total_arc_updates;
    return true;
}

bool EmotionalArcSystem::applyLoss(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->confidence = std::max(0.0f, comp->confidence - 0.08f);
    comp->hope       = std::max(0.0f, comp->hope       - 0.05f);
    comp->fatigue    = std::min(1.0f, comp->fatigue    + 0.05f);
    ++comp->losses;
    ++comp->total_arc_updates;
    return true;
}

bool EmotionalArcSystem::applyNearDeath(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->trust_in_player = std::max(0.0f, comp->trust_in_player - 0.10f);
    comp->confidence      = std::max(0.0f, comp->confidence      - 0.05f);
    comp->fatigue         = std::min(1.0f, comp->fatigue         + 0.15f);
    ++comp->near_deaths;
    ++comp->total_arc_updates;
    return true;
}

bool EmotionalArcSystem::applySave(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->trust_in_player = std::min(1.0f, comp->trust_in_player + 0.12f);
    comp->hope            = std::min(1.0f, comp->hope             + 0.05f);
    ++comp->saves_by_player;
    ++comp->total_arc_updates;
    return true;
}

bool EmotionalArcSystem::applyRest(const std::string& entity_id, float hours) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (hours < 0.0f) return false;
    float reduction = hours * 0.05f;
    comp->fatigue = std::max(0.0f, comp->fatigue - reduction);
    ++comp->total_arc_updates;
    return true;
}

bool EmotionalArcSystem::applyExploration(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->hope    = std::min(1.0f, comp->hope    + 0.03f);
    comp->fatigue = std::min(1.0f, comp->fatigue + 0.01f);
    ++comp->total_arc_updates;
    return true;
}

bool EmotionalArcSystem::setConfidence(const std::string& entity_id, float val) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->confidence = std::clamp(val, 0.0f, 1.0f);
    return true;
}

bool EmotionalArcSystem::setTrustInPlayer(const std::string& entity_id, float val) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->trust_in_player = std::clamp(val, 0.0f, 1.0f);
    return true;
}

bool EmotionalArcSystem::setFatigue(const std::string& entity_id, float val) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->fatigue = std::clamp(val, 0.0f, 1.0f);
    return true;
}

bool EmotionalArcSystem::setHope(const std::string& entity_id, float val) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->hope = std::clamp(val, 0.0f, 1.0f);
    return true;
}

bool EmotionalArcSystem::setCaptainId(const std::string& entity_id,
                                       const std::string& captain_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (captain_id.empty()) return false;
    comp->captain_id = captain_id;
    return true;
}

float EmotionalArcSystem::getConfidence(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->confidence : 0.5f;
}

float EmotionalArcSystem::getTrustInPlayer(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->trust_in_player : 0.5f;
}

float EmotionalArcSystem::getFatigue(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->fatigue : 0.0f;
}

float EmotionalArcSystem::getHope(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->hope : 0.5f;
}

std::string EmotionalArcSystem::getArcLabel(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return "";
    float c = comp->confidence;
    float t = comp->trust_in_player;
    float f = comp->fatigue;
    float h = comp->hope;
    if (f > 0.8f)                  return "The Weary";
    if (c > 0.7f && h > 0.7f)     return "The Optimist";
    if (c < 0.3f && h > 0.6f)     return "The Survivor";
    if (t < 0.3f && c > 0.5f)     return "The Skeptic";
    if (t > 0.7f && f > 0.6f)     return "The Faithful";
    return "Undefined";
}

bool EmotionalArcSystem::isWornDown(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    return comp->fatigue > 0.8f;
}

bool EmotionalArcSystem::isLoyalToPlayer(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    return comp->trust_in_player > 0.7f;
}

bool EmotionalArcSystem::isOptimistic(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    return comp->hope > 0.6f && comp->confidence > 0.6f;
}

int EmotionalArcSystem::getWins(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->wins : 0;
}

int EmotionalArcSystem::getLosses(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->losses : 0;
}

int EmotionalArcSystem::getNearDeaths(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->near_deaths : 0;
}

int EmotionalArcSystem::getSavesByPlayer(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->saves_by_player : 0;
}

int EmotionalArcSystem::getTotalArcUpdates(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_arc_updates : 0;
}

std::string EmotionalArcSystem::getCaptainId(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->captain_id : "";
}

// ── Convenience aliases ───────────────────────────────────────────────────

bool EmotionalArcSystem::onCombatVictory(const std::string& entity_id) {
    // Ensure EmotionalArcState exists before applying win
    if (!getComponentFor(entity_id)) {
        auto* entity = world_->getEntity(entity_id);
        if (!entity) return false;
        entity->addComponent(std::make_unique<components::EmotionalArcState>());
    }
    return applyWin(entity_id);
}

bool EmotionalArcSystem::onCombatDefeat(const std::string& entity_id) {
    if (!getComponentFor(entity_id)) {
        auto* entity = world_->getEntity(entity_id);
        if (!entity) return false;
        entity->addComponent(std::make_unique<components::EmotionalArcState>());
    }
    return applyLoss(entity_id);
}

bool EmotionalArcSystem::onRest(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* state = entity->getComponent<components::EmotionalState>();
    if (!state) return false;
    state->fatigue = std::max(0.0f, state->fatigue - 10.0f);
    return true;
}

bool EmotionalArcSystem::onPlayerTrust(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* state = entity->getComponent<components::EmotionalState>();
    if (!state) return false;
    state->trust_in_player = std::min(100.0f, state->trust_in_player + 5.0f);
    return true;
}

bool EmotionalArcSystem::onPlayerBetray(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* state = entity->getComponent<components::EmotionalState>();
    if (!state) return false;
    state->trust_in_player = std::max(0.0f, state->trust_in_player - 10.0f);
    return true;
}

float EmotionalArcSystem::getTrust(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 50.0f;
    auto* state = entity->getComponent<components::EmotionalState>();
    return state ? state->trust_in_player : 50.0f;
}

} // namespace systems
} // namespace atlas
