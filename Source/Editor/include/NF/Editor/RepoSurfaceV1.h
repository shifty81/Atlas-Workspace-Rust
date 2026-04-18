#pragma once
// NF::Editor — Repo surface v1: branch listing, status, recent commits, diff/blame routing
#include "NF/Core/Core.h"
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

// ── Repo State ────────────────────────────────────────────────────

enum class RepoState : uint8_t {
    Uninitialized,
    Clean,
    Modified,
    Merging,
    Rebasing,
    Detached,
    Error,
};

inline const char* repoStateName(RepoState s) {
    switch (s) {
        case RepoState::Uninitialized: return "Uninitialized";
        case RepoState::Clean:         return "Clean";
        case RepoState::Modified:      return "Modified";
        case RepoState::Merging:       return "Merging";
        case RepoState::Rebasing:      return "Rebasing";
        case RepoState::Detached:      return "Detached";
        case RepoState::Error:         return "Error";
    }
    return "Unknown";
}

// ── File Change Status ────────────────────────────────────────────

enum class FileChangeStatus : uint8_t {
    Unmodified,
    Modified,
    Added,
    Deleted,
    Renamed,
    Copied,
    Untracked,
    Conflicted,
};

inline const char* fileChangeStatusName(FileChangeStatus s) {
    switch (s) {
        case FileChangeStatus::Unmodified: return "Unmodified";
        case FileChangeStatus::Modified:   return "Modified";
        case FileChangeStatus::Added:      return "Added";
        case FileChangeStatus::Deleted:    return "Deleted";
        case FileChangeStatus::Renamed:    return "Renamed";
        case FileChangeStatus::Copied:     return "Copied";
        case FileChangeStatus::Untracked:  return "Untracked";
        case FileChangeStatus::Conflicted: return "Conflicted";
    }
    return "Unknown";
}

// ── File Status Entry ─────────────────────────────────────────────

struct FileStatusEntry {
    std::string      path;
    std::string      renamedFrom;  // only for Renamed
    FileChangeStatus status = FileChangeStatus::Unmodified;
    bool             staged = false;

    [[nodiscard]] bool isValid()      const { return !path.empty(); }
    [[nodiscard]] bool hasConflict()  const { return status == FileChangeStatus::Conflicted; }
};

// ── Branch Info ───────────────────────────────────────────────────

struct BranchInfo {
    std::string name;
    std::string trackingBranch;  // e.g. "origin/main"
    bool        isCurrent = false;
    bool        isRemote  = false;
    int         aheadBy   = 0;
    int         behindBy  = 0;

    [[nodiscard]] bool isValid()   const { return !name.empty(); }
    [[nodiscard]] bool isSynced()  const { return aheadBy == 0 && behindBy == 0; }
};

// ── Commit Info ────────────────────────────────────────────────────

struct CommitInfo {
    std::string hash;       // short SHA
    std::string fullHash;
    std::string message;
    std::string author;
    std::string authorEmail;
    uint64_t    timestampMs = 0;
    std::vector<std::string> parentHashes;

    [[nodiscard]] bool isValid()        const { return !hash.empty() && !message.empty(); }
    [[nodiscard]] bool isMergeCommit()  const { return parentHashes.size() > 1; }
};

// ── Diff Request ──────────────────────────────────────────────────

struct DiffRequest {
    std::string filePath;
    std::string fromRef;   // commit hash or "HEAD"
    std::string toRef;     // commit hash or "" (working tree)
    bool        staged    = false;

    [[nodiscard]] bool isValid() const { return !filePath.empty(); }
};

// ── Blame Request ─────────────────────────────────────────────────

struct BlameRequest {
    std::string filePath;
    int         startLine = 1;
    int         endLine   = -1;  // -1 = to end of file
    std::string ref;             // empty = HEAD

    [[nodiscard]] bool isValid() const { return !filePath.empty(); }
};

// ── Repo Surface V1 ───────────────────────────────────────────────

using RepoDiffCallback  = std::function<void(const DiffRequest&)>;
using RepoBlameCallback = std::function<void(const BlameRequest&)>;

class RepoSurfaceV1 {
public:
    static constexpr size_t MAX_BRANCHES = 512;
    static constexpr size_t MAX_COMMITS  = 1024;

    void initialize(const std::string& repoRoot) {
        m_repoRoot   = repoRoot;
        m_state      = RepoState::Clean;
        m_initialized = true;
    }

    void setCurrentBranch(const std::string& name) {
        m_currentBranch = name;
        for (auto& b : m_branches) b.isCurrent = (b.name == name);
    }

    void setState(RepoState state) { m_state = state; }

    // Branches
    bool addBranch(const BranchInfo& branch) {
        if (!branch.isValid()) return false;
        if (m_branches.size() >= MAX_BRANCHES) return false;
        for (const auto& b : m_branches) if (b.name == branch.name) return false;
        m_branches.push_back(branch);
        return true;
    }

