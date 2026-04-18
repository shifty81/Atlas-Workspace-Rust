#pragma once
// NF::Editor — Quest editor panel
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

enum class QuestStatus : uint8_t {
    Locked, Available, Active, Completed, Failed, Abandoned
};

inline const char* questStatusName(QuestStatus s) {
    switch (s) {
        case QuestStatus::Locked:    return "Locked";
        case QuestStatus::Available: return "Available";
        case QuestStatus::Active:    return "Active";
        case QuestStatus::Completed: return "Completed";
        case QuestStatus::Failed:    return "Failed";
        case QuestStatus::Abandoned: return "Abandoned";
    }
    return "Unknown";
}

enum class QuestCategory : uint8_t {
    Main, Side, Daily, Hidden, Tutorial, Achievement
};

inline const char* questCategoryName(QuestCategory c) {
    switch (c) {
        case QuestCategory::Main:        return "Main";
        case QuestCategory::Side:        return "Side";
        case QuestCategory::Daily:       return "Daily";
        case QuestCategory::Hidden:      return "Hidden";
        case QuestCategory::Tutorial:    return "Tutorial";
        case QuestCategory::Achievement: return "Achievement";
    }
    return "Unknown";
}

enum class QuestObjectiveType : uint8_t {
    Collect, Kill, Reach, Talk, Craft, Survive, Explore, Deliver, Protect
};

inline const char* questObjectiveTypeName(QuestObjectiveType t) {
    switch (t) {
        case QuestObjectiveType::Collect:  return "Collect";
        case QuestObjectiveType::Kill:     return "Kill";
        case QuestObjectiveType::Reach:    return "Reach";
        case QuestObjectiveType::Talk:     return "Talk";
        case QuestObjectiveType::Craft:    return "Craft";
        case QuestObjectiveType::Survive:  return "Survive";
        case QuestObjectiveType::Explore:  return "Explore";
        case QuestObjectiveType::Deliver:  return "Deliver";
        case QuestObjectiveType::Protect:  return "Protect";
    }
    return "Unknown";
}

class QuestObjective {
public:
    explicit QuestObjective(const std::string& id, QuestObjectiveType type)
        : m_id(id), m_type(type) {}

    void setDescription(const std::string& d) { m_description = d; }
    void setRequired(bool v)                  { m_required    = v; }
    void setTargetCount(uint32_t n)           { m_targetCount = n; }

    [[nodiscard]] const std::string& id()          const { return m_id;          }
    [[nodiscard]] QuestObjectiveType type()        const { return m_type;        }
    [[nodiscard]] const std::string& description() const { return m_description; }
    [[nodiscard]] bool               isRequired()  const { return m_required;    }
    [[nodiscard]] uint32_t           targetCount() const { return m_targetCount; }

private:
    std::string      m_id;
    QuestObjectiveType m_type;
    std::string      m_description;
    uint32_t         m_targetCount = 1;
    bool             m_required    = true;
};

class QuestAsset {
public:
    explicit QuestAsset(const std::string& id, QuestCategory category)
        : m_id(id), m_category(category) {}

    void setStatus(QuestStatus s)   { m_status = s; }
    void setTitle(const std::string& t)  { m_title  = t; }
    void setEnabled(bool v)         { m_enabled = v; }

    [[nodiscard]] bool addObjective(const QuestObjective& obj) {
        for (auto& o : m_objectives) if (o.id() == obj.id()) return false;
        m_objectives.push_back(obj); return true;
    }

    [[nodiscard]] const std::string& id()             const { return m_id;       }
    [[nodiscard]] QuestCategory      category()       const { return m_category; }
    [[nodiscard]] QuestStatus        status()         const { return m_status;   }
    [[nodiscard]] const std::string& title()          const { return m_title;    }
    [[nodiscard]] bool               isEnabled()      const { return m_enabled;  }
    [[nodiscard]] size_t             objectiveCount() const { return m_objectives.size(); }
    [[nodiscard]] size_t             requiredObjectiveCount() const {
        size_t c = 0; for (auto& o : m_objectives) if (o.isRequired()) ++c; return c;
    }

private:
    std::string              m_id;
    QuestCategory            m_category;
    std::vector<QuestObjective> m_objectives;
    QuestStatus              m_status  = QuestStatus::Locked;
    std::string              m_title;
    bool                     m_enabled = true;
};

class QuestEditor {
public:
    static constexpr size_t MAX_QUESTS = 1024;

    [[nodiscard]] bool addQuest(const QuestAsset& quest) {
        for (auto& q : m_quests) if (q.id() == quest.id()) return false;
        if (m_quests.size() >= MAX_QUESTS) return false;
        m_quests.push_back(quest);
        return true;
    }

    [[nodiscard]] bool removeQuest(const std::string& id) {
        for (auto it = m_quests.begin(); it != m_quests.end(); ++it) {
            if (it->id() == id) { m_quests.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] QuestAsset* findQuest(const std::string& id) {
        for (auto& q : m_quests) if (q.id() == id) return &q;
        return nullptr;
    }

    [[nodiscard]] size_t questCount()  const { return m_quests.size(); }
    [[nodiscard]] size_t countByStatus(QuestStatus s) const {
        size_t c = 0; for (auto& q : m_quests) if (q.status() == s) ++c; return c;
    }
    [[nodiscard]] size_t countByCategory(QuestCategory cat) const {
        size_t c = 0; for (auto& q : m_quests) if (q.category() == cat) ++c; return c;
    }
    [[nodiscard]] size_t enabledCount() const {
        size_t c = 0; for (auto& q : m_quests) if (q.isEnabled()) ++c; return c;
    }

private:
    std::vector<QuestAsset> m_quests;
};

} // namespace NF
