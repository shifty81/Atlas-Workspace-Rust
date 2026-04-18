#ifndef NOVAFORGE_SYSTEMS_SPACE_SCAR_SYSTEM_H
#define NOVAFORGE_SYSTEMS_SPACE_SCAR_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/exploration_components.h"
#include <string>

namespace atlas {
namespace systems {

class SpaceScarSystem
    : public ecs::SingleComponentSystem<components::SpaceScarState> {
public:
    explicit SpaceScarSystem(ecs::World* world);
    ~SpaceScarSystem() override = default;

    std::string getName() const override { return "SpaceScarSystem"; }

    bool initialize(const std::string& entity_id);

    bool addScar(const std::string& entity_id,
                 const std::string& scar_id,
                 const std::string& name,
                 components::SpaceScarState::ScarType scar_type,
                 components::SpaceScarState::DiscoverySource discovery_source,
                 const std::string& location_label,
                 const std::string& first_discoverer);
    bool removeScar(const std::string& entity_id, const std::string& scar_id);
    bool clearScars(const std::string& entity_id);
    bool nameScar(const std::string& entity_id,
                  const std::string& scar_id,
                  const std::string& new_name,
                  bool official);
    bool recordMention(const std::string& entity_id, const std::string& scar_id);
    bool addNote(const std::string& entity_id,
                 const std::string& scar_id,
                 const std::string& note);
    bool setSystemId(const std::string& entity_id, const std::string& system_id);
    bool setMaxScars(const std::string& entity_id, int max);

    int         getScarCount(const std::string& entity_id) const;
    bool        hasScar(const std::string& entity_id, const std::string& scar_id) const;
    std::string getScarName(const std::string& entity_id, const std::string& scar_id) const;
    components::SpaceScarState::ScarType getScarType(const std::string& entity_id, const std::string& scar_id) const;
    components::SpaceScarState::DiscoverySource getDiscoverySource(const std::string& entity_id, const std::string& scar_id) const;
    std::string getLocationLabel(const std::string& entity_id, const std::string& scar_id) const;
    std::string getFirstDiscoverer(const std::string& entity_id, const std::string& scar_id) const;
    int         getMentionCount(const std::string& entity_id, const std::string& scar_id) const;
    bool        isOfficiallyNamed(const std::string& entity_id, const std::string& scar_id) const;
    std::string getScarNotes(const std::string& entity_id, const std::string& scar_id) const;
    int         getTotalDiscovered(const std::string& entity_id) const;
    int         getTotalMentions(const std::string& entity_id) const;
    std::string getSystemId(const std::string& entity_id) const;
    int         getMaxScars(const std::string& entity_id) const;
    int         getCountByType(const std::string& entity_id,
                               components::SpaceScarState::ScarType type) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::SpaceScarState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_SPACE_SCAR_SYSTEM_H
