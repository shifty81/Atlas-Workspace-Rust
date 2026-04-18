#include "systems/space_scar_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

SpaceScarSystem::SpaceScarSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void SpaceScarSystem::updateComponent(
        ecs::Entity& /*entity*/,
        components::SpaceScarState& comp,
        float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;
}

bool SpaceScarSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::SpaceScarState>();
    entity->addComponent(std::move(comp));
    return true;
}

bool SpaceScarSystem::addScar(const std::string& entity_id,
                               const std::string& scar_id,
                               const std::string& name,
                               components::SpaceScarState::ScarType scar_type,
                               components::SpaceScarState::DiscoverySource discovery_source,
                               const std::string& location_label,
                               const std::string& first_discoverer) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (scar_id.empty()) return false;
    if (name.empty()) return false;
    if (static_cast<int>(comp->scars.size()) >= comp->max_scars) return false;
    for (const auto& s : comp->scars) {
        if (s.scar_id == scar_id) return false;
    }
    components::SpaceScarState::SpaceScar scar;
    scar.scar_id          = scar_id;
    scar.name             = name;
    scar.scar_type        = scar_type;
    scar.discovery_source = discovery_source;
    scar.location_label   = location_label;
    scar.first_discoverer = first_discoverer;
    scar.mention_count    = 0;
    scar.is_officially_named = false;
    comp->scars.push_back(scar);
    ++comp->total_discovered;
    return true;
}

bool SpaceScarSystem::removeScar(const std::string& entity_id,
                                  const std::string& scar_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    auto it = std::find_if(comp->scars.begin(), comp->scars.end(),
        [&](const components::SpaceScarState::SpaceScar& s) {
            return s.scar_id == scar_id;
        });
    if (it == comp->scars.end()) return false;
    comp->scars.erase(it);
    return true;
}

bool SpaceScarSystem::clearScars(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->scars.clear();
    return true;
}

bool SpaceScarSystem::nameScar(const std::string& entity_id,
                                const std::string& scar_id,
                                const std::string& new_name,
                                bool official) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (new_name.empty()) return false;
    for (auto& s : comp->scars) {
        if (s.scar_id == scar_id) {
            s.name = new_name;
            s.is_officially_named = official;
            return true;
        }
    }
    return false;
}

bool SpaceScarSystem::recordMention(const std::string& entity_id,
                                     const std::string& scar_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto& s : comp->scars) {
        if (s.scar_id == scar_id) {
            ++s.mention_count;
            ++comp->total_mentions;
            return true;
        }
    }
    return false;
}

bool SpaceScarSystem::addNote(const std::string& entity_id,
                               const std::string& scar_id,
                               const std::string& note) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (note.empty()) return false;
    for (auto& s : comp->scars) {
        if (s.scar_id == scar_id) {
            s.notes = note;
            return true;
        }
    }
    return false;
}

bool SpaceScarSystem::setSystemId(const std::string& entity_id,
                                   const std::string& system_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (system_id.empty()) return false;
    comp->system_id = system_id;
    return true;
}

bool SpaceScarSystem::setMaxScars(const std::string& entity_id, int max) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (max < 1) return false;
    comp->max_scars = max;
    return true;
}

int SpaceScarSystem::getScarCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return static_cast<int>(comp->scars.size());
}

bool SpaceScarSystem::hasScar(const std::string& entity_id,
                               const std::string& scar_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& s : comp->scars) {
        if (s.scar_id == scar_id) return true;
    }
    return false;
}

std::string SpaceScarSystem::getScarName(const std::string& entity_id,
                                          const std::string& scar_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return "";
    for (const auto& s : comp->scars) {
        if (s.scar_id == scar_id) return s.name;
    }
    return "";
}

components::SpaceScarState::ScarType SpaceScarSystem::getScarType(
        const std::string& entity_id,
        const std::string& scar_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return components::SpaceScarState::ScarType::WreckField;
    for (const auto& s : comp->scars) {
        if (s.scar_id == scar_id) return s.scar_type;
    }
    return components::SpaceScarState::ScarType::WreckField;
}

components::SpaceScarState::DiscoverySource SpaceScarSystem::getDiscoverySource(
        const std::string& entity_id,
        const std::string& scar_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return components::SpaceScarState::DiscoverySource::Unknown;
    for (const auto& s : comp->scars) {
        if (s.scar_id == scar_id) return s.discovery_source;
    }
    return components::SpaceScarState::DiscoverySource::Unknown;
}

std::string SpaceScarSystem::getLocationLabel(const std::string& entity_id,
                                               const std::string& scar_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return "";
    for (const auto& s : comp->scars) {
        if (s.scar_id == scar_id) return s.location_label;
    }
    return "";
}

std::string SpaceScarSystem::getFirstDiscoverer(const std::string& entity_id,
                                                  const std::string& scar_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return "";
    for (const auto& s : comp->scars) {
        if (s.scar_id == scar_id) return s.first_discoverer;
    }
    return "";
}

int SpaceScarSystem::getMentionCount(const std::string& entity_id,
                                      const std::string& scar_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    for (const auto& s : comp->scars) {
        if (s.scar_id == scar_id) return s.mention_count;
    }
    return 0;
}

bool SpaceScarSystem::isOfficiallyNamed(const std::string& entity_id,
                                         const std::string& scar_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& s : comp->scars) {
        if (s.scar_id == scar_id) return s.is_officially_named;
    }
    return false;
}

std::string SpaceScarSystem::getScarNotes(const std::string& entity_id,
                                           const std::string& scar_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return "";
    for (const auto& s : comp->scars) {
        if (s.scar_id == scar_id) return s.notes;
    }
    return "";
}

int SpaceScarSystem::getTotalDiscovered(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return comp->total_discovered;
}

int SpaceScarSystem::getTotalMentions(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return comp->total_mentions;
}

std::string SpaceScarSystem::getSystemId(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return "";
    return comp->system_id;
}

int SpaceScarSystem::getMaxScars(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return comp->max_scars;
}

int SpaceScarSystem::getCountByType(
        const std::string& entity_id,
        components::SpaceScarState::ScarType type) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    int count = 0;
    for (const auto& s : comp->scars) {
        if (s.scar_type == type) ++count;
    }
    return count;
}

} // namespace systems
} // namespace atlas
