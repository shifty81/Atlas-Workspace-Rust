#include "systems/structure_browser_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"

namespace atlas {
namespace systems {

StructureBrowserSystem::StructureBrowserSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void StructureBrowserSystem::updateComponent(
        ecs::Entity& /*entity*/,
        components::StructureBrowserState& comp,
        float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;
}

bool StructureBrowserSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::StructureBrowserState>();
    entity->addComponent(std::move(comp));
    return true;
}

// --- Entry management ---

bool StructureBrowserSystem::addStructure(
        const std::string& entity_id,
        const std::string& structure_id,
        const std::string& name,
        components::StructureBrowserState::StructureType type,
        const std::string& system_name,
        const std::string& owner_corp) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (structure_id.empty()) return false;
    if (name.empty()) return false;

    // Duplicate prevention
    for (const auto& e : comp->entries) {
        if (e.structure_id == structure_id) return false;
    }

    // Capacity check
    if (static_cast<int>(comp->entries.size()) >= comp->max_entries) return false;

    components::StructureBrowserState::StructureEntry entry;
    entry.structure_id = structure_id;
    entry.name         = name;
    entry.type         = type;
    entry.system_name  = system_name;
    entry.owner_corp   = owner_corp;
    comp->entries.push_back(entry);
    ++comp->total_entries_added;
    return true;
}

bool StructureBrowserSystem::removeStructure(const std::string& entity_id,
                                              const std::string& structure_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto it = comp->entries.begin(); it != comp->entries.end(); ++it) {
        if (it->structure_id == structure_id) {
            comp->entries.erase(it);
            ++comp->total_entries_removed;
            return true;
        }
    }
    return false;
}

bool StructureBrowserSystem::clearStructures(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->total_entries_removed += static_cast<int>(comp->entries.size());
    comp->entries.clear();
    return true;
}

// --- Structure modification ---

bool StructureBrowserSystem::setStructureStatus(
        const std::string& entity_id,
        const std::string& structure_id,
        components::StructureBrowserState::StructureStatus status) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto& e : comp->entries) {
        if (e.structure_id == structure_id) {
            e.status = status;
            return true;
        }
    }
    return false;
}

bool StructureBrowserSystem::setFuelRemaining(
        const std::string& entity_id,
        const std::string& structure_id,
        float hours) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (hours < 0.0f) return false;
    for (auto& e : comp->entries) {
        if (e.structure_id == structure_id) {
            e.fuel_remaining = hours;
            return true;
        }
    }
    return false;
}

bool StructureBrowserSystem::setPublic(
        const std::string& entity_id,
        const std::string& structure_id,
        bool is_public) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto& e : comp->entries) {
        if (e.structure_id == structure_id) {
            e.is_public = is_public;
            return true;
        }
    }
    return false;
}

bool StructureBrowserSystem::addService(
        const std::string& entity_id,
        const std::string& structure_id,
        const std::string& service_id,
        const std::string& service_name) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (service_id.empty()) return false;
    for (auto& e : comp->entries) {
        if (e.structure_id == structure_id) {
            // Check for duplicate service
            for (const auto& s : e.services) {
                if (s.service_id == service_id) return false;
            }
            components::StructureBrowserState::StructureService svc;
            svc.service_id   = service_id;
            svc.service_name = service_name;
            e.services.push_back(svc);
            return true;
        }
    }
    return false;
}

bool StructureBrowserSystem::removeService(
        const std::string& entity_id,
        const std::string& structure_id,
        const std::string& service_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto& e : comp->entries) {
        if (e.structure_id == structure_id) {
            for (auto it = e.services.begin(); it != e.services.end(); ++it) {
                if (it->service_id == service_id) {
                    e.services.erase(it);
                    return true;
                }
            }
            return false;
        }
    }
    return false;
}

// --- Search / filter ---

bool StructureBrowserSystem::setSearchFilter(
        const std::string& entity_id,
        const std::string& filter) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->search_filter = filter;
    ++comp->total_searches;
    return true;
}

bool StructureBrowserSystem::setTypeFilter(
        const std::string& entity_id,
        components::StructureBrowserState::StructureType type,
        bool enabled) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->type_filter     = type;
    comp->use_type_filter = enabled;
    return true;
}

// --- Queries ---

int StructureBrowserSystem::getStructureCount(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return static_cast<int>(comp->entries.size());
}

bool StructureBrowserSystem::hasStructure(
        const std::string& entity_id,
        const std::string& structure_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& e : comp->entries) {
        if (e.structure_id == structure_id) return true;
    }
    return false;
}

int StructureBrowserSystem::getFilteredCount(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    int count = 0;
    for (const auto& e : comp->entries) {
        bool match = true;
        // Name filter
        if (!comp->search_filter.empty()) {
            if (e.name.find(comp->search_filter) == std::string::npos &&
                e.system_name.find(comp->search_filter) == std::string::npos &&
                e.owner_corp.find(comp->search_filter) == std::string::npos) {
                match = false;
            }
        }
        // Type filter
        if (comp->use_type_filter && e.type != comp->type_filter) {
            match = false;
        }
        if (match) ++count;
    }
    return count;
}

int StructureBrowserSystem::getCountByType(
        const std::string& entity_id,
        components::StructureBrowserState::StructureType type) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    int count = 0;
    for (const auto& e : comp->entries) {
        if (e.type == type) ++count;
    }
    return count;
}

int StructureBrowserSystem::getCountByStatus(
        const std::string& entity_id,
        components::StructureBrowserState::StructureStatus status) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    int count = 0;
    for (const auto& e : comp->entries) {
        if (e.status == status) ++count;
    }
    return count;
}

int StructureBrowserSystem::getPublicCount(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    int count = 0;
    for (const auto& e : comp->entries) {
        if (e.is_public) ++count;
    }
    return count;
}

float StructureBrowserSystem::getFuelRemaining(
        const std::string& entity_id,
        const std::string& structure_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    for (const auto& e : comp->entries) {
        if (e.structure_id == structure_id) return e.fuel_remaining;
    }
    return 0.0f;
}

int StructureBrowserSystem::getServiceCount(
        const std::string& entity_id,
        const std::string& structure_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    for (const auto& e : comp->entries) {
        if (e.structure_id == structure_id) {
            return static_cast<int>(e.services.size());
        }
    }
    return 0;
}

std::string StructureBrowserSystem::getSearchFilter(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return "";
    return comp->search_filter;
}

int StructureBrowserSystem::getTotalSearches(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return comp->total_searches;
}

int StructureBrowserSystem::getTotalEntriesAdded(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return comp->total_entries_added;
}

int StructureBrowserSystem::getTotalEntriesRemoved(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return comp->total_entries_removed;
}

} // namespace systems
} // namespace atlas
