#pragma once
// NF::Workspace — Phase 56: Workspace Task Queue
//
// Background task management with progress tracking, cancellation, and priority.
//
//   TaskPriority    — Low / Normal / High / Critical
//   TaskState       — Pending / Running / Completed / Failed / Cancelled
//   TaskDescriptor  — id + label + priority + category + handler; isValid()
//   TaskResult      — succeeded/errorMessage/durationMs
//   TaskEntry       — descriptor + state + progress(0-100) + result; lifecycle methods
//   TaskQueue       — enqueue/cancel/tick; observers; active/pending/completed views;
//                     MAX_ENTRIES=256; priority-sorted dispatch

#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

// ═════════════════════════════════════════════════════════════════
// TaskPriority
// ═════════════════════════════════════════════════════════════════

enum class TaskPriority : uint8_t {
    Low      = 0,
    Normal   = 1,
    High     = 2,
    Critical = 3,
};

inline const char* taskPriorityName(TaskPriority p) {
    switch (p) {
        case TaskPriority::Low:      return "Low";
        case TaskPriority::Normal:   return "Normal";
        case TaskPriority::High:     return "High";
        case TaskPriority::Critical: return "Critical";
        default:                     return "Unknown";
    }
}

// ═════════════════════════════════════════════════════════════════
// TaskState
// ═════════════════════════════════════════════════════════════════

enum class TaskState : uint8_t {
    Pending    = 0,
    Running    = 1,
    Completed  = 2,
    Failed     = 3,
    Cancelled  = 4,
};

inline const char* taskStateName(TaskState s) {
    switch (s) {
        case TaskState::Pending:   return "Pending";
        case TaskState::Running:   return "Running";
        case TaskState::Completed: return "Completed";
        case TaskState::Failed:    return "Failed";
        case TaskState::Cancelled: return "Cancelled";
        default:                   return "Unknown";
    }
}

// ═════════════════════════════════════════════════════════════════
// TaskResult
// ═════════════════════════════════════════════════════════════════

struct TaskResult {
    bool        succeeded    = false;
    std::string errorMessage;
    uint64_t    durationMs   = 0;

    static TaskResult ok(uint64_t dur = 0)  { return {true, {}, dur}; }
    static TaskResult fail(const std::string& msg, uint64_t dur = 0) { return {false, msg, dur}; }
};

// ═════════════════════════════════════════════════════════════════
// TaskDescriptor
// ═════════════════════════════════════════════════════════════════

struct TaskDescriptor {
    std::string  id;
    std::string  label;
    TaskPriority priority = TaskPriority::Normal;
    std::string  category;  // optional grouping tag

    // Handler returns true on success
    using TaskHandler = std::function<bool(std::function<void(int)> setProgress)>;
    TaskHandler  handler;

    [[nodiscard]] bool isValid() const { return !id.empty() && !label.empty() && handler; }
};

// ═════════════════════════════════════════════════════════════════
// TaskEntry
// ═════════════════════════════════════════════════════════════════

class TaskEntry {
public:
    TaskDescriptor descriptor;
    TaskState      state      = TaskState::Pending;
    int            progress   = 0;  // 0-100
    TaskResult     result;
    uint64_t       enqueuedMs = 0;

    [[nodiscard]] bool isTerminal() const {
        return state == TaskState::Completed
            || state == TaskState::Failed
            || state == TaskState::Cancelled;
    }

    bool start() {
        if (state != TaskState::Pending) return false;
        state    = TaskState::Running;
        progress = 0;
        return true;
    }

    bool complete(uint64_t durationMs = 0) {
        if (state != TaskState::Running) return false;
        state    = TaskState::Completed;
        progress = 100;
        result   = TaskResult::ok(durationMs);
        return true;
    }

    bool fail(const std::string& error, uint64_t durationMs = 0) {
        if (state != TaskState::Running) return false;
        state  = TaskState::Failed;
        result = TaskResult::fail(error, durationMs);
        return true;
    }

    bool cancel() {
        if (state == TaskState::Completed || state == TaskState::Failed || state == TaskState::Cancelled)
            return false;
        state  = TaskState::Cancelled;
        result = {false, "Cancelled", 0};
        return true;
    }

    void setProgress(int pct) {
        if (state == TaskState::Running) {
            progress = std::clamp(pct, 0, 100);
        }
    }
};

// ═════════════════════════════════════════════════════════════════
// TaskQueue
// ═════════════════════════════════════════════════════════════════

class TaskQueue {
public:
    using TaskObserver = std::function<void(const TaskEntry&)>;

    static constexpr int MAX_ENTRIES   = 256;
    static constexpr int MAX_OBSERVERS = 16;
    static constexpr int MAX_CONCURRENT = 4;

    // Enqueue ──────────────────────────────────────────────────

