#pragma once
// NovaForge::NovaForgeAssetPreview — per-asset preview scene for AssetEditorTool.
//
// Implements NF::IViewportSceneProvider so AssetEditorTool can render a preview
// of the selected asset in the viewport without a full game-boot sequence.
//
// The asset is represented as a single PreviewEntity whose mesh, material, and
// attachment metadata are editable. Changes are tracked as dirty and can be
// applied (committed) or reverted.
//
// Phase D.3 — Asset Preview Viewport (collider/socket/anchor + PCG tag metadata)

#include "NF/Workspace/IViewportSceneProvider.h"
#include "NovaForge/EditorAdapter/NovaForgePreviewWorld.h"
#include "NovaForge/EditorAdapter/PCGPreviewService.h"
#include <cstdio>
#include <string>
#include <utility>
#include <vector>

namespace NovaForge {

// ── ColliderDescriptor ────────────────────────────────────────────────────────

enum class ColliderShape : uint8_t {
    Box,
    Sphere,
    Capsule,
    ConvexHull,
    TriangleMesh,
};

inline const char* colliderShapeName(ColliderShape s) {
    switch (s) {
        case ColliderShape::Box:          return "Box";
        case ColliderShape::Sphere:       return "Sphere";
        case ColliderShape::Capsule:      return "Capsule";
        case ColliderShape::ConvexHull:   return "ConvexHull";
        case ColliderShape::TriangleMesh: return "TriangleMesh";
    }
    return "Unknown";
}

struct ColliderDescriptor {
    ColliderShape shape    = ColliderShape::Box;
    PreviewVec3   extents  = {1.f, 1.f, 1.f}; ///< half-extents for Box; radius,height,0 for Capsule
    float         radius   = 0.5f;             ///< sphere/capsule radius
    bool          isTrigger = false;
    std::string   tag;                          ///< optional physics tag
};

// ── SocketDescriptor ──────────────────────────────────────────────────────────
// Named attachment socket on the asset (e.g., weapon mount, engine hardpoint).

struct SocketDescriptor {
    std::string      name;
    PreviewTransform localTransform; ///< offset from asset root
    std::string      socketType;     ///< semantic type e.g. "weapon", "engine", "cargo"
};

// ── AnchorDescriptor ──────────────────────────────────────────────────────────
// Named positional anchor for docking/placement (e.g., dock approach point).

struct AnchorDescriptor {
    std::string      name;
    PreviewTransform localTransform;
    std::string      anchorType;     ///< e.g. "dock", "spawn", "interact"
};

// ── AssetPCGMetadata ──────────────────────────────────────────────────────────
// PCG placement tags and generation constraints attached to the asset.

struct AssetPCGMetadata {
    std::string              placementTag;    ///< tag used by PCG rules to select this asset
    std::vector<std::string> generationTags; ///< additional tags controlling how asset is placed
    float                    minScale  = 0.8f;
    float                    maxScale  = 1.2f;
    float                    density   = 1.0f;
    bool                     allowRotation = true;
    bool                     alignToNormal = false;
    std::string              exclusionGroup; ///< assets in the same group don't overlap
};

// ── AssetPreviewDescriptor ────────────────────────────────────────────────────

struct AssetPreviewDescriptor {
    std::string      assetPath;
    std::string      meshTag;
    std::string      materialTag;
    std::string      attachmentTag;
    PreviewTransform transform;

    // D.3 extension: collider, sockets, anchors, PCG metadata
    ColliderDescriptor             collider;
    std::vector<SocketDescriptor>  sockets;
    std::vector<AnchorDescriptor>  anchors;
    AssetPCGMetadata               pcgMetadata;
};

// ── NovaForgeAssetPreview ─────────────────────────────────────────────────────

class NovaForgeAssetPreview : public NF::IViewportSceneProvider {
public:
    NovaForgeAssetPreview()  = default;
    ~NovaForgeAssetPreview() override = default;

    // ── IViewportSceneProvider ────────────────────────────────────────────

    NF::ViewportSceneState provideScene(NF::ViewportHandle       /*handle*/,
                                        const NF::ViewportSlot&  /*slot*/) override {
        NF::ViewportSceneState st;
        st.hasContent  = !m_descriptor.assetPath.empty();
        st.entityCount = m_world.entityCount();
        st.clearColor  = 0x1A1A1AFFu;
        return st;
    }

