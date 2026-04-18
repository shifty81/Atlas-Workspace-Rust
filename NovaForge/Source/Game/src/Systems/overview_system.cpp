#include "systems/overview_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"

namespace atlas {
namespace systems {

OverviewSystem::OverviewSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void OverviewSystem::updateComponent(ecs::Entity& /*entity*/,
                                     components::OverviewState& comp,
                                     float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;
}

bool OverviewSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::OverviewState>();
    entity->addComponent(std::move(comp));
    return true;
}

// --- Entry management ---

bool OverviewSystem::addEntry(const std::string& entity_id,
                              const std::string& entry_id,
                              const std::string& name,
                              components::OverviewState::EntryType type,
                              float distance) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (entry_id.empty()) return false;
    if (distance < 0.0f) return false;
    for (const auto& e : comp->entries) {
        if (e.entry_id == entry_id) return false;
    }
    if (static_cast<int>(comp->entries.size()) >= comp->max_entries)
        return false;
    if (comp->max_range > 0.0f && distance > comp->max_range)
        return false;
    components::OverviewState::OverviewEntry entry;
    entry.entry_id = entry_id;
    entry.name     = name;
    entry.type     = type;
    entry.distance = distance;
    comp->entries.push_back(entry);
    comp->total_entries_tracked++;
    return true;
}

bool OverviewSystem::removeEntry(const std::string& entity_id,
                                 const std::string& entry_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto it = comp->entries.begin(); it != comp->entries.end(); ++it) {
        if (it->entry_id == entry_id) {
            comp->entries.erase(it);
            comp->total_entries_removed++;
            return true;
        }
    }
    return false;
}

bool OverviewSystem::clearEntries(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->total_entries_removed += static_cast<int>(comp->entries.size());
    comp->entries.clear();
    return true;
}

bool OverviewSystem::updateEntryDistance(const std::string& entity_id,
                                        const std::string& entry_id,
                                        float distance) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (distance < 0.0f) return false;
    for (auto& e : comp->entries) {
        if (e.entry_id == entry_id) {
            e.distance = distance;
            return true;
        }
    }
    return false;
}

bool OverviewSystem::updateEntrySpeed(const std::string& entity_id,
                                      const std::string& entry_id,
                                      float speed) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (speed < 0.0f) return false;
    for (auto& e : comp->entries) {
        if (e.entry_id == entry_id) {
            e.speed = speed;
            return true;
        }
    }
    return false;
}

bool OverviewSystem::setEntryHostile(const std::string& entity_id,
                                     const std::string& entry_id,
                                     bool hostile) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto& e : comp->entries) {
        if (e.entry_id == entry_id) {
            e.is_hostile = hostile;
            return true;
        }
    }
    return false;
}

bool OverviewSystem::setEntryTargeted(const std::string& entity_id,
                                      const std::string& entry_id,
                                      bool targeted) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto& e : comp->entries) {
        if (e.entry_id == entry_id) {
            e.is_targeted = targeted;
            return true;
        }
    }
    return false;
}

// --- Profile management ---

bool OverviewSystem::createProfile(const std::string& entity_id,
                                   const std::string& profile_id,
                                   const std::string& profile_name) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (profile_id.empty()) return false;
    for (const auto& p : comp->profiles) {
        if (p.profile_id == profile_id) return false;
    }
    components::OverviewState::OverviewProfile prof;
    prof.profile_id   = profile_id;
    prof.profile_name = profile_name;
    comp->profiles.push_back(prof);
    return true;
}

bool OverviewSystem::deleteProfile(const std::string& entity_id,
                                   const std::string& profile_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto it = comp->profiles.begin(); it != comp->profiles.end(); ++it) {
        if (it->profile_id == profile_id) {
            if (comp->active_profile_id == profile_id) {
                comp->active_profile_id.clear();
            }
            comp->profiles.erase(it);
            return true;
        }
    }
    return false;
}

