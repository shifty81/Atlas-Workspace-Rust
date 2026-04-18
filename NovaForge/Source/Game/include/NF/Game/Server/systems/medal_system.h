#ifndef NOVAFORGE_SYSTEMS_MEDAL_SYSTEM_H
#define NOVAFORGE_SYSTEMS_MEDAL_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/social_components.h"
#include <string>

namespace atlas {
namespace systems {

class MedalSystem
    : public ecs::SingleComponentSystem<components::MedalCollectionState> {
public:
    explicit MedalSystem(ecs::World* world);
    ~MedalSystem() override = default;

    std::string getName() const override { return "MedalSystem"; }

    // --- Lifecycle ---
    bool initialize(const std::string& entity_id);

    // --- Medal definition management ---
    bool createMedal(const std::string& entity_id,
                     const std::string& medal_id,
                     const std::string& name,
                     const std::string& description,
                     const std::string& creator_id);
    bool removeMedal(const std::string& entity_id,
                     const std::string& medal_id);
    bool clearMedals(const std::string& entity_id);

    // --- Award management ---
    bool awardMedal(const std::string& entity_id,
                    const std::string& award_id,
                    const std::string& medal_id,
                    const std::string& recipient_id,
                    const std::string& reason);
    bool revokeAward(const std::string& entity_id,
                     const std::string& award_id);
    bool clearAwards(const std::string& entity_id);

    // --- Configuration ---
    bool setCorpId(const std::string& entity_id,
                   const std::string& corp_id);
    bool setMaxMedals(const std::string& entity_id, int max_medals);
    bool setMaxAwards(const std::string& entity_id, int max_awards);

    // --- Queries ---
    int         getMedalCount(const std::string& entity_id) const;
    int         getAwardCount(const std::string& entity_id) const;
    bool        hasMedal(const std::string& entity_id,
                         const std::string& medal_id) const;
    bool        hasAward(const std::string& entity_id,
                         const std::string& award_id) const;
    std::string getCorpId(const std::string& entity_id) const;
    std::string getMedalName(const std::string& entity_id,
                             const std::string& medal_id) const;
    std::string getMedalDescription(const std::string& entity_id,
                                    const std::string& medal_id) const;
    int         getTotalMedalsCreated(const std::string& entity_id) const;
    int         getTotalAwardsGiven(const std::string& entity_id) const;
    int         getTotalAwardsRevoked(const std::string& entity_id) const;
    int         getAwardCountForMedal(const std::string& entity_id,
                                      const std::string& medal_id) const;
    int         getAwardCountForRecipient(const std::string& entity_id,
                                          const std::string& recipient_id) const;
    std::string getAwardReason(const std::string& entity_id,
                               const std::string& award_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::MedalCollectionState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_MEDAL_SYSTEM_H
