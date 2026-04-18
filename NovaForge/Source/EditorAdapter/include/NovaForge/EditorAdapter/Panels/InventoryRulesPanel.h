#pragma once
// NovaForge::InventoryRulesPanel — document panel for inventory/module slot authoring.
//
// Loads real module data from data/modules/*.json when a project is opened.
// Derives slot configs (high/med/low) from the actual module definitions.
// Falls back to built-in defaults when files are absent.

#include "NovaForge/EditorAdapter/DocumentPanelBase.h"
#include <filesystem>
#include <fstream>
#include <string>
#include <unordered_set>
#include <vector>
#if __has_include(<nlohmann/json.hpp>)
#  include <nlohmann/json.hpp>
#  define NF_INVENTORY_HAS_JSON 1
#endif

namespace NovaForge {

// ── Inventory data schema ────────────────────────────────────────────────

struct InventorySlotConfig {
    // Original 4-field layout preserved for aggregate-init compatibility
    std::string slotId;
    std::string acceptedCategory;
    int         maxStack = 1;
    bool        locked   = false;
    // Extended field (loaded from JSON)
    std::string slotType;             // "high", "med", "low", "rig"
};

struct StorageRule {
    std::string containerId;
    int         maxSlots  = 20;
    int         maxVolume = 0;        // 0 = unlimited (m³ × 100)
    bool        autoSort  = false;
};

// ── InventoryRulesPanel ──────────────────────────────────────────────────

class InventoryRulesPanel final : public DocumentPanelBase {
public:
    static constexpr const char* kPanelId = "novaforge.inventory_rules";

    InventoryRulesPanel() : m_id(kPanelId), m_title("Inventory Rules") {}

    [[nodiscard]] const std::string& panelId()    const override { return m_id; }
    [[nodiscard]] const std::string& panelTitle() const override { return m_title; }

    [[nodiscard]] const std::vector<InventorySlotConfig>& slots()        const { return m_slots;        }
    [[nodiscard]] const std::vector<StorageRule>&         storageRules() const { return m_storageRules; }
    [[nodiscard]] int  defaultMaxSlots() const { return m_defaultMaxSlots; }
    [[nodiscard]] bool globalStackable() const { return m_globalStackable; }

    // ── Editing API ───────────────────────────────────────────────────────
    void addSlot(InventorySlotConfig slot) {
        auto old = m_slots;
        m_slots.push_back(std::move(slot));
        markFieldChanged();
        m_undoStack.push(PanelUndoEntry{
            "Add Slot",
            [this, cur  = m_slots]() { m_slots = cur;  markFieldChanged(); return true; },
            [this, prev = old]()     { m_slots = prev; markFieldChanged(); return true; }
        });
    }
    void setDefaultMaxSlots(int max) {
        pushPropertyEdit("Set Max Slots", m_defaultMaxSlots, m_defaultMaxSlots, max);
    }
    void setGlobalStackable(bool stackable) {
        pushPropertyEdit("Toggle Stackable", m_globalStackable, m_globalStackable, stackable);
    }
    void addStorageRule(StorageRule rule) {
        auto old = m_storageRules;
        m_storageRules.push_back(std::move(rule));
        markFieldChanged();
        m_undoStack.push(PanelUndoEntry{
            "Add Storage Rule",
            [this, cur  = m_storageRules]() { m_storageRules = cur;  markFieldChanged(); return true; },
            [this, prev = old]()            { m_storageRules = prev; markFieldChanged(); return true; }
        });
    }

    // ── Validation ────────────────────────────────────────────────────────
    [[nodiscard]] std::vector<DocumentPanelValidationMessage> validate() const override {
        std::vector<DocumentPanelValidationMessage> msgs;
        for (const auto& s : m_slots) {
            if (s.slotId.empty())
                msgs.push_back({"slots", "Slot has empty id",
                                DocumentPanelValidationSeverity::Error});
            if (s.maxStack < 1)
                msgs.push_back({"slots." + s.slotId + ".maxStack",
                                "maxStack must be >= 1",
                                DocumentPanelValidationSeverity::Error});
        }
        if (m_defaultMaxSlots < 1)
            msgs.push_back({"defaultMaxSlots", "Max slots must be >= 1",
                            DocumentPanelValidationSeverity::Error});
        return msgs;
    }

