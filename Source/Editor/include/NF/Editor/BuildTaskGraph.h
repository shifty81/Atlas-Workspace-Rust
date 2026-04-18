#pragma once
// NF::Editor — BuildTaskGraph: build task graph execution model.
//
// Phase G.6 — Build Tool full tool wiring.
//
// A BuildTaskGraph models the DAG of build tasks that must execute to produce
// a build output.  Each task can depend on others (edges).  The graph provides:
//   - Task registration and dependency declaration
//   - Topological order computation
//   - Per-task state tracking (Pending/Running/Succeeded/Failed/Skipped)
//   - Build log (per-task + aggregate output lines)
//   - Summary statistics (totalErrors, totalWarnings, durationMs)
//
// BuildTool owns one BuildTaskGraph and rebuilds it per build session.

#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

// ── Task identifier ────────────────────────────────────────────────────────────

using BuildTaskId = uint32_t;
static constexpr BuildTaskId kInvalidBuildTaskId = 0u;

// ── Task state ─────────────────────────────────────────────────────────────────

enum class BuildTaskState : uint8_t {
    Pending,    ///< not yet started
    Running,    ///< currently executing
    Succeeded,  ///< completed successfully
    Failed,     ///< completed with errors
    Skipped,    ///< skipped (dependency failed)
};

inline const char* buildTaskStateName(BuildTaskState s) {
    switch (s) {
    case BuildTaskState::Pending:   return "Pending";
    case BuildTaskState::Running:   return "Running";
    case BuildTaskState::Succeeded: return "Succeeded";
    case BuildTaskState::Failed:    return "Failed";
    case BuildTaskState::Skipped:   return "Skipped";
    }
    return "Unknown";
}

// ── Build output line ──────────────────────────────────────────────────────────

enum class BuildLineType : uint8_t { Info, Warning, Error };

struct BuildOutputLine {
    BuildTaskId  taskId   = kInvalidBuildTaskId;
    BuildLineType type    = BuildLineType::Info;
    std::string  message;
};

// ── BuildTask ──────────────────────────────────────────────────────────────────

struct BuildTask {
    BuildTaskId                id           = kInvalidBuildTaskId;
    std::string                name;
    BuildTaskState             state        = BuildTaskState::Pending;
    float                      durationMs   = 0.f;
    uint32_t                   errorCount   = 0;
    uint32_t                   warningCount = 0;
    std::vector<BuildTaskId>   dependencies; ///< tasks this one depends on
};

// ── Graph execution result ─────────────────────────────────────────────────────

struct BuildGraphResult {
    bool     success        = false;
    uint32_t tasksSucceeded = 0;
    uint32_t tasksFailed    = 0;
    uint32_t tasksSkipped   = 0;
    float    totalDurationMs = 0.f;
};

// ── BuildTaskGraph ─────────────────────────────────────────────────────────────

class BuildTaskGraph {
public:
    BuildTaskGraph() = default;

    // ── Task management ────────────────────────────────────────────────────────

    BuildTaskId addTask(const std::string& name) {
        BuildTaskId id = ++m_nextTaskId;
        BuildTask task;
        task.id   = id;
        task.name = name;
        m_tasks.push_back(std::move(task));
        return id;
    }

    bool depends(BuildTaskId taskId, BuildTaskId dependsOnId) {
        auto* t = findTask(taskId);
        if (!t || !findTask(dependsOnId)) return false;
        t->dependencies.push_back(dependsOnId);
        return true;
    }

    [[nodiscard]] uint32_t taskCount() const {
        return static_cast<uint32_t>(m_tasks.size());
    }

    [[nodiscard]] const BuildTask* findTask(BuildTaskId id) const {
        for (const auto& t : m_tasks) if (t.id == id) return &t;
        return nullptr;
    }

    [[nodiscard]] BuildTask* findTask(BuildTaskId id) {
        for (auto& t : m_tasks) if (t.id == id) return &t;
        return nullptr;
    }

    [[nodiscard]] const std::vector<BuildTask>& tasks() const { return m_tasks; }

    // ── Topological order ──────────────────────────────────────────────────────

