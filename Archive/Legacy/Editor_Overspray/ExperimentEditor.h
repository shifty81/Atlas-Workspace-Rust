#pragma once
// NF::Editor — experiment (A/B testing) editor
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

enum class ExperimentStatus : uint8_t {
    Draft, Active, Paused, Concluded, Archived
};

inline const char* experimentStatusName(ExperimentStatus s) {
    switch (s) {
        case ExperimentStatus::Draft:     return "Draft";
        case ExperimentStatus::Active:    return "Active";
        case ExperimentStatus::Paused:    return "Paused";
        case ExperimentStatus::Concluded: return "Concluded";
        case ExperimentStatus::Archived:  return "Archived";
    }
    return "Unknown";
}

enum class ExperimentType : uint8_t {
    BinaryAB, Multivariate, Bandit, HoldOut, Custom
};

inline const char* experimentTypeName(ExperimentType t) {
    switch (t) {
        case ExperimentType::BinaryAB:     return "BinaryAB";
        case ExperimentType::Multivariate: return "Multivariate";
        case ExperimentType::Bandit:       return "Bandit";
        case ExperimentType::HoldOut:      return "HoldOut";
        case ExperimentType::Custom:       return "Custom";
    }
    return "Unknown";
}

class ExperimentEntry {
public:
    explicit ExperimentEntry(uint32_t id, const std::string& name,
                             ExperimentStatus status, ExperimentType type)
        : m_id(id), m_name(name), m_status(status), m_type(type) {}

    void setStatus(ExperimentStatus v)  { m_status         = v; }
    void setType(ExperimentType v)      { m_type           = v; }
    void setTrafficPercent(float v)     { m_trafficPercent = v; }
    void setIsAnalyzed(bool v)          { m_isAnalyzed     = v; }

    [[nodiscard]] uint32_t           id()             const { return m_id;             }
    [[nodiscard]] const std::string& name()           const { return m_name;           }
    [[nodiscard]] ExperimentStatus   status()         const { return m_status;         }
    [[nodiscard]] ExperimentType     type()           const { return m_type;           }
    [[nodiscard]] float              trafficPercent() const { return m_trafficPercent; }
    [[nodiscard]] bool               isAnalyzed()     const { return m_isAnalyzed;     }

private:
    uint32_t         m_id;
    std::string      m_name;
    ExperimentStatus m_status;
    ExperimentType   m_type;
    float            m_trafficPercent = 10.0f;
    bool             m_isAnalyzed     = false;
};

class ExperimentEditor {
public:
    void setIsShowArchived(bool v)        { m_isShowArchived    = v; }
    void setIsGroupByType(bool v)         { m_isGroupByType     = v; }
    void setMinTrafficPercent(float v)    { m_minTrafficPercent = v; }

    bool addExperiment(const ExperimentEntry& e) {
        for (auto& x : m_experiments) if (x.id() == e.id()) return false;
        m_experiments.push_back(e); return true;
    }
    bool removeExperiment(uint32_t id) {
        auto it = std::find_if(m_experiments.begin(), m_experiments.end(),
            [&](const ExperimentEntry& e){ return e.id() == id; });
        if (it == m_experiments.end()) return false;
        m_experiments.erase(it); return true;
    }
    [[nodiscard]] ExperimentEntry* findExperiment(uint32_t id) {
        for (auto& e : m_experiments) if (e.id() == id) return &e;
        return nullptr;
    }

    [[nodiscard]] bool  isShowArchived()    const { return m_isShowArchived;    }
    [[nodiscard]] bool  isGroupByType()     const { return m_isGroupByType;     }
    [[nodiscard]] float minTrafficPercent() const { return m_minTrafficPercent; }
    [[nodiscard]] size_t experimentCount()  const { return m_experiments.size(); }

    [[nodiscard]] size_t countByStatus(ExperimentStatus s) const {
        size_t n = 0; for (auto& e : m_experiments) if (e.status() == s) ++n; return n;
    }
    [[nodiscard]] size_t countByType(ExperimentType t) const {
        size_t n = 0; for (auto& e : m_experiments) if (e.type() == t) ++n; return n;
    }
    [[nodiscard]] size_t countActive() const {
        size_t n = 0; for (auto& e : m_experiments) if (e.status() == ExperimentStatus::Active) ++n; return n;
    }

private:
    std::vector<ExperimentEntry> m_experiments;
    bool  m_isShowArchived    = false;
    bool  m_isGroupByType     = false;
    float m_minTrafficPercent = 5.0f;
};

} // namespace NF
