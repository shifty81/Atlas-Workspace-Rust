#pragma once
// NF::Workspace — PatchApplier: formal patch apply/remove workflow.
//
// A "patch" in Atlas Workspace is a versioned, named set of file-level changes
// that can be applied to or removed from the workspace on demand. This header
// defines the data model (PatchRecord, PatchState) and the controller
// (PatchApplier) that manages the lifecycle of patches.
//
// Lifecycle: Unavailable → Available → Applying → Applied → Removing → Available
//
// Design notes:
//   - Header-only; no filesystem I/O in this layer. PatchApplier stores state
//     in memory. A higher layer (e.g. a PatchStore) persists records to disk.
//   - patch IDs are opaque strings (e.g. "v1.2.3-hotfix-inventory-crash").
//   - Each PatchRecord carries a list of PathEntry items describing the files
//     that the patch touches (informational; actual application is external).
//   - applyPatch() / removePatch() call user-supplied callbacks so callers
//     can plug in their own file-IO or dry-run logic.
//
// See Docs/Roadmap/00_MASTER_ROADMAP.md Phase 6.

#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace NF {

// ── Patch lifecycle state ─────────────────────────────────────────────────

enum class PatchState : uint8_t {
    Unavailable, // not registered in this applier
    Available,   // registered, not applied
    Applying,    // apply in progress
    Applied,     // successfully applied
    Removing,    // remove in progress
    Failed,      // last operation failed
};

inline const char* patchStateName(PatchState s) {
    switch (s) {
        case PatchState::Unavailable: return "Unavailable";
        case PatchState::Available:   return "Available";
        case PatchState::Applying:    return "Applying";
        case PatchState::Applied:     return "Applied";
        case PatchState::Removing:    return "Removing";
        case PatchState::Failed:      return "Failed";
    }
    return "Unknown";
}

// ── Per-file patch entry ──────────────────────────────────────────────────

enum class PatchFileOp : uint8_t {
    Add,     // file is created by the patch
    Modify,  // file is modified by the patch
    Remove,  // file is deleted by the patch
};

inline const char* patchFileOpName(PatchFileOp op) {
    switch (op) {
        case PatchFileOp::Add:    return "Add";
        case PatchFileOp::Modify: return "Modify";
        case PatchFileOp::Remove: return "Remove";
    }
    return "Unknown";
}

struct PatchFileEntry {
    std::string  relativePath; // workspace-relative path
    PatchFileOp  op = PatchFileOp::Modify;
    std::string  checksum;     // optional expected checksum after apply
};

// ── PatchRecord ───────────────────────────────────────────────────────────
// Describes a single patch including identity, dependencies, and file list.

struct PatchRecord {
    std::string              id;           // unique patch identifier
    std::string              displayName;  // human-readable name
    std::string              version;      // e.g. "1.2.3"
    std::string              description;  // optional long-form notes
    std::vector<std::string> dependencies; // IDs of patches that must be applied first
    std::vector<PatchFileEntry> files;     // files touched by this patch

    // State is tracked separately in PatchApplier; this field is for serialization round-trips.
    PatchState   state = PatchState::Available;
};

// ── PatchApplyCallback ────────────────────────────────────────────────────
// Called by PatchApplier::applyPatch() / removePatch().
// Return true to indicate success, false to mark the patch as Failed.

using PatchApplyCallback  = std::function<bool(const PatchRecord&)>;
using PatchRemoveCallback = std::function<bool(const PatchRecord&)>;

// ── PatchApplyResult ──────────────────────────────────────────────────────

struct PatchApplyResult {
    bool        success = false;
    std::string patchId;
    std::string errorMessage; // non-empty on failure
};

// ── PatchApplier ──────────────────────────────────────────────────────────

class PatchApplier {
public:
    // ── Registration ──────────────────────────────────────────────

    /// Register a patch. Returns false if a patch with the same ID already exists.
    bool registerPatch(PatchRecord record) {
        if (record.id.empty()) return false;
        if (m_records.count(record.id)) return false;
        auto id = record.id;
        record.state = PatchState::Available;
        m_records[id] = std::move(record);
        return true;
    }

    [[nodiscard]] bool isRegistered(const std::string& id) const {
        return m_records.count(id) > 0;
    }

    [[nodiscard]] uint32_t registeredCount() const {
        return static_cast<uint32_t>(m_records.size());
    }

