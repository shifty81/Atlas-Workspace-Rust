/**
 * Tests for UniverseMapSystem — star system network, jump gates,
 * route planning (BFS/Dijkstra), security classification.
 */

#include <cassert>
#include <cmath>
#include "../engine/sim/UniverseMapSystem.h"

using namespace atlas::sim;

static bool approxEq(float a, float b, float eps = 0.01f) {
    return std::fabs(a - b) < eps;
}

// ══════════════════════════════════════════════════════════════════
// System management
// ══════════════════════════════════════════════════════════════════

void test_universe_defaults() {
    UniverseMapSystem map;
    assert(map.SystemCount() == 0);
    assert(map.GateCount() == 0);
}

void test_universe_add_system() {
    UniverseMapSystem map;
    assert(map.AddSystem({1, "Jita", 1.0f, 0.0f, 0.0f, 2, 3}));
    assert(map.SystemCount() == 1);
    const StarSystem* sys = map.GetSystem(1);
    assert(sys != nullptr);
    assert(sys->name == "Jita");
    assert(approxEq(sys->security, 1.0f));
}

void test_universe_add_duplicate_rejected() {
    UniverseMapSystem map;
    assert(map.AddSystem({1, "Jita", 1.0f}));
    assert(!map.AddSystem({1, "Jita2", 0.5f}));
    assert(map.SystemCount() == 1);
}

void test_universe_remove_system() {
    UniverseMapSystem map;
    map.AddSystem({1, "Jita", 1.0f});
    map.AddSystem({2, "Rens", 0.9f});
    map.AddGate({1, 2, 5.0f});
    assert(map.RemoveSystem(1));
    assert(map.SystemCount() == 1);
    assert(map.GetSystem(1) == nullptr);
    assert(map.GateCount() == 0);
}

void test_universe_remove_nonexistent() {
    UniverseMapSystem map;
    assert(!map.RemoveSystem(999));
}

// ══════════════════════════════════════════════════════════════════
// Gate management
// ══════════════════════════════════════════════════════════════════

void test_universe_add_gate() {
    UniverseMapSystem map;
    map.AddSystem({1, "A", 1.0f});
    map.AddSystem({2, "B", 0.8f});
    assert(map.AddGate({1, 2, 5.0f}));
    assert(map.GateCount() == 1);
    auto nbrs = map.GetNeighbours(1);
    assert(nbrs.size() == 1);
    assert(nbrs[0] == 2);
    // Bidirectional
    auto nbrs2 = map.GetNeighbours(2);
    assert(nbrs2.size() == 1);
    assert(nbrs2[0] == 1);
}

void test_universe_add_gate_duplicate_rejected() {
    UniverseMapSystem map;
    map.AddSystem({1, "A", 1.0f});
    map.AddSystem({2, "B", 0.8f});
    assert(map.AddGate({1, 2, 5.0f}));
    assert(!map.AddGate({1, 2, 3.0f}));
    assert(map.GateCount() == 1);
}

void test_universe_add_gate_self_rejected() {
    UniverseMapSystem map;
    map.AddSystem({1, "A", 1.0f});
    assert(!map.AddGate({1, 1, 0.0f}));
}

void test_universe_add_gate_missing_system() {
    UniverseMapSystem map;
    map.AddSystem({1, "A", 1.0f});
    assert(!map.AddGate({1, 999, 5.0f}));
}

void test_universe_remove_gate() {
    UniverseMapSystem map;
    map.AddSystem({1, "A", 1.0f});
    map.AddSystem({2, "B", 0.8f});
    map.AddGate({1, 2, 5.0f});
    assert(map.RemoveGate(1, 2));
    assert(map.GateCount() == 0);
    assert(map.GetNeighbours(1).empty());
    assert(map.GetNeighbours(2).empty());
}

void test_universe_remove_gate_nonexistent() {
    UniverseMapSystem map;
    assert(!map.RemoveGate(1, 2));
}

// ══════════════════════════════════════════════════════════════════
// Route planning
// ══════════════════════════════════════════════════════════════════

void test_universe_route_direct() {
    UniverseMapSystem map;
    map.AddSystem({1, "A", 1.0f});
    map.AddSystem({2, "B", 0.8f});
    map.AddGate({1, 2, 5.0f});
    auto route = map.PlanRoute(1, 2);
    assert(route.size() == 2);
    assert(route[0].systemId == 1);
    assert(route[1].systemId == 2);
    assert(approxEq(UniverseMapSystem::RouteDistance(route), 5.0f));
    assert(UniverseMapSystem::RouteJumps(route) == 1);
}

void test_universe_route_multi_hop() {
    UniverseMapSystem map;
    map.AddSystem({1, "A", 1.0f});
    map.AddSystem({2, "B", 0.8f});
    map.AddSystem({3, "C", 0.7f});
    map.AddGate({1, 2, 3.0f});
    map.AddGate({2, 3, 4.0f});
    auto route = map.PlanRoute(1, 3);
    assert(route.size() == 3);
    assert(route[0].systemId == 1);
    assert(route[2].systemId == 3);
    assert(UniverseMapSystem::RouteJumps(route) == 2);
}

void test_universe_route_no_path() {
    UniverseMapSystem map;
    map.AddSystem({1, "A", 1.0f});
    map.AddSystem({2, "B", 0.8f});
    // No gate
    auto route = map.PlanRoute(1, 2);
    assert(route.empty());
}

