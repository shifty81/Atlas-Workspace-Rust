#pragma once
// NF::Editor — Experiment editor v1: A/B testing and variant management
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Expv1ExperimentState : uint8_t { Draft, Running, Paused, Concluded, Archived };
enum class Expv1VariantType     : uint8_t { Control, Treatment, Holdout };

inline const char* expv1ExperimentStateName(Expv1ExperimentState s) {
    switch (s) {
        case Expv1ExperimentState::Draft:     return "Draft";
        case Expv1ExperimentState::Running:   return "Running";
        case Expv1ExperimentState::Paused:    return "Paused";
        case Expv1ExperimentState::Concluded: return "Concluded";
        case Expv1ExperimentState::Archived:  return "Archived";
    }
    return "Unknown";
}

inline const char* expv1VariantTypeName(Expv1VariantType t) {
    switch (t) {
        case Expv1VariantType::Control:   return "Control";
        case Expv1VariantType::Treatment: return "Treatment";
        case Expv1VariantType::Holdout:   return "Holdout";
    }
    return "Unknown";
}

struct Expv1Variant {
    std::string      name;
    Expv1VariantType type   = Expv1VariantType::Control;
    float            weight = 0.5f;
    bool isValid() const { return !name.empty() && weight > 0.f; }
};

struct Expv1Experiment {
    uint64_t              id    = 0;
    std::string           name;
    Expv1ExperimentState  state = Expv1ExperimentState::Draft;
    std::vector<Expv1Variant> variants;

    [[nodiscard]] bool isValid()     const { return id != 0 && !name.empty(); }
    [[nodiscard]] bool isRunning()   const { return state == Expv1ExperimentState::Running; }
    [[nodiscard]] bool isConcluded() const { return state == Expv1ExperimentState::Concluded; }
    [[nodiscard]] bool isArchived()  const { return state == Expv1ExperimentState::Archived; }

    bool addVariant(const Expv1Variant& variant) {
        if (!variant.isValid()) return false;
        for (const auto& v : variants) if (v.name == variant.name) return false;
        variants.push_back(variant);
        return true;
    }

    [[nodiscard]] size_t countByVariantType(Expv1VariantType type) const {
        size_t c = 0; for (const auto& v : variants) if (v.type == type) ++c; return c;
    }
};

using Expv1ChangeCallback = std::function<void(uint64_t)>;

class ExperimentEditorV1 {
public:
    static constexpr size_t MAX_EXPERIMENTS = 256;

    bool addExperiment(const Expv1Experiment& experiment) {
        if (!experiment.isValid()) return false;
        for (const auto& e : m_experiments) if (e.id == experiment.id) return false;
        if (m_experiments.size() >= MAX_EXPERIMENTS) return false;
        m_experiments.push_back(experiment);
        return true;
    }

    bool removeExperiment(uint64_t id) {
        for (auto it = m_experiments.begin(); it != m_experiments.end(); ++it) {
            if (it->id == id) { m_experiments.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] Expv1Experiment* findExperiment(uint64_t id) {
        for (auto& e : m_experiments) if (e.id == id) return &e;
        return nullptr;
    }

    bool addVariant(uint64_t experimentId, const Expv1Variant& variant) {
        auto* e = findExperiment(experimentId);
        return e && e->addVariant(variant);
    }

    bool setState(uint64_t id, Expv1ExperimentState state) {
        auto* e = findExperiment(id);
        if (!e) return false;
        e->state = state;
        if (m_onChange) m_onChange(id);
        return true;
    }

    [[nodiscard]] size_t experimentCount()  const { return m_experiments.size(); }
    [[nodiscard]] size_t runningCount()     const {
        size_t c = 0; for (const auto& e : m_experiments) if (e.isRunning())   ++c; return c;
    }
    [[nodiscard]] size_t concludedCount()   const {
        size_t c = 0; for (const auto& e : m_experiments) if (e.isConcluded()) ++c; return c;
    }
    [[nodiscard]] size_t countByVariantType(uint64_t experimentId, Expv1VariantType type) const {
        for (const auto& e : m_experiments) {
            if (e.id == experimentId) return e.countByVariantType(type);
        }
        return 0;
    }

    void setOnChange(Expv1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Expv1Experiment> m_experiments;
    Expv1ChangeCallback          m_onChange;
};

} // namespace NF
