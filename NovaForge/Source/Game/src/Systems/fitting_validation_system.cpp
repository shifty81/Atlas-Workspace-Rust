#include "systems/fitting_validation_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"

namespace atlas {
namespace systems {

FittingValidationSystem::FittingValidationSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void FittingValidationSystem::updateComponent(
        ecs::Entity& /*entity*/,
        components::FittingValidationState& comp,
        float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;
}

bool FittingValidationSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::FittingValidationState>();
    entity->addComponent(std::move(comp));
    return true;
}

// --- Ship configuration ---

bool FittingValidationSystem::setShipStats(
        const std::string& entity_id,
        float total_cpu, float total_powergrid,
        int high_slots, int medium_slots, int low_slots, int rig_slots) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (total_cpu < 0.0f || total_powergrid < 0.0f) return false;
    if (high_slots < 0 || medium_slots < 0 || low_slots < 0 || rig_slots < 0)
        return false;
    comp->total_cpu       = total_cpu;
    comp->total_powergrid = total_powergrid;
    comp->high_slots      = high_slots;
    comp->medium_slots    = medium_slots;
    comp->low_slots       = low_slots;
    comp->rig_slots       = rig_slots;
    return true;
}

bool FittingValidationSystem::setShipTypeId(
        const std::string& entity_id,
        const std::string& ship_type_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->ship_type_id = ship_type_id;
    return true;
}

bool FittingValidationSystem::setCalibrationTotal(
        const std::string& entity_id, int total) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (total < 0) return false;
    comp->calibration_total = total;
    return true;
}

// --- Module management ---

static int slotTotal(const components::FittingValidationState& comp,
                     components::FittingValidationState::SlotType type) {
    using ST = components::FittingValidationState::SlotType;
    switch (type) {
        case ST::High:   return comp.high_slots;
        case ST::Medium: return comp.medium_slots;
        case ST::Low:    return comp.low_slots;
        case ST::Rig:    return comp.rig_slots;
    }
    return 0;
}

static int slotUsed(const components::FittingValidationState& comp,
                    components::FittingValidationState::SlotType type) {
    int count = 0;
    for (const auto& m : comp.modules) {
        if (m.slot_type == type) ++count;
    }
    return count;
}

bool FittingValidationSystem::fitModule(
        const std::string& entity_id,
        const std::string& module_id,
        const std::string& module_name,
        components::FittingValidationState::SlotType slot_type,
        int slot_index,
        float cpu_usage,
        float powergrid_usage) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (module_id.empty()) return false;
    if (cpu_usage < 0.0f || powergrid_usage < 0.0f) return false;
    if (slot_index < 0) return false;

    // Duplicate check
    for (const auto& m : comp->modules) {
        if (m.module_id == module_id) {
            comp->total_modules_rejected++;
            return false;
        }
    }

    // Slot capacity check
    if (slotUsed(*comp, slot_type) >= slotTotal(*comp, slot_type)) {
        comp->total_modules_rejected++;
        return false;
    }

    // Slot index collision check
    for (const auto& m : comp->modules) {
        if (m.slot_type == slot_type && m.slot_index == slot_index) {
            comp->total_modules_rejected++;
            return false;
        }
    }

    components::FittingValidationState::FittedModule mod;
    mod.module_id       = module_id;
    mod.module_name     = module_name;
    mod.slot_type       = slot_type;
    mod.slot_index      = slot_index;
    mod.cpu_usage       = cpu_usage;
    mod.powergrid_usage = powergrid_usage;
    comp->modules.push_back(mod);
    comp->total_fits_applied++;
    return true;
}

bool FittingValidationSystem::unfitModule(const std::string& entity_id,
                                          const std::string& module_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto it = comp->modules.begin(); it != comp->modules.end(); ++it) {
        if (it->module_id == module_id) {
            comp->modules.erase(it);
            return true;
        }
    }
    return false;
}

bool FittingValidationSystem::clearFitting(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->modules.clear();
    comp->calibration_used = 0;
    return true;
}

