#pragma once
// NF::Editor — VCS repository, version control system
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

enum class VCSProviderType : uint8_t {
    Git,
    SVN,
    Perforce,
    Mercurial,
    Plastic,
    Fossil,
    Custom,
    None
};

inline const char* vcsProviderTypeName(VCSProviderType t) noexcept {
    switch (t) {
        case VCSProviderType::Git:       return "Git";
        case VCSProviderType::SVN:       return "SVN";
        case VCSProviderType::Perforce:  return "Perforce";
        case VCSProviderType::Mercurial: return "Mercurial";
        case VCSProviderType::Plastic:   return "Plastic";
        case VCSProviderType::Fossil:    return "Fossil";
        case VCSProviderType::Custom:    return "Custom";
        case VCSProviderType::None:      return "None";
        default:                         return "Unknown";
    }
}

enum class VCSFileStatus : uint8_t {
    Untracked,
    Added,
    Modified,
    Deleted,
    Renamed,
    Conflicted,
    Ignored,
    Unchanged
};

inline const char* vcsFileStatusName(VCSFileStatus s) noexcept {
    switch (s) {
        case VCSFileStatus::Untracked:  return "Untracked";
        case VCSFileStatus::Added:      return "Added";
        case VCSFileStatus::Modified:   return "Modified";
        case VCSFileStatus::Deleted:    return "Deleted";
        case VCSFileStatus::Renamed:    return "Renamed";
        case VCSFileStatus::Conflicted: return "Conflicted";
        case VCSFileStatus::Ignored:    return "Ignored";
        case VCSFileStatus::Unchanged:  return "Unchanged";
        default:                        return "Unknown";
    }
}

struct VCSCommitInfo {
    std::string hash;
    std::string author;
    std::string message;
    double timestamp = 0.0;
    std::string parentHash;
    size_t fileCount = 0;

    [[nodiscard]] bool isValid() const { return !hash.empty() && !author.empty(); }
    [[nodiscard]] bool isRoot() const { return parentHash.empty(); }
    [[nodiscard]] bool hasMessage() const { return !message.empty(); }
};

struct VCSBranchInfo {
    std::string name;
    bool isActive = false;
    bool isRemote = false;
    std::string lastCommitHash;
    size_t aheadCount = 0;
    size_t behindCount = 0;

    [[nodiscard]] bool isSynced() const { return aheadCount == 0 && behindCount == 0; }
    [[nodiscard]] bool isLocal() const { return !isRemote; }
    [[nodiscard]] bool hasCommits() const { return !lastCommitHash.empty(); }
};

struct VCSDiffEntry {
    std::string filePath;
    VCSFileStatus status = VCSFileStatus::Unchanged;
    size_t additions = 0;
    size_t deletions = 0;
    bool isBinary = false;

    [[nodiscard]] size_t totalChanges() const { return additions + deletions; }
    [[nodiscard]] bool hasChanges() const { return additions > 0 || deletions > 0; }
};

class VCSRepository {
public:
    explicit VCSRepository(const std::string& repoName, VCSProviderType provider = VCSProviderType::Git)
        : m_name(repoName), m_provider(provider) {}

    bool addBranch(const VCSBranchInfo& branch) {
        if (m_branches.size() >= kMaxBranches) return false;
        for (const auto& b : m_branches) {
            if (b.name == branch.name) return false;
        }
        m_branches.push_back(branch);
        return true;
    }

    bool removeBranch(const std::string& branchName) {
        for (auto it = m_branches.begin(); it != m_branches.end(); ++it) {
            if (it->name == branchName && !it->isActive) {
                m_branches.erase(it);
                return true;
            }
        }
        return false;
    }

    bool switchBranch(const std::string& branchName) {
        VCSBranchInfo* target = nullptr;
        for (auto& b : m_branches) {
            if (b.name == branchName) target = &b;
        }
        if (!target) return false;
        for (auto& b : m_branches) b.isActive = false;
        target->isActive = true;
        return true;
    }

    [[nodiscard]] VCSBranchInfo* activeBranch() {
        for (auto& b : m_branches) {
            if (b.isActive) return &b;
        }
        return nullptr;
    }

    [[nodiscard]] const VCSBranchInfo* activeBranch() const {
        for (const auto& b : m_branches) {
            if (b.isActive) return &b;
        }
        return nullptr;
    }

    [[nodiscard]] VCSBranchInfo* findBranch(const std::string& name) {
        for (auto& b : m_branches) {
            if (b.name == name) return &b;
        }
        return nullptr;
    }

