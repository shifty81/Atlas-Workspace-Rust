#pragma once
// NF::Editor — NFRenderViewport + specialized editor panels (mesh, material, skeletal, animation, prefab)
#include "NF/Core/Core.h"
#include "NF/Engine/Engine.h"
#include "NF/Renderer/Renderer.h"
#include "NF/UI/UI.h"
#include "NF/GraphVM/GraphVM.h"
#include "NF/Input/Input.h"
#include "NF/UI/UIWidgets.h"
#include "NF/Editor/Material/MaterialEditor.h"
#include <filesystem>
#include <fstream>
#include <set>
#include <map>
#include <deque>
#include <unordered_map>
#include "NF/Editor/EditorPanel.h"
#include "NF/UI/ViewportRenderMode.h"
#include "NF/Workspace/IViewportSceneProvider.h"

namespace NF {

// Forward declaration so panels can hold a WorkspaceViewportManager* without
// pulling in the full WorkspaceViewportManager.h header.
class WorkspaceViewportManager;

// ViewportRenderMode and viewportRenderModeName() are defined in NF/UI/ViewportRenderMode.h

enum class ViewportGizmoMode : uint8_t {
    None, Translate, Rotate, Scale
};

inline const char* viewportGizmoModeName(ViewportGizmoMode m) {
    switch (m) {
        case ViewportGizmoMode::None:      return "None";
        case ViewportGizmoMode::Translate: return "Translate";
        case ViewportGizmoMode::Rotate:    return "Rotate";
        case ViewportGizmoMode::Scale:     return "Scale";
    }
    return "Unknown";
}

enum class ViewportCameraMode : uint8_t {
    FPS, Orbit, Flythrough, TopDown, Cinematic
};

inline const char* viewportCameraModeName(ViewportCameraMode m) {
    switch (m) {
        case ViewportCameraMode::FPS:        return "FPS";
        case ViewportCameraMode::Orbit:      return "Orbit";
        case ViewportCameraMode::Flythrough: return "Flythrough";
        case ViewportCameraMode::TopDown:    return "TopDown";
        case ViewportCameraMode::Cinematic:  return "Cinematic";
    }
    return "Unknown";
}

class NFRenderViewport {
public:
    explicit NFRenderViewport(const std::string& name,
                              uint32_t width = 1280, uint32_t height = 720)
        : m_name(name), m_width(width), m_height(height) {}

    virtual ~NFRenderViewport() = default;

    // Camera control
    void setCameraMode(ViewportCameraMode mode)   { m_cameraMode = mode; }
    void setRenderMode(ViewportRenderMode mode)   { m_renderMode = mode; }
    void setGizmoMode(ViewportGizmoMode mode)     { m_gizmoMode = mode; }

    void setCameraPosition(float x, float y, float z) { m_camX = x; m_camY = y; m_camZ = z; }
    void setCameraTarget(float x, float y, float z)    { m_tgtX = x; m_tgtY = y; m_tgtZ = z; }
    void setCameraFOV(float fov)                       { m_fov = fov; }
    void setNearFar(float nearP, float farP)           { m_nearPlane = nearP; m_farPlane = farP; }

    void resize(uint32_t w, uint32_t h) { m_width = w; m_height = h; }

    // FPS camera: right-click WASD, Q/E vertical, Shift sprint
    void setSprintMultiplier(float mult)    { m_sprintMult = mult; }
    void setMoveSpeed(float speed)          { m_moveSpeed = speed; }
    void setLookSensitivity(float sens)     { m_lookSens = sens; }
    void setGridVisible(bool v)             { m_gridVisible = v; }
    void setGridSize(float s)               { m_gridSize = s; }

