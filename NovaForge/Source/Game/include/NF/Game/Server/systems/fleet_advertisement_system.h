#ifndef NOVAFORGE_SYSTEMS_FLEET_ADVERTISEMENT_SYSTEM_H
#define NOVAFORGE_SYSTEMS_FLEET_ADVERTISEMENT_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/social_components.h"
#include <string>

namespace atlas {
namespace systems {

class FleetAdvertisementSystem
    : public ecs::SingleComponentSystem<components::FleetAdvertisementState> {
public:
    explicit FleetAdvertisementSystem(ecs::World* world);
    ~FleetAdvertisementSystem() override = default;

    std::string getName() const override { return "FleetAdvertisementSystem"; }

    // --- Lifecycle ---
    bool initialize(const std::string& entity_id);

    // --- Ad management ---
    bool postAd(const std::string& entity_id,
                const std::string& title,
                const std::string& description,
                components::FleetAdvertisementState::FleetType type);
    bool delistAd(const std::string& entity_id);

    // --- Configuration ---
    bool setFleetId(const std::string& entity_id,
                    const std::string& fleet_id);
    bool setBossName(const std::string& entity_id,
                     const std::string& boss_name);
    bool setTtl(const std::string& entity_id, float ttl);
    bool setMinMembers(const std::string& entity_id, int min_members);
    bool setMaxMembers(const std::string& entity_id, int max_members);
    bool setMaxApplications(const std::string& entity_id, int max_apps);
    bool setCurrentMembers(const std::string& entity_id, int count);

    // --- Application management ---
    bool applyToFleet(const std::string& entity_id,
                      const std::string& app_id,
                      const std::string& pilot_name,
                      const std::string& ship_type);
    bool acceptApplication(const std::string& entity_id,
                           const std::string& app_id);
    bool rejectApplication(const std::string& entity_id,
                           const std::string& app_id);
    bool removeApplication(const std::string& entity_id,
                           const std::string& app_id);
    bool clearApplications(const std::string& entity_id);

    // --- Queries ---
    int         getApplicationCount(const std::string& entity_id) const;
    int         getPendingCount(const std::string& entity_id) const;
    bool        hasApplication(const std::string& entity_id,
                               const std::string& app_id) const;
    bool        isListed(const std::string& entity_id) const;
    float       getTimeRemaining(const std::string& entity_id) const;
    std::string getTitle(const std::string& entity_id) const;
    std::string getBossName(const std::string& entity_id) const;
    std::string getFleetId(const std::string& entity_id) const;
    int         getCurrentMembers(const std::string& entity_id) const;
    int         getMaxMembers(const std::string& entity_id) const;
    int         getTotalAdsPosted(const std::string& entity_id) const;
    int         getTotalApplicationsReceived(const std::string& entity_id) const;
    int         getTotalAccepted(const std::string& entity_id) const;
    int         getTotalRejected(const std::string& entity_id) const;
    components::FleetAdvertisementState::FleetType
                getFleetType(const std::string& entity_id) const;
    components::FleetAdvertisementState::AppStatus
                getApplicationStatus(const std::string& entity_id,
                                     const std::string& app_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::FleetAdvertisementState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_FLEET_ADVERTISEMENT_SYSTEM_H