bool FittingValidationSystem::setModuleMetaLevel(
        const std::string& entity_id,
        const std::string& module_id,
        int meta_level) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (meta_level < 0) return false;
    for (auto& m : comp->modules) {
        if (m.module_id == module_id) {
            m.meta_level = meta_level;
            return true;
        }
    }
    return false;
}

bool FittingValidationSystem::setModuleSkillRequirement(
        const std::string& entity_id,
        const std::string& module_id,
        const std::string& skill,
        int level) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (level < 1 || level > 5) return false;
    for (auto& m : comp->modules) {
        if (m.module_id == module_id) {
            m.required_skill       = skill;
            m.required_skill_level = level;
            return true;
        }
    }
    return false;
}

bool FittingValidationSystem::addCalibrationUsage(
        const std::string& entity_id,
        const std::string& module_id,
        int cost) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (cost < 0) return false;
    for (const auto& m : comp->modules) {
        if (m.module_id == module_id) {
            if (comp->calibration_used + cost > comp->calibration_total)
                return false;
            comp->calibration_used += cost;
            return true;
        }
    }
    return false;
}

// --- Validation ---

bool FittingValidationSystem::validateFitting(
        const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->total_validations++;

    float cpu_used = 0.0f;
    float pg_used  = 0.0f;
    for (const auto& m : comp->modules) {
        cpu_used += m.cpu_usage;
        pg_used  += m.powergrid_usage;
    }
    if (cpu_used > comp->total_cpu) return false;
    if (pg_used > comp->total_powergrid) return false;
    if (comp->calibration_used > comp->calibration_total) return false;

    using ST = components::FittingValidationState::SlotType;
    if (slotUsed(*comp, ST::High)   > comp->high_slots)   return false;
    if (slotUsed(*comp, ST::Medium) > comp->medium_slots)  return false;
    if (slotUsed(*comp, ST::Low)    > comp->low_slots)     return false;
    if (slotUsed(*comp, ST::Rig)    > comp->rig_slots)     return false;

    return true;
}

// --- Queries ---

float FittingValidationSystem::getCpuUsed(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    float total = 0.0f;
    for (const auto& m : comp->modules) total += m.cpu_usage;
    return total;
}

float FittingValidationSystem::getCpuRemaining(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    float used = 0.0f;
    for (const auto& m : comp->modules) used += m.cpu_usage;
    return comp->total_cpu - used;
}

float FittingValidationSystem::getPowergridUsed(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    float total = 0.0f;
    for (const auto& m : comp->modules) total += m.powergrid_usage;
    return total;
}

float FittingValidationSystem::getPowergridRemaining(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    float used = 0.0f;
    for (const auto& m : comp->modules) used += m.powergrid_usage;
    return comp->total_powergrid - used;
}

int FittingValidationSystem::getModuleCount(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return static_cast<int>(comp->modules.size());
}

bool FittingValidationSystem::hasModule(const std::string& entity_id,
                                        const std::string& module_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& m : comp->modules) {
        if (m.module_id == module_id) return true;
    }
    return false;
}

int FittingValidationSystem::getSlotsUsed(
        const std::string& entity_id,
        components::FittingValidationState::SlotType type) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return slotUsed(*comp, type);
}

int FittingValidationSystem::getSlotsTotal(
        const std::string& entity_id,
        components::FittingValidationState::SlotType type) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return slotTotal(*comp, type);
}

int FittingValidationSystem::getCalibrationRemaining(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return comp->calibration_total - comp->calibration_used;
}

bool FittingValidationSystem::isSlotAvailable(
        const std::string& entity_id,
        components::FittingValidationState::SlotType type,
        int slot_index) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (slotUsed(*comp, type) >= slotTotal(*comp, type)) return false;
    for (const auto& m : comp->modules) {
        if (m.slot_type == type && m.slot_index == slot_index) return false;
    }
    return true;
}

int FittingValidationSystem::getTotalValidations(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return comp->total_validations;
}

int FittingValidationSystem::getTotalFitsApplied(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return comp->total_fits_applied;
}

int FittingValidationSystem::getTotalModulesRejected(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return comp->total_modules_rejected;
}

} // namespace systems
} // namespace atlas
