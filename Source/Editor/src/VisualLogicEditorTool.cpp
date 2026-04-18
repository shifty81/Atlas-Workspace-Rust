// NF::Editor — VisualLogicEditorTool implementation.
//
// Sixth real NF::IHostedTool from Phase 3 consolidation.
// Manages visual scripting / blueprint graph authoring within AtlasWorkspace.

#include "NF/Editor/VisualLogicEditorTool.h"
#include "NF/Workspace/ToolViewRenderContext.h"
#include "NF/Workspace/WorkspacePanelHost.h"
#include "NF/Workspace/WorkspaceShell.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <string>

namespace NF {

VisualLogicEditorTool::VisualLogicEditorTool() {
    buildDescriptor();
}

void VisualLogicEditorTool::buildDescriptor() {
    m_descriptor.toolId      = kToolId;
    m_descriptor.displayName = "Visual Logic Editor";
    m_descriptor.category    = HostedToolCategory::LogicAuthoring;
    m_descriptor.isPrimary   = true;
    m_descriptor.acceptsProjectExtensions = false;

    m_descriptor.supportedPanels = {
        "panel.node_graph",
        "panel.inspector",
        "panel.console",
    };

    m_descriptor.commands = {
        "vl.add_node",
        "vl.delete_node",
        "vl.connect_pins",
        "vl.disconnect_pins",
        "vl.compile",
        "vl.save",
    };
}

bool VisualLogicEditorTool::initialize() {
    if (m_state != HostedToolState::Unloaded) return false;
    m_editMode = VisualLogicMode::Graph;
    m_stats    = {};
    m_state    = HostedToolState::Ready;
    return true;
}

void VisualLogicEditorTool::shutdown() {
    m_state           = HostedToolState::Unloaded;
    m_editMode        = VisualLogicMode::Graph;
    m_stats           = {};
    m_activeProjectId = {};
}

void VisualLogicEditorTool::activate() {
    if (m_state == HostedToolState::Ready || m_state == HostedToolState::Suspended) {
        m_state = HostedToolState::Active;
    }
}

void VisualLogicEditorTool::suspend() {
    if (m_state == HostedToolState::Active) {
        m_stats.isCompiling = false;
        m_state = HostedToolState::Suspended;
    }
}

void VisualLogicEditorTool::update(float /*dt*/) {
    // Compilation and graph evaluation are async; no per-frame polling needed here.
}

void VisualLogicEditorTool::onProjectLoaded(const std::string& projectId) {
    m_activeProjectId = projectId;
    m_stats           = {};
}

void VisualLogicEditorTool::onProjectUnloaded() {
    m_activeProjectId = {};
    m_stats           = {};
}

void VisualLogicEditorTool::setEditMode(VisualLogicMode mode) {
    m_editMode = mode;
}

void VisualLogicEditorTool::markDirty() {
    m_stats.isDirty = true;
}

void VisualLogicEditorTool::clearDirty() {
    m_stats.isDirty = false;
}

void VisualLogicEditorTool::setNodeCount(uint32_t count) {
    m_stats.nodeCount = count;
}

void VisualLogicEditorTool::setConnectionCount(uint32_t count) {
    m_stats.connectionCount = count;
}

void VisualLogicEditorTool::setCompiling(bool compiling) {
    m_stats.isCompiling = compiling;
}

void VisualLogicEditorTool::setErrorCount(uint32_t count) {
    m_stats.errorCount = count;
}

void VisualLogicEditorTool::renderToolView(const ToolViewRenderContext& ctx) const {
    // ── Mode tab strip ────────────────────────────────────────────
    static constexpr struct { const char* label; VisualLogicMode mode; } kTabs[] = {
        {"Graph",     VisualLogicMode::Graph},
        {"Dialogue",  VisualLogicMode::Dialogue},
        {"FSM",       VisualLogicMode::StateMachine},
        {"BehTree",   VisualLogicMode::BehaviorTree},
        {"VFX",       VisualLogicMode::VFX},
        {"Debug",     VisualLogicMode::Debug},
        {"Diff",      VisualLogicMode::Diff},
    };
    {
        float tx = ctx.x + 4.f;
        float ty = ctx.y + 2.f;
        for (const auto& tab : kTabs) {
            bool active = (m_editMode == tab.mode);
            float tw = static_cast<float>(std::strlen(tab.label)) * 7.f + 14.f;
            uint32_t bg = active ? ctx.kAccentBlue : 0x2A2A2AFF;
            if (ctx.drawButton(tx, ty, tw, 18.f, tab.label, bg))
                m_editMode = tab.mode;
            tx += tw + 2.f;
        }
    }

    const float cy = ctx.y + 22.f;
    const float ch = ctx.h - 22.f;
    const float nodesW = ctx.w * 0.20f;
    const float graphW = ctx.w * 0.60f;
    const float propW  = ctx.w - nodesW - graphW;

    // ── GRAPH mode (ScriptGraphEditorV1) ─────────────────────────
    if (m_editMode == VisualLogicMode::Graph) {
        // Palette from ScriptGraphEditorV1 node categories
        static const char* kNodeTypes[] = {
            "On Start", "On Event", "Branch", "Loop",
            "Set Variable", "Get Variable", "Spawn Entity",
            "Play Sound", "Delay", "Function Call",
        };
        static constexpr int kPaletteCount = 10;
        static constexpr uint32_t kPalColors[] = {
            0xCC4444FF, 0xCC6644FF, 0x448844FF, 0x44AA44FF,
            0x4488CCFF, 0x4466AAFF, 0x884488FF,
            0x446688FF, 0x666644FF, 0x886644FF,
        };

        // Left — Node palette
        ctx.drawPanel(ctx.x, cy, nodesW, ch, "Nodes");
        {
            char nodeBuf[32];
            std::snprintf(nodeBuf, sizeof(nodeBuf), "%u nodes", m_stats.nodeCount);
            ctx.ui.drawText(ctx.x + 8.f, cy + 8.f, nodeBuf, ctx.kTextSecond);
            char connBuf[32];
            std::snprintf(connBuf, sizeof(connBuf), "%u connections", m_stats.connectionCount);
            ctx.ui.drawText(ctx.x + 8.f, cy + 26.f, connBuf, ctx.kTextSecond);
            ctx.ui.drawText(ctx.x + 8.f, cy + 46.f, "Palette:", ctx.kTextMuted);
            float ny = cy + 62.f;
            for (int i = 0; i < kPaletteCount; ++i) {
                if (ny + 16.f > cy + ch - 4.f) break;
                bool sel = (m_viewSelectedNode == i);
                bool hov = ctx.isHovered({ctx.x + 2.f, ny, nodesW - 4.f, 14.f});
                uint32_t bg = sel ? ctx.kAccentBlue : (hov ? 0x2A2A3AFF : 0u);
                if (bg) ctx.ui.drawRect({ctx.x + 2.f, ny, nodesW - 4.f, 14.f}, bg);
                ctx.ui.drawRect({ctx.x + 6.f, ny + 4.f, 6.f, 6.f}, kPalColors[i]);
                ctx.ui.drawText(ctx.x + 16.f, ny + 1.f, kNodeTypes[i],
                                sel ? ctx.kTextPrimary : ctx.kTextSecond);
                if (ctx.hitRegion({ctx.x + 2.f, ny, nodesW - 4.f, 14.f}, false))
                    m_viewSelectedNode = sel ? -1 : i;
                ny += 16.f;
            }
        }

        // Centre — Graph canvas with typed node cards
        ctx.drawPanel(ctx.x + nodesW, cy, graphW, ch, "Graph Canvas");
        {
            const float cvx = ctx.x + nodesW + 4.f;
            const float cvy = cy + 4.f;
            const float cvw = graphW - 8.f;
            const float cvh = ch - 8.f;
            ctx.ui.drawRect({cvx, cvy, cvw, cvh}, 0x181818FF);
            for (float gx = cvx; gx < cvx + cvw; gx += 28.f)
                for (float gy = cvy; gy < cvy + cvh; gy += 28.f)
                    ctx.ui.drawRect({gx, gy, 1.f, 1.f}, 0x2A2A2AFF);

            struct GNode { const char* label; float rx; float ry; uint32_t hc; int in; int out; };
            static const GNode kGNodes[] = {
                {"On Start",     0.05f, 0.20f, 0xCC4444FF, 0, 1},
                {"Branch",       0.30f, 0.18f, 0x448844FF, 1, 2},
                {"Set Variable", 0.55f, 0.10f, 0x4488CCFF, 1, 1},
                {"Spawn Entity", 0.55f, 0.45f, 0x884488FF, 1, 1},
                {"Play Sound",   0.80f, 0.25f, 0x446688FF, 1, 0},
            };
            static constexpr int kGNodeCount = 5;
            static constexpr float kNW = 100.f, kNH = 48.f;

            for (int i = 0; i < kGNodeCount; ++i) {
                float nx = cvx + cvw * kGNodes[i].rx;
                float ny = cvy + cvh * kGNodes[i].ry;
                bool sel = (m_viewSelectedNode == i);
                ctx.ui.drawRect({nx, ny, kNW, kNH}, sel ? 0x1E2E3EFF : ctx.kCardBg);
                ctx.ui.drawRectOutline({nx, ny, kNW, kNH},
                                       sel ? ctx.kAccentBlue : ctx.kBorder, 1.f);
                ctx.ui.drawRect({nx, ny, kNW, 20.f}, kGNodes[i].hc);
                ctx.ui.drawText(nx + 4.f, ny + 4.f, kGNodes[i].label, ctx.kTextPrimary);
                if (kGNodes[i].in > 0)
                    ctx.ui.drawRect({nx - 6.f, ny + 28.f, 6.f, 6.f}, 0xAAAAAA88u);
                if (kGNodes[i].out > 0)
                    ctx.ui.drawRect({nx + kNW, ny + 28.f, 6.f, 6.f}, 0xAAAAAA88u);
                if (ctx.hitRegion({nx, ny, kNW, kNH}, false))
                    m_viewSelectedNode = (m_viewSelectedNode == i) ? -1 : i;
            }
            // Wires
            static constexpr struct { int f; int t; } kWires[] = {{0,1},{1,2},{1,3},{3,4}};
            for (const auto& w : kWires) {
                float x0 = cvx + cvw * kGNodes[w.f].rx + kNW + 6.f;
                float y0 = cvy + cvh * kGNodes[w.f].ry + 31.f;
                float x1 = cvx + cvw * kGNodes[w.t].rx - 6.f;
                float y1 = cvy + cvh * kGNodes[w.t].ry + 31.f;
                float mx = (x0 + x1) * 0.5f;
                ctx.ui.drawRect({x0, y0, mx - x0, 1.f}, ctx.kTextMuted);
                if (y0 != y1) {
                    float top = y0 < y1 ? y0 : y1;
                    float bot = y0 < y1 ? y1 : y0;
                    ctx.ui.drawRect({mx, top, 1.f, bot - top}, ctx.kTextMuted);
                }
                ctx.ui.drawRect({mx, y1, x1 - mx, 1.f}, ctx.kTextMuted);
            }

            // Compile / Run buttons
            if (ctx.drawButton(cvx + cvw - 96.f, cvy + 4.f, 44.f, 16.f, "Compile")) {
                if (ctx.shell)
                    (void)ctx.shell->commandBus().execute("logic.compile");
            }
            if (ctx.drawButton(cvx + cvw - 48.f, cvy + 4.f, 40.f, 16.f, "Run")) {
                if (ctx.shell)
                    (void)ctx.shell->commandBus().execute("logic.run");
            }

            // Compile status
            ctx.drawStatusPill(cvx + 4.f, cvy + 4.f,
                               visualLogicModeName(m_editMode), ctx.kAccentBlue);
            if (m_stats.isCompiling)
                ctx.drawStatusPill(cvx + 60.f, cvy + 4.f, "Compiling...", ctx.kAccentBlue);
            else if (m_stats.errorCount > 0) {
                char eb[24]; std::snprintf(eb, sizeof(eb), "%u errors", m_stats.errorCount);
                ctx.drawStatusPill(cvx + 60.f, cvy + 4.f, eb, ctx.kRed);
            } else
                ctx.drawStatusPill(cvx + 60.f, cvy + 4.f, "OK", ctx.kGreen);
        }

        // Right — Properties
        ctx.drawPanel(ctx.x + nodesW + graphW, cy, propW, ch, "Properties");
        const float px = ctx.x + nodesW + graphW;
        if (m_viewSelectedNode >= 0 && m_viewSelectedNode < kPaletteCount) {
            ctx.ui.drawText(px + 8.f, cy + 10.f, kNodeTypes[m_viewSelectedNode],
                            ctx.kTextPrimary);
            ctx.ui.drawRect({px + 4.f, cy + 24.f, propW - 8.f, 1.f}, ctx.kBorder);
            ctx.ui.drawText(px + 8.f, cy + 30.f, "Type: Logic Node", ctx.kTextSecond);
            ctx.ui.drawText(px + 8.f, cy + 48.f, "Inputs:  1",  ctx.kTextMuted);
            ctx.ui.drawText(px + 8.f, cy + 66.f, "Outputs: 2",  ctx.kTextMuted);
        } else {
            ctx.ui.drawText(px + 8.f, cy + 20.f, "Select a node", ctx.kTextMuted);
            char nb[16]; std::snprintf(nb, sizeof(nb), "%u", m_stats.nodeCount);
            ctx.drawStatRow(px + 8.f, cy + 40.f, "Nodes:", nb);
        }
        if (m_stats.isDirty)
            ctx.ui.drawText(px + 8.f, cy + ch - 18.f, "* unsaved", ctx.kRed);
    }

    // ── DIALOGUE mode (DialogueTreeEditorV1) ─────────────────────
    else if (m_editMode == VisualLogicMode::Dialogue) {
        // Node types: Entry/Line/Choice/Condition/Action/Exit (Dtev1NodeType)
        static constexpr struct {
            const char* label;
            const char* type;
            uint32_t    headerColor;
            float       rx, ry;
            const char* text;
        } kDlgNodes[] = {
            {"Entry",        "Entry",     0x337733FF, 0.04f, 0.40f, ""},
            {"NPC: Hello!",  "Line",      0x336699FF, 0.22f, 0.25f, "Hello, traveller!"},
            {"Choice",       "Choice",    0x885544FF, 0.45f, 0.38f, ""},
            {"Option: Yes",  "Line",      0x336699FF, 0.66f, 0.18f, "Yes, please."},
            {"Option: No",   "Line",      0x336699FF, 0.66f, 0.58f, "No, thank you."},
            {"Cond: HasKey", "Condition", 0xAA6633FF, 0.66f, 0.38f, "HasItem(key)"},
            {"Exit",         "Exit",      0xAA3333FF, 0.88f, 0.38f, ""},
        };
        static constexpr int kDlgCount = 7;

        // Left — Dialogue tree list
        ctx.drawPanel(ctx.x, cy, nodesW, ch, "Dialogue");
        {
            ctx.ui.drawText(ctx.x + 8.f, cy + 8.f, "Nodes:", ctx.kTextMuted);
            float ry = cy + 26.f;
            for (int i = 0; i < kDlgCount; ++i) {
                if (ry + 18.f > cy + ch - 4.f) break;
                bool sel = (m_viewSelectedDlgNode == i);
                bool hov = ctx.isHovered({ctx.x + 2.f, ry, nodesW - 4.f, 16.f});
                uint32_t bg = sel ? ctx.kAccentBlue : (hov ? 0x2A2A3AFF : 0u);
                if (bg) ctx.ui.drawRect({ctx.x + 2.f, ry, nodesW - 4.f, 16.f}, bg);
                ctx.ui.drawRect({ctx.x + 6.f, ry + 4.f, 6.f, 6.f}, kDlgNodes[i].headerColor);
                ctx.ui.drawText(ctx.x + 16.f, ry + 1.f, kDlgNodes[i].label,
                                sel ? ctx.kTextPrimary : ctx.kTextSecond);
                if (ctx.hitRegion({ctx.x + 2.f, ry, nodesW - 4.f, 16.f}, false))
                    m_viewSelectedDlgNode = sel ? -1 : i;
                ry += 18.f;
            }
        }

        // Centre — Dialogue tree canvas
        ctx.drawPanel(ctx.x + nodesW, cy, graphW, ch, "Dialogue Tree");
        {
            const float cvx = ctx.x + nodesW + 4.f;
            const float cvy = cy + 4.f;
            const float cvw = graphW - 8.f;
            const float cvh = ch - 8.f;
            ctx.ui.drawRect({cvx, cvy, cvw, cvh}, 0x181818FF);
            static constexpr float kNW = 120.f, kNH = 54.f;

            for (int i = 0; i < kDlgCount; ++i) {
                float nx = cvx + cvw * kDlgNodes[i].rx;
                float ny = cvy + cvh * kDlgNodes[i].ry;
                bool sel = (m_viewSelectedDlgNode == i);
                ctx.ui.drawRect({nx, ny, kNW, kNH}, sel ? 0x1E2E3EFF : ctx.kCardBg);
                ctx.ui.drawRectOutline({nx, ny, kNW, kNH},
                                       sel ? ctx.kAccentBlue : ctx.kBorder, 1.f);
                ctx.ui.drawRect({nx, ny, kNW, 18.f}, kDlgNodes[i].headerColor);
                ctx.ui.drawText(nx + 4.f, ny + 2.f, kDlgNodes[i].label, ctx.kTextPrimary);
                ctx.ui.drawText(nx + 4.f, ny + 12.f, kDlgNodes[i].type, 0x888888FF);
                if (kDlgNodes[i].text[0])
                    ctx.ui.drawText(nx + 4.f, ny + 30.f, kDlgNodes[i].text, ctx.kTextSecond);
                // Pins
                ctx.ui.drawRect({nx - 5.f, ny + kNH * 0.5f, 5.f, 5.f}, 0x888866FFu);
                if (std::strcmp(kDlgNodes[i].type, "Exit") != 0)
                    ctx.ui.drawRect({nx + kNW, ny + kNH * 0.5f, 5.f, 5.f}, 0x886688FFu);
                if (ctx.hitRegion({nx, ny, kNW, kNH}, false))
                    m_viewSelectedDlgNode = (m_viewSelectedDlgNode == i) ? -1 : i;
            }
            // Arrows (Entry→Line→Choice→Options→Exit)
            static constexpr struct { int f; int t; } kDlgWires[] = {
                {0,1},{1,2},{2,3},{2,4},{2,5},{3,6},{4,6},{5,6}
            };
            for (const auto& w : kDlgWires) {
                float x0 = cvx + cvw * kDlgNodes[w.f].rx + kNW + 5.f;
                float y0 = cvy + cvh * kDlgNodes[w.f].ry + kNH * 0.5f + 2.f;
                float x1 = cvx + cvw * kDlgNodes[w.t].rx - 5.f;
                float y1 = cvy + cvh * kDlgNodes[w.t].ry + kNH * 0.5f + 2.f;
                float mx = (x0 + x1) * 0.5f;
                ctx.ui.drawRect({x0, y0, mx - x0, 1.f}, 0x666666FF);
                if (y0 != y1) {
                    float top = y0 < y1 ? y0 : y1;
                    ctx.ui.drawRect({mx, top, 1.f, std::abs(y1 - y0)}, 0x666666FF);
                }
                ctx.ui.drawRect({mx, y1, x1 - mx, 1.f}, 0x666666FF);
            }
        }

        // Right — Node inspector
        ctx.drawPanel(ctx.x + nodesW + graphW, cy, propW, ch, "Properties");
        const float px = ctx.x + nodesW + graphW;
        if (m_viewSelectedDlgNode >= 0 && m_viewSelectedDlgNode < kDlgCount) {
            ctx.ui.drawText(px + 8.f, cy + 10.f, kDlgNodes[m_viewSelectedDlgNode].label,
                            ctx.kTextPrimary);
            ctx.ui.drawRect({px + 4.f, cy + 24.f, propW - 8.f, 1.f}, ctx.kBorder);
            ctx.drawStatRow(px + 8.f, cy + 30.f, "Type:", kDlgNodes[m_viewSelectedDlgNode].type);
            if (kDlgNodes[m_viewSelectedDlgNode].text[0]) {
                ctx.ui.drawText(px + 8.f, cy + 50.f, "Text:", ctx.kTextMuted);
                ctx.ui.drawText(px + 8.f, cy + 68.f, kDlgNodes[m_viewSelectedDlgNode].text,
                                ctx.kTextSecond);
            }
        } else {
            ctx.ui.drawText(px + 8.f, cy + 20.f, "Select a node", ctx.kTextMuted);
        }
    }

    // ── STATE MACHINE mode (StateGraphV1) ─────────────────────────
    else if (m_editMode == VisualLogicMode::StateMachine) {
        // State types: State/AnyState/Entry/Exit (Sgv1NodeType)
        static constexpr struct {
            const char* name;
            const char* type;
            uint32_t    color;
            float       rx, ry;
        } kStates[] = {
            {"Entry",     "Entry",    0x339933FF, 0.10f, 0.45f},
            {"Idle",      "State",    0x336699FF, 0.30f, 0.25f},
            {"Run",       "State",    0x336699FF, 0.30f, 0.65f},
            {"Jump",      "State",    0x336699FF, 0.58f, 0.25f},
            {"Attack",    "State",    0x993333FF, 0.58f, 0.65f},
            {"Any State", "AnyState", 0x665533FF, 0.44f, 0.45f},
            {"Exit",      "Exit",     0xAA3333FF, 0.82f, 0.45f},
        };
        static constexpr int kStateCount = 7;

        // Left — State list
        ctx.drawPanel(ctx.x, cy, nodesW, ch, "States");
        {
            float ry = cy + 8.f;
            for (int i = 0; i < kStateCount; ++i) {
                if (ry + 18.f > cy + ch - 4.f) break;
                bool sel = (m_viewSelectedState == i);
                bool hov = ctx.isHovered({ctx.x + 2.f, ry, nodesW - 4.f, 16.f});
                uint32_t bg = sel ? ctx.kAccentBlue : (hov ? 0x2A2A3AFF : 0u);
                if (bg) ctx.ui.drawRect({ctx.x + 2.f, ry, nodesW - 4.f, 16.f}, bg);
                ctx.ui.drawRect({ctx.x + 6.f, ry + 4.f, 6.f, 6.f}, kStates[i].color);
                ctx.ui.drawText(ctx.x + 16.f, ry + 1.f, kStates[i].name,
                                sel ? ctx.kTextPrimary : ctx.kTextSecond);
                if (ctx.hitRegion({ctx.x + 2.f, ry, nodesW - 4.f, 16.f}, false))
                    m_viewSelectedState = sel ? -1 : i;
                ry += 18.f;
            }
        }

        // Centre — FSM canvas
        ctx.drawPanel(ctx.x + nodesW, cy, graphW, ch, "State Machine");
        {
            const float cvx = ctx.x + nodesW + 4.f;
            const float cvy = cy + 4.f;
            const float cvw = graphW - 8.f;
            const float cvh = ch - 8.f;
            ctx.ui.drawRect({cvx, cvy, cvw, cvh}, 0x181818FF);
            // Grid
            for (float gx = cvx; gx < cvx + cvw; gx += 32.f)
                ctx.ui.drawRect({gx, cvy, 1.f, cvh}, 0x202020FF);
            for (float gy = cvy; gy < cvy + cvh; gy += 32.f)
                ctx.ui.drawRect({cvx, gy, cvw, 1.f}, 0x202020FF);

            static constexpr float kNW = 80.f, kNH = 40.f;
            for (int i = 0; i < kStateCount; ++i) {
                float nx = cvx + cvw * kStates[i].rx;
                float ny = cvy + cvh * kStates[i].ry;
                bool sel = (m_viewSelectedState == i);
                ctx.ui.drawRect({nx, ny, kNW, kNH}, sel ? 0x1E2E3EFF : ctx.kCardBg);
                ctx.ui.drawRectOutline({nx, ny, kNW, kNH},
                                       sel ? ctx.kAccentBlue : kStates[i].color, 1.f);
                ctx.ui.drawRect({nx, ny, kNW, 14.f}, kStates[i].color);
                ctx.ui.drawText(nx + 4.f, ny + 1.f, kStates[i].type, 0x888888FF);
                ctx.ui.drawText(nx + 4.f, ny + 18.f, kStates[i].name, ctx.kTextPrimary);
                if (ctx.hitRegion({nx, ny, kNW, kNH}, false))
                    m_viewSelectedState = (m_viewSelectedState == i) ? -1 : i;
            }
            // Transitions with arrow stubs
            static constexpr struct { int f; int t; const char* cond; } kTrans[] = {
                {0,1,""},            // Entry → Idle
                {1,2,"Speed > 0.5"}, // Idle → Run
                {2,1,"Speed < 0.1"}, // Run → Idle
                {1,3,"Jump"},        // Idle → Jump
                {2,3,"Jump"},        // Run → Jump
                {3,1,"Grounded"},    // Jump → Idle
                {5,4,"AttackBtn"},   // AnyState → Attack
                {4,1,"AttackEnd"},   // Attack → Idle
            };
            for (const auto& tr : kTrans) {
                float x0 = cvx + cvw * kStates[tr.f].rx + kNW;
                float y0 = cvy + cvh * kStates[tr.f].ry + kNH * 0.5f;
                float x1 = cvx + cvw * kStates[tr.t].rx;
                float y1 = cvy + cvh * kStates[tr.t].ry + kNH * 0.5f;
                float mx = (x0 + x1) * 0.5f;
                ctx.ui.drawRect({x0, y0, mx - x0, 1.f}, ctx.kTextMuted);
                if (std::abs(y1 - y0) > 1.f) {
                    float top = y0 < y1 ? y0 : y1;
                    ctx.ui.drawRect({mx, top, 1.f, std::abs(y1 - y0)}, ctx.kTextMuted);
                }
                ctx.ui.drawRect({mx, y1, x1 - mx, 1.f}, ctx.kTextMuted);
                // Condition label on midpoint
                if (tr.cond[0] && mx + 4.f < cvx + cvw - 30.f)
                    ctx.ui.drawText(mx + 4.f, (y0 + y1) * 0.5f - 6.f, tr.cond, 0x666666FF);
            }
        }

        // Right — State inspector
        ctx.drawPanel(ctx.x + nodesW + graphW, cy, propW, ch, "State");
        const float px = ctx.x + nodesW + graphW;
        if (m_viewSelectedState >= 0 && m_viewSelectedState < kStateCount) {
            ctx.ui.drawText(px + 8.f, cy + 10.f, kStates[m_viewSelectedState].name,
                            ctx.kTextPrimary);
            ctx.ui.drawRect({px + 4.f, cy + 24.f, propW - 8.f, 1.f}, ctx.kBorder);
            ctx.drawStatRow(px + 8.f, cy + 30.f, "Type:",    kStates[m_viewSelectedState].type);
            ctx.drawStatRow(px + 8.f, cy + 48.f, "Transitions:", "2");
            ctx.drawStatRow(px + 8.f, cy + 66.f, "Speed:",   "1.0x");
        } else {
            ctx.ui.drawText(px + 8.f, cy + 20.f, "Select a state", ctx.kTextMuted);
            char sc[8]; std::snprintf(sc, sizeof(sc), "%d", kStateCount);
            ctx.drawStatRow(px + 8.f, cy + 40.f, "States:", sc);
        }
    }

    // ── BEHAVIOR TREE mode (AIBehaviorTreeEditorV1) ───────────────
    else if (m_editMode == VisualLogicMode::BehaviorTree) {
        // Node types: Root/Selector/Sequence/Parallel/Decorator/Condition/Action
        static constexpr struct {
            const char* name;
            const char* type;
            uint32_t    color;
            int         depth;
            int         parent;
        } kBTNodes[] = {
            {"Root",            "Root",      0x338833FF, 0, -1},
            {"Selector",        "Selector",  0xCC8833FF, 1,  0},
            {"Idle Seq",        "Sequence",  0x4488CCFF, 2,  1},
            {"IsIdle",          "Condition", 0x886644FF, 3,  2},
            {"DoIdle",          "Action",    0x664488FF, 3,  2},
            {"Combat Selector", "Selector",  0xCC8833FF, 2,  1},
            {"HasTarget",       "Condition", 0x886644FF, 3,  5},
            {"AttackAction",    "Action",    0x664488FF, 3,  5},
            {"Flee",            "Action",    0x994444FF, 3,  5},
        };
        static constexpr int kBTCount = 9;

        // Blackboard keys (AIBlackboardEditorV1)
        static constexpr struct { const char* key; const char* type; const char* value; }
        kBBKeys[] = {
            {"Target",    "Entity",  "None"},
            {"IsIdle",    "Bool",    "true"},
            {"Health",    "Float",   "100.0"},
            {"AlertLevel","Int",     "0"},
            {"HomePos",   "Vector3", "0,0,0"},
        };
        static constexpr int kBBCount = 5;

        // Left — Blackboard key-value table
        ctx.drawPanel(ctx.x, cy, nodesW, ch, "Blackboard");
        {
            ctx.ui.drawText(ctx.x + 8.f, cy + 8.f, "Keys:", ctx.kTextMuted);
            float ry = cy + 26.f;
            for (int i = 0; i < kBBCount; ++i) {
                if (ry + 22.f > cy + ch - 4.f) break;
                ctx.ui.drawText(ctx.x + 8.f,  ry,      kBBKeys[i].key,   ctx.kTextSecond);
                ctx.ui.drawText(ctx.x + 8.f,  ry + 10.f, kBBKeys[i].type,  ctx.kTextMuted);
                ctx.ui.drawText(ctx.x + 8.f,  ry + 10.f, "  : ", ctx.kTextMuted);
                ctx.ui.drawText(ctx.x + 40.f, ry + 10.f, kBBKeys[i].value, ctx.kAccentBlue);
                ry += 24.f;
            }
        }

        // Centre — Tree canvas with tree layout
        ctx.drawPanel(ctx.x + nodesW, cy, graphW, ch, "Behavior Tree");
        {
            const float cvx = ctx.x + nodesW + 4.f;
            const float cvy = cy + 4.f;
            const float cvw = graphW - 8.f;
            const float cvh = ch - 8.f;
            ctx.ui.drawRect({cvx, cvy, cvw, cvh}, 0x181818FF);
            static constexpr float kNW = 110.f, kNH = 36.f;
            // Fixed horizontal spacing by depth, vertical by sibling index
            static constexpr float kDepthX[] = {0.02f, 0.24f, 0.46f, 0.70f};
            static constexpr float kSibY[] = {0.45f, 0.45f,
                                               0.20f, 0.70f,
                                               0.10f, 0.30f, 0.55f, 0.70f, 0.88f};

            for (int i = 0; i < kBTCount; ++i) {
                float nx = cvx + cvw * kDepthX[kBTNodes[i].depth];
                float ny = cvy + cvh * kSibY[i];
                bool sel = (m_viewSelectedBTNode == i);
                ctx.ui.drawRect({nx, ny, kNW, kNH}, sel ? 0x1E2E3EFF : ctx.kCardBg);
                ctx.ui.drawRectOutline({nx, ny, kNW, kNH},
                                       sel ? ctx.kAccentBlue : ctx.kBorder, 1.f);
                ctx.ui.drawRect({nx, ny, kNW, 14.f}, kBTNodes[i].color);
                ctx.ui.drawText(nx + 4.f, ny + 1.f, kBTNodes[i].type, 0x888888FF);
                ctx.ui.drawText(nx + 4.f, ny + 16.f, kBTNodes[i].name, ctx.kTextPrimary);
                // Connection line to parent
                if (kBTNodes[i].parent >= 0) {
                    float px2 = cvx + cvw * kDepthX[kBTNodes[kBTNodes[i].parent].depth] + kNW;
                    float py2 = cvy + cvh * kSibY[kBTNodes[i].parent] + kNH * 0.5f;
                    float cx2 = nx;
                    float cy2 = ny + kNH * 0.5f;
                    ctx.ui.drawRect({px2, py2, cx2 - px2, 1.f}, 0x555555FF);
                    if (std::abs(cy2 - py2) > 1.f) {
                        float top = py2 < cy2 ? py2 : cy2;
                        ctx.ui.drawRect({cx2, top, 1.f, std::abs(cy2 - py2)}, 0x555555FF);
                    }
                }
                if (ctx.hitRegion({nx, ny, kNW, kNH}, false))
                    m_viewSelectedBTNode = (m_viewSelectedBTNode == i) ? -1 : i;
            }
        }

        // Right — Node inspector
        ctx.drawPanel(ctx.x + nodesW + graphW, cy, propW, ch, "Node");
        const float px = ctx.x + nodesW + graphW;
        if (m_viewSelectedBTNode >= 0 && m_viewSelectedBTNode < kBTCount) {
            ctx.ui.drawText(px + 8.f, cy + 10.f, kBTNodes[m_viewSelectedBTNode].name,
                            ctx.kTextPrimary);
            ctx.ui.drawRect({px + 4.f, cy + 24.f, propW - 8.f, 1.f}, ctx.kBorder);
            ctx.drawStatRow(px + 8.f, cy + 30.f, "Type:", kBTNodes[m_viewSelectedBTNode].type);
            ctx.drawStatRow(px + 8.f, cy + 48.f, "Children:", "2");
            ctx.drawStatRow(px + 8.f, cy + 66.f, "Status:", "Idle");
        } else {
            ctx.ui.drawText(px + 8.f, cy + 20.f, "Select a node", ctx.kTextMuted);
            char nc[8]; std::snprintf(nc, sizeof(nc), "%d", kBTCount);
            ctx.drawStatRow(px + 8.f, cy + 40.f, "Nodes:", nc);
            ctx.drawStatRow(px + 8.f, cy + 58.f, "BB keys:", "5");
        }
    }

    // ── VFX mode (VFXGraphEditorV1) ───────────────────────────────
    else if (m_editMode == VisualLogicMode::VFX) {
        // Node types: Emitter/Force/Collision/ColorOverLife/SizeOverLife/Output
        static constexpr struct {
            const char* name;
            const char* type;
            uint32_t    headerColor;
            float       rx, ry;
        } kVFXNodes[] = {
            {"Emitter",        "Emitter",     0xCC4466FF, 0.05f, 0.40f},
            {"Sphere Shape",   "Emitter",     0xCC4466FF, 0.22f, 0.25f},
            {"Velocity Force", "Force",       0x448844FF, 0.22f, 0.60f},
            {"Color Over Life","ColorOLife",  0x884488FF, 0.50f, 0.25f},
            {"Size Over Life", "SizeOLife",   0x446688FF, 0.50f, 0.60f},
            {"Particle Output","Output",      0x225522FF, 0.78f, 0.40f},
        };
        static constexpr int kVFXCount = 6;

        // Left — Node palette
        ctx.drawPanel(ctx.x, cy, nodesW, ch, "VFX Nodes");
        {
            ctx.ui.drawText(ctx.x + 8.f, cy + 8.f, "Palette:", ctx.kTextMuted);
            float ry = cy + 26.f;
            static const char* kVFXPal[] = {"Emitter","Force","Collision","ColorOLife","SizeOLife","Noise","Output"};
            static constexpr uint32_t kVFXPalColors[] =
                {0xCC4466FF, 0x448844FF, 0x885544FF, 0x884488FF, 0x446688FF, 0x336688FF, 0x225522FF};
            for (int i = 0; i < 7; ++i) {
                if (ry + 16.f > cy + ch - 4.f) break;
                bool sel = (m_viewSelectedVFXNode == i);
                bool hov = ctx.isHovered({ctx.x + 2.f, ry, nodesW - 4.f, 14.f});
                uint32_t bg = sel ? ctx.kAccentBlue : (hov ? 0x2A2A3AFF : 0u);
                if (bg) ctx.ui.drawRect({ctx.x + 2.f, ry, nodesW - 4.f, 14.f}, bg);
                ctx.ui.drawRect({ctx.x + 6.f, ry + 4.f, 6.f, 6.f}, kVFXPalColors[i]);
                ctx.ui.drawText(ctx.x + 16.f, ry + 1.f, kVFXPal[i],
                                sel ? ctx.kTextPrimary : ctx.kTextSecond);
                if (ctx.hitRegion({ctx.x + 2.f, ry, nodesW - 4.f, 14.f}, false))
                    m_viewSelectedVFXNode = sel ? -1 : i;
                ry += 16.f;
            }
        }

        // Centre — VFX graph canvas
        ctx.drawPanel(ctx.x + nodesW, cy, graphW, ch, "VFX Graph");
        {
            const float cvx = ctx.x + nodesW + 4.f;
            const float cvy = cy + 4.f;
            const float cvw = graphW - 8.f;
            const float cvh = ch - 8.f;
            ctx.ui.drawRect({cvx, cvy, cvw, cvh}, 0x181818FF);
            for (float gx = cvx; gx < cvx + cvw; gx += 26.f)
                for (float gy = cvy; gy < cvy + cvh; gy += 26.f)
                    ctx.ui.drawRect({gx, gy, 1.f, 1.f}, 0x282828FF);

            static constexpr float kNW = 110.f, kNH = 50.f;
            for (int i = 0; i < kVFXCount; ++i) {
                float nx = cvx + cvw * kVFXNodes[i].rx;
                float ny = cvy + cvh * kVFXNodes[i].ry;
                bool sel = (m_viewSelectedVFXNode == i);
                ctx.ui.drawRect({nx, ny, kNW, kNH}, sel ? 0x1E2E3EFF : ctx.kCardBg);
                ctx.ui.drawRectOutline({nx, ny, kNW, kNH},
                                       sel ? ctx.kAccentBlue : ctx.kBorder, 1.f);
                ctx.ui.drawRect({nx, ny, kNW, 18.f}, kVFXNodes[i].headerColor);
                ctx.ui.drawText(nx + 4.f, ny + 2.f, kVFXNodes[i].name, ctx.kTextPrimary);
                ctx.ui.drawText(nx + 4.f, ny + 12.f, kVFXNodes[i].type, 0x888888FF);
                // Pins
                ctx.ui.drawRect({nx - 5.f, ny + kNH * 0.5f, 5.f, 5.f}, 0x886644FFu);
                if (i < kVFXCount - 1)
                    ctx.ui.drawRect({nx + kNW, ny + kNH * 0.5f, 5.f, 5.f}, kVFXNodes[i].headerColor);
                if (ctx.hitRegion({nx, ny, kNW, kNH}, false))
                    m_viewSelectedVFXNode = (m_viewSelectedVFXNode == i) ? -1 : i;
            }
            // Wires
            static constexpr struct { int f; int t; } kVFXWires[] = {
                {0,3},{0,4},{1,0},{2,0},{3,5},{4,5}
            };
            for (const auto& w : kVFXWires) {
                float x0 = cvx + cvw * kVFXNodes[w.f].rx + kNW + 5.f;
                float y0 = cvy + cvh * kVFXNodes[w.f].ry + kNH * 0.5f;
                float x1 = cvx + cvw * kVFXNodes[w.t].rx - 5.f;
                float y1 = cvy + cvh * kVFXNodes[w.t].ry + kNH * 0.5f;
                if (x0 > x1) { // skip wires that go backwards for clarity
                    std::swap(x0, x1); std::swap(y0, y1);
                }
                float mx = (x0 + x1) * 0.5f;
                ctx.ui.drawRect({x0, y0, mx - x0, 1.f}, 0x555555FF);
                if (std::abs(y1 - y0) > 1.f) {
                    float top = y0 < y1 ? y0 : y1;
                    ctx.ui.drawRect({mx, top, 1.f, std::abs(y1 - y0)}, 0x555555FF);
                }
                ctx.ui.drawRect({mx, y1, x1 - mx, 1.f}, 0x555555FF);
            }
        }

        // Right — VFX node inspector
        ctx.drawPanel(ctx.x + nodesW + graphW, cy, propW, ch, "VFX Properties");
        const float px = ctx.x + nodesW + graphW;
        if (m_viewSelectedVFXNode >= 0 && m_viewSelectedVFXNode < kVFXCount) {
            ctx.ui.drawText(px + 8.f, cy + 10.f, kVFXNodes[m_viewSelectedVFXNode].name,
                            ctx.kTextPrimary);
            ctx.ui.drawRect({px + 4.f, cy + 24.f, propW - 8.f, 1.f}, ctx.kBorder);
            ctx.drawStatRow(px + 8.f, cy + 30.f, "Type:", kVFXNodes[m_viewSelectedVFXNode].type);
            ctx.drawStatRow(px + 8.f, cy + 48.f, "Rate:", "50/s");
            ctx.drawStatRow(px + 8.f, cy + 66.f, "Lifetime:", "2.0s");
        } else {
            ctx.ui.drawText(px + 8.f, cy + 20.f, "Select a node", ctx.kTextMuted);
        }
    }

    // ── DEBUG mode ────────────────────────────────────────────────
    else if (m_editMode == VisualLogicMode::Debug) {
        ctx.drawPanel(ctx.x, cy, ctx.w * 0.35f, ch, "Breakpoints");
        ctx.ui.drawText(ctx.x + 8.f, cy + 10.f, "No breakpoints set", ctx.kTextMuted);
        ctx.drawPanel(ctx.x + ctx.w * 0.35f, cy, ctx.w * 0.65f, ch, "Execution Trace");
        ctx.drawStatusPill(ctx.x + ctx.w * 0.35f + 8.f, cy + 10.f,
                           m_stats.isCompiling ? "Running" : "Stopped",
                           m_stats.isCompiling ? ctx.kGreen : ctx.kTextMuted);
        if (m_stats.errorCount > 0) {
            char eb[32]; std::snprintf(eb, sizeof(eb), "%u errors", m_stats.errorCount);
            ctx.drawStatusPill(ctx.x + ctx.w * 0.35f + 80.f, cy + 10.f, eb, ctx.kRed);
        }
    }

    // ── DIFF mode ─────────────────────────────────────────────────
    else {
        ctx.drawPanel(ctx.x, cy, ctx.w, ch, "Graph Diff");
        ctx.ui.drawText(ctx.x + 8.f, cy + 10.f, "No diff — graph matches last compile",
                        ctx.kTextMuted);
    }

    // ── Status strip (all modes) ──────────────────────────────────
    ctx.drawStatusPill(ctx.x + 4.f, ctx.y + ctx.h - 18.f,
                       visualLogicModeName(m_editMode), ctx.kAccentBlue);
    if (m_stats.isDirty)
        ctx.ui.drawText(ctx.x + 90.f, ctx.y + ctx.h - 16.f, "* unsaved", ctx.kRed);
}


void VisualLogicEditorTool::syncPanels(WorkspacePanelHost& host) const {
    (void)host;
}

} // namespace NF
