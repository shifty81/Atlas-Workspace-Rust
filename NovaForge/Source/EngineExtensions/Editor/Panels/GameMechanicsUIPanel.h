#pragma once
#include "../ui/EditorPanel.h"
#include "../../engine/ui/UIDrawList.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>

namespace atlas::editor {

struct MechanicsUIElement {
    uint64_t id = 0;
    std::string name;
    std::string elementType;
    std::unordered_map<std::string, std::string> properties;
};

class GameMechanicsUIPanel : public EditorPanel {
public:
    const char* Name() const override { return "Game Mechanics UI"; }
    void Draw() override;

    uint64_t AddElement(const MechanicsUIElement& element);
    bool RemoveElement(uint64_t id);
    const MechanicsUIElement* GetElement(uint64_t id) const;
    std::vector<MechanicsUIElement> ListElements() const;
    size_t ElementCount() const;

    void SelectElement(uint64_t id);
    uint64_t SelectedElementID() const;

    void SetPreviewMode(bool enabled);
    bool IsPreviewMode() const;

    std::string ExportLayout() const;
    bool ImportLayout(const std::string& json);

    void Clear();

    const atlas::ui::UIDrawList& GetDrawList() const { return m_drawList; }

private:
    std::vector<MechanicsUIElement> m_elements;
    uint64_t m_nextID = 1;
    uint64_t m_selectedID = 0;
    bool m_previewMode = false;
    atlas::ui::UIDrawList m_drawList;
};

} // namespace atlas::editor
