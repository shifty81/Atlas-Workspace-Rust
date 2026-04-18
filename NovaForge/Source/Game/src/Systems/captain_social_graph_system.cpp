#include "systems/captain_social_graph_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

CaptainSocialGraphSystem::CaptainSocialGraphSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void CaptainSocialGraphSystem::updateComponent(ecs::Entity& /*entity*/,
                                                components::CaptainSocialGraphState& comp,
                                                float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;
}

bool CaptainSocialGraphSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::CaptainSocialGraphState>();
    entity->addComponent(std::move(comp));
    return true;
}

bool CaptainSocialGraphSystem::addRelationship(const std::string& entity_id,
                                                const std::string& target_captain_id,
                                                components::CaptainSocialGraphState::RelationshipType type,
                                                float initial_trust,
                                                float initial_affinity) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (target_captain_id.empty()) return false;
    if (initial_trust < 0.0f || initial_trust > 1.0f) return false;
    if (initial_affinity < -1.0f || initial_affinity > 1.0f) return false;
    if (static_cast<int>(comp->relationships.size()) >= comp->max_relationships) return false;
    for (const auto& r : comp->relationships) {
        if (r.target_captain_id == target_captain_id) return false;
    }
    components::CaptainSocialGraphState::Relationship rel;
    rel.target_captain_id = target_captain_id;
    rel.type = type;
    rel.trust = initial_trust;
    rel.affinity = initial_affinity;
    rel.interactions = 0;
    comp->relationships.push_back(rel);
    comp->total_relationships_formed++;
    if (type == components::CaptainSocialGraphState::RelationshipType::Friendship)
        comp->total_friendships++;
    if (type == components::CaptainSocialGraphState::RelationshipType::Grudge)
        comp->total_grudges++;
    return true;
}

bool CaptainSocialGraphSystem::removeRelationship(const std::string& entity_id,
                                                   const std::string& target_captain_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    auto it = std::find_if(comp->relationships.begin(), comp->relationships.end(),
        [&](const auto& r) { return r.target_captain_id == target_captain_id; });
    if (it == comp->relationships.end()) return false;
    comp->relationships.erase(it);
    return true;
}

bool CaptainSocialGraphSystem::clearRelationships(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->relationships.clear();
    return true;
}

bool CaptainSocialGraphSystem::setRelationshipType(const std::string& entity_id,
                                                     const std::string& target_captain_id,
                                                     components::CaptainSocialGraphState::RelationshipType type) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto& r : comp->relationships) {
        if (r.target_captain_id == target_captain_id) {
            r.type = type;
            if (type == components::CaptainSocialGraphState::RelationshipType::Friendship)
                comp->total_friendships++;
            if (type == components::CaptainSocialGraphState::RelationshipType::Grudge)
                comp->total_grudges++;
            return true;
        }
    }
    return false;
}

bool CaptainSocialGraphSystem::adjustTrust(const std::string& entity_id,
                                            const std::string& target_captain_id,
                                            float delta) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto& r : comp->relationships) {
        if (r.target_captain_id == target_captain_id) {
            r.trust = std::max(0.0f, std::min(1.0f, r.trust + delta));
            r.interactions++;
            return true;
        }
    }
    return false;
}

bool CaptainSocialGraphSystem::adjustAffinity(const std::string& entity_id,
                                               const std::string& target_captain_id,
                                               float delta) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto& r : comp->relationships) {
        if (r.target_captain_id == target_captain_id) {
            r.affinity = std::max(-1.0f, std::min(1.0f, r.affinity + delta));
            r.interactions++;
            return true;
        }
    }
    return false;
}

bool CaptainSocialGraphSystem::recordEvent(const std::string& entity_id,
                                            const std::string& event_id,
                                            const std::string& captain_a,
                                            const std::string& captain_b,
                                            components::CaptainSocialGraphState::EventType event_type,
                                            float impact) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (event_id.empty() || captain_a.empty() || captain_b.empty()) return false;
    if (impact < -1.0f || impact > 1.0f) return false;
    for (const auto& e : comp->events) {
        if (e.event_id == event_id) return false;
    }
    if (static_cast<int>(comp->events.size()) >= comp->max_events) {
        comp->events.erase(comp->events.begin());
    }
    components::CaptainSocialGraphState::SocialEvent evt;
    evt.event_id = event_id;
    evt.captain_a = captain_a;
    evt.captain_b = captain_b;
    evt.event_type = event_type;
    evt.impact = impact;
    comp->events.push_back(evt);
    comp->total_events_recorded++;
    return true;
}

