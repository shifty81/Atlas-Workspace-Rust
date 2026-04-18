#include "systems/captain_mood_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>
#include <sstream>

namespace atlas {
namespace systems {

CaptainMoodSystem::CaptainMoodSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void CaptainMoodSystem::updateComponent(
        ecs::Entity& /*entity*/,
        components::CaptainMoodState& comp,
        float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;

    // Age all history entries
    for (auto& ev : comp.mood_history) {
        ev.age_seconds += delta_time;
    }

    // Decay mood intensity toward 0
    if (comp.mood_intensity > 0.0f) {
        comp.mood_intensity = std::max(0.0f,
            comp.mood_intensity - comp.decay_rate * delta_time);
    }
    // Revert to Neutral when intensity falls below threshold
    if (comp.mood_intensity < comp.mood_threshold &&
        comp.current_mood != components::CaptainMood::Neutral) {
        comp.current_mood   = components::CaptainMood::Neutral;
        comp.mood_intensity = 0.0f;
    }
}

bool CaptainMoodSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::CaptainMoodState>();
    entity->addComponent(std::move(comp));
    return true;
}

void CaptainMoodSystem::applyMoodEvent(
        components::CaptainMoodState& comp,
        const std::string& event_id,
        components::CaptainMood mood,
        float intensity) {
    comp.current_mood   = mood;
    comp.mood_intensity = std::clamp(intensity, 0.0f, 1.0f);
    ++comp.total_events_logged;

    // Record in history, capping at max_history
    components::MoodEvent ev;
    ev.event_id    = event_id;
    ev.mood_result = mood;
    ev.intensity   = comp.mood_intensity;
    ev.age_seconds = 0.0f;
    comp.mood_history.push_back(ev);
    if (static_cast<int>(comp.mood_history.size()) > comp.max_history) {
        comp.mood_history.erase(comp.mood_history.begin());
    }
}

bool CaptainMoodSystem::applyVictory(const std::string& entity_id, float intensity) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (intensity <= 0.0f || intensity > 1.0f) return false;
    applyMoodEvent(*comp, "victory", components::CaptainMood::Confident, intensity);
    return true;
}

bool CaptainMoodSystem::applySetback(const std::string& entity_id, float intensity) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (intensity <= 0.0f || intensity > 1.0f) return false;
    applyMoodEvent(*comp, "setback", components::CaptainMood::Frustrated, intensity);
    return true;
}

bool CaptainMoodSystem::applyNearDeath(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    applyMoodEvent(*comp, "near_death", components::CaptainMood::Anxious, 0.9f);
    return true;
}

bool CaptainMoodSystem::applyComradeship(const std::string& entity_id, float intensity) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (intensity <= 0.0f || intensity > 1.0f) return false;
    applyMoodEvent(*comp, "comradeship", components::CaptainMood::Confident, intensity);
    return true;
}

bool CaptainMoodSystem::applyInsult(const std::string& entity_id, float intensity) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (intensity <= 0.0f || intensity > 1.0f) return false;
    applyMoodEvent(*comp, "insult", components::CaptainMood::Frustrated, intensity);
    return true;
}

bool CaptainMoodSystem::applyFocus(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    applyMoodEvent(*comp, "focus", components::CaptainMood::Focused, 0.8f);
    return true;
}

bool CaptainMoodSystem::applyElation(const std::string& entity_id, float intensity) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (intensity <= 0.0f || intensity > 1.0f) return false;
    applyMoodEvent(*comp, "elation", components::CaptainMood::Elated, intensity);
    return true;
}

bool CaptainMoodSystem::resetMood(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->current_mood   = components::CaptainMood::Neutral;
    comp->mood_intensity = 0.0f;
    return true;
}

bool CaptainMoodSystem::setDecayRate(const std::string& entity_id, float rate) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (rate < 0.0f) return false;
    comp->decay_rate = rate;
    return true;
}

bool CaptainMoodSystem::setMoodThreshold(const std::string& entity_id, float threshold) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (threshold < 0.0f || threshold > 1.0f) return false;
    comp->mood_threshold = threshold;
    return true;
}

bool CaptainMoodSystem::setMaxHistory(const std::string& entity_id, int max) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (max < 1) return false;
    comp->max_history = max;
    return true;
}

bool CaptainMoodSystem::setCaptainId(const std::string& entity_id,
                                      const std::string& captain_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (captain_id.empty()) return false;
    comp->captain_id = captain_id;
    return true;
}

components::CaptainMood CaptainMoodSystem::getMood(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->current_mood : components::CaptainMood::Neutral;
}

float CaptainMoodSystem::getMoodIntensity(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->mood_intensity : 0.0f;
}

float CaptainMoodSystem::getDecayRate(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->decay_rate : 0.0f;
}

float CaptainMoodSystem::getMoodThreshold(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->mood_threshold : 0.0f;
}

bool CaptainMoodSystem::isPositiveMood(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    return comp->current_mood == components::CaptainMood::Confident ||
           comp->current_mood == components::CaptainMood::Elated    ||
           comp->current_mood == components::CaptainMood::Focused;
}

bool CaptainMoodSystem::isNegativeMood(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    return comp->current_mood == components::CaptainMood::Frustrated ||
           comp->current_mood == components::CaptainMood::Anxious    ||
           comp->current_mood == components::CaptainMood::Tense;
}

bool CaptainMoodSystem::isNeutral(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return true;
    return comp->current_mood == components::CaptainMood::Neutral;
}

std::string CaptainMoodSystem::getMoodLabel(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return "Neutral";
    switch (comp->current_mood) {
        case components::CaptainMood::Neutral:    return "Neutral";
        case components::CaptainMood::Confident:  return "Confident";
        case components::CaptainMood::Elated:     return "Elated";
        case components::CaptainMood::Focused:    return "Focused";
        case components::CaptainMood::Tense:      return "Tense";
        case components::CaptainMood::Frustrated: return "Frustrated";
        case components::CaptainMood::Anxious:    return "Anxious";
        default:                                   return "Neutral";
    }
}

int CaptainMoodSystem::getMoodHistoryCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->mood_history.size()) : 0;
}

int CaptainMoodSystem::getTotalEventsLogged(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_events_logged : 0;
}

std::string CaptainMoodSystem::getCaptainId(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->captain_id : "";
}

} // namespace systems
} // namespace atlas
