#pragma once
// NovaForge::MissionRulesPanel — document panel for mission/quest rule authoring.
//
// Loads real mission data from data/missions/*.json when a project is opened.
// Falls back to built-in defaults when files are absent.

#include "NovaForge/EditorAdapter/DocumentPanelBase.h"
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#if __has_include(<nlohmann/json.hpp>)
#  include <nlohmann/json.hpp>
#  define NF_MISSION_HAS_JSON 1
#endif

namespace NovaForge {

// ── Mission data schema ───────────────────────────────────────────────────

enum class ObjectiveType : uint8_t {
    Kill      = 0,
    Collect   = 1,
    Reach     = 2,
    Interact  = 3,
    Escort    = 4,
    Defend    = 5,
    Survive   = 6,
    Mine      = 7,
    Custom    = 8,
};

inline const char* objectiveTypeName(ObjectiveType t) {
    switch (t) {
    case ObjectiveType::Kill:     return "Kill";
    case ObjectiveType::Collect:  return "Collect";
    case ObjectiveType::Reach:    return "Reach";
    case ObjectiveType::Interact: return "Interact";
    case ObjectiveType::Escort:   return "Escort";
    case ObjectiveType::Defend:   return "Defend";
    case ObjectiveType::Survive:  return "Survive";
    case ObjectiveType::Mine:     return "Mine";
    case ObjectiveType::Custom:   return "Custom";
    }
    return "Unknown";
}

struct MissionObjective {
    std::string   objectiveId;
    ObjectiveType type        = ObjectiveType::Kill;
    std::string   targetId;
    int           targetCount = 1;
    bool          optional    = false;
};

struct MissionReward {
    std::string currencyId;
    float       amount    = 0.0f;
    std::string itemId;
    int         itemCount = 0;
    int         loyaltyPoints = 0;
};

struct MissionDefinitionData {
    // Original 6-field layout preserved for aggregate-init compatibility
    std::string                   missionId;
    std::string                   displayName;
    std::string                   prerequisiteMissionId;
    std::vector<MissionObjective> objectives;
    std::vector<MissionReward>    rewards;
    bool                          repeatable = false;
    // Extended fields (loaded from JSON, default-initialised otherwise)
    std::string                   description;
    std::string                   faction;
    int                           level = 1;
};

// ── MissionRulesPanel ────────────────────────────────────────────────────

class MissionRulesPanel final : public DocumentPanelBase {
public:
    static constexpr const char* kPanelId = "novaforge.mission_rules";

    MissionRulesPanel() : m_id(kPanelId), m_title("Mission Rules") {}

    [[nodiscard]] const std::string& panelId()    const override { return m_id; }
    [[nodiscard]] const std::string& panelTitle() const override { return m_title; }

    [[nodiscard]] const std::vector<MissionDefinitionData>& missions() const { return m_missions; }
    [[nodiscard]] int  maxActiveMissions() const { return m_maxActiveMissions; }
    [[nodiscard]] bool chainEnabled()      const { return m_chainEnabled; }

    // ── Editing API ───────────────────────────────────────────────────────
    void addMission(MissionDefinitionData def) {
        auto old = m_missions;
        m_missions.push_back(std::move(def));
        markFieldChanged();
        m_undoStack.push(PanelUndoEntry{
            "Add Mission",
            [this, cur  = m_missions]() { m_missions = cur;  markFieldChanged(); return true; },
            [this, prev = old]()        { m_missions = prev; markFieldChanged(); return true; }
        });
    }

    void removeMission(const std::string& missionId) {
        auto it = std::find_if(m_missions.begin(), m_missions.end(),
                               [&](const auto& m) { return m.missionId == missionId; });
        if (it == m_missions.end()) return;
        auto old = m_missions;
        m_missions.erase(it);
        markFieldChanged();
        m_undoStack.push(PanelUndoEntry{
            "Remove Mission",
            [this, cur  = m_missions]() { m_missions = cur;  markFieldChanged(); return true; },
            [this, prev = old]()        { m_missions = prev; markFieldChanged(); return true; }
        });
    }

    void setMaxActiveMissions(int max) {
        pushPropertyEdit("Set Max Active Missions",
                         m_maxActiveMissions, m_maxActiveMissions, max);
    }
    void setChainEnabled(bool enabled) {
        pushPropertyEdit("Toggle Mission Chains", m_chainEnabled, m_chainEnabled, enabled);
    }

    // ── Validation ────────────────────────────────────────────────────────
    [[nodiscard]] std::vector<DocumentPanelValidationMessage> validate() const override {
        std::vector<DocumentPanelValidationMessage> msgs;
        for (const auto& m : m_missions) {
            if (m.missionId.empty())
                msgs.push_back({"missions", "Mission has empty id",
                                DocumentPanelValidationSeverity::Error});
            if (m.objectives.empty())
                msgs.push_back({"missions." + m.missionId + ".objectives",
                                "Mission has no objectives",
                                DocumentPanelValidationSeverity::Warning});
        }
        if (m_maxActiveMissions < 1)
            msgs.push_back({"maxActiveMissions", "Must be >= 1",
                            DocumentPanelValidationSeverity::Error});
        return msgs;
    }

