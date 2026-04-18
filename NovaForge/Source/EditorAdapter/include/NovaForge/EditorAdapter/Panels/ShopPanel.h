#pragma once
// NovaForge::ShopPanel — document panel for market/shop authoring.
//
// Loads real data from data/market/prices.json when a project is opened.
// Trade hubs become shop configs; base_prices become shop listings.
// Falls back to built-in defaults when files are absent.

#include "NovaForge/EditorAdapter/DocumentPanelBase.h"
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#if __has_include(<nlohmann/json.hpp>)
#  include <nlohmann/json.hpp>
#  define NF_SHOP_HAS_JSON 1
#endif

namespace NovaForge {

// ── Shop data schema ─────────────────────────────────────────────────────

struct ShopListing {
    std::string itemId;
    std::string currencyId      = "isk";
    float       basePrice       = 1.0f;
    int         stockLimit      = 0;       // 0 = unlimited
    bool        requiresUnlock  = false;
    std::string unlockCondition;
};

struct ShopConfig {
    // Original 4-field layout preserved for aggregate-init compatibility
    std::string shopId;
    std::string displayName;
    std::string merchantFaction;
    bool        globallyAvailable = false;
    // Extended field (loaded from JSON)
    std::string location;
};

// ── ShopPanel ────────────────────────────────────────────────────────────

class ShopPanel final : public DocumentPanelBase {
public:
    static constexpr const char* kPanelId = "novaforge.shop";

    ShopPanel() : m_id(kPanelId), m_title("Shop") {}

    [[nodiscard]] const std::string& panelId()    const override { return m_id; }
    [[nodiscard]] const std::string& panelTitle() const override { return m_title; }

    [[nodiscard]] const std::vector<ShopConfig>&  shops()          const { return m_shops;    }
    [[nodiscard]] const std::vector<ShopListing>& listings()       const { return m_listings; }
    [[nodiscard]] float                            globalDiscount() const { return m_globalDiscount; }

    // Legacy single-config accessor — returns the primary shop or a sentinel.
    [[nodiscard]] const ShopConfig& shopConfig() const {
        static const ShopConfig empty{};
        return m_shops.empty() ? empty : m_shops.front();
    }

    // Legacy single-config setter — replaces/sets the primary shop entry.
    void setShopConfig(ShopConfig config) {
        auto old = m_shops;
        if (m_shops.empty())
            m_shops.push_back(std::move(config));
        else
            m_shops.front() = std::move(config);
        markFieldChanged();
        m_undoStack.push(PanelUndoEntry{
            "Update Shop Config",
            [this, cur  = m_shops]() { m_shops = cur;  markFieldChanged(); return true; },
            [this, prev = old]()     { m_shops = prev; markFieldChanged(); return true; }
        });
    }

    // ── Editing API ───────────────────────────────────────────────────────
    void addShop(ShopConfig config) {
        auto old = m_shops;
        m_shops.push_back(std::move(config));
        markFieldChanged();
        m_undoStack.push(PanelUndoEntry{
            "Add Shop",
            [this, cur  = m_shops]() { m_shops = cur;  markFieldChanged(); return true; },
            [this, prev = old]()     { m_shops = prev; markFieldChanged(); return true; }
        });
    }
    void addListing(ShopListing listing) {
        auto old = m_listings;
        m_listings.push_back(std::move(listing));
        markFieldChanged();
        m_undoStack.push(PanelUndoEntry{
            "Add Listing",
            [this, cur  = m_listings]() { m_listings = cur;  markFieldChanged(); return true; },
            [this, prev = old]()        { m_listings = prev; markFieldChanged(); return true; }
        });
    }
    void removeListing(const std::string& itemId) {
        auto it = std::find_if(m_listings.begin(), m_listings.end(),
                               [&](const auto& l) { return l.itemId == itemId; });
        if (it == m_listings.end()) return;
        auto old = m_listings;
        m_listings.erase(it);
        markFieldChanged();
        m_undoStack.push(PanelUndoEntry{
            "Remove Listing",
            [this, cur  = m_listings]() { m_listings = cur;  markFieldChanged(); return true; },
            [this, prev = old]()        { m_listings = prev; markFieldChanged(); return true; }
        });
    }
    void setGlobalDiscount(float pct) {
        pushPropertyEdit("Set Global Discount", m_globalDiscount, m_globalDiscount, pct);
    }

