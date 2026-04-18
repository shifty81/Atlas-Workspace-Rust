#pragma once
// NovaForge::CharacterRulesPanel — document panel for character rule authoring.
//
// Loads real data from data/character_creation/races.json and careers.json
// when a project is opened. Falls back to built-in defaults when absent.

#include "NovaForge/EditorAdapter/DocumentPanelBase.h"
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#if __has_include(<nlohmann/json.hpp>)
#  include <nlohmann/json.hpp>
#  define NF_CHARACTER_HAS_JSON 1
#endif

namespace NovaForge {

// ── Character data schema ─────────────────────────────────────────────────

struct StatCapEntry {
    std::string statId;
    float       baseValue = 0.0f;
    float       minValue  = 0.0f;
    float       maxValue  = 100.0f;
};

struct CharacterClassPreset {
    // Original 6-field layout preserved for aggregate-init compatibility
    std::string               classId;
    std::string               displayName;
    std::string               description;
    std::vector<StatCapEntry> baseStats;
    std::vector<std::string>  startingSkills;
    bool                      selectable = true;
    // Extended field (loaded from JSON)
    std::string               raceId;
};

struct AppearanceConfig {
    int  numBodyPresets      = 5;
    int  numFacePresets      = 10;
    bool customColorEnabled  = true;
    bool heightSlider        = true;
};

// ── CharacterRulesPanel ──────────────────────────────────────────────────

class CharacterRulesPanel final : public DocumentPanelBase {
public:
    static constexpr const char* kPanelId = "novaforge.character_rules";

    CharacterRulesPanel() : m_id(kPanelId), m_title("Character Rules") {}

    [[nodiscard]] const std::string& panelId()    const override { return m_id; }
    [[nodiscard]] const std::string& panelTitle() const override { return m_title; }

    [[nodiscard]] const std::vector<CharacterClassPreset>& classPresets()     const { return m_classes;    }
    [[nodiscard]] const AppearanceConfig&                  appearanceConfig()  const { return m_appearance; }
    [[nodiscard]] int  maxCharactersPerAccount() const { return m_maxCharactersPerAccount; }
    [[nodiscard]] bool customClassEnabled()      const { return m_customClassEnabled; }

    // ── Editing API ───────────────────────────────────────────────────────
    void addClass(CharacterClassPreset preset) {
        auto old = m_classes;
        m_classes.push_back(std::move(preset));
        markFieldChanged();
        m_undoStack.push(PanelUndoEntry{
            "Add Class",
            [this, cur  = m_classes]() { m_classes = cur;  markFieldChanged(); return true; },
            [this, prev = old]()       { m_classes = prev; markFieldChanged(); return true; }
        });
    }
    void removeClass(const std::string& classId) {
        auto it = std::find_if(m_classes.begin(), m_classes.end(),
                               [&](const auto& c) { return c.classId == classId; });
        if (it == m_classes.end()) return;
        auto old = m_classes;
        m_classes.erase(it);
        markFieldChanged();
        m_undoStack.push(PanelUndoEntry{
            "Remove Class",
            [this, cur  = m_classes]() { m_classes = cur;  markFieldChanged(); return true; },
            [this, prev = old]()       { m_classes = prev; markFieldChanged(); return true; }
        });
    }
    void setMaxCharactersPerAccount(int max) {
        pushPropertyEdit("Set Max Characters",
                         m_maxCharactersPerAccount, m_maxCharactersPerAccount, max);
    }
    void setCustomClassEnabled(bool enabled) {
        pushPropertyEdit("Toggle Custom Class",
                         m_customClassEnabled, m_customClassEnabled, enabled);
    }

    // ── Validation ────────────────────────────────────────────────────────
    [[nodiscard]] std::vector<DocumentPanelValidationMessage> validate() const override {
        std::vector<DocumentPanelValidationMessage> msgs;
        if (m_classes.empty())
            msgs.push_back({"classes", "At least one class preset is required",
                            DocumentPanelValidationSeverity::Error});
        for (const auto& c : m_classes)
            if (c.classId.empty())
                msgs.push_back({"classes", "Class has empty id",
                                DocumentPanelValidationSeverity::Error});
        if (m_maxCharactersPerAccount < 1)
            msgs.push_back({"maxCharactersPerAccount", "Must be >= 1",
                            DocumentPanelValidationSeverity::Error});
        return msgs;
    }

    // ── Summary rows (for workspace dashboard display) ────────────────────
    [[nodiscard]] std::vector<std::pair<std::string, std::string>>
        summaryRows() const override
    {
        std::vector<std::pair<std::string, std::string>> rows;
        rows.emplace_back("Class presets",
                          std::to_string(m_classes.size()));
        rows.emplace_back("Max characters / account",
                          std::to_string(m_maxCharactersPerAccount));
        rows.emplace_back("Custom classes",
                          m_customClassEnabled ? "Enabled" : "Disabled");
        if (!m_classes.empty())
            rows.emplace_back("First class", m_classes[0].displayName);
        return rows;
    }

    // ── Project load hook ─────────────────────────────────────────────────
    void onProjectLoaded(const std::string& projectRoot) override {
        DocumentPanelBase::onProjectLoaded(projectRoot);
        m_classes.clear();
        loadFromProjectFiles(projectRoot);
    }

