#pragma once
// NF::Editor — AssetPreviewState: 3D preview state for the Asset Editor.
//
// Tracks the edit state of the asset preview viewport: transform editing,
// active material slot selection, and attachment point management.
// This is the data model consumed by the asset preview panel renderer;
// the renderer is responsible for projecting these values onto the
// actual scene-provider provided by NovaForgeAssetPreview.
//
// Phase G.2 — Asset Editor: 3D preview with editable transform/material/attachment

#include "NF/Core/Core.h"
#include <cstdint>
#include <string>
#include <vector>

namespace NF {

// ── Preview transform ─────────────────────────────────────────────────────────

struct PreviewTransform {
    float posX = 0.f, posY = 0.f, posZ = 0.f;     ///< world-space position
    float rotX = 0.f, rotY = 0.f, rotZ = 0.f;     ///< Euler angles in degrees
    float scaleX = 1.f, scaleY = 1.f, scaleZ = 1.f;

    void reset() {
        posX = posY = posZ = 0.f;
        rotX = rotY = rotZ = 0.f;
        scaleX = scaleY = scaleZ = 1.f;
    }

    [[nodiscard]] bool isIdentity() const {
        return posX == 0.f && posY == 0.f && posZ == 0.f
            && rotX == 0.f && rotY == 0.f && rotZ == 0.f
            && scaleX == 1.f && scaleY == 1.f && scaleZ == 1.f;
    }
};

// ── Material slot override ────────────────────────────────────────────────────

struct PreviewMaterialSlot {
    uint32_t    slotIndex    = 0;
    std::string materialGuid;     ///< override material GUID, empty = use default
    std::string displayName;

    [[nodiscard]] bool hasOverride() const { return !materialGuid.empty(); }
};

// ── Attachment point ──────────────────────────────────────────────────────────

struct PreviewAttachmentPoint {
    std::string socketName;           ///< bone/socket name on the asset
    std::string attachedAssetGuid;    ///< GUID of the attached prop, empty = none
    PreviewTransform localOffset;     ///< local offset relative to the socket
};

// ── AssetPreviewState ─────────────────────────────────────────────────────────

class AssetPreviewState {
public:
    AssetPreviewState() = default;

    // ── Transform ─────────────────────────────────────────────────────────

    [[nodiscard]] const PreviewTransform& transform() const { return m_transform; }

    void setPosition(float x, float y, float z) {
        m_transform.posX = x; m_transform.posY = y; m_transform.posZ = z;
        m_dirty = true;
    }

    void setRotation(float rx, float ry, float rz) {
        m_transform.rotX = rx; m_transform.rotY = ry; m_transform.rotZ = rz;
        m_dirty = true;
    }

    void setScale(float sx, float sy, float sz) {
        m_transform.scaleX = sx; m_transform.scaleY = sy; m_transform.scaleZ = sz;
        m_dirty = true;
    }

    void resetTransform() { m_transform.reset(); m_dirty = true; }

    // ── Material slots ────────────────────────────────────────────────────

    void addMaterialSlot(const PreviewMaterialSlot& slot) {
        m_materialSlots.push_back(slot);
        m_dirty = true;
    }

    bool setMaterialOverride(uint32_t slotIndex, const std::string& materialGuid) {
        for (auto& s : m_materialSlots) {
            if (s.slotIndex == slotIndex) {
                s.materialGuid = materialGuid;
                m_dirty = true;
                return true;
            }
        }
        return false;
    }

    bool clearMaterialOverride(uint32_t slotIndex) {
        return setMaterialOverride(slotIndex, "");
    }

    [[nodiscard]] uint32_t materialSlotCount() const {
        return static_cast<uint32_t>(m_materialSlots.size());
    }

    [[nodiscard]] const std::vector<PreviewMaterialSlot>& materialSlots() const {
        return m_materialSlots;
    }

    [[nodiscard]] const PreviewMaterialSlot* findMaterialSlot(uint32_t slotIndex) const {
        for (const auto& s : m_materialSlots)
            if (s.slotIndex == slotIndex) return &s;
        return nullptr;
    }

    [[nodiscard]] uint32_t activeSlotIndex() const { return m_activeSlotIndex; }

    void setActiveSlot(uint32_t slotIndex) {
        m_activeSlotIndex = slotIndex;
    }

    // ── Attachment points ─────────────────────────────────────────────────

    void addAttachmentPoint(const PreviewAttachmentPoint& point) {
        m_attachments.push_back(point);
        m_dirty = true;
    }

    bool removeAttachmentPoint(const std::string& socketName) {
        for (auto it = m_attachments.begin(); it != m_attachments.end(); ++it) {
            if (it->socketName == socketName) {
                m_attachments.erase(it);
                m_dirty = true;
                return true;
            }
        }
        return false;
    }

    bool setAttachment(const std::string& socketName, const std::string& assetGuid) {
        for (auto& a : m_attachments) {
            if (a.socketName == socketName) {
                a.attachedAssetGuid = assetGuid;
                m_dirty = true;
                return true;
            }
        }
        return false;
    }

    bool clearAttachment(const std::string& socketName) {
        return setAttachment(socketName, "");
    }

    [[nodiscard]] uint32_t attachmentCount() const {
        return static_cast<uint32_t>(m_attachments.size());
    }

    [[nodiscard]] const std::vector<PreviewAttachmentPoint>& attachments() const {
        return m_attachments;
    }

    [[nodiscard]] const PreviewAttachmentPoint* findAttachment(const std::string& socketName) const {
        for (const auto& a : m_attachments)
            if (a.socketName == socketName) return &a;
        return nullptr;
    }

    // ── Dirty tracking ────────────────────────────────────────────────────

    [[nodiscard]] bool isDirty() const { return m_dirty; }
    void clearDirty() { m_dirty = false; }

    /// Reset everything to defaults.
    void reset() {
        m_transform.reset();
        m_materialSlots.clear();
        m_attachments.clear();
        m_activeSlotIndex = 0;
        m_dirty = false;
    }

private:
    PreviewTransform                   m_transform;
    std::vector<PreviewMaterialSlot>   m_materialSlots;
    std::vector<PreviewAttachmentPoint> m_attachments;
    uint32_t                           m_activeSlotIndex = 0;
    bool                               m_dirty = false;
};

} // namespace NF
