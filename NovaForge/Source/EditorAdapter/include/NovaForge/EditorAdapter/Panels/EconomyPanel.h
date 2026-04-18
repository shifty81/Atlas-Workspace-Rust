#pragma once
// NovaForge::EconomyPanel — document panel for economy authoring.
//
// Binds to a NovaForgeDocument of type EconomyRules.
// Manages in-game currency definitions, pricing multipliers, reward rates,
// and economy balance parameters.
//
// Loads real data from data/market/prices.json when a project is opened.
// Falls back to built-in defaults when the file is absent.

#include "NovaForge/EditorAdapter/DocumentPanelBase.h"
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#if __has_include(<nlohmann/json.hpp>)
#  include <nlohmann/json.hpp>
#  define NF_ECONOMY_HAS_JSON 1
#endif

namespace NovaForge {

// ── Economy data schema ───────────────────────────────────────────────────

struct CurrencyDefinition {
    std::string id;
    std::string displayName;
    float       earnRate    = 1.0f;
    float       spendCap    = 0.0f;
    bool        tradeable   = true;
};

struct EconomyPricingRule {
    std::string itemId;
    std::string itemCategory;
    float       basePrice        = 1.0f;
    float       demandMultiplier = 1.0f;
    float       supplyMultiplier = 1.0f;
};

// ── EconomyPanel ─────────────────────────────────────────────────────────

class EconomyPanel final : public DocumentPanelBase {
public:
    static constexpr const char* kPanelId = "novaforge.economy";

    EconomyPanel() : m_id(kPanelId), m_title("Economy") {}

    [[nodiscard]] const std::string& panelId()    const override { return m_id; }
    [[nodiscard]] const std::string& panelTitle() const override { return m_title; }

    [[nodiscard]] const std::vector<CurrencyDefinition>& currencies()   const { return m_currencies;   }
    [[nodiscard]] const std::vector<EconomyPricingRule>& pricingRules() const { return m_pricingRules; }
    [[nodiscard]] float globalInflationRate() const { return m_globalInflationRate; }
    [[nodiscard]] bool  economyEnabled()      const { return m_economyEnabled; }

    // ── Editing API ───────────────────────────────────────────────────────
    void addCurrency(CurrencyDefinition def) {
        auto old = m_currencies;
        m_currencies.push_back(std::move(def));
        markFieldChanged();
        m_undoStack.push(PanelUndoEntry{
            "Add Currency",
            [this, cur  = m_currencies]() { m_currencies = cur;  markFieldChanged(); return true; },
            [this, prev = old]()          { m_currencies = prev; markFieldChanged(); return true; }
        });
    }

    void removeCurrency(const std::string& id) {
        auto it = std::find_if(m_currencies.begin(), m_currencies.end(),
                               [&](const auto& c) { return c.id == id; });
        if (it == m_currencies.end()) return;
        auto old = m_currencies;
        m_currencies.erase(it);
        markFieldChanged();
        m_undoStack.push(PanelUndoEntry{
            "Remove Currency",
            [this, cur  = m_currencies]() { m_currencies = cur;  markFieldChanged(); return true; },
            [this, prev = old]()          { m_currencies = prev; markFieldChanged(); return true; }
        });
    }

    void setGlobalInflationRate(float rate) {
        pushPropertyEdit("Set Inflation Rate", m_globalInflationRate, m_globalInflationRate, rate);
    }

    void setEconomyEnabled(bool enabled) {
        pushPropertyEdit("Toggle Economy", m_economyEnabled, m_economyEnabled, enabled);
    }

    void addPricingRule(EconomyPricingRule rule) {
        auto old = m_pricingRules;
        m_pricingRules.push_back(std::move(rule));
        markFieldChanged();
        m_undoStack.push(PanelUndoEntry{
            "Add Pricing Rule",
            [this, cur  = m_pricingRules]() { m_pricingRules = cur;  markFieldChanged(); return true; },
            [this, prev = old]()            { m_pricingRules = prev; markFieldChanged(); return true; }
        });
    }

    // ── Validation ────────────────────────────────────────────────────────
    [[nodiscard]] std::vector<DocumentPanelValidationMessage> validate() const override {
        std::vector<DocumentPanelValidationMessage> msgs;
        for (const auto& c : m_currencies) {
            if (c.id.empty())
                msgs.push_back({"currencies", "Currency has empty id",
                                DocumentPanelValidationSeverity::Error});
            if (c.earnRate < 0.0f)
                msgs.push_back({"currencies." + c.id + ".earnRate",
                                "Earn rate must be >= 0",
                                DocumentPanelValidationSeverity::Error});
        }
        if (m_globalInflationRate < 0.0f)
            msgs.push_back({"globalInflationRate", "Inflation rate must be >= 0",
                            DocumentPanelValidationSeverity::Warning});
        return msgs;
    }

