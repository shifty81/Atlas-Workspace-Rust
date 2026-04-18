#pragma once
// NF::Editor — Road tool v1: spline-based road path authoring
#include "NF/Core/Core.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace NF {

enum class Rtv1RoadType  : uint8_t { Asphalt, Gravel, Dirt, Cobblestone, Custom };
enum class Rtv1JoinType  : uint8_t { Sharp, Rounded, Beveled };
enum class Rtv1LaneMode  : uint8_t { Single, Double, Highway };

struct Rtv1RoadNode {
    uint64_t id   = 0;
    float    x    = 0.f;
    float    y    = 0.f;
    float    z    = 0.f;
    float    width= 4.f;
    [[nodiscard]] bool isValid() const { return id != 0 && width > 0.f; }
};

struct Rtv1Road {
    uint64_t               id        = 0;
    std::string            name;
    Rtv1RoadType           type      = Rtv1RoadType::Asphalt;
    Rtv1JoinType           joinType  = Rtv1JoinType::Rounded;
    Rtv1LaneMode           laneMode  = Rtv1LaneMode::Double;
    std::vector<Rtv1RoadNode> nodes;
    bool                   closed    = false;
    [[nodiscard]] bool isValid() const { return id != 0 && !name.empty(); }
};

using Rtv1RoadCallback = std::function<void(uint64_t)>;

class RoadToolV1 {
public:
    bool addRoad(const Rtv1Road& r) {
        if (!r.isValid()) return false;
        for (const auto& er : m_roads) if (er.id == r.id) return false;
        m_roads.push_back(r);
        return true;
    }

    bool removeRoad(uint64_t id) {
        for (auto it = m_roads.begin(); it != m_roads.end(); ++it) {
            if (it->id == id) { m_roads.erase(it); return true; }
        }
        return false;
    }

    bool addNode(uint64_t roadId, const Rtv1RoadNode& node) {
        if (!node.isValid()) return false;
        for (auto& r : m_roads) {
            if (r.id == roadId) {
                r.nodes.push_back(node);
                if (m_onChange) m_onChange(roadId);
                return true;
            }
        }
        return false;
    }

    bool removeNode(uint64_t roadId, uint64_t nodeId) {
        for (auto& r : m_roads) {
            if (r.id == roadId) {
                for (auto it = r.nodes.begin(); it != r.nodes.end(); ++it) {
                    if (it->id == nodeId) { r.nodes.erase(it); if (m_onChange) m_onChange(roadId); return true; }
                }
            }
        }
        return false;
    }

    bool setRoadType(uint64_t roadId, Rtv1RoadType type) {
        for (auto& r : m_roads) {
            if (r.id == roadId) { r.type = type; if (m_onChange) m_onChange(roadId); return true; }
        }
        return false;
    }

    bool setClosed(uint64_t roadId, bool closed) {
        for (auto& r : m_roads) {
            if (r.id == roadId) { r.closed = closed; if (m_onChange) m_onChange(roadId); return true; }
        }
        return false;
    }

    [[nodiscard]] size_t roadCount() const { return m_roads.size(); }

    [[nodiscard]] const Rtv1Road* findRoad(uint64_t id) const {
        for (const auto& r : m_roads) if (r.id == id) return &r;
        return nullptr;
    }

    void setOnChange(Rtv1RoadCallback cb) { m_onChange = std::move(cb); }

private:
    std::vector<Rtv1Road> m_roads;
    Rtv1RoadCallback      m_onChange;
};

} // namespace NF
