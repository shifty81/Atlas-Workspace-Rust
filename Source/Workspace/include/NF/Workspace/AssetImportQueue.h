#pragma once
// NF::Editor — Asset import queue: batch pending assets from intake pipeline
#include "NF/Workspace/FileIntakePipeline.h"
#include <cstdint>
#include <string>
#include <vector>
#include <functional>

namespace NF {

// ── Import Job Status ─────────────────────────────────────────────

enum class ImportJobStatus : uint8_t {
    Queued,
    Validating,
    Importing,
    PostProcess,
    Done,
    Failed,
    Cancelled,
};

inline const char* importJobStatusName(ImportJobStatus s) {
    switch (s) {
        case ImportJobStatus::Queued:      return "Queued";
        case ImportJobStatus::Validating:  return "Validating";
        case ImportJobStatus::Importing:   return "Importing";
        case ImportJobStatus::PostProcess: return "PostProcess";
        case ImportJobStatus::Done:        return "Done";
        case ImportJobStatus::Failed:      return "Failed";
        case ImportJobStatus::Cancelled:   return "Cancelled";
    }
    return "Unknown";
}

// ── Import Job ────────────────────────────────────────────────────

struct ImportJob {
    uint32_t      id         = 0;
    IntakeItem    intakeItem;
    ImportJobStatus status   = ImportJobStatus::Queued;
    float         progress   = 0.f;   // 0..1
    std::string   errorMsg;
    std::string   outputPath;         // resulting asset path after import
    int           priority   = 0;     // higher = processed first

    [[nodiscard]] bool isDone()     const { return status == ImportJobStatus::Done;      }
    [[nodiscard]] bool isFailed()   const { return status == ImportJobStatus::Failed;    }
    [[nodiscard]] bool isActive()   const {
        return status == ImportJobStatus::Validating
            || status == ImportJobStatus::Importing
            || status == ImportJobStatus::PostProcess;
    }
    [[nodiscard]] bool isFinished() const { return isDone() || isFailed() || status == ImportJobStatus::Cancelled; }
};

// ── Asset Import Queue ────────────────────────────────────────────

using ImportCallback = std::function<void(const ImportJob&)>;

class AssetImportQueue {
public:
    static constexpr size_t MAX_JOBS     = 256;
    static constexpr size_t MAX_PARALLEL = 4;

    bool enqueue(IntakeItem item, int priority = 0) {
        if (m_jobs.size() >= MAX_JOBS) return false;

        ImportJob job;
        job.id         = ++m_nextId;
        job.intakeItem = std::move(item);
        job.status     = ImportJobStatus::Queued;
        job.priority   = priority;

        m_jobs.push_back(std::move(job));
        sortJobs();
        ++m_totalEnqueued;
        return true;
    }

    // Enqueue all pending items from the intake pipeline
    size_t enqueueFromPipeline(FileIntakePipeline& pipeline, int defaultPriority = 0) {
        size_t count = 0;
        for (const auto& item : pipeline.pendingItems()) {
            if (enqueue(item, defaultPriority)) ++count;
        }
        return count;
    }

    bool cancel(uint32_t jobId) {
        for (auto& j : m_jobs) {
            if (j.id == jobId && !j.isFinished()) {
                j.status = ImportJobStatus::Cancelled;
                ++m_totalCancelled;
                return true;
            }
        }
        return false;
    }

    // Advance one queued job to Validating (simulates a tick)
    bool startNext() {
        for (auto& j : m_jobs) {
            if (j.status == ImportJobStatus::Queued) {
                j.status = ImportJobStatus::Validating;
                return true;
            }
        }
        return false;
    }

    // Advance a job from Validating→Importing→PostProcess→Done
    bool advance(uint32_t jobId, float progressDelta = 0.5f) {
        for (auto& j : m_jobs) {
            if (j.id != jobId) continue;
            switch (j.status) {
                case ImportJobStatus::Queued:
                    j.status = ImportJobStatus::Validating;
                    return true;
                case ImportJobStatus::Validating:
                    j.status = ImportJobStatus::Importing;
                    j.progress = 0.f;
                    return true;
                case ImportJobStatus::Importing:
                    j.progress = std::min(1.f, j.progress + progressDelta);
                    if (j.progress >= 1.f) j.status = ImportJobStatus::PostProcess;
                    return true;
                case ImportJobStatus::PostProcess:
                    j.status = ImportJobStatus::Done;
                    j.progress = 1.f;
                    j.outputPath = "Assets/Imported/" + j.intakeItem.filename();
                    ++m_totalCompleted;
                    if (m_onComplete) m_onComplete(j);
                    return true;
                default: return false;
            }
        }
        return false;
    }

    bool failJob(uint32_t jobId, const std::string& reason) {
        for (auto& j : m_jobs) {
            if (j.id == jobId && !j.isFinished()) {
                j.status   = ImportJobStatus::Failed;
                j.errorMsg = reason;
                ++m_totalFailed;
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] const ImportJob* find(uint32_t id) const {
        for (const auto& j : m_jobs) if (j.id == id) return &j;
        return nullptr;
    }

    [[nodiscard]] size_t queuedCount() const {
        size_t n = 0;
        for (const auto& j : m_jobs) if (j.status == ImportJobStatus::Queued) ++n;
        return n;
    }

    [[nodiscard]] size_t activeCount() const {
        size_t n = 0;
        for (const auto& j : m_jobs) if (j.isActive()) ++n;
        return n;
    }

    [[nodiscard]] size_t totalCount()     const { return m_jobs.size();      }
    [[nodiscard]] size_t totalEnqueued()  const { return m_totalEnqueued;    }
    [[nodiscard]] size_t totalCompleted() const { return m_totalCompleted;   }
    [[nodiscard]] size_t totalFailed()    const { return m_totalFailed;      }
    [[nodiscard]] size_t totalCancelled() const { return m_totalCancelled;   }

    [[nodiscard]] const std::vector<ImportJob>& jobs() const { return m_jobs; }

    void setOnComplete(ImportCallback cb) { m_onComplete = std::move(cb); }

    void clearFinished() {
        m_jobs.erase(std::remove_if(m_jobs.begin(), m_jobs.end(),
            [](const ImportJob& j) { return j.isFinished(); }), m_jobs.end());
    }

private:
    void sortJobs() {
        std::stable_sort(m_jobs.begin(), m_jobs.end(),
            [](const ImportJob& a, const ImportJob& b) {
                return a.priority > b.priority;
            });
    }

    std::vector<ImportJob> m_jobs;
    ImportCallback m_onComplete;
    uint32_t m_nextId          = 0;
    size_t   m_totalEnqueued   = 0;
    size_t   m_totalCompleted  = 0;
    size_t   m_totalFailed     = 0;
    size_t   m_totalCancelled  = 0;
};

} // namespace NF
