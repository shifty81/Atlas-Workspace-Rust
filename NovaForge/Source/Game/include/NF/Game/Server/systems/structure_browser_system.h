#ifndef NOVAFORGE_SYSTEMS_STRUCTURE_BROWSER_SYSTEM_H
#define NOVAFORGE_SYSTEMS_STRUCTURE_BROWSER_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/ui_components.h"
#include <string>

namespace atlas {
namespace systems {

class StructureBrowserSystem
    : public ecs::SingleComponentSystem<components::StructureBrowserState> {
public:
    explicit StructureBrowserSystem(ecs::World* world);
    ~StructureBrowserSystem() override = default;

    std::string getName() const override { return "StructureBrowserSystem"; }

    // --- Lifecycle ---
    bool initialize(const std::string& entity_id);

    // --- Entry management ---
    bool addStructure(const std::string& entity_id,
                      const std::string& structure_id,
                      const std::string& name,
                      components::StructureBrowserState::StructureType type,
                      const std::string& system_name,
                      const std::string& owner_corp);
    bool removeStructure(const std::string& entity_id,
                         const std::string& structure_id);
    bool clearStructures(const std::string& entity_id);

    // --- Structure modification ---
    bool setStructureStatus(const std::string& entity_id,
                            const std::string& structure_id,
                            components::StructureBrowserState::StructureStatus status);
    bool setFuelRemaining(const std::string& entity_id,
                          const std::string& structure_id,
                          float hours);
    bool setPublic(const std::string& entity_id,
                   const std::string& structure_id,
                   bool is_public);
    bool addService(const std::string& entity_id,
                    const std::string& structure_id,
                    const std::string& service_id,
                    const std::string& service_name);
    bool removeService(const std::string& entity_id,
                       const std::string& structure_id,
                       const std::string& service_id);

    // --- Search / filter ---
    bool setSearchFilter(const std::string& entity_id,
                         const std::string& filter);
    bool setTypeFilter(const std::string& entity_id,
                       components::StructureBrowserState::StructureType type,
                       bool enabled);

    // --- Queries ---
    int         getStructureCount(const std::string& entity_id) const;
    bool        hasStructure(const std::string& entity_id,
                             const std::string& structure_id) const;
    int         getFilteredCount(const std::string& entity_id) const;
    int         getCountByType(const std::string& entity_id,
                    components::StructureBrowserState::StructureType type) const;
    int         getCountByStatus(const std::string& entity_id,
                    components::StructureBrowserState::StructureStatus status) const;
    int         getPublicCount(const std::string& entity_id) const;
    float       getFuelRemaining(const std::string& entity_id,
                                 const std::string& structure_id) const;
    int         getServiceCount(const std::string& entity_id,
                                const std::string& structure_id) const;
    std::string getSearchFilter(const std::string& entity_id) const;
    int         getTotalSearches(const std::string& entity_id) const;
    int         getTotalEntriesAdded(const std::string& entity_id) const;
    int         getTotalEntriesRemoved(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::StructureBrowserState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_STRUCTURE_BROWSER_SYSTEM_H
