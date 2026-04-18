#include "systems/captain_milestone_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

namespace {

std::string computeRank(int points) {
    if (points >= 100) return "Admiral";
    if (points >= 50)  return "Captain";
    if (points >= 30)  return "Commander";
    if (points >= 15)  return "Lieutenant";
    if (points >= 5)   return "Ensign";
    return "Recruit";
}

} // anonymous namespace

CaptainMilestoneSystem::CaptainMilestoneSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void CaptainMilestoneSystem::updateComponent(
        ecs::Entity& /*entity*/,
        components::CaptainMilestoneState& comp,
        float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;
}

bool CaptainMilestoneSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::CaptainMilestoneState>();
    comp->captain_rank = computeRank(0);
    entity->addComponent(std::move(comp));
    return true;
}

bool CaptainMilestoneSystem::addMilestone(
        const std::string& entity_id,
        const std::string& milestone_id,
        components::CaptainMilestoneState::MilestoneType type,
        const std::string& description,
        int career_points) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (milestone_id.empty()) return false;
    if (description.empty()) return false;
    if (career_points < 0) return false;
    if (static_cast<int>(comp->milestones.size()) >= comp->max_milestones) return false;
    for (const auto& m : comp->milestones) {
        if (m.milestone_id == milestone_id) return false;
    }
    components::CaptainMilestoneState::Milestone ms;
    ms.milestone_id = milestone_id;
    ms.type = type;
    ms.description = description;
    ms.is_achieved = false;
    ms.career_points_awarded = career_points;
    comp->milestones.push_back(ms);
    return true;
}

bool CaptainMilestoneSystem::achieveMilestone(
        const std::string& entity_id, const std::string& milestone_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto& m : comp->milestones) {
        if (m.milestone_id == milestone_id) {
            if (m.is_achieved) return false;
            m.is_achieved = true;
            comp->career_points += m.career_points_awarded;
            ++comp->total_achieved;
            comp->captain_rank = computeRank(comp->career_points);
            return true;
        }
    }
    return false;
}

bool CaptainMilestoneSystem::resetMilestone(
        const std::string& entity_id, const std::string& milestone_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto& m : comp->milestones) {
        if (m.milestone_id == milestone_id) {
            if (!m.is_achieved) return false;
            m.is_achieved = false;
            comp->career_points -= m.career_points_awarded;
            if (comp->career_points < 0) comp->career_points = 0;
            --comp->total_achieved;
            comp->captain_rank = computeRank(comp->career_points);
            return true;
        }
    }
    return false;
}

bool CaptainMilestoneSystem::removeMilestone(
        const std::string& entity_id, const std::string& milestone_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto it = comp->milestones.begin(); it != comp->milestones.end(); ++it) {
        if (it->milestone_id == milestone_id) {
            if (it->is_achieved) return false;
            comp->milestones.erase(it);
            return true;
        }
    }
    return false;
}

bool CaptainMilestoneSystem::clearMilestones(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->milestones.erase(
        std::remove_if(comp->milestones.begin(), comp->milestones.end(),
            [](const components::CaptainMilestoneState::Milestone& m) {
                return !m.is_achieved;
            }),
        comp->milestones.end());
    return true;
}

bool CaptainMilestoneSystem::setCaptainId(
        const std::string& entity_id, const std::string& captain_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (captain_id.empty()) return false;
    comp->captain_id = captain_id;
    return true;
}

bool CaptainMilestoneSystem::isMilestoneAchieved(
        const std::string& entity_id, const std::string& milestone_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& m : comp->milestones) {
        if (m.milestone_id == milestone_id) return m.is_achieved;
    }
    return false;
}

int CaptainMilestoneSystem::getMilestoneCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return static_cast<int>(comp->milestones.size());
}

int CaptainMilestoneSystem::getAchievedCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    int count = 0;
    for (const auto& m : comp->milestones) {
        if (m.is_achieved) ++count;
    }
    return count;
}

int CaptainMilestoneSystem::getCareerPoints(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return comp->career_points;
}

std::string CaptainMilestoneSystem::getCaptainRank(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return "";
    return comp->captain_rank;
}

bool CaptainMilestoneSystem::hasMilestone(
        const std::string& entity_id, const std::string& milestone_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& m : comp->milestones) {
        if (m.milestone_id == milestone_id) return true;
    }
    return false;
}

int CaptainMilestoneSystem::getMilestoneCareerPoints(
        const std::string& entity_id, const std::string& milestone_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    for (const auto& m : comp->milestones) {
        if (m.milestone_id == milestone_id) return m.career_points_awarded;
    }
    return 0;
}

std::string CaptainMilestoneSystem::getCaptainId(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return "";
    return comp->captain_id;
}

int CaptainMilestoneSystem::getCountByType(
        const std::string& entity_id,
        components::CaptainMilestoneState::MilestoneType type) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    int count = 0;
    for (const auto& m : comp->milestones) {
        if (m.type == type) ++count;
    }
    return count;
}

int CaptainMilestoneSystem::getTotalAchieved(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return comp->total_achieved;
}

} // namespace systems
} // namespace atlas