    // ── Asset binding ─────────────────────────────────────────────────────

    void bindAsset(const AssetPreviewDescriptor& descriptor) {
        m_descriptor  = descriptor;
        m_lastApplied = descriptor;
        m_dirty       = false;
        rebuildPreview();
    }

    void clearAsset() {
        m_descriptor  = {};
        m_lastApplied = {};
        m_dirty       = false;
        m_world.clearEntities();
        m_previewEntity = kInvalidEntityId;
    }

    [[nodiscard]] bool hasAsset() const { return !m_descriptor.assetPath.empty(); }

    [[nodiscard]] const AssetPreviewDescriptor& descriptor() const { return m_descriptor; }

    // ── Editable fields — mesh / material / attachment ────────────────────

    bool setTransform(const PreviewTransform& t) {
        if (!hasAsset()) return false;
        m_descriptor.transform = t;
        m_world.setTransform(m_previewEntity, t);
        m_dirty = true;
        return true;
    }

    bool setMeshTag(const std::string& tag) {
        if (!hasAsset()) return false;
        m_descriptor.meshTag = tag;
        m_world.setMesh(m_previewEntity, tag);
        m_dirty = true;
        return true;
    }

    bool setMaterialTag(const std::string& tag) {
        if (!hasAsset()) return false;
        m_descriptor.materialTag = tag;
        m_world.setMaterial(m_previewEntity, tag);
        m_dirty = true;
        return true;
    }

    bool setAttachmentTag(const std::string& tag) {
        if (!hasAsset()) return false;
        m_descriptor.attachmentTag = tag;
        m_dirty = true;
        return true;
    }

    // ── Editable fields — collider ────────────────────────────────────────

    bool setCollider(const ColliderDescriptor& c) {
        if (!hasAsset()) return false;
        m_descriptor.collider = c;
        m_dirty = true;
        return true;
    }

    bool setColliderShape(ColliderShape shape) {
        if (!hasAsset()) return false;
        m_descriptor.collider.shape = shape;
        m_dirty = true;
        return true;
    }

    bool setColliderExtents(const PreviewVec3& extents) {
        if (!hasAsset()) return false;
        m_descriptor.collider.extents = extents;
        m_dirty = true;
        return true;
    }

    bool setColliderIsTrigger(bool trigger) {
        if (!hasAsset()) return false;
        m_descriptor.collider.isTrigger = trigger;
        m_dirty = true;
        return true;
    }

    bool setColliderTag(const std::string& tag) {
        if (!hasAsset()) return false;
        m_descriptor.collider.tag = tag;
        m_dirty = true;
        return true;
    }

    [[nodiscard]] const ColliderDescriptor& collider() const { return m_descriptor.collider; }

    // ── Editable fields — sockets ─────────────────────────────────────────

    bool addSocket(const SocketDescriptor& socket) {
        if (!hasAsset()) return false;
        m_descriptor.sockets.push_back(socket);
        m_dirty = true;
        return true;
    }

    bool removeSocket(const std::string& name) {
        if (!hasAsset()) return false;
        auto& v = m_descriptor.sockets;
        for (auto it = v.begin(); it != v.end(); ++it) {
            if (it->name == name) { v.erase(it); m_dirty = true; return true; }
        }
        return false;
    }

    bool setSocketTransform(const std::string& name, const PreviewTransform& t) {
        if (!hasAsset()) return false;
        for (auto& s : m_descriptor.sockets) {
            if (s.name == name) { s.localTransform = t; m_dirty = true; return true; }
        }
        return false;
    }

    [[nodiscard]] const std::vector<SocketDescriptor>& sockets() const { return m_descriptor.sockets; }
    [[nodiscard]] uint32_t socketCount() const { return static_cast<uint32_t>(m_descriptor.sockets.size()); }

    // ── Editable fields — anchors ─────────────────────────────────────────

    bool addAnchor(const AnchorDescriptor& anchor) {
        if (!hasAsset()) return false;
        m_descriptor.anchors.push_back(anchor);
        m_dirty = true;
        return true;
    }

    bool removeAnchor(const std::string& name) {
        if (!hasAsset()) return false;
        auto& v = m_descriptor.anchors;
        for (auto it = v.begin(); it != v.end(); ++it) {
            if (it->name == name) { v.erase(it); m_dirty = true; return true; }
        }
        return false;
    }

