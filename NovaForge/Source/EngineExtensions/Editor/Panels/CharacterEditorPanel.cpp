#include "CharacterEditorPanel.h"
#include <cstdio>
#include <cmath>
#include <algorithm>

namespace atlas::editor {

// --- Animation helpers ---

void CharacterEditorPanel::SetAnimState(animation::CharacterAnimState s) {
    m_animController.SetState(s);
}

animation::CharacterAnimState CharacterEditorPanel::AnimState() const {
    return m_animController.CurrentState();
}

void CharacterEditorPanel::UpdateAnimation(float dt) {
    m_animController.Update(dt);
}

// --- Mesh generation ---

bool CharacterEditorPanel::RegenerateMesh() {
    if (!m_dirty) return false;
    m_mesh = character::HumanMeshGenerator::Generate(m_params);
    m_meshValid = m_mesh.IsValid();
    m_dirty = false;
    if (m_onParamsChanged) {
        m_onParamsChanged(m_params);
    }
    return true;
}

std::string CharacterEditorPanel::Summary() const {
    std::ostringstream oss;
    oss << "Height: " << m_params.height
        << "m, Mass: " << m_params.mass
        << "kg";
    if (m_meshValid) {
        oss << " | Verts: " << m_mesh.VertexCount()
            << ", Tris: " << m_mesh.TriangleCount();
    }
    return oss.str();
}

// --- Draw helpers ---

void CharacterEditorPanel::DrawSlider(int32_t x, int32_t y, int32_t w,
                                       const char* label, float value,
                                       float minVal, float maxVal) {
    char buf[128];
    std::snprintf(buf, sizeof(buf), "%s: %.2f", label, value);
    m_drawList.DrawText({x, y, 140, 16}, buf, {200, 200, 200, 255});

    // Track
    int32_t trackX = x + 150;
    int32_t trackW = w - 150;
    m_drawList.DrawRect({trackX, y + 5, trackW, 6}, {50, 53, 58, 255});

    // Fill
    float t = (maxVal > minVal) ? (value - minVal) / (maxVal - minVal) : 0.0f;
    t = std::clamp(t, 0.0f, 1.0f);
    int32_t fillW = static_cast<int32_t>(t * trackW);
    m_drawList.DrawRect({trackX, y + 5, fillW, 6}, {66, 135, 245, 255});

    // Thumb
    int32_t thumbX = trackX + fillW - 4;
    m_drawList.DrawRect({thumbX, y + 2, 8, 12}, {200, 200, 200, 255});
}

// --- Draw ---

void CharacterEditorPanel::Draw() {
    m_drawList.Clear();

    const int32_t panelW = 360;
    const int32_t panelH = 600;

    // Background
    m_drawList.DrawRect({0, 0, panelW, panelH}, {35, 37, 42, 255});

    // Header
    m_drawList.DrawRect({0, 0, panelW, 28}, {45, 48, 54, 255});
    m_drawList.DrawText({8, 4, panelW - 16, 20}, "Character Editor",
                        {220, 220, 220, 255});

    int32_t y = 32;

    // Section tabs
    DrawSectionTabs(y);

    // Section content
    switch (m_section) {
        case CharacterEditorSection::Body:      DrawBodySection(y);      break;
        case CharacterEditorSection::Face:       DrawFaceSection(y);      break;
        case CharacterEditorSection::Hair:       DrawHairSection(y);      break;
        case CharacterEditorSection::Material:   DrawMaterialSection(y);  break;
        case CharacterEditorSection::Animation:  DrawAnimationSection(y); break;
    }

    // Summary at bottom
    m_drawList.DrawRect({0, panelH - 24, panelW, 24}, {40, 42, 48, 255});
    m_drawList.DrawText({8, panelH - 20, panelW - 16, 16},
                        Summary(), {160, 160, 160, 255});
}

void CharacterEditorPanel::DrawSectionTabs(int32_t& y) {
    const char* labels[] = {"Body", "Face", "Hair", "Material", "Anim"};
    int32_t tx = 0;
    int32_t tabW = 72;
    m_drawList.DrawRect({0, y, 360, 26}, {38, 40, 45, 255});
    for (int i = 0; i < 5; ++i) {
        bool active = (static_cast<int>(m_section) == i);
        auto bg = active ? atlas::ui::UIColor{50, 120, 220, 255}
                         : atlas::ui::UIColor{42, 44, 50, 255};
        m_drawList.DrawRect({tx, y, tabW, 24}, bg);
        m_drawList.DrawText({tx + 4, y + 4, tabW - 8, 16}, labels[i],
                            active ? atlas::ui::UIColor{255, 255, 255, 255}
                                   : atlas::ui::UIColor{160, 160, 160, 255});
        tx += tabW;
    }
    y += 30;
}

void CharacterEditorPanel::DrawBodySection(int32_t y) {
    int32_t x = 8, w = 344;
    m_drawList.DrawText({x, y, w, 18}, "Body Proportions", {180, 180, 180, 255});
    y += 22;
    DrawSlider(x, y, w, "Height",    m_params.height,   1.5f, 2.1f);  y += 22;
    DrawSlider(x, y, w, "Mass",      m_params.mass,     50.0f, 120.0f); y += 22;
    DrawSlider(x, y, w, "Arm Ratio", m_params.armRatio, 0.8f, 1.2f);  y += 22;
    DrawSlider(x, y, w, "Leg Ratio", m_params.legRatio, 0.9f, 1.1f);  y += 22;
}

void CharacterEditorPanel::DrawFaceSection(int32_t y) {
    int32_t x = 8, w = 344;
    m_drawList.DrawText({x, y, w, 18}, "Facial Features", {180, 180, 180, 255});
    y += 22;
    DrawSlider(x, y, w, "Nose Width",   m_params.noseWidth,   0.3f, 0.7f); y += 22;
    DrawSlider(x, y, w, "Eye Spacing",  m_params.eyeSpacing,  0.4f, 0.7f); y += 22;
    DrawSlider(x, y, w, "Jaw Width",    m_params.jawWidth,    0.5f, 1.0f); y += 22;
    DrawSlider(x, y, w, "Brow Height",  m_params.browHeight,  0.2f, 0.7f); y += 22;
    DrawSlider(x, y, w, "Lip Fullness", m_params.lipFullness, 0.2f, 0.8f); y += 22;
}

void CharacterEditorPanel::DrawHairSection(int32_t y) {
    int32_t x = 8, w = 344;
    m_drawList.DrawText({x, y, w, 18}, "Hair Style", {180, 180, 180, 255});
    y += 22;
    DrawSlider(x, y, w, "Style",   m_params.hairStyle,   0.0f, 7.0f); y += 22;
    DrawSlider(x, y, w, "Length",  m_params.hairLength,   0.1f, 1.0f); y += 22;
    DrawSlider(x, y, w, "Density", m_params.hairDensity,  0.3f, 1.0f); y += 22;

    m_drawList.DrawText({x, y, w, 18}, "Hair Color", {180, 180, 180, 255});
    y += 22;
    DrawSlider(x, y, w, "Red",   m_params.hairR, 0.0f, 1.0f); y += 22;
    DrawSlider(x, y, w, "Green", m_params.hairG, 0.0f, 1.0f); y += 22;
    DrawSlider(x, y, w, "Blue",  m_params.hairB, 0.0f, 1.0f); y += 22;
}

void CharacterEditorPanel::DrawMaterialSection(int32_t y) {
    int32_t x = 8, w = 344;
    m_drawList.DrawText({x, y, w, 18}, "Material / Appearance", {180, 180, 180, 255});
    y += 22;
    DrawSlider(x, y, w, "Skin Tone",  m_params.skinTone,  0.0f, 1.0f); y += 22;
    DrawSlider(x, y, w, "Hair Color", m_params.hairColor, 0.0f, 1.0f); y += 22;
    DrawSlider(x, y, w, "Eye Color",  m_params.eyeColor,  0.0f, 1.0f); y += 22;
}

void CharacterEditorPanel::DrawAnimationSection(int32_t y) {
    int32_t x = 8, w = 344;
    m_drawList.DrawText({x, y, w, 18}, "Animation Preview", {180, 180, 180, 255});
    y += 22;

    const char* states[] = {"Idle", "Walk", "Run", "Attack", "Hit", "Death"};
    int32_t btnW = 54;
    for (int i = 0; i < 6; ++i) {
        bool active = (static_cast<int>(m_animController.CurrentState()) == i);
        auto bg = active ? atlas::ui::UIColor{50, 140, 80, 255}
                         : atlas::ui::UIColor{50, 53, 58, 255};
        m_drawList.DrawRect({x, y, btnW, 22}, bg);
        m_drawList.DrawBorder({x, y, btnW, 22}, 1, {70, 73, 78, 255});
        m_drawList.DrawText({x + 4, y + 3, btnW - 8, 16}, states[i],
                            {200, 200, 200, 255});
        x += btnW + 4;
    }
    y += 28;

    x = 8;
    DrawSlider(x, y, w, "Speed", m_animController.Speed(), 0.1f, 3.0f);
    y += 22;

    char buf[64];
    std::snprintf(buf, sizeof(buf), "Tick: %u  Time: %.2fs",
                  m_animController.Tick(), m_animController.ElapsedTime());
    m_drawList.DrawText({x, y, w, 16}, buf, {140, 140, 140, 255});
}

} // namespace atlas::editor
