#include "../engine/world/GalaxyGenerator.h"
#include <iostream>
#include <cassert>
#include <cmath>
#include <set>

using namespace atlas::world;

void test_galaxy_system_count() {
    GalaxyParams params;
    params.systemCount = 100;
    params.seed = 42;

    auto systems = GalaxyGenerator::Generate(params);
    assert(systems.size() == 100);

    std::cout << "[PASS] test_galaxy_system_count" << std::endl;
}

void test_galaxy_deterministic() {
    GalaxyParams params;
    params.systemCount = 50;
    params.seed = 123;

    auto sys1 = GalaxyGenerator::Generate(params);
    auto sys2 = GalaxyGenerator::Generate(params);

    assert(sys1.size() == sys2.size());
    for (size_t i = 0; i < sys1.size(); ++i) {
        assert(sys1[i].x == sys2[i].x);
        assert(sys1[i].y == sys2[i].y);
        assert(sys1[i].z == sys2[i].z);
    }

    std::cout << "[PASS] test_galaxy_deterministic" << std::endl;
}

void test_galaxy_seed_variation() {
    GalaxyParams p1, p2;
    p1.systemCount = 20;
    p1.seed = 1;
    p2.systemCount = 20;
    p2.seed = 999;

    auto sys1 = GalaxyGenerator::Generate(p1);
    auto sys2 = GalaxyGenerator::Generate(p2);

    // Different seeds should produce different positions
    bool anyDifferent = false;
    for (size_t i = 0; i < sys1.size(); ++i) {
        if (sys1[i].x != sys2[i].x || sys1[i].z != sys2[i].z) {
            anyDifferent = true;
            break;
        }
    }
    assert(anyDifferent);

    std::cout << "[PASS] test_galaxy_seed_variation" << std::endl;
}

void test_galaxy_region_filter() {
    GalaxyParams params;
    params.systemCount = 1000;
    params.seed = 42;
    params.galaxyRadius = 1000.0f;

    auto regional = GalaxyGenerator::GenerateInRegion(params, -100.0, -100.0, 100.0, 100.0, 500);

    // All returned systems should be in the region
    for (const auto& sys : regional) {
        assert(sys.x >= -100.0 && sys.x <= 100.0);
        assert(sys.z >= -100.0 && sys.z <= 100.0);
    }

    // Should have fewer systems than total
    assert(regional.size() <= 1000);

    std::cout << "[PASS] test_galaxy_region_filter" << std::endl;
}

void test_galaxy_unique_ids() {
    GalaxyParams params;
    params.systemCount = 100;
    params.seed = 42;

    auto systems = GalaxyGenerator::Generate(params);

    std::set<uint64_t> ids;
    for (const auto& sys : systems) {
        ids.insert(sys.id);
    }
    assert(ids.size() == 100);

    std::cout << "[PASS] test_galaxy_unique_ids" << std::endl;
}

void test_galaxy_star_classes() {
    GalaxyParams params;
    params.systemCount = 200;
    params.seed = 42;

    auto systems = GalaxyGenerator::Generate(params);

    for (const auto& sys : systems) {
        assert(sys.starClass < 7);
        assert(sys.luminosity >= 0.0f && sys.luminosity <= 1.0f);
        assert(sys.planetCount >= 0 && sys.planetCount < 12);
    }

    std::cout << "[PASS] test_galaxy_star_classes" << std::endl;
}
