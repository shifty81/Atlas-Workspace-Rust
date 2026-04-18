#ifndef NOVAFORGE_SYSTEMS_INSURANCE_CONTRACT_SYSTEM_H
#define NOVAFORGE_SYSTEMS_INSURANCE_CONTRACT_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/economy_components.h"
#include <string>

namespace atlas {
namespace systems {

class InsuranceContractSystem
    : public ecs::SingleComponentSystem<components::InsuranceContractState> {
public:
    explicit InsuranceContractSystem(ecs::World* world);
    ~InsuranceContractSystem() override = default;

    std::string getName() const override { return "InsuranceContractSystem"; }

    // --- Lifecycle ---
    bool initialize(const std::string& entity_id);

    // --- Contract management ---
    bool purchaseContract(const std::string& entity_id,
                          const std::string& contract_id,
                          const std::string& ship_id,
                          components::InsuranceContractState::InsuranceTier tier,
                          float base_value);
    bool fileClaim(const std::string& entity_id,
                   const std::string& contract_id);
    bool cancelContract(const std::string& entity_id,
                        const std::string& contract_id);
    bool removeContract(const std::string& entity_id,
                        const std::string& contract_id);
    bool clearContracts(const std::string& entity_id);

    // --- Configuration ---
    bool setOwnerId(const std::string& entity_id,
                    const std::string& owner_id);
    bool setMaxContracts(const std::string& entity_id, int max_contracts);

    // --- Queries ---
    int  getContractCount(const std::string& entity_id) const;
    bool hasContract(const std::string& entity_id,
                     const std::string& contract_id) const;
    bool isContractActive(const std::string& entity_id,
                          const std::string& contract_id) const;
    float getContractPayout(const std::string& entity_id,
                            const std::string& contract_id) const;
    std::string getOwnerId(const std::string& entity_id) const;
    float getTotalPremiumsPaid(const std::string& entity_id) const;
    float getTotalPayoutsReceived(const std::string& entity_id) const;
    int   getTotalClaims(const std::string& entity_id) const;
    int   getCountByTier(
              const std::string& entity_id,
              components::InsuranceContractState::InsuranceTier tier) const;
    int   getCountByStatus(
              const std::string& entity_id,
              components::InsuranceContractState::ContractStatus status) const;

    // --- Static helpers ---
    static float getPremiumForTier(
        components::InsuranceContractState::InsuranceTier tier,
        float base_value);

protected:
    void updateComponent(ecs::Entity& entity,
                         components::InsuranceContractState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_INSURANCE_CONTRACT_SYSTEM_H
