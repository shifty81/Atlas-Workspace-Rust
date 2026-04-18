#include "systems/fleet_cargo_aggregator_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

FleetCargoAggregatorSystem::FleetCargoAggregatorSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void FleetCargoAggregatorSystem::updateComponent(
        ecs::Entity& /*entity*/,
        components::FleetCargoAggregatorState& comp,
        float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;
}

bool FleetCargoAggregatorSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::FleetCargoAggregatorState>();
    entity->addComponent(std::move(comp));
    return true;
}

// --- Contribution management ---

bool FleetCargoAggregatorSystem::addContribution(
        const std::string& entity_id,
        const std::string& ship_id,
        const std::string& ship_name,
        components::FleetCargoAggregatorState::ResourceType resource_type,
        float quantity,
        float capacity) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (ship_id.empty()) return false;
    if (ship_name.empty()) return false;
    if (quantity < 0.0f) return false;
    if (capacity <= 0.0f) return false;
    if (quantity > capacity) return false;
    if (static_cast<int>(comp->contributions.size()) >= comp->max_contributions) return false;
    for (const auto& c : comp->contributions) {
        if (c.ship_id == ship_id) return false;
    }
    components::FleetCargoAggregatorState::CargoContribution contrib;
    contrib.ship_id = ship_id;
    contrib.ship_name = ship_name;
    contrib.resource_type = resource_type;
    contrib.quantity = quantity;
    contrib.capacity = capacity;
    comp->contributions.push_back(contrib);
    ++comp->total_contributions_added;
    return true;
}

bool FleetCargoAggregatorSystem::removeContribution(
        const std::string& entity_id,
        const std::string& ship_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    auto it = std::find_if(comp->contributions.begin(), comp->contributions.end(),
        [&](const components::FleetCargoAggregatorState::CargoContribution& c) {
            return c.ship_id == ship_id;
        });
    if (it == comp->contributions.end()) return false;
    comp->contributions.erase(it);
    return true;
}

bool FleetCargoAggregatorSystem::clearContributions(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->contributions.clear();
    return true;
}

bool FleetCargoAggregatorSystem::updateContributionQuantity(
        const std::string& entity_id,
        const std::string& ship_id,
        float new_quantity) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (new_quantity < 0.0f) return false;
    for (auto& c : comp->contributions) {
        if (c.ship_id == ship_id) {
            if (new_quantity > c.capacity) return false;
            c.quantity = new_quantity;
            return true;
        }
    }
    return false;
}

// --- Pool management ---

bool FleetCargoAggregatorSystem::addPool(
        const std::string& entity_id,
        const std::string& pool_id,
        components::FleetCargoAggregatorState::ResourceType resource_type,
        float capacity) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (pool_id.empty()) return false;
    if (capacity <= 0.0f) return false;
    if (static_cast<int>(comp->pools.size()) >= comp->max_pools) return false;
    for (const auto& p : comp->pools) {
        if (p.pool_id == pool_id) return false;
    }
    components::FleetCargoAggregatorState::ResourcePool pool;
    pool.pool_id = pool_id;
    pool.resource_type = resource_type;
    pool.total_quantity = 0.0f;
    pool.total_capacity = capacity;
    comp->pools.push_back(pool);
    return true;
}

bool FleetCargoAggregatorSystem::removePool(
        const std::string& entity_id,
        const std::string& pool_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    auto it = std::find_if(comp->pools.begin(), comp->pools.end(),
        [&](const components::FleetCargoAggregatorState::ResourcePool& p) {
            return p.pool_id == pool_id;
        });
    if (it == comp->pools.end()) return false;
    comp->pools.erase(it);
    return true;
}

bool FleetCargoAggregatorSystem::clearPools(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->pools.clear();
    return true;
}

// --- Transfer ---

bool FleetCargoAggregatorSystem::transferToPool(
        const std::string& entity_id,
        const std::string& ship_id,
        const std::string& pool_id,
        float amount) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (amount <= 0.0f) return false;

    // Find the contribution
    auto contrib_it = std::find_if(comp->contributions.begin(), comp->contributions.end(),
        [&](const components::FleetCargoAggregatorState::CargoContribution& c) {
            return c.ship_id == ship_id;
        });
    if (contrib_it == comp->contributions.end()) return false;

    // Find the pool
    auto pool_it = std::find_if(comp->pools.begin(), comp->pools.end(),
        [&](const components::FleetCargoAggregatorState::ResourcePool& p) {
            return p.pool_id == pool_id;
        });
    if (pool_it == comp->pools.end()) return false;

    // Validate: ship has enough, pool has remaining capacity
    if (amount > contrib_it->quantity) return false;
    float remaining = pool_it->total_capacity - pool_it->total_quantity;
    if (amount > remaining) return false;

    contrib_it->quantity -= amount;
    pool_it->total_quantity += amount;
    ++comp->total_transfers;
    comp->total_quantity_transferred += amount;
    return true;
}