    bool removeBranch(const std::string& name) {
        for (auto it = m_branches.begin(); it != m_branches.end(); ++it) {
            if (it->name == name) { m_branches.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] const BranchInfo* findBranch(const std::string& name) const {
        for (const auto& b : m_branches) if (b.name == name) return &b;
        return nullptr;
    }

    [[nodiscard]] std::vector<const BranchInfo*> localBranches() const {
        std::vector<const BranchInfo*> out;
        for (const auto& b : m_branches) if (!b.isRemote) out.push_back(&b);
        return out;
    }

    [[nodiscard]] std::vector<const BranchInfo*> remoteBranches() const {
        std::vector<const BranchInfo*> out;
        for (const auto& b : m_branches) if (b.isRemote) out.push_back(&b);
        return out;
    }

    // File status
    void setFileStatus(const std::vector<FileStatusEntry>& status) {
        m_fileStatus = status;
        // Update repo state
        bool hasConflict = false;
        bool hasChanges  = false;
        for (const auto& f : m_fileStatus) {
            if (f.hasConflict()) hasConflict = true;
            if (f.status != FileChangeStatus::Unmodified) hasChanges = true;
        }
        if (m_state != RepoState::Merging && m_state != RepoState::Rebasing) {
            m_state = hasChanges ? RepoState::Modified : RepoState::Clean;
        }
    }

    bool addFileStatus(const FileStatusEntry& entry) {
        if (!entry.isValid()) return false;
        for (auto& f : m_fileStatus) {
            if (f.path == entry.path) { f = entry; return true; }  // update
        }
        m_fileStatus.push_back(entry);
        return true;
    }

    [[nodiscard]] std::vector<const FileStatusEntry*> changedFiles() const {
        std::vector<const FileStatusEntry*> out;
        for (const auto& f : m_fileStatus)
            if (f.status != FileChangeStatus::Unmodified) out.push_back(&f);
        return out;
    }

    [[nodiscard]] std::vector<const FileStatusEntry*> conflictedFiles() const {
        std::vector<const FileStatusEntry*> out;
        for (const auto& f : m_fileStatus) if (f.hasConflict()) out.push_back(&f);
        return out;
    }

    // Commit log
    bool addCommit(const CommitInfo& commit) {
        if (!commit.isValid()) return false;
        if (m_commits.size() >= MAX_COMMITS) m_commits.erase(m_commits.begin());
        for (const auto& c : m_commits) if (c.hash == commit.hash) return false;
        m_commits.push_back(commit);
        return true;
    }

    [[nodiscard]] const CommitInfo* findCommit(const std::string& hash) const {
        for (const auto& c : m_commits) if (c.hash == hash || c.fullHash == hash) return &c;
        return nullptr;
    }

    // Diff / Blame routing
    void requestDiff(const DiffRequest& req) {
        if (!req.isValid()) return;
        m_pendingDiffs.push_back(req);
        ++m_diffRequestCount;
        if (m_onDiff) m_onDiff(req);
    }

    void requestBlame(const BlameRequest& req) {
        if (!req.isValid()) return;
        m_pendingBlames.push_back(req);
        ++m_blameRequestCount;
        if (m_onBlame) m_onBlame(req);
    }

    void setOnDiff(RepoDiffCallback cb)   { m_onDiff   = std::move(cb); }
    void setOnBlame(RepoBlameCallback cb) { m_onBlame  = std::move(cb); }

    [[nodiscard]] bool        isInitialized()      const { return m_initialized;       }
    [[nodiscard]] RepoState   state()              const { return m_state;             }
    [[nodiscard]] const std::string& repoRoot()    const { return m_repoRoot;          }
    [[nodiscard]] const std::string& currentBranch() const { return m_currentBranch;  }
    [[nodiscard]] size_t      branchCount()        const { return m_branches.size();   }
    [[nodiscard]] size_t      fileStatusCount()    const { return m_fileStatus.size(); }
    [[nodiscard]] size_t      commitCount()        const { return m_commits.size();    }
    [[nodiscard]] size_t      diffRequestCount()   const { return m_diffRequestCount;  }
    [[nodiscard]] size_t      blameRequestCount()  const { return m_blameRequestCount; }

    [[nodiscard]] const std::vector<BranchInfo>&    branches()    const { return m_branches;    }
    [[nodiscard]] const std::vector<FileStatusEntry>& fileStatus() const { return m_fileStatus; }
    [[nodiscard]] const std::vector<CommitInfo>&    commits()     const { return m_commits;     }

private:
    std::string                  m_repoRoot;
    std::string                  m_currentBranch;
    std::vector<BranchInfo>      m_branches;
    std::vector<FileStatusEntry> m_fileStatus;
    std::vector<CommitInfo>      m_commits;
    std::vector<DiffRequest>     m_pendingDiffs;
    std::vector<BlameRequest>    m_pendingBlames;
    RepoDiffCallback             m_onDiff;
    RepoBlameCallback            m_onBlame;
    RepoState                    m_state            = RepoState::Uninitialized;
    size_t                       m_diffRequestCount  = 0;
    size_t                       m_blameRequestCount = 0;
    bool                         m_initialized       = false;
};

} // namespace NF
