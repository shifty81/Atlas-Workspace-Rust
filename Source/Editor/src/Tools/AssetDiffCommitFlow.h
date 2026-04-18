#pragma once
#include "VisualDiff.h"
#include <string>
#include <vector>
#include <functional>
#include <sstream>
#include <cstdint>

namespace atlas::editor {

enum class CommitStatus {
    Idle,       ///< No pending changes.
    Staged,     ///< Changes staged, ready to commit.
    Committed,  ///< Changes committed successfully.
    Failed      ///< Commit failed (e.g., validation error).
};

struct AssetChange {
    std::string assetPath;
    DiffResult diff;
    bool staged = false;
};

/// Editor diff → commit flow for assets.
/// Tracks asset modifications, stages changes, and commits them
/// as a single atomic operation with an immutable commit message.
class AssetDiffCommitFlow {
public:
    // --- Tracking ---

    /// Record a modified asset with its before/after diff.
    void TrackChange(const std::string& assetPath, const DiffResult& diff) {
        for (auto& c : m_changes) {
            if (c.assetPath == assetPath) {
                c.diff = diff;
                return;
            }
        }
        m_changes.push_back({assetPath, diff, false});
    }

    /// Remove tracking for an asset (e.g., after reverting).
    void UntrackChange(const std::string& assetPath) {
        for (auto it = m_changes.begin(); it != m_changes.end(); ++it) {
            if (it->assetPath == assetPath) {
                m_changes.erase(it);
                return;
            }
        }
    }

    const std::vector<AssetChange>& Changes() const { return m_changes; }

    bool HasChanges() const {
        for (const auto& c : m_changes) {
            if (VisualDiff::HasChanges(c.diff)) return true;
        }
        return false;
    }

    // --- Staging ---

    /// Stage a specific asset change for commit.
    void Stage(const std::string& assetPath) {
        for (auto& c : m_changes) {
            if (c.assetPath == assetPath) {
                c.staged = true;
                return;
            }
        }
    }

    /// Unstage a specific asset change.
    void Unstage(const std::string& assetPath) {
        for (auto& c : m_changes) {
            if (c.assetPath == assetPath) {
                c.staged = false;
                return;
            }
        }
    }

    /// Stage all tracked changes.
    void StageAll() {
        for (auto& c : m_changes) {
            if (VisualDiff::HasChanges(c.diff)) c.staged = true;
        }
    }

    /// Unstage all changes.
    void UnstageAll() {
        for (auto& c : m_changes) c.staged = false;
    }

    std::vector<AssetChange> StagedChanges() const {
        std::vector<AssetChange> staged;
        for (const auto& c : m_changes) {
            if (c.staged) staged.push_back(c);
        }
        return staged;
    }

    bool HasStagedChanges() const {
        for (const auto& c : m_changes) {
            if (c.staged) return true;
        }
        return false;
    }

    // --- Commit ---

    /// Set validation callback. Called before commit to verify asset integrity.
    using ValidateCallback = std::function<bool(const std::vector<AssetChange>&)>;
    void SetValidateCallback(ValidateCallback cb) { m_validateCb = std::move(cb); }

    /// Set commit callback. Called to persist changes.
    using CommitCallback = std::function<bool(const std::vector<AssetChange>&, const std::string&)>;
    void SetCommitCallback(CommitCallback cb) { m_commitCb = std::move(cb); }

    /// Commit all staged changes with a message.
    /// Returns true if commit succeeded.
    bool Commit(const std::string& message) {
        auto staged = StagedChanges();
        if (staged.empty()) {
            m_status = CommitStatus::Failed;
            m_lastError = "No staged changes to commit";
            return false;
        }

        // Validate
        if (m_validateCb && !m_validateCb(staged)) {
            m_status = CommitStatus::Failed;
            m_lastError = "Validation failed";
            return false;
        }

        // Commit
        if (m_commitCb && !m_commitCb(staged, message)) {
            m_status = CommitStatus::Failed;
            m_lastError = "Commit callback failed";
            return false;
        }

        // Remove committed changes from tracking
        for (const auto& s : staged) {
            UntrackChange(s.assetPath);
        }

        m_status = CommitStatus::Committed;
        m_lastError.clear();
        m_commitCount++;
        return true;
    }

    // --- Status ---

    CommitStatus Status() const { return m_status; }
    const std::string& LastError() const { return m_lastError; }
    uint32_t CommitCount() const { return m_commitCount; }

    /// Clear all tracked changes and reset status.
    void Clear() {
        m_changes.clear();
        m_status = CommitStatus::Idle;
        m_lastError.clear();
    }

    std::string Summary() const {
        std::ostringstream oss;
        size_t tracked = 0, staged = 0;
        for (const auto& c : m_changes) {
            if (VisualDiff::HasChanges(c.diff)) tracked++;
            if (c.staged) staged++;
        }
        oss << "Tracked: " << tracked << ", Staged: " << staged
            << ", Commits: " << m_commitCount;
        return oss.str();
    }

private:
    std::vector<AssetChange> m_changes;
    CommitStatus m_status = CommitStatus::Idle;
    std::string m_lastError;
    uint32_t m_commitCount = 0;
    ValidateCallback m_validateCb;
    CommitCallback m_commitCb;
};

} // namespace atlas::editor