bool CaptainSocialGraphSystem::removeEvent(const std::string& entity_id,
                                            const std::string& event_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    auto it = std::find_if(comp->events.begin(), comp->events.end(),
        [&](const auto& e) { return e.event_id == event_id; });
    if (it == comp->events.end()) return false;
    comp->events.erase(it);
    return true;
}

bool CaptainSocialGraphSystem::clearEvents(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->events.clear();
    return true;
}

bool CaptainSocialGraphSystem::setOwnerCaptainId(const std::string& entity_id,
                                                   const std::string& owner_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->owner_captain_id = owner_id;
    return true;
}

bool CaptainSocialGraphSystem::setMaxRelationships(const std::string& entity_id, int max) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (max < 0) return false;
    comp->max_relationships = max;
    return true;
}

bool CaptainSocialGraphSystem::setMaxEvents(const std::string& entity_id, int max) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (max < 0) return false;
    comp->max_events = max;
    return true;
}

int CaptainSocialGraphSystem::getRelationshipCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return static_cast<int>(comp->relationships.size());
}

int CaptainSocialGraphSystem::getEventCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return static_cast<int>(comp->events.size());
}

bool CaptainSocialGraphSystem::hasRelationship(const std::string& entity_id,
                                                const std::string& target_captain_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& r : comp->relationships) {
        if (r.target_captain_id == target_captain_id) return true;
    }
    return false;
}

bool CaptainSocialGraphSystem::hasEvent(const std::string& entity_id,
                                         const std::string& event_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& e : comp->events) {
        if (e.event_id == event_id) return true;
    }
    return false;
}

float CaptainSocialGraphSystem::getTrust(const std::string& entity_id,
                                          const std::string& target_captain_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    for (const auto& r : comp->relationships) {
        if (r.target_captain_id == target_captain_id) return r.trust;
    }
    return 0.0f;
}

float CaptainSocialGraphSystem::getAffinity(const std::string& entity_id,
                                              const std::string& target_captain_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    for (const auto& r : comp->relationships) {
        if (r.target_captain_id == target_captain_id) return r.affinity;
    }
    return 0.0f;
}

int CaptainSocialGraphSystem::getInteractionCount(const std::string& entity_id,
                                                    const std::string& target_captain_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    for (const auto& r : comp->relationships) {
        if (r.target_captain_id == target_captain_id) return r.interactions;
    }
    return 0;
}

components::CaptainSocialGraphState::RelationshipType
CaptainSocialGraphSystem::getRelationshipType(const std::string& entity_id,
                                               const std::string& target_captain_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return components::CaptainSocialGraphState::RelationshipType::Neutral;
    for (const auto& r : comp->relationships) {
        if (r.target_captain_id == target_captain_id) return r.type;
    }
    return components::CaptainSocialGraphState::RelationshipType::Neutral;
}

int CaptainSocialGraphSystem::getCountByRelationshipType(const std::string& entity_id,
                                                          components::CaptainSocialGraphState::RelationshipType type) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    int count = 0;
    for (const auto& r : comp->relationships) {
        if (r.type == type) count++;
    }
    return count;
}

int CaptainSocialGraphSystem::getCountByEventType(const std::string& entity_id,
                                                    components::CaptainSocialGraphState::EventType type) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    int count = 0;
    for (const auto& e : comp->events) {
        if (e.event_type == type) count++;
    }
    return count;
}

std::string CaptainSocialGraphSystem::getOwnerCaptainId(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return "";
    return comp->owner_captain_id;
}

int CaptainSocialGraphSystem::getTotalRelationshipsFormed(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return comp->total_relationships_formed;
}

int CaptainSocialGraphSystem::getTotalEventsRecorded(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return comp->total_events_recorded;
}

int CaptainSocialGraphSystem::getTotalGrudges(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return comp->total_grudges;
}

int CaptainSocialGraphSystem::getTotalFriendships(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return comp->total_friendships;
}

int CaptainSocialGraphSystem::getMaxRelationships(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return comp->max_relationships;
}

int CaptainSocialGraphSystem::getMaxEvents(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return comp->max_events;
}

float CaptainSocialGraphSystem::getAverageTrust(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    if (comp->relationships.empty()) return 0.0f;
    float sum = 0.0f;
    for (const auto& r : comp->relationships) {
        sum += r.trust;
    }
    return sum / static_cast<float>(comp->relationships.size());
}

} // namespace systems
} // namespace atlas
