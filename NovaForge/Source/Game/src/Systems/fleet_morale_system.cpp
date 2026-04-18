#include "systems/fleet_morale_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"

#include <algorithm>

namespace atlas {
namespace systems {

static float clampMorale(float v) {
    return std::max(0.0f, std::min(1.0f, v));
}

FleetMoraleSystem::FleetMoraleSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void FleetMoraleSystem::updateComponent(
        ecs::Entity& /*entity*/,
        components::FleetMoraleState& comp,
        float delta_time) {
    if (!comp.active) return;

    comp.elapsed += delta_time;

    // Drift morale toward baseline
    float m_diff = comp.morale_baseline - comp.morale;
    if (m_diff > 0.0f) {
        comp.morale = std::min(comp.morale + comp.morale_decay * delta_time,
                               comp.morale_baseline);
    } else if (m_diff < 0.0f) {
        comp.morale = std::max(comp.morale - comp.morale_decay * delta_time,
                               comp.morale_baseline);
    }

    // Drift cohesion toward baseline
    float c_diff = comp.cohesion_baseline - comp.cohesion;
    if (c_diff > 0.0f) {
        comp.cohesion = std::min(comp.cohesion + comp.cohesion_decay * delta_time,
                                 comp.cohesion_baseline);
    } else if (c_diff < 0.0f) {
        comp.cohesion = std::max(comp.cohesion - comp.cohesion_decay * delta_time,
                                 comp.cohesion_baseline);
    }

    comp.morale   = clampMorale(comp.morale);
    comp.cohesion = clampMorale(comp.cohesion);
}

bool FleetMoraleSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::FleetMoraleState>();
    entity->addComponent(std::move(comp));
    return true;
}

bool FleetMoraleSystem::recordEvent(
        const std::string& entity_id,
        components::FleetMoraleState::MoraleEvent event) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;

    using ME = components::FleetMoraleState::MoraleEvent;
    float m_impact = 0.0f;
    float c_impact = 0.0f;

    switch (event) {
        case ME::Victory:
            m_impact =  0.1f;
            c_impact =  0.05f;
            comp->victories++;
            break;
        case ME::Defeat:
            m_impact = -0.15f;
            c_impact = -0.05f;
            comp->defeats++;
            break;
        case ME::AllyDestroyed:
            m_impact = -0.2f;
            c_impact = -0.1f;
            break;
        case ME::SuccessfulRetreat:
            m_impact =  0.05f;
            c_impact =  0.08f;
            break;
        case ME::LootShared:
            m_impact =  0.08f;
            c_impact =  0.1f;
            break;
        case ME::LongIdle:
            m_impact = -0.03f;
            c_impact = -0.02f;
            break;
        case ME::OrderFollowed:
            m_impact =  0.0f;
            c_impact =  0.05f;
            break;
        case ME::OrderIgnored:
            m_impact = -0.05f;
            c_impact = -0.1f;
            break;
    }

    comp->morale   = clampMorale(comp->morale + m_impact);
    comp->cohesion = clampMorale(comp->cohesion + c_impact);

    components::FleetMoraleState::EventEntry entry;
    entry.event     = event;
    entry.timestamp = comp->elapsed;
    entry.impact    = m_impact;
    comp->event_log.push_back(entry);

    // Trim oldest entries if log exceeds max
    while (static_cast<int>(comp->event_log.size()) > comp->max_event_log) {
        comp->event_log.erase(comp->event_log.begin());
    }

    comp->total_events++;
    return true;
}

bool FleetMoraleSystem::clearEventLog(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->event_log.clear();
    return true;
}

bool FleetMoraleSystem::boostMorale(const std::string& entity_id, float amount) {
    if (amount <= 0.0f) return false;
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->morale = clampMorale(comp->morale + amount);
    return true;
}

bool FleetMoraleSystem::reduceMorale(const std::string& entity_id, float amount) {
    if (amount <= 0.0f) return false;
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->morale = clampMorale(comp->morale - amount);
    return true;
}

bool FleetMoraleSystem::resetMorale(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->morale = comp->morale_baseline;
    return true;
}

bool FleetMoraleSystem::boostCohesion(const std::string& entity_id, float amount) {
    if (amount <= 0.0f) return false;
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->cohesion = clampMorale(comp->cohesion + amount);
    return true;
}

bool FleetMoraleSystem::reduceCohesion(const std::string& entity_id, float amount) {
    if (amount <= 0.0f) return false;
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->cohesion = clampMorale(comp->cohesion - amount);
    return true;
}

