#pragma once
// NF::Editor — Render pass editor v1: attachment and pass configuration for render pipelines
#include "NF/Core/Core.h"
#include <cstdint>
#include <string>
#include <vector>

namespace NF {

enum class RpvAttachmentType : uint8_t { Color, Depth, Stencil, DepthStencil };
enum class RpvLoadOp         : uint8_t { Load, Clear, DontCare };
enum class RpvStoreOp        : uint8_t { Store, Discard };

struct RpvAttachment {
    uint32_t          id    = 0;
    RpvAttachmentType type  = RpvAttachmentType::Color;
    RpvLoadOp         load  = RpvLoadOp::Clear;
    RpvStoreOp        store = RpvStoreOp::Store;
    [[nodiscard]] bool isValid() const { return id != 0; }
};

struct RpvPass {
    uint32_t              id          = 0;
    std::string           name;
    std::vector<uint32_t> attachmentIds;
    bool                  depthTest   = true;
    bool                  depthWrite  = true;
    [[nodiscard]] bool isValid() const { return id != 0 && !name.empty(); }
};

class RenderPassEditorV1 {
public:
    bool addAttachment(const RpvAttachment& att) {
        if (!att.isValid()) return false;
        for (const auto& a : m_attachments) if (a.id == att.id) return false;
        m_attachments.push_back(att);
        return true;
    }

    bool removeAttachment(uint32_t id) {
        for (auto it = m_attachments.begin(); it != m_attachments.end(); ++it) {
            if (it->id == id) { m_attachments.erase(it); return true; }
        }
        return false;
    }

    bool addPass(const RpvPass& pass) {
        if (!pass.isValid()) return false;
        for (const auto& p : m_passes) if (p.id == pass.id) return false;
        m_passes.push_back(pass);
        return true;
    }

    bool removePass(uint32_t id) {
        for (auto it = m_passes.begin(); it != m_passes.end(); ++it) {
            if (it->id == id) { m_passes.erase(it); return true; }
        }
        return false;
    }

    [[nodiscard]] bool validate() const {
        for (const auto& pass : m_passes) {
            for (auto attId : pass.attachmentIds) {
                bool found = false;
                for (const auto& a : m_attachments) if (a.id == attId) { found = true; break; }
                if (!found) return false;
            }
        }
        return true;
    }

    [[nodiscard]] size_t attachmentCount() const { return m_attachments.size(); }
    [[nodiscard]] size_t passCount()       const { return m_passes.size();       }

private:
    std::vector<RpvAttachment> m_attachments;
    std::vector<RpvPass>       m_passes;
};

} // namespace NF