    [[nodiscard]] const std::string&   name()       const { return m_name; }
    [[nodiscard]] uint32_t             width()      const { return m_width; }
    [[nodiscard]] uint32_t             height()     const { return m_height; }
    [[nodiscard]] ViewportCameraMode   cameraMode() const { return m_cameraMode; }
    [[nodiscard]] ViewportRenderMode   renderMode() const { return m_renderMode; }
    [[nodiscard]] ViewportGizmoMode    gizmoMode()  const { return m_gizmoMode; }
    [[nodiscard]] float                fov()        const { return m_fov; }
    [[nodiscard]] float                nearPlane()  const { return m_nearPlane; }
    [[nodiscard]] float                farPlane()   const { return m_farPlane; }
    [[nodiscard]] float                moveSpeed()  const { return m_moveSpeed; }
    [[nodiscard]] float                lookSensitivity() const { return m_lookSens; }
    [[nodiscard]] float                sprintMultiplier() const { return m_sprintMult; }
    [[nodiscard]] bool                 gridVisible() const { return m_gridVisible; }
    [[nodiscard]] float                gridSize()   const { return m_gridSize; }
    [[nodiscard]] float                camX() const { return m_camX; }
    [[nodiscard]] float                camY() const { return m_camY; }
    [[nodiscard]] float                camZ() const { return m_camZ; }
    [[nodiscard]] float                tgtX() const { return m_tgtX; }
    [[nodiscard]] float                tgtY() const { return m_tgtY; }
    [[nodiscard]] float                tgtZ() const { return m_tgtZ; }

    [[nodiscard]] float aspectRatio() const {
        return m_height > 0 ? static_cast<float>(m_width) / static_cast<float>(m_height) : 1.0f;
    }
    [[nodiscard]] bool isGizmoActive() const { return m_gizmoMode != ViewportGizmoMode::None; }

    // Frame counter for rendering tick
    void tick() { ++m_frameCount; }
    [[nodiscard]] uint64_t frameCount() const { return m_frameCount; }

protected:
    std::string         m_name;
    uint32_t            m_width      = 1280;
    uint32_t            m_height     = 720;
    ViewportCameraMode  m_cameraMode = ViewportCameraMode::FPS;
    ViewportRenderMode  m_renderMode = ViewportRenderMode::Lit;
    ViewportGizmoMode   m_gizmoMode  = ViewportGizmoMode::None;
    float               m_fov        = 60.0f;
    float               m_nearPlane  = 0.1f;
    float               m_farPlane   = 1000.0f;
    float               m_moveSpeed  = 5.0f;
    float               m_lookSens   = 0.15f;
    float               m_sprintMult = 2.5f;
    bool                m_gridVisible = true;
    float               m_gridSize    = 1.0f;
    float               m_camX = 0.0f, m_camY = 5.0f, m_camZ = 10.0f;
    float               m_tgtX = 0.0f, m_tgtY = 0.0f, m_tgtZ = 0.0f;
    uint64_t            m_frameCount = 0;
};

// ── M1-C — MeshViewerPanel ────────────────────────────────────────

enum class MeshDisplayMode : uint8_t {
    Wireframe, Solid, Lit, UV, Normals
};

inline const char* meshDisplayModeName(MeshDisplayMode m) {
    switch (m) {
        case MeshDisplayMode::Wireframe: return "Wireframe";
        case MeshDisplayMode::Solid:     return "Solid";
        case MeshDisplayMode::Lit:       return "Lit";
        case MeshDisplayMode::UV:        return "UV";
        case MeshDisplayMode::Normals:   return "Normals";
    }
    return "Unknown";
}

struct MeshViewerAsset {
    std::string name;
    uint32_t    vertexCount   = 0;
    uint32_t    triangleCount = 0;
    uint32_t    lodCount      = 1;
    bool        hasNormals    = true;
    bool        hasUVs        = true;
    bool        loaded        = false;

    [[nodiscard]] bool isHighPoly()  const { return triangleCount >= 100000; }
    [[nodiscard]] bool isMultiLOD()  const { return lodCount > 1; }
    [[nodiscard]] bool isComplete()  const { return hasNormals && hasUVs && loaded; }
};

class MeshViewerPanel : public NFRenderViewport, public IViewportSceneProvider {
public:
    static constexpr size_t MAX_MESHES = 256;

    MeshViewerPanel() : NFRenderViewport("MeshViewer") {
        m_cameraMode = ViewportCameraMode::Orbit;
    }