    bool enqueue(const TaskDescriptor& desc, uint64_t nowMs = 0) {
        if (!desc.isValid()) return false;
        if (findEntry(desc.id)) return false;
        if (static_cast<int>(m_entries.size()) >= MAX_ENTRIES) return false;
        TaskEntry entry;
        entry.descriptor = desc;
        entry.state      = TaskState::Pending;
        entry.enqueuedMs = nowMs;
        m_entries.push_back(std::move(entry));
        return true;
    }

    // Cancel ───────────────────────────────────────────────────

    bool cancel(const std::string& taskId) {
        auto* entry = findEntry(taskId);
        if (!entry) return false;
        if (!entry->cancel()) return false;
        notifyObservers(*entry);
        return true;
    }

    // Tick — dispatch pending tasks up to MAX_CONCURRENT ───────

    int tick() {
        int dispatched = 0;
        int running = countByState(TaskState::Running);

        // Sort pending by priority (highest first)
        std::vector<size_t> pendingIndices;
        for (size_t i = 0; i < m_entries.size(); ++i) {
            if (m_entries[i].state == TaskState::Pending) {
                pendingIndices.push_back(i);
            }
        }
        std::sort(pendingIndices.begin(), pendingIndices.end(), [&](size_t a, size_t b) {
            return static_cast<int>(m_entries[a].descriptor.priority)
                 > static_cast<int>(m_entries[b].descriptor.priority);
        });

        for (size_t idx : pendingIndices) {
            if (running >= MAX_CONCURRENT) break;
            auto& entry = m_entries[idx];
            entry.start();
            ++running;
            ++dispatched;

            // Execute handler synchronously (in real system this would be async)
            auto progressFn = [&entry](int pct) { entry.setProgress(pct); };
            bool ok = false;
            if (entry.descriptor.handler) {
                ok = entry.descriptor.handler(progressFn);
            }
            if (ok) {
                entry.complete(0);
            } else if (entry.state == TaskState::Running) {
                entry.fail("Handler returned false", 0);
            }
            notifyObservers(entry);
        }
        return dispatched;
    }

    // Queries ──────────────────────────────────────────────────

    const TaskEntry* findEntry(const std::string& taskId) const {
        auto it = std::find_if(m_entries.begin(), m_entries.end(),
                               [&](const TaskEntry& e) { return e.descriptor.id == taskId; });
        return it != m_entries.end() ? &(*it) : nullptr;
    }

    TaskEntry* findEntry(const std::string& taskId) {
        auto it = std::find_if(m_entries.begin(), m_entries.end(),
                               [&](const TaskEntry& e) { return e.descriptor.id == taskId; });
        return it != m_entries.end() ? &(*it) : nullptr;
    }

    bool hasEntry(const std::string& taskId) const { return findEntry(taskId) != nullptr; }

    int countByState(TaskState s) const {
        return static_cast<int>(std::count_if(m_entries.begin(), m_entries.end(),
            [s](const TaskEntry& e) { return e.state == s; }));
    }

    int countByPriority(TaskPriority p) const {
        return static_cast<int>(std::count_if(m_entries.begin(), m_entries.end(),
            [p](const TaskEntry& e) { return e.descriptor.priority == p; }));
    }

    [[nodiscard]] int  totalEntries() const { return static_cast<int>(m_entries.size()); }
    [[nodiscard]] bool empty()       const { return m_entries.empty(); }

    [[nodiscard]] std::vector<const TaskEntry*> pendingTasks() const {
        return tasksWithState(TaskState::Pending);
    }

    [[nodiscard]] std::vector<const TaskEntry*> completedTasks() const {
        return tasksWithState(TaskState::Completed);
    }

    [[nodiscard]] std::vector<const TaskEntry*> failedTasks() const {
        return tasksWithState(TaskState::Failed);
    }

    // Remove all terminal entries ──────────────────────────────

    int clearCompleted() {
        int removed = 0;
        m_entries.erase(std::remove_if(m_entries.begin(), m_entries.end(),
            [&removed](const TaskEntry& e) {
                if (e.isTerminal()) { ++removed; return true; }
                return false;
            }), m_entries.end());
        return removed;
    }

    // Observers ────────────────────────────────────────────────

    bool addObserver(TaskObserver cb) {
        if (!cb) return false;
        if (static_cast<int>(m_observers.size()) >= MAX_OBSERVERS) return false;
        m_observers.push_back(std::move(cb));
        return true;
    }

    void clearObservers() { m_observers.clear(); }

    void clear() {
        m_entries.clear();
        m_observers.clear();
    }

private:
    std::vector<TaskEntry>    m_entries;
    std::vector<TaskObserver> m_observers;

    void notifyObservers(const TaskEntry& entry) {
        for (auto& cb : m_observers) {
            if (cb) cb(entry);
        }
    }

    std::vector<const TaskEntry*> tasksWithState(TaskState s) const {
        std::vector<const TaskEntry*> out;
        for (const auto& e : m_entries) {
            if (e.state == s) out.push_back(&e);
        }
        return out;
    }
};

} // namespace NF
