#pragma once
// NF::Editor — virtual currency editor
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

enum class CurrencyType : uint8_t {
    Hard, Soft, Premium, Seasonal, Event
};

inline const char* currencyTypeName(CurrencyType t) {
    switch (t) {
        case CurrencyType::Hard:     return "Hard";
        case CurrencyType::Soft:     return "Soft";
        case CurrencyType::Premium:  return "Premium";
        case CurrencyType::Seasonal: return "Seasonal";
        case CurrencyType::Event:    return "Event";
    }
    return "Unknown";
}

enum class CurrencyAcquisition : uint8_t {
    Purchase, Reward, Achievement, Quest, Trade
};

inline const char* currencyAcquisitionName(CurrencyAcquisition a) {
    switch (a) {
        case CurrencyAcquisition::Purchase:    return "Purchase";
        case CurrencyAcquisition::Reward:      return "Reward";
        case CurrencyAcquisition::Achievement: return "Achievement";
        case CurrencyAcquisition::Quest:       return "Quest";
        case CurrencyAcquisition::Trade:       return "Trade";
    }
    return "Unknown";
}

class CurrencyEntry {
public:
    explicit CurrencyEntry(uint32_t id, const std::string& code,
                           CurrencyType type, CurrencyAcquisition acquisition)
        : m_id(id), m_code(code), m_type(type), m_acquisition(acquisition) {}

    void setExchangeRate(float v) { m_exchangeRate = v; }
    void setIsEnabled(bool v)     { m_isEnabled    = v; }

    [[nodiscard]] uint32_t              id()           const { return m_id;           }
    [[nodiscard]] const std::string&    code()         const { return m_code;         }
    [[nodiscard]] CurrencyType          type()         const { return m_type;         }
    [[nodiscard]] CurrencyAcquisition   acquisition()  const { return m_acquisition;  }
    [[nodiscard]] float                 exchangeRate() const { return m_exchangeRate; }
    [[nodiscard]] bool                  isEnabled()    const { return m_isEnabled;    }

private:
    uint32_t            m_id;
    std::string         m_code;
    CurrencyType        m_type;
    CurrencyAcquisition m_acquisition;
    float               m_exchangeRate = 1.0f;
    bool                m_isEnabled    = true;
};

class VirtualCurrencyEditor {
public:
    void setIsShowDisabled(bool v)    { m_isShowDisabled  = v; }
    void setIsGroupByType(bool v)     { m_isGroupByType   = v; }
    void setBaseExchangeRate(float v) { m_baseExchangeRate = v; }

    bool addCurrency(const CurrencyEntry& c) {
        for (auto& x : m_currencies) if (x.id() == c.id()) return false;
        m_currencies.push_back(c); return true;
    }
    bool removeCurrency(uint32_t id) {
        auto it = std::find_if(m_currencies.begin(), m_currencies.end(),
            [&](const CurrencyEntry& c){ return c.id() == id; });
        if (it == m_currencies.end()) return false;
        m_currencies.erase(it); return true;
    }
    [[nodiscard]] CurrencyEntry* findCurrency(uint32_t id) {
        for (auto& c : m_currencies) if (c.id() == id) return &c;
        return nullptr;
    }

    [[nodiscard]] bool  isShowDisabled()   const { return m_isShowDisabled;  }
    [[nodiscard]] bool  isGroupByType()    const { return m_isGroupByType;   }
    [[nodiscard]] float baseExchangeRate() const { return m_baseExchangeRate; }
    [[nodiscard]] size_t currencyCount()   const { return m_currencies.size(); }

    [[nodiscard]] size_t countByType(CurrencyType t) const {
        size_t n = 0; for (auto& c : m_currencies) if (c.type() == t) ++n; return n;
    }
    [[nodiscard]] size_t countByAcquisition(CurrencyAcquisition a) const {
        size_t n = 0; for (auto& c : m_currencies) if (c.acquisition() == a) ++n; return n;
    }
    [[nodiscard]] size_t countEnabled() const {
        size_t n = 0; for (auto& c : m_currencies) if (c.isEnabled()) ++n; return n;
    }

private:
    std::vector<CurrencyEntry> m_currencies;
    bool  m_isShowDisabled  = false;
    bool  m_isGroupByType   = false;
    float m_baseExchangeRate = 1.0f;
};

} // namespace NF
