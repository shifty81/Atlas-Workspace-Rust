#include "sim/UniverseMapSystem.h"
#include <queue>
#include <limits>
#include <algorithm>
#include <cmath>

namespace atlas::sim {

// ─────────────────────────────────────────────────────────────────
// System management
// ─────────────────────────────────────────────────────────────────

bool UniverseMapSystem::AddSystem(const StarSystem& sys) {
    if (m_systems.count(sys.systemId)) return false;
    m_systems[sys.systemId] = sys;
    // Ensure adjacency entry exists
    m_adjacency[sys.systemId];
    return true;
}

bool UniverseMapSystem::RemoveSystem(uint32_t systemId) {
    auto it = m_systems.find(systemId);
    if (it == m_systems.end()) return false;

    // Remove all gates connected to this system
    auto& neighbours = m_adjacency[systemId];
    for (auto& [nbr, dist] : neighbours) {
        auto& nbrAdj = m_adjacency[nbr];
        nbrAdj.erase(
            std::remove_if(nbrAdj.begin(), nbrAdj.end(),
                [systemId](const std::pair<uint32_t, float>& p) {
                    return p.first == systemId;
                }),
            nbrAdj.end());
        --m_gateCount;
    }
    m_adjacency.erase(systemId);
    m_systems.erase(it);
    return true;
}

const StarSystem* UniverseMapSystem::GetSystem(uint32_t systemId) const {
    auto it = m_systems.find(systemId);
    return (it != m_systems.end()) ? &it->second : nullptr;
}

// ─────────────────────────────────────────────────────────────────
// Gate management
// ─────────────────────────────────────────────────────────────────

bool UniverseMapSystem::AddGate(const JumpGate& gate) {
    // Both systems must exist
    if (!m_systems.count(gate.fromSystem) || !m_systems.count(gate.toSystem))
        return false;
    if (gate.fromSystem == gate.toSystem) return false;

    // Check for duplicate
    auto& fromAdj = m_adjacency[gate.fromSystem];
    for (auto& [nbr, d] : fromAdj) {
        if (nbr == gate.toSystem) return false;
    }

    float dist = gate.distance;
    if (dist <= 0.0f) {
        // Compute from positions
        const auto& a = m_systems[gate.fromSystem];
        const auto& b = m_systems[gate.toSystem];
        float dx = b.posX - a.posX;
        float dy = b.posY - a.posY;
        dist = std::sqrt(dx * dx + dy * dy);
        if (dist <= 0.0f) dist = 1.0f;
    }

    fromAdj.push_back({gate.toSystem, dist});
    m_adjacency[gate.toSystem].push_back({gate.fromSystem, dist});
    ++m_gateCount;
    return true;
}

bool UniverseMapSystem::RemoveGate(uint32_t fromSystem, uint32_t toSystem) {
    auto itFrom = m_adjacency.find(fromSystem);
    auto itTo   = m_adjacency.find(toSystem);
    if (itFrom == m_adjacency.end() || itTo == m_adjacency.end())
        return false;

    bool found = false;
    auto& fromAdj = itFrom->second;
    auto newEnd = std::remove_if(fromAdj.begin(), fromAdj.end(),
        [toSystem](const std::pair<uint32_t, float>& p) {
            return p.first == toSystem;
        });
    if (newEnd != fromAdj.end()) {
        fromAdj.erase(newEnd, fromAdj.end());
        found = true;
    }

    auto& toAdj = itTo->second;
    toAdj.erase(
        std::remove_if(toAdj.begin(), toAdj.end(),
            [fromSystem](const std::pair<uint32_t, float>& p) {
                return p.first == fromSystem;
            }),
        toAdj.end());

    if (found) --m_gateCount;
    return found;
}

std::vector<uint32_t> UniverseMapSystem::GetNeighbours(uint32_t systemId) const {
    std::vector<uint32_t> result;
    auto it = m_adjacency.find(systemId);
    if (it == m_adjacency.end()) return result;
    for (auto& [nbr, d] : it->second) {
        result.push_back(nbr);
    }
    return result;
}

// ─────────────────────────────────────────────────────────────────
// Route planning — BFS (fewest jumps)
// ─────────────────────────────────────────────────────────────────

std::vector<RouteHop> UniverseMapSystem::PlanRoute(
    uint32_t fromSystem, uint32_t toSystem) const {

    if (!m_systems.count(fromSystem) || !m_systems.count(toSystem))
        return {};
    if (fromSystem == toSystem)
        return {{fromSystem, 0.0f}};

    std::unordered_map<uint32_t, uint32_t> parent;
    std::unordered_map<uint32_t, float> dist;
    std::queue<uint32_t> q;

    parent[fromSystem] = fromSystem;
    dist[fromSystem] = 0.0f;
    q.push(fromSystem);

    while (!q.empty()) {
        uint32_t cur = q.front();
        q.pop();

        if (cur == toSystem) break;

        auto adjIt = m_adjacency.find(cur);
        if (adjIt == m_adjacency.end()) continue;

        for (auto& [nbr, d] : adjIt->second) {
            if (parent.count(nbr)) continue;
            parent[nbr] = cur;
            dist[nbr] = dist[cur] + d;
            q.push(nbr);
        }
    }

    if (!parent.count(toSystem)) return {};

    // Reconstruct path
    std::vector<RouteHop> path;
    uint32_t cur = toSystem;
    while (cur != fromSystem) {
        path.push_back({cur, dist[cur]});
        cur = parent[cur];
    }
    path.push_back({fromSystem, 0.0f});
    std::reverse(path.begin(), path.end());
    return path;
}

// ─────────────────────────────────────────────────────────────────
// Route planning — Dijkstra (shortest distance)
// ─────────────────────────────────────────────────────────────────

std::vector<RouteHop> UniverseMapSystem::PlanRouteByDistance(
    uint32_t fromSystem, uint32_t toSystem) const {

    if (!m_systems.count(fromSystem) || !m_systems.count(toSystem))
        return {};
    if (fromSystem == toSystem)
        return {{fromSystem, 0.0f}};

    using PQEntry = std::pair<float, uint32_t>; // (distance, systemId)
    std::priority_queue<PQEntry, std::vector<PQEntry>, std::greater<>> pq;
    std::unordered_map<uint32_t, float> best;
    std::unordered_map<uint32_t, uint32_t> parent;

    best[fromSystem] = 0.0f;
    parent[fromSystem] = fromSystem;
    pq.push({0.0f, fromSystem});

    while (!pq.empty()) {
        auto [curDist, cur] = pq.top();
        pq.pop();

        if (cur == toSystem) break;
        if (curDist > best[cur]) continue;

        auto adjIt = m_adjacency.find(cur);
        if (adjIt == m_adjacency.end()) continue;

        for (auto& [nbr, d] : adjIt->second) {
            float nd = curDist + d;
            if (!best.count(nbr) || nd < best[nbr]) {
                best[nbr] = nd;
                parent[nbr] = cur;
                pq.push({nd, nbr});
            }
        }
    }

    if (!parent.count(toSystem)) return {};

    std::vector<RouteHop> path;
    uint32_t cur = toSystem;
    while (cur != fromSystem) {
        path.push_back({cur, best[cur]});
        cur = parent[cur];
    }
    path.push_back({fromSystem, 0.0f});
    std::reverse(path.begin(), path.end());
    return path;
}

// ─────────────────────────────────────────────────────────────────
// Safe route (avoid low-sec)
// ─────────────────────────────────────────────────────────────────

std::vector<RouteHop> UniverseMapSystem::PlanSafeRoute(
    uint32_t fromSystem, uint32_t toSystem, float minSecurity) const {

    if (!m_systems.count(fromSystem) || !m_systems.count(toSystem))
        return {};
    if (fromSystem == toSystem)
        return {{fromSystem, 0.0f}};

    std::unordered_map<uint32_t, uint32_t> parent;
    std::unordered_map<uint32_t, float> dist;
    std::queue<uint32_t> q;

    parent[fromSystem] = fromSystem;
    dist[fromSystem] = 0.0f;
    q.push(fromSystem);

    while (!q.empty()) {
        uint32_t cur = q.front();
        q.pop();

        if (cur == toSystem) break;

        auto adjIt = m_adjacency.find(cur);
        if (adjIt == m_adjacency.end()) continue;

        for (auto& [nbr, d] : adjIt->second) {
            if (parent.count(nbr)) continue;
            // Skip low-sec systems (unless it's the destination)
            if (nbr != toSystem) {
                auto sysIt = m_systems.find(nbr);
                if (sysIt != m_systems.end() && sysIt->second.security < minSecurity)
                    continue;
            }
            parent[nbr] = cur;
            dist[nbr] = dist[cur] + d;
            q.push(nbr);
        }
    }

    if (!parent.count(toSystem)) return {};

    std::vector<RouteHop> path;
    uint32_t cur = toSystem;
    while (cur != fromSystem) {
        path.push_back({cur, dist[cur]});
        cur = parent[cur];
    }
    path.push_back({fromSystem, 0.0f});
    std::reverse(path.begin(), path.end());
    return path;
}

// ─────────────────────────────────────────────────────────────────
// Utility queries
// ─────────────────────────────────────────────────────────────────

SecurityClass UniverseMapSystem::ClassifySecurity(float security) {
    if (security >= 0.5f)  return SecurityClass::HighSec;
    if (security >= 0.1f)  return SecurityClass::LowSec;
    if (security >= 0.0f)  return SecurityClass::NullSec;
    return SecurityClass::Wormhole;
}

float UniverseMapSystem::RouteDistance(const std::vector<RouteHop>& route) {
    if (route.empty()) return 0.0f;
    return route.back().cumDistance;
}

size_t UniverseMapSystem::RouteJumps(const std::vector<RouteHop>& route) {
    return route.empty() ? 0 : route.size() - 1;
}

float UniverseMapSystem::EstimateTravelTime(
    const std::vector<RouteHop>& route, float warpSpeed) {
    if (route.empty() || warpSpeed <= 0.0f) return 0.0f;
    return RouteDistance(route) / warpSpeed;
}

std::vector<uint32_t> UniverseMapSystem::GetSystemsBySecurityClass(
    SecurityClass sc) const {
    std::vector<uint32_t> result;
    for (auto& [id, sys] : m_systems) {
        if (ClassifySecurity(sys.security) == sc) {
            result.push_back(id);
        }
    }
    return result;
}

} // namespace atlas::sim
