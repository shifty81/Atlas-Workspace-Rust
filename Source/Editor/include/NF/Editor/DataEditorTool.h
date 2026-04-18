#pragma once
// NF::Editor — DataEditorTool: primary data table and config editing tool.
//
// Implements NF::IHostedTool for the Data Editor, one of the ~10
// primary tools in the canonical workspace roster.
//
// The Data Editor hosts:
//   Shared panels: data_table, inspector, console
//   Commands:      data.new_row, data.delete_row, data.duplicate_row,
//                  data.import_csv, data.export_csv, data.save
//
// See Docs/Canon/05_EDITOR_STRATEGY.md for the locked tool roster.
// See Docs/Roadmap/04_EDITOR_CONSOLIDATION.md for Phase 3 status.

#include "NF/Workspace/IHostedTool.h"
#include "NF/Workspace/ToolViewRenderContext.h"
#include <string>

namespace NF {

// ── Data editing mode ─────────────────────────────────────────────

enum class DataEditMode : uint8_t {
    Table,  // spreadsheet row/column view
    Json,   // raw JSON source view
    Diff,   // side-by-side diff vs. last save
};

inline const char* dataEditModeName(DataEditMode m) {
    switch (m) {
        case DataEditMode::Table: return "Table";
        case DataEditMode::Json:  return "Json";
        case DataEditMode::Diff:  return "Diff";
    }
    return "Unknown";
}

// ── Data Editor statistics ────────────────────────────────────────

struct DataEditorStats {
    uint32_t rowCount         = 0; // rows in active data table
    uint32_t selectedRowCount = 0; // selected rows
    uint32_t columnCount      = 0; // columns / fields in schema
    bool     isDirty          = false;
};

// ── DataEditorTool ────────────────────────────────────────────────

class DataEditorTool final : public IHostedTool {
public:
    static constexpr const char* kToolId = "workspace.data_editor";

    DataEditorTool();
    ~DataEditorTool() override = default;

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
    void onProjectLoaded(const std::string& projectId) override;
    void onProjectUnloaded() override;

    // ── Data Editor interface ─────────────────────────────────────

    [[nodiscard]] DataEditMode           editMode() const { return m_editMode; }
    void                                 setEditMode(DataEditMode mode);

    [[nodiscard]] const DataEditorStats& stats()   const { return m_stats; }
    [[nodiscard]] bool                   isDirty() const { return m_stats.isDirty; }
    void                                 markDirty();
    void                                 clearDirty();

    void setRowCount(uint32_t count);
    void setSelectedRowCount(uint32_t count);
    void setColumnCount(uint32_t count);

    [[nodiscard]] const std::string& openTablePath() const { return m_openTablePath; }
    void                             setOpenTablePath(const std::string& path);

    // ── Render contract ───────────────────────────────────────────
    // Renders: Tables | Data Grid | Inspector — three-column layout.
    void renderToolView(const ToolViewRenderContext& ctx) const override;
    void syncPanels(WorkspacePanelHost& host) const override;

private:
    HostedToolDescriptor m_descriptor;
    HostedToolState      m_state        = HostedToolState::Unloaded;
    DataEditMode         m_editMode     = DataEditMode::Table;
    DataEditorStats      m_stats;
    std::string          m_openTablePath;
    std::string          m_activeProjectId;

    // ── Mutable per-view UI state (safe from const renderToolView) ─
    mutable int         m_viewSelectedRow  = -1;
    mutable std::string m_viewOpenTable;           // selected table in the tree panel

    void buildDescriptor();
};

} // namespace NF
