#pragma once
#include "../../engine/character/CharacterGraph.h"
#include "../../engine/character/HumanMeshGenerator.h"
#include "../../engine/animation/CharacterAnimationController.h"
#include "../../engine/procedural/ProceduralMeshGraph.h"
#include "../../engine/ui/UIDrawList.h"
#include <cstdint>
#include <string>
#include <vector>

namespace atlas::editor {

/// Represents a single selectable character slot on the character
/// select screen.  Each slot holds the generation seed and a cached
/// mesh so the 3D preview can be rendered without re-generating every
/// frame.
struct CharacterSlot {
    uint64_t seed       = 0;
    uint32_t factionID  = 0;
    float    age        = 25.0f;
    float    fitness    = 0.5f;
    std::string name;
    character::HumanMeshParams params;
    procedural::MeshData mesh;
    bool meshValid = false;
};

/// The character select screen presented before gameplay.
/// It renders a rotatable 3D view of the currently-selected character
/// and allows cycling between character slots.
///
/// Rotation is driven by SetRotationY / Rotate which update a simple
/// Y-axis orbit angle.  The Draw() method projects the character mesh
/// into a 2D viewport using a basic orthographic projection rotated
/// around the Y axis by the current orbit angle.
class CharacterSelectScreen {
public:
    CharacterSelectScreen();

    /// Populate initial character slots from seeds.
    void InitSlots(const std::vector<uint64_t>& seeds,
                   uint32_t factionID = 1,
                   float age = 25.0f,
                   float fitness = 0.5f);

    /// Add a single character slot.
    void AddSlot(const CharacterSlot& slot);

    /// Number of available character slots.
    size_t SlotCount() const { return m_slots.size(); }

    /// Select a slot by index.
    void SelectSlot(size_t index);
    size_t SelectedIndex() const { return m_selectedIndex; }
    const CharacterSlot* SelectedSlot() const;

    /// Confirm the character selection (user clicked "Select").
    void ConfirmSelection();
    bool IsSelectionConfirmed() const { return m_confirmed; }

    // --- 3D Rotation ---

    /// Set the Y-axis rotation in degrees.
    void SetRotationY(float degrees) { m_rotationY = degrees; }

    /// Rotate the view by a delta (e.g. from mouse drag).
    void Rotate(float deltaDegrees) { m_rotationY += deltaDegrees; }

    /// Current Y-axis rotation in degrees.
    float RotationY() const { return m_rotationY; }

    // --- Zoom ---
    void SetZoom(float z) { m_zoom = z; }
    float Zoom() const { return m_zoom; }

    // --- Animation ---

    /// Step the idle animation forward by dt seconds.
    void UpdateAnimation(float dt);

    /// Current animation controller (read-only).
    const animation::CharacterAnimationController& AnimController() const { return m_animController; }

    // --- Rendering ---

    /// Rebuild the draw list for the current frame.
    void Draw();

    /// Access the draw list (for testing / flushing to a renderer).
    const atlas::ui::UIDrawList& GetDrawList() const { return m_drawList; }

    /// Regenerate the mesh for the selected slot (e.g. after editing).
    void RegenerateSelectedMesh();

private:
    std::vector<CharacterSlot> m_slots;
    size_t m_selectedIndex = 0;
    bool m_confirmed = false;

    float m_rotationY = 0.0f;
    float m_zoom = 1.0f;

    animation::CharacterAnimationController m_animController;
    atlas::ui::UIDrawList m_drawList;

    // Simple 3D→2D projection helpers
    void ProjectAndDrawMesh(const procedural::MeshData& mesh,
                            int32_t vpX, int32_t vpY,
                            int32_t vpW, int32_t vpH);
};

} // namespace atlas::editor