    /// Returns task IDs in dependency-first order (Kahn's algorithm).
    /// Returns empty vector if a cycle is detected.
    [[nodiscard]] std::vector<BuildTaskId> topologicalOrder() const {
        // Build in-degree map
        std::vector<uint32_t> inDegree(m_nextTaskId + 1, 0);
        for (const auto& t : m_tasks) {
            for (BuildTaskId dep : t.dependencies) {
                (void)dep;
                inDegree[t.id]++;
            }
        }
        // Recompute: in-degree = number of tasks that list this task as dependency
        for (uint32_t i = 0; i <= m_nextTaskId; ++i) inDegree[i] = 0;
        for (const auto& t : m_tasks) {
            for (BuildTaskId dep : t.dependencies) {
                if (dep <= m_nextTaskId) inDegree[t.id]++;
            }
        }

        std::vector<BuildTaskId> queue;
        for (const auto& t : m_tasks) {
            if (inDegree[t.id] == 0) queue.push_back(t.id);
        }

        std::vector<BuildTaskId> order;
        while (!queue.empty()) {
            BuildTaskId cur = queue.front();
            queue.erase(queue.begin());
            order.push_back(cur);

            // Find tasks that depend on 'cur' and reduce their in-degree
            for (const auto& t : m_tasks) {
                for (BuildTaskId dep : t.dependencies) {
                    if (dep == cur) {
                        if (--inDegree[t.id] == 0) {
                            queue.push_back(t.id);
                        }
                    }
                }
            }
        }

        if (order.size() != m_tasks.size()) return {}; // cycle detected
        return order;
    }

    // ── Task state transitions ─────────────────────────────────────────────────

    bool setTaskState(BuildTaskId id, BuildTaskState state, float durationMs = 0.f) {
        auto* t = findTask(id);
        if (!t) return false;
        t->state      = state;
        t->durationMs = durationMs;
        return true;
    }

    bool setTaskCounts(BuildTaskId id, uint32_t errors, uint32_t warnings) {
        auto* t = findTask(id);
        if (!t) return false;
        t->errorCount   = errors;
        t->warningCount = warnings;
        return true;
    }

    void resetAllToState(BuildTaskState state) {
        for (auto& t : m_tasks) {
            t.state        = state;
            t.durationMs   = 0.f;
            t.errorCount   = 0;
            t.warningCount = 0;
        }
        m_outputLines.clear();
    }

    // ── Build log ──────────────────────────────────────────────────────────────

    using OutputCallback = std::function<void(const BuildOutputLine&)>;

    void setOutputCallback(OutputCallback cb) { m_outputCallback = std::move(cb); }

    void pushOutputLine(BuildTaskId taskId, BuildLineType type, const std::string& msg) {
        BuildOutputLine line;
        line.taskId  = taskId;
        line.type    = type;
        line.message = msg;
        m_outputLines.push_back(line);
        if (m_outputCallback) m_outputCallback(line);
    }

    [[nodiscard]] uint32_t outputLineCount() const {
        return static_cast<uint32_t>(m_outputLines.size());
    }

    [[nodiscard]] const std::vector<BuildOutputLine>& outputLines() const {
        return m_outputLines;
    }

    void clearOutput() { m_outputLines.clear(); }

    // ── Summary statistics ─────────────────────────────────────────────────────

    [[nodiscard]] uint32_t totalErrors() const {
        uint32_t n = 0;
        for (const auto& t : m_tasks) n += t.errorCount;
        return n;
    }

    [[nodiscard]] uint32_t totalWarnings() const {
        uint32_t n = 0;
        for (const auto& t : m_tasks) n += t.warningCount;
        return n;
    }

    [[nodiscard]] float totalDurationMs() const {
        float sum = 0.f;
        for (const auto& t : m_tasks) sum += t.durationMs;
        return sum;
    }

    [[nodiscard]] BuildGraphResult summarize() const {
        BuildGraphResult r;
        for (const auto& t : m_tasks) {
            switch (t.state) {
            case BuildTaskState::Succeeded: ++r.tasksSucceeded; break;
            case BuildTaskState::Failed:    ++r.tasksFailed;    break;
            case BuildTaskState::Skipped:   ++r.tasksSkipped;   break;
            default: break;
            }
        }
        r.success        = (r.tasksFailed == 0 && r.tasksSkipped == 0);
        r.totalDurationMs = totalDurationMs();
        return r;
    }

private:
    BuildTaskId              m_nextTaskId = 0u;
    std::vector<BuildTask>   m_tasks;
    std::vector<BuildOutputLine> m_outputLines;
    OutputCallback           m_outputCallback;
};

} // namespace NF
