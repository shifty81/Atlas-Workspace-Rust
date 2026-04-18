#include "systems/medal_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"

namespace atlas {
namespace systems {

MedalSystem::MedalSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void MedalSystem::updateComponent(
        ecs::Entity& /*entity*/,
        components::MedalCollectionState& comp,
        float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;
}

bool MedalSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::MedalCollectionState>();
    entity->addComponent(std::move(comp));
    return true;
}

// --- Medal definition management ---

bool MedalSystem::createMedal(const std::string& entity_id,
                               const std::string& medal_id,
                               const std::string& name,
                               const std::string& description,
                               const std::string& creator_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (medal_id.empty()) return false;
    if (name.empty()) return false;

    // Duplicate prevention
    for (const auto& m : comp->medals) {
        if (m.medal_id == medal_id) return false;
    }

    // Capacity check
    if (static_cast<int>(comp->medals.size()) >= comp->max_medals) return false;

    components::MedalCollectionState::Medal medal;
    medal.medal_id    = medal_id;
    medal.name        = name;
    medal.description = description;
    medal.creator_id  = creator_id;
    comp->medals.push_back(medal);
    ++comp->total_medals_created;
    return true;
}

bool MedalSystem::removeMedal(const std::string& entity_id,
                               const std::string& medal_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto it = comp->medals.begin(); it != comp->medals.end(); ++it) {
        if (it->medal_id == medal_id) {
            // Also remove all awards for this medal
            auto ait = comp->awards.begin();
            while (ait != comp->awards.end()) {
                if (ait->medal_id == medal_id) {
                    ait = comp->awards.erase(ait);
                } else {
                    ++ait;
                }
            }
            comp->medals.erase(it);
            return true;
        }
    }
    return false;
}

bool MedalSystem::clearMedals(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->medals.clear();
    comp->awards.clear();
    return true;
}

// --- Award management ---

bool MedalSystem::awardMedal(const std::string& entity_id,
                              const std::string& award_id,
                              const std::string& medal_id,
                              const std::string& recipient_id,
                              const std::string& reason) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (award_id.empty()) return false;
    if (recipient_id.empty()) return false;

    // Medal must exist
    bool medal_exists = false;
    for (const auto& m : comp->medals) {
        if (m.medal_id == medal_id) { medal_exists = true; break; }
    }
    if (!medal_exists) return false;

    // Duplicate award_id prevention
    for (const auto& a : comp->awards) {
        if (a.award_id == award_id) return false;
    }

    // Capacity check
    if (static_cast<int>(comp->awards.size()) >= comp->max_awards) return false;

    components::MedalCollectionState::AwardedMedal award;
    award.award_id     = award_id;
    award.medal_id     = medal_id;
    award.recipient_id = recipient_id;
    award.reason       = reason;
    award.timestamp    = comp->elapsed;
    comp->awards.push_back(award);
    ++comp->total_awards_given;
    return true;
}

bool MedalSystem::revokeAward(const std::string& entity_id,
                               const std::string& award_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto it = comp->awards.begin(); it != comp->awards.end(); ++it) {
        if (it->award_id == award_id) {
            comp->awards.erase(it);
            ++comp->total_awards_revoked;
            return true;
        }
    }
    return false;
}

bool MedalSystem::clearAwards(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->awards.clear();
    return true;
}

// --- Configuration ---

bool MedalSystem::setCorpId(const std::string& entity_id,
                             const std::string& corp_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->corp_id = corp_id;
    return true;
}

bool MedalSystem::setMaxMedals(const std::string& entity_id, int max_medals) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (max_medals <= 0) return false;
    comp->max_medals = max_medals;
    return true;
}

bool MedalSystem::setMaxAwards(const std::string& entity_id, int max_awards) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (max_awards <= 0) return false;
    comp->max_awards = max_awards;
    return true;
}

// --- Queries ---

int MedalSystem::getMedalCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return static_cast<int>(comp->medals.size());
}

int MedalSystem::getAwardCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return static_cast<int>(comp->awards.size());
}

bool MedalSystem::hasMedal(const std::string& entity_id,
                            const std::string& medal_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& m : comp->medals) {
        if (m.medal_id == medal_id) return true;
    }
    return false;
}

bool MedalSystem::hasAward(const std::string& entity_id,
                            const std::string& award_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& a : comp->awards) {
        if (a.award_id == award_id) return true;
    }
    return false;
}

std::string MedalSystem::getCorpId(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return "";
    return comp->corp_id;
}

std::string MedalSystem::getMedalName(const std::string& entity_id,
                                       const std::string& medal_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return "";
    for (const auto& m : comp->medals) {
        if (m.medal_id == medal_id) return m.name;
    }
    return "";
}

std::string MedalSystem::getMedalDescription(const std::string& entity_id,
                                              const std::string& medal_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return "";
    for (const auto& m : comp->medals) {
        if (m.medal_id == medal_id) return m.description;
    }
    return "";
}

int MedalSystem::getTotalMedalsCreated(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return comp->total_medals_created;
}

int MedalSystem::getTotalAwardsGiven(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return comp->total_awards_given;
}

int MedalSystem::getTotalAwardsRevoked(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return comp->total_awards_revoked;
}

int MedalSystem::getAwardCountForMedal(const std::string& entity_id,
                                        const std::string& medal_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    int count = 0;
    for (const auto& a : comp->awards) {
        if (a.medal_id == medal_id) ++count;
    }
    return count;
}

int MedalSystem::getAwardCountForRecipient(const std::string& entity_id,
                                            const std::string& recipient_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    int count = 0;
    for (const auto& a : comp->awards) {
        if (a.recipient_id == recipient_id) ++count;
    }
    return count;
}

std::string MedalSystem::getAwardReason(const std::string& entity_id,
                                         const std::string& award_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return "";
    for (const auto& a : comp->awards) {
        if (a.award_id == award_id) return a.reason;
    }
    return "";
}

} // namespace systems
} // namespace atlas