    // ── State queries ─────────────────────────────────────────────

    [[nodiscard]] PatchState stateOf(const std::string& id) const {
        auto it = m_records.find(id);
        return it != m_records.end() ? it->second.state : PatchState::Unavailable;
    }

    [[nodiscard]] bool isApplied(const std::string& id) const {
        return stateOf(id) == PatchState::Applied;
    }

    [[nodiscard]] const PatchRecord* find(const std::string& id) const {
        auto it = m_records.find(id);
        return it != m_records.end() ? &it->second : nullptr;
    }

    [[nodiscard]] std::vector<std::string> appliedIds() const {
        std::vector<std::string> ids;
        for (const auto& [id, rec] : m_records)
            if (rec.state == PatchState::Applied)
                ids.push_back(id);
        return ids;
    }

    [[nodiscard]] std::vector<std::string> availableIds() const {
        std::vector<std::string> ids;
        for (const auto& [id, rec] : m_records)
            if (rec.state == PatchState::Available)
                ids.push_back(id);
        return ids;
    }

    // ── Apply ─────────────────────────────────────────────────────

    /// Apply a patch. Calls applyFn; on success, state → Applied.
    /// Dependencies must all be Applied before this patch can be applied.
    PatchApplyResult applyPatch(const std::string& id,
                                PatchApplyCallback applyFn = nullptr) {
        PatchApplyResult result;
        result.patchId = id;

        auto it = m_records.find(id);
        if (it == m_records.end()) {
            result.errorMessage = "Patch '" + id + "' is not registered.";
            return result;
        }

        auto& rec = it->second;

        if (rec.state == PatchState::Applied) {
            result.errorMessage = "Patch '" + id + "' is already applied.";
            return result;
        }

        if (rec.state == PatchState::Failed) {
            result.errorMessage = "Patch '" + id + "' is in Failed state; reset it first.";
            return result;
        }

        // Check dependencies
        for (const auto& depId : rec.dependencies) {
            if (stateOf(depId) != PatchState::Applied) {
                result.errorMessage = "Dependency '" + depId +
                    "' must be applied before patch '" + id + "'.";
                return result;
            }
        }

        rec.state = PatchState::Applying;

        bool ok = true;
        if (applyFn) ok = applyFn(rec);

        if (ok) {
            rec.state  = PatchState::Applied;
            result.success = true;
        } else {
            rec.state = PatchState::Failed;
            result.errorMessage = "Patch callback returned failure for '" + id + "'.";
        }

        return result;
    }

    // ── Remove ────────────────────────────────────────────────────

    /// Remove an applied patch. Calls removeFn; on success, state → Available.
    /// Patches that are dependencies of other applied patches cannot be removed.
    PatchApplyResult removePatch(const std::string& id,
                                 PatchRemoveCallback removeFn = nullptr) {
        PatchApplyResult result;
        result.patchId = id;

        auto it = m_records.find(id);
        if (it == m_records.end()) {
            result.errorMessage = "Patch '" + id + "' is not registered.";
            return result;
        }

        auto& rec = it->second;

        if (rec.state != PatchState::Applied) {
            result.errorMessage = "Patch '" + id + "' is not currently applied.";
            return result;
        }

        // Check that no applied patch depends on this one
        for (const auto& [otherId, otherRec] : m_records) {
            if (otherId == id) continue;
            if (otherRec.state != PatchState::Applied) continue;
            for (const auto& depId : otherRec.dependencies) {
                if (depId == id) {
                    result.errorMessage = "Cannot remove '" + id +
                        "': patch '" + otherId + "' depends on it.";
                    return result;
                }
            }
        }

        rec.state = PatchState::Removing;

        bool ok = true;
        if (removeFn) ok = removeFn(rec);

        if (ok) {
            rec.state  = PatchState::Available;
            result.success = true;
        } else {
            rec.state = PatchState::Failed;
            result.errorMessage = "Remove callback returned failure for '" + id + "'.";
        }

        return result;
    }

    /// Reset a Failed patch back to Available so it can be retried.
    bool resetPatch(const std::string& id) {
        auto it = m_records.find(id);
        if (it == m_records.end()) return false;
        if (it->second.state != PatchState::Failed) return false;
        it->second.state = PatchState::Available;
        return true;
    }

private:
    std::unordered_map<std::string, PatchRecord> m_records;
};

} // namespace NF