// --- Configuration ---

bool FleetCargoAggregatorSystem::setFleetId(
        const std::string& entity_id,
        const std::string& fleet_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (fleet_id.empty()) return false;
    comp->fleet_id = fleet_id;
    return true;
}

bool FleetCargoAggregatorSystem::setMaxContributions(
        const std::string& entity_id, int max) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (max < 1) return false;
    comp->max_contributions = max;
    return true;
}

bool FleetCargoAggregatorSystem::setMaxPools(
        const std::string& entity_id, int max) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (max < 1) return false;
    comp->max_pools = max;
    return true;
}

// --- Queries ---

int FleetCargoAggregatorSystem::getContributionCount(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return static_cast<int>(comp->contributions.size());
}

int FleetCargoAggregatorSystem::getPoolCount(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return static_cast<int>(comp->pools.size());
}

bool FleetCargoAggregatorSystem::hasContribution(
        const std::string& entity_id,
        const std::string& ship_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& c : comp->contributions) {
        if (c.ship_id == ship_id) return true;
    }
    return false;
}

bool FleetCargoAggregatorSystem::hasPool(
        const std::string& entity_id,
        const std::string& pool_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& p : comp->pools) {
        if (p.pool_id == pool_id) return true;
    }
    return false;
}

float FleetCargoAggregatorSystem::getContributionQuantity(
        const std::string& entity_id,
        const std::string& ship_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    for (const auto& c : comp->contributions) {
        if (c.ship_id == ship_id) return c.quantity;
    }
    return 0.0f;
}

float FleetCargoAggregatorSystem::getContributionCapacity(
        const std::string& entity_id,
        const std::string& ship_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    for (const auto& c : comp->contributions) {
        if (c.ship_id == ship_id) return c.capacity;
    }
    return 0.0f;
}

float FleetCargoAggregatorSystem::getPoolQuantity(
        const std::string& entity_id,
        const std::string& pool_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    for (const auto& p : comp->pools) {
        if (p.pool_id == pool_id) return p.total_quantity;
    }
    return 0.0f;
}

float FleetCargoAggregatorSystem::getPoolCapacity(
        const std::string& entity_id,
        const std::string& pool_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    for (const auto& p : comp->pools) {
        if (p.pool_id == pool_id) return p.total_capacity;
    }
    return 0.0f;
}

float FleetCargoAggregatorSystem::getTotalFleetQuantity(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    float total = 0.0f;
    for (const auto& c : comp->contributions) {
        total += c.quantity;
    }
    return total;
}

float FleetCargoAggregatorSystem::getTotalFleetCapacity(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    float total = 0.0f;
    for (const auto& c : comp->contributions) {
        total += c.capacity;
    }
    return total;
}

float FleetCargoAggregatorSystem::getQuantityByType(
        const std::string& entity_id,
        components::FleetCargoAggregatorState::ResourceType type) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    float total = 0.0f;
    for (const auto& c : comp->contributions) {
        if (c.resource_type == type) total += c.quantity;
    }
    return total;
}

int FleetCargoAggregatorSystem::getCountByType(
        const std::string& entity_id,
        components::FleetCargoAggregatorState::ResourceType type) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    int count = 0;
    for (const auto& c : comp->contributions) {
        if (c.resource_type == type) ++count;
    }
    return count;
}

std::string FleetCargoAggregatorSystem::getFleetId(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return "";
    return comp->fleet_id;
}

int FleetCargoAggregatorSystem::getTotalTransfers(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return comp->total_transfers;
}

float FleetCargoAggregatorSystem::getTotalQuantityTransferred(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    return comp->total_quantity_transferred;
}

int FleetCargoAggregatorSystem::getTotalContributionsAdded(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return comp->total_contributions_added;
}

int FleetCargoAggregatorSystem::getMaxContributions(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return comp->max_contributions;
}

int FleetCargoAggregatorSystem::getMaxPools(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return comp->max_pools;
}

std::string FleetCargoAggregatorSystem::getShipName(
        const std::string& entity_id,
        const std::string& ship_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return "";
    for (const auto& c : comp->contributions) {
        if (c.ship_id == ship_id) return c.ship_name;
    }
    return "";
}

} // namespace systems
} // namespace atlas
