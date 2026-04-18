#pragma once
#include "../ui/EditorPanel.h"
#include "../../engine/character/CharacterGraph.h"
#include "../../engine/character/HumanMeshGenerator.h"
#include "../../engine/animation/CharacterAnimationController.h"
#include "../../engine/procedural/ProceduralMeshGraph.h"
#include "../../engine/ui/UIDrawList.h"
#include <string>
#include <sstream>
#include <functional>

namespace atlas::editor {

/// Which section of the character editor is expanded.
enum class CharacterEditorSection : uint8_t {
    Body,
    Face,
    Hair,
    Material,
    Animation
};

/// The Character Editor Panel allows editing all procedural character
/// parameters through sliders and value fields.  Changes are
/// immediately reflected by regenerating the character mesh.
///
/// The panel exposes the full HumanMeshParams surface and can push
/// updates to a CharacterSelectScreen or MeshViewerPanel.
class CharacterEditorPanel : public EditorPanel {
public:
    const char* Name() const override { return "Character Editor"; }
    void Draw() override;

    // --- Parameters ---

    /// Set the params to edit.
    void SetParams(const character::HumanMeshParams& p) { m_params = p; m_dirty = true; }
    const character::HumanMeshParams& Params() const { return m_params; }

    // --- Individual setters (mark dirty) ---

    void SetHeight(float v)       { m_params.height = v;       m_dirty = true; }
    void SetMass(float v)         { m_params.mass = v;         m_dirty = true; }
    void SetArmRatio(float v)     { m_params.armRatio = v;     m_dirty = true; }
    void SetLegRatio(float v)     { m_params.legRatio = v;     m_dirty = true; }

    void SetNoseWidth(float v)    { m_params.noseWidth = v;    m_dirty = true; }
    void SetEyeSpacing(float v)   { m_params.eyeSpacing = v;  m_dirty = true; }
    void SetJawWidth(float v)     { m_params.jawWidth = v;     m_dirty = true; }
    void SetBrowHeight(float v)   { m_params.browHeight = v;   m_dirty = true; }
    void SetLipFullness(float v)  { m_params.lipFullness = v;  m_dirty = true; }

    void SetHairStyle(float v)    { m_params.hairStyle = v;    m_dirty = true; }
    void SetHairLength(float v)   { m_params.hairLength = v;   m_dirty = true; }
    void SetHairDensity(float v)  { m_params.hairDensity = v;  m_dirty = true; }
    void SetHairColor(float r, float g, float b) {
        m_params.hairR = r; m_params.hairG = g; m_params.hairB = b;
        m_dirty = true;
    }

    void SetSkinTone(float v)     { m_params.skinTone = v;     m_dirty = true; }
    void SetEyeColor(float v)     { m_params.eyeColor = v;     m_dirty = true; }

    // --- Sections ---

    void SetSection(CharacterEditorSection s) { m_section = s; }
    CharacterEditorSection CurrentSection() const { return m_section; }

    // --- Animation preview ---

    void SetAnimState(animation::CharacterAnimState s);
    animation::CharacterAnimState AnimState() const;
    void UpdateAnimation(float dt);
    const animation::CharacterAnimationController& AnimController() const { return m_animController; }

    // --- Mesh generation ---

    /// Regenerate mesh from current params.  Returns true if the mesh
    /// changed (params were dirty).
    bool RegenerateMesh();

    bool HasMesh() const { return m_meshValid; }
    const procedural::MeshData& GetMesh() const { return m_mesh; }

    bool IsDirty() const { return m_dirty; }
    void MarkClean() { m_dirty = false; }

    /// Optional callback invoked when params change.
    void SetOnParamsChanged(std::function<void(const character::HumanMeshParams&)> cb) {
        m_onParamsChanged = std::move(cb);
    }

    std::string Summary() const;
    const atlas::ui::UIDrawList& GetDrawList() const { return m_drawList; }

private:
    character::HumanMeshParams m_params;
    bool m_dirty = true;
    bool m_meshValid = false;
    procedural::MeshData m_mesh;

    CharacterEditorSection m_section = CharacterEditorSection::Body;
    animation::CharacterAnimationController m_animController;

    atlas::ui::UIDrawList m_drawList;

    std::function<void(const character::HumanMeshParams&)> m_onParamsChanged;

    void DrawSectionTabs(int32_t& y);
    void DrawBodySection(int32_t y);
    void DrawFaceSection(int32_t y);
    void DrawHairSection(int32_t y);
    void DrawMaterialSection(int32_t y);
    void DrawAnimationSection(int32_t y);
    void DrawSlider(int32_t x, int32_t y, int32_t w,
                    const char* label, float value,
                    float minVal, float maxVal);
};

} // namespace atlas::editor
