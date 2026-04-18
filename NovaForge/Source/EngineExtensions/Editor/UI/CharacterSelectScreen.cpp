#include "CharacterSelectScreen.h"
#include <cmath>
#include <cstdio>
#include <algorithm>

namespace atlas::editor {

CharacterSelectScreen::CharacterSelectScreen() {
    m_animController.SetState(animation::CharacterAnimState::Idle);
}

void CharacterSelectScreen::InitSlots(const std::vector<uint64_t>& seeds,
                                       uint32_t factionID,
                                       float age,
                                       float fitness) {
    m_slots.clear();
    for (uint64_t seed : seeds) {
        CharacterSlot slot;
        slot.seed      = seed;
        slot.factionID = factionID;
        slot.age       = age;
        slot.fitness   = fitness;
        slot.name      = "Character " + std::to_string(seed);

        // Generate mesh
        character::CharacterContext ctx{seed, factionID, age, fitness};
        slot.mesh = character::HumanMeshGenerator::GenerateFromContext(ctx);
        slot.meshValid = slot.mesh.IsValid();

        // Extract params for the editor
        character::CharacterGraph graph;
        auto bodyId = graph.AddNode(std::make_unique<character::BaseBodyNode>());
        auto faceId = graph.AddNode(std::make_unique<character::FacialFeatureNode>());
        auto hairId = graph.AddNode(std::make_unique<character::HairStyleNode>());
        auto matId  = graph.AddNode(std::make_unique<character::MaterialNode>());
        graph.Compile();
        graph.Execute(ctx);
        slot.params = character::HumanMeshGenerator::ParamsFromGraph(
            graph, bodyId, faceId, hairId, matId);

        m_slots.push_back(std::move(slot));
    }
    if (!m_slots.empty()) m_selectedIndex = 0;
}

void CharacterSelectScreen::AddSlot(const CharacterSlot& slot) {
    m_slots.push_back(slot);
}

void CharacterSelectScreen::SelectSlot(size_t index) {
    if (index < m_slots.size()) {
        m_selectedIndex = index;
    }
}

const CharacterSlot* CharacterSelectScreen::SelectedSlot() const {
    if (m_selectedIndex < m_slots.size()) {
        return &m_slots[m_selectedIndex];
    }
    return nullptr;
}

void CharacterSelectScreen::ConfirmSelection() {
    m_confirmed = true;
}

void CharacterSelectScreen::UpdateAnimation(float dt) {
    m_animController.Update(dt);
}

void CharacterSelectScreen::RegenerateSelectedMesh() {
    if (m_selectedIndex >= m_slots.size()) return;
    auto& slot = m_slots[m_selectedIndex];
    slot.mesh = character::HumanMeshGenerator::Generate(slot.params);
    slot.meshValid = slot.mesh.IsValid();
}

// ---------------------------------------------------------------------------
// 3D → 2D projection with Y-axis rotation
// ---------------------------------------------------------------------------

void CharacterSelectScreen::ProjectAndDrawMesh(
    const procedural::MeshData& mesh,
    int32_t vpX, int32_t vpY,
    int32_t vpW, int32_t vpH)
{
    if (!mesh.IsValid()) return;

    float rad = m_rotationY * 3.14159265f / 180.0f;
    float cosA = std::cos(rad);
    float sinA = std::sin(rad);

    float scale = static_cast<float>(vpH) * 0.45f * m_zoom;
    float cx = static_cast<float>(vpX + vpW / 2);
    float cy = static_cast<float>(vpY + vpH) - 40.0f; // bottom-up

    // Draw each triangle as a filled rect approximation (wireframe-style)
    for (size_t i = 0; i < mesh.indices.size(); i += 3) {
        uint32_t i0 = mesh.indices[i + 0] * 3;
        uint32_t i1 = mesh.indices[i + 1] * 3;
        uint32_t i2 = mesh.indices[i + 2] * 3;

        // Rotated X/Z → screen X, Y → screen Y
        auto proj = [&](uint32_t vi, float& sx, float& sy) {
            float x = mesh.vertices[vi + 0];
            float y = mesh.vertices[vi + 1];
            float z = mesh.vertices[vi + 2];
            float rx = x * cosA + z * sinA;
            // float rz = -x * sinA + z * cosA; // depth (unused for now)
            sx = cx + rx * scale;
            sy = cy - y * scale;
        };

        float sx0, sy0, sx1, sy1, sx2, sy2;
        proj(i0, sx0, sy0);
        proj(i1, sx1, sy1);
        proj(i2, sx2, sy2);

        // Simple edge drawing via thin rects
        auto drawEdge = [&](float ax, float ay, float bx, float by) {
            int32_t ex = static_cast<int32_t>(std::min(ax, bx));
            int32_t ey = static_cast<int32_t>(std::min(ay, by));
            int32_t ew = std::max(1, static_cast<int32_t>(std::abs(bx - ax)));
            int32_t eh = std::max(1, static_cast<int32_t>(std::abs(by - ay)));
            m_drawList.DrawRect({ex, ey, ew, eh}, {120, 200, 255, 180});
        };

        drawEdge(sx0, sy0, sx1, sy1);
        drawEdge(sx1, sy1, sx2, sy2);
        drawEdge(sx2, sy2, sx0, sy0);
    }
}

// ---------------------------------------------------------------------------
// Draw
// ---------------------------------------------------------------------------

void CharacterSelectScreen::Draw() {
    m_drawList.Clear();

    const int32_t screenW = 1020;
    const int32_t screenH = 690;

    // Background
    m_drawList.DrawRect({0, 0, screenW, screenH}, {20, 22, 26, 255});

    // Title
    m_drawList.DrawText({screenW / 2 - 120, 10, 240, 24},
                        "Character Select",
                        {220, 220, 220, 255});

    // 3D Viewport area
    int32_t vpX = 200, vpY = 50;
    int32_t vpW = 620, vpH = 500;
    m_drawList.DrawRect({vpX, vpY, vpW, vpH}, {30, 32, 38, 255});
    m_drawList.DrawBorder({vpX, vpY, vpW, vpH}, 1, {60, 63, 68, 255});

    // Grid lines
    int32_t midX = vpX + vpW / 2;
    int32_t midY = vpY + vpH / 2;
    m_drawList.DrawRect({midX, vpY, 1, vpH}, {40, 42, 46, 255});
    m_drawList.DrawRect({vpX, midY, vpW, 1}, {40, 42, 46, 255});

    // Render selected character mesh
    if (auto* slot = SelectedSlot()) {
        if (slot->meshValid) {
            ProjectAndDrawMesh(slot->mesh, vpX, vpY, vpW, vpH);
        }

        // Rotation indicator
        char buf[64];
        std::snprintf(buf, sizeof(buf), "Rotation: %.0f°", m_rotationY);
        m_drawList.DrawText({vpX + 8, vpY + vpH - 22, 200, 16}, buf,
                            {150, 150, 150, 200});

        // Character name
        m_drawList.DrawText({vpX + 8, vpY + 8, vpW - 16, 20},
                            slot->name, {200, 200, 200, 255});
    }

    // Slot list (left sidebar)
    int32_t slotY = 50;
    for (size_t i = 0; i < m_slots.size(); ++i) {
        bool selected = (i == m_selectedIndex);
        auto bg = selected ? atlas::ui::UIColor{50, 120, 220, 255}
                           : atlas::ui::UIColor{40, 42, 48, 255};
        m_drawList.DrawRect({10, slotY, 180, 36}, bg);
        m_drawList.DrawBorder({10, slotY, 180, 36}, 1, {60, 63, 68, 255});
        m_drawList.DrawText({18, slotY + 8, 164, 20},
                            m_slots[i].name,
                            selected ? atlas::ui::UIColor{255, 255, 255, 255}
                                     : atlas::ui::UIColor{180, 180, 180, 255});
        slotY += 40;
    }

    // Confirm button
    int32_t btnW = 160, btnH = 36;
    int32_t btnX = screenW / 2 - btnW / 2;
    int32_t btnY = screenH - 50;
    m_drawList.DrawRect({btnX, btnY, btnW, btnH}, {50, 140, 80, 255});
    m_drawList.DrawBorder({btnX, btnY, btnW, btnH}, 1, {80, 200, 120, 255});
    m_drawList.DrawText({btnX + 20, btnY + 8, btnW - 40, 20},
                        "Select Character", {255, 255, 255, 255});

    // Rotation hint
    m_drawList.DrawText({vpX + vpW / 2 - 80, vpY + vpH + 4, 160, 14},
                        "Drag to rotate", {100, 100, 100, 200});
}

} // namespace atlas::editor
