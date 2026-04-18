#include "EconomySystem.h"
#include <algorithm>

namespace atlas::gameplay {

void EconomySystem::RegisterResource(const std::string& name, double initialAmount, double capacity) {
    EconomyResource res;
    res.name = name;
    res.amount = initialAmount;
    res.capacity = capacity;
    m_resources[name] = res;
}

bool EconomySystem::HasResource(const std::string& name) const {
    return m_resources.count(name) > 0;
}

double EconomySystem::GetAmount(const std::string& name) const {
    auto it = m_resources.find(name);
    return it != m_resources.end() ? it->second.amount : 0.0;
}

double EconomySystem::GetCapacity(const std::string& name) const {
    auto it = m_resources.find(name);
    return it != m_resources.end() ? it->second.capacity : -1.0;
}

size_t EconomySystem::ResourceCount() const {
    return m_resources.size();
}

bool EconomySystem::Add(const std::string& name, double amount) {
    auto it = m_resources.find(name);
    if (it == m_resources.end() || amount < 0.0) return false;

    double newAmount = it->second.amount + amount;
    if (it->second.capacity >= 0.0) {
        newAmount = std::min(newAmount, it->second.capacity);
    }
    it->second.amount = newAmount;
    return true;
}

bool EconomySystem::Spend(const std::string& name, double amount) {
    auto it = m_resources.find(name);
    if (it == m_resources.end() || amount < 0.0) return false;
    if (it->second.amount < amount) return false;

    it->second.amount -= amount;
    return true;
}

bool EconomySystem::CanAfford(const std::string& name, double amount) const {
    auto it = m_resources.find(name);
    return it != m_resources.end() && it->second.amount >= amount;
}

void EconomySystem::Clear() {
    m_resources.clear();
}

} // namespace atlas::gameplay
