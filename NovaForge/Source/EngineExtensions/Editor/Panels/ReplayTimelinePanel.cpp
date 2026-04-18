#include "ReplayTimelinePanel.h"
#include <algorithm>
#include <cstdio>

namespace atlas::editor {

void ReplayTimelinePanel::Draw() {
    m_drawList.Clear();

    // Background
    m_drawList.DrawRect({0, 0, 800, 120}, {30, 30, 30, 255});

    // Title
    m_drawList.DrawRect({0, 0, 800, 24}, {50, 50, 50, 255});
    std::string title = "Replay Timeline  Tick: " + std::to_string(m_currentTick)
        + " / " + std::to_string(TotalTicks())
        + "  Rate: " + std::to_string(m_tickRate) + " Hz";
    m_drawList.DrawText({4, 4, 790, 20}, title, {220, 220, 220, 255});

    // Timeline bar
    m_drawList.DrawRect({4, 32, 792, 20}, {60, 60, 60, 255});
    if (TotalTicks() > 0) {
        int32_t progressW = static_cast<int32_t>(
            792.0 * m_currentTick / TotalTicks());
        m_drawList.DrawRect({4, 32, progressW, 20}, {80, 140, 200, 255});
    }

    // Markers
    for (const auto& marker : m_markers) {
        if (TotalTicks() == 0) break;
        int32_t mx = 4 + static_cast<int32_t>(792.0 * marker.tick / TotalTicks());
        atlas::ui::UIColor markerColor = {200, 200, 200, 255};
        switch (marker.type) {
            case MarkerType::Bookmark:   markerColor = {100, 200, 255, 255}; break;
            case MarkerType::Branch:     markerColor = {100, 255, 100, 255}; break;
            case MarkerType::Injection:  markerColor = {255, 200, 100, 255}; break;
            case MarkerType::Divergence: markerColor = {255, 80, 80, 255};   break;
        }
        m_drawList.DrawRect({mx, 28, 3, 28}, markerColor);
        m_drawList.DrawText({mx + 4, 56, 100, 14}, marker.label, markerColor);
    }

    // Divergence indicator
    if (HasDivergence()) {
        std::string divText = "DIVERGENCE at tick " + std::to_string(DivergenceTick());
        m_drawList.DrawText({4, 80, 400, 16}, divText, {255, 80, 80, 255});
    }
}

void ReplayTimelinePanel::LoadReplay(const std::vector<atlas::sim::ReplayFrame>& frames,
                                     uint32_t tickRate) {
    m_frames = frames;
    m_tickRate = tickRate;
    m_currentTick = 0;
    m_markers.clear();
    m_hasComparison = false;
    m_comparisonResult = ComparisonResult{};
}

void ReplayTimelinePanel::SetCurrentTick(uint32_t tick) {
    if (m_frames.empty()) {
        m_currentTick = 0;
        return;
    }
    m_currentTick = std::min(tick, m_frames.back().tick);
}

uint32_t ReplayTimelinePanel::GetCurrentTick() const {
    return m_currentTick;
}

uint32_t ReplayTimelinePanel::TotalTicks() const {
    if (m_frames.empty()) return 0;
    return m_frames.back().tick + 1;
}

uint32_t ReplayTimelinePanel::TickRate() const {
    return m_tickRate;
}

const atlas::sim::ReplayFrame* ReplayTimelinePanel::CurrentFrame() const {
    if (m_frames.empty()) return nullptr;
    atlas::sim::ReplayFrame key;
    key.tick = m_currentTick;
    auto it = std::lower_bound(m_frames.begin(), m_frames.end(), key,
        [](const atlas::sim::ReplayFrame& a, const atlas::sim::ReplayFrame& b) {
            return a.tick < b.tick;
        });
    if (it != m_frames.end() && it->tick == m_currentTick) return &(*it);
    return nullptr;
}

// --- Markers ---

void ReplayTimelinePanel::AddMarker(const TimelineMarker& marker) {
    m_markers.push_back(marker);
}

void ReplayTimelinePanel::RemoveMarker(uint32_t tick) {
    m_markers.erase(
        std::remove_if(m_markers.begin(), m_markers.end(),
                       [tick](const TimelineMarker& m) { return m.tick == tick; }),
        m_markers.end());
}

const std::vector<TimelineMarker>& ReplayTimelinePanel::Markers() const {
    return m_markers;
}

std::vector<TimelineMarker> ReplayTimelinePanel::MarkersInRange(uint32_t start,
                                                                 uint32_t end) const {
    std::vector<TimelineMarker> result;
    for (const auto& m : m_markers) {
        if (m.tick >= start && m.tick <= end) {
            result.push_back(m);
        }
    }
    return result;
}

// --- Comparison ---

ComparisonResult ReplayTimelinePanel::CompareWith(
    const std::vector<atlas::sim::ReplayFrame>& other) const {
    ComparisonResult result;

    if (m_frames.empty() && other.empty()) {
        m_comparisonResult = result;
        m_hasComparison = true;
        return result;
    }

    result.firstTick = 0;
    size_t minLen = std::min(m_frames.size(), other.size());
    size_t maxLen = std::max(m_frames.size(), other.size());
    result.lastTick = static_cast<int64_t>(maxLen > 0 ? maxLen - 1 : 0);

    size_t matchCount = 0;
    result.divergeTick = -1;

    for (size_t i = 0; i < minLen; ++i) {
        if (m_frames[i].stateHash == other[i].stateHash &&
            m_frames[i].inputData == other[i].inputData) {
            ++matchCount;
        } else if (result.divergeTick < 0) {
            result.divergeTick = static_cast<int64_t>(m_frames[i].tick);
        }
    }

    // Frames beyond the shorter replay count as divergent
    if (minLen < maxLen && result.divergeTick < 0) {
        // Use the tick from the frame at the boundary, or fall back to index
        if (minLen < m_frames.size()) {
            result.divergeTick = static_cast<int64_t>(m_frames[minLen].tick);
        } else if (minLen < other.size()) {
            result.divergeTick = static_cast<int64_t>(other[minLen].tick);
        } else {
            result.divergeTick = static_cast<int64_t>(minLen);
        }
    }

    result.matchPercentage = maxLen > 0
        ? (static_cast<double>(matchCount) / static_cast<double>(maxLen)) * 100.0
        : 100.0;

    m_comparisonResult = result;
    m_hasComparison = true;
    return result;
}

bool ReplayTimelinePanel::HasDivergence() const {
    return m_hasComparison && m_comparisonResult.divergeTick >= 0;
}

int64_t ReplayTimelinePanel::DivergenceTick() const {
    if (!m_hasComparison) return -1;
    return m_comparisonResult.divergeTick;
}

// --- Input injection ---

void ReplayTimelinePanel::InjectInput(uint32_t tick,
                                      const std::vector<uint8_t>& inputData) {
    for (auto& f : m_frames) {
        if (f.tick == tick) {
            f.inputData = inputData;
            return;
        }
    }
    // If no frame exists at this tick, insert in sorted order
    atlas::sim::ReplayFrame newFrame;
    newFrame.tick = tick;
    newFrame.inputData = inputData;
    auto it = std::lower_bound(m_frames.begin(), m_frames.end(), newFrame,
        [](const atlas::sim::ReplayFrame& a, const atlas::sim::ReplayFrame& b) {
            return a.tick < b.tick;
        });
    m_frames.insert(it, newFrame);
}

bool ReplayTimelinePanel::HasInjectedInputs() const {
    // Any frame with non-empty inputData that was modified counts;
    // for simplicity we track whether any frame has data at all.
    for (const auto& f : m_frames) {
        if (!f.inputData.empty()) return true;
    }
    return false;
}

// --- Branching ---

std::vector<atlas::sim::ReplayFrame> ReplayTimelinePanel::BranchAt(uint32_t tick) {
    std::vector<atlas::sim::ReplayFrame> branch;
    for (const auto& f : m_frames) {
        if (f.tick <= tick) {
            branch.push_back(f);
        }
    }
    return branch;
}

void ReplayTimelinePanel::SetOnDivergenceDetected(DivergenceCallback callback) {
    m_divergenceCallback = std::move(callback);
}

void ReplayTimelinePanel::CheckAndNotifyDivergence() {
    if (HasDivergence() && m_divergenceCallback) {
        m_divergenceCallback(DivergenceTick());
    }
}

// --- Replay Inspector: Input Frame Viewer ---

static std::string BytesToHex(const std::vector<uint8_t>& data, size_t maxBytes) {
    std::string hex;
    size_t limit = std::min(data.size(), maxBytes);
    for (size_t i = 0; i < limit; ++i) {
        char buf[4];
        std::snprintf(buf, sizeof(buf), "%02X", data[i]);
        if (!hex.empty()) hex += ' ';
        hex += buf;
    }
    return hex;
}

std::vector<InputFrameEntry> ReplayTimelinePanel::GetInputFrames(
    uint32_t startTick, uint32_t endTick) const {
    std::vector<InputFrameEntry> entries;
    for (const auto& f : m_frames) {
        if (f.tick >= startTick && f.tick <= endTick) {
            InputFrameEntry e;
            e.tick = f.tick;
            e.dataSize = f.inputData.size();
            e.stateHash = f.stateHash;
            e.isSavePoint = f.isSavePoint;
            e.hexPreview = BytesToHex(f.inputData, 16);
            entries.push_back(e);
        }
    }
    return entries;
}

InputFrameEntry ReplayTimelinePanel::GetInputFrameAt(uint32_t tick) const {
    InputFrameEntry e;
    e.tick = tick;
    for (const auto& f : m_frames) {
        if (f.tick == tick) {
            e.dataSize = f.inputData.size();
            e.stateHash = f.stateHash;
            e.isSavePoint = f.isSavePoint;
            e.hexPreview = BytesToHex(f.inputData, 16);
            return e;
        }
    }
    return e;
}

// --- Replay Inspector: Event Timeline ---

std::vector<TimelineEvent> ReplayTimelinePanel::BuildEventTimeline() const {
    std::vector<TimelineEvent> events;

    // Add input events from frames
    for (const auto& f : m_frames) {
        if (!f.inputData.empty()) {
            TimelineEvent ev;
            ev.tick = f.tick;
            ev.type = TimelineEventType::Input;
            ev.description = "Input (" + std::to_string(f.inputData.size()) + " bytes)";
            events.push_back(ev);
        }
        if (f.isSavePoint) {
            TimelineEvent ev;
            ev.tick = f.tick;
            ev.type = TimelineEventType::SavePoint;
            ev.description = "Save Point";
            events.push_back(ev);
        }
    }

    // Add marker-based events
    for (const auto& m : m_markers) {
        TimelineEvent ev;
        ev.tick = m.tick;
        ev.description = m.label;
        switch (m.type) {
            case MarkerType::Branch:
                ev.type = TimelineEventType::Branch;
                break;
            case MarkerType::Divergence:
                ev.type = TimelineEventType::Divergence;
                break;
            default:
                continue; // skip bookmarks and injections in event timeline
        }
        events.push_back(ev);
    }

    // Sort by tick
    std::sort(events.begin(), events.end(),
              [](const TimelineEvent& a, const TimelineEvent& b) {
                  return a.tick < b.tick;
              });

    return events;
}

// --- Replay Inspector: Branch Point Markers ---

std::vector<TimelineMarker> ReplayTimelinePanel::BranchPoints() const {
    std::vector<TimelineMarker> points;
    for (const auto& m : m_markers) {
        if (m.type == MarkerType::Branch) {
            points.push_back(m);
        }
    }
    return points;
}

void ReplayTimelinePanel::AddBranchPoint(uint32_t tick, const std::string& label) {
    TimelineMarker marker;
    marker.tick = tick;
    marker.label = label;
    marker.type = MarkerType::Branch;
    m_markers.push_back(marker);
}

}
