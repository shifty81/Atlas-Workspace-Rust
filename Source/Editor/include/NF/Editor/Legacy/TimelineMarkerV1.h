#pragma once
// NF::Editor — Timeline marker v1: event markers, bookmarks, and range labels
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Tmv1MarkerType   : uint8_t { Event, Bookmark, Chapter, Warning, Note };
enum class Tmv1RangeType    : uint8_t { Section, Loop, Fade, Highlight };

inline const char* tmv1MarkerTypeName(Tmv1MarkerType t) {
    switch (t) {
        case Tmv1MarkerType::Event:    return "Event";
        case Tmv1MarkerType::Bookmark: return "Bookmark";
        case Tmv1MarkerType::Chapter:  return "Chapter";
        case Tmv1MarkerType::Warning:  return "Warning";
        case Tmv1MarkerType::Note:     return "Note";
    }
    return "Unknown";
}

inline const char* tmv1RangeTypeName(Tmv1RangeType t) {
    switch (t) {
        case Tmv1RangeType::Section:   return "Section";
        case Tmv1RangeType::Loop:      return "Loop";
        case Tmv1RangeType::Fade:      return "Fade";
        case Tmv1RangeType::Highlight: return "Highlight";
    }
    return "Unknown";
}

struct Tmv1Marker {
    uint64_t        id       = 0;
    std::string     label;
    Tmv1MarkerType  type     = Tmv1MarkerType::Event;
    float           time     = 0.f;
    bool            pinned   = false;

    [[nodiscard]] bool isValid() const { return id != 0 && !label.empty() && time >= 0.f; }
};

struct Tmv1Range {
    uint64_t       id        = 0;
    std::string    label;
    Tmv1RangeType  type      = Tmv1RangeType::Section;
    float          startTime = 0.f;
    float          endTime   = 1.f;

    [[nodiscard]] bool  isValid()    const { return id != 0 && !label.empty() && endTime > startTime; }
    [[nodiscard]] float duration()   const { return endTime - startTime; }
};

using Tmv1ChangeCallback = std::function<void()>;

class TimelineMarkerV1 {
public:
    static constexpr size_t MAX_MARKERS = 1024;
    static constexpr size_t MAX_RANGES  = 256;

    bool addMarker(const Tmv1Marker& marker) {
        if (!marker.isValid()) return false;
        for (const auto& m : m_markers) if (m.id == marker.id) return false;
        if (m_markers.size() >= MAX_MARKERS) return false;
        m_markers.push_back(marker);
        if (m_onChange) m_onChange();
        return true;
    }

    bool removeMarker(uint64_t id) {
        for (auto it = m_markers.begin(); it != m_markers.end(); ++it) {
            if (it->id == id) { m_markers.erase(it); if (m_onChange) m_onChange(); return true; }
        }
        return false;
    }

    [[nodiscard]] const Tmv1Marker* findMarker(uint64_t id) const {
        for (const auto& m : m_markers) if (m.id == id) return &m;
        return nullptr;
    }

    bool addRange(const Tmv1Range& range) {
        if (!range.isValid()) return false;
        for (const auto& r : m_ranges) if (r.id == range.id) return false;
        if (m_ranges.size() >= MAX_RANGES) return false;
        m_ranges.push_back(range);
        if (m_onChange) m_onChange();
        return true;
    }

    bool removeRange(uint64_t id) {
        for (auto it = m_ranges.begin(); it != m_ranges.end(); ++it) {
            if (it->id == id) { m_ranges.erase(it); if (m_onChange) m_onChange(); return true; }
        }
        return false;
    }

    [[nodiscard]] const Tmv1Range* findRange(uint64_t id) const {
        for (const auto& r : m_ranges) if (r.id == id) return &r;
        return nullptr;
    }

    [[nodiscard]] size_t markerCount() const { return m_markers.size(); }
    [[nodiscard]] size_t rangeCount()  const { return m_ranges.size(); }
    [[nodiscard]] size_t countMarkersByType(Tmv1MarkerType t) const {
        size_t c = 0; for (const auto& m : m_markers) if (m.type == t) ++c; return c;
    }
    [[nodiscard]] size_t countRangesByType(Tmv1RangeType t) const {
        size_t c = 0; for (const auto& r : m_ranges) if (r.type == t) ++c; return c;
    }
    [[nodiscard]] size_t pinnedCount() const {
        size_t c = 0; for (const auto& m : m_markers) if (m.pinned) ++c; return c;
    }
    [[nodiscard]] std::vector<const Tmv1Marker*> markersInRange(float start, float end) const {
        std::vector<const Tmv1Marker*> result;
        for (const auto& m : m_markers)
            if (m.time >= start && m.time <= end) result.push_back(&m);
        return result;
    }

    void setOnChange(Tmv1ChangeCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Tmv1Marker> m_markers;
    std::vector<Tmv1Range>  m_ranges;
    Tmv1ChangeCallback      m_onChange;
};

} // namespace NF
