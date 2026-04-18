#include "systems/titan_countermeasure_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

TitanCountermeasureSystem::TitanCountermeasureSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void TitanCountermeasureSystem::updateComponent(
        ecs::Entity& /*entity*/,
        components::TitanCountermeasureState& comp,
        float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;

    // Passive awareness decay
    if (comp.pirate_awareness > 0.0f) {
        comp.pirate_awareness = std::max(0.0f,
            comp.pirate_awareness - comp.awareness_decay_rate * delta_time);
    }
}

bool TitanCountermeasureSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::TitanCountermeasureState>();
    entity->addComponent(std::move(comp));
    return true;
}

bool TitanCountermeasureSystem::executeOp(
        const std::string& entity_id,
        const std::string& op_id,
        components::CountermeasureType type,
        const std::string& target_node,
        float              effectiveness) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (op_id.empty() || target_node.empty()) return false;
    if (effectiveness < 0.0f || effectiveness > 1.0f) return false;
    // Duplicate prevention
    for (const auto& op : comp->operations) {
        if (op.op_id == op_id) return false;
    }
    // Capacity check — auto-purge oldest completed op if full
    if (static_cast<int>(comp->operations.size()) >= comp->max_operations) {
        auto it = std::find_if(comp->operations.begin(), comp->operations.end(),
            [](const components::CountermeasureOp& o) { return o.is_complete; });
        if (it == comp->operations.end()) return false; // no room
        comp->operations.erase(it);
    }

    // Compute delay contribution: sabotage ops contribute more
    float base_delay = effectiveness * 10.0f;
    if (type == components::CountermeasureType::SabotageOp) {
        base_delay *= 2.0f;
    }

    // High awareness reduces effectiveness
    float awareness_penalty = comp->pirate_awareness * 0.5f;
    float delay = base_delay * (1.0f - awareness_penalty);

    // Awareness increase from visible ops (intel ops are stealthy)
    float awareness_gain = 0.0f;
    if (type != components::CountermeasureType::IntelGathering) {
        awareness_gain = effectiveness * 0.2f;
    } else {
        awareness_gain = effectiveness * 0.05f;
    }
    comp->pirate_awareness = std::min(1.0f,
        comp->pirate_awareness + awareness_gain);

    // Record retaliation if threshold crossed during this op
    bool triggered = false;
    if (comp->pirate_awareness >= comp->awareness_threshold) {
        triggered = true;
        ++comp->retaliation_events;
    }

    components::CountermeasureOp op;
    op.op_id                = op_id;
    op.type                 = type;
    op.target_node          = target_node;
    op.effectiveness        = effectiveness;
    op.delay_contributed    = delay;
    op.is_complete          = true;
    op.triggered_awareness  = triggered;
    comp->operations.push_back(op);

    comp->titan_delay_score += delay;
    ++comp->total_ops_executed;
    if (type == components::CountermeasureType::SabotageOp) {
        ++comp->total_sabotage_ops;
    }
    if (type == components::CountermeasureType::IntelGathering) {
        ++comp->total_intel_ops;
    }
    return true;
}

bool TitanCountermeasureSystem::removeOp(const std::string& entity_id,
                                          const std::string& op_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    auto it = std::remove_if(comp->operations.begin(), comp->operations.end(),
        [&](const components::CountermeasureOp& o) { return o.op_id == op_id; });
    if (it == comp->operations.end()) return false;
    comp->operations.erase(it, comp->operations.end());
    return true;
}

bool TitanCountermeasureSystem::clearOps(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->operations.clear();
    return true;
}

bool TitanCountermeasureSystem::recordRetaliation(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    ++comp->retaliation_events;
    return true;
}

bool TitanCountermeasureSystem::applyAwarenessBoost(const std::string& entity_id,
                                                      float amount) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (amount < 0.0f) return false;
    comp->pirate_awareness = std::min(1.0f, comp->pirate_awareness + amount);
    return true;
}

bool TitanCountermeasureSystem::setMaxOperations(const std::string& entity_id,
                                                   int max) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (max < 1) return false;
    comp->max_operations = max;
    return true;
}

bool TitanCountermeasureSystem::setAwarenessDecayRate(const std::string& entity_id,
                                                        float rate) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (rate < 0.0f) return false;
    comp->awareness_decay_rate = rate;
    return true;
}

bool TitanCountermeasureSystem::setAwarenessThreshold(const std::string& entity_id,
                                                        float threshold) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (threshold < 0.0f || threshold > 1.0f) return false;
    comp->awareness_threshold = threshold;
    return true;
}

bool TitanCountermeasureSystem::setPlayerId(const std::string& entity_id,
                                             const std::string& player_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (player_id.empty()) return false;
    comp->player_id = player_id;
    return true;
}

int TitanCountermeasureSystem::getOperationCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->operations.size()) : 0;
}

bool TitanCountermeasureSystem::hasOp(const std::string& entity_id,
                                       const std::string& op_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& op : comp->operations) {
        if (op.op_id == op_id) return true;
    }
    return false;
}

float TitanCountermeasureSystem::getTitanDelayScore(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->titan_delay_score : 0.0f;
}

float TitanCountermeasureSystem::getPirateAwareness(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->pirate_awareness : 0.0f;
}

bool TitanCountermeasureSystem::isAwarenessHigh(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    return comp->pirate_awareness >= comp->awareness_threshold;
}

float TitanCountermeasureSystem::getOpEffectiveness(const std::string& entity_id,
                                                      const std::string& op_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    for (const auto& op : comp->operations) {
        if (op.op_id == op_id) return op.effectiveness;
    }
    return 0.0f;
}

float TitanCountermeasureSystem::getOpDelayContributed(
        const std::string& entity_id,
        const std::string& op_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    for (const auto& op : comp->operations) {
        if (op.op_id == op_id) return op.delay_contributed;
    }
    return 0.0f;
}

int TitanCountermeasureSystem::getTotalOpsExecuted(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_ops_executed : 0;
}

int TitanCountermeasureSystem::getTotalSabotageOps(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_sabotage_ops : 0;
}

int TitanCountermeasureSystem::getTotalIntelOps(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_intel_ops : 0;
}

int TitanCountermeasureSystem::getRetaliationEvents(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->retaliation_events : 0;
}

int TitanCountermeasureSystem::getCountByType(
        const std::string& entity_id,
        components::CountermeasureType type) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    int count = 0;
    for (const auto& op : comp->operations) {
        if (op.type == type) ++count;
    }
    return count;
}

std::string TitanCountermeasureSystem::getPlayerId(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->player_id : "";
}

} // namespace systems
} // namespace atlas
