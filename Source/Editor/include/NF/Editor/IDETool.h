#pragma once
// NF::Editor — IDETool: integrated script/code editor tool.
//
// Implements NF::IHostedTool for the IDE, one of the primary tools in
// the canonical workspace roster.
//
// The IDE Tool hosts:
//   Shared panels: file_tree, code_editor, find_references, console
//   Commands:      ide.open_file, ide.save_file, ide.save_all,
//                  ide.go_to_definition, ide.find_in_files,
//                  ide.toggle_comment, ide.format_document,
//                  ide.run_script, ide.new_script
//
// Layout (renderToolView):
//   File Tree (20%) | Editor Area (60%) | Output / Symbol Panel (20%)
//
// See Docs/Canon/05_EDITOR_STRATEGY.md for the locked tool roster.

#include "NF/Workspace/IHostedTool.h"
#include "NF/Workspace/ToolViewRenderContext.h"
#include <map>
#include <string>
#include <vector>

namespace NF {

// ── IDE edit mode ─────────────────────────────────────────────────

enum class IDEEditMode : uint8_t {
    Code,       // normal code editing
    Diff,       // side-by-side diff view
    ReadOnly,   // read-only inspection (e.g. compiled output)
};

inline const char* ideEditModeName(IDEEditMode m) {
    switch (m) {
        case IDEEditMode::Code:     return "Code";
        case IDEEditMode::Diff:     return "Diff";
        case IDEEditMode::ReadOnly: return "Read-Only";
    }
    return "Unknown";
}

// ── IDE Tool statistics ───────────────────────────────────────────

struct IDEToolStats {
    uint32_t openFileCount    = 0; // number of open editor tabs
    uint32_t indexedFileCount = 0; // files indexed by the project indexer
    bool     isDirty          = false; // unsaved changes in any open file
    bool     isRunning        = false; // script/process is running
};

// ── IDETool ───────────────────────────────────────────────────────

class IDETool final : public IHostedTool {
public:
    static constexpr const char* kToolId = "workspace.ide";

    IDETool();
    ~IDETool() override = default;

    // ── IHostedTool identity ──────────────────────────────────────
    [[nodiscard]] const HostedToolDescriptor& descriptor() const override { return m_descriptor; }
    [[nodiscard]] const std::string& toolId()              const override { return m_descriptor.toolId; }

    // ── IHostedTool lifecycle ─────────────────────────────────────
    bool initialize() override;
    void shutdown()   override;
    void activate()   override;
    void suspend()    override;
    void update(float dt) override;

    [[nodiscard]] HostedToolState state() const override { return m_state; }

    // ── Project adapter hooks ─────────────────────────────────────
    void onProjectLoaded(const std::string& projectId)  override;
    void onProjectUnloaded()                             override;

    // ── Render ───────────────────────────────────────────────────
    void renderToolView(const ToolViewRenderContext& ctx) const override;
    void syncPanels(WorkspacePanelHost& host) const override;

    // ── IDE Tool interface ────────────────────────────────────────
    [[nodiscard]] IDEEditMode         editMode()   const { return m_editMode;        }
    [[nodiscard]] const IDEToolStats& stats()      const { return m_stats;           }
    [[nodiscard]] const std::string&  activeFile() const { return m_activeFile;      }

    void setEditMode(IDEEditMode mode);
    void openFile(const std::string& path);
    void closeFile(const std::string& path);

    void markDirty();
    void clearDirty();

private:
    void buildDescriptor();

    HostedToolDescriptor m_descriptor;
    HostedToolState      m_state       = HostedToolState::Unloaded;
    mutable IDEEditMode  m_editMode    = IDEEditMode::Code;
    IDEToolStats         m_stats;

    std::string              m_activeFile;
    std::vector<std::string> m_openFiles;
    std::string              m_activeProjectId;

    // Loaded file content cache: path → vector of lines
    std::map<std::string, std::vector<std::string>> m_fileContents;

    // ── Mutable per-view UI state (safe from const renderToolView) ─
    mutable int m_viewSelectedFile = -1;
};

} // namespace NF
