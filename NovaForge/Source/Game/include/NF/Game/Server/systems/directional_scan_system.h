#ifndef NOVAFORGE_SYSTEMS_DIRECTIONAL_SCAN_SYSTEM_H
#define NOVAFORGE_SYSTEMS_DIRECTIONAL_SCAN_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/navigation_components.h"
#include <string>

namespace atlas {
namespace systems {

class DirectionalScanSystem
    : public ecs::SingleComponentSystem<components::DirectionalScanState> {
public:
    explicit DirectionalScanSystem(ecs::World* world);
    ~DirectionalScanSystem() override = default;

    std::string getName() const override { return "DirectionalScanSystem"; }

    // --- Lifecycle ---
    bool initialize(const std::string& entity_id);

    // --- Scan control ---
    bool startScan(const std::string& entity_id);
    bool cancelScan(const std::string& entity_id);

    // --- Configuration ---
    bool setScanAngle(const std::string& entity_id, float angle_degrees);
    bool setScanRange(const std::string& entity_id, float range_au);
    bool setScanDuration(const std::string& entity_id, float duration);
    bool setCooldownDuration(const std::string& entity_id, float duration);
    bool setMaxResults(const std::string& entity_id, int max_results);

    // --- Result management ---
    bool addResult(const std::string& entity_id,
                   const std::string& result_id,
                   const std::string& object_name,
                   components::DirectionalScanState::ObjectType type,
                   float distance,
                   float bearing);
    bool removeResult(const std::string& entity_id,
                      const std::string& result_id);
    bool clearResults(const std::string& entity_id);

    // --- Queries ---
    int   getResultCount(const std::string& entity_id) const;
    bool  hasResult(const std::string& entity_id,
                    const std::string& result_id) const;
    float getScanAngle(const std::string& entity_id) const;
    float getScanRange(const std::string& entity_id) const;
    float getScanProgress(const std::string& entity_id) const;
    float getCooldownRemaining(const std::string& entity_id) const;
    bool  isScanning(const std::string& entity_id) const;
    bool  isScanComplete(const std::string& entity_id) const;
    bool  isOnCooldown(const std::string& entity_id) const;
    int   getTotalScans(const std::string& entity_id) const;
    int   getTotalObjectsFound(const std::string& entity_id) const;
    float getResultDistance(const std::string& entity_id,
                           const std::string& result_id) const;
    float getResultBearing(const std::string& entity_id,
                           const std::string& result_id) const;
    int   getCountByType(
              const std::string& entity_id,
              components::DirectionalScanState::ObjectType type) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::DirectionalScanState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_DIRECTIONAL_SCAN_SYSTEM_H
