#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>

namespace atlas::sim {

/**
 * Security classification for a star system.
 */
enum class SecurityClass : uint8_t {
    HighSec,   // 0.5 – 1.0
    LowSec,    // 0.1 – 0.4
    NullSec,   // 0.0
    Wormhole   // unknown / volatile
};

/**
 * Describes a single star system.
 */
struct StarSystem {
    uint32_t      systemId    = 0;
    std::string   name;
    float         security    = 1.0f;     // 0.0 – 1.0
    float         posX        = 0.0f;     // universe coordinates
    float         posY        = 0.0f;
    uint32_t      stationCount = 0;
    uint32_t      beltCount    = 0;
};

/**
 * A directional jump-gate connection between two systems.
 * Gates are bidirectional by convention; each direction is stored
 * as a separate entry.
 */
struct JumpGate {
    uint32_t fromSystem = 0;
    uint32_t toSystem   = 0;
    float    distance   = 0.0f;   // light-years (used for travel time)
};

/**
 * One hop in a planned route.
 */
struct RouteHop {
    uint32_t systemId   = 0;
    float    cumDistance = 0.0f;   // total distance so far
};

/**
 * @brief Universe map: star systems, jump gates, and route planning.
 *
 * Maintains the graph of star systems connected by jump gates.
 * Provides shortest-path route planning (BFS for fewest jumps,
 * Dijkstra for shortest distance), security-class queries, and
 * neighbour look-ups.
 *
 * Usage:
 *   UniverseMapSystem map;
 *   map.AddSystem({1, "Jita", 1.0f, 0, 0, 2, 3});
 *   map.AddSystem({2, "Rens", 0.9f, 10, 5, 1, 2});
 *   map.AddGate({1, 2, 4.5f});
 *   auto route = map.PlanRoute(1, 2);
 */
class UniverseMapSystem {
public:
    UniverseMapSystem() = default;

    // --- System management ---

    /** Add a star system. Returns false if systemId already exists. */
    bool AddSystem(const StarSystem& sys);

    /** Remove a system and all its gates. */
    bool RemoveSystem(uint32_t systemId);

    /** Get a system by id. Returns nullptr if not found. */
    const StarSystem* GetSystem(uint32_t systemId) const;

    /** Number of registered systems. */
    size_t SystemCount() const { return m_systems.size(); }

    // --- Gate management ---

    /** Add a bidirectional jump gate. Returns false if it already exists. */
    bool AddGate(const JumpGate& gate);

    /** Remove a bidirectional gate. */
    bool RemoveGate(uint32_t fromSystem, uint32_t toSystem);

    /** Number of gate connections (each bidirectional pair counts as 1). */
    size_t GateCount() const { return m_gateCount; }

    /** Get systems directly connected to the given system. */
    std::vector<uint32_t> GetNeighbours(uint32_t systemId) const;

    // --- Route planning ---

    /**
     * Plan shortest route by number of jumps (BFS).
     * Returns empty vector if no path exists.
     */
    std::vector<RouteHop> PlanRoute(uint32_t fromSystem,
                                    uint32_t toSystem) const;

    /**
     * Plan shortest route by total distance (Dijkstra).
     * Returns empty vector if no path exists.
     */
    std::vector<RouteHop> PlanRouteByDistance(uint32_t fromSystem,
                                             uint32_t toSystem) const;

    /**
     * Plan route avoiding low/null-sec systems (security >= threshold).
     * Uses BFS on the filtered sub-graph.
     * Start and end systems are always included regardless of security.
     */
    std::vector<RouteHop> PlanSafeRoute(uint32_t fromSystem,
                                        uint32_t toSystem,
                                        float minSecurity = 0.5f) const;

    // --- Queries ---

    /** Classify a system's security level. */
    static SecurityClass ClassifySecurity(float security);

    /** Total distance of a route (last hop's cumDistance). */
    static float RouteDistance(const std::vector<RouteHop>& route);

    /** Number of jumps in a route. */
    static size_t RouteJumps(const std::vector<RouteHop>& route);

    /** Estimated travel time in seconds (warpSpeed AU/s). */
    static float EstimateTravelTime(const std::vector<RouteHop>& route,
                                    float warpSpeed = 3.0f);

    /** Get all systems matching a security class. */
    std::vector<uint32_t> GetSystemsBySecurityClass(SecurityClass sc) const;

private:
    std::unordered_map<uint32_t, StarSystem> m_systems;
    // adjacency list: systemId → [(neighbourId, distance), ...]
    std::unordered_map<uint32_t,
        std::vector<std::pair<uint32_t, float>>> m_adjacency;
    size_t m_gateCount = 0;
};

} // namespace atlas::sim
