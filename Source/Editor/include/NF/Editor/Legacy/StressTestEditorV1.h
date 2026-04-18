#pragma once
// NF::Editor — Stress test editor v1: stress test case and failure threshold management
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Sttv1CaseState    : uint8_t { Idle, Active, Passed, Failed, Skipped };
enum class Sttv1StressTarget : uint8_t { CPU, GPU, Memory, Network, Storage, Combined };

inline const char* sttv1CaseStateName(Sttv1CaseState s) {
    switch (s) {
        case Sttv1CaseState::Idle:    return "Idle";
        case Sttv1CaseState::Active:  return "Active";
        case Sttv1CaseState::Passed:  return "Passed";
        case Sttv1CaseState::Failed:  return "Failed";
        case Sttv1CaseState::Skipped: return "Skipped";
    }
    return "Unknown";
}

inline const char* sttv1StressTargetName(Sttv1StressTarget t) {
    switch (t) {
        case Sttv1StressTarget::CPU:      return "CPU";
        case Sttv1StressTarget::GPU:      return "GPU";
        case Sttv1StressTarget::Memory:   return "Memory";
        case Sttv1StressTarget::Network:  return "Network";
        case Sttv1StressTarget::Storage:  return "Storage";
        case Sttv1StressTarget::Combined: return "Combined";
    }
    return "Unknown";
}

struct Sttv1Threshold {
    uint64_t    id         = 0;
    uint64_t    caseId     = 0;
    std::string metricName;
    float       limitValue = 0.f;

    [[nodiscard]] bool isValid() const { return id != 0 && caseId != 0 && !metricName.empty(); }
};

struct Sttv1TestCase {
    uint64_t          id      = 0;
    std::string       name;
    Sttv1CaseState    state   = Sttv1CaseState::Idle;
    Sttv1StressTarget target  = Sttv1StressTarget::CPU;

    [[nodiscard]] bool isValid()   const { return id != 0 && !name.empty(); }
    [[nodiscard]] bool isPassed()  const { return state == Sttv1CaseState::Passed; }
    [[nodiscard]] bool isFailed()  const { return state == Sttv1CaseState::Failed; }
    [[nodiscard]] bool isActive()  const { return state == Sttv1CaseState::Active; }
};

using Sttv1ChangeCallback = std::function<void(uint64_t)>;

class StressTestEditorV1 {
public:
    static constexpr size_t MAX_CASES      = 4096;
    static constexpr size_t MAX_THRESHOLDS = 65536;

    bool addCase(const Sttv1TestCase& testCase) {
        if (!testCase.isValid()) return false;
        for (const auto& c : m_cases) if (c.id == testCase.id) return false;
        if (m_cases.size() >= MAX_CASES) return false;
        m_cases.push_back(testCase);
        return true;
    }

    bool removeCase(uint64_t id) {
        for (auto it = m_cases.begin(); it != m_cases.end(); ++it) {
            if (it->id == id) { m_cases.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] Sttv1TestCase* findCase(uint64_t id) {
        for (auto& c : m_cases) if (c.id == id) return &c;
        return nullptr;
    }

    bool addThreshold(const Sttv1Threshold& threshold) {
        if (!threshold.isValid()) return false;
        for (const auto& t : m_thresholds) if (t.id == threshold.id) return false;
        if (m_thresholds.size() >= MAX_THRESHOLDS) return false;
        m_thresholds.push_back(threshold);
        if (m_onChange) m_onChange(threshold.caseId);
        return true;
    }

    bool removeThreshold(uint64_t id) {
        for (auto it = m_thresholds.begin(); it != m_thresholds.end(); ++it) {
            if (it->id == id) { m_thresholds.erase(it); return true; }
        }
        return false;
    }

    bool setState(uint64_t id, Sttv1CaseState state) {
        auto* c = findCase(id);
        if (!c) return false;
        c->state = state;
        if (m_onChange) m_onChange(id);
        return true;
    }

    [[nodiscard]] size_t caseCount()      const { return m_cases.size(); }
    [[nodiscard]] size_t thresholdCount() const { return m_thresholds.size(); }

    [[nodiscard]] size_t passedCount() const {
        size_t c = 0; for (const auto& tc : m_cases) if (tc.isPassed()) ++c; return c;
    }
    [[nodiscard]] size_t failedCount() const {
        size_t c = 0; for (const auto& tc : m_cases) if (tc.isFailed()) ++c; return c;
    }
    [[nodiscard]] size_t countByTarget(Sttv1StressTarget target) const {
        size_t c = 0; for (const auto& tc : m_cases) if (tc.target == target) ++c; return c;
    }

    void setOnChange(Sttv1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Sttv1TestCase>  m_cases;
    std::vector<Sttv1Threshold> m_thresholds;
    Sttv1ChangeCallback         m_onChange;
};

} // namespace NF
