#include "PBRMaterial.h"
#include <algorithm>
#include <cmath>

namespace atlas::render {

uint32_t PBRMaterial::s_nextId = 1;

PBRMaterialParams PBRMaterialParams::Default() {
    PBRMaterialParams p;
    p.albedoColor[0] = 0.5f;
    p.albedoColor[1] = 0.5f;
    p.albedoColor[2] = 0.5f;
    p.albedoColor[3] = 1.0f;
    p.metallic = 0.0f;
    p.roughness = 0.5f;
    p.ao = 1.0f;
    p.emissiveColor[0] = 0.0f;
    p.emissiveColor[1] = 0.0f;
    p.emissiveColor[2] = 0.0f;
    p.emissiveIntensity = 0.0f;
    p.normalScale = 1.0f;
    p.heightScale = 0.05f;
    p.alphaMode = AlphaMode::Opaque;
    p.alphaCutoff = 0.5f;
    return p;
}

bool PBRMaterialParams::IsEmissive() const {
    if (emissiveIntensity <= 0.0f) return false;
    return (emissiveColor[0] > 0.0f || emissiveColor[1] > 0.0f || emissiveColor[2] > 0.0f);
}

PBRMaterial::PBRMaterial()
    : m_id(s_nextId++)
    , m_params(PBRMaterialParams::Default()) {
}

void PBRMaterial::SetParams(const PBRMaterialParams& params) {
    m_params = params;
}

void PBRMaterial::BindTexture(PBRTextureSlot slot, uint32_t textureId, uint32_t samplerId) {
    // Replace existing binding for this slot if present
    auto it = std::find_if(m_bindings.begin(), m_bindings.end(),
        [slot](const PBRTextureBinding& b) { return b.slot == slot; });
    if (it != m_bindings.end()) {
        it->textureId = textureId;
        it->samplerId = samplerId;
    } else {
        m_bindings.push_back({slot, textureId, samplerId});
    }
}

void PBRMaterial::UnbindTexture(PBRTextureSlot slot) {
    m_bindings.erase(
        std::remove_if(m_bindings.begin(), m_bindings.end(),
            [slot](const PBRTextureBinding& b) { return b.slot == slot; }),
        m_bindings.end());
}

const PBRTextureBinding* PBRMaterial::GetTextureBinding(PBRTextureSlot slot) const {
    auto it = std::find_if(m_bindings.begin(), m_bindings.end(),
        [slot](const PBRTextureBinding& b) { return b.slot == slot; });
    if (it != m_bindings.end()) {
        return &(*it);
    }
    return nullptr;
}

bool PBRMaterial::HasTexture(PBRTextureSlot slot) const {
    return GetTextureBinding(slot) != nullptr;
}

uint32_t PBRMaterial::TextureBindingCount() const {
    return static_cast<uint32_t>(m_bindings.size());
}

bool PBRMaterial::Validate() const {
    if (m_params.metallic < 0.0f || m_params.metallic > 1.0f) return false;
    if (m_params.roughness < 0.0f || m_params.roughness > 1.0f) return false;
    if (m_params.ao < 0.0f || m_params.ao > 1.0f) return false;
    if (m_params.alphaCutoff < 0.0f || m_params.alphaCutoff > 1.0f) return false;
    if (m_params.emissiveIntensity < 0.0f) return false;
    for (int i = 0; i < 4; ++i) {
        if (m_params.albedoColor[i] < 0.0f || m_params.albedoColor[i] > 1.0f) return false;
    }
    for (int i = 0; i < 3; ++i) {
        if (m_params.emissiveColor[i] < 0.0f || m_params.emissiveColor[i] > 1.0f) return false;
    }
    return true;
}

} // namespace atlas::render
