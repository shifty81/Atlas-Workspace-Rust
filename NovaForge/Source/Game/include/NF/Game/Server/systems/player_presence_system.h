#ifndef NOVAFORGE_SYSTEMS_PLAYER_PRESENCE_SYSTEM_H
#define NOVAFORGE_SYSTEMS_PLAYER_PRESENCE_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

class PlayerPresenceSystem
    : public ecs::SingleComponentSystem<components::PlayerPresenceState> {
public:
    explicit PlayerPresenceSystem(ecs::World* world);
    ~PlayerPresenceSystem() override = default;

    std::string getName() const override { return "PlayerPresenceSystem"; }

    // --- Lifecycle ---
    bool initialize(const std::string& entity_id);

    // --- Activity recording ---
    bool recordActivity(const std::string& entity_id,
                        components::PlayerPresenceState::ActivityType type);
    bool resetActivity(const std::string& entity_id);

    // --- Configuration ---
    bool setSilenceThreshold(const std::string& entity_id, float seconds);
    bool setEngagementWindow(const std::string& entity_id, float seconds);
    bool setPlayerId(const std::string& entity_id, const std::string& player_id);

    // --- Queries ---
    float getTimeSinceLastAction(const std::string& entity_id) const;
    bool  isSilent(const std::string& entity_id) const;
    float getEngagementScore(const std::string& entity_id) const;
    int   getTotalCommandsIssued(const std::string& entity_id) const;
    int   getSilenceStreak(const std::string& entity_id) const;
    std::string getPlayerId(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::PlayerPresenceState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_PLAYER_PRESENCE_SYSTEM_H
