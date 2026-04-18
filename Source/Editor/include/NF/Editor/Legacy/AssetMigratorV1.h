#pragma once
// NF::Editor — Asset migrator v1: version-aware asset migration job authoring
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Amv1MigrateStatus : uint8_t { Pending, Running, Done, Skipped, Failed, Rollback };
enum class Amv1ConflictRule  : uint8_t { Overwrite, Skip, Rename, Abort };
enum class Amv1MigrateScope  : uint8_t { Asset, Folder, Project, Selection };

inline const char* amv1MigrateStatusName(Amv1MigrateStatus s) {
    switch (s) {
        case Amv1MigrateStatus::Pending:  return "Pending";
        case Amv1MigrateStatus::Running:  return "Running";
        case Amv1MigrateStatus::Done:     return "Done";
        case Amv1MigrateStatus::Skipped:  return "Skipped";
        case Amv1MigrateStatus::Failed:   return "Failed";
        case Amv1MigrateStatus::Rollback: return "Rollback";
    }
    return "Unknown";
}

inline const char* amv1ConflictRuleName(Amv1ConflictRule r) {
    switch (r) {
        case Amv1ConflictRule::Overwrite: return "Overwrite";
        case Amv1ConflictRule::Skip:      return "Skip";
        case Amv1ConflictRule::Rename:    return "Rename";
        case Amv1ConflictRule::Abort:     return "Abort";
    }
    return "Unknown";
}

inline const char* amv1MigrateScopeName(Amv1MigrateScope s) {
    switch (s) {
        case Amv1MigrateScope::Asset:     return "Asset";
        case Amv1MigrateScope::Folder:    return "Folder";
        case Amv1MigrateScope::Project:   return "Project";
        case Amv1MigrateScope::Selection: return "Selection";
    }
    return "Unknown";
}

struct Amv1MigrateJob {
    uint64_t           id            = 0;
    std::string        name;
    std::string        fromVersion;
    std::string        toVersion;
    Amv1MigrateStatus  status        = Amv1MigrateStatus::Pending;
    Amv1ConflictRule   conflictRule  = Amv1ConflictRule::Overwrite;
    Amv1MigrateScope   scope         = Amv1MigrateScope::Project;
    float              progress      = 0.f;
    uint32_t           assetsTotal   = 0;
    uint32_t           assetsDone    = 0;
    uint32_t           assetsFailed  = 0;

    [[nodiscard]] bool isValid()   const { return id != 0 && !name.empty() && !fromVersion.empty() && !toVersion.empty(); }
    [[nodiscard]] bool isDone()    const { return status == Amv1MigrateStatus::Done; }
    [[nodiscard]] bool isRunning() const { return status == Amv1MigrateStatus::Running; }
    [[nodiscard]] bool hasFailed() const { return status == Amv1MigrateStatus::Failed; }
};

using Amv1ChangeCallback = std::function<void(uint64_t)>;

class AssetMigratorV1 {
public:
    static constexpr size_t MAX_JOBS = 256;

    bool addJob(const Amv1MigrateJob& job) {
        if (!job.isValid()) return false;
        for (const auto& j : m_jobs) if (j.id == job.id) return false;
        if (m_jobs.size() >= MAX_JOBS) return false;
        m_jobs.push_back(job);
        return true;
    }

    bool removeJob(uint64_t id) {
        for (auto it = m_jobs.begin(); it != m_jobs.end(); ++it) {
            if (it->id == id) { m_jobs.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] Amv1MigrateJob* findJob(uint64_t id) {
        for (auto& j : m_jobs) if (j.id == id) return &j;
        return nullptr;
    }

    bool updateProgress(uint64_t id, float progress, uint32_t done, uint32_t failed = 0) {
        auto* j = findJob(id);
        if (!j) return false;
        j->progress     = progress;
        j->assetsDone   = done;
        j->assetsFailed = failed;
        if (m_onChange) m_onChange(id);
        return true;
    }

    bool setStatus(uint64_t id, Amv1MigrateStatus status) {
        auto* j = findJob(id);
        if (!j) return false;
        j->status = status;
        if (m_onChange) m_onChange(id);
        return true;
    }

    bool setConflictRule(uint64_t id, Amv1ConflictRule rule) {
        auto* j = findJob(id);
        if (!j) return false;
        j->conflictRule = rule;
        if (m_onChange) m_onChange(id);
        return true;
    }

    [[nodiscard]] size_t jobCount()     const { return m_jobs.size(); }
    [[nodiscard]] size_t doneCount()    const {
        size_t c = 0; for (const auto& j : m_jobs) if (j.isDone())    ++c; return c;
    }
    [[nodiscard]] size_t runningCount() const {
        size_t c = 0; for (const auto& j : m_jobs) if (j.isRunning()) ++c; return c;
    }
    [[nodiscard]] size_t failedCount()  const {
        size_t c = 0; for (const auto& j : m_jobs) if (j.hasFailed()) ++c; return c;
    }
    [[nodiscard]] size_t countByScope(Amv1MigrateScope scope) const {
        size_t c = 0; for (const auto& j : m_jobs) if (j.scope == scope) ++c; return c;
    }

    void setOnChange(Amv1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Amv1MigrateJob> m_jobs;
    Amv1ChangeCallback          m_onChange;
};

} // namespace NF
