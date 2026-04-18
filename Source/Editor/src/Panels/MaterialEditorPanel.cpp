#include "MaterialEditorPanel.h"

namespace atlas::editor {

void MaterialEditorPanel::Draw() {
    m_drawList.Clear();

    // Background
    m_drawList.DrawRect({0, 0, 600, 400}, {30, 30, 30, 255});

    // Title bar
    m_drawList.DrawRect({0, 0, 600, 24}, {50, 50, 50, 255});
    std::string title = m_previewMode ? "Material Editor [Preview]" : "Material Editor";
    m_drawList.DrawText({4, 4, 300, 20}, title, {220, 220, 220, 255});

    if (!m_loaded) return;

    // Parameter list
    int32_t y = 28;
    for (size_t i = 0; i < m_parameters.size(); ++i) {
        const auto& param = m_parameters[i];
        bool selected = (static_cast<int>(i) == m_selectedParam);
        atlas::ui::UIColor bgColor = selected
            ? atlas::ui::UIColor{60, 80, 120, 255}
            : atlas::ui::UIColor{40, 40, 40, 255};
        m_drawList.DrawRect({0, y, 600, 20}, bgColor);

        std::string line = param.name + " (" + param.type + ") = " + param.value;
        m_drawList.DrawText({4, y + 2, 590, 16}, line, {200, 200, 200, 255});
        y += 22;
    }
}

} // namespace atlas::editor
