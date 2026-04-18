#ifndef NOVAFORGE_SYSTEMS_WALLET_JOURNAL_SYSTEM_H
#define NOVAFORGE_SYSTEMS_WALLET_JOURNAL_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/economy_components.h"
#include <string>

namespace atlas {
namespace systems {

class WalletJournalSystem
    : public ecs::SingleComponentSystem<components::WalletJournalState> {
public:
    explicit WalletJournalSystem(ecs::World* world);
    ~WalletJournalSystem() override = default;

    std::string getName() const override { return "WalletJournalSystem"; }

    // --- Lifecycle ---
    bool initialize(const std::string& entity_id);

    // --- Entry management ---
    bool addEntry(const std::string& entity_id,
                  const std::string& entry_id,
                  components::WalletJournalState::TransactionType type,
                  float amount,
                  const std::string& description,
                  const std::string& counterparty);
    bool removeEntry(const std::string& entity_id,
                     const std::string& entry_id);
    bool clearJournal(const std::string& entity_id);

    // --- Configuration ---
    bool setOwner(const std::string& entity_id,
                  const std::string& owner_id);
    bool setMaxEntries(const std::string& entity_id, int max_entries);
    bool setBalance(const std::string& entity_id, float balance);

    // --- Queries ---
    int         getEntryCount(const std::string& entity_id) const;
    bool        hasEntry(const std::string& entity_id,
                         const std::string& entry_id) const;
    float       getBalance(const std::string& entity_id) const;
    std::string getOwner(const std::string& entity_id) const;
    int         getTotalEntriesEver(const std::string& entity_id) const;
    float       getTotalCredits(const std::string& entity_id) const;
    float       getTotalDebits(const std::string& entity_id) const;
    float       getNetFlow(const std::string& entity_id) const;
    float       getCreditsByType(
                    const std::string& entity_id,
                    components::WalletJournalState::TransactionType type) const;
    float       getDebitsByType(
                    const std::string& entity_id,
                    components::WalletJournalState::TransactionType type) const;
    int         getCountByType(
                    const std::string& entity_id,
                    components::WalletJournalState::TransactionType type) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::WalletJournalState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_WALLET_JOURNAL_SYSTEM_H
