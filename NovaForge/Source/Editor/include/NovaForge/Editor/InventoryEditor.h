#pragma once
// NF::Editor — inventory editor
#include "NF/Core/Core.h"
#include "NF/Engine/Engine.h"
#include "NF/Renderer/Renderer.h"
#include "NF/UI/UI.h"
#include "NF/GraphVM/GraphVM.h"
#include "NF/Input/Input.h"
#include "NF/UI/UIWidgets.h"
#include <filesystem>
#include <fstream>
#include <set>
#include <map>
#include <deque>
#include <unordered_map>

namespace NF {

enum class ItemRarity : uint8_t {
    Common, Uncommon, Rare, Epic, Legendary, Unique
};

inline const char* itemRarityName(ItemRarity r) {
    switch (r) {
        case ItemRarity::Common:    return "Common";
        case ItemRarity::Uncommon:  return "Uncommon";
        case ItemRarity::Rare:      return "Rare";
        case ItemRarity::Epic:      return "Epic";
        case ItemRarity::Legendary: return "Legendary";
        case ItemRarity::Unique:    return "Unique";
    }
    return "Unknown";
}

enum class ItemCategory : uint8_t {
    Weapon, Armor, Consumable, Material, QuestItem, Misc, Accessory
};

inline const char* itemCategoryName(ItemCategory c) {
    switch (c) {
        case ItemCategory::Weapon:     return "Weapon";
        case ItemCategory::Armor:      return "Armor";
        case ItemCategory::Consumable: return "Consumable";
        case ItemCategory::Material:   return "Material";
        case ItemCategory::QuestItem:  return "QuestItem";
        case ItemCategory::Misc:       return "Misc";
        case ItemCategory::Accessory:  return "Accessory";
    }
    return "Unknown";
}

enum class InventorySlotType : uint8_t {
    MainHand, OffHand, Head, Chest, Legs, Feet, Ring, General
};

inline const char* inventorySlotTypeName(InventorySlotType s) {
    switch (s) {
        case InventorySlotType::MainHand: return "MainHand";
        case InventorySlotType::OffHand:  return "OffHand";
        case InventorySlotType::Head:     return "Head";
        case InventorySlotType::Chest:    return "Chest";
        case InventorySlotType::Legs:     return "Legs";
        case InventorySlotType::Feet:     return "Feet";
        case InventorySlotType::Ring:     return "Ring";
        case InventorySlotType::General:  return "General";
    }
    return "Unknown";
}

class InventoryItem {
public:
    explicit InventoryItem(uint32_t id, const std::string& name, ItemRarity rarity, ItemCategory category)
        : m_id(id), m_name(name), m_rarity(rarity), m_category(category) {}

    void setRarity(ItemRarity v)      { m_rarity = v; }
    void setCategory(ItemCategory v)  { m_category = v; }
    void setStackSize(uint32_t v)     { m_stackSize = v; }
    void setMaxStack(uint32_t v)      { m_maxStack = v; }
    void setWeight(float v)           { m_weight = v; }
    void setEquippable(bool v)        { m_isEquippable = v; }

    [[nodiscard]] uint32_t      id()          const { return m_id; }
    [[nodiscard]] const std::string& name()   const { return m_name; }
    [[nodiscard]] ItemRarity    rarity()      const { return m_rarity; }
    [[nodiscard]] ItemCategory  category()    const { return m_category; }
    [[nodiscard]] uint32_t      stackSize()   const { return m_stackSize; }
    [[nodiscard]] uint32_t      maxStack()    const { return m_maxStack; }
    [[nodiscard]] float         weight()      const { return m_weight; }
    [[nodiscard]] bool          isEquippable() const { return m_isEquippable; }

private:
    uint32_t     m_id;
    std::string  m_name;
    ItemRarity   m_rarity      = ItemRarity::Common;
    ItemCategory m_category    = ItemCategory::Misc;
    uint32_t     m_stackSize   = 1;
    uint32_t     m_maxStack    = 1;
    float        m_weight      = 1.0f;
    bool         m_isEquippable = false;
};

class InventoryEditor {
public:
    bool addItem(const InventoryItem& item) {
        for (const auto& i : m_items)
            if (i.id() == item.id()) return false;
        m_items.push_back(item);
        return true;
    }

    bool removeItem(uint32_t id) {
        for (auto it = m_items.begin(); it != m_items.end(); ++it) {
            if (it->id() == id) { m_items.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] InventoryItem* findItem(uint32_t id) {
        for (auto& i : m_items)
            if (i.id() == id) return &i;
        return nullptr;
    }

    [[nodiscard]] size_t itemCount() const { return m_items.size(); }

    [[nodiscard]] size_t countByRarity(ItemRarity r) const {
        size_t n = 0;
        for (const auto& i : m_items) if (i.rarity() == r) ++n;
        return n;
    }

    [[nodiscard]] size_t countByCategory(ItemCategory c) const {
        size_t n = 0;
        for (const auto& i : m_items) if (i.category() == c) ++n;
        return n;
    }

    [[nodiscard]] size_t countEquippable() const {
        size_t n = 0;
        for (const auto& i : m_items) if (i.isEquippable()) ++n;
        return n;
    }

    void setGridColumns(uint32_t v)       { m_gridColumns = v; }
    void setGridRows(uint32_t v)          { m_gridRows = v; }
    void setShowWeight(bool v)            { m_showWeight = v; }
    void setShowRarityColor(bool v)       { m_showRarityColor = v; }

    [[nodiscard]] uint32_t gridColumns()       const { return m_gridColumns; }
    [[nodiscard]] uint32_t gridRows()          const { return m_gridRows; }
    [[nodiscard]] bool     isShowWeight()      const { return m_showWeight; }
    [[nodiscard]] bool     isShowRarityColor() const { return m_showRarityColor; }

private:
    std::vector<InventoryItem> m_items;
    uint32_t m_gridColumns    = 10;
    uint32_t m_gridRows       = 8;
    bool     m_showWeight     = true;
    bool     m_showRarityColor = true;
};

} // namespace NF
