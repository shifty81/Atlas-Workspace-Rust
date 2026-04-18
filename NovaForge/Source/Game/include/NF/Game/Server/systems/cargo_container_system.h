#ifndef NOVAFORGE_SYSTEMS_CARGO_CONTAINER_SYSTEM_H
#define NOVAFORGE_SYSTEMS_CARGO_CONTAINER_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/economy_components.h"
#include <string>

namespace atlas {
namespace systems {

class CargoContainerSystem
    : public ecs::SingleComponentSystem<components::CargoContainerState> {
public:
    explicit CargoContainerSystem(ecs::World* world);
    ~CargoContainerSystem() override = default;

    std::string getName() const override { return "CargoContainerSystem"; }

    // --- Lifecycle ---
    bool initialize(const std::string& entity_id);

    // --- Container configuration ---
    bool setContainerType(const std::string& entity_id,
                          components::CargoContainerState::ContainerType type);
    bool setOwner(const std::string& entity_id,
                  const std::string& owner_id);
    bool setPassword(const std::string& entity_id,
                     const std::string& password);
    bool setCapacity(const std::string& entity_id, float capacity);
    bool setLifetime(const std::string& entity_id, float lifetime);

    // --- Item management ---
    bool addItem(const std::string& entity_id,
                 const std::string& item_id,
                 const std::string& item_name,
                 int quantity,
                 float volume_per_unit);
    bool removeItem(const std::string& entity_id,
                    const std::string& item_id,
                    int quantity);
    bool clearItems(const std::string& entity_id);

    // --- Anchoring ---
    bool anchor(const std::string& entity_id);
    bool unanchor(const std::string& entity_id);

    // --- Queries ---
    int         getItemCount(const std::string& entity_id) const;
    bool        hasItem(const std::string& entity_id,
                        const std::string& item_id) const;
    int         getItemQuantity(const std::string& entity_id,
                                const std::string& item_id) const;
    float       getUsedVolume(const std::string& entity_id) const;
    float       getRemainingVolume(const std::string& entity_id) const;
    bool        isAnchored(const std::string& entity_id) const;
    float       getTimeRemaining(const std::string& entity_id) const;
    std::string getOwner(const std::string& entity_id) const;
    bool        isPasswordProtected(const std::string& entity_id) const;
    bool        checkPassword(const std::string& entity_id,
                              const std::string& password) const;
    int         getTotalItemsAdded(const std::string& entity_id) const;
    int         getTotalItemsRemoved(const std::string& entity_id) const;
    components::CargoContainerState::ContainerType
                getContainerType(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::CargoContainerState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_CARGO_CONTAINER_SYSTEM_H
