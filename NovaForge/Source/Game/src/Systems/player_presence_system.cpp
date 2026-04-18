#include "systems/player_presence_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

PlayerPresenceSystem::PlayerPresenceSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void PlayerPresenceSystem::updateComponent(
        ecs::Entity& /*entity*/,
        components::PlayerPresenceState& comp,
        float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;

    // Advance silence timer
    comp.time_since_last_action += delta_time;

    // Detect silence transition
    bool was_silent = comp.is_silent;
    comp.is_silent = (comp.time_since_last_action >= comp.silence_threshold);
    if (comp.is_silent && !was_silent) {
        ++comp.silence_streak;
    }

    // Advance rolling engagement window
    comp.window_timer += delta_time;
    if (comp.window_timer >= comp.engagement_window) {
        comp.window_timer = 0.0f;
        comp.activity_count_in_window = 0;
    }

    // Recompute engagement score: max one action per second expected
    const float expected = comp.engagement_window;  // one action/second baseline
    comp.engagement_score = std::min(
        1.0f,
        static_cast<float>(comp.activity_count_in_window) / expected);
}

bool PlayerPresenceSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::PlayerPresenceState>();
    entity->addComponent(std::move(comp));
    return true;
}

bool PlayerPresenceSystem::recordActivity(
        const std::string& entity_id,
        components::PlayerPresenceState::ActivityType /*type*/) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->time_since_last_action = 0.0f;
    comp->is_silent = false;
    ++comp->total_commands_issued;
    ++comp->activity_count_in_window;
    return true;
}

bool PlayerPresenceSystem::resetActivity(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->time_since_last_action = 0.0f;
    comp->is_silent = false;
    comp->silence_streak = 0;
    comp->activity_count_in_window = 0;
    comp->window_timer = 0.0f;
    comp->engagement_score = 0.0f;
    return true;
}

bool PlayerPresenceSystem::setSilenceThreshold(const std::string& entity_id,
                                               float seconds) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (seconds <= 0.0f) return false;
    comp->silence_threshold = seconds;
    return true;
}

bool PlayerPresenceSystem::setEngagementWindow(const std::string& entity_id,
                                               float seconds) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (seconds <= 0.0f) return false;
    comp->engagement_window = seconds;
    return true;
}

bool PlayerPresenceSystem::setPlayerId(const std::string& entity_id,
                                       const std::string& player_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (player_id.empty()) return false;
    comp->player_id = player_id;
    return true;
}

float PlayerPresenceSystem::getTimeSinceLastAction(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    return comp->time_since_last_action;
}

bool PlayerPresenceSystem::isSilent(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    return comp->is_silent;
}

float PlayerPresenceSystem::getEngagementScore(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    return comp->engagement_score;
}

int PlayerPresenceSystem::getTotalCommandsIssued(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return comp->total_commands_issued;
}

int PlayerPresenceSystem::getSilenceStreak(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return comp->silence_streak;
}

std::string PlayerPresenceSystem::getPlayerId(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return "";
    return comp->player_id;
}

} // namespace systems
} // namespace atlas
