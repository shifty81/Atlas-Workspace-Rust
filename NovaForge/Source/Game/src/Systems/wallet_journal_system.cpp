#include "systems/wallet_journal_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"

namespace atlas {
namespace systems {

WalletJournalSystem::WalletJournalSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void WalletJournalSystem::updateComponent(
        ecs::Entity& /*entity*/,
        components::WalletJournalState& comp,
        float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;
}

bool WalletJournalSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::WalletJournalState>();
    entity->addComponent(std::move(comp));
    return true;
}

// --- Entry management ---

bool WalletJournalSystem::addEntry(
        const std::string& entity_id,
        const std::string& entry_id,
        components::WalletJournalState::TransactionType type,
        float amount,
        const std::string& description,
        const std::string& counterparty) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (entry_id.empty()) return false;
    if (amount == 0.0f) return false;

    // Duplicate prevention
    for (const auto& e : comp->entries) {
        if (e.entry_id == entry_id) return false;
    }

    // Auto-purge oldest at capacity
    while (static_cast<int>(comp->entries.size()) >= comp->max_entries) {
        comp->entries.erase(comp->entries.begin());
    }

    // Update balance and aggregates
    comp->balance += amount;
    if (amount > 0.0f) {
        comp->total_credits += amount;
    } else {
        comp->total_debits += (-amount);
    }

    components::WalletJournalState::JournalEntry entry;
    entry.entry_id     = entry_id;
    entry.type         = type;
    entry.amount       = amount;
    entry.balance      = comp->balance;
    entry.description  = description;
    entry.counterparty = counterparty;
    comp->entries.push_back(entry);
    ++comp->total_entries_ever;
    return true;
}

bool WalletJournalSystem::removeEntry(const std::string& entity_id,
                                      const std::string& entry_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto it = comp->entries.begin(); it != comp->entries.end(); ++it) {
        if (it->entry_id == entry_id) {
            comp->entries.erase(it);
            return true;
        }
    }
    return false;
}

bool WalletJournalSystem::clearJournal(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->entries.clear();
    return true;
}

// --- Configuration ---

bool WalletJournalSystem::setOwner(const std::string& entity_id,
                                   const std::string& owner_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->owner_id = owner_id;
    return true;
}

bool WalletJournalSystem::setMaxEntries(const std::string& entity_id,
                                        int max_entries) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (max_entries <= 0) return false;
    comp->max_entries = max_entries;
    return true;
}

bool WalletJournalSystem::setBalance(const std::string& entity_id,
                                     float balance) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->balance = balance;
    return true;
}

// --- Queries ---

int WalletJournalSystem::getEntryCount(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return static_cast<int>(comp->entries.size());
}

bool WalletJournalSystem::hasEntry(const std::string& entity_id,
                                   const std::string& entry_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& e : comp->entries) {
        if (e.entry_id == entry_id) return true;
    }
    return false;
}

float WalletJournalSystem::getBalance(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    return comp->balance;
}

std::string WalletJournalSystem::getOwner(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return "";
    return comp->owner_id;
}

int WalletJournalSystem::getTotalEntriesEver(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return comp->total_entries_ever;
}

float WalletJournalSystem::getTotalCredits(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    return comp->total_credits;
}

float WalletJournalSystem::getTotalDebits(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    return comp->total_debits;
}

float WalletJournalSystem::getNetFlow(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    return comp->total_credits - comp->total_debits;
}

float WalletJournalSystem::getCreditsByType(
        const std::string& entity_id,
        components::WalletJournalState::TransactionType type) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    float total = 0.0f;
    for (const auto& e : comp->entries) {
        if (e.type == type && e.amount > 0.0f) total += e.amount;
    }
    return total;
}

float WalletJournalSystem::getDebitsByType(
        const std::string& entity_id,
        components::WalletJournalState::TransactionType type) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    float total = 0.0f;
    for (const auto& e : comp->entries) {
        if (e.type == type && e.amount < 0.0f) total += (-e.amount);
    }
    return total;
}

int WalletJournalSystem::getCountByType(
        const std::string& entity_id,
        components::WalletJournalState::TransactionType type) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    int count = 0;
    for (const auto& e : comp->entries) {
        if (e.type == type) ++count;
    }
    return count;
}

} // namespace systems
} // namespace atlas