    // ── Summary rows (for workspace dashboard display) ────────────────────
    [[nodiscard]] std::vector<std::pair<std::string, std::string>>
        summaryRows() const override
    {
        std::vector<std::pair<std::string, std::string>> rows;
        rows.emplace_back("Default max slots",
                          std::to_string(m_defaultMaxSlots));
        rows.emplace_back("Global stackable",
                          m_globalStackable ? "Yes" : "No");
        rows.emplace_back("Slot configs",
                          std::to_string(m_slots.size()));
        rows.emplace_back("Storage rules",
                          std::to_string(m_storageRules.size()));
        return rows;
    }

    // ── Project load hook ─────────────────────────────────────────────────
    void onProjectLoaded(const std::string& projectRoot) override {
        DocumentPanelBase::onProjectLoaded(projectRoot);
        m_slots.clear();
        m_storageRules.clear();
        loadFromProjectFiles(projectRoot);
    }

    void onProjectUnloaded() override {
        DocumentPanelBase::onProjectUnloaded();
        m_slots.clear();
        m_storageRules.clear();
    }

protected:
    void loadFromDocument(const NovaForgeDocument& /*doc*/) override {
        if (m_slots.empty()) applyDefaults();
    }

private:
    void loadFromProjectFiles([[maybe_unused]] const std::string& projectRoot) {
#ifdef NF_INVENTORY_HAS_JSON
        namespace fs = std::filesystem;

        auto tryDir = [&](const char* rel) -> fs::path {
            fs::path p = fs::path(projectRoot) / rel;
            return (fs::exists(p) && fs::is_directory(p)) ? p : fs::path{};
        };

        fs::path dir = tryDir("data/modules");
        if (dir.empty()) dir = tryDir("Data/Modules");

        // Track which slot types we've seen to build slot configs
        std::unordered_set<std::string> seenSlots;

        if (!dir.empty()) {
            for (auto& entry : fs::directory_iterator(dir)) {
                if (entry.path().extension() != ".json") continue;
                try {
                    std::ifstream f(entry.path());
                    if (!f.is_open()) continue;
                    auto j = nlohmann::json::parse(f, nullptr, false);
                    if (j.is_discarded() || !j.is_object()) continue;
                    for (auto& [id, mj] : j.items()) {
                        if (!mj.is_object()) continue;
                        std::string slotType = mj.value("slot", "cargo");
                        std::string category = mj.value("type", "module");
                        if (seenSlots.insert(slotType).second) {
                            InventorySlotConfig cfg;
                            cfg.slotId           = slotType + "_slot";
                            cfg.slotType         = slotType;
                            cfg.acceptedCategory = category;
                            cfg.maxStack         = 1;
                            cfg.locked           = false;
                            m_slots.push_back(std::move(cfg));
                        }
                    }
                } catch (...) {}
            }
        }
#endif

        // Always provide the canonical ship slot layout
        if (m_slots.empty()) applyDefaults();

        // Standard storage rules are always valid regardless of module data
        if (m_storageRules.empty()) {
            m_storageRules.push_back({"ship_cargo",        400, 40000, true});
            m_storageRules.push_back({"station_hangar", 200000,     0, false});
            m_storageRules.push_back({"drone_bay",          50,  5000, false});
        }
    }

    void applyDefaults() {
        // {slotId, acceptedCategory, maxStack, locked, slotType}
        m_slots.push_back({"high_slot", "weapon", 1, false, "high"});
        m_slots.push_back({"med_slot",  "shield", 1, false, "med"});
        m_slots.push_back({"low_slot",  "armor",  1, false, "low"});
        m_slots.push_back({"rig_slot",  "rig",    1, false, "rig"});
        m_slots.push_back({"cargo",     "any",  500, false, "cargo"});
        m_slots.push_back({"drone_bay", "drone",125, false, "drone"});
    }

    std::string m_id;
    std::string m_title;

    std::vector<InventorySlotConfig> m_slots;
    std::vector<StorageRule>         m_storageRules;
    int                              m_defaultMaxSlots = 400;
    bool                             m_globalStackable = true;
};

} // namespace NovaForge
