#pragma once

#include <string>
#include <vector>

namespace NF::Patch {

struct EquipmentSlotVM {
    std::string slotId;
    std::string displayName;
    std::string category;
    bool unlocked = false;
    bool occupied = false;
    bool compatible = false;
    bool damaged = false;
    float powerDrawKw = 0.0f;
    std::string equippedItemId;
    std::vector<std::string> unmetDependencies;
};

struct EquipmentScreenVM {
    std::string rigTier;
    float powerBudgetKw = 0.0f;
    float usedPowerKw = 0.0f;
    std::vector<EquipmentSlotVM> slots;
    std::string focusedSlotId;
    std::string focusedItemId;
    std::vector<std::string> unlockedSystems;
    std::vector<std::string> warnings;
};

class IEquipmentScreenAdapter {
public:
    virtual ~IEquipmentScreenAdapter() = default;
    virtual EquipmentScreenVM BuildViewModel() const = 0;
    virtual bool EquipItem(const std::string& slotId, const std::string& itemId) = 0;
    virtual bool UnequipItem(const std::string& slotId) = 0;
    virtual std::vector<std::string> GetCompatibleItems(const std::string& slotId) const = 0;
    virtual std::string GetFailureReason() const = 0;
};

} // namespace NF::Patch
