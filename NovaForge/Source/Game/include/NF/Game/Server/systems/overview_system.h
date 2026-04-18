#ifndef NOVAFORGE_SYSTEMS_OVERVIEW_SYSTEM_H
#define NOVAFORGE_SYSTEMS_OVERVIEW_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/ui_components.h"
#include <string>

namespace atlas {
namespace systems {

class OverviewSystem
    : public ecs::SingleComponentSystem<components::OverviewState> {
public:
    explicit OverviewSystem(ecs::World* world);
    ~OverviewSystem() override = default;

    std::string getName() const override { return "OverviewSystem"; }

    // --- Lifecycle ---
    bool initialize(const std::string& entity_id);

    // --- Entry management ---
    bool addEntry(const std::string& entity_id,
                  const std::string& entry_id,
                  const std::string& name,
                  components::OverviewState::EntryType type,
                  float distance);
    bool removeEntry(const std::string& entity_id,
                     const std::string& entry_id);
    bool clearEntries(const std::string& entity_id);
    bool updateEntryDistance(const std::string& entity_id,
                            const std::string& entry_id,
                            float distance);
    bool updateEntrySpeed(const std::string& entity_id,
                          const std::string& entry_id,
                          float speed);
    bool setEntryHostile(const std::string& entity_id,
                         const std::string& entry_id,
                         bool hostile);
    bool setEntryTargeted(const std::string& entity_id,
                          const std::string& entry_id,
                          bool targeted);

    // --- Profile management ---
    bool createProfile(const std::string& entity_id,
                       const std::string& profile_id,
                       const std::string& profile_name);
    bool deleteProfile(const std::string& entity_id,
                       const std::string& profile_id);
    bool activateProfile(const std::string& entity_id,
                         const std::string& profile_id);
    bool addTypeToProfile(const std::string& entity_id,
                          const std::string& profile_id,
                          components::OverviewState::EntryType type);
    bool removeTypeFromProfile(const std::string& entity_id,
                               const std::string& profile_id,
                               components::OverviewState::EntryType type);

    // --- Configuration ---
    bool setSortField(const std::string& entity_id,
                      components::OverviewState::SortField field);
    bool setSortAscending(const std::string& entity_id, bool ascending);
    bool setMaxRange(const std::string& entity_id, float range);

    // --- Queries ---
    int         getEntryCount(const std::string& entity_id) const;
    bool        hasEntry(const std::string& entity_id,
                         const std::string& entry_id) const;
    float       getEntryDistance(const std::string& entity_id,
                                const std::string& entry_id) const;
    std::string getEntryName(const std::string& entity_id,
                             const std::string& entry_id) const;
    bool        isEntryHostile(const std::string& entity_id,
                               const std::string& entry_id) const;
    bool        isEntryTargeted(const std::string& entity_id,
                                const std::string& entry_id) const;
    std::string getActiveProfileId(const std::string& entity_id) const;
    int         getProfileCount(const std::string& entity_id) const;
    int         getVisibleEntryCount(const std::string& entity_id) const;
    int         getTotalEntriesTracked(const std::string& entity_id) const;
    int         getTotalEntriesRemoved(const std::string& entity_id) const;
    components::OverviewState::SortField
                getSortField(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::OverviewState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_OVERVIEW_SYSTEM_H