    bool setAnchorTransform(const std::string& name, const PreviewTransform& t) {
        if (!hasAsset()) return false;
        for (auto& a : m_descriptor.anchors) {
            if (a.name == name) { a.localTransform = t; m_dirty = true; return true; }
        }
        return false;
    }

    [[nodiscard]] const std::vector<AnchorDescriptor>& anchors() const { return m_descriptor.anchors; }
    [[nodiscard]] uint32_t anchorCount() const { return static_cast<uint32_t>(m_descriptor.anchors.size()); }

    // ── Editable fields — PCG metadata ────────────────────────────────────

    bool setPCGMetadata(const AssetPCGMetadata& meta) {
        if (!hasAsset()) return false;
        m_descriptor.pcgMetadata = meta;
        m_dirty = true;
        return true;
    }

    bool setPlacementTag(const std::string& tag) {
        if (!hasAsset()) return false;
        m_descriptor.pcgMetadata.placementTag = tag;
        m_dirty = true;
        return true;
    }

    bool addGenerationTag(const std::string& tag) {
        if (!hasAsset()) return false;
        m_descriptor.pcgMetadata.generationTags.push_back(tag);
        m_dirty = true;
        return true;
    }

    bool removeGenerationTag(const std::string& tag) {
        if (!hasAsset()) return false;
        auto& v = m_descriptor.pcgMetadata.generationTags;
        for (auto it = v.begin(); it != v.end(); ++it) {
            if (*it == tag) { v.erase(it); m_dirty = true; return true; }
        }
        return false;
    }

    bool setPCGScaleRange(float minScale, float maxScale) {
        if (!hasAsset() || minScale > maxScale) return false;
        m_descriptor.pcgMetadata.minScale = minScale;
        m_descriptor.pcgMetadata.maxScale = maxScale;
        m_dirty = true;
        return true;
    }

    bool setPCGDensity(float density) {
        if (!hasAsset() || density < 0.f) return false;
        m_descriptor.pcgMetadata.density = density;
        m_dirty = true;
        return true;
    }

    bool setPCGExclusionGroup(const std::string& group) {
        if (!hasAsset()) return false;
        m_descriptor.pcgMetadata.exclusionGroup = group;
        m_dirty = true;
        return true;
    }

    [[nodiscard]] const AssetPCGMetadata& pcgMetadata() const { return m_descriptor.pcgMetadata; }

    // ── Dirty tracking ────────────────────────────────────────────────────

    [[nodiscard]] bool isDirty() const { return m_dirty; }
    void clearDirty()                  { m_dirty = false; }

    // ── Save/revert ───────────────────────────────────────────────────────

    /// Commit current changes as the new baseline.
    bool apply() {
        if (!hasAsset()) return false;
        m_lastApplied = m_descriptor;
        m_dirty       = false;
        return true;
    }

    /// Revert to the last applied state.
    bool revert() {
        if (!hasAsset()) return false;
        m_descriptor = m_lastApplied;
        m_dirty      = false;
        rebuildPreview();
        return true;
    }

    // ── Inspector data ────────────────────────────────────────────────────

    [[nodiscard]] std::vector<std::pair<std::string, std::string>> properties() const {
        auto fmtF = [](float v) -> std::string {
            char buf[32];
            std::snprintf(buf, sizeof(buf), "%.3f", static_cast<double>(v));
            return buf;
        };

        std::vector<std::pair<std::string, std::string>> props = {
            {"assetPath",           m_descriptor.assetPath},
            {"meshTag",             m_descriptor.meshTag},
            {"materialTag",         m_descriptor.materialTag},
            {"attachmentTag",       m_descriptor.attachmentTag},
            {"position.x",         fmtF(m_descriptor.transform.position.x)},
            {"position.y",         fmtF(m_descriptor.transform.position.y)},
            {"position.z",         fmtF(m_descriptor.transform.position.z)},
            {"rotation.x",         fmtF(m_descriptor.transform.rotation.x)},
            {"rotation.y",         fmtF(m_descriptor.transform.rotation.y)},
            {"rotation.z",         fmtF(m_descriptor.transform.rotation.z)},
            {"scale.x",            fmtF(m_descriptor.transform.scale.x)},
            {"scale.y",            fmtF(m_descriptor.transform.scale.y)},
            {"scale.z",            fmtF(m_descriptor.transform.scale.z)},
            {"collider.shape",     colliderShapeName(m_descriptor.collider.shape)},
            {"collider.isTrigger", m_descriptor.collider.isTrigger ? "true" : "false"},
            {"collider.tag",       m_descriptor.collider.tag},
            {"pcg.placementTag",   m_descriptor.pcgMetadata.placementTag},
            {"pcg.minScale",       fmtF(m_descriptor.pcgMetadata.minScale)},
            {"pcg.maxScale",       fmtF(m_descriptor.pcgMetadata.maxScale)},
            {"pcg.density",        fmtF(m_descriptor.pcgMetadata.density)},
        };
        return props;
    }

