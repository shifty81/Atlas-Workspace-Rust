#pragma once
#include <string>
#include <unordered_map>
#include <cstdint>

namespace atlas::gameplay {

struct EconomyResource {
    std::string name;
    double amount = 0.0;
    double capacity = -1.0; // -1 = unlimited
};

class EconomySystem {
public:
    void RegisterResource(const std::string& name, double initialAmount = 0.0, double capacity = -1.0);
    bool HasResource(const std::string& name) const;
    double GetAmount(const std::string& name) const;
    double GetCapacity(const std::string& name) const;
    size_t ResourceCount() const;

    bool Add(const std::string& name, double amount);
    bool Spend(const std::string& name, double amount);
    bool CanAfford(const std::string& name, double amount) const;

    void Clear();

private:
    std::unordered_map<std::string, EconomyResource> m_resources;
};

} // namespace atlas::gameplay
