#pragma once
// NF::Workspace — ProjectSerializer: snapshot and restore WorkspaceShell state.
//
// A ProjectSerializer converts a WorkspaceShellSnapshot (lightweight, copyable
// description of the shell state at a point in time) into/from a
// WorkspaceProjectFile.
//
// Design:
//   WorkspaceShellSnapshot — flat, copyable summary of shell state suitable for
//                            serialization (no raw pointers or runtime state).
//   ProjectSerializer      — static serialize/deserialize helpers that read/write
//                            the snapshot to/from a WorkspaceProjectFile.
//
// The serializer writes into the "Core" and "Tools" sections of the project file.
// It makes no filesystem calls — callers handle I/O.

#include "NF/Workspace/WorkspaceProjectFile.h"
#include <cstdint>
#include <string>
#include <vector>

namespace NF {

// ── Workspace Shell Snapshot ───────────────────────────────────────
// A flat, copyable summary of workspace shell state.

struct WorkspaceShellSnapshot {
    // Project identity
    std::string projectId;
    std::string projectName;
    std::string contentRoot;

    // Active tool
    std::string activeToolId;

    // Registered tool ids (in registration order)
    std::vector<std::string> registeredToolIds;

    // Visible shared panel ids
    std::vector<std::string> visiblePanelIds;

    // Schema version used when this snapshot was created
    ProjectFileVersion fileVersion = ProjectFileVersion::current();

    [[nodiscard]] bool isValid() const {
        return !projectId.empty() && !projectName.empty();
    }
};

// ── Serialization / Deserialization Result ─────────────────────────

struct SerializeResult {
    bool        succeeded  = false;
    std::string errorMessage;

    [[nodiscard]] bool failed() const { return !succeeded; }

    static SerializeResult ok()                          { return {true,  {}}; }
    static SerializeResult fail(const std::string& msg) { return {false, msg}; }
};

// ── Project Serializer ─────────────────────────────────────────────

class ProjectSerializer {
public:
    static constexpr const char* CORE_SECTION    = "Core";
    static constexpr const char* TOOLS_SECTION   = "Tools";
    static constexpr const char* PANELS_SECTION  = "Panels";

    // Serialize snapshot → project file sections.
    // Populates Core, Tools, and Panels sections.
    static SerializeResult serialize(const WorkspaceShellSnapshot& snap,
                                     WorkspaceProjectFile& file) {
        if (!snap.isValid()) {
            return SerializeResult::fail("snapshot is not valid (empty projectId or projectName)");
        }

        file.setProjectId(snap.projectId);
        file.setProjectName(snap.projectName);
        file.setContentRoot(snap.contentRoot);
        file.setVersion(snap.fileVersion);

        auto& core = file.section(CORE_SECTION);
        core.set("projectId",   snap.projectId);
        core.set("projectName", snap.projectName);
        core.set("contentRoot", snap.contentRoot);
        core.set("activeTool",  snap.activeToolId);

        auto& tools = file.section(TOOLS_SECTION);
        tools.set("count", std::to_string(snap.registeredToolIds.size()));
        for (size_t i = 0; i < snap.registeredToolIds.size(); ++i) {
            tools.set("tool." + std::to_string(i), snap.registeredToolIds[i]);
        }

        auto& panels = file.section(PANELS_SECTION);
        panels.set("count", std::to_string(snap.visiblePanelIds.size()));
        for (size_t i = 0; i < snap.visiblePanelIds.size(); ++i) {
            panels.set("panel." + std::to_string(i), snap.visiblePanelIds[i]);
        }

        return SerializeResult::ok();
    }

    // Deserialize project file → snapshot.
    static SerializeResult deserialize(const WorkspaceProjectFile& file,
                                       WorkspaceShellSnapshot& snap) {
        if (!file.isValid()) {
            return SerializeResult::fail("project file is not valid");
        }

        snap = {};
        snap.projectId   = file.projectId();
        snap.projectName = file.projectName();
        snap.contentRoot = file.contentRoot();
        snap.fileVersion = file.version();

        const auto* core = file.findSection(CORE_SECTION);
        if (core) {
            snap.activeToolId = core->getOr("activeTool", {});
        }

        const auto* tools = file.findSection(TOOLS_SECTION);
        if (tools) {
            size_t count = 0;
            const auto* countStr = tools->get("count");
            if (countStr) count = static_cast<size_t>(std::stoul(*countStr));
            for (size_t i = 0; i < count; ++i) {
                const auto* val = tools->get("tool." + std::to_string(i));
                if (val) snap.registeredToolIds.push_back(*val);
            }
        }

        const auto* panels = file.findSection(PANELS_SECTION);
        if (panels) {
            size_t count = 0;
            const auto* countStr = panels->get("count");
            if (countStr) count = static_cast<size_t>(std::stoul(*countStr));
            for (size_t i = 0; i < count; ++i) {
                const auto* val = panels->get("panel." + std::to_string(i));
                if (val) snap.visiblePanelIds.push_back(*val);
            }
        }

        return SerializeResult::ok();
    }

    // Round-trip: serialize snap → text → parse → deserialize → out.
    // Returns false if either step fails.
    static SerializeResult roundTrip(const WorkspaceShellSnapshot& snap,
                                     WorkspaceShellSnapshot& out) {
        WorkspaceProjectFile file;
        auto res = serialize(snap, file);
        if (res.failed()) return res;

        std::string text = file.serialize();

        WorkspaceProjectFile parsed;
        if (!WorkspaceProjectFile::parse(text, parsed)) {
            return SerializeResult::fail("failed to parse serialized project file");
        }

        return deserialize(parsed, out);
    }
};

} // namespace NF