    void onProjectUnloaded() override {
        DocumentPanelBase::onProjectUnloaded();
        m_classes.clear();
    }

protected:
    void loadFromDocument(const NovaForgeDocument& /*doc*/) override {
        if (m_classes.empty()) applyDefaults();
    }

private:
    void loadFromProjectFiles([[maybe_unused]] const std::string& projectRoot) {
#ifdef NF_CHARACTER_HAS_JSON
        namespace fs = std::filesystem;

        auto tryFile = [&](const char* rel) -> fs::path {
            fs::path p = fs::path(projectRoot) / rel;
            return fs::exists(p) ? p : fs::path{};
        };

        // Load races → each race + each bloodline becomes a CharacterClassPreset
        for (auto* rel : {"data/character_creation/races.json",
                          "Data/CharacterCreation/races.json"}) {
            fs::path p = tryFile(rel);
            if (p.empty()) continue;
            try {
                std::ifstream f(p);
                if (!f.is_open()) continue;
                auto j = nlohmann::json::parse(f, nullptr, false);
                if (j.is_discarded()) continue;
                auto& races = j.contains("races") ? j["races"] : j;
                for (auto& [raceId, rj] : races.items()) {
                    if (!rj.is_object()) continue;
                    // Add a preset for each bloodline within the race
                    if (rj.contains("bloodlines") && rj["bloodlines"].is_object()) {
                        for (auto& [blId, blj] : rj["bloodlines"].items()) {
                            CharacterClassPreset preset;
                            preset.classId     = raceId + "_" + blId;
                            preset.displayName = rj.value("name", raceId)
                                              + " / " + blj.value("name", blId);
                            preset.description = blj.value("description", "");
                            preset.raceId      = raceId;
                            preset.selectable  = true;

                            // Map race base attributes to stat caps
                            if (rj.contains("base_attributes") &&
                                rj["base_attributes"].is_object()) {
                                for (auto& [stat, val] : rj["base_attributes"].items()) {
                                    float base = val.get<float>();
                                    preset.baseStats.push_back(
                                        {stat, base, 0.0f, base + 5.0f});
                                }
                            }

                            // Starting skills from bloodline
                            if (blj.contains("starting_skills") &&
                                blj["starting_skills"].is_object()) {
                                for (auto& [sk, _] : blj["starting_skills"].items())
                                    preset.startingSkills.push_back(sk);
                            }
                            m_classes.push_back(std::move(preset));
                        }
                    }
                }
            } catch (...) {}
            if (!m_classes.empty()) break;
        }

        // Also load careers as additional class descriptors
        for (auto* rel : {"data/character_creation/careers.json",
                          "Data/CharacterCreation/careers.json"}) {
            fs::path p = tryFile(rel);
            if (p.empty()) continue;
            try {
                std::ifstream f(p);
                if (!f.is_open()) continue;
                auto j = nlohmann::json::parse(f, nullptr, false);
                if (j.is_discarded()) continue;
                auto& careers = j.contains("careers") ? j["careers"] : j;
                for (auto& [cid, cj] : careers.items()) {
                    if (!cj.is_object()) continue;
                    CharacterClassPreset preset;
                    preset.classId     = "career_" + cid;
                    preset.displayName = "Career: " + cj.value("name", cid);
                    preset.description = cj.value("description", "");
                    preset.selectable  = true;
                    if (cj.contains("bonus_skills") && cj["bonus_skills"].is_object())
                        for (auto& [sk, _] : cj["bonus_skills"].items())
                            preset.startingSkills.push_back(sk);
                    m_classes.push_back(std::move(preset));
                }
            } catch (...) {}
            break;
        }
#endif
        if (m_classes.empty()) applyDefaults();
    }

    void applyDefaults() {
        // {classId, displayName, description, baseStats, startingSkills, selectable, raceId}
        CharacterClassPreset solari;
        solari.classId       = "solari_solarian";
        solari.displayName   = "Solari / Solarian";
        solari.description   = "A proud warrior lineage of the Solari Dominion.";
        solari.baseStats     = {{"perception",7,0,15},{"memory",7,0,15},{"willpower",5,0,15}};
        solari.startingSkills= {"small_energy","gunnery","spaceship_command"};
        solari.selectable    = true;
        solari.raceId        = "solari";
        m_classes.push_back(std::move(solari));

        CharacterClassPreset veyren;
        veyren.classId       = "veyren_corsair";
        veyren.displayName   = "Veyren / Corsair";
        veyren.description   = "Cunning traders and scouts from the Veyren borderlands.";
        veyren.baseStats     = {{"perception",6,0,15},{"memory",8,0,15},{"charisma",7,0,15}};
        veyren.startingSkills= {"navigation","trade","missiles"};
        veyren.selectable    = true;
        veyren.raceId        = "veyren";
        m_classes.push_back(std::move(veyren));
    }

    std::string m_id;
    std::string m_title;

    std::vector<CharacterClassPreset> m_classes;
    AppearanceConfig                  m_appearance              = {5, 10, true, true};
    int                               m_maxCharactersPerAccount = 3;
    bool                              m_customClassEnabled      = false;
};

} // namespace NovaForge