    bool addCommit(const VCSCommitInfo& commit) {
        if (!commit.isValid()) return false;
        if (m_commits.size() >= kMaxCommits) return false;
        m_commits.push_back(commit);
        return true;
    }

    bool trackFile(const std::string& path, VCSFileStatus status) {
        for (auto& d : m_diffs) {
            if (d.filePath == path) {
                d.status = status;
                return true;
            }
        }
        if (m_diffs.size() >= kMaxDiffs) return false;
        VCSDiffEntry entry;
        entry.filePath = path;
        entry.status = status;
        m_diffs.push_back(entry);
        return true;
    }

    [[nodiscard]] const VCSDiffEntry* findDiff(const std::string& path) const {
        for (const auto& d : m_diffs) {
            if (d.filePath == path) return &d;
        }
        return nullptr;
    }

    [[nodiscard]] size_t branchCount() const { return m_branches.size(); }
    [[nodiscard]] size_t commitCount() const { return m_commits.size(); }
    [[nodiscard]] size_t diffCount() const { return m_diffs.size(); }
    [[nodiscard]] const std::string& name() const { return m_name; }
    [[nodiscard]] VCSProviderType provider() const { return m_provider; }
    [[nodiscard]] const std::vector<VCSBranchInfo>& branches() const { return m_branches; }
    [[nodiscard]] const std::vector<VCSCommitInfo>& commits() const { return m_commits; }
    [[nodiscard]] const std::vector<VCSDiffEntry>& diffs() const { return m_diffs; }

    [[nodiscard]] size_t modifiedFileCount() const {
        size_t count = 0;
        for (const auto& d : m_diffs) {
            if (d.status != VCSFileStatus::Unchanged && d.status != VCSFileStatus::Ignored) ++count;
        }
        return count;
    }

    static constexpr size_t kMaxBranches = 64;
    static constexpr size_t kMaxCommits = 1024;
    static constexpr size_t kMaxDiffs = 512;

private:
    std::string m_name;
    VCSProviderType m_provider;
    std::vector<VCSBranchInfo> m_branches;
    std::vector<VCSCommitInfo> m_commits;
    std::vector<VCSDiffEntry> m_diffs;
};

struct VCSConfig {
    VCSProviderType defaultProvider = VCSProviderType::Git;
    bool autoDetect = true;
    bool watchFileChanges = true;
    double pollIntervalSec = 2.0;
    size_t maxRepositories = 8;
};

class VersionControlSystem {
public:
    void init(const VCSConfig& config = {}) {
        m_config = config;
        m_initialized = true;
    }

    void shutdown() {
        m_repos.clear();
        m_initialized = false;
        m_tickCount = 0;
    }

    [[nodiscard]] bool isInitialized() const { return m_initialized; }

    int openRepository(const std::string& repoName, VCSProviderType provider = VCSProviderType::Git) {
        if (!m_initialized) return -1;
        if (m_repos.size() >= m_config.maxRepositories) return -1;
        for (const auto& r : m_repos) {
            if (r.name() == repoName) return -1;
        }
        m_repos.emplace_back(repoName, provider);
        return static_cast<int>(m_repos.size()) - 1;
    }

    [[nodiscard]] VCSRepository* repository(int index) {
        if (index < 0 || index >= static_cast<int>(m_repos.size())) return nullptr;
        return &m_repos[static_cast<size_t>(index)];
    }

    [[nodiscard]] VCSRepository* repositoryByName(const std::string& name) {
        for (auto& r : m_repos) {
            if (r.name() == name) return &r;
        }
        return nullptr;
    }

    void tick(float /*dt*/) {
        if (!m_initialized) return;
        m_tickCount++;
    }

    [[nodiscard]] size_t repositoryCount() const { return m_repos.size(); }
    [[nodiscard]] size_t tickCount() const { return m_tickCount; }
    [[nodiscard]] const VCSConfig& config() const { return m_config; }

    [[nodiscard]] size_t totalBranches() const {
        size_t count = 0;
        for (const auto& r : m_repos) count += r.branchCount();
        return count;
    }

    [[nodiscard]] size_t totalCommits() const {
        size_t count = 0;
        for (const auto& r : m_repos) count += r.commitCount();
        return count;
    }

    [[nodiscard]] size_t totalModifiedFiles() const {
        size_t count = 0;
        for (const auto& r : m_repos) count += r.modifiedFileCount();
        return count;
    }

private:
    VCSConfig m_config;
    std::vector<VCSRepository> m_repos;
    bool m_initialized = false;
    size_t m_tickCount = 0;
};

// ── S13 — Localization System ───────────────────────────────────


} // namespace NF
