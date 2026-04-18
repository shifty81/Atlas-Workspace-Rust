#include "systems/fleet_recon_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

FleetReconSystem::FleetReconSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void FleetReconSystem::updateComponent(
        ecs::Entity& /*entity*/,
        components::FleetReconState& comp,
        float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;

    for (auto& m : comp.missions) {
        if (m.status == components::ReconStatus::Deployed) {
            m.elapsed += delta_time;
            // Auto-return when duration exceeded
            if (m.elapsed >= m.duration) {
                m.status = components::ReconStatus::Returning;
            }
            // Mark lost if elapsed exceeds loss timeout
            if (m.elapsed >= comp.scout_loss_timeout) {
                m.status = components::ReconStatus::Scout_Lost;
                ++comp.total_scouts_lost;
            }
        } else if (m.status == components::ReconStatus::Returning) {
            m.elapsed += delta_time;
            // Arrive immediately after entering Returning (caller may set intel)
            m.status = components::ReconStatus::Intel_Ready;
            ++comp.total_missions_returned;
        }
    }
}

bool FleetReconSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::FleetReconState>();
    entity->addComponent(std::move(comp));
    return true;
}

bool FleetReconSystem::deployScout(const std::string& entity_id,
                                    const std::string& mission_id,
                                    const std::string& target_system,
                                    const std::string& scout_id,
                                    float              duration) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (mission_id.empty() || target_system.empty() || scout_id.empty()) return false;
    if (duration <= 0.0f) return false;
    // Duplicate prevention
    for (const auto& m : comp->missions) {
        if (m.mission_id == mission_id) return false;
    }
    // Capacity check
    if (static_cast<int>(comp->missions.size()) >= comp->max_missions) return false;

    components::ReconMission mission;
    mission.mission_id    = mission_id;
    mission.target_system = target_system;
    mission.scout_id      = scout_id;
    mission.duration      = duration;
    mission.elapsed       = 0.0f;
    mission.status        = components::ReconStatus::Deployed;
    comp->missions.push_back(mission);
    ++comp->total_missions_sent;
    return true;
}

bool FleetReconSystem::recallScout(const std::string& entity_id,
                                    const std::string& mission_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto& m : comp->missions) {
        if (m.mission_id == mission_id) {
            if (m.status != components::ReconStatus::Deployed) return false;
            m.status = components::ReconStatus::Returning;
            return true;
        }
    }
    return false;
}

bool FleetReconSystem::setMissionIntel(const std::string& entity_id,
                                        const std::string& mission_id,
                                        float              threat,
                                        int                ships,
                                        int                anomalies) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (threat < 0.0f || threat > 1.0f) return false;
    if (ships < 0 || anomalies < 0) return false;
    for (auto& m : comp->missions) {
        if (m.mission_id == mission_id) {
            m.intel_threat    = threat;
            m.intel_ships     = ships;
            m.intel_anomalies = anomalies;
            m.intel_consumed  = false;
            if (m.status == components::ReconStatus::Deployed ||
                m.status == components::ReconStatus::Returning) {
                m.status = components::ReconStatus::Intel_Ready;
            }
            return true;
        }
    }
    return false;
}

bool FleetReconSystem::consumeIntel(const std::string& entity_id,
                                     const std::string& mission_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto& m : comp->missions) {
        if (m.mission_id == mission_id) {
            if (m.status != components::ReconStatus::Intel_Ready) return false;
            m.intel_consumed = true;
            m.status = components::ReconStatus::Idle;
            return true;
        }
    }
    return false;
}

bool FleetReconSystem::removeMission(const std::string& entity_id,
                                      const std::string& mission_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    auto it = std::remove_if(comp->missions.begin(), comp->missions.end(),
        [&](const components::ReconMission& m) {
            return m.mission_id == mission_id;
        });
    if (it == comp->missions.end()) return false;
    comp->missions.erase(it, comp->missions.end());
    return true;
}

bool FleetReconSystem::clearMissions(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->missions.clear();
    return true;
}

bool FleetReconSystem::setMaxMissions(const std::string& entity_id, int max) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (max < 1) return false;
    comp->max_missions = max;
    return true;
}

bool FleetReconSystem::setScoutLossTimeout(const std::string& entity_id, float timeout) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (timeout <= 0.0f) return false;
    comp->scout_loss_timeout = timeout;
    return true;
}

bool FleetReconSystem::setFleetId(const std::string& entity_id,
                                   const std::string& fleet_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (fleet_id.empty()) return false;
    comp->fleet_id = fleet_id;
    return true;
}

int FleetReconSystem::getMissionCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->missions.size()) : 0;
}

bool FleetReconSystem::hasMission(const std::string& entity_id,
                                   const std::string& mission_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& m : comp->missions) {
        if (m.mission_id == mission_id) return true;
    }
    return false;
}

components::ReconStatus FleetReconSystem::getMissionStatus(
        const std::string& entity_id,
        const std::string& mission_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return components::ReconStatus::Idle;
    for (const auto& m : comp->missions) {
        if (m.mission_id == mission_id) return m.status;
    }
    return components::ReconStatus::Idle;
}

float FleetReconSystem::getMissionElapsed(const std::string& entity_id,
                                           const std::string& mission_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    for (const auto& m : comp->missions) {
        if (m.mission_id == mission_id) return m.elapsed;
    }
    return 0.0f;
}

float FleetReconSystem::getMissionDuration(const std::string& entity_id,
                                            const std::string& mission_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    for (const auto& m : comp->missions) {
        if (m.mission_id == mission_id) return m.duration;
    }
    return 0.0f;
}

float FleetReconSystem::getIntelThreat(const std::string& entity_id,
                                        const std::string& mission_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    for (const auto& m : comp->missions) {
        if (m.mission_id == mission_id) return m.intel_threat;
    }
    return 0.0f;
}

int FleetReconSystem::getIntelShips(const std::string& entity_id,
                                     const std::string& mission_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    for (const auto& m : comp->missions) {
        if (m.mission_id == mission_id) return m.intel_ships;
    }
    return 0;
}

int FleetReconSystem::getIntelAnomalies(const std::string& entity_id,
                                         const std::string& mission_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    for (const auto& m : comp->missions) {
        if (m.mission_id == mission_id) return m.intel_anomalies;
    }
    return 0;
}

bool FleetReconSystem::isIntelReady(const std::string& entity_id,
                                     const std::string& mission_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& m : comp->missions) {
        if (m.mission_id == mission_id) {
            return m.status == components::ReconStatus::Intel_Ready;
        }
    }
    return false;
}

int FleetReconSystem::getDeployedCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    int count = 0;
    for (const auto& m : comp->missions) {
        if (m.status == components::ReconStatus::Deployed) ++count;
    }
    return count;
}

int FleetReconSystem::getTotalMissionsSent(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_missions_sent : 0;
}

int FleetReconSystem::getTotalMissionsReturned(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_missions_returned : 0;
}

int FleetReconSystem::getTotalScoutsLost(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_scouts_lost : 0;
}

std::string FleetReconSystem::getFleetId(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->fleet_id : "";
}

} // namespace systems
} // namespace atlas
