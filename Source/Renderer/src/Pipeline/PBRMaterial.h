#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace atlas::render {

/// Texture slots for PBR materials.
enum class PBRTextureSlot : uint8_t {
    Albedo,
    Normal,
    Metallic,
    Roughness,
    AO,
    Emissive,
    Height
};

/// Alpha blending mode for PBR materials.
enum class AlphaMode : uint8_t {
    Opaque,
    Mask,
    Blend
};

/// Parameters describing a PBR material's properties.
struct PBRMaterialParams {
    float albedoColor[4] = {0.5f, 0.5f, 0.5f, 1.0f};
    float metallic = 0.0f;
    float roughness = 0.5f;
    float ao = 1.0f;
    float emissiveColor[3] = {0.0f, 0.0f, 0.0f};
    float emissiveIntensity = 0.0f;
    float normalScale = 1.0f;
    float heightScale = 0.05f;
    AlphaMode alphaMode = AlphaMode::Opaque;
    float alphaCutoff = 0.5f;

    /// Create default grey material parameters.
    static PBRMaterialParams Default();

    /// Returns true if the material emits light.
    bool IsEmissive() const;
};

/// Binds a texture to a PBR texture slot.
struct PBRTextureBinding {
    PBRTextureSlot slot = PBRTextureSlot::Albedo;
    uint32_t textureId = 0;
    uint32_t samplerId = 0;
};

/// PBR material with texture bindings and material parameters.
class PBRMaterial {
public:
    PBRMaterial();

    void SetParams(const PBRMaterialParams& params);
    const PBRMaterialParams& GetParams() const { return m_params; }

    void BindTexture(PBRTextureSlot slot, uint32_t textureId, uint32_t samplerId);
    void UnbindTexture(PBRTextureSlot slot);
    const PBRTextureBinding* GetTextureBinding(PBRTextureSlot slot) const;
    bool HasTexture(PBRTextureSlot slot) const;
    uint32_t TextureBindingCount() const;

    uint32_t GetId() const { return m_id; }
    void SetName(const std::string& name) { m_name = name; }
    const std::string& GetName() const { return m_name; }

    /// Validate that all parameters are within valid ranges.
    bool Validate() const;

private:
    static uint32_t s_nextId;

    uint32_t m_id = 0;
    std::string m_name;
    PBRMaterialParams m_params;
    std::vector<PBRTextureBinding> m_bindings;
};

} // namespace atlas::render
