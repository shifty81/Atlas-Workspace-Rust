#ifndef NOVAFORGE_SYSTEMS_CAPTAIN_SOCIAL_GRAPH_SYSTEM_H
#define NOVAFORGE_SYSTEMS_CAPTAIN_SOCIAL_GRAPH_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/social_components.h"
#include <string>

namespace atlas {
namespace systems {

class CaptainSocialGraphSystem
    : public ecs::SingleComponentSystem<components::CaptainSocialGraphState> {
public:
    explicit CaptainSocialGraphSystem(ecs::World* world);
    ~CaptainSocialGraphSystem() override = default;

    std::string getName() const override { return "CaptainSocialGraphSystem"; }

    bool initialize(const std::string& entity_id);

    // Relationship management
    bool addRelationship(const std::string& entity_id,
                         const std::string& target_captain_id,
                         components::CaptainSocialGraphState::RelationshipType type,
                         float initial_trust,
                         float initial_affinity);
    bool removeRelationship(const std::string& entity_id, const std::string& target_captain_id);
    bool clearRelationships(const std::string& entity_id);
    bool setRelationshipType(const std::string& entity_id,
                              const std::string& target_captain_id,
                              components::CaptainSocialGraphState::RelationshipType type);
    bool adjustTrust(const std::string& entity_id,
                     const std::string& target_captain_id,
                     float delta);
    bool adjustAffinity(const std::string& entity_id,
                        const std::string& target_captain_id,
                        float delta);

    // Event recording
    bool recordEvent(const std::string& entity_id,
                     const std::string& event_id,
                     const std::string& captain_a,
                     const std::string& captain_b,
                     components::CaptainSocialGraphState::EventType event_type,
                     float impact);
    bool removeEvent(const std::string& entity_id, const std::string& event_id);
    bool clearEvents(const std::string& entity_id);

    // Configuration
    bool setOwnerCaptainId(const std::string& entity_id, const std::string& owner_id);
    bool setMaxRelationships(const std::string& entity_id, int max);
    bool setMaxEvents(const std::string& entity_id, int max);

    // Queries
    int         getRelationshipCount(const std::string& entity_id) const;
    int         getEventCount(const std::string& entity_id) const;
    bool        hasRelationship(const std::string& entity_id, const std::string& target_captain_id) const;
    bool        hasEvent(const std::string& entity_id, const std::string& event_id) const;
    float       getTrust(const std::string& entity_id, const std::string& target_captain_id) const;
    float       getAffinity(const std::string& entity_id, const std::string& target_captain_id) const;
    int         getInteractionCount(const std::string& entity_id, const std::string& target_captain_id) const;
    components::CaptainSocialGraphState::RelationshipType
                getRelationshipType(const std::string& entity_id, const std::string& target_captain_id) const;
    int         getCountByRelationshipType(const std::string& entity_id,
                    components::CaptainSocialGraphState::RelationshipType type) const;
    int         getCountByEventType(const std::string& entity_id,
                    components::CaptainSocialGraphState::EventType type) const;
    std::string getOwnerCaptainId(const std::string& entity_id) const;
    int         getTotalRelationshipsFormed(const std::string& entity_id) const;
    int         getTotalEventsRecorded(const std::string& entity_id) const;
    int         getTotalGrudges(const std::string& entity_id) const;
    int         getTotalFriendships(const std::string& entity_id) const;
    int         getMaxRelationships(const std::string& entity_id) const;
    int         getMaxEvents(const std::string& entity_id) const;
    float       getAverageTrust(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::CaptainSocialGraphState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_CAPTAIN_SOCIAL_GRAPH_SYSTEM_H