bool OverviewSystem::activateProfile(const std::string& entity_id,
                                     const std::string& profile_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& p : comp->profiles) {
        if (p.profile_id == profile_id) {
            comp->active_profile_id = profile_id;
            return true;
        }
    }
    return false;
}

bool OverviewSystem::addTypeToProfile(
        const std::string& entity_id,
        const std::string& profile_id,
        components::OverviewState::EntryType type) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto& p : comp->profiles) {
        if (p.profile_id == profile_id) {
            for (const auto& t : p.visible_types) {
                if (t == type) return false;
            }
            p.visible_types.push_back(type);
            return true;
        }
    }
    return false;
}

bool OverviewSystem::removeTypeFromProfile(
        const std::string& entity_id,
        const std::string& profile_id,
        components::OverviewState::EntryType type) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto& p : comp->profiles) {
        if (p.profile_id == profile_id) {
            for (auto it = p.visible_types.begin();
                 it != p.visible_types.end(); ++it) {
                if (*it == type) {
                    p.visible_types.erase(it);
                    return true;
                }
            }
            return false;
        }
    }
    return false;
}

// --- Configuration ---

bool OverviewSystem::setSortField(
        const std::string& entity_id,
        components::OverviewState::SortField field) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->sort_field = field;
    return true;
}

bool OverviewSystem::setSortAscending(const std::string& entity_id,
                                      bool ascending) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->sort_ascending = ascending;
    return true;
}

bool OverviewSystem::setMaxRange(const std::string& entity_id, float range) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (range < 0.0f) return false;
    comp->max_range = range;
    return true;
}

// --- Queries ---

int OverviewSystem::getEntryCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return static_cast<int>(comp->entries.size());
}

bool OverviewSystem::hasEntry(const std::string& entity_id,
                              const std::string& entry_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& e : comp->entries) {
        if (e.entry_id == entry_id) return true;
    }
    return false;
}

float OverviewSystem::getEntryDistance(const std::string& entity_id,
                                      const std::string& entry_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    for (const auto& e : comp->entries) {
        if (e.entry_id == entry_id) return e.distance;
    }
    return 0.0f;
}

std::string OverviewSystem::getEntryName(const std::string& entity_id,
                                         const std::string& entry_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return "";
    for (const auto& e : comp->entries) {
        if (e.entry_id == entry_id) return e.name;
    }
    return "";
}

bool OverviewSystem::isEntryHostile(const std::string& entity_id,
                                    const std::string& entry_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& e : comp->entries) {
        if (e.entry_id == entry_id) return e.is_hostile;
    }
    return false;
}

bool OverviewSystem::isEntryTargeted(const std::string& entity_id,
                                     const std::string& entry_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& e : comp->entries) {
        if (e.entry_id == entry_id) return e.is_targeted;
    }
    return false;
}

std::string OverviewSystem::getActiveProfileId(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return "";
    return comp->active_profile_id;
}

int OverviewSystem::getProfileCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return static_cast<int>(comp->profiles.size());
}

int OverviewSystem::getVisibleEntryCount(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    if (comp->active_profile_id.empty()) {
        return static_cast<int>(comp->entries.size());
    }
    const components::OverviewState::OverviewProfile* active = nullptr;
    for (const auto& p : comp->profiles) {
        if (p.profile_id == comp->active_profile_id) {
            active = &p;
            break;
        }
    }
    if (!active || active->visible_types.empty()) {
        return static_cast<int>(comp->entries.size());
    }
    int count = 0;
    for (const auto& e : comp->entries) {
        for (const auto& t : active->visible_types) {
            if (t == e.type) { ++count; break; }
        }
    }
    return count;
}

int OverviewSystem::getTotalEntriesTracked(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return comp->total_entries_tracked;
}

int OverviewSystem::getTotalEntriesRemoved(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return comp->total_entries_removed;
}

components::OverviewState::SortField
OverviewSystem::getSortField(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return components::OverviewState::SortField::Distance;
    return comp->sort_field;
}

} // namespace systems
} // namespace atlas
