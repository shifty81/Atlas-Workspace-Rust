#pragma once
// NF::Editor — AnimationEditorTool: primary animation and sequencer authoring tool.
//
// Implements NF::IHostedTool for the Animation Editor, one of the ~10
// primary tools in the canonical workspace roster.
//
// The Animation Editor hosts:
//   Shared panels: viewport_animation, timeline, outliner, inspector, console
//   Commands:      animation.play, animation.pause, animation.stop,
//                  animation.record, animation.add_keyframe,
//                  animation.delete_keyframe, animation.export_clip
//
// See Docs/Canon/05_EDITOR_STRATEGY.md for the locked tool roster.
// See Docs/Roadmap/04_EDITOR_CONSOLIDATION.md for Phase 3 status.

#include "NF/Workspace/IHostedTool.h"
#include "NF/Workspace/IViewportSceneProvider.h"
#include "NF/Workspace/ViewportHostContract.h"
#include "NF/Workspace/WorkspaceViewportManager.h"
#include "NF/Workspace/ToolViewRenderContext.h"
#include <string>

namespace NF {

// ── Animation editing mode ────────────────────────────────────────

enum class AnimationEditMode : uint8_t {
    Timeline,    // keyframe timeline with multi-track lanes + marker bar
    Curves,      // animation curve editor (AnimationCurveEditorV1)
    Retargeting, // bone tree (SkeletonEditorV1) + IK chains + blend shapes
    Preview,     // isolated pose / clip preview
    Cinematic,   // shot list (CinematicEditorV1) + multi-track (CutsceneEditorV1)
};

inline const char* animationEditModeName(AnimationEditMode m) {
    switch (m) {
        case AnimationEditMode::Timeline:    return "Timeline";
        case AnimationEditMode::Curves:      return "Curves";
        case AnimationEditMode::Retargeting: return "Retargeting";
        case AnimationEditMode::Preview:     return "Preview";
        case AnimationEditMode::Cinematic:   return "Cinematic";
    }
    return "Unknown";
}

// ── Animation Editor statistics ───────────────────────────────────

struct AnimationEditorStats {
    float    clipDurationMs    = 0.0f; // active clip length in milliseconds
    uint32_t frameCount        = 0;    // total keyframes in clip
    uint32_t selectedBoneCount = 0;    // selected bones in the rig
    bool     isPlaying         = false;
    bool     isRecording       = false;
    bool     isDirty           = false;
};

// ── AnimationEditorTool ───────────────────────────────────────────

class AnimationEditorTool final : public IHostedTool, public IViewportSceneProvider {
public:
    static constexpr const char* kToolId = "workspace.animation_editor";

    AnimationEditorTool();
    ~AnimationEditorTool() override = default;

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

    // ── IViewportSceneProvider ────────────────────────────────────
    ViewportSceneState provideScene(ViewportHandle handle,
                                    const ViewportSlot& slot) override;

    // ── Viewport slot access ──────────────────────────────────────
    [[nodiscard]] ViewportHandle viewportHandle() const { return m_viewportHandle; }

    /// Attach the workspace viewport manager so this tool can request/release slots.
    void attachViewportManager(WorkspaceViewportManager* mgr) { m_viewportMgr = mgr; }

    /// Attach an animation preview provider so provideScene() delegates to it.
    /// Pass nullptr to detach and revert to stub state (hasContent = false).
    void attachAnimationPreviewProvider(IViewportSceneProvider* provider) {
        m_animPreviewProvider = provider;
    }

    [[nodiscard]] IViewportSceneProvider* animationPreviewProvider() const {
        return m_animPreviewProvider;
    }

    // ── Project adapter hooks ─────────────────────────────────────
    void onProjectLoaded(const std::string& projectId) override;
    void onProjectUnloaded() override;

    // ── Animation Editor interface ────────────────────────────────

    [[nodiscard]] AnimationEditMode           editMode() const { return m_editMode; }
    void                                      setEditMode(AnimationEditMode mode);

    [[nodiscard]] const AnimationEditorStats& stats()      const { return m_stats; }
    [[nodiscard]] bool                        isDirty()    const { return m_stats.isDirty; }
    [[nodiscard]] bool                        isPlaying()  const { return m_stats.isPlaying; }
    [[nodiscard]] bool                        isRecording() const { return m_stats.isRecording; }

    void markDirty();
    void clearDirty();

    void play();
    void pause();
    void stop();
    void record(bool enable);

    void setClipDurationMs(float ms);
    void setFrameCount(uint32_t count);
    void setSelectedBoneCount(uint32_t count);

    // ── Render contract ───────────────────────────────────────────
    // Renders: Skeleton/Clips | Timeline | Clip Properties — three-column layout.
    void renderToolView(const ToolViewRenderContext& ctx) const override;
    void syncPanels(WorkspacePanelHost& host) const override;

private:
    HostedToolDescriptor  m_descriptor;
    HostedToolState       m_state    = HostedToolState::Unloaded;
    mutable AnimationEditMode m_editMode = AnimationEditMode::Timeline;
    AnimationEditorStats  m_stats;
    std::string           m_activeProjectId;

    // ── Viewport provider members ─────────────────────────────────
    WorkspaceViewportManager*  m_viewportMgr         = nullptr;
    ViewportHandle             m_viewportHandle       = kInvalidViewportHandle;
    IViewportSceneProvider*    m_animPreviewProvider  = nullptr;

    // ── Mutable per-view UI state (safe from const renderToolView) ─
    mutable int  m_viewSelectedClip    = -1;
    mutable bool m_viewIsPlaying       = false;
    mutable bool m_viewIsRecording     = false;
    // Retargeting mode
    mutable int  m_viewSelectedBone    = -1;
    mutable int  m_viewSelectedIK      = -1;
    mutable int  m_viewSelectedMorph   = -1;
    // Curves mode
    mutable int  m_viewSelectedCurve   = -1;
    // Cinematic mode
    mutable int  m_viewSelectedShot    = -1;
    mutable int  m_viewSelectedTrack   = -1;

    void buildDescriptor();
};

} // namespace NF
