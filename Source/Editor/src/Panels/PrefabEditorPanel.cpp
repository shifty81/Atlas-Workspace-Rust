#include "PrefabEditorPanel.h"

namespace atlas::editor {

void PrefabEditorPanel::Draw() {
    m_drawList.Clear();

    // Background
    m_drawList.DrawRect({0, 0, 600, 400}, {30, 30, 30, 255});

    // Title bar
    m_drawList.DrawRect({0, 0, 600, 24}, {50, 50, 50, 255});
    std::string title = "Prefab Editor";
    if (m_dirty) title += " *";
    m_drawList.DrawText({4, 4, 300, 20}, title, {220, 220, 220, 255});

    if (m_entities.empty()) {
        m_drawList.DrawText({4, 30, 590, 16}, "Empty prefab", {160, 160, 160, 255});
        return;
    }

    // Entity hierarchy section header
    int32_t y = 28;
    m_drawList.DrawRect({0, y, 300, 20}, {45, 45, 45, 255});
    m_drawList.DrawText({4, y + 2, 290, 16}, "Hierarchy", {180, 200, 220, 255});
    y += 22;

    // Draw root entities and their children recursively
    auto roots = RootEntities();
    for (uint32_t rootId : roots) {
        y = DrawEntityRow(rootId, 0, y);
    }

    // Component inspector section (right side)
    if (m_selectedEntity > 0) {
        auto it = m_entities.find(m_selectedEntity);
        if (it != m_entities.end()) {
            const auto& entity = it->second;
            int32_t cx = 310;
            int32_t cy = 28;

            // Component section header
            m_drawList.DrawRect({cx, cy, 290, 20}, {45, 45, 45, 255});
            std::string compHeader = "Components: " + entity.name;
            m_drawList.DrawText({cx + 4, cy + 2, 280, 16}, compHeader, {180, 200, 220, 255});
            cy += 22;

            if (entity.components.empty()) {
                m_drawList.DrawText({cx + 4, cy, 280, 16}, "(no components)", {120, 120, 120, 255});
            } else {
                for (const auto& comp : entity.components) {
                    m_drawList.DrawRect({cx, cy, 290, 20}, {40, 40, 40, 255});
                    m_drawList.DrawText({cx + 4, cy + 2, 280, 16}, comp.type, {200, 200, 200, 255});
                    cy += 22;

                    // Show properties
                    for (const auto& [key, value] : comp.properties) {
                        std::string propLine = "  " + key + ": " + value;
                        m_drawList.DrawText({cx + 12, cy, 270, 16}, propLine, {160, 160, 160, 255});
                        cy += 18;
                    }
                }
            }
        }
    }
}

int32_t PrefabEditorPanel::DrawEntityRow(uint32_t entityId, int depth, int32_t y) {
    auto it = m_entities.find(entityId);
    if (it == m_entities.end()) return y;

    const auto& entity = it->second;
    bool selected = (entityId == m_selectedEntity);
    int32_t indent = depth * 16;

    atlas::ui::UIColor bgColor = selected
        ? atlas::ui::UIColor{60, 80, 120, 255}
        : atlas::ui::UIColor{40, 40, 40, 255};
    m_drawList.DrawRect({indent, y, 300 - indent, 20}, bgColor);

    std::string prefix = Children(entityId).empty() ? "  " : "> ";
    std::string label = prefix + entity.name;
    m_drawList.DrawText({indent + 4, y + 2, 290 - indent, 16}, label, {200, 200, 200, 255});
    y += 22;

    // Draw children
    auto kids = Children(entityId);
    for (uint32_t childId : kids) {
        y = DrawEntityRow(childId, depth + 1, y);
    }

    return y;
}

} // namespace atlas::editor