void test_universe_route_same_system() {
    UniverseMapSystem map;
    map.AddSystem({1, "A", 1.0f});
    auto route = map.PlanRoute(1, 1);
    assert(route.size() == 1);
    assert(route[0].systemId == 1);
}

void test_universe_route_by_distance() {
    // Create two paths: short distance but more jumps vs long but fewer jumps
    UniverseMapSystem map;
    map.AddSystem({1, "A", 1.0f, 0, 0});
    map.AddSystem({2, "B", 0.8f, 5, 0});
    map.AddSystem({3, "C", 0.7f, 10, 0});
    map.AddSystem({4, "D", 0.9f, 15, 0});
    // Path 1: 1→2→3→4, distance = 2+2+2 = 6
    map.AddGate({1, 2, 2.0f});
    map.AddGate({2, 3, 2.0f});
    map.AddGate({3, 4, 2.0f});
    // Path 2: 1→4 direct, distance = 20
    map.AddGate({1, 4, 20.0f});

    // BFS gives fewest jumps: 1→4 (1 jump)
    auto bfsRoute = map.PlanRoute(1, 4);
    assert(UniverseMapSystem::RouteJumps(bfsRoute) == 1);

    // Dijkstra gives shortest distance: 1→2→3→4 (6.0 LY)
    auto dijkstra = map.PlanRouteByDistance(1, 4);
    assert(approxEq(UniverseMapSystem::RouteDistance(dijkstra), 6.0f));
    assert(UniverseMapSystem::RouteJumps(dijkstra) == 3);
}

// ══════════════════════════════════════════════════════════════════
// Safe route planning
// ══════════════════════════════════════════════════════════════════

void test_universe_safe_route() {
    UniverseMapSystem map;
    map.AddSystem({1, "HighA", 1.0f});
    map.AddSystem({2, "LowB", 0.2f}); // low-sec
    map.AddSystem({3, "HighC", 0.8f});
    map.AddSystem({4, "HighD", 0.9f});
    // Direct: 1→2→4 (through low-sec)
    map.AddGate({1, 2, 3.0f});
    map.AddGate({2, 4, 3.0f});
    // Safe: 1→3→4
    map.AddGate({1, 3, 5.0f});
    map.AddGate({3, 4, 5.0f});

    auto safe = map.PlanSafeRoute(1, 4, 0.5f);
    assert(!safe.empty());
    // Should avoid system 2
    for (auto& hop : safe) {
        assert(hop.systemId != 2);
    }
    assert(UniverseMapSystem::RouteJumps(safe) == 2);
}

void test_universe_safe_route_no_path() {
    UniverseMapSystem map;
    map.AddSystem({1, "HighA", 1.0f});
    map.AddSystem({2, "LowB", 0.2f});
    map.AddSystem({3, "HighC", 0.8f});
    // Only path goes through low-sec
    map.AddGate({1, 2, 3.0f});
    map.AddGate({2, 3, 3.0f});

    auto safe = map.PlanSafeRoute(1, 3, 0.5f);
    assert(safe.empty()); // no safe path
}

// ══════════════════════════════════════════════════════════════════
// Security and utility
// ══════════════════════════════════════════════════════════════════

void test_universe_security_classification() {
    assert(UniverseMapSystem::ClassifySecurity(1.0f) == SecurityClass::HighSec);
    assert(UniverseMapSystem::ClassifySecurity(0.5f) == SecurityClass::HighSec);
    assert(UniverseMapSystem::ClassifySecurity(0.4f) == SecurityClass::LowSec);
    assert(UniverseMapSystem::ClassifySecurity(0.1f) == SecurityClass::LowSec);
    assert(UniverseMapSystem::ClassifySecurity(0.0f) == SecurityClass::NullSec);
    assert(UniverseMapSystem::ClassifySecurity(-0.1f) == SecurityClass::Wormhole);
}

void test_universe_travel_time() {
    std::vector<RouteHop> route = {{1, 0.0f}, {2, 9.0f}};
    float t = UniverseMapSystem::EstimateTravelTime(route, 3.0f);
    assert(approxEq(t, 3.0f)); // 9.0 / 3.0 = 3.0
}

void test_universe_travel_time_empty() {
    std::vector<RouteHop> empty;
    assert(approxEq(UniverseMapSystem::EstimateTravelTime(empty), 0.0f));
}

void test_universe_systems_by_security() {
    UniverseMapSystem map;
    map.AddSystem({1, "High1", 1.0f});
    map.AddSystem({2, "High2", 0.7f});
    map.AddSystem({3, "Low1", 0.3f});
    map.AddSystem({4, "Null1", 0.0f});

    auto high = map.GetSystemsBySecurityClass(SecurityClass::HighSec);
    assert(high.size() == 2);
    auto low = map.GetSystemsBySecurityClass(SecurityClass::LowSec);
    assert(low.size() == 1);
    auto null = map.GetSystemsBySecurityClass(SecurityClass::NullSec);
    assert(null.size() == 1);
}

void test_universe_neighbours_nonexistent() {
    UniverseMapSystem map;
    auto nbrs = map.GetNeighbours(999);
    assert(nbrs.empty());
}
