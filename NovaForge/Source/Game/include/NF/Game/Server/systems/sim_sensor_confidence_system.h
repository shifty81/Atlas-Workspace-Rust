#ifndef NOVAFORGE_SYSTEMS_SIM_SENSOR_CONFIDENCE_SYSTEM_H
#define NOVAFORGE_SYSTEMS_SIM_SENSOR_CONFIDENCE_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/navigation_components.h"
#include <string>

namespace atlas {
namespace systems {

class SimSensorConfidenceSystem
    : public ecs::SingleComponentSystem<components::SensorConfidenceState> {
public:
    explicit SimSensorConfidenceSystem(ecs::World* world);
    ~SimSensorConfidenceSystem() override = default;

    std::string getName() const override { return "SimSensorConfidenceSystem"; }

    // --- Lifecycle ---
    bool initialize(const std::string& entity_id);

    // --- Entry management ---
    bool addEntry(const std::string& entity_id,
                  const std::string& entry_id,
                  const std::string& target_id,
                  const std::string& ship_class_estimate,
                  float confidence,
                  float dist_min,
                  float dist_max);

    bool removeEntry(const std::string& entity_id, const std::string& entry_id);
    bool clearEntries(const std::string& entity_id);

    bool refreshEntry(const std::string& entity_id,
                      const std::string& entry_id,
                      float new_confidence);

    // --- Configuration ---
    bool setDecayRate(const std::string& entity_id, float rate);
    bool setMaxEntries(const std::string& entity_id, int max);
    bool setScannerID(const std::string& entity_id,
                      const std::string& scanner_id);

    // --- Queries ---
    int         getEntryCount(const std::string& entity_id) const;
    bool        hasEntry(const std::string& entity_id,
                         const std::string& entry_id) const;
    float       getConfidence(const std::string& entity_id,
                              const std::string& entry_id) const;
    float       getEntryAge(const std::string& entity_id,
                            const std::string& entry_id) const;
    bool        isDecayed(const std::string& entity_id,
                          const std::string& entry_id) const;
    std::string getShipClassEstimate(const std::string& entity_id,
                                     const std::string& entry_id) const;
    float       getDistanceMin(const std::string& entity_id,
                               const std::string& entry_id) const;
    float       getDistanceMax(const std::string& entity_id,
                               const std::string& entry_id) const;
    int         getActiveEntryCount(const std::string& entity_id) const;
    int         getTotalEntriesRecorded(const std::string& entity_id) const;
    int         getTotalHighConfidence(const std::string& entity_id) const;
    std::string getScannerID(const std::string& entity_id) const;
    float       getDecayRate(const std::string& entity_id) const;
    int         getMaxEntries(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::SensorConfidenceState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_SIM_SENSOR_CONFIDENCE_SYSTEM_H
