#pragma once
// NF::Workspace — Phase 28: Workspace Minimap / Overview
//
// Workspace-level minimap region and viewport tracking:
//   MinimapRegion   — id + label + rect (normalized 0-1) + color + visible; isValid()
//   MinimapViewport — tracks the currently visible viewport window (normalized rect)
//   MinimapManager  — region registry with viewport sync and observers

#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

// ═════════════════════════════════════════════════════════════════
// MinimapRect — normalized [0,1] float rectangle
// ═════════════════════════════════════════════════════════════════

struct MinimapRect {
    float x = 0.0f;
    float y = 0.0f;
    float w = 0.0f;
    float h = 0.0f;

    bool isValid() const { return w > 0.0f && h > 0.0f; }

    bool operator==(const MinimapRect& o) const {
        return x == o.x && y == o.y && w == o.w && h == o.h;
    }
    bool operator!=(const MinimapRect& o) const { return !(*this == o); }
};

// ═════════════════════════════════════════════════════════════════
// MinimapRegion — a labeled area on the minimap
// ═════════════════════════════════════════════════════════════════

struct MinimapRegion {
    std::string  id;
    std::string  label;
    MinimapRect  rect;
    uint32_t     color   = 0xFFFFFFFF; // AARRGGBB
    bool         visible = true;

    bool isValid() const { return !id.empty() && rect.isValid(); }

    bool operator==(const MinimapRegion& o) const { return id == o.id; }
    bool operator!=(const MinimapRegion& o) const { return id != o.id; }
};

// ═════════════════════════════════════════════════════════════════
// MinimapViewport — normalized rect representing the visible window
// ═════════════════════════════════════════════════════════════════

struct MinimapViewport {
    MinimapRect  rect;
    bool         locked = false; // if true, viewport cannot be scrolled via minimap

    bool isValid() const { return rect.isValid(); }
};

// ═════════════════════════════════════════════════════════════════
// MinimapManager — region registry with viewport sync and observers
// ═════════════════════════════════════════════════════════════════

class MinimapManager {
public:
    using RegionObserver   = std::function<void(const MinimapRegion&, bool added)>;
    using ViewportObserver = std::function<void(const MinimapViewport&)>;

    static constexpr int MAX_REGIONS   = 256;
    static constexpr int MAX_OBSERVERS = 16;

    // Region management ────────────────────────────────────────

    bool addRegion(const MinimapRegion& region) {
        if (!region.isValid()) return false;
        if (findRegion(region.id)) return false;
        if ((int)m_regions.size() >= MAX_REGIONS) return false;
        m_regions.push_back(region);
        notifyRegion(region, true);
        return true;
    }

    bool removeRegion(const std::string& id) {
        auto it = findIt(id);
        if (it == m_regions.end()) return false;
        MinimapRegion copy = *it;
        m_regions.erase(it);
        notifyRegion(copy, false);
        return true;
    }

    bool updateRegion(const MinimapRegion& region) {
        auto it = findIt(region.id);
        if (it == m_regions.end()) return false;
        *it = region;
        notifyRegion(*it, true);
        return true;
    }

    bool isRegistered(const std::string& id) const { return findRegion(id) != nullptr; }

    const MinimapRegion* findRegion(const std::string& id) const {
        for (auto& r : m_regions)
            if (r.id == id) return &r;
        return nullptr;
    }

    MinimapRegion* findRegion(const std::string& id) {
        for (auto& r : m_regions)
            if (r.id == id) return &r;
        return nullptr;
    }

    bool setVisible(const std::string& id, bool visible) {
        MinimapRegion* r = findRegion(id);
        if (!r) return false;
        r->visible = visible;
        notifyRegion(*r, true);
        return true;
    }

    int  regionCount() const { return (int)m_regions.size(); }
    bool empty()       const { return m_regions.empty(); }

    const std::vector<MinimapRegion>& regions() const { return m_regions; }

    std::vector<const MinimapRegion*> visibleRegions() const {
        std::vector<const MinimapRegion*> result;
        for (auto& r : m_regions)
            if (r.visible) result.push_back(&r);
        return result;
    }

    // Viewport ─────────────────────────────────────────────────

    bool setViewport(const MinimapViewport& vp) {
        if (!vp.isValid()) return false;
        m_viewport = vp;
        notifyViewport(m_viewport);
        return true;
    }

    bool scrollViewport(float dx, float dy) {
        if (m_viewport.locked) return false;
        m_viewport.rect.x = std::max(0.0f, std::min(1.0f - m_viewport.rect.w, m_viewport.rect.x + dx));
        m_viewport.rect.y = std::max(0.0f, std::min(1.0f - m_viewport.rect.h, m_viewport.rect.y + dy));
        notifyViewport(m_viewport);
        return true;
    }

    bool lockViewport()   { m_viewport.locked = true;  return true; }
    bool unlockViewport() { m_viewport.locked = false; return true; }

    const MinimapViewport& viewport() const { return m_viewport; }

    // Bulk ─────────────────────────────────────────────────────

    void clear() { m_regions.clear(); }

    // Observers ────────────────────────────────────────────────

    uint32_t addRegionObserver(RegionObserver cb) {
        if (!cb || (int)m_regionObservers.size() >= MAX_OBSERVERS) return 0;
        uint32_t id = ++m_nextObserverId;
        m_regionObservers.push_back({id, std::move(cb)});
        return id;
    }

    uint32_t addViewportObserver(ViewportObserver cb) {
        if (!cb || (int)m_viewportObservers.size() >= MAX_OBSERVERS) return 0;
        uint32_t id = ++m_nextObserverId;
        m_viewportObservers.push_back({id, std::move(cb)});
        return id;
    }

    void removeObserver(uint32_t id) {
        m_regionObservers.erase(
            std::remove_if(m_regionObservers.begin(), m_regionObservers.end(),
                [id](const RegionEntry& e) { return e.id == id; }),
            m_regionObservers.end());
        m_viewportObservers.erase(
            std::remove_if(m_viewportObservers.begin(), m_viewportObservers.end(),
                [id](const ViewportEntry& e) { return e.id == id; }),
            m_viewportObservers.end());
    }

    void clearObservers() {
        m_regionObservers.clear();
        m_viewportObservers.clear();
    }

private:
    struct RegionEntry   { uint32_t id; RegionObserver   cb; };
    struct ViewportEntry { uint32_t id; ViewportObserver cb; };

    using RegionIt = std::vector<MinimapRegion>::iterator;

    RegionIt findIt(const std::string& id) {
        return std::find_if(m_regions.begin(), m_regions.end(),
            [&](const MinimapRegion& r) { return r.id == id; });
    }

    void notifyRegion(const MinimapRegion& r, bool added) {
        for (auto& e : m_regionObservers) e.cb(r, added);
    }
    void notifyViewport(const MinimapViewport& vp) {
        for (auto& e : m_viewportObservers) e.cb(vp);
    }

    std::vector<MinimapRegion>   m_regions;
    MinimapViewport               m_viewport;
    uint32_t                      m_nextObserverId = 0;
    std::vector<RegionEntry>      m_regionObservers;
    std::vector<ViewportEntry>    m_viewportObservers;
};

} // namespace NF
