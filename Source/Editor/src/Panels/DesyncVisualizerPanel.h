#pragma once
#include "../ui/EditorPanel.h"
#include "../../engine/ui/UIDrawList.h"
#include "../../engine/sim/SimMirror.h"
#include <cstdint>
#include <string>
#include <vector>

namespace atlas::editor {

/// Per-field desync detail for visualization
struct DesyncFieldDetail {
    std::string entityName;
    std::string componentName;
    std::string fieldName;
    std::string serverValue;
    std::string clientValue;
};

/// A single desync event with optional field-level detail
struct DesyncDisplayEvent {
    uint64_t tick = 0;
    uint64_t serverHash = 0;
    uint64_t clientHash = 0;
    std::vector<DesyncFieldDetail> details;
};

/// Live net-desync visualizer panel
/// Displays desync events from a SimMirrorController with per-field
/// breakdowns when available.
class DesyncVisualizerPanel : public EditorPanel {
public:
    const char* Name() const override { return "Desync Visualizer"; }
    void Draw() override;

    /// Feed desync events from the mirror controller
    void SetMirrorController(const atlas::sim::SimMirrorController* controller);

    /// Add a desync event with optional field-level detail
    void AddEvent(const DesyncDisplayEvent& event);

    /// Clear all events
    void ClearEvents();

    /// Get all events
    const std::vector<DesyncDisplayEvent>& Events() const;

    /// Select an event by index for detail view
    void SelectEvent(size_t index);
    size_t SelectedIndex() const;

    /// Access the draw list for testing
    const atlas::ui::UIDrawList& GetDrawList() const { return m_drawList; }

private:
    const atlas::sim::SimMirrorController* m_controller = nullptr;
    std::vector<DesyncDisplayEvent> m_events;
    size_t m_selectedIndex = 0;
    atlas::ui::UIDrawList m_drawList;
};

}  // namespace atlas::editor
