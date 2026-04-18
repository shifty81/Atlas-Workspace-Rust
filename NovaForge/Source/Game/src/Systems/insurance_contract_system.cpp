#include "systems/insurance_contract_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

using IT = components::InsuranceContractState::InsuranceTier;
using CS = components::InsuranceContractState::ContractStatus;

static float payout_ratio_for_tier(IT tier) {
    switch (tier) {
        case IT::Basic:    return 0.4f;
        case IT::Standard: return 0.5f;
        case IT::Bronze:   return 0.6f;
        case IT::Silver:   return 0.7f;
        case IT::Gold:     return 0.85f;
        case IT::Platinum: return 1.0f;
    }
    return 0.4f;
}

static float premium_multiplier_for_tier(IT tier) {
    switch (tier) {
        case IT::Basic:    return 0.1f;
        case IT::Standard: return 0.15f;
        case IT::Bronze:   return 0.2f;
        case IT::Silver:   return 0.3f;
        case IT::Gold:     return 0.5f;
        case IT::Platinum: return 0.8f;
    }
    return 0.1f;
}

InsuranceContractSystem::InsuranceContractSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void InsuranceContractSystem::updateComponent(
        ecs::Entity& /*entity*/,
        components::InsuranceContractState& comp,
        float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;

    for (auto& c : comp.contracts) {
        if (c.status == CS::Active) {
            c.time_remaining -= delta_time;
            if (c.time_remaining <= 0.0f) {
                c.time_remaining = 0.0f;
                c.status = CS::Expired;
            }
        }
    }
}

bool InsuranceContractSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::InsuranceContractState>();
    entity->addComponent(std::move(comp));
    return true;
}

bool InsuranceContractSystem::purchaseContract(
        const std::string& entity_id,
        const std::string& contract_id,
        const std::string& ship_id,
        components::InsuranceContractState::InsuranceTier tier,
        float base_value) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (contract_id.empty()) return false;
    if (ship_id.empty()) return false;
    if (base_value <= 0.0f) return false;
    if (static_cast<int>(comp->contracts.size()) >= comp->max_contracts) return false;
    for (const auto& c : comp->contracts)
        if (c.contract_id == contract_id) return false;

    components::InsuranceContractState::InsuranceContract contract;
    contract.contract_id  = contract_id;
    contract.ship_id      = ship_id;
    contract.tier         = tier;
    contract.status       = CS::Active;
    contract.base_value   = base_value;
    contract.payout_ratio = payout_ratio_for_tier(tier);
    contract.premium_paid = premium_multiplier_for_tier(tier) * base_value;
    contract.time_remaining = 604800.0f;
    contract.created_at   = comp->elapsed;
    comp->contracts.push_back(contract);

    comp->total_premiums_paid += contract.premium_paid;
    return true;
}

bool InsuranceContractSystem::fileClaim(const std::string& entity_id,
                                        const std::string& contract_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    auto it = std::find_if(comp->contracts.begin(), comp->contracts.end(),
        [&](const auto& c) { return c.contract_id == contract_id; });
    if (it == comp->contracts.end()) return false;
    if (it->status != CS::Active) return false;
    it->status = CS::ClaimedPaid;
    float payout = it->payout_ratio * it->base_value;
    comp->total_payouts_received += payout;
    ++comp->total_claims;
    return true;
}

bool InsuranceContractSystem::cancelContract(const std::string& entity_id,
                                              const std::string& contract_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    auto it = std::find_if(comp->contracts.begin(), comp->contracts.end(),
        [&](const auto& c) { return c.contract_id == contract_id; });
    if (it == comp->contracts.end()) return false;
    if (it->status != CS::Active) return false;
    it->status = CS::Cancelled;
    return true;
}

bool InsuranceContractSystem::removeContract(const std::string& entity_id,
                                              const std::string& contract_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    auto it = std::find_if(comp->contracts.begin(), comp->contracts.end(),
        [&](const auto& c) { return c.contract_id == contract_id; });
    if (it == comp->contracts.end()) return false;
    comp->contracts.erase(it);
    return true;
}

bool InsuranceContractSystem::clearContracts(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->contracts.clear();
    return true;
}

bool InsuranceContractSystem::setOwnerId(const std::string& entity_id,
                                          const std::string& owner_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (owner_id.empty()) return false;
    comp->owner_id = owner_id;
    return true;
}

bool InsuranceContractSystem::setMaxContracts(const std::string& entity_id,
                                               int max_contracts) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (max_contracts < 1) return false;
    comp->max_contracts = max_contracts;
    return true;
}

int InsuranceContractSystem::getContractCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return static_cast<int>(comp->contracts.size());
}

bool InsuranceContractSystem::hasContract(const std::string& entity_id,
                                           const std::string& contract_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& c : comp->contracts)
        if (c.contract_id == contract_id) return true;
    return false;
}

bool InsuranceContractSystem::isContractActive(const std::string& entity_id,
                                                const std::string& contract_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& c : comp->contracts)
        if (c.contract_id == contract_id) return c.status == CS::Active;
    return false;
}

float InsuranceContractSystem::getContractPayout(const std::string& entity_id,
                                                  const std::string& contract_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    for (const auto& c : comp->contracts)
        if (c.contract_id == contract_id)
            return c.payout_ratio * c.base_value;
    return 0.0f;
}

float InsuranceContractSystem::getPremiumForTier(
        components::InsuranceContractState::InsuranceTier tier,
        float base_value) {
    return premium_multiplier_for_tier(tier) * base_value;
}

std::string InsuranceContractSystem::getOwnerId(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return "";
    return comp->owner_id;
}

float InsuranceContractSystem::getTotalPremiumsPaid(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    return comp->total_premiums_paid;
}

float InsuranceContractSystem::getTotalPayoutsReceived(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    return comp->total_payouts_received;
}

int InsuranceContractSystem::getTotalClaims(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return comp->total_claims;
}

int InsuranceContractSystem::getCountByTier(
        const std::string& entity_id,
        components::InsuranceContractState::InsuranceTier tier) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    int count = 0;
    for (const auto& c : comp->contracts)
        if (c.tier == tier) ++count;
    return count;
}

int InsuranceContractSystem::getCountByStatus(
        const std::string& entity_id,
        components::InsuranceContractState::ContractStatus status) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    int count = 0;
    for (const auto& c : comp->contracts)
        if (c.status == status) ++count;
    return count;
}

} // namespace systems
} // namespace atlas
