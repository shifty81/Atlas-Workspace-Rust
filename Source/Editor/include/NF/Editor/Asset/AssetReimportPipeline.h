#pragma once
// NF::Editor — AssetReimportPipeline: import/reimport pipeline for the Asset Editor.
//
// Queues reimport tasks for one or more assets, tracks per-asset import state,
// and reports import results.  The pipeline is a stub implementation backed by
// in-memory state; the real asset import execution is gated behind the platform
// asset transformer layer (AssetTransformer.h).
//
// Phase G.2 — Asset Editor: import/reimport pipeline integration
//
//   enqueue(guid, sourcePath)  — queue an asset for reimport
//   processNext()              — run the next queued import (stub: always succeeds)
//   processAll()               — drain the queue
//   statusOf(guid)             — per-asset import status
//   results()                  — completed import results

#include "NF/Core/Core.h"
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

// ── ImportJobStatus ───────────────────────────────────────────────────────────

enum class ImportJobStatus : uint8_t {
    Queued,     ///< waiting to be processed
    Processing, ///< currently importing
    Succeeded,  ///< import completed without errors
    Failed,     ///< import encountered an error
    Skipped,    ///< source path was unchanged; skipped
};

inline const char* importJobStatusName(ImportJobStatus s) {
    switch (s) {
    case ImportJobStatus::Queued:     return "Queued";
    case ImportJobStatus::Processing: return "Processing";
    case ImportJobStatus::Succeeded:  return "Succeeded";
    case ImportJobStatus::Failed:     return "Failed";
    case ImportJobStatus::Skipped:    return "Skipped";
    }
    return "Unknown";
}

// ── ImportJobResult ───────────────────────────────────────────────────────────

struct ImportJobResult {
    std::string    guid;
    std::string    sourcePath;
    ImportJobStatus status    = ImportJobStatus::Queued;
    std::string    errorMsg;
    uint32_t       jobId      = 0;

    [[nodiscard]] bool succeeded() const { return status == ImportJobStatus::Succeeded; }
    [[nodiscard]] bool failed()    const { return status == ImportJobStatus::Failed; }
};

// ── AssetReimportPipeline ─────────────────────────────────────────────────────

class AssetReimportPipeline {
public:
    using ResultCallback = std::function<void(const ImportJobResult&)>;

    AssetReimportPipeline() = default;

    // ── Queue management ──────────────────────────────────────────────────

    /// Queue an asset for reimport.  Returns the assigned job ID.
    /// If the asset is already queued or processing, this is a no-op and
    /// returns 0.
    uint32_t enqueue(const std::string& guid, const std::string& sourcePath) {
        if (guid.empty() || sourcePath.empty()) return 0;
        if (isQueued(guid)) return 0;

        ++m_nextJobId;
        ImportJobResult job;
        job.guid       = guid;
        job.sourcePath = sourcePath;
        job.status     = ImportJobStatus::Queued;
        job.jobId      = m_nextJobId;
        m_queue.push_back(std::move(job));
        return m_nextJobId;
    }

    /// Cancel a queued job.  Returns true if the job was found and removed.
    bool cancel(const std::string& guid) {
        for (auto it = m_queue.begin(); it != m_queue.end(); ++it) {
            if (it->guid == guid && it->status == ImportJobStatus::Queued) {
                m_queue.erase(it);
                return true;
            }
        }
        return false;
    }

    // ── Processing ────────────────────────────────────────────────────────

    /// Process the next queued job (stub: always succeeds unless sourcePath
    /// is empty).  Returns false if the queue is empty.
    bool processNext() {
        ImportJobResult* job = nextQueued();
        if (!job) return false;

        job->status = ImportJobStatus::Processing;

        // Stub implementation — real work is delegated to AssetTransformer.
        if (job->sourcePath.empty()) {
            job->status   = ImportJobStatus::Failed;
            job->errorMsg = "Source path is empty";
        } else {
            job->status = ImportJobStatus::Succeeded;
        }

        ++m_processedCount;
        if (job->status == ImportJobStatus::Succeeded) ++m_succeededCount;
        else                                            ++m_failedCount;

        m_results.push_back(*job);
        if (m_onResult) m_onResult(*job);

        // Remove from queue
        for (auto it = m_queue.begin(); it != m_queue.end(); ++it) {
            if (it->jobId == job->jobId) { m_queue.erase(it); break; }
        }

        return true;
    }

    /// Process all queued jobs.  Returns the number of jobs processed.
    uint32_t processAll() {
        uint32_t count = 0;
        while (processNext()) ++count;
        return count;
    }

    // ── Status query ──────────────────────────────────────────────────────

    [[nodiscard]] bool isQueued(const std::string& guid) const {
        for (const auto& j : m_queue)
            if (j.guid == guid) return true;
        return false;
    }

    [[nodiscard]] ImportJobStatus statusOf(const std::string& guid) const {
        // Check active queue first
        for (const auto& j : m_queue)
            if (j.guid == guid) return j.status;
        // Then completed results
        for (auto it = m_results.rbegin(); it != m_results.rend(); ++it)
            if (it->guid == guid) return it->status;
        return ImportJobStatus::Skipped;
    }

    // ── Results ───────────────────────────────────────────────────────────

    [[nodiscard]] const std::vector<ImportJobResult>& results() const { return m_results; }

    [[nodiscard]] uint32_t queuedCount()    const { return static_cast<uint32_t>(m_queue.size()); }
    [[nodiscard]] uint32_t processedCount() const { return m_processedCount; }
    [[nodiscard]] uint32_t succeededCount() const { return m_succeededCount; }
    [[nodiscard]] uint32_t failedCount()    const { return m_failedCount; }

    void clearResults() { m_results.clear(); }

    // ── Observer ──────────────────────────────────────────────────────────

    void setOnResult(ResultCallback cb) { m_onResult = std::move(cb); }

private:
    std::vector<ImportJobResult> m_queue;
    std::vector<ImportJobResult> m_results;
    uint32_t m_nextJobId      = 0;
    uint32_t m_processedCount = 0;
    uint32_t m_succeededCount = 0;
    uint32_t m_failedCount    = 0;
    ResultCallback m_onResult;

    ImportJobResult* nextQueued() {
        for (auto& j : m_queue)
            if (j.status == ImportJobStatus::Queued) return &j;
        return nullptr;
    }
};

} // namespace NF
