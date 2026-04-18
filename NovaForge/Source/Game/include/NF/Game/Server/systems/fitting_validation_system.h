#ifndef NOVAFORGE_SYSTEMS_FITTING_VALIDATION_SYSTEM_H
#define NOVAFORGE_SYSTEMS_FITTING_VALIDATION_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/ship_components.h"
#include <string>

namespace atlas {
namespace systems {

class FittingValidationSystem
    : public ecs::SingleComponentSystem<components::FittingValidationState> {
public:
    explicit FittingValidationSystem(ecs::World* world);
    ~FittingValidationSystem() override = default;

    std::string getName() const override { return "FittingValidationSystem"; }

    // --- Lifecycle ---
    bool initialize(const std::string& entity_id);

    // --- Ship configuration ---
    bool setShipStats(const std::string& entity_id,
                      float total_cpu,
                      float total_powergrid,
                      int high_slots,
                      int medium_slots,
                      int low_slots,
                      int rig_slots);
    bool setShipTypeId(const std::string& entity_id,
                       const std::string& ship_type_id);
    bool setCalibrationTotal(const std::string& entity_id, int total);

    // --- Module management ---
    bool fitModule(const std::string& entity_id,
                   const std::string& module_id,
                   const std::string& module_name,
                   components::FittingValidationState::SlotType slot_type,
                   int slot_index,
                   float cpu_usage,
                   float powergrid_usage);
    bool unfitModule(const std::string& entity_id,
                     const std::string& module_id);
    bool clearFitting(const std::string& entity_id);
    bool setModuleMetaLevel(const std::string& entity_id,
                            const std::string& module_id,
                            int meta_level);
    bool setModuleSkillRequirement(const std::string& entity_id,
                                   const std::string& module_id,
                                   const std::string& skill,
                                   int level);
    bool addCalibrationUsage(const std::string& entity_id,
                             const std::string& module_id,
                             int cost);

    // --- Validation ---
    bool validateFitting(const std::string& entity_id);

    // --- Queries ---
    float getCpuUsed(const std::string& entity_id) const;
    float getCpuRemaining(const std::string& entity_id) const;
    float getPowergridUsed(const std::string& entity_id) const;
    float getPowergridRemaining(const std::string& entity_id) const;
    int   getModuleCount(const std::string& entity_id) const;
    bool  hasModule(const std::string& entity_id,
                    const std::string& module_id) const;
    int   getSlotsUsed(const std::string& entity_id,
                       components::FittingValidationState::SlotType type) const;
    int   getSlotsTotal(const std::string& entity_id,
                        components::FittingValidationState::SlotType type) const;
    int   getCalibrationRemaining(const std::string& entity_id) const;
    bool  isSlotAvailable(const std::string& entity_id,
                          components::FittingValidationState::SlotType type,
                          int slot_index) const;
    int   getTotalValidations(const std::string& entity_id) const;
    int   getTotalFitsApplied(const std::string& entity_id) const;
    int   getTotalModulesRejected(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::FittingValidationState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_FITTING_VALIDATION_SYSTEM_H
