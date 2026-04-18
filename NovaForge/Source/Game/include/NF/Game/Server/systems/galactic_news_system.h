#ifndef NOVAFORGE_SYSTEMS_GALACTIC_NEWS_SYSTEM_H
#define NOVAFORGE_SYSTEMS_GALACTIC_NEWS_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/exploration_components.h"
#include <string>

namespace atlas {
namespace systems {

class GalacticNewsSystem
    : public ecs::SingleComponentSystem<components::GalacticNewsState> {
public:
    explicit GalacticNewsSystem(ecs::World* world);
    ~GalacticNewsSystem() override = default;

    std::string getName() const override { return "GalacticNewsSystem"; }

    bool initialize(const std::string& entity_id);

    bool publishNews(const std::string& entity_id,
                     const std::string& entry_id,
                     const std::string& headline,
                     components::GalacticNewsState::NewsCategory category,
                     const std::string& source_system);
    bool removeNews(const std::string& entity_id, const std::string& entry_id);
    bool clearNews(const std::string& entity_id);
    bool setSystemId(const std::string& entity_id, const std::string& system_id);
    bool setDecayRate(const std::string& entity_id, float rate);
    bool setMaxEntries(const std::string& entity_id, int max);
    bool setExpiryThreshold(const std::string& entity_id, float threshold);

    int         getNewsCount(const std::string& entity_id) const;
    bool        hasNews(const std::string& entity_id, const std::string& entry_id) const;
    std::string getHeadline(const std::string& entity_id, const std::string& entry_id) const;
    float       getNewsAge(const std::string& entity_id, const std::string& entry_id) const;
    bool        isExpired(const std::string& entity_id, const std::string& entry_id) const;
    int         getActiveCount(const std::string& entity_id) const;
    int         getCountByCategory(const std::string& entity_id, components::GalacticNewsState::NewsCategory cat) const;
    int         getTotalPublished(const std::string& entity_id) const;
    int         getTotalExpired(const std::string& entity_id) const;
    std::string getSystemId(const std::string& entity_id) const;
    float       getDecayRate(const std::string& entity_id) const;
    std::string getSourceSystem(const std::string& entity_id, const std::string& entry_id) const;
    components::GalacticNewsState::NewsCategory getNewsCategory(const std::string& entity_id, const std::string& entry_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::GalacticNewsState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_GALACTIC_NEWS_SYSTEM_H
