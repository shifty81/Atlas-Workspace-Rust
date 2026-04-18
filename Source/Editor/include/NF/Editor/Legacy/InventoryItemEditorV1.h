#pragma once
// NF::Editor — Inventory item editor v1: item definition and property management
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Iiev1ItemRarity : uint8_t { Common, Uncommon, Rare, Epic, Legendary };
enum class Iiev1ItemState  : uint8_t { Draft, Active, Deprecated, Locked };

inline const char* iiev1ItemRarityName(Iiev1ItemRarity r) {
    switch (r) {
        case Iiev1ItemRarity::Common:    return "Common";
        case Iiev1ItemRarity::Uncommon:  return "Uncommon";
        case Iiev1ItemRarity::Rare:      return "Rare";
        case Iiev1ItemRarity::Epic:      return "Epic";
        case Iiev1ItemRarity::Legendary: return "Legendary";
    }
    return "Unknown";
}

inline const char* iiev1ItemStateName(Iiev1ItemState s) {
    switch (s) {
        case Iiev1ItemState::Draft:      return "Draft";
        case Iiev1ItemState::Active:     return "Active";
        case Iiev1ItemState::Deprecated: return "Deprecated";
        case Iiev1ItemState::Locked:     return "Locked";
    }
    return "Unknown";
}

struct Iiev1InventoryItem {
    uint64_t        id          = 0;
    std::string     name;
    Iiev1ItemRarity rarity      = Iiev1ItemRarity::Common;
    Iiev1ItemState  state       = Iiev1ItemState::Draft;
    int             stackSize   = 1;

    [[nodiscard]] bool isValid()      const { return id != 0 && !name.empty(); }
    [[nodiscard]] bool isActive()     const { return state == Iiev1ItemState::Active; }
    [[nodiscard]] bool isDeprecated() const { return state == Iiev1ItemState::Deprecated; }
    [[nodiscard]] bool isLocked()     const { return state == Iiev1ItemState::Locked; }
};

struct Iiev1ItemProperty {
    uint64_t    id     = 0;
    uint64_t    itemId = 0;
    std::string key;
    std::string value;

    [[nodiscard]] bool isValid() const { return id != 0 && itemId != 0 && !key.empty(); }
};

using Iiev1ChangeCallback = std::function<void(uint64_t)>;

class InventoryItemEditorV1 {
public:
    static constexpr size_t MAX_ITEMS      = 4096;
    static constexpr size_t MAX_PROPERTIES = 16384;

    bool addItem(const Iiev1InventoryItem& item) {
        if (!item.isValid()) return false;
        for (const auto& i : m_items) if (i.id == item.id) return false;
        if (m_items.size() >= MAX_ITEMS) return false;
        m_items.push_back(item);
        if (m_onChange) m_onChange(item.id);
        return true;
    }

    bool removeItem(uint64_t id) {
        for (auto it = m_items.begin(); it != m_items.end(); ++it) {
            if (it->id == id) { m_items.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] Iiev1InventoryItem* findItem(uint64_t id) {
        for (auto& i : m_items) if (i.id == id) return &i;
        return nullptr;
    }

    bool addProperty(const Iiev1ItemProperty& prop) {
        if (!prop.isValid()) return false;
        for (const auto& p : m_properties) if (p.id == prop.id) return false;
        if (m_properties.size() >= MAX_PROPERTIES) return false;
        m_properties.push_back(prop);
        if (m_onChange) m_onChange(prop.id);
        return true;
    }

    bool removeProperty(uint64_t id) {
        for (auto it = m_properties.begin(); it != m_properties.end(); ++it) {
            if (it->id == id) { m_properties.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] size_t itemCount()     const { return m_items.size(); }
    [[nodiscard]] size_t propertyCount() const { return m_properties.size(); }

    [[nodiscard]] size_t activeCount() const {
        size_t c = 0; for (const auto& i : m_items) if (i.isActive()) ++c; return c;
    }
    [[nodiscard]] size_t lockedCount() const {
        size_t c = 0; for (const auto& i : m_items) if (i.isLocked()) ++c; return c;
    }
    [[nodiscard]] size_t countByRarity(Iiev1ItemRarity rarity) const {
        size_t c = 0; for (const auto& i : m_items) if (i.rarity == rarity) ++c; return c;
    }
    [[nodiscard]] size_t propertiesForItem(uint64_t itemId) const {
        size_t c = 0; for (const auto& p : m_properties) if (p.itemId == itemId) ++c; return c;
    }

    void setOnChange(Iiev1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Iiev1InventoryItem> m_items;
    std::vector<Iiev1ItemProperty>  m_properties;
    Iiev1ChangeCallback             m_onChange;
};

} // namespace NF
