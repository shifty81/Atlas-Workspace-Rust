#include "systems/behavioral_reputation_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>
#include <cmath>

namespace atlas {
namespace systems {

using BRState = components::BehavioralReputationState;
using BType   = BRState::BehaviorType;

BehavioralReputationSystem::BehavioralReputationSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void BehavioralReputationSystem::updateComponent(
        ecs::Entity& /*entity*/,
        BRState& comp,
        float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;
}

// static helper — applies +/- impact to the appropriate score axis
void BehavioralReputationSystem::applyImpactToScores(
        BRState& comp,
        BType type,
        float impact,
        float sign) {
    float delta = impact * sign;
    switch (type) {
        case BType::RescueShip:
        case BType::HoardResources:
            comp.generosity_score += delta;
            break;
        case BType::HelpAlly:
            comp.generosity_score += delta;
            comp.loyalty_score    += delta;
            break;
        case BType::OvercommitAlly:
        case BType::FriendlyFire:
        case BType::TacticalRetreat:
            comp.loyalty_score += delta;
            break;
        case BType::SalvageField:
        case BType::AbandonWreck:
            comp.salvage_score += delta;
            break;
        case BType::RespondDistress:
        case BType::IgnoreDistress:
            comp.distress_score += delta;
            break;
        default:
            break;
    }
}

// static helper — converts BehaviorType to a readable string
std::string BehavioralReputationSystem::behaviorTypeToString(BType type) {
    switch (type) {
        case BType::AbandonWreck:    return "AbandonWreck";
        case BType::RescueShip:      return "RescueShip";
        case BType::HoardResources:  return "HoardResources";
        case BType::OvercommitAlly:  return "OvercommitAlly";
        case BType::HelpAlly:        return "HelpAlly";
        case BType::SalvageField:    return "SalvageField";
        case BType::IgnoreDistress:  return "IgnoreDistress";
        case BType::RespondDistress: return "RespondDistress";
        case BType::FriendlyFire:    return "FriendlyFire";
        case BType::TacticalRetreat: return "TacticalRetreat";
        default:                     return "Unknown";
    }
}

bool BehavioralReputationSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<BRState>();
    entity->addComponent(std::move(comp));
    return true;
}

bool BehavioralReputationSystem::recordBehavior(const std::string& entity_id,
                                                 const std::string& record_id,
                                                 BType type,
                                                 float impact) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (record_id.empty()) return false;

    // If already exists: increment occurrence count (no score re-add)
    for (auto& r : comp->records) {
        if (r.record_id == record_id) {
            ++r.occurrence_count;
            ++comp->total_records_ever;
            return true;
        }
    }

    // Auto-purge oldest record if at capacity
    if (static_cast<int>(comp->records.size()) >= comp->max_records) {
        // Remove oldest from front and subtract its impact from scores
        const auto& oldest = comp->records.front();
        applyImpactToScores(*comp, oldest.behavior_type, oldest.impact, -1.0f);
        comp->records.erase(comp->records.begin());
    }

    // Add new record
    BRState::BehaviorRecord rec;
    rec.record_id       = record_id;
    rec.behavior_type   = type;
    rec.impact          = impact;
    rec.occurrence_count = 1;
    comp->records.push_back(rec);
    ++comp->total_records_ever;

    // Update score axes
    applyImpactToScores(*comp, type, impact, 1.0f);
    return true;
}

bool BehavioralReputationSystem::removeBehavior(const std::string& entity_id,
                                                 const std::string& record_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    auto it = std::find_if(comp->records.begin(), comp->records.end(),
        [&](const BRState::BehaviorRecord& r) {
            return r.record_id == record_id;
        });
    if (it == comp->records.end()) return false;
    applyImpactToScores(*comp, it->behavior_type, it->impact, -1.0f);
    comp->records.erase(it);
    return true;
}

bool BehavioralReputationSystem::clearRecords(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->records.clear();
    comp->generosity_score = 0.0f;
    comp->loyalty_score    = 0.0f;
    comp->salvage_score    = 0.0f;
    comp->distress_score   = 0.0f;
    return true;
}

bool BehavioralReputationSystem::setPlayerId(const std::string& entity_id,
                                              const std::string& player_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (player_id.empty()) return false;
    comp->player_id = player_id;
    return true;
}

bool BehavioralReputationSystem::setMaxRecords(const std::string& entity_id,
                                                int max) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (max < 1) return false;
    comp->max_records = max;
    return true;
}

float BehavioralReputationSystem::getGenerosityScore(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    return comp->generosity_score;
}

float BehavioralReputationSystem::getLoyaltyScore(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    return comp->loyalty_score;
}

float BehavioralReputationSystem::getSalvageScore(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    return comp->salvage_score;
}

float BehavioralReputationSystem::getDistressScore(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    return comp->distress_score;
}

float BehavioralReputationSystem::getOverallReputation(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    return (comp->generosity_score + comp->loyalty_score +
            comp->salvage_score + comp->distress_score) / 4.0f;
}

int BehavioralReputationSystem::getRecordCount(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return static_cast<int>(comp->records.size());
}

bool BehavioralReputationSystem::hasRecord(
        const std::string& entity_id,
        const std::string& record_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& r : comp->records) {
        if (r.record_id == record_id) return true;
    }
    return false;
}

int BehavioralReputationSystem::getOccurrenceCount(
        const std::string& entity_id,
        const std::string& record_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    for (const auto& r : comp->records) {
        if (r.record_id == record_id) return r.occurrence_count;
    }
    return 0;
}

float BehavioralReputationSystem::getImpact(
        const std::string& entity_id,
        const std::string& record_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    for (const auto& r : comp->records) {
        if (r.record_id == record_id) return r.impact;
    }
    return 0.0f;
}

int BehavioralReputationSystem::getTotalRecordsEver(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return comp->total_records_ever;
}

int BehavioralReputationSystem::getCountByType(
        const std::string& entity_id,
        BType type) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    int count = 0;
    for (const auto& r : comp->records) {
        if (r.behavior_type == type) ++count;
    }
    return count;
}

std::string BehavioralReputationSystem::getDominantBehaviorType(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp || comp->records.empty()) return "";

    // Find the behavior type with the largest absolute cumulative impact
    std::string dominant;
    float max_abs = -1.0f;
    for (const auto& r : comp->records) {
        float abs_imp = std::fabs(r.impact);
        if (abs_imp > max_abs) {
            max_abs  = abs_imp;
            dominant = behaviorTypeToString(r.behavior_type);
        }
    }
    return dominant;
}

std::string BehavioralReputationSystem::getPlayerId(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return "";
    return comp->player_id;
}

int BehavioralReputationSystem::getMaxRecords(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return comp->max_records;
}

} // namespace systems
} // namespace atlas