bool FleetMoraleSystem::setFleetId(const std::string& entity_id,
                                    const std::string& fleet_id) {
    if (fleet_id.empty()) return false;
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->fleet_id = fleet_id;
    return true;
}

bool FleetMoraleSystem::setMoraleDecay(const std::string& entity_id, float rate) {
    if (rate <= 0.0f || rate > 1.0f) return false;
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->morale_decay = rate;
    return true;
}

bool FleetMoraleSystem::setCohesionDecay(const std::string& entity_id, float rate) {
    if (rate <= 0.0f || rate > 1.0f) return false;
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->cohesion_decay = rate;
    return true;
}

bool FleetMoraleSystem::setMoraleBaseline(const std::string& entity_id, float value) {
    if (value < 0.0f || value > 1.0f) return false;
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->morale_baseline = value;
    return true;
}

bool FleetMoraleSystem::setCohesionBaseline(const std::string& entity_id, float value) {
    if (value < 0.0f || value > 1.0f) return false;
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->cohesion_baseline = value;
    return true;
}

bool FleetMoraleSystem::setMaxEventLog(const std::string& entity_id, int max_size) {
    if (max_size < 1) return false;
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->max_event_log = max_size;
    while (static_cast<int>(comp->event_log.size()) > comp->max_event_log) {
        comp->event_log.erase(comp->event_log.begin());
    }
    return true;
}

float FleetMoraleSystem::getMorale(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->morale : 0.0f;
}

float FleetMoraleSystem::getCohesion(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->cohesion : 0.0f;
}

int FleetMoraleSystem::getEventCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->event_log.size()) : 0;
}

int FleetMoraleSystem::getTotalEvents(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_events : 0;
}

int FleetMoraleSystem::getVictories(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->victories : 0;
}

int FleetMoraleSystem::getDefeats(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->defeats : 0;
}

bool FleetMoraleSystem::isHighMorale(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->morale > 0.7f : false;
}

bool FleetMoraleSystem::isLowMorale(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->morale < 0.3f : false;
}

bool FleetMoraleSystem::isHighCohesion(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->cohesion > 0.7f : false;
}

bool FleetMoraleSystem::isLowCohesion(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->cohesion < 0.3f : false;
}

std::string FleetMoraleSystem::getFleetId(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->fleet_id : std::string();
}

// ── FleetMorale convenience API ───────────────────────────────────────────

static components::FleetMorale* ensureFleetMorale(ecs::World* world,
                                                   const std::string& entity_id) {
    auto* entity = world->getEntity(entity_id);
    if (!entity) return nullptr;
    auto* morale = entity->getComponent<components::FleetMorale>();
    if (!morale) {
        entity->addComponent(std::make_unique<components::FleetMorale>());
        morale = entity->getComponent<components::FleetMorale>();
    }
    return morale;
}

bool FleetMoraleSystem::recordWin(const std::string& entity_id) {
    auto* morale = ensureFleetMorale(world_, entity_id);
    if (!morale) return false;
    morale->wins++;
    morale->updateMoraleScore();
    return true;
}

bool FleetMoraleSystem::recordLoss(const std::string& entity_id) {
    auto* morale = ensureFleetMorale(world_, entity_id);
    if (!morale) return false;
    morale->losses++;
    morale->updateMoraleScore();
    return true;
}

bool FleetMoraleSystem::recordShipLost(const std::string& entity_id) {
    auto* morale = ensureFleetMorale(world_, entity_id);
    if (!morale) return false;
    morale->ships_lost++;
    morale->updateMoraleScore();
    return true;
}

bool FleetMoraleSystem::recordSavedByPlayer(const std::string& entity_id) {
    auto* morale = ensureFleetMorale(world_, entity_id);
    if (!morale) return false;
    morale->times_saved_by_player++;
    morale->updateMoraleScore();
    return true;
}

bool FleetMoraleSystem::recordMissionTogether(const std::string& entity_id) {
    auto* morale = ensureFleetMorale(world_, entity_id);
    if (!morale) return false;
    morale->missions_together++;
    return true;
}

float FleetMoraleSystem::getMoraleScore(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0.0f;
    auto* morale = entity->getComponent<components::FleetMorale>();
    return morale ? morale->morale_score : 0.0f;
}

std::string FleetMoraleSystem::getMoraleState(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return "Steady";
    auto* morale = entity->getComponent<components::FleetMorale>();
    return morale ? morale->morale_state : "Steady";
}

} // namespace systems
} // namespace atlas