    bool addMesh(const MeshViewerAsset& mesh) {
        if (m_meshes.size() >= MAX_MESHES) return false;
        for (auto& m : m_meshes) if (m.name == mesh.name) return false;
        m_meshes.push_back(mesh);
        return true;
    }

    bool removeMesh(const std::string& name) {
        for (auto it = m_meshes.begin(); it != m_meshes.end(); ++it) {
            if (it->name == name) {
                if (m_activeMesh == name) m_activeMesh.clear();
                m_meshes.erase(it);
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] MeshViewerAsset* findMesh(const std::string& name) {
        for (auto& m : m_meshes) if (m.name == name) return &m;
        return nullptr;
    }

    bool setActiveMesh(const std::string& name) {
        for (auto& m : m_meshes)
            if (m.name == name) { m_activeMesh = name; return true; }
        return false;
    }

    void setDisplayMode(MeshDisplayMode dm) { m_displayMode = dm; }

    [[nodiscard]] MeshDisplayMode         displayMode()  const { return m_displayMode; }
    [[nodiscard]] const std::string&      activeMesh()   const { return m_activeMesh; }
    [[nodiscard]] size_t                  meshCount()    const { return m_meshes.size(); }

    [[nodiscard]] size_t highPolyCount() const {
        size_t n = 0; for (auto& m : m_meshes) if (m.isHighPoly()) ++n; return n;
    }
    [[nodiscard]] size_t loadedCount() const {
        size_t n = 0; for (auto& m : m_meshes) if (m.loaded) ++n; return n;
    }
    [[nodiscard]] size_t multiLODCount() const {
        size_t n = 0; for (auto& m : m_meshes) if (m.isMultiLOD()) ++n; return n;
    }
    [[nodiscard]] size_t completeCount() const {
        size_t n = 0; for (auto& m : m_meshes) if (m.isComplete()) ++n; return n;
    }

    // ── IViewportSceneProvider ────────────────────────────────────────────
    ViewportSceneState provideScene(ViewportHandle /*handle*/,
                                    const ViewportSlot& /*slot*/) override {
        ViewportSceneState st;
        st.hasContent  = !m_activeMesh.empty();
        st.entityCount = static_cast<uint32_t>(m_meshes.size());
        return st;
    }

    void attachViewportManager(WorkspaceViewportManager* mgr) { m_viewportMgr = mgr; }
    [[nodiscard]] ViewportHandle viewportHandle() const { return m_viewportHandle; }

private:
    std::vector<MeshViewerAsset> m_meshes;
    std::string                  m_activeMesh;
    MeshDisplayMode              m_displayMode = MeshDisplayMode::Lit;
    WorkspaceViewportManager*    m_viewportMgr    = nullptr;
    ViewportHandle               m_viewportHandle = kInvalidViewportHandle;
};

// ── M1-C — MaterialEditorPanel (enhanced with NFRenderViewport) ──
//
// DEPRECATED: Use MaterialEditorTool + NovaForgeMaterialPreview instead.
// This panel is retained only to avoid breaking compile dependencies.
// It will be removed in a follow-up cleanup pass.
#pragma message("MaterialEditorPanel in AdvancedViewports.h is deprecated. Use MaterialEditorTool + NovaForgeMaterialPreview.")

enum class MaterialPreviewShape : uint8_t {
    Sphere, Cube, Cylinder, Plane, Custom
};

inline const char* materialPreviewShapeName(MaterialPreviewShape s) {
    switch (s) {
        case MaterialPreviewShape::Sphere:   return "Sphere";
        case MaterialPreviewShape::Cube:     return "Cube";
        case MaterialPreviewShape::Cylinder: return "Cylinder";
        case MaterialPreviewShape::Plane:    return "Plane";
        case MaterialPreviewShape::Custom:   return "Custom";
    }
    return "Unknown";
}

class MaterialEditorPanel : public NFRenderViewport, public IViewportSceneProvider {
public:
    static constexpr size_t MAX_MATERIALS = 128;

    MaterialEditorPanel() : NFRenderViewport("MaterialEditor") {}

    bool addMaterial(const MaterialAsset& mat) {
        if (m_materials.size() >= MAX_MATERIALS) return false;
        for (auto& m : m_materials) if (m.name() == mat.name()) return false;
        m_materials.push_back(mat);
        return true;
    }

    bool removeMaterial(const std::string& name) {
        for (auto it = m_materials.begin(); it != m_materials.end(); ++it) {
            if (it->name() == name) {
                if (m_activeMaterial == name) m_activeMaterial.clear();
                m_materials.erase(it);
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] MaterialAsset* findMaterial(const std::string& name) {
        for (auto& m : m_materials) if (m.name() == name) return &m;
        return nullptr;
    }

    bool setActiveMaterial(const std::string& name) {
        for (auto& m : m_materials)
            if (m.name() == name) { m_activeMaterial = name; return true; }
        return false;
    }

    void setPreviewShape(MaterialPreviewShape s) { m_previewShape = s; }
    void setAutoRecompile(bool v)                { m_autoRecompile = v; }
    void setLivePreview(bool v)                  { m_livePreview = v; }

    [[nodiscard]] MaterialPreviewShape  previewShape()   const { return m_previewShape; }
    [[nodiscard]] bool                  autoRecompile()  const { return m_autoRecompile; }
    [[nodiscard]] bool                  livePreview()    const { return m_livePreview; }
    [[nodiscard]] const std::string&    activeMaterial() const { return m_activeMaterial; }
    [[nodiscard]] size_t                materialCount()  const { return m_materials.size(); }

    [[nodiscard]] size_t dirtyCount() const {
        size_t n = 0; for (auto& m : m_materials) if (m.isDirty()) ++n; return n;
    }
    [[nodiscard]] size_t pbrCount() const {
        size_t n = 0; for (auto& m : m_materials)
            if (m.shadingModel() == MaterialShadingModel::PBR) ++n;
        return n;
    }

    // ── IViewportSceneProvider ────────────────────────────────────────────
    // Delegates to the authoritative MaterialEditorTool provider when one is
    // set; falls back to the local material list state.
    ViewportSceneState provideScene(ViewportHandle /*handle*/,
                                    const ViewportSlot& /*slot*/) override {
        ViewportSceneState st;
        st.hasContent  = !m_activeMaterial.empty();
        st.entityCount = static_cast<uint32_t>(m_materials.size());
        return st;
    }

    void attachViewportManager(WorkspaceViewportManager* mgr) { m_viewportMgr = mgr; }
    [[nodiscard]] ViewportHandle viewportHandle() const { return m_viewportHandle; }

private:
    std::vector<MaterialAsset>  m_materials;
    std::string                 m_activeMaterial;
    MaterialPreviewShape        m_previewShape  = MaterialPreviewShape::Sphere;
    bool                        m_autoRecompile = true;
    bool                        m_livePreview   = true;
    WorkspaceViewportManager*   m_viewportMgr    = nullptr;
    ViewportHandle              m_viewportHandle = kInvalidViewportHandle;
};

// ── M1-C — SkeletalEditorPanel ────────────────────────────────────

enum class BoneDisplayMode : uint8_t {
    Lines, Octahedral, Stick, BBone, Envelope
};

inline const char* boneDisplayModeName(BoneDisplayMode m) {
    switch (m) {
        case BoneDisplayMode::Lines:      return "Lines";
        case BoneDisplayMode::Octahedral: return "Octahedral";
        case BoneDisplayMode::Stick:      return "Stick";
        case BoneDisplayMode::BBone:      return "BBone";
        case BoneDisplayMode::Envelope:   return "Envelope";
    }
    return "Unknown";
}

enum class WeightPaintMode : uint8_t {
    Off, Add, Subtract, Smooth, Replace, Blur
};

inline const char* weightPaintModeName(WeightPaintMode m) {
    switch (m) {
        case WeightPaintMode::Off:       return "Off";
        case WeightPaintMode::Add:       return "Add";
        case WeightPaintMode::Subtract:  return "Subtract";
        case WeightPaintMode::Smooth:    return "Smooth";
        case WeightPaintMode::Replace:   return "Replace";
        case WeightPaintMode::Blur:      return "Blur";
    }
    return "Unknown";
}

struct SkeletalBone {
    std::string name;
    int32_t     parentIndex = -1;
    float       length      = 1.0f;
    bool        selected    = false;
    bool        locked      = false;

    [[nodiscard]] bool isRoot() const { return parentIndex < 0; }
};

struct SkeletalAsset {
    std::string name;
    std::vector<SkeletalBone> bones;
    bool        loaded = false;
    bool        dirty  = false;

    explicit SkeletalAsset(const std::string& n) : name(n) {}

    bool addBone(const SkeletalBone& bone) {
        for (auto& b : bones) if (b.name == bone.name) return false;
        bones.push_back(bone);
        return true;
    }

    bool removeBone(const std::string& n) {
        for (auto it = bones.begin(); it != bones.end(); ++it) {
            if (it->name == n) { bones.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] SkeletalBone* findBone(const std::string& n) {
        for (auto& b : bones) if (b.name == n) return &b;
        return nullptr;
    }

    [[nodiscard]] size_t boneCount()     const { return bones.size(); }
    [[nodiscard]] size_t selectedCount() const {
        size_t c = 0; for (auto& b : bones) if (b.selected) ++c; return c;
    }
    [[nodiscard]] size_t rootCount() const {
        size_t c = 0; for (auto& b : bones) if (b.isRoot()) ++c; return c;
    }
    [[nodiscard]] size_t lockedCount() const {
        size_t c = 0; for (auto& b : bones) if (b.locked) ++c; return c;
    }
    [[nodiscard]] bool isComplex() const { return bones.size() >= 50; }
};

class SkeletalEditorPanel : public NFRenderViewport, public IViewportSceneProvider {
public:
    static constexpr size_t MAX_SKELETONS = 128;

    SkeletalEditorPanel() : NFRenderViewport("SkeletalEditor") {}

    bool addSkeleton(const SkeletalAsset& skel) {
        if (m_skeletons.size() >= MAX_SKELETONS) return false;
        for (auto& s : m_skeletons) if (s.name == skel.name) return false;
        m_skeletons.push_back(skel);
        return true;
    }

    bool removeSkeleton(const std::string& name) {
        for (auto it = m_skeletons.begin(); it != m_skeletons.end(); ++it) {
            if (it->name == name) {
                if (m_activeSkeleton == name) m_activeSkeleton.clear();
                m_skeletons.erase(it);
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] SkeletalAsset* findSkeleton(const std::string& name) {
        for (auto& s : m_skeletons) if (s.name == name) return &s;
        return nullptr;
    }

    bool setActiveSkeleton(const std::string& name) {
        for (auto& s : m_skeletons)
            if (s.name == name) { m_activeSkeleton = name; return true; }
        return false;
    }

    void openAsset(const std::string& name) { setActiveSkeleton(name); }

    void setBoneDisplayMode(BoneDisplayMode m) { m_boneDisplay = m; }
    void setWeightPaintMode(WeightPaintMode m) { m_weightPaint = m; }
    void setBrushRadius(float r)               { m_brushRadius = r; }
    void setBrushStrength(float s)             { m_brushStrength = s; }

    [[nodiscard]] BoneDisplayMode     boneDisplayMode()  const { return m_boneDisplay; }
    [[nodiscard]] WeightPaintMode     weightPaintMode()  const { return m_weightPaint; }
    [[nodiscard]] float               brushRadius()      const { return m_brushRadius; }
    [[nodiscard]] float               brushStrength()    const { return m_brushStrength; }
    [[nodiscard]] const std::string&  activeSkeleton()   const { return m_activeSkeleton; }
    [[nodiscard]] size_t              skeletonCount()    const { return m_skeletons.size(); }
    [[nodiscard]] bool                isPainting()       const { return m_weightPaint != WeightPaintMode::Off; }

    [[nodiscard]] size_t dirtyCount() const {
        size_t n = 0; for (auto& s : m_skeletons) if (s.dirty) ++n; return n;
    }
    [[nodiscard]] size_t loadedCount() const {
        size_t n = 0; for (auto& s : m_skeletons) if (s.loaded) ++n; return n;
    }
    [[nodiscard]] size_t complexCount() const {
        size_t n = 0; for (auto& s : m_skeletons) if (s.isComplex()) ++n; return n;
    }

    // ── IViewportSceneProvider ────────────────────────────────────────────
    ViewportSceneState provideScene(ViewportHandle /*handle*/,
                                    const ViewportSlot& /*slot*/) override {
        ViewportSceneState st;
        st.hasContent  = !m_activeSkeleton.empty();
        st.entityCount = static_cast<uint32_t>(m_skeletons.size());
        return st;
    }

    void attachViewportManager(WorkspaceViewportManager* mgr) { m_viewportMgr = mgr; }
    [[nodiscard]] ViewportHandle viewportHandle() const { return m_viewportHandle; }

private:
    std::vector<SkeletalAsset> m_skeletons;
    std::string                m_activeSkeleton;
    BoneDisplayMode            m_boneDisplay   = BoneDisplayMode::Octahedral;
    WeightPaintMode            m_weightPaint   = WeightPaintMode::Off;
    float                      m_brushRadius   = 5.0f;
    float                      m_brushStrength = 0.5f;
    WorkspaceViewportManager*  m_viewportMgr    = nullptr;
    ViewportHandle             m_viewportHandle = kInvalidViewportHandle;
};

// ── M1-C — AnimationEditorPanel ───────────────────────────────────

enum class AnimPlaybackState : uint8_t {
    Stopped, Playing, Paused, Looping, Reverse
};

inline const char* animPlaybackStateName(AnimPlaybackState s) {
    switch (s) {
        case AnimPlaybackState::Stopped: return "Stopped";
        case AnimPlaybackState::Playing: return "Playing";
        case AnimPlaybackState::Paused:  return "Paused";
        case AnimPlaybackState::Looping: return "Looping";
        case AnimPlaybackState::Reverse: return "Reverse";
    }
    return "Unknown";
}

enum class AnimBlendTreeType : uint8_t {
    Simple, Additive, Layered, Override, Masked
};

inline const char* animBlendTreeTypeName(AnimBlendTreeType t) {
    switch (t) {
        case AnimBlendTreeType::Simple:   return "Simple";
        case AnimBlendTreeType::Additive: return "Additive";
        case AnimBlendTreeType::Layered:  return "Layered";
        case AnimBlendTreeType::Override: return "Override";
        case AnimBlendTreeType::Masked:   return "Masked";
    }
    return "Unknown";
}

struct AnimClipAsset {
    std::string name;
    float       duration    = 0.0f;
    float       frameRate   = 30.0f;
    uint32_t    keyframeCount = 0;
    bool        looping     = false;
    bool        dirty       = false;
    bool        loaded      = false;
    AnimBlendTreeType blendType = AnimBlendTreeType::Simple;

    explicit AnimClipAsset(const std::string& n) : name(n) {}

    [[nodiscard]] uint32_t totalFrames() const {
        return static_cast<uint32_t>(duration * frameRate);
    }
    [[nodiscard]] bool isLong()   const { return duration >= 10.0f; }
    [[nodiscard]] bool isDense()  const { return keyframeCount >= 100; }
    [[nodiscard]] bool isReady()  const { return loaded && !dirty; }
};

class AnimationEditorPanel : public NFRenderViewport, public IViewportSceneProvider {
public:
    static constexpr size_t MAX_CLIPS = 256;

    AnimationEditorPanel() : NFRenderViewport("AnimationEditor") {}

    bool addClip(const AnimClipAsset& clip) {
        if (m_clips.size() >= MAX_CLIPS) return false;
        for (auto& c : m_clips) if (c.name == clip.name) return false;
        m_clips.push_back(clip);
        return true;
    }

    bool removeClip(const std::string& name) {
        for (auto it = m_clips.begin(); it != m_clips.end(); ++it) {
            if (it->name == name) {
                if (m_activeClip == name) m_activeClip.clear();
                m_clips.erase(it);
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] AnimClipAsset* findClip(const std::string& name) {
        for (auto& c : m_clips) if (c.name == name) return &c;
        return nullptr;
    }

    bool setActiveClip(const std::string& name) {
        for (auto& c : m_clips)
            if (c.name == name) { m_activeClip = name; return true; }
        return false;
    }

    void openAsset(const std::string& name) { setActiveClip(name); }

    void setPlaybackState(AnimPlaybackState s) { m_playback = s; }
    void setPlaybackSpeed(float s)             { m_playbackSpeed = s; }
    void setCurrentFrame(uint32_t f)           { m_currentFrame = f; }
    void setTimelineZoom(float z)              { m_timelineZoom = z; }
    void setOnionSkinning(bool v)              { m_onionSkinning = v; }
    void setShowBlendTree(bool v)              { m_showBlendTree = v; }

    [[nodiscard]] AnimPlaybackState  playbackState()  const { return m_playback; }
    [[nodiscard]] float              playbackSpeed()  const { return m_playbackSpeed; }
    [[nodiscard]] uint32_t           currentFrame()   const { return m_currentFrame; }
    [[nodiscard]] float              timelineZoom()   const { return m_timelineZoom; }
    [[nodiscard]] bool               onionSkinning()  const { return m_onionSkinning; }
    [[nodiscard]] bool               showBlendTree()  const { return m_showBlendTree; }
    [[nodiscard]] const std::string& activeClip()     const { return m_activeClip; }
    [[nodiscard]] size_t             clipCount()      const { return m_clips.size(); }
    [[nodiscard]] bool               isPlaying()      const {
        return m_playback == AnimPlaybackState::Playing || m_playback == AnimPlaybackState::Looping;
    }

    [[nodiscard]] size_t dirtyCount() const {
        size_t n = 0; for (auto& c : m_clips) if (c.dirty) ++n; return n;
    }
    [[nodiscard]] size_t loadedCount() const {
        size_t n = 0; for (auto& c : m_clips) if (c.loaded) ++n; return n;
    }
    [[nodiscard]] size_t loopingClipCount() const {
        size_t n = 0; for (auto& c : m_clips) if (c.looping) ++n; return n;
    }
    [[nodiscard]] size_t longClipCount() const {
        size_t n = 0; for (auto& c : m_clips) if (c.isLong()) ++n; return n;
    }
    [[nodiscard]] size_t denseClipCount() const {
        size_t n = 0; for (auto& c : m_clips) if (c.isDense()) ++n; return n;
    }
    [[nodiscard]] size_t countByBlendType(AnimBlendTreeType t) const {
        size_t n = 0; for (auto& c : m_clips) if (c.blendType == t) ++n; return n;
    }

    // ── IViewportSceneProvider ────────────────────────────────────────────
    ViewportSceneState provideScene(ViewportHandle /*handle*/,
                                    const ViewportSlot& /*slot*/) override {
        ViewportSceneState st;
        st.hasContent  = !m_activeClip.empty();
        st.entityCount = static_cast<uint32_t>(m_clips.size());
        return st;
    }

    void attachViewportManager(WorkspaceViewportManager* mgr) { m_viewportMgr = mgr; }
    [[nodiscard]] ViewportHandle viewportHandle() const { return m_viewportHandle; }

private:
    std::vector<AnimClipAsset> m_clips;
    std::string                m_activeClip;
    AnimPlaybackState          m_playback      = AnimPlaybackState::Stopped;
    float                      m_playbackSpeed = 1.0f;
    uint32_t                   m_currentFrame  = 0;
    float                      m_timelineZoom  = 1.0f;
    bool                       m_onionSkinning = false;
    bool                       m_showBlendTree = false;
    WorkspaceViewportManager*  m_viewportMgr    = nullptr;
    ViewportHandle             m_viewportHandle = kInvalidViewportHandle;
};

// ── M1-C — PrefabEditorPanel ──────────────────────────────────────

enum class PrefabEditMode : uint8_t {
    View, Place, Delete, Transform, Connect
};

inline const char* prefabEditModeName(PrefabEditMode m) {
    switch (m) {
        case PrefabEditMode::View:      return "View";
        case PrefabEditMode::Place:     return "Place";
        case PrefabEditMode::Delete:    return "Delete";
        case PrefabEditMode::Transform: return "Transform";
        case PrefabEditMode::Connect:   return "Connect";
    }
    return "Unknown";
}

struct PrefabInstance {
    std::string name;
    std::string prefabSource;
    float       posX = 0.0f, posY = 0.0f, posZ = 0.0f;
    float       scale = 1.0f;
    bool        visible   = true;
    bool        locked    = false;
    bool        overridden = false;

    [[nodiscard]] bool isModified()  const { return overridden; }
    [[nodiscard]] bool isScaled()    const { return scale != 1.0f; }
};

class PrefabEditorPanel : public NFRenderViewport, public IViewportSceneProvider {
public:
    static constexpr size_t MAX_INSTANCES = 512;

    PrefabEditorPanel() : NFRenderViewport("PrefabEditor") {}

    bool addInstance(const PrefabInstance& inst) {
        if (m_instances.size() >= MAX_INSTANCES) return false;
        for (auto& i : m_instances) if (i.name == inst.name) return false;
        m_instances.push_back(inst);
        return true;
    }

    bool removeInstance(const std::string& name) {
        for (auto it = m_instances.begin(); it != m_instances.end(); ++it) {
            if (it->name == name) {
                if (m_activeInstance == name) m_activeInstance.clear();
                m_instances.erase(it);
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] PrefabInstance* findInstance(const std::string& name) {
        for (auto& i : m_instances) if (i.name == name) return &i;
        return nullptr;
    }

    bool setActiveInstance(const std::string& name) {
        for (auto& i : m_instances)
            if (i.name == name) { m_activeInstance = name; return true; }
        return false;
    }

    void setEditMode(PrefabEditMode m)   { m_editMode = m; }
    void setSnapToGrid(bool v)           { m_snapToGrid = v; }
    void setSnapSize(float s)            { m_snapSize = s; }
    void setIsolationMode(bool v)        { m_isolationMode = v; }

    [[nodiscard]] PrefabEditMode       editMode()        const { return m_editMode; }
    [[nodiscard]] bool                 snapToGrid()      const { return m_snapToGrid; }
    [[nodiscard]] float                snapSize()        const { return m_snapSize; }
    [[nodiscard]] bool                 isolationMode()   const { return m_isolationMode; }
    [[nodiscard]] const std::string&   activeInstance()  const { return m_activeInstance; }
    [[nodiscard]] size_t               instanceCount()   const { return m_instances.size(); }

    [[nodiscard]] size_t visibleCount() const {
        size_t n = 0; for (auto& i : m_instances) if (i.visible) ++n; return n;
    }
    [[nodiscard]] size_t lockedCount() const {
        size_t n = 0; for (auto& i : m_instances) if (i.locked) ++n; return n;
    }
    [[nodiscard]] size_t overriddenCount() const {
        size_t n = 0; for (auto& i : m_instances) if (i.overridden) ++n; return n;
    }
    [[nodiscard]] size_t scaledCount() const {
        size_t n = 0; for (auto& i : m_instances) if (i.isScaled()) ++n; return n;
    }

    // ── IViewportSceneProvider ────────────────────────────────────────────
    ViewportSceneState provideScene(ViewportHandle /*handle*/,
                                    const ViewportSlot& /*slot*/) override {
        ViewportSceneState st;
        st.hasContent  = !m_activeInstance.empty();
        st.entityCount = static_cast<uint32_t>(m_instances.size());
        return st;
    }

    void attachViewportManager(WorkspaceViewportManager* mgr) { m_viewportMgr = mgr; }
    [[nodiscard]] ViewportHandle viewportHandle() const { return m_viewportHandle; }

private:
    std::vector<PrefabInstance> m_instances;
    std::string                m_activeInstance;
    PrefabEditMode             m_editMode      = PrefabEditMode::View;
    bool                       m_snapToGrid    = true;
    float                      m_snapSize      = 0.5f;
    bool                       m_isolationMode = false;
    WorkspaceViewportManager*  m_viewportMgr    = nullptr;
    ViewportHandle             m_viewportHandle = kInvalidViewportHandle;
};

} // namespace NF