    // ── Summary rows (for workspace dashboard display) ────────────────────
    [[nodiscard]] std::vector<std::pair<std::string, std::string>>
        summaryRows() const override
    {
        std::vector<std::pair<std::string, std::string>> rows;
        rows.emplace_back("Missions",
                          std::to_string(m_missions.size()));
        rows.emplace_back("Max active missions",
                          std::to_string(m_maxActiveMissions));
        rows.emplace_back("Mission chains",
                          m_chainEnabled ? "Enabled" : "Disabled");
        if (!m_missions.empty())
            rows.emplace_back("First mission", m_missions[0].displayName);
        return rows;
    }

    // ── Project load hook ─────────────────────────────────────────────────
    void onProjectLoaded(const std::string& projectRoot) override {
        DocumentPanelBase::onProjectLoaded(projectRoot);
        m_missions.clear();
        loadFromProjectFiles(projectRoot);
    }

    void onProjectUnloaded() override {
        DocumentPanelBase::onProjectUnloaded();
        m_missions.clear();
    }

protected:
    void loadFromDocument(const NovaForgeDocument& /*doc*/) override {
        if (m_missions.empty()) applyDefaults();
    }

private:
#ifdef NF_MISSION_HAS_JSON
    static ObjectiveType parseObjType(const std::string& s) {
        if (s == "destroy" || s == "kill")  return ObjectiveType::Kill;
        if (s == "mine")                    return ObjectiveType::Mine;
        if (s == "collect")                 return ObjectiveType::Collect;
        if (s == "reach")                   return ObjectiveType::Reach;
        if (s == "escort")                  return ObjectiveType::Escort;
        if (s == "defend")                  return ObjectiveType::Defend;
        if (s == "survive")                 return ObjectiveType::Survive;
        if (s == "interact")                return ObjectiveType::Interact;
        return ObjectiveType::Custom;
    }

    void loadMissionFile(const std::filesystem::path& path) {
        try {
            std::ifstream f(path);
            if (!f.is_open()) return;
            auto j = nlohmann::json::parse(f, nullptr, false);
            if (j.is_discarded() || !j.is_object()) return;

            for (auto& [id, mj] : j.items()) {
                MissionDefinitionData def;
                def.missionId   = id;
                def.displayName = mj.value("name",        id);
                def.description = mj.value("description", "");
                def.faction     = mj.value("faction",     "");
                def.level       = mj.value("level",       1);
                def.repeatable  = false;

                if (mj.contains("objectives") && mj["objectives"].is_array()) {
                    int idx = 0;
                    for (auto& oj : mj["objectives"]) {
                        MissionObjective obj;
                        obj.objectiveId  = id + "_obj" + std::to_string(idx++);
                        obj.type         = parseObjType(oj.value("type", "kill"));
                        obj.targetId     = oj.value("target", "");
                        obj.targetCount  = oj.contains("count")    ? oj["count"].get<int>()
                                         : oj.contains("quantity") ? oj["quantity"].get<int>()
                                         : 1;
                        obj.optional     = false;
                        def.objectives.push_back(std::move(obj));
                    }
                }

                if (mj.contains("rewards") && mj["rewards"].is_object()) {
                    auto& rj = mj["rewards"];
                    MissionReward reward;
                    reward.currencyId   = "isk";
                    reward.amount       = rj.value("isk", 0.0f);
                    reward.loyaltyPoints= rj.value("loyalty_points", 0);
                    def.rewards.push_back(std::move(reward));
                }

                m_missions.push_back(std::move(def));
            }
        } catch (...) {}
    }
#endif

    void loadFromProjectFiles([[maybe_unused]] const std::string& projectRoot) {
#ifdef NF_MISSION_HAS_JSON
        namespace fs = std::filesystem;

        auto tryDir = [&](const char* rel) -> fs::path {
            fs::path p = fs::path(projectRoot) / rel;
            return (fs::exists(p) && fs::is_directory(p)) ? p : fs::path{};
        };

        fs::path dir = tryDir("data/missions");
        if (dir.empty()) dir = tryDir("Data/Missions");
        if (!dir.empty()) {
            for (auto& entry : fs::directory_iterator(dir)) {
                if (entry.path().extension() == ".json")
                    loadMissionFile(entry.path());
            }
        }
#endif
        if (m_missions.empty()) applyDefaults();
    }

    void applyDefaults() {
        MissionDefinitionData intro;
        intro.missionId   = "first_steps";
        intro.displayName = "First Steps";
        intro.description = "Get your bearings in the Astralis cluster.";
        intro.faction     = "Veyren Fleet";
        intro.level       = 1;
        intro.objectives  = {{"obj1", ObjectiveType::Reach, "thyrkstad_station", 1, false}};
        intro.rewards     = {{"isk", 25000.0f, "", 0, 100}};
        m_missions.push_back(std::move(intro));

        MissionDefinitionData hunt;
        hunt.missionId            = "venom_hunt";
        hunt.displayName          = "Venom Syndicate Infestation";
        hunt.description          = "Eliminate Venom Syndicate pirates from the belt.";
        hunt.faction              = "Veyren Fleet";
        hunt.level                = 1;
        hunt.prerequisiteMissionId= "first_steps";
        hunt.objectives           = {{"obj1", ObjectiveType::Kill, "venom_syndicate_scout", 5, false}};
        hunt.rewards              = {{"isk", 50000.0f, "", 0, 150}};
        hunt.repeatable           = true;
        m_missions.push_back(std::move(hunt));
    }

    std::string m_id;
    std::string m_title;

    std::vector<MissionDefinitionData> m_missions;
    int                                m_maxActiveMissions = 5;
    bool                               m_chainEnabled      = true;
};

} // namespace NovaForge
