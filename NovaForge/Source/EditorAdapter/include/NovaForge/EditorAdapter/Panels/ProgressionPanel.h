#pragma once
// NovaForge::ProgressionPanel — document panel for XP/leveling/skill authoring.
//
// Loads real data from data/skills/skills.json when a project is opened.
// Falls back to built-in defaults when the file is absent.

#include "NovaForge/EditorAdapter/DocumentPanelBase.h"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#if __has_include(<nlohmann/json.hpp>)
#  include <nlohmann/json.hpp>
#  define NF_PROGRESSION_HAS_JSON 1
#endif

namespace NovaForge {

// ── Progression data schema ───────────────────────────────────────────────

struct LevelThreshold {
    int   level        = 1;
    float xpRequired   = 100.0f;
    float xpMultiplier = 1.0f;
};

struct SkillUnlock {
    std::string skillId;
    std::string displayName;
    std::string category;
    int         requiredLevel  = 1;
    int         maxLevel       = 5;
    std::string prerequisiteId;
    int         trainingMultiplier = 1;
};

// ── ProgressionPanel ─────────────────────────────────────────────────────

class ProgressionPanel final : public DocumentPanelBase {
public:
    static constexpr const char* kPanelId = "novaforge.progression";

    ProgressionPanel() : m_id(kPanelId), m_title("Progression") {}

    [[nodiscard]] const std::string& panelId()    const override { return m_id; }
    [[nodiscard]] const std::string& panelTitle() const override { return m_title; }

    [[nodiscard]] const std::vector<LevelThreshold>& levelThresholds() const { return m_levels; }
    [[nodiscard]] const std::vector<SkillUnlock>&    skillUnlocks()    const { return m_skills; }
    [[nodiscard]] int   maxLevel()            const { return m_maxLevel; }
    [[nodiscard]] float xpGlobalMultiplier()  const { return m_xpGlobalMultiplier; }
    [[nodiscard]] int   skillPointsPerLevel() const { return m_skillPointsPerLevel; }

    // ── Editing API ───────────────────────────────────────────────────────
    void setMaxLevel(int max) {
        pushPropertyEdit("Set Max Level", m_maxLevel, m_maxLevel, max);
    }
    void setXpGlobalMultiplier(float mult) {
        pushPropertyEdit("Set XP Multiplier",
                         m_xpGlobalMultiplier, m_xpGlobalMultiplier, mult);
    }
    void setSkillPointsPerLevel(int pts) {
        pushPropertyEdit("Set Skill Points Per Level",
                         m_skillPointsPerLevel, m_skillPointsPerLevel, pts);
    }
    void addLevelThreshold(LevelThreshold thresh) {
        auto old = m_levels;
        m_levels.push_back(std::move(thresh));
        std::sort(m_levels.begin(), m_levels.end(),
                  [](const auto& a, const auto& b) { return a.level < b.level; });
        markFieldChanged();
        m_undoStack.push(PanelUndoEntry{
            "Add Level Threshold",
            [this, cur  = m_levels]() { m_levels = cur;  markFieldChanged(); return true; },
            [this, prev = old]()      { m_levels = prev; markFieldChanged(); return true; }
        });
    }
    void addSkillUnlock(SkillUnlock skill) {
        auto old = m_skills;
        m_skills.push_back(std::move(skill));
        markFieldChanged();
        m_undoStack.push(PanelUndoEntry{
            "Add Skill",
            [this, cur  = m_skills]() { m_skills = cur;  markFieldChanged(); return true; },
            [this, prev = old]()      { m_skills = prev; markFieldChanged(); return true; }
        });
    }

    // ── Validation ────────────────────────────────────────────────────────
    [[nodiscard]] std::vector<DocumentPanelValidationMessage> validate() const override {
        std::vector<DocumentPanelValidationMessage> msgs;
        if (m_maxLevel < 1)
            msgs.push_back({"maxLevel", "Max level must be >= 1",
                            DocumentPanelValidationSeverity::Error});
        if (m_xpGlobalMultiplier <= 0.0f)
            msgs.push_back({"xpGlobalMultiplier", "XP multiplier must be > 0",
                            DocumentPanelValidationSeverity::Error});
        for (const auto& s : m_skills)
            if (s.skillId.empty())
                msgs.push_back({"skills", "Skill has empty id",
                                DocumentPanelValidationSeverity::Error});
        return msgs;
    }

