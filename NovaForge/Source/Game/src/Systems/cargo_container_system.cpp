#include "systems/cargo_container_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"

namespace atlas {
namespace systems {

CargoContainerSystem::CargoContainerSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void CargoContainerSystem::updateComponent(
        ecs::Entity& /*entity*/,
        components::CargoContainerState& comp,
        float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;
    if (comp.time_remaining > 0.0f) {
        comp.time_remaining -= delta_time;
        if (comp.time_remaining < 0.0f) comp.time_remaining = 0.0f;
    }
}

bool CargoContainerSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::CargoContainerState>();
    entity->addComponent(std::move(comp));
    return true;
}

// --- Container configuration ---

bool CargoContainerSystem::setContainerType(
        const std::string& entity_id,
        components::CargoContainerState::ContainerType type) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->container_type = type;
    return true;
}

bool CargoContainerSystem::setOwner(const std::string& entity_id,
                                    const std::string& owner_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->owner_id = owner_id;
    return true;
}

bool CargoContainerSystem::setPassword(const std::string& entity_id,
                                       const std::string& password) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->password = password;
    return true;
}

bool CargoContainerSystem::setCapacity(const std::string& entity_id,
                                       float capacity) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (capacity <= 0.0f) return false;
    comp->capacity = capacity;
    return true;
}

bool CargoContainerSystem::setLifetime(const std::string& entity_id,
                                       float lifetime) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (lifetime <= 0.0f) return false;
    comp->lifetime       = lifetime;
    comp->time_remaining = lifetime;
    return true;
}

// --- Item management ---

bool CargoContainerSystem::addItem(const std::string& entity_id,
                                   const std::string& item_id,
                                   const std::string& item_name,
                                   int quantity,
                                   float volume_per_unit) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (item_id.empty()) return false;
    if (quantity <= 0) return false;
    if (volume_per_unit < 0.0f) return false;

    float added_volume = quantity * volume_per_unit;
    if (comp->used_volume + added_volume > comp->capacity) return false;

    // Stack on existing item
    for (auto& item : comp->items) {
        if (item.item_id == item_id) {
            float stack_volume = quantity * volume_per_unit;
            if (comp->used_volume + stack_volume > comp->capacity) return false;
            item.quantity += quantity;
            item.volume   += stack_volume;
            comp->used_volume += stack_volume;
            comp->total_items_added += quantity;
            return true;
        }
    }

    components::CargoContainerState::CargoItem item;
    item.item_id   = item_id;
    item.item_name = item_name;
    item.quantity  = quantity;
    item.volume    = added_volume;
    comp->items.push_back(item);
    comp->used_volume += added_volume;
    comp->total_items_added += quantity;
    return true;
}

bool CargoContainerSystem::removeItem(const std::string& entity_id,
                                      const std::string& item_id,
                                      int quantity) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (quantity <= 0) return false;
    for (auto it = comp->items.begin(); it != comp->items.end(); ++it) {
        if (it->item_id == item_id) {
            if (quantity > it->quantity) return false;
            float vol_per = (it->quantity > 0)
                            ? it->volume / it->quantity
                            : 0.0f;
            float removed_vol = quantity * vol_per;
            it->quantity -= quantity;
            it->volume   -= removed_vol;
            comp->used_volume -= removed_vol;
            comp->total_items_removed += quantity;
            if (it->quantity <= 0) {
                comp->items.erase(it);
            }
            return true;
        }
    }
    return false;
}

bool CargoContainerSystem::clearItems(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->total_items_removed += comp->total_items_added
                                 - comp->total_items_removed;
    comp->items.clear();
    comp->used_volume = 0.0f;
    return true;
}

// --- Anchoring ---

bool CargoContainerSystem::anchor(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (comp->is_anchored) return false;
    if (comp->container_type ==
        components::CargoContainerState::ContainerType::Jetcan)
        return false;
    comp->is_anchored = true;
    return true;
}

bool CargoContainerSystem::unanchor(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (!comp->is_anchored) return false;
    comp->is_anchored = false;
    return true;
}

// --- Queries ---

int CargoContainerSystem::getItemCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return static_cast<int>(comp->items.size());
}

bool CargoContainerSystem::hasItem(const std::string& entity_id,
                                   const std::string& item_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& i : comp->items) {
        if (i.item_id == item_id) return true;
    }
    return false;
}

int CargoContainerSystem::getItemQuantity(const std::string& entity_id,
                                          const std::string& item_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    for (const auto& i : comp->items) {
        if (i.item_id == item_id) return i.quantity;
    }
    return 0;
}

float CargoContainerSystem::getUsedVolume(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    return comp->used_volume;
}

float CargoContainerSystem::getRemainingVolume(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    return comp->capacity - comp->used_volume;
}

bool CargoContainerSystem::isAnchored(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    return comp->is_anchored;
}

float CargoContainerSystem::getTimeRemaining(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    return comp->time_remaining;
}

std::string CargoContainerSystem::getOwner(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return "";
    return comp->owner_id;
}

bool CargoContainerSystem::isPasswordProtected(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    return !comp->password.empty();
}

bool CargoContainerSystem::checkPassword(const std::string& entity_id,
                                         const std::string& password) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (comp->password.empty()) return true;
    return comp->password == password;
}

int CargoContainerSystem::getTotalItemsAdded(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return comp->total_items_added;
}

int CargoContainerSystem::getTotalItemsRemoved(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return comp->total_items_removed;
}

components::CargoContainerState::ContainerType
CargoContainerSystem::getContainerType(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp)
        return components::CargoContainerState::ContainerType::Jetcan;
    return comp->container_type;
}

} // namespace systems
} // namespace atlas
