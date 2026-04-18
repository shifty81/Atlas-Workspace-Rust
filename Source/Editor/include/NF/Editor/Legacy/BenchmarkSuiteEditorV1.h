#pragma once
// NF::Editor — Benchmark suite editor v1: benchmark suite and run management
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Bchv1RunState          : uint8_t { Pending, Running, Complete, Failed, Skipped };
enum class Bchv1BenchmarkCategory : uint8_t { CPU, GPU, Memory, IO, Network, Physics };

inline const char* bchv1RunStateName(Bchv1RunState s) {
    switch (s) {
        case Bchv1RunState::Pending:  return "Pending";
        case Bchv1RunState::Running:  return "Running";
        case Bchv1RunState::Complete: return "Complete";
        case Bchv1RunState::Failed:   return "Failed";
        case Bchv1RunState::Skipped:  return "Skipped";
    }
    return "Unknown";
}

inline const char* bchv1BenchmarkCategoryName(Bchv1BenchmarkCategory c) {
    switch (c) {
        case Bchv1BenchmarkCategory::CPU:     return "CPU";
        case Bchv1BenchmarkCategory::GPU:     return "GPU";
        case Bchv1BenchmarkCategory::Memory:  return "Memory";
        case Bchv1BenchmarkCategory::IO:      return "IO";
        case Bchv1BenchmarkCategory::Network: return "Network";
        case Bchv1BenchmarkCategory::Physics: return "Physics";
    }
    return "Unknown";
}

struct Bchv1Run {
    uint64_t              id      = 0;
    uint64_t              suiteId = 0;
    std::string           name;
    Bchv1RunState         state   = Bchv1RunState::Pending;
    Bchv1BenchmarkCategory category = Bchv1BenchmarkCategory::CPU;

    [[nodiscard]] bool isValid()    const { return id != 0 && suiteId != 0 && !name.empty(); }
    [[nodiscard]] bool isComplete() const { return state == Bchv1RunState::Complete; }
    [[nodiscard]] bool isFailed()   const { return state == Bchv1RunState::Failed; }
};

struct Bchv1Suite {
    uint64_t    id   = 0;
    std::string name;

    [[nodiscard]] bool isValid() const { return id != 0 && !name.empty(); }
};

using Bchv1ChangeCallback = std::function<void(uint64_t)>;

class BenchmarkSuiteEditorV1 {
public:
    static constexpr size_t MAX_SUITES = 256;
    static constexpr size_t MAX_RUNS   = 16384;

    bool addSuite(const Bchv1Suite& suite) {
        if (!suite.isValid()) return false;
        for (const auto& s : m_suites) if (s.id == suite.id) return false;
        if (m_suites.size() >= MAX_SUITES) return false;
        m_suites.push_back(suite);
        return true;
    }

    bool removeSuite(uint64_t id) {
        for (auto it = m_suites.begin(); it != m_suites.end(); ++it) {
            if (it->id == id) { m_suites.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] Bchv1Suite* findSuite(uint64_t id) {
        for (auto& s : m_suites) if (s.id == id) return &s;
        return nullptr;
    }

    bool addRun(const Bchv1Run& run) {
        if (!run.isValid()) return false;
        for (const auto& r : m_runs) if (r.id == run.id) return false;
        if (m_runs.size() >= MAX_RUNS) return false;
        m_runs.push_back(run);
        if (m_onChange) m_onChange(run.suiteId);
        return true;
    }

    bool removeRun(uint64_t id) {
        for (auto it = m_runs.begin(); it != m_runs.end(); ++it) {
            if (it->id == id) { m_runs.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] Bchv1Run* findRun(uint64_t id) {
        for (auto& r : m_runs) if (r.id == id) return &r;
        return nullptr;
    }

    bool setRunState(uint64_t runId, Bchv1RunState state) {
        auto* r = findRun(runId);
        if (!r) return false;
        r->state = state;
        if (m_onChange) m_onChange(r->suiteId);
        return true;
    }

    [[nodiscard]] size_t suiteCount() const { return m_suites.size(); }
    [[nodiscard]] size_t runCount()   const { return m_runs.size(); }

    [[nodiscard]] size_t completeCount() const {
        size_t c = 0; for (const auto& r : m_runs) if (r.isComplete()) ++c; return c;
    }
    [[nodiscard]] size_t failedCount() const {
        size_t c = 0; for (const auto& r : m_runs) if (r.isFailed()) ++c; return c;
    }
    [[nodiscard]] size_t countByCategory(Bchv1BenchmarkCategory category) const {
        size_t c = 0; for (const auto& r : m_runs) if (r.category == category) ++c; return c;
    }

    void setOnChange(Bchv1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Bchv1Suite> m_suites;
    std::vector<Bchv1Run>   m_runs;
    Bchv1ChangeCallback     m_onChange;
};

} // namespace NF
