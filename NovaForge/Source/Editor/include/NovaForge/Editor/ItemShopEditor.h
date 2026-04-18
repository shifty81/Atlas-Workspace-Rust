#pragma once
// NF::Editor — item shop editor
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

enum class ShopCategory : uint8_t {
    Cosmetic, Consumable, Equipment, Bundle, Currency, Seasonal
};

inline const char* shopCategoryName(ShopCategory c) {
    switch (c) {
        case ShopCategory::Cosmetic:    return "Cosmetic";
        case ShopCategory::Consumable:  return "Consumable";
        case ShopCategory::Equipment:   return "Equipment";
        case ShopCategory::Bundle:      return "Bundle";
        case ShopCategory::Currency:    return "Currency";
        case ShopCategory::Seasonal:    return "Seasonal";
    }
    return "Unknown";
}

enum class PricingModel : uint8_t {
    Fixed, Dynamic, Auction, Tiered, Subscription
};

inline const char* pricingModelName(PricingModel m) {
    switch (m) {
        case PricingModel::Fixed:        return "Fixed";
        case PricingModel::Dynamic:      return "Dynamic";
        case PricingModel::Auction:      return "Auction";
        case PricingModel::Tiered:       return "Tiered";
        case PricingModel::Subscription: return "Subscription";
    }
    return "Unknown";
}

class ShopItem {
public:
    explicit ShopItem(uint32_t id, const std::string& name,
                      ShopCategory category, PricingModel pricingModel)
        : m_id(id), m_name(name), m_category(category), m_pricingModel(pricingModel) {}

    void setBasePrice(uint32_t v) { m_basePrice  = v; }
    void setIsLimited(bool v)     { m_isLimited  = v; }

    [[nodiscard]] uint32_t           id()           const { return m_id;           }
    [[nodiscard]] const std::string& name()         const { return m_name;         }
    [[nodiscard]] ShopCategory       category()     const { return m_category;     }
    [[nodiscard]] PricingModel       pricingModel() const { return m_pricingModel; }
    [[nodiscard]] uint32_t           basePrice()    const { return m_basePrice;    }
    [[nodiscard]] bool               isLimited()    const { return m_isLimited;    }

private:
    uint32_t     m_id;
    std::string  m_name;
    ShopCategory m_category;
    PricingModel m_pricingModel;
    uint32_t     m_basePrice  = 100u;
    bool         m_isLimited  = false;
};

class ItemShopEditor {
public:
    void setIsShowOutOfStock(bool v) { m_isShowOutOfStock = v; }
    void setIsGroupByCategory(bool v){ m_isGroupByCategory = v; }
    void setTaxRate(float v)         { m_taxRate           = v; }

    bool addItem(const ShopItem& item) {
        for (auto& x : m_items) if (x.id() == item.id()) return false;
        m_items.push_back(item); return true;
    }
    bool removeItem(uint32_t id) {
        auto it = std::find_if(m_items.begin(), m_items.end(),
            [&](const ShopItem& i){ return i.id() == id; });
        if (it == m_items.end()) return false;
        m_items.erase(it); return true;
    }
    [[nodiscard]] ShopItem* findItem(uint32_t id) {
        for (auto& i : m_items) if (i.id() == id) return &i;
        return nullptr;
    }

    [[nodiscard]] bool  isShowOutOfStock()  const { return m_isShowOutOfStock;  }
    [[nodiscard]] bool  isGroupByCategory() const { return m_isGroupByCategory; }
    [[nodiscard]] float taxRate()           const { return m_taxRate;           }
    [[nodiscard]] size_t itemCount()        const { return m_items.size();      }

    [[nodiscard]] size_t countByCategory(ShopCategory c) const {
        size_t n = 0; for (auto& i : m_items) if (i.category() == c) ++n; return n;
    }
    [[nodiscard]] size_t countByPricingModel(PricingModel m) const {
        size_t n = 0; for (auto& i : m_items) if (i.pricingModel() == m) ++n; return n;
    }
    [[nodiscard]] size_t countLimited() const {
        size_t n = 0; for (auto& i : m_items) if (i.isLimited()) ++n; return n;
    }

private:
    std::vector<ShopItem> m_items;
    bool  m_isShowOutOfStock  = false;
    bool  m_isGroupByCategory = false;
    float m_taxRate           = 0.0f;
};

} // namespace NF
