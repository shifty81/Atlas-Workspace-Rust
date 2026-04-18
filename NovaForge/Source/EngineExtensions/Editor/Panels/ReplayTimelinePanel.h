#pragma once
#include "../ui/EditorPanel.h"
#include "../../engine/sim/ReplayRecorder.h"
#include "../../engine/ui/UIDrawList.h"
#include <string>
#include <vector>
#include <cstdint>
#include <functional>

namespace atlas::editor {

enum class MarkerType {
    Bookmark,
    Branch,
    Injection,
    Divergence
};

struct TimelineMarker {
    uint32_t tick = 0;
    std::string label;
    MarkerType type = MarkerType::Bookmark;
};

struct ComparisonResult {
    int64_t firstTick = 0;
    int64_t lastTick = 0;
    int64_t divergeTick = -1;
    double matchPercentage = 100.0;
};

/// Snapshot of a single input frame for the Input Frame Viewer.
struct InputFrameEntry {
    uint32_t tick = 0;
    size_t dataSize = 0;
    uint64_t stateHash = 0;
    bool isSavePoint = false;
    std::string hexPreview; // first N bytes as hex string
};

/// A single event on the Event Timeline.
enum class TimelineEventType {
    Input,
    Divergence,
    Branch,
    SavePoint,
};

struct TimelineEvent {
    uint32_t tick = 0;
    TimelineEventType type = TimelineEventType::Input;
    std::string description;
};

class ReplayTimelinePanel : public EditorPanel {
public:
    const char* Name() const override { return "Replay Timeline"; }
    void Draw() override;

    // Load / query
    void LoadReplay(const std::vector<atlas::sim::ReplayFrame>& frames, uint32_t tickRate);
    void SetCurrentTick(uint32_t tick);
    uint32_t GetCurrentTick() const;
    uint32_t TotalTicks() const;
    uint32_t TickRate() const;
    const atlas::sim::ReplayFrame* CurrentFrame() const;

    // Markers
    void AddMarker(const TimelineMarker& marker);
    void RemoveMarker(uint32_t tick);
    const std::vector<TimelineMarker>& Markers() const;
    std::vector<TimelineMarker> MarkersInRange(uint32_t start, uint32_t end) const;

    // Comparison
    ComparisonResult CompareWith(const std::vector<atlas::sim::ReplayFrame>& other) const;
    bool HasDivergence() const;
    int64_t DivergenceTick() const;

    // Divergence callback
    using DivergenceCallback = std::function<void(int64_t divergenceTick)>;
    void SetOnDivergenceDetected(DivergenceCallback callback);
    void CheckAndNotifyDivergence();

    // Input injection
    void InjectInput(uint32_t tick, const std::vector<uint8_t>& inputData);
    bool HasInjectedInputs() const;

    // Branching
    std::vector<atlas::sim::ReplayFrame> BranchAt(uint32_t tick);

    // --- Replay Inspector: Input Frame Viewer ---
    std::vector<InputFrameEntry> GetInputFrames(uint32_t startTick, uint32_t endTick) const;
    InputFrameEntry GetInputFrameAt(uint32_t tick) const;

    // --- Replay Inspector: Event Timeline ---
    std::vector<TimelineEvent> BuildEventTimeline() const;

    // --- Replay Inspector: Branch Point Markers ---
    std::vector<TimelineMarker> BranchPoints() const;
    void AddBranchPoint(uint32_t tick, const std::string& label);

    const atlas::ui::UIDrawList& GetDrawList() const { return m_drawList; }

private:
    std::vector<atlas::sim::ReplayFrame> m_frames;
    std::vector<TimelineMarker> m_markers;
    uint32_t m_currentTick = 0;
    uint32_t m_tickRate = 0;
    mutable ComparisonResult m_comparisonResult;
    mutable bool m_hasComparison = false;
    DivergenceCallback m_divergenceCallback;
    atlas::ui::UIDrawList m_drawList;
};

}
