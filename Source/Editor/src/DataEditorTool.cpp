// NF::Editor — DataEditorTool implementation.
//
// Fifth real NF::IHostedTool from Phase 3 consolidation.
// Manages data tables, game configs, and localization keys within AtlasWorkspace.

#include "NF/Editor/DataEditorTool.h"
#include "NF/Workspace/ToolViewRenderContext.h"
#include "NF/Workspace/WorkspacePanelHost.h"
#include "NF/Workspace/WorkspaceShell.h"
#include <cstdio>
#include <cstring>

namespace NF {

DataEditorTool::DataEditorTool() {
    buildDescriptor();
}

void DataEditorTool::buildDescriptor() {
    m_descriptor.toolId      = kToolId;
    m_descriptor.displayName = "Data Editor";
    m_descriptor.category    = HostedToolCategory::DataEditing;
    m_descriptor.isPrimary   = true;
    m_descriptor.acceptsProjectExtensions = true;

    m_descriptor.supportedPanels = {
        "panel.data_table",
        "panel.inspector",
        "panel.console",
    };

    m_descriptor.commands = {
        "data.new_row",
        "data.delete_row",
        "data.duplicate_row",
        "data.import_csv",
        "data.export_csv",
        "data.save",
    };
}

bool DataEditorTool::initialize() {
    if (m_state != HostedToolState::Unloaded) return false;
    m_editMode      = DataEditMode::Table;
    m_stats         = {};
    m_openTablePath = {};
    m_state         = HostedToolState::Ready;
    return true;
}

void DataEditorTool::shutdown() {
    m_state           = HostedToolState::Unloaded;
    m_editMode        = DataEditMode::Table;
    m_stats           = {};
    m_openTablePath   = {};
    m_activeProjectId = {};
}

void DataEditorTool::activate() {
    if (m_state == HostedToolState::Ready || m_state == HostedToolState::Suspended) {
        m_state = HostedToolState::Active;
    }
}

void DataEditorTool::suspend() {
    if (m_state == HostedToolState::Active) {
        m_state = HostedToolState::Suspended;
    }
}

void DataEditorTool::update(float /*dt*/) {
    // Data editor updates are event-driven (cell edits, import/export).
}

void DataEditorTool::onProjectLoaded(const std::string& projectId) {
    m_activeProjectId = projectId;
    m_stats           = {};
    m_openTablePath   = {};
}

void DataEditorTool::onProjectUnloaded() {
    m_activeProjectId = {};
    m_stats           = {};
    m_openTablePath   = {};
}

void DataEditorTool::setEditMode(DataEditMode mode) {
    m_editMode = mode;
}

void DataEditorTool::markDirty() {
    m_stats.isDirty = true;
}

void DataEditorTool::clearDirty() {
    m_stats.isDirty = false;
}

void DataEditorTool::setRowCount(uint32_t count) {
    m_stats.rowCount = count;
}

void DataEditorTool::setSelectedRowCount(uint32_t count) {
    m_stats.selectedRowCount = count;
}

void DataEditorTool::setColumnCount(uint32_t count) {
    m_stats.columnCount = count;
}

void DataEditorTool::setOpenTablePath(const std::string& path) {
    m_openTablePath = path;
}

void DataEditorTool::renderToolView(const ToolViewRenderContext& ctx) const {
    // Three-column layout: Tables (25%) | Data Grid (50%) | Inspector (25%)
    const float tablesW = ctx.w * 0.25f;
    const float gridW   = ctx.w * 0.50f;
    const float inspW   = ctx.w - tablesW - gridW;

    // Stub table list (reflects open table + project-catalog tables when shell wired)
    static const char* kTableNames[] = {
        "PlayerStats", "EnemyConfig", "ItemTable", "QuestData", "SkillTree"
    };
    static constexpr int kTableCount = 5;

    // ── Tables panel ──────────────────────────────────────────────
    ctx.drawPanel(ctx.x, ctx.y, tablesW, ctx.h, "Tables");
    {
        // Show the externally-set table path if available, otherwise the view selection
        const std::string& displayPath = !m_openTablePath.empty() ? m_openTablePath : m_viewOpenTable;
        float ty2 = ctx.y + 28.f;
        for (int i = 0; i < kTableCount; ++i) {
            if (ty2 + 20.f > ctx.y + ctx.h - 4.f) break;
            bool active = (displayPath == kTableNames[i]);
            bool hov    = ctx.isHovered({ctx.x + 2.f, ty2, tablesW - 4.f, 18.f});
            uint32_t bg = active ? ctx.kAccentBlue
                        : (hov ? 0x2A2A3AFF : 0x00000000u);
            if (bg) ctx.ui.drawRect({ctx.x + 2.f, ty2, tablesW - 4.f, 18.f}, bg);
            ctx.ui.drawText(ctx.x + 8.f, ty2 + 2.f, kTableNames[i],
                            active ? ctx.kTextPrimary : ctx.kTextSecond);
            if (ctx.hitRegion({ctx.x + 2.f, ty2, tablesW - 4.f, 18.f}, false))
                m_viewOpenTable = active ? "" : kTableNames[i];
            ty2 += 20.f;
        }
    }

    // ── Data Grid ─────────────────────────────────────────────────
    ctx.drawPanel(ctx.x + tablesW, ctx.y, gridW, ctx.h, "Data Grid");
    {
        // Determine active table — prefer explicitly set path, then view selection
        const std::string& activeName = !m_openTablePath.empty() ? m_openTablePath : m_viewOpenTable;

        // Schema-aware column definitions per table (from story stubs).
        // Each table has up to 5 columns with labels, x-offsets (relative to gridW), and sample data.
        struct ColDef { const char* header; float xRel; uint32_t labelColor; };
        struct RowData { const char* cols[5]; };

        // PlayerStats schema (InventorySystemEditorV1 / ProgressionEditorV1 types)
        static constexpr ColDef kPlayerCols[] = {
            {"ID",    0.01f, 0x888888FF}, {"Name",   0.10f, 0xE0E0E0FF},
            {"HP",    0.35f, 0xE05050FF}, {"Speed",  0.50f, 0x4EC94EFF},
            {"Level", 0.65f, 0xCCAA44FF},
        };
        static constexpr RowData kPlayerRows[] = {
            {{"001","Player",   "100","5.0","1"}},
            {{"002","NPC_Guard","80", "3.5","2"}},
            {{"003","Boss_Ogre","350","2.0","8"}},
        };

        // EnemyConfig schema (AISpawnEditorV1 / AIGoalEditorV1 types)
        static constexpr ColDef kEnemyCols[] = {
            {"ID",     0.01f, 0x888888FF}, {"Name",    0.10f, 0xE0E0E0FF},
            {"HP",     0.35f, 0xE05050FF}, {"Damage",  0.50f, 0xCCAA44FF},
            {"Type",   0.65f, 0x4488CCFF},
        };
        static constexpr RowData kEnemyRows[] = {
            {{"001","Skeleton","50","10","Melee"}},
            {{"002","Archer",  "30","15","Ranged"}},
            {{"003","Troll",   "200","30","Tank"}},
        };

        // ItemTable schema (WeaponEditorV1 / AbilityEditorV1 types)
        static constexpr ColDef kItemCols[] = {
            {"ID",     0.01f, 0x888888FF}, {"Name",   0.10f, 0xE0E0E0FF},
            {"Type",   0.35f, 0x4488CCFF}, {"Rarity", 0.50f, 0xCCAA44FF},
            {"Value",  0.65f, 0x4EC94EFF},
        };
        static constexpr RowData kItemRows[] = {
            {{"001","Iron Sword", "Weapon","Common",   "50"}},
            {{"002","Fireball",   "Ability","Rare",    "0"}},
            {{"003","Health Pot", "Consumable","Common","25"}},
        };

        // QuestData schema (QuestScriptEditorV1 types)
        static constexpr ColDef kQuestCols[] = {
            {"ID",    0.01f, 0x888888FF}, {"Title",  0.10f, 0xE0E0E0FF},
            {"State", 0.40f, 0x4EC94EFF}, {"Req",    0.55f, 0xCCAA44FF},
            {"Reward",0.70f, 0x4488CCFF},
        };
        static constexpr RowData kQuestRows[] = {
            {{"001","The Beginning","Active","Kill 3","100g"}},
            {{"002","Lost Sword",   "Locked","None",  "Sword"}},
            {{"003","Escort NPC",   "Done",  "None",  "200g"}},
        };

        // SkillTree schema (SkillSystemEditorV1 types)
        static constexpr ColDef kSkillCols[] = {
            {"ID",   0.01f, 0x888888FF}, {"Skill",    0.10f, 0xE0E0E0FF},
            {"Cost", 0.35f, 0xCCAA44FF}, {"Cooldown", 0.50f, 0x4488CCFF},
            {"Type", 0.65f, 0x4EC94EFF},
        };
        static constexpr RowData kSkillRows[] = {
            {{"001","Fireball",  "20","3.0s","Active"}},
            {{"002","Block",     "0", "0.5s","Passive"}},
            {{"003","Dash",      "10","1.0s","Active"}},
        };

        // Select columns and rows by active table name
        struct SchemaView {
            const ColDef*  cols;
            int            colCount;
            const RowData* rows;
            int            rowCount;
        };
        static constexpr SchemaView kSchemas[5] = {
            {kPlayerCols, 5, kPlayerRows, 3},
            {kEnemyCols,  5, kEnemyRows,  3},
            {kItemCols,   5, kItemRows,   3},
            {kQuestCols,  5, kQuestRows,  3},
            {kSkillCols,  5, kSkillRows,  3},
        };
        int schemaIdx = 0;
        for (int i = 0; i < kTableCount; ++i) {
            if (activeName == kTableNames[i]) { schemaIdx = i; break; }
        }
        const SchemaView& schema = kSchemas[schemaIdx];

        char gridBuf[64];
        std::snprintf(gridBuf, sizeof(gridBuf), "%u rows  x  %d cols",
                      m_stats.rowCount > 0 ? m_stats.rowCount
                      : static_cast<uint32_t>(schema.rowCount),
                      schema.colCount);
        ctx.ui.drawText(ctx.x + tablesW + 8.f, ctx.y + 30.f, gridBuf, ctx.kTextSecond);

        // Schema type badge
        ctx.drawStatusPill(ctx.x + tablesW + 8.f, ctx.y + 30.f + 16.f,
                           kTableNames[schemaIdx], ctx.kAccentBlue);

        // Header row with schema-specific columns
        float hdrY = ctx.y + 64.f;
        ctx.ui.drawRect({ctx.x + tablesW, hdrY, gridW, 18.f}, 0x303030FF);
        for (int c = 0; c < schema.colCount; ++c)
            ctx.ui.drawText(ctx.x + tablesW + gridW * schema.cols[c].xRel + 2.f,
                            hdrY + 2.f, schema.cols[c].header, ctx.kTextSecond);
        ctx.ui.drawRect({ctx.x + tablesW, hdrY + 18.f, gridW, 1.f}, ctx.kBorder);

        // Data rows
        float ry2 = hdrY + 20.f;
        int rowsToShow = schema.rowCount;
        for (int i = 0; i < rowsToShow; ++i) {
            if (ry2 + 18.f > ctx.y + ctx.h - 4.f) break;
            bool sel = (m_viewSelectedRow == i);
            bool hov = ctx.isHovered({ctx.x + tablesW + 2.f, ry2, gridW - 4.f, 16.f});
            uint32_t bg = sel ? 0x1A3A6AFF : (hov ? 0x2A2A3AFF : 0u);
            if (bg) ctx.ui.drawRect({ctx.x + tablesW + 2.f, ry2, gridW - 4.f, 16.f}, bg);
            ctx.ui.drawRect({ctx.x + tablesW + 2.f, ry2, 2.f, 16.f},
                            sel ? ctx.kAccentBlue : 0x404040FF);
            for (int c = 0; c < schema.colCount; ++c) {
                ctx.ui.drawText(ctx.x + tablesW + gridW * schema.cols[c].xRel + 4.f,
                                ry2 + 1.f, schema.rows[i].cols[c],
                                sel ? ctx.kTextPrimary : schema.cols[c].labelColor);
            }
            if (ctx.hitRegion({ctx.x + tablesW + 2.f, ry2, gridW - 4.f, 16.f}, false))
                m_viewSelectedRow = sel ? -1 : i;
            ry2 += 18.f;
        }
        if (m_stats.selectedRowCount > 0) {
            char selBuf[32];
            std::snprintf(selBuf, sizeof(selBuf), "%u selected", m_stats.selectedRowCount);
            ctx.drawStatusPill(ctx.x + tablesW + 8.f, ctx.y + ctx.h - 22.f,
                               selBuf, ctx.kAccentBlue);
        }
    }
    if (m_stats.isDirty) {
        ctx.ui.drawText(ctx.x + tablesW + 8.f, ctx.y + ctx.h - 20.f,
                        "* unsaved changes", ctx.kRed);
    }

    // ── Inspector ─────────────────────────────────────────────────
    ctx.drawPanel(ctx.x + tablesW + gridW, ctx.y, inspW, ctx.h, "Inspector");
    const float ix = ctx.x + tablesW + gridW;
    if (m_viewSelectedRow >= 0) {
        static const char* kRowIds[]    = {"001","002","003","004","005","006","007","008","009","010"};
        static const char* kRowValues[] = {"100","50","12","1","0","42","7","99","3","18"};
        ctx.ui.drawText(ix + 8.f, ctx.y + 30.f, "Row Details", ctx.kTextPrimary);
        ctx.ui.drawRect({ix + 4.f, ctx.y + 44.f, inspW - 8.f, 1.f}, ctx.kBorder);
        int ri = m_viewSelectedRow;
        ctx.drawStatRow(ix + 8.f, ctx.y + 50.f, "ID:",    kRowIds[ri % 10]);
        ctx.drawStatRow(ix + 8.f, ctx.y + 68.f, "Table:", kTableNames[0]);
        ctx.drawStatRow(ix + 8.f, ctx.y + 86.f, "Value:", kRowValues[ri % 10]);

        // Save row button
        if (ctx.drawButton(ix + 8.f, ctx.y + ctx.h - 30.f, inspW - 16.f, 20.f, "Save Row")) {
            if (ctx.shell)
                (void)ctx.shell->commandBus().execute("data.save_row");
        }
    } else {
        ctx.ui.drawText(ix + 8.f, ctx.y + 30.f, "Row Properties", ctx.kTextSecond);
        ctx.ui.drawText(ix + 8.f, ctx.y + 50.f, "Select a row", ctx.kTextMuted);
    }
}


void DataEditorTool::syncPanels(WorkspacePanelHost& host) const {
    (void)host;
}

} // namespace NF
