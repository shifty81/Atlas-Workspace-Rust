// NF::Editor — MaterialEditorTool implementation.
//
// Third real NF::IHostedTool from Phase 3 consolidation.
// Manages material and shader-graph authoring within AtlasWorkspace.

#include "NF/Editor/MaterialEditorTool.h"
#include "NF/Workspace/ToolViewRenderContext.h"
#include "NF/Workspace/WorkspacePanelHost.h"
#include "NF/Workspace/WorkspaceShell.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <string>

namespace NF {

MaterialEditorTool::MaterialEditorTool() {
    buildDescriptor();
}

void MaterialEditorTool::buildDescriptor() {
    m_descriptor.toolId      = kToolId;
    m_descriptor.displayName = "Material Editor";
    m_descriptor.category    = HostedToolCategory::AssetAuthoring;
    m_descriptor.isPrimary   = true;
    m_descriptor.acceptsProjectExtensions = false;

    m_descriptor.supportedPanels = {
        "panel.viewport_material",
        "panel.inspector",
        "panel.asset_preview",
        "panel.console",
    };

    m_descriptor.commands = {
        "material.create",
        "material.save",
        "material.set_shader",
        "material.add_texture",
        "material.duplicate",
        "material.open",
    };
}

bool MaterialEditorTool::initialize() {
    if (m_state != HostedToolState::Unloaded) return false;
    m_editMode     = MaterialEditMode::Properties;
    m_stats        = {};
    m_openAssetPath = {};
    m_state        = HostedToolState::Ready;
    return true;
}

void MaterialEditorTool::shutdown() {
    m_state           = HostedToolState::Unloaded;
    m_editMode        = MaterialEditMode::Properties;
    m_stats           = {};
    m_openAssetPath   = {};
    m_activeProjectId = {};
}

void MaterialEditorTool::activate() {
    if (m_state == HostedToolState::Ready || m_state == HostedToolState::Suspended) {
        m_state = HostedToolState::Active;
    }
}

void MaterialEditorTool::suspend() {
    if (m_state == HostedToolState::Active) {
        m_state = HostedToolState::Suspended;
    }
}

void MaterialEditorTool::update(float /*dt*/) {
    // Material editor updates are event-driven (shader recompile, live preview).
}

ViewportSceneState MaterialEditorTool::provideScene(ViewportHandle handle,
                                                     const ViewportSlot& slot) {
    if (m_materialPreviewProvider)
        return m_materialPreviewProvider->provideScene(handle, slot);

    ViewportSceneState state;
    state.hasContent  = false;
    state.entityCount = 0;
    state.clearColor  = 0x222222FFu;
    return state;
}

void MaterialEditorTool::onProjectLoaded(const std::string& projectId) {
    m_activeProjectId = projectId;
    m_stats           = {};
    m_openAssetPath   = {};
}

void MaterialEditorTool::onProjectUnloaded() {
    m_activeProjectId = {};
    m_stats           = {};
    m_openAssetPath   = {};
}

void MaterialEditorTool::setEditMode(MaterialEditMode mode) {
    m_editMode = mode;
}

void MaterialEditorTool::markDirty() {
    m_stats.isDirty = true;
}

void MaterialEditorTool::clearDirty() {
    m_stats.isDirty = false;
}

void MaterialEditorTool::setNodeCount(uint32_t count) {
    m_stats.nodeCount = count;
}

void MaterialEditorTool::setTextureSlotCount(uint32_t count) {
    m_stats.textureSlotCount = count;
}

void MaterialEditorTool::setOpenAssetPath(const std::string& path) {
    m_openAssetPath = path;
}

void MaterialEditorTool::renderToolView(const ToolViewRenderContext& ctx) const {
    // ── Mode tab strip ────────────────────────────────────────────
    static constexpr struct { const char* label; MaterialEditMode mode; } kTabs[] = {
        {"Properties",  MaterialEditMode::Properties},
        {"Node Graph",  MaterialEditMode::NodeGraph},
        {"Preview",     MaterialEditMode::Preview},
        {"Post Process",MaterialEditMode::PostProcess},
    };
    {
        float tx = ctx.x + 4.f;
        float ty = ctx.y + 2.f;
        for (const auto& tab : kTabs) {
            bool active = (m_editMode == tab.mode);
            float tw = static_cast<float>(std::strlen(tab.label)) * 7.f + 16.f;
            uint32_t bg = active ? ctx.kAccentBlue : 0x2A2A2AFF;
            if (ctx.drawButton(tx, ty, tw, 18.f, tab.label, bg))
                m_editMode = tab.mode;
            tx += tw + 2.f;
        }
    }

    const float cy = ctx.y + 22.f;
    const float ch = ctx.h - 22.f;
    const float graphW   = ctx.w * 0.35f;
    const float centreW  = ctx.w * 0.40f;
    const float propW    = ctx.w - graphW - centreW;

    // ── PROPERTIES mode ───────────────────────────────────────────
    if (m_editMode == MaterialEditMode::Properties) {
        ctx.drawPanel(ctx.x, cy, graphW, ch, "Parameters");
        {
            char nodeBuf[32];
            std::snprintf(nodeBuf, sizeof(nodeBuf), "%u nodes", m_stats.nodeCount);
            ctx.ui.drawText(ctx.x + 8.f, cy + 8.f, nodeBuf, ctx.kTextSecond);
            char texBuf[32];
            std::snprintf(texBuf, sizeof(texBuf), "%u tex slots", m_stats.textureSlotCount);
            ctx.ui.drawText(ctx.x + 8.f, cy + 26.f, texBuf, ctx.kTextSecond);
            if (m_stats.isDirty)
                ctx.drawStatusPill(ctx.x + 8.f, cy + 48.f, "unsaved", ctx.kRed);

            // Material property rows
            ctx.ui.drawText(ctx.x + 8.f, cy + 72.f, "Shader:", ctx.kTextMuted);
            ctx.ui.drawText(ctx.x + 8.f, cy + 88.f, "PBR_Standard.hlsl", ctx.kTextSecond);
            ctx.ui.drawRect({ctx.x + 4.f, cy + 104.f, graphW - 8.f, 1.f}, ctx.kBorder);

            // Float sliders (Metallic, Roughness, Opacity)
            static constexpr struct { const char* name; float value; } kFloatProps[] = {
                {"Metallic",   0.0f},
                {"Roughness",  0.5f},
                {"Opacity",    1.0f},
                {"Emissive",   0.0f},
            };
            float py = cy + 110.f;
            for (const auto& prop : kFloatProps) {
                if (py + 22.f > cy + ch - 4.f) break;
                ctx.ui.drawText(ctx.x + 8.f, py, prop.name, ctx.kTextSecond);
                float trackX = ctx.x + 8.f;
                float trackW = graphW - 16.f;
                ctx.ui.drawRect({trackX, py + 14.f, trackW, 8.f}, 0x1A1A1AFF);
                ctx.ui.drawRectOutline({trackX, py + 14.f, trackW, 8.f}, ctx.kBorder, 1.f);
                ctx.ui.drawRect({trackX, py + 14.f, trackW * prop.value, 8.f}, ctx.kAccentBlue);
                float hx = trackX + trackW * prop.value - 3.f;
                ctx.ui.drawRect({hx, py + 12.f, 6.f, 12.f}, 0xBBBBBBFF);
                py += 26.f;
            }
        }

        // Centre — Gradient editor widget (GradientEditorV1)
        ctx.drawPanel(ctx.x + graphW, cy, centreW, ch, "Gradient / Ramp");
        {
            // Color stops (GradientEditorV1: Linear/Radial type, Linear/Step/Smooth interp)
            static constexpr struct { float pos; float r; float g; float b; } kStops[] = {
                {0.00f, 0.05f, 0.05f, 0.05f},
                {0.30f, 0.15f, 0.35f, 0.65f},
                {0.65f, 0.50f, 0.75f, 0.90f},
                {1.00f, 1.00f, 1.00f, 1.00f},
            };
            static constexpr int kStopCount = 4;

            float rampX = ctx.x + graphW + 8.f;
            float rampY = cy + 14.f;
            float rampW = centreW - 16.f;
            float rampH = 40.f;

            // Ramp band — interpolated color display
            for (int px2 = 0; px2 < static_cast<int>(rampW); ++px2) {
                float t = px2 / rampW;
                // Find surrounding stops
                int s0 = 0, s1 = 1;
                for (int s = 0; s < kStopCount - 1; ++s) {
                    if (t >= kStops[s].pos && t <= kStops[s + 1].pos) {
                        s0 = s; s1 = s + 1; break;
                    }
                }
                float span = kStops[s1].pos - kStops[s0].pos;
                float alpha = (span > 0.f) ? (t - kStops[s0].pos) / span : 0.f;
                auto lerp = [](float a, float b, float f) { return a + (b - a) * f; };
                float r = lerp(kStops[s0].r, kStops[s1].r, alpha);
                float g = lerp(kStops[s0].g, kStops[s1].g, alpha);
                float bv = lerp(kStops[s0].b, kStops[s1].b, alpha);
                auto tc = [](float v) -> uint8_t {
                    return static_cast<uint8_t>(v < 0.f ? 0 : (v > 1.f ? 255 : v * 255.f));
                };
                uint32_t col = (static_cast<uint32_t>(tc(r)) << 24)
                             | (static_cast<uint32_t>(tc(g)) << 16)
                             | (static_cast<uint32_t>(tc(bv)) << 8)
                             | 0xFFu;
                ctx.ui.drawRect({rampX + px2, rampY, 1.f, rampH}, col);
            }
            ctx.ui.drawRectOutline({rampX, rampY, rampW, rampH}, ctx.kBorder, 1.f);

            // Stop handles
            for (int s = 0; s < kStopCount; ++s) {
                float hx = rampX + rampW * kStops[s].pos;
                ctx.ui.drawRect({hx - 4.f, rampY + rampH, 8.f, 10.f}, 0xBBBBBBFF);
                ctx.ui.drawRectOutline({hx - 4.f, rampY + rampH, 8.f, 10.f},
                                       ctx.kBorder, 1.f);
            }

            // Interp type buttons
            float iy = rampY + rampH + 16.f;
            ctx.ui.drawText(ctx.x + graphW + 8.f, iy, "Interpolation:", ctx.kTextMuted);
            iy += 16.f;
            static const char* kInterps[] = {"Linear", "Step", "Smooth"};
            float ibx = ctx.x + graphW + 8.f;
            for (int i = 0; i < 3; ++i) {
                float iw = static_cast<float>(std::strlen(kInterps[i])) * 7.f + 14.f;
                uint32_t bg = (i == 0) ? ctx.kAccentBlue : ctx.kButtonBg; // Linear default
                (void)ctx.drawButton(ibx, iy, iw, 16.f, kInterps[i], bg);
                ibx += iw + 4.f;
            }
        }

        // Right — Properties summary
        ctx.drawPanel(ctx.x + graphW + centreW, cy, propW, ch, "Properties");
        const float px = ctx.x + graphW + centreW;
        if (!m_openAssetPath.empty()) {
            std::string label = m_openAssetPath;
            if (label.size() > 20) label = "..." + label.substr(label.size() - 17);
            ctx.ui.drawText(px + 8.f, cy + 10.f, label.c_str(), ctx.kTextSecond);
        }
        char nb[16]; std::snprintf(nb, sizeof(nb), "%u", m_stats.nodeCount);
        ctx.drawStatRow(px + 8.f, cy + 30.f, "Nodes:", nb);
        char tb[16]; std::snprintf(tb, sizeof(tb), "%u", m_stats.textureSlotCount);
        ctx.drawStatRow(px + 8.f, cy + 48.f, "Textures:", tb);
        ctx.drawStatRow(px + 8.f, cy + 66.f, "Blend:", "Opaque");
        ctx.drawStatRow(px + 8.f, cy + 84.f, "Cull:", "Back");
        if (m_stats.isDirty)
            ctx.ui.drawText(px + 8.f, cy + ch - 18.f, "* unsaved", ctx.kRed);
    }

    // ── NODE GRAPH mode ───────────────────────────────────────────
    else if (m_editMode == MaterialEditMode::NodeGraph) {
        // Node types from MaterialNodeEditorV1 (Texture/Color/Math/Blend/Normal/Output)
        // Data types (MnvDataType): Float/Vec2/Vec3/Vec4/Sampler2D
        static constexpr struct {
            const char* name;
            const char* type;
            uint32_t    headerColor;
            float       rx, ry;  // canvas placement (normalised)
            int         inPins;
            int         outPins;
        } kNodes[] = {
            {"BaseColor",  "Texture",  0x336699FF, 0.05f, 0.15f, 0, 1},
            {"Normal Map", "Texture",  0x336699FF, 0.05f, 0.50f, 0, 1},
            {"Lerp",       "Math",     0x448844FF, 0.30f, 0.28f, 3, 1},
            {"Roughness",  "Color",    0x884488FF, 0.05f, 0.78f, 0, 1},
            {"Multiply",   "Math",     0x448844FF, 0.55f, 0.45f, 2, 1},
            {"PBR Output", "Output",   0x225522FF, 0.80f, 0.35f, 4, 0},
        };
        static constexpr int kNodeCount = 6;

        // Left — Node palette
        ctx.drawPanel(ctx.x, cy, graphW * 0.35f, ch, "Palette");
        {
            static const char* kPalette[] = {
                "Texture", "Color", "Math",
                "Blend", "Normal", "Output",
                "UV", "Fresnel",
            };
            float ry = cy + 8.f;
            for (int i = 0; i < 8; ++i) {
                if (ry + 16.f > cy + ch - 4.f) break;
                bool hov = ctx.isHovered({ctx.x + 2.f, ry, graphW * 0.35f - 4.f, 14.f});
                if (hov) ctx.ui.drawRect({ctx.x + 2.f, ry, graphW * 0.35f - 4.f, 14.f},
                                         0x2A2A3AFF);
                static constexpr uint32_t kPalColors[] = {
                    0x336699FF, 0x884488FF, 0x448844FF,
                    0x885544FF, 0x448899FF, 0x225522FF,
                    0x667733FF, 0x774455FF,
                };
                ctx.ui.drawRect({ctx.x + 6.f, ry + 4.f, 6.f, 6.f}, kPalColors[i]);
                ctx.ui.drawText(ctx.x + 16.f, ry + 1.f, kPalette[i], ctx.kTextSecond);
                ry += 16.f;
            }
        }

        // Centre — Graph canvas with typed node cards
        float canvasX = ctx.x + graphW * 0.35f;
        float canvasW = ctx.w - graphW * 0.35f - propW;
        ctx.drawPanel(canvasX, cy, canvasW, ch, "Graph Canvas");
        {
            const float cvx = canvasX + 4.f;
            const float cvy = cy + 4.f;
            const float cvw = canvasW - 8.f;
            const float cvh = ch - 8.f;
            ctx.ui.drawRect({cvx, cvy, cvw, cvh}, 0x181818FF);
            // Dot grid
            for (float gx = cvx; gx < cvx + cvw; gx += 24.f)
                for (float gy = cvy; gy < cvy + cvh; gy += 24.f)
                    ctx.ui.drawRect({gx, gy, 1.f, 1.f}, 0x2A2A2AFF);

            // Draw node cards
            static constexpr float kNodeW = 110.f;
            static constexpr float kNodeH = 70.f;
            for (int i = 0; i < kNodeCount; ++i) {
                float nx = cvx + cvw * kNodes[i].rx;
                float ny = cvy + cvh * kNodes[i].ry;
                bool sel = (m_viewSelectedNode == i);
                uint32_t cardBg = sel ? 0x1E2E3EFF : 0x222222FF;
                ctx.ui.drawRect({nx, ny, kNodeW, kNodeH}, cardBg);
                ctx.ui.drawRectOutline({nx, ny, kNodeW, kNodeH},
                                       sel ? ctx.kAccentBlue : ctx.kBorder, 1.f);
                // Header bar with type color
                ctx.ui.drawRect({nx, ny, kNodeW, 20.f}, kNodes[i].headerColor);
                ctx.ui.drawText(nx + 4.f, ny + 3.f, kNodes[i].name, ctx.kTextPrimary);
                ctx.ui.drawText(nx + 4.f, ny + 13.f, kNodes[i].type, 0xAAAAAAFF);

                // Input pins (left side) — MnvPort style
                for (int p = 0; p < kNodes[i].inPins && p < 3; ++p) {
                    float py2 = ny + 28.f + p * 14.f;
                    ctx.ui.drawRect({nx - 6.f, py2, 6.f, 6.f}, 0x886644FF);
                    ctx.ui.drawRectOutline({nx - 6.f, py2, 6.f, 6.f}, ctx.kBorder, 1.f);
                }
                // Output pins (right side)
                for (int p = 0; p < kNodes[i].outPins; ++p) {
                    float py2 = ny + 32.f;
                    ctx.ui.drawRect({nx + kNodeW, py2, 6.f, 6.f}, 0x4488CCFF);
                    ctx.ui.drawRectOutline({nx + kNodeW, py2, 6.f, 6.f}, ctx.kBorder, 1.f);
                }

                if (ctx.hitRegion({nx, ny, kNodeW, kNodeH}, false))
                    m_viewSelectedNode = (m_viewSelectedNode == i) ? -1 : i;
            }

            // Wires between nodes (simplified)
            static constexpr struct { int from; int to; } kWires[] = {
                {0, 2}, {1, 2}, {2, 4}, {3, 4}, {4, 5},
            };
            for (const auto& w : kWires) {
                float x0 = cvx + cvw * kNodes[w.from].rx + kNodeW + 6.f;
                float y0 = cvy + cvh * kNodes[w.from].ry + 32.f + 3.f;
                float x1 = cvx + cvw * kNodes[w.to].rx - 6.f;
                float y1 = cvy + cvh * kNodes[w.to].ry + 28.f + 3.f
                         + static_cast<float>(0) * 14.f;  // first in-pin
                // Horizontal segments of the bezier approximation
                float mx = (x0 + x1) * 0.5f;
                ctx.ui.drawRect({x0, y0 - 1.f, mx - x0, 2.f}, 0x555555FF);
                ctx.ui.drawRect({mx - 1.f, std::min(y0,y1) - 1.f, 2.f,
                                 std::abs(y1 - y0) + 2.f}, 0x555555FF);
                ctx.ui.drawRect({mx, y1 - 1.f, x1 - mx, 2.f}, 0x555555FF);
            }
        }

        // Right — Node inspector
        ctx.drawPanel(ctx.x + canvasX - ctx.x + canvasW, cy, propW, ch, "Node Inspector");
        const float px = ctx.x + canvasX - ctx.x + canvasW;
        if (m_viewSelectedNode >= 0 && m_viewSelectedNode < kNodeCount) {
            ctx.ui.drawText(px + 8.f, cy + 10.f, kNodes[m_viewSelectedNode].name,
                            ctx.kTextPrimary);
            ctx.ui.drawRect({px + 4.f, cy + 24.f, propW - 8.f, 1.f}, ctx.kBorder);
            ctx.drawStatRow(px + 8.f, cy + 30.f, "Type:",
                            kNodes[m_viewSelectedNode].type);
            char ibuf[8], obuf[8];
            std::snprintf(ibuf, sizeof(ibuf), "%d", kNodes[m_viewSelectedNode].inPins);
            std::snprintf(obuf, sizeof(obuf), "%d", kNodes[m_viewSelectedNode].outPins);
            ctx.drawStatRow(px + 8.f, cy + 48.f, "Inputs:",  ibuf);
            ctx.drawStatRow(px + 8.f, cy + 66.f, "Outputs:", obuf);
            ctx.drawStatRow(px + 8.f, cy + 84.f, "Preview:", "Enabled");
        } else {
            ctx.ui.drawText(px + 8.f, cy + 20.f, "Select a node", ctx.kTextMuted);
            char nb[16]; std::snprintf(nb, sizeof(nb), "%u", m_stats.nodeCount);
            ctx.drawStatRow(px + 8.f, cy + 40.f, "Total nodes:", nb);
        }
        if (m_stats.isDirty)
            ctx.ui.drawText(px + 8.f, cy + ch - 18.f, "* unsaved", ctx.kRed);
    }

    // ── PREVIEW mode ──────────────────────────────────────────────
    else if (m_editMode == MaterialEditMode::Preview) {
        const char* assetLabel = m_openAssetPath.empty() ? "No material open" : nullptr;
        ctx.drawPanel(ctx.x, cy, ctx.w, ch, "Material Preview", assetLabel);
        if (!m_openAssetPath.empty()) {
            std::string label = "Asset: " + m_openAssetPath;
            if (label.size() > 40) label = label.substr(0, 37) + "...";
            ctx.ui.drawText(ctx.x + 8.f, cy + 10.f, label.c_str(), ctx.kTextSecond);
        }
        // Sphere preview placeholder
        {
            float cx2 = ctx.x + ctx.w * 0.5f;
            float cy2 = cy + ch * 0.5f;
            float r   = std::min(ctx.w, ch) * 0.30f;
            ctx.ui.drawRectOutline({cx2 - r, cy2 - r, r * 2.f, r * 2.f}, 0x555555FF, 1.f);
            ctx.ui.drawRect({cx2 - r + 2.f, cy2 - r + 2.f, r * 2.f - 4.f, r * 2.f - 4.f},
                            0x202020FF);
            ctx.ui.drawText(cx2 - 30.f, cy2 - 7.f, "Sphere", ctx.kTextMuted);
        }
        // Preview shape options
        static const char* kShapes[] = {"Sphere", "Cube", "Plane", "Cylinder"};
        float bx = ctx.x + 8.f;
        for (int i = 0; i < 4; ++i) {
            float bw = static_cast<float>(std::strlen(kShapes[i])) * 7.f + 14.f;
            uint32_t bg = (i == 0) ? ctx.kAccentBlue : ctx.kButtonBg;
            (void)ctx.drawButton(bx, cy + 8.f, bw, 16.f, kShapes[i], bg);
            bx += bw + 4.f;
        }
    }

    // ── POST PROCESS mode ─────────────────────────────────────────
    else {
        // Effects from PostProcessEditorV1:
        // Bloom/DOF/SSAO/MotionBlur/ChromaticAberration/Vignette/ColorGrading/ToneMapping
        static constexpr struct {
            const char* name;
            bool        enabled;
            uint32_t    color;
        } kEffects[] = {
            {"Bloom",               true,  0x4488CCFF},
            {"Depth of Field",      false, 0x448844FF},
            {"SSAO",                true,  0x884488FF},
            {"Motion Blur",         false, 0x885544FF},
            {"Chromatic Aberration",false, 0xCC4466FF},
            {"Vignette",            true,  0x665533FF},
            {"Color Grading",       true,  0xCCAA44FF},
            {"Tone Mapping",        true,  0x44AACCFF},
        };
        static constexpr int kEffectCount = 8;

        // Left — Effect list
        ctx.drawPanel(ctx.x, cy, graphW, ch, "Effects");
        {
            float ry = cy + 8.f;
            for (int i = 0; i < kEffectCount; ++i) {
                if (ry + 22.f > cy + ch - 4.f) break;
                bool sel = (m_viewSelectedEffect == i);
                bool hov = ctx.isHovered({ctx.x + 2.f, ry, graphW - 4.f, 20.f});
                uint32_t bg = sel ? ctx.kAccentBlue : (hov ? 0x2A2A3AFF : 0u);
                if (bg) ctx.ui.drawRect({ctx.x + 2.f, ry, graphW - 4.f, 20.f}, bg);
                // Enable indicator
                uint32_t dot = kEffects[i].enabled ? kEffects[i].color : ctx.kBorder;
                ctx.ui.drawRect({ctx.x + 8.f, ry + 7.f, 8.f, 8.f}, dot);
                ctx.ui.drawRectOutline({ctx.x + 8.f, ry + 7.f, 8.f, 8.f}, ctx.kBorder, 1.f);
                ctx.ui.drawText(ctx.x + 20.f, ry + 3.f, kEffects[i].name,
                                sel ? ctx.kTextPrimary : ctx.kTextSecond);
                if (kEffects[i].enabled)
                    ctx.ui.drawText(ctx.x + 20.f, ry + 12.f, "Enabled", ctx.kTextMuted);
                else
                    ctx.ui.drawText(ctx.x + 20.f, ry + 12.f, "Disabled", ctx.kTextMuted);
                if (ctx.hitRegion({ctx.x + 2.f, ry, graphW - 4.f, 20.f}, false))
                    m_viewSelectedEffect = sel ? -1 : i;
                ry += 22.f;
            }
        }

        // Centre — Effect parameters
        ctx.drawPanel(ctx.x + graphW, cy, centreW, ch, "Parameters");
        if (m_viewSelectedEffect >= 0 && m_viewSelectedEffect < kEffectCount) {
            float py = cy + 8.f;
            ctx.ui.drawText(ctx.x + graphW + 8.f, py, kEffects[m_viewSelectedEffect].name,
                            ctx.kTextPrimary);
            py += 20.f;
            ctx.ui.drawRect({ctx.x + graphW + 4.f, py, centreW - 8.f, 1.f}, ctx.kBorder);
            py += 8.f;

            // Per-effect parameter sliders
            static constexpr struct {
                const char* param;
                float       value;
            } kEffectParams[8][4] = {
                {{"Threshold", 0.8f},  {"Intensity",0.4f}, {"Radius",  0.5f}, {"", 0.f}}, // Bloom
                {{"FocalDist", 0.5f},  {"FStop",    2.8f}, {"MaxBlur", 0.6f}, {"", 0.f}}, // DOF
                {{"Radius",    0.5f},  {"Bias",     0.025f},{"Power",  1.0f}, {"", 0.f}}, // SSAO
                {{"Strength",  0.5f},  {"MaxLen",   0.02f}, {"", 0.f},        {"", 0.f}}, // MBlur
                {{"Intensity", 0.3f},  {"", 0.f},           {"", 0.f},        {"", 0.f}}, // CA
                {{"Intensity", 0.4f},  {"Feather",  0.8f},  {"", 0.f},        {"", 0.f}}, // Vig
                {{"Saturation",1.0f},  {"Contrast", 1.0f},  {"Gamma", 1.0f},  {"", 0.f}}, // CG
                {{"Exposure",  1.0f},  {"Gamma",    2.2f},  {"", 0.f},        {"", 0.f}}, // TM
            };
            for (int p = 0; p < 4; ++p) {
                const auto& par = kEffectParams[m_viewSelectedEffect][p];
                if (par.param[0] == '\0') break;
                if (py + 22.f > cy + ch - 4.f) break;
                ctx.ui.drawText(ctx.x + graphW + 8.f, py, par.param, ctx.kTextSecond);
                float trackX = ctx.x + graphW + 8.f;
                float trackW = centreW - 16.f;
                ctx.ui.drawRect({trackX, py + 14.f, trackW, 8.f}, 0x1A1A1AFF);
                ctx.ui.drawRectOutline({trackX, py + 14.f, trackW, 8.f}, ctx.kBorder, 1.f);
                float fill = (par.value < 0.f ? 0.f : (par.value > 1.f ? 1.f : par.value));
                ctx.ui.drawRect({trackX, py + 14.f, trackW * fill, 8.f},
                                kEffects[m_viewSelectedEffect].color);
                ctx.ui.drawRect({trackX + trackW * fill - 3.f, py + 12.f, 6.f, 12.f},
                                0xBBBBBBFF);
                char vb[16]; std::snprintf(vb, sizeof(vb), "%.2f", par.value);
                ctx.ui.drawText(ctx.x + graphW + 8.f + trackW + 4.f, py + 12.f, vb,
                                ctx.kTextMuted);
                py += 26.f;
            }
        } else {
            ctx.ui.drawText(ctx.x + graphW + 8.f, cy + ch * 0.5f - 7.f,
                            "Select an effect", ctx.kTextMuted);
        }

        // Right — Pass order + preview
        ctx.drawPanel(ctx.x + graphW + centreW, cy, propW, ch, "Stack");
        const float px = ctx.x + graphW + centreW;
        ctx.ui.drawText(px + 8.f, cy + 8.f, "Pass order:", ctx.kTextMuted);
        float sy = cy + 28.f;
        for (int i = 0; i < kEffectCount; ++i) {
            if (!kEffects[i].enabled) continue;
            if (sy + 16.f > cy + ch - 4.f) break;
            uint32_t dot = kEffects[i].color;
            ctx.ui.drawRect({px + 8.f, sy + 4.f, 6.f, 6.f}, dot);
            ctx.ui.drawText(px + 18.f, sy, kEffects[i].name, ctx.kTextSecond);
            sy += 18.f;
        }
    }
}


void MaterialEditorTool::syncPanels(WorkspacePanelHost& host) const {
    (void)host;
}

} // namespace NF