    // ── Preview world access ──────────────────────────────────────────────

    [[nodiscard]] const NovaForgePreviewWorld& previewWorld() const { return m_world; }

    // ── E.4 — Event-driven PCG preview wiring ────────────────────────────
    //
    // Attach a PCGPreviewService so that any change to asset PCG tags
    // (placementTag, generationTags, scale, density, exclusionGroup) causes
    // an automatic preview regeneration in the attached service.
    //
    // The caller retains ownership of the service.

    void attachPCGPreviewService(PCGPreviewService* service) {
        m_pcgPreview = service;
    }

    void detachPCGPreviewService() { m_pcgPreview = nullptr; }

    [[nodiscard]] bool hasPCGPreviewService() const { return m_pcgPreview != nullptr; }

    // ── E.4 — PCG tag mutators with auto-trigger ──────────────────────────
    //
    // These wrappers call the existing setPlacementTag / addGenerationTag /
    // removeGenerationTag / setPCGScaleRange / setPCGDensity / setPCGExclusionGroup
    // and additionally trigger a forced PCG preview regeneration when a service
    // is attached, providing the event-driven update loop required by E.4.

    bool setPlacementTagAndNotify(const std::string& tag) {
        if (!setPlacementTag(tag)) return false;
        triggerPCGRegen();
        return true;
    }

    bool addGenerationTagAndNotify(const std::string& tag) {
        if (!addGenerationTag(tag)) return false;
        triggerPCGRegen();
        return true;
    }

    bool removeGenerationTagAndNotify(const std::string& tag) {
        if (!removeGenerationTag(tag)) return false;
        triggerPCGRegen();
        return true;
    }

    bool setPCGScaleRangeAndNotify(float minScale, float maxScale) {
        if (!setPCGScaleRange(minScale, maxScale)) return false;
        triggerPCGRegen();
        return true;
    }

    bool setPCGDensityAndNotify(float density) {
        if (!setPCGDensity(density)) return false;
        triggerPCGRegen();
        return true;
    }

    bool setPCGExclusionGroupAndNotify(const std::string& group) {
        if (!setPCGExclusionGroup(group)) return false;
        triggerPCGRegen();
        return true;
    }

    /// Number of times a PCG regeneration was triggered by a tag-change event.
    [[nodiscard]] uint32_t pcgRegenTriggerCount() const { return m_pcgRegenTriggers; }

private:
    AssetPreviewDescriptor m_descriptor;
    AssetPreviewDescriptor m_lastApplied;
    NovaForgePreviewWorld  m_world;
    EntityId               m_previewEntity   = kInvalidEntityId;
    bool                   m_dirty           = false;
    PCGPreviewService*     m_pcgPreview      = nullptr;
    uint32_t               m_pcgRegenTriggers = 0;

    void triggerPCGRegen() {
        ++m_pcgRegenTriggers;
        if (m_pcgPreview) m_pcgPreview->forceRegenerate();
    }

    void rebuildPreview() {
        m_world.clearEntities();
        if (!m_descriptor.assetPath.empty()) {
            m_previewEntity = m_world.createEntity("PreviewAsset");
            m_world.setTransform(m_previewEntity, m_descriptor.transform);
            m_world.setMesh(m_previewEntity, m_descriptor.meshTag);
            m_world.setMaterial(m_previewEntity, m_descriptor.materialTag);
            m_world.clearDirty();
        }
    }
};


} // namespace NovaForge
