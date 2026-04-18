#include "GameMechanicsUIPanel.h"
#include <sstream>
#include <algorithm>

namespace atlas::editor {

void GameMechanicsUIPanel::Draw() {
    m_drawList.Clear();

    // Background
    m_drawList.DrawRect({0, 0, 600, 400}, {30, 30, 30, 255});

    // Title
    m_drawList.DrawRect({0, 0, 600, 24}, {50, 50, 50, 255});
    std::string title = m_previewMode ? "Game Mechanics UI [Preview]" : "Game Mechanics UI";
    m_drawList.DrawText({4, 4, 300, 20}, title, {220, 220, 220, 255});

    // Element list
    int32_t y = 28;
    for (const auto& elem : m_elements) {
        bool selected = (elem.id == m_selectedID);
        atlas::ui::UIColor bgColor = selected
            ? atlas::ui::UIColor{60, 80, 120, 255}
            : atlas::ui::UIColor{40, 40, 40, 255};
        m_drawList.DrawRect({0, y, 600, 20}, bgColor);

        std::string line = elem.name + " (" + elem.elementType + ")";
        m_drawList.DrawText({4, y + 2, 590, 16}, line, {200, 200, 200, 255});
        y += 22;
    }
}

uint64_t GameMechanicsUIPanel::AddElement(const MechanicsUIElement& element) {
    MechanicsUIElement e = element;
    e.id = m_nextID++;
    m_elements.push_back(e);
    return e.id;
}

bool GameMechanicsUIPanel::RemoveElement(uint64_t id) {
    auto it = std::find_if(m_elements.begin(), m_elements.end(),
                           [id](const MechanicsUIElement& e) { return e.id == id; });
    if (it != m_elements.end()) {
        if (m_selectedID == id) m_selectedID = 0;
        m_elements.erase(it);
        return true;
    }
    return false;
}

const MechanicsUIElement* GameMechanicsUIPanel::GetElement(uint64_t id) const {
    for (auto& e : m_elements) {
        if (e.id == id) return &e;
    }
    return nullptr;
}

std::vector<MechanicsUIElement> GameMechanicsUIPanel::ListElements() const {
    return m_elements;
}

size_t GameMechanicsUIPanel::ElementCount() const {
    return m_elements.size();
}

void GameMechanicsUIPanel::SelectElement(uint64_t id) {
    m_selectedID = id;
}

uint64_t GameMechanicsUIPanel::SelectedElementID() const {
    return m_selectedID;
}

void GameMechanicsUIPanel::SetPreviewMode(bool enabled) {
    m_previewMode = enabled;
}

bool GameMechanicsUIPanel::IsPreviewMode() const {
    return m_previewMode;
}

std::string GameMechanicsUIPanel::ExportLayout() const {
    std::ostringstream ss;
    ss << "{\"elements\":[";
    bool first = true;
    for (auto& e : m_elements) {
        if (!first) ss << ",";
        first = false;
        ss << "{\"id\":" << e.id
           << ",\"name\":\"" << e.name << "\""
           << ",\"type\":\"" << e.elementType << "\"}";
    }
    ss << "]}";
    return ss.str();
}

bool GameMechanicsUIPanel::ImportLayout(const std::string& json) {
    if (json.empty()) return false;
    if (json.find("\"elements\"") == std::string::npos) return false;
    return true;
}

void GameMechanicsUIPanel::Clear() {
    m_elements.clear();
    m_nextID = 1;
    m_selectedID = 0;
    m_previewMode = false;
}

} // namespace atlas::editor
