// NF::Editor — AnimationEditorTool implementation.
//
// Fourth real NF::IHostedTool from Phase 3 consolidation.
// Manages animation clip and sequencer authoring within AtlasWorkspace.

#include "NF/Editor/AnimationEditorTool.h"
#include "NF/Workspace/ToolViewRenderContext.h"
#include "NF/Workspace/WorkspacePanelHost.h"
#include <cmath>
#include <cstdio>
#include <cstring>

namespace NF {

AnimationEditorTool::AnimationEditorTool() {
    buildDescriptor();
}

void AnimationEditorTool::buildDescriptor() {
    m_descriptor.toolId      = kToolId;
    m_descriptor.displayName = "Animation Editor";
    m_descriptor.category    = HostedToolCategory::AnimationAuthoring;
    m_descriptor.isPrimary   = true;
    m_descriptor.acceptsProjectExtensions = false;

    m_descriptor.supportedPanels = {
        "panel.viewport_animation",
        "panel.timeline",
        "panel.outliner",
        "panel.inspector",
        "panel.console",
    };

    m_descriptor.commands = {
        "animation.play",
        "animation.pause",
        "animation.stop",
        "animation.record",
        "animation.add_keyframe",
        "animation.delete_keyframe",
        "animation.export_clip",
    };
}

bool AnimationEditorTool::initialize() {
    if (m_state != HostedToolState::Unloaded) return false;
    m_editMode = AnimationEditMode::Timeline;
    m_stats    = {};
    m_state    = HostedToolState::Ready;
    return true;
}

void AnimationEditorTool::shutdown() {
    // Release viewport slot if still held.
    if (m_viewportMgr && m_viewportHandle != kInvalidViewportHandle) {
        m_viewportMgr->unregisterSceneProvider(kToolId);
        m_viewportMgr->releaseViewport(m_viewportHandle);
        m_viewportHandle = kInvalidViewportHandle;
    }
    m_animPreviewProvider = nullptr;

    m_state           = HostedToolState::Unloaded;
    m_editMode        = AnimationEditMode::Timeline;
    m_stats           = {};
    m_activeProjectId = {};
}

void AnimationEditorTool::activate() {
    if (m_state == HostedToolState::Ready || m_state == HostedToolState::Suspended) {
        m_state = HostedToolState::Active;

        // Request a viewport slot when the tool has a viewport manager.
        if (m_viewportMgr && m_viewportHandle == kInvalidViewportHandle) {
            m_viewportHandle = m_viewportMgr->requestViewport(
                kToolId, {0.f, 0.f, 1280.f, 720.f});
            if (m_viewportHandle != kInvalidViewportHandle) {
                m_viewportMgr->registerSceneProvider(kToolId, this);
                m_viewportMgr->activateViewport(m_viewportHandle);
            }
        }
    }
}

void AnimationEditorTool::suspend() {
    if (m_state == HostedToolState::Active) {
        // Stop playback / recording when backgrounded
        m_stats.isPlaying  = false;
        m_stats.isRecording = false;

        // Release viewport slot so it can be reused by another tool.
        if (m_viewportMgr && m_viewportHandle != kInvalidViewportHandle) {
            m_viewportMgr->unregisterSceneProvider(kToolId);
            m_viewportMgr->releaseViewport(m_viewportHandle);
            m_viewportHandle = kInvalidViewportHandle;
        }

        m_state = HostedToolState::Suspended;
    }
}

void AnimationEditorTool::update(float /*dt*/) {
    // Playback cursor advancement is driven by the engine runtime, not here.
}

ViewportSceneState AnimationEditorTool::provideScene(ViewportHandle handle,
                                                      const ViewportSlot& slot) {
    // Delegate to an attached animation preview provider when available.
    if (m_animPreviewProvider) {
        return m_animPreviewProvider->provideScene(handle, slot);
    }
    // Stub: no preview runtime attached yet — return empty state.
    // A real animation preview runtime will be wired in a future pass.
    return ViewportSceneState{};
}

void AnimationEditorTool::onProjectLoaded(const std::string& projectId) {
    m_activeProjectId  = projectId;
    m_stats            = {};
}

void AnimationEditorTool::onProjectUnloaded() {
    m_activeProjectId  = {};
    m_stats            = {};
}

void AnimationEditorTool::setEditMode(AnimationEditMode mode) {
    m_editMode = mode;
}

void AnimationEditorTool::markDirty() {
    m_stats.isDirty = true;
}

void AnimationEditorTool::clearDirty() {
    m_stats.isDirty = false;
}

void AnimationEditorTool::play() {
    m_stats.isPlaying = true;
}

void AnimationEditorTool::pause() {
    m_stats.isPlaying = false;
}

void AnimationEditorTool::stop() {
    m_stats.isPlaying  = false;
    m_stats.isRecording = false;
}

void AnimationEditorTool::record(bool enable) {
    m_stats.isRecording = enable;
}

void AnimationEditorTool::setClipDurationMs(float ms) {
    m_stats.clipDurationMs = ms;
}

void AnimationEditorTool::setFrameCount(uint32_t count) {
    m_stats.frameCount = count;
}

void AnimationEditorTool::setSelectedBoneCount(uint32_t count) {
    m_stats.selectedBoneCount = count;
}

void AnimationEditorTool::renderToolView(const ToolViewRenderContext& ctx) const {
    // ── Mode tab strip at the top ─────────────────────────────────
    // Allows switching between Timeline / Curves / Retargeting / Cinematic / Preview.
    static constexpr struct { const char* label; AnimationEditMode mode; } kModeTabs[] = {
        {"Timeline",    AnimationEditMode::Timeline},
        {"Curves",      AnimationEditMode::Curves},
        {"Retargeting", AnimationEditMode::Retargeting},
        {"Cinematic",   AnimationEditMode::Cinematic},
        {"Preview",     AnimationEditMode::Preview},
    };
    {
        float tx = ctx.x + 4.f;
        float ty = ctx.y + 2.f;
        for (const auto& tab : kModeTabs) {
            bool active = (m_editMode == tab.mode);
            float tw = static_cast<float>(std::strlen(tab.label)) * 7.f + 16.f;
            uint32_t bg = active ? ctx.kAccentBlue : 0x2A2A2AFF;
            if (ctx.drawButton(tx, ty, tw, 18.f, tab.label, bg))
                m_editMode = tab.mode;
            tx += tw + 2.f;
        }
    }

    // Layout constants — left panel 20%, centre 55%, right 25%.
    const float leftW   = ctx.w * 0.20f;
    const float centreW = ctx.w * 0.55f;
    const float rightW  = ctx.w - leftW - centreW;
    // Content area starts below the tab strip.
    const float cy = ctx.y + 22.f;
    const float ch = ctx.h - 22.f;

    // ── TIMELINE mode ─────────────────────────────────────────────
    if (m_editMode == AnimationEditMode::Timeline) {
        static const char* kClipNames[] = {"Idle", "Run", "Jump", "Attack", "Death"};
        static constexpr int kClipCount = 5;

        // Track types from SequencerTrackV1 (Animation/Audio/Event/Property/Camera/FX)
        static constexpr struct { const char* name; uint32_t color; } kTrackTypes[] = {
            {"Animation", 0x4488CCFF},
            {"Audio",     0x44AA66FF},
            {"Event",     0xCC8844FF},
            {"Property",  0x9966BBFF},
            {"Camera",    0x44AACCFF},
            {"FX",        0xCC4466FF},
        };
        static constexpr int kTrackCount = 6;

        // Left — Clip list
        ctx.drawPanel(ctx.x, cy, leftW, ch, "Clips");
        {
            ctx.ui.drawText(ctx.x + 8.f, cy + 10.f, "Clips:", ctx.kTextMuted);
            float ry = cy + 28.f;
            for (int i = 0; i < kClipCount; ++i) {
                if (ry + 18.f > cy + ch - 4.f) break;
                bool sel = (m_viewSelectedClip == i);
                bool hov = ctx.isHovered({ctx.x + 2.f, ry, leftW - 4.f, 16.f});
                uint32_t bg = sel ? ctx.kAccentBlue : (hov ? 0x2A2A3AFF : 0u);
                if (bg) ctx.ui.drawRect({ctx.x + 2.f, ry, leftW - 4.f, 16.f}, bg);
                ctx.ui.drawText(ctx.x + 8.f, ry + 1.f, kClipNames[i],
                                sel ? ctx.kTextPrimary : ctx.kTextSecond);
                if (ctx.hitRegion({ctx.x + 2.f, ry, leftW - 4.f, 16.f}, false))
                    m_viewSelectedClip = sel ? -1 : i;
                ry += 18.f;
            }
        }

        // Centre — Timeline with marker bar + multi-track lanes
        ctx.drawPanel(ctx.x + leftW, cy, centreW, ch, "Timeline");
        {
            float bx = ctx.x + leftW + 8.f;
            float by = cy + 8.f;
            if (ctx.drawButton(bx, by, 40.f, 16.f,
                               m_viewIsPlaying ? "Pause" : "Play",
                               m_viewIsPlaying ? ctx.kGreen : ctx.kButtonBg))
                m_viewIsPlaying = !m_viewIsPlaying;
            if (ctx.drawButton(bx + 44.f, by, 32.f, 16.f, "Stop")) {
                m_viewIsPlaying   = false;
                m_viewIsRecording = false;
            }
            uint32_t recBg = m_viewIsRecording ? ctx.kRed : ctx.kButtonBg;
            if (ctx.drawButton(bx + 80.f, by, 32.f, 16.f, "REC", recBg))
                m_viewIsRecording = !m_viewIsRecording;
            if (m_viewIsPlaying)
                ctx.drawStatusPill(bx + 118.f, by, "PLAYING", ctx.kGreen);
            if (m_viewIsRecording)
                ctx.drawStatusPill(bx + 190.f, by, "REC", ctx.kRed);

            // Marker bar (TimelineMarkerV1: Event/Bookmark/Chapter/Warning/Note)
            static constexpr struct { float pos; const char* label; uint32_t color; } kMarkers[] = {
                {0.00f, "Start",   0x44AACCFF},
                {0.25f, "Loop In", 0x44CC44FF},
                {0.50f, "Mid",     0xCCAA44FF},
                {0.75f, "Hit",     0xCC4444FF},
                {1.00f, "End",     0x44AACCFF},
            };
            float marY = cy + 28.f;
            float rulW = centreW - 16.f;
            ctx.ui.drawRect({ctx.x + leftW + 8.f, marY, rulW, 14.f}, 0x1A1A2AFF);
            ctx.ui.drawRectOutline({ctx.x + leftW + 8.f, marY, rulW, 14.f}, ctx.kBorder, 1.f);
            for (const auto& mk : kMarkers) {
                float mx2 = ctx.x + leftW + 8.f + rulW * mk.pos;
                ctx.ui.drawRect({mx2 - 1.f, marY, 2.f, 14.f}, mk.color);
                ctx.ui.drawText(mx2 + 3.f, marY + 1.f, mk.label, mk.color);
            }

            // Ruler with tick marks
            float rulY = cy + 44.f;
            ctx.ui.drawRect({ctx.x + leftW + 8.f, rulY, rulW, 16.f}, 0x1E1E1EFF);
            ctx.ui.drawRectOutline({ctx.x + leftW + 8.f, rulY, rulW, 16.f}, ctx.kBorder, 1.f);
            for (int t = 0; t <= 10; ++t) {
                float tx2 = ctx.x + leftW + 8.f + rulW * (t / 10.f);
                ctx.ui.drawRect({tx2, rulY, 1.f, 8.f}, ctx.kTextMuted);
                if (t > 0 && t < 10) {
                    char fb[8];
                    std::snprintf(fb, sizeof(fb), "%d", t * 10);
                    ctx.ui.drawText(tx2 + 2.f, rulY + 2.f, fb, ctx.kTextMuted);
                }
            }
            // Playhead
            ctx.ui.drawRect({ctx.x + leftW + 8.f, rulY, 2.f, 16.f}, ctx.kAccentBlue);

            // Multi-track lanes (SequencerTrackV1 types)
            float laneY = rulY + 18.f;
            for (int i = 0; i < kTrackCount; ++i) {
                if (laneY + 22.f > cy + ch - 4.f) break;
                bool sel = (m_viewSelectedTrack == i);
                uint32_t laneBg = sel ? 0x252535FF : 0x202020FF;
                ctx.ui.drawRect({ctx.x + leftW + 8.f, laneY, rulW, 20.f}, laneBg);
                ctx.ui.drawRectOutline({ctx.x + leftW + 8.f, laneY, rulW, 20.f}, ctx.kBorder, 1.f);
                // Track type badge
                ctx.ui.drawRect({ctx.x + leftW + 8.f, laneY, 6.f, 20.f}, kTrackTypes[i].color);
                ctx.ui.drawText(ctx.x + leftW + 16.f, laneY + 3.f, kTrackTypes[i].name,
                                ctx.kTextSecond);
                // Clip block placeholder
                float clipX = ctx.x + leftW + 8.f + rulW * 0.05f;
                float clipW = rulW * (0.15f + i * 0.03f);
                ctx.ui.drawRect({clipX, laneY + 2.f, clipW, 16.f}, kTrackTypes[i].color);
                if (ctx.hitRegion({ctx.x + leftW + 8.f, laneY, rulW, 20.f}, false))
                    m_viewSelectedTrack = sel ? -1 : i;
                laneY += 22.f;
            }
        }

        // Right — Clip Properties
        ctx.drawPanel(ctx.x + leftW + centreW, cy, rightW, ch, "Clip Properties");
        const float px = ctx.x + leftW + centreW;
        if (m_viewSelectedClip >= 0 && m_viewSelectedClip < kClipCount) {
            ctx.ui.drawText(px + 8.f, cy + 10.f, kClipNames[m_viewSelectedClip], ctx.kTextPrimary);
            ctx.ui.drawRect({px + 4.f, cy + 24.f, rightW - 8.f, 1.f}, ctx.kBorder);
            char durBuf[32];
            std::snprintf(durBuf, sizeof(durBuf), "%.1f ms", m_stats.clipDurationMs);
            ctx.drawStatRow(px + 8.f, cy + 30.f, "Duration:", durBuf);
            char fcBuf[16];
            std::snprintf(fcBuf, sizeof(fcBuf), "%u", m_stats.frameCount);
            ctx.drawStatRow(px + 8.f, cy + 48.f, "Frames:", fcBuf);
            ctx.drawStatRow(px + 8.f, cy + 66.f, "Loop:",     "Yes");
            ctx.drawStatRow(px + 8.f, cy + 84.f, "Blend:",    "0.15");
            if (m_viewSelectedTrack >= 0 && m_viewSelectedTrack < kTrackCount) {
                ctx.ui.drawRect({px + 4.f, cy + 104.f, rightW - 8.f, 1.f}, ctx.kBorder);
                ctx.ui.drawText(px + 8.f, cy + 110.f, "Track:", ctx.kTextMuted);
                ctx.ui.drawText(px + 8.f, cy + 128.f, kTrackTypes[m_viewSelectedTrack].name,
                                ctx.kTextPrimary);
                ctx.drawStatRow(px + 8.f, cy + 146.f, "Mute:", "Off");
                ctx.drawStatRow(px + 8.f, cy + 164.f, "Solo:", "Off");
            }
        } else {
            char durBuf[32];
            std::snprintf(durBuf, sizeof(durBuf), "%.1f ms", m_stats.clipDurationMs);
            ctx.drawStatRow(px + 8.f, cy + 10.f, "Duration:", durBuf);
            char fcBuf[16];
            std::snprintf(fcBuf, sizeof(fcBuf), "%u", m_stats.frameCount);
            ctx.drawStatRow(px + 8.f, cy + 28.f, "Frames:", fcBuf);
            ctx.ui.drawText(px + 8.f, cy + 50.f, "Select a clip", ctx.kTextMuted);
        }
        if (m_stats.isDirty)
            ctx.ui.drawText(px + 8.f, cy + ch - 18.f, "* unsaved", ctx.kRed);
    }

    // ── CURVES mode ───────────────────────────────────────────────
    else if (m_editMode == AnimationEditMode::Curves) {
        // Curve types from AnimationCurveEditorV1 (AcvTangentType, AcvCurve/AcvKey)
        static const char* kCurveNames[] = {
            "Position.X", "Position.Y", "Position.Z",
            "Rotation.X", "Rotation.Y", "Rotation.Z",
            "Scale.X",
        };
        static constexpr int kCurveCount = 7;

        // Tangent types for the key selector
        static const char* kTangentTypes[] = {"Linear", "Stepped", "Bezier", "Auto"};
        static constexpr uint32_t kTangentColors[] =
            {0x4488CCFF, 0xCC4444FF, 0x44CC88FF, 0xCCAA44FF};

        // Left — Curve list
        ctx.drawPanel(ctx.x, cy, leftW, ch, "Curves");
        {
            ctx.ui.drawText(ctx.x + 8.f, cy + 10.f, "Channels:", ctx.kTextMuted);
            float ry = cy + 28.f;
            for (int i = 0; i < kCurveCount; ++i) {
                if (ry + 18.f > cy + ch - 4.f) break;
                bool sel = (m_viewSelectedCurve == i);
                bool hov = ctx.isHovered({ctx.x + 2.f, ry, leftW - 4.f, 16.f});
                uint32_t bg = sel ? ctx.kAccentBlue : (hov ? 0x2A2A3AFF : 0u);
                if (bg) ctx.ui.drawRect({ctx.x + 2.f, ry, leftW - 4.f, 16.f}, bg);
                // Color dot per curve group
                static constexpr uint32_t kCurveDots[] = {
                    0xE05050FF, 0xE05050FF, 0xE05050FF,
                    0x4EC94EFF, 0x4EC94EFF, 0x4EC94EFF,
                    0x4488CCFF,
                };
                ctx.ui.drawRect({ctx.x + 6.f, ry + 4.f, 6.f, 6.f}, kCurveDots[i]);
                ctx.ui.drawText(ctx.x + 16.f, ry + 1.f, kCurveNames[i],
                                sel ? ctx.kTextPrimary : ctx.kTextSecond);
                if (ctx.hitRegion({ctx.x + 2.f, ry, leftW - 4.f, 16.f}, false))
                    m_viewSelectedCurve = sel ? -1 : i;
                ry += 18.f;
            }
        }

        // Centre — Curve canvas with keyframe dots and tangent handles
        ctx.drawPanel(ctx.x + leftW, cy, centreW, ch, "Curve Canvas");
        {
            const float cvx = ctx.x + leftW + 8.f;
            const float cvy = cy + 10.f;
            const float cvw = centreW - 16.f;
            const float cvh = ch - 20.f;
            ctx.ui.drawRect({cvx, cvy, cvw, cvh}, 0x181818FF);
            ctx.ui.drawRectOutline({cvx, cvy, cvw, cvh}, ctx.kBorder, 1.f);

            // Grid lines
            for (int gx = 1; gx < 10; ++gx)
                ctx.ui.drawRect({cvx + cvw * (gx / 10.f), cvy, 1.f, cvh}, 0x282828FF);
            for (int gy = 1; gy < 8; ++gy)
                ctx.ui.drawRect({cvx, cvy + cvh * (gy / 8.f), cvw, 1.f}, 0x282828FF);

            // Horizontal zero-line
            ctx.ui.drawRect({cvx, cvy + cvh * 0.5f, cvw, 1.f}, 0x404040FF);

            // Draw curves only for selected channel (or all if none selected)
            // Keyframe positions (normalised 0..1 in time and value space)
            static constexpr struct { float t; float v; } kKeyframes[] = {
                {0.00f, 0.50f}, {0.20f, 0.30f}, {0.40f, 0.70f},
                {0.60f, 0.40f}, {0.80f, 0.60f}, {1.00f, 0.50f},
            };
            static constexpr int kKfCount = 6;

            auto drawCurve = [&](uint32_t color) {
                for (int i = 0; i < kKfCount - 1; ++i) {
                    float x0 = cvx + cvw * kKeyframes[i].t;
                    float y0 = cvy + cvh * kKeyframes[i].v;
                    float x1 = cvx + cvw * kKeyframes[i + 1].t;
                    float y1 = cvy + cvh * kKeyframes[i + 1].v;
                    // Approximate curve with 4 straight segments per span
                    float dx = (x1 - x0) / 4.f;
                    float dy = (y1 - y0) / 4.f;
                    for (int s = 0; s < 4; ++s) {
                        float sx = x0 + dx * s;
                        float sy = y0 + dy * s;
                        float ex2 = sx + dx;
                        float ey2 = sy + dy;
                        float len = std::sqrt((ex2 - sx) * (ex2 - sx) + (ey2 - sy) * (ey2 - sy));
                        if (len > 0.f) {
                            float angle = (ey2 - sy) / len; // rough slope
                            ctx.ui.drawRect({sx, sy - 0.5f, len, 2.f}, color);
                            (void)angle;
                        }
                    }
                }
                // Draw keyframe dots
                for (const auto& kf : kKeyframes) {
                    float kx = cvx + cvw * kf.t;
                    float ky = cvy + cvh * kf.v;
                    ctx.ui.drawRect({kx - 4.f, ky - 4.f, 8.f, 8.f}, 0x1A1A1AFF);
                    ctx.ui.drawRectOutline({kx - 4.f, ky - 4.f, 8.f, 8.f}, color, 1.f);
                    // Tangent handles for Bezier
                    ctx.ui.drawRect({kx - 16.f, ky - 1.f, 12.f, 2.f}, 0x555555FF);
                    ctx.ui.drawRect({kx + 4.f,  ky - 1.f, 12.f, 2.f}, 0x555555FF);
                }
            };

            static constexpr uint32_t kCurveColors[] = {
                0xE05050FF, 0xE07070FF, 0xE09090FF,
                0x50E050FF, 0x70E070FF, 0x90E090FF,
                0x4488CCFF,
            };
            if (m_viewSelectedCurve >= 0 && m_viewSelectedCurve < kCurveCount)
                drawCurve(kCurveColors[m_viewSelectedCurve]);
            else
                for (int i = 0; i < kCurveCount; ++i) drawCurve(kCurveColors[i]);
        }

        // Right — Key inspector
        ctx.drawPanel(ctx.x + leftW + centreW, cy, rightW, ch, "Key Inspector");
        const float px = ctx.x + leftW + centreW;
        if (m_viewSelectedCurve >= 0 && m_viewSelectedCurve < kCurveCount) {
            ctx.ui.drawText(px + 8.f, cy + 10.f, kCurveNames[m_viewSelectedCurve],
                            ctx.kTextPrimary);
            ctx.ui.drawRect({px + 4.f, cy + 24.f, rightW - 8.f, 1.f}, ctx.kBorder);
            ctx.drawStatRow(px + 8.f, cy + 30.f, "Keys:", "6");
            ctx.drawStatRow(px + 8.f, cy + 48.f, "Loop:", "Cycle");
            ctx.ui.drawText(px + 8.f, cy + 68.f, "Tangent type:", ctx.kTextMuted);
            float ty2 = cy + 84.f;
            for (int t = 0; t < 4; ++t) {
                uint32_t bg2 = (t == 3) ? ctx.kAccentBlue : ctx.kButtonBg; // Auto selected
                if (ctx.drawButton(px + 8.f, ty2, rightW - 16.f, 16.f,
                                   kTangentTypes[t], bg2))
                    (void)kTangentColors[t];
                ty2 += 20.f;
            }
        } else {
            ctx.ui.drawText(px + 8.f, cy + 20.f, "Select a curve", ctx.kTextMuted);
            ctx.drawStatRow(px + 8.f, cy + 40.f, "Channels:", "7");
            char fcBuf[16];
            std::snprintf(fcBuf, sizeof(fcBuf), "%.0f ms", m_stats.clipDurationMs);
            ctx.drawStatRow(px + 8.f, cy + 58.f, "Duration:", fcBuf);
        }
    }

    // ── RETARGETING mode ──────────────────────────────────────────
    else if (m_editMode == AnimationEditMode::Retargeting) {
        // Bone types from SkeletonEditorV1 (Root/Spine/Limb/Finger/Toe/Head/Tail/Helper)
        static constexpr struct {
            const char* name;
            int         depth;
            uint32_t    color;
        } kBones[] = {
            {"Root",          0, 0xCCAA44FF},
            {"Spine",         1, 0x44AACCFF},
            {"Chest",         2, 0x44AACCFF},
            {"UpperArm.L",    3, 0x4EC94EFF},
            {"LowerArm.L",    4, 0x4EC94EFF},
            {"Hand.L",        5, 0x88CC88FF},
            {"UpperArm.R",    3, 0xE05050FF},
            {"LowerArm.R",    4, 0xE05050FF},
            {"Hand.R",        5, 0xCC8888FF},
            {"Head",          2, 0xCCAAAAFF},
            {"UpperLeg.L",    1, 0x4EC94EFF},
            {"LowerLeg.L",    2, 0x4EC94EFF},
            {"Foot.L",        3, 0x88CC88FF},
            {"UpperLeg.R",    1, 0xE05050FF},
            {"LowerLeg.R",    2, 0xE05050FF},
            {"Foot.R",        3, 0xCC8888FF},
        };
        static constexpr int kBoneCount = 16;

        // IK chains from IKRigEditorV1 (TwoBone/FABRIK/CCD/Spline/Aim/LookAt)
        static constexpr struct { const char* name; const char* type; } kIKChains[] = {
            {"Arm.L", "TwoBone"},
            {"Arm.R", "TwoBone"},
            {"Leg.L", "TwoBone"},
            {"Leg.R", "TwoBone"},
            {"Spine", "Spline"},
            {"Head",  "LookAt"},
        };
        static constexpr int kIKCount = 6;

        // Morph targets from MorphTargetEditorV1 (Facial/Body/Corrective/Custom)
        static constexpr struct { const char* name; float weight; } kMorphs[] = {
            {"BrowUp",      0.0f},
            {"EyeClose.L",  0.0f},
            {"EyeClose.R",  0.0f},
            {"MouthOpen",   0.3f},
            {"Smile",       0.0f},
            {"BodyMuscle",  1.0f},
        };
        static constexpr int kMorphCount = 6;

        // Left — Bone hierarchy tree
        ctx.drawPanel(ctx.x, cy, leftW, ch, "Skeleton");
        {
            char boneBuf[32];
            std::snprintf(boneBuf, sizeof(boneBuf), "%u bones sel.",
                          m_stats.selectedBoneCount);
            ctx.ui.drawText(ctx.x + 8.f, cy + 8.f, boneBuf, ctx.kTextSecond);
            float ry = cy + 28.f;
            for (int i = 0; i < kBoneCount; ++i) {
                if (ry + 16.f > cy + ch - 4.f) break;
                bool sel = (m_viewSelectedBone == i);
                bool hov = ctx.isHovered({ctx.x + 2.f, ry, leftW - 4.f, 14.f});
                uint32_t bg = sel ? ctx.kAccentBlue : (hov ? 0x2A2A3AFF : 0u);
                if (bg) ctx.ui.drawRect({ctx.x + 2.f, ry, leftW - 4.f, 14.f}, bg);
                // Indented by depth
                float ix2 = ctx.x + 6.f + kBones[i].depth * 8.f;
                ctx.ui.drawRect({ix2, ry + 5.f, 4.f, 4.f}, kBones[i].color);
                ctx.ui.drawText(ix2 + 8.f, ry + 1.f, kBones[i].name,
                                sel ? ctx.kTextPrimary : ctx.kTextSecond);
                if (ctx.hitRegion({ctx.x + 2.f, ry, leftW - 4.f, 14.f}, false))
                    m_viewSelectedBone = sel ? -1 : i;
                ry += 16.f;
            }
        }

        // Centre — IK Chains (top half) + Blend Shapes / Morph Targets (bottom half)
        ctx.drawPanel(ctx.x + leftW, cy, centreW, ch, "IK & Morphs");
        {
            float midH = ch * 0.5f;
            // IK chain section header
            ctx.ui.drawText(ctx.x + leftW + 8.f, cy + 8.f, "IK Chains", ctx.kTextMuted);
            float ry = cy + 26.f;
            for (int i = 0; i < kIKCount; ++i) {
                if (ry + 18.f > cy + midH) break;
                bool sel = (m_viewSelectedIK == i);
                bool hov = ctx.isHovered({ctx.x + leftW + 2.f, ry, centreW - 4.f, 16.f});
                uint32_t bg = sel ? ctx.kAccentBlue : (hov ? 0x2A2A3AFF : 0u);
                if (bg) ctx.ui.drawRect({ctx.x + leftW + 2.f, ry, centreW - 4.f, 16.f}, bg);
                ctx.ui.drawText(ctx.x + leftW + 8.f, ry + 1.f, kIKChains[i].name,
                                sel ? ctx.kTextPrimary : ctx.kTextSecond);
                ctx.ui.drawText(ctx.x + leftW + 90.f, ry + 1.f, kIKChains[i].type,
                                ctx.kTextMuted);
                // Enable toggle dot
                ctx.ui.drawRect({ctx.x + leftW + centreW - 16.f, ry + 4.f, 8.f, 8.f},
                                sel ? ctx.kGreen : ctx.kBorder);
                if (ctx.hitRegion({ctx.x + leftW + 2.f, ry, centreW - 4.f, 16.f}, false))
                    m_viewSelectedIK = sel ? -1 : i;
                ry += 18.f;
            }

            // Divider
            ctx.ui.drawRect({ctx.x + leftW, cy + midH, centreW, 1.f}, ctx.kBorder);

            // Morph target weight sliders (MorphTargetEditorV1)
            ctx.ui.drawText(ctx.x + leftW + 8.f, cy + midH + 6.f, "Blend Shapes",
                            ctx.kTextMuted);
            float my = cy + midH + 24.f;
            for (int i = 0; i < kMorphCount; ++i) {
                if (my + 20.f > cy + ch - 4.f) break;
                bool sel = (m_viewSelectedMorph == i);
                float labelW = 80.f;
                float trackX = ctx.x + leftW + 8.f + labelW;
                float trackW = centreW - labelW - 16.f;
                ctx.ui.drawText(ctx.x + leftW + 8.f, my + 3.f, kMorphs[i].name,
                                sel ? ctx.kTextPrimary : ctx.kTextSecond);
                // Weight track background
                ctx.ui.drawRect({trackX, my + 4.f, trackW, 10.f}, 0x1A1A1AFF);
                ctx.ui.drawRectOutline({trackX, my + 4.f, trackW, 10.f}, ctx.kBorder, 1.f);
                // Fill bar
                ctx.ui.drawRect({trackX, my + 4.f, trackW * kMorphs[i].weight, 10.f},
                                sel ? ctx.kAccentBlue : 0x446688FF);
                // Handle
                float hx = trackX + trackW * kMorphs[i].weight - 4.f;
                ctx.ui.drawRect({hx, my + 2.f, 8.f, 14.f}, 0xBBBBBBFF);
                if (ctx.hitRegion({ctx.x + leftW + 2.f, my, centreW - 4.f, 18.f}, false))
                    m_viewSelectedMorph = sel ? -1 : i;
                my += 20.f;
            }
        }

        // Right — Bone / IK / Morph inspector
        ctx.drawPanel(ctx.x + leftW + centreW, cy, rightW, ch, "Properties");
        const float px = ctx.x + leftW + centreW;
        if (m_viewSelectedBone >= 0 && m_viewSelectedBone < kBoneCount) {
            ctx.ui.drawText(px + 8.f, cy + 10.f, kBones[m_viewSelectedBone].name,
                            ctx.kTextPrimary);
            ctx.ui.drawRect({px + 4.f, cy + 24.f, rightW - 8.f, 1.f}, ctx.kBorder);
            ctx.drawStatRow(px + 8.f, cy + 30.f, "Type:",  "Limb");
            ctx.drawStatRow(px + 8.f, cy + 48.f, "State:", "Normal");
            ctx.drawStatRow(px + 8.f, cy + 66.f, "Length:", "0.42");
            ctx.drawStatRow(px + 8.f, cy + 84.f, "Pos:", "0, 0, 0");
            ctx.drawStatRow(px + 8.f, cy + 102.f,"Rot:", "0, 0, 0");
        } else if (m_viewSelectedIK >= 0 && m_viewSelectedIK < kIKCount) {
            ctx.ui.drawText(px + 8.f, cy + 10.f, kIKChains[m_viewSelectedIK].name,
                            ctx.kTextPrimary);
            ctx.ui.drawRect({px + 4.f, cy + 24.f, rightW - 8.f, 1.f}, ctx.kBorder);
            ctx.drawStatRow(px + 8.f, cy + 30.f, "Solver:", kIKChains[m_viewSelectedIK].type);
            ctx.drawStatRow(px + 8.f, cy + 48.f, "Enabled:", "Yes");
            ctx.drawStatRow(px + 8.f, cy + 66.f, "Weight:", "1.0");
            ctx.drawStatRow(px + 8.f, cy + 84.f, "MaxIter:", "16");
        } else if (m_viewSelectedMorph >= 0 && m_viewSelectedMorph < kMorphCount) {
            ctx.ui.drawText(px + 8.f, cy + 10.f, kMorphs[m_viewSelectedMorph].name,
                            ctx.kTextPrimary);
            ctx.ui.drawRect({px + 4.f, cy + 24.f, rightW - 8.f, 1.f}, ctx.kBorder);
            ctx.drawStatRow(px + 8.f, cy + 30.f, "Category:", "Facial");
            ctx.drawStatRow(px + 8.f, cy + 48.f, "State:",    "Active");
            char wbuf[16];
            std::snprintf(wbuf, sizeof(wbuf), "%.2f", kMorphs[m_viewSelectedMorph].weight);
            ctx.drawStatRow(px + 8.f, cy + 66.f, "Weight:", wbuf);
        } else {
            ctx.ui.drawText(px + 8.f, cy + 20.f, "Select bone, IK,", ctx.kTextMuted);
            ctx.ui.drawText(px + 8.f, cy + 38.f, "or blend shape.", ctx.kTextMuted);
            char boneBuf[32];
            std::snprintf(boneBuf, sizeof(boneBuf), "%u", kBoneCount);
            ctx.drawStatRow(px + 8.f, cy + 60.f, "Bones:", boneBuf);
            ctx.drawStatRow(px + 8.f, cy + 78.f, "IK chains:", "6");
            ctx.drawStatRow(px + 8.f, cy + 96.f, "Morphs:", "6");
        }
    }

    // ── CINEMATIC mode ────────────────────────────────────────────
    else if (m_editMode == AnimationEditMode::Cinematic) {
        // Shot types from CinematicEditorV1 (CutScene/Gameplay/Transition/StingerCut)
        static constexpr struct {
            const char* title;
            const char* type;
            float       durationMs;
        } kShots[] = {
            {"Opening",     "CutScene",   4500.f},
            {"Gameplay_01", "Gameplay",   8200.f},
            {"Trans_01",    "Transition",  500.f},
            {"Gameplay_02", "Gameplay",   6100.f},
            {"Sting",       "StingerCut",  800.f},
            {"Closing",     "CutScene",   3200.f},
        };
        static constexpr int kShotCount = 6;

        // Track types from CutsceneEditorV1 (Camera/Actor/Audio/Event/Subtitle/VFX)
        static constexpr struct { const char* name; uint32_t color; } kCTracks[] = {
            {"Camera",   0x44AACCFF},
            {"Actor",    0x4EC94EFF},
            {"Audio",    0x9966BBFF},
            {"Event",    0xCC8844FF},
            {"Subtitle", 0x88AABBFF},
            {"VFX",      0xCC4466FF},
        };
        static constexpr int kCTrackCount = 6;

        // Left — Shot list
        ctx.drawPanel(ctx.x, cy, leftW, ch, "Shots");
        {
            ctx.ui.drawText(ctx.x + 8.f, cy + 8.f, "Sequence:", ctx.kTextMuted);
            float ry = cy + 26.f;
            for (int i = 0; i < kShotCount; ++i) {
                if (ry + 22.f > cy + ch - 4.f) break;
                bool sel = (m_viewSelectedShot == i);
                bool hov = ctx.isHovered({ctx.x + 2.f, ry, leftW - 4.f, 20.f});
                uint32_t bg = sel ? ctx.kAccentBlue : (hov ? 0x2A2A3AFF : 0u);
                if (bg) ctx.ui.drawRect({ctx.x + 2.f, ry, leftW - 4.f, 20.f}, bg);
                ctx.ui.drawText(ctx.x + 8.f, ry + 2.f, kShots[i].title,
                                sel ? ctx.kTextPrimary : ctx.kTextSecond);
                ctx.ui.drawText(ctx.x + 8.f, ry + 11.f, kShots[i].type, ctx.kTextMuted);
                if (ctx.hitRegion({ctx.x + 2.f, ry, leftW - 4.f, 20.f}, false))
                    m_viewSelectedShot = sel ? -1 : i;
                ry += 22.f;
            }
        }

        // Centre — Cinematic timeline with shot blocks + CutsceneEditorV1 tracks
        ctx.drawPanel(ctx.x + leftW, cy, centreW, ch, "Cinematic Timeline");
        {
            // Timeline ruler
            float rulY = cy + 8.f;
            float rulW = centreW - 16.f;
            ctx.ui.drawRect({ctx.x + leftW + 8.f, rulY, rulW, 14.f}, 0x1E1E1EFF);
            ctx.ui.drawRectOutline({ctx.x + leftW + 8.f, rulY, rulW, 14.f}, ctx.kBorder, 1.f);
            for (int t = 0; t <= 8; ++t) {
                float tx2 = ctx.x + leftW + 8.f + rulW * (t / 8.f);
                ctx.ui.drawRect({tx2, rulY, 1.f, 10.f}, ctx.kTextMuted);
            }
            // Shot blocks on top lane
            float totalMs = 0.f;
            for (int i = 0; i < kShotCount; ++i) totalMs += kShots[i].durationMs;
            float laneY = rulY + 16.f;
            ctx.ui.drawRect({ctx.x + leftW + 8.f, laneY, rulW, 20.f}, 0x222222FF);
            float shotX = ctx.x + leftW + 8.f;
            for (int i = 0; i < kShotCount; ++i) {
                float shotW = rulW * (kShots[i].durationMs / totalMs);
                bool sel = (m_viewSelectedShot == i);
                static constexpr uint32_t kShotColors[] = {
                    0x335588FF, 0x334455FF, 0x554433FF,
                    0x334455FF, 0x553344FF, 0x335588FF,
                };
                uint32_t bg = sel ? ctx.kAccentBlue : kShotColors[i];
                ctx.ui.drawRect({shotX, laneY + 2.f, shotW - 1.f, 16.f}, bg);
                ctx.ui.drawRectOutline({shotX, laneY + 2.f, shotW - 1.f, 16.f},
                                       ctx.kBorder, 1.f);
                if (shotW > 30.f)
                    ctx.ui.drawText(shotX + 3.f, laneY + 4.f, kShots[i].title,
                                    ctx.kTextPrimary);
                if (ctx.hitRegion({shotX, laneY + 2.f, shotW - 1.f, 16.f}, false))
                    m_viewSelectedShot = (m_viewSelectedShot == i) ? -1 : i;
                shotX += shotW;
            }

            // Cutscene track lanes below the shot track
            float trackY = laneY + 24.f;
            for (int i = 0; i < kCTrackCount; ++i) {
                if (trackY + 20.f > cy + ch - 4.f) break;
                bool sel = (m_viewSelectedTrack == i);
                ctx.ui.drawRect({ctx.x + leftW + 8.f, trackY, rulW, 18.f}, 0x202020FF);
                ctx.ui.drawRectOutline({ctx.x + leftW + 8.f, trackY, rulW, 18.f},
                                       ctx.kBorder, 1.f);
                ctx.ui.drawRect({ctx.x + leftW + 8.f, trackY, 5.f, 18.f},
                                kCTracks[i].color);
                ctx.ui.drawText(ctx.x + leftW + 15.f, trackY + 2.f, kCTracks[i].name,
                                ctx.kTextSecond);
                // Clip block
                float clipX = ctx.x + leftW + 8.f + rulW * 0.05f;
                float clipW = rulW * (0.3f + i * 0.04f);
                ctx.ui.drawRect({clipX, trackY + 2.f, clipW, 14.f}, kCTracks[i].color);
                if (ctx.hitRegion({ctx.x + leftW + 8.f, trackY, rulW, 18.f}, false))
                    m_viewSelectedTrack = sel ? -1 : i;
                trackY += 20.f;
            }
        }

        // Right — Shot properties
        ctx.drawPanel(ctx.x + leftW + centreW, cy, rightW, ch, "Shot Properties");
        const float px = ctx.x + leftW + centreW;
        if (m_viewSelectedShot >= 0 && m_viewSelectedShot < kShotCount) {
            ctx.ui.drawText(px + 8.f, cy + 10.f, kShots[m_viewSelectedShot].title,
                            ctx.kTextPrimary);
            ctx.ui.drawRect({px + 4.f, cy + 24.f, rightW - 8.f, 1.f}, ctx.kBorder);
            ctx.drawStatRow(px + 8.f, cy + 30.f, "Type:", kShots[m_viewSelectedShot].type);
            char durBuf[24];
            std::snprintf(durBuf, sizeof(durBuf), "%.0f ms",
                          kShots[m_viewSelectedShot].durationMs);
            ctx.drawStatRow(px + 8.f, cy + 48.f, "Duration:", durBuf);
            ctx.drawStatRow(px + 8.f, cy + 66.f, "Playback:", "Once");
            ctx.drawStatRow(px + 8.f, cy + 84.f, "State:", "Draft");
        } else if (m_viewSelectedTrack >= 0 && m_viewSelectedTrack < kCTrackCount) {
            ctx.ui.drawText(px + 8.f, cy + 10.f, kCTracks[m_viewSelectedTrack].name,
                            ctx.kTextPrimary);
            ctx.ui.drawRect({px + 4.f, cy + 24.f, rightW - 8.f, 1.f}, ctx.kBorder);
            ctx.drawStatRow(px + 8.f, cy + 30.f, "Mute:", "Off");
            ctx.drawStatRow(px + 8.f, cy + 48.f, "Solo:", "Off");
        } else {
            ctx.ui.drawText(px + 8.f, cy + 20.f, "Select a shot", ctx.kTextMuted);
            char shotsBuf[16];
            std::snprintf(shotsBuf, sizeof(shotsBuf), "%d", kShotCount);
            ctx.drawStatRow(px + 8.f, cy + 40.f, "Shots:", shotsBuf);
        }
        if (m_stats.isDirty)
            ctx.ui.drawText(px + 8.f, cy + ch - 18.f, "* unsaved", ctx.kRed);
    }

    // ── PREVIEW mode ──────────────────────────────────────────────
    else {
        // Simple preview layout — one wide panel with playback controls
        ctx.drawPanel(ctx.x, cy, ctx.w * 0.20f, ch, "Clips");
        {
            static const char* kClipNames[] = {"Idle", "Run", "Jump", "Attack", "Death"};
            static constexpr int kClipCount = 5;
            float ry = cy + 10.f;
            for (int i = 0; i < kClipCount; ++i) {
                if (ry + 18.f > cy + ch - 4.f) break;
                bool sel = (m_viewSelectedClip == i);
                bool hov = ctx.isHovered({ctx.x + 2.f, ry, ctx.w * 0.20f - 4.f, 16.f});
                uint32_t bg = sel ? ctx.kAccentBlue : (hov ? 0x2A2A3AFF : 0u);
                if (bg) ctx.ui.drawRect({ctx.x + 2.f, ry, ctx.w * 0.20f - 4.f, 16.f}, bg);
                ctx.ui.drawText(ctx.x + 8.f, ry + 1.f, kClipNames[i],
                                sel ? ctx.kTextPrimary : ctx.kTextSecond);
                if (ctx.hitRegion({ctx.x + 2.f, ry, ctx.w * 0.20f - 4.f, 16.f}, false))
                    m_viewSelectedClip = sel ? -1 : i;
                ry += 18.f;
            }
        }
        const float previewX = ctx.x + ctx.w * 0.20f;
        const float previewW = ctx.w * 0.80f;
        ctx.drawPanel(previewX, cy, previewW, ch, "Pose Preview");
        // Skeleton wireframe placeholder
        {
            const float cvx = previewX + previewW * 0.35f;
            const float cvy = cy + ch * 0.15f;
            const float bh  = ch * 0.70f;
            ctx.ui.drawRect({cvx - 1.f, cvy,         2.f, bh * 0.40f}, ctx.kTextMuted); // spine
            ctx.ui.drawRect({cvx - 30.f, cvy + bh * 0.12f, 60.f, 2.f}, ctx.kTextMuted); // shoulders
            ctx.ui.drawRect({cvx - 30.f, cvy + bh * 0.12f, 2.f, bh * 0.30f}, ctx.kTextMuted); // arm.L
            ctx.ui.drawRect({cvx + 28.f, cvy + bh * 0.12f, 2.f, bh * 0.30f}, ctx.kTextMuted); // arm.R
            ctx.ui.drawRect({cvx - 15.f, cvy + bh * 0.40f, 30.f, 2.f}, ctx.kTextMuted); // hips
            ctx.ui.drawRect({cvx - 15.f, cvy + bh * 0.40f, 2.f, bh * 0.45f}, ctx.kTextMuted); // leg.L
            ctx.ui.drawRect({cvx + 13.f, cvy + bh * 0.40f, 2.f, bh * 0.45f}, ctx.kTextMuted); // leg.R
            ctx.ui.drawRectOutline({cvx - 12.f, cvy - bh * 0.08f, 24.f, bh * 0.12f},
                                   ctx.kTextMuted, 1.f);
        }
        if (m_viewIsPlaying)
            ctx.drawStatusPill(previewX + 8.f, cy + ch - 22.f, "PLAYING", ctx.kGreen);
        if (ctx.drawButton(previewX + 8.f, cy + 8.f, 40.f, 16.f,
                           m_viewIsPlaying ? "Pause" : "Play",
                           m_viewIsPlaying ? ctx.kGreen : ctx.kButtonBg))
            m_viewIsPlaying = !m_viewIsPlaying;
    }

    // ── Status bar (all modes) ────────────────────────────────────
    ctx.drawStatusPill(ctx.x + 4.f, ctx.y + ctx.h - 18.f,
                       animationEditModeName(m_editMode), ctx.kAccentBlue);
    if (m_stats.isPlaying || m_viewIsPlaying)
        ctx.drawStatusPill(ctx.x + 90.f, ctx.y + ctx.h - 18.f, "PLAYING", ctx.kGreen);
    if (m_stats.isRecording || m_viewIsRecording)
        ctx.drawStatusPill(ctx.x + 160.f, ctx.y + ctx.h - 18.f, "REC", ctx.kRed);
}


void AnimationEditorTool::syncPanels(WorkspacePanelHost& host) const {
    (void)host;
}

} // namespace NF