    // ── Validation ────────────────────────────────────────────────────────
    [[nodiscard]] std::vector<DocumentPanelValidationMessage> validate() const override {
        std::vector<DocumentPanelValidationMessage> msgs;
        if (m_shops.empty())
            msgs.push_back({"shops", "At least one trade hub is required",
                            DocumentPanelValidationSeverity::Warning});
        for (const auto& s : m_shops) {
            if (s.shopId.empty())
                msgs.push_back({"shops.shopId", "Shop config has empty shopId",
                                DocumentPanelValidationSeverity::Error});
        }
        for (const auto& l : m_listings) {
            if (l.itemId.empty())
                msgs.push_back({"listings", "Listing has empty itemId",
                                DocumentPanelValidationSeverity::Error});
            if (l.basePrice < 0.0f)
                msgs.push_back({"listings." + l.itemId + ".basePrice",
                                "Price must be >= 0",
                                DocumentPanelValidationSeverity::Error});
        }
        if (m_globalDiscount < 0.0f || m_globalDiscount > 1.0f)
            msgs.push_back({"globalDiscount", "Discount must be in [0, 1]",
                            DocumentPanelValidationSeverity::Warning});
        return msgs;
    }

    // ── Summary rows (for workspace dashboard display) ────────────────────
    [[nodiscard]] std::vector<std::pair<std::string, std::string>>
        summaryRows() const override
    {
        std::vector<std::pair<std::string, std::string>> rows;
        rows.emplace_back("Trade hubs",
                          std::to_string(m_shops.size()));
        rows.emplace_back("Listings",
                          std::to_string(m_listings.size()));
        rows.emplace_back("Global discount",
                          std::to_string(m_globalDiscount));
        if (!m_shops.empty())
            rows.emplace_back("Primary hub", m_shops.front().displayName);
        return rows;
    }

    // ── Project load hook ─────────────────────────────────────────────────
    void onProjectLoaded(const std::string& projectRoot) override {
        DocumentPanelBase::onProjectLoaded(projectRoot);
        m_shops.clear();
        m_listings.clear();
        loadFromProjectFiles(projectRoot);
    }

    void onProjectUnloaded() override {
        DocumentPanelBase::onProjectUnloaded();
        m_shops.clear();
        m_listings.clear();
    }

protected:
    void loadFromDocument(const NovaForgeDocument& /*doc*/) override {
        if (m_shops.empty()) applyDefaults();
    }

private:
    void loadFromProjectFiles([[maybe_unused]] const std::string& projectRoot) {
#ifdef NF_SHOP_HAS_JSON
        namespace fs = std::filesystem;

        auto tryFile = [&](const char* rel) -> fs::path {
            fs::path p = fs::path(projectRoot) / rel;
            return fs::exists(p) ? p : fs::path{};
        };

        fs::path prices = tryFile("data/market/prices.json");
        if (prices.empty()) prices = tryFile("Data/Market/prices.json");

        if (!prices.empty()) {
            try {
                std::ifstream f(prices);
                if (f.is_open()) {
                    auto j = nlohmann::json::parse(f, nullptr, false);
                    if (!j.is_discarded()) {
                        // Trade hubs → shop configs
                        if (j.contains("trade_hubs") && j["trade_hubs"].is_array()) {
                            for (auto& hub : j["trade_hubs"]) {
                                ShopConfig cfg;
                                cfg.shopId          = hub.value("location_id", "hub");
                                cfg.displayName     = hub.value("name",        cfg.shopId);
                                cfg.location        = hub.value("system",      "");
                                cfg.merchantFaction = "Market";
                                cfg.globallyAvailable = false;
                                m_shops.push_back(std::move(cfg));
                            }
                        }
                        // base_prices → listings
                        if (j.contains("base_prices") && j["base_prices"].is_object()) {
                            for (auto& [itemId, price] : j["base_prices"].items()) {
                                ShopListing listing;
                                listing.itemId    = itemId;
                                listing.basePrice = price.get<float>();
                                m_listings.push_back(std::move(listing));
                            }
                        }
                    }
                }
            } catch (...) {}
        }
#endif
        if (m_shops.empty())    applyDefaults();
    }

    void applyDefaults() {
        // {shopId, displayName, merchantFaction, globallyAvailable, location}
        ShopConfig hub;
        hub.shopId           = "thyrkstad_4_4";
        hub.displayName      = "Thyrkstad IV - Moon 4 - Veyren Fleet Assembly Yard";
        hub.merchantFaction  = "Veyren Fleet";
        hub.globallyAvailable= false;
        hub.location         = "thyrkstad";
        m_shops.push_back(std::move(hub));
        m_listings.push_back({"fang",             "isk", 350000.0f, 0, false, ""});
        m_listings.push_back({"200mm_autocannon", "isk",  50000.0f, 0, false, ""});
        m_listings.push_back({"light_missile",    "isk",    100.0f, 0, false, ""});
        m_listings.push_back({"ferrium",          "isk",      5.0f, 0, false, ""});
    }

    std::string m_id;
    std::string m_title;

    std::vector<ShopConfig>   m_shops;
    std::vector<ShopListing>  m_listings;
    float                     m_globalDiscount = 0.0f;
};

} // namespace NovaForge
