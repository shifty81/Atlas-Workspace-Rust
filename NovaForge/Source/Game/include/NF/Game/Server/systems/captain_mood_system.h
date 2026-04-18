#ifndef NOVAFORGE_SYSTEMS_CAPTAIN_MOOD_SYSTEM_H
#define NOVAFORGE_SYSTEMS_CAPTAIN_MOOD_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/social_components.h"
#include <string>

namespace atlas {
namespace systems {

// CaptainMoodSystem — Phase B (Fleet Personality)
// Tracks short-term per-captain mood driven by discrete events (victories,
// setbacks, near-deaths, comradeship, insults). Mood decays back toward
// Neutral over time. Unlike EmotionalArcSystem (long-term arcs across
// campaigns) this captures the *immediate* psychological state that drives
// chatter tone, response latency, and reaction quality in the current session.
class CaptainMoodSystem
    : public ecs::SingleComponentSystem<components::CaptainMoodState> {
public:
    explicit CaptainMoodSystem(ecs::World* world);
    ~CaptainMoodSystem() override = default;

    std::string getName() const override { return "CaptainMoodSystem"; }

    bool initialize(const std::string& entity_id);

    // Mood events — each sets the mood, records a history entry, resets intensity
    bool applyVictory(const std::string& entity_id, float intensity);
    bool applySetback(const std::string& entity_id, float intensity);
    bool applyNearDeath(const std::string& entity_id);
    bool applyComradeship(const std::string& entity_id, float intensity);
    bool applyInsult(const std::string& entity_id, float intensity);
    bool applyFocus(const std::string& entity_id);
    bool applyElation(const std::string& entity_id, float intensity);
    bool resetMood(const std::string& entity_id);

    // Configuration
    bool setDecayRate(const std::string& entity_id, float rate);
    bool setMoodThreshold(const std::string& entity_id, float threshold);
    bool setMaxHistory(const std::string& entity_id, int max);
    bool setCaptainId(const std::string& entity_id, const std::string& captain_id);

    // Queries
    components::CaptainMood getMood(const std::string& entity_id) const;
    float       getMoodIntensity(const std::string& entity_id) const;
    float       getDecayRate(const std::string& entity_id) const;
    float       getMoodThreshold(const std::string& entity_id) const;
    bool        isPositiveMood(const std::string& entity_id) const;
    bool        isNegativeMood(const std::string& entity_id) const;
    bool        isNeutral(const std::string& entity_id) const;
    std::string getMoodLabel(const std::string& entity_id) const;
    int         getMoodHistoryCount(const std::string& entity_id) const;
    int         getTotalEventsLogged(const std::string& entity_id) const;
    std::string getCaptainId(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::CaptainMoodState& comp,
                         float delta_time) override;

private:
    void applyMoodEvent(components::CaptainMoodState& comp,
                        const std::string& event_id,
                        components::CaptainMood mood,
                        float intensity);
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_CAPTAIN_MOOD_SYSTEM_H