    // ── Summary rows (for workspace dashboard display) ────────────────────
    [[nodiscard]] std::vector<std::pair<std::string, std::string>>
        summaryRows() const override
    {
        std::vector<std::pair<std::string, std::string>> rows;
        rows.emplace_back("Economy enabled",
                          m_economyEnabled ? "Yes" : "No");
        rows.emplace_back("Global inflation rate",
                          std::to_string(m_globalInflationRate));
        rows.emplace_back("Currencies",
                          std::to_string(m_currencies.size()));
        rows.emplace_back("Pricing rules",
                          std::to_string(m_pricingRules.size()));
        if (!m_currencies.empty())
            rows.emplace_back("Base currency", m_currencies[0].displayName);
        return rows;
    }

    // ── Project load hook ─────────────────────────────────────────────────
    // Called by the workspace after a project is manually opened by the user.
    // Reads data/market/prices.json from the project root and populates the
    // currency list and pricing rules from live project data.
    void onProjectLoaded(const std::string& projectRoot) override {
        DocumentPanelBase::onProjectLoaded(projectRoot);
        m_currencies.clear();
        m_pricingRules.clear();
        loadFromProjectFiles(projectRoot);
    }

    void onProjectUnloaded() override {
        DocumentPanelBase::onProjectUnloaded();
        m_currencies.clear();
        m_pricingRules.clear();
    }

protected:
    void loadFromDocument(const NovaForgeDocument& /*doc*/) override {
        // Only populate defaults when no project data was loaded.
        if (m_currencies.empty())   applyDefaults();
    }

private:
    void loadFromProjectFiles([[maybe_unused]] const std::string& projectRoot) {
        namespace fs = std::filesystem;

        // ISK is always the base currency
        m_currencies.push_back({"isk", "ISK", 1.0f, 0.0f, true});

#ifdef NF_ECONOMY_HAS_JSON
        // Probe both capitalised and lowercase data paths
        auto tryPath = [&](const char* rel) -> fs::path {
            fs::path p = fs::path(projectRoot) / rel;
            return fs::exists(p) ? p : fs::path{};
        };

        fs::path prices = tryPath("data/market/prices.json");
        if (prices.empty()) prices = tryPath("Data/Market/prices.json");
        if (!prices.empty()) {
            try {
                std::ifstream f(prices);
                if (f.is_open()) {
                    auto j = nlohmann::json::parse(f, nullptr, /*exceptions=*/false);
                    if (!j.is_discarded() && j.contains("base_prices")) {
                        for (auto& [itemId, price] : j["base_prices"].items()) {
                            EconomyPricingRule rule;
                            rule.itemId        = itemId;
                            rule.itemCategory  = "item";
                            rule.basePrice     = price.get<float>();
                            m_pricingRules.push_back(std::move(rule));
                        }
                    }
                    // Loyalty points as a secondary non-tradeable currency
                    if (j.contains("trade_hubs") && !j["trade_hubs"].empty()) {
                        m_currencies.push_back({"loyalty_points", "Loyalty Points",
                                                0.0f, 0.0f, false});
                    }
                }
            } catch (...) {}
        }
#endif

        if (m_currencies.size() == 1 && m_pricingRules.empty())
            applyDefaults();
    }

    void applyDefaults() {
        if (m_currencies.empty()) {
            m_currencies.push_back({"isk",           "ISK",            1.0f, 0.0f,   true});
            m_currencies.push_back({"loyalty_points","Loyalty Points",  0.0f, 0.0f,  false});
        }
        if (m_pricingRules.empty()) {
            m_pricingRules.push_back({"frigate_hull",  "ships",    360000.0f, 1.0f, 1.0f});
            m_pricingRules.push_back({"weapon_turret", "weapons",   25000.0f, 1.2f, 0.8f});
            m_pricingRules.push_back({"ore_ferrium",   "minerals",      5.0f, 1.0f, 1.0f});
        }
    }

    std::string m_id;
    std::string m_title;

    std::vector<CurrencyDefinition> m_currencies;
    std::vector<EconomyPricingRule> m_pricingRules;
    float                           m_globalInflationRate = 0.02f;
    bool                            m_economyEnabled      = true;
};

} // namespace NovaForge