    // ── Summary rows (for workspace dashboard display) ────────────────────
    [[nodiscard]] std::vector<std::pair<std::string, std::string>>
        summaryRows() const override
    {
        std::vector<std::pair<std::string, std::string>> rows;
        rows.emplace_back("Max level",
                          std::to_string(m_maxLevel));
        rows.emplace_back("XP multiplier",
                          std::to_string(m_xpGlobalMultiplier));
        rows.emplace_back("Skill points / level",
                          std::to_string(m_skillPointsPerLevel));
        rows.emplace_back("Level thresholds",
                          std::to_string(m_levels.size()));
        rows.emplace_back("Skill unlocks",
                          std::to_string(m_skills.size()));
        return rows;
    }

    // ── Project load hook ─────────────────────────────────────────────────
    void onProjectLoaded(const std::string& projectRoot) override {
        DocumentPanelBase::onProjectLoaded(projectRoot);
        m_levels.clear();
        m_skills.clear();
        loadFromProjectFiles(projectRoot);
    }

    void onProjectUnloaded() override {
        DocumentPanelBase::onProjectUnloaded();
        m_levels.clear();
        m_skills.clear();
    }

protected:
    void loadFromDocument(const NovaForgeDocument& /*doc*/) override {
        if (m_skills.empty()) applyDefaults();
    }

private:
    void loadFromProjectFiles([[maybe_unused]] const std::string& projectRoot) {
#ifdef NF_PROGRESSION_HAS_JSON
        namespace fs = std::filesystem;

        auto tryFile = [&](const char* rel) -> fs::path {
            fs::path p = fs::path(projectRoot) / rel;
            return fs::exists(p) ? p : fs::path{};
        };

        // Load all skill files (skills.json + science_skills.json)
        for (auto* rel : {"data/skills/skills.json",
                          "data/skills/science_skills.json",
                          "Data/Skills/skills.json"}) {
            fs::path p = tryFile(rel);
            if (p.empty()) continue;
            try {
                std::ifstream f(p);
                if (!f.is_open()) continue;
                auto j = nlohmann::json::parse(f, nullptr, false);
                if (j.is_discarded() || !j.is_object()) continue;
                for (auto& [id, sj] : j.items()) {
                    if (!sj.is_object()) continue;
                    SkillUnlock skill;
                    skill.skillId           = id;
                    skill.displayName       = sj.value("name",                 id);
                    skill.category          = sj.value("category",             "General");
                    skill.maxLevel          = sj.value("max_level",            5);
                    skill.trainingMultiplier= sj.value("training_multiplier",  1);
                    skill.requiredLevel     = 1;
                    if (sj.contains("prerequisites") && sj["prerequisites"].is_object()) {
                        for (auto& [prereqId, lvl] : sj["prerequisites"].items()) {
                            skill.prerequisiteId = prereqId;
                            skill.requiredLevel  = lvl.get<int>();
                            break; // use first prereq as the primary one
                        }
                    }
                    m_skills.push_back(std::move(skill));
                }
            } catch (...) {}
        }
#endif
        // Generate level thresholds (always — not driven by external data)
        if (m_levels.empty()) {
            float xp = 100.0f;
            for (int i = 1; i <= m_maxLevel; ++i) {
                m_levels.push_back({i, xp, 1.0f + (i - 1) * 0.05f});
                xp *= 1.4f;
            }
        }
        if (m_skills.empty()) applyDefaults();
    }

    void applyDefaults() {
        m_skills.push_back({"gunnery",       "Gunnery",            "Combat",    1, 5, "",         1});
        m_skills.push_back({"missiles",      "Missiles",           "Combat",    1, 5, "",         1});
        m_skills.push_back({"navigation",    "Navigation",         "Piloting",  1, 5, "",         1});
        m_skills.push_back({"spaceship_cmd", "Spaceship Command",  "Piloting",  1, 5, "",         2});
        m_skills.push_back({"mining",        "Mining",             "Industry",  1, 5, "",         1});
        if (m_levels.empty()) {
            float xp = 100.0f;
            for (int i = 1; i <= 50; ++i) {
                m_levels.push_back({i, xp, 1.0f + (i - 1) * 0.05f});
                xp *= 1.4f;
            }
        }
    }

    std::string m_id;
    std::string m_title;

    std::vector<LevelThreshold> m_levels;
    std::vector<SkillUnlock>    m_skills;
    int                         m_maxLevel            = 50;
    float                       m_xpGlobalMultiplier  = 1.0f;
    int                         m_skillPointsPerLevel = 1;
};

} // namespace NovaForge
