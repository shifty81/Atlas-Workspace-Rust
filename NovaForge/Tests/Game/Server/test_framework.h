#pragma once
/**
 * Shared test framework utilities for Atlas NovaForge server tests.
 *
 * Provides assertion helpers, component creation utilities, and test
 * counters used across all per-domain test files.
 */

#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/game_components.h"
#include <iostream>
#include <cassert>
#include <string>
#include <cmath>
#include <memory>

namespace atlas::test {

// Global test counters (defined in test_main.cpp)
extern int testsRun;
extern int testsPassed;

inline void assertTrue(bool condition, const std::string& testName) {
    testsRun++;
    if (condition) {
        testsPassed++;
        std::cout << "  \xe2\x9c\x93 " << testName << std::endl;
    } else {
        std::cout << "  \xe2\x9c\x97 " << testName << " FAILED" << std::endl;
    }
}

inline bool approxEqual(float a, float b, float epsilon = 0.01f) {
    return std::fabs(a - b) < epsilon;
}

/// Helper to add a component and return a raw pointer to it
template<typename T>
T* addComp(ecs::Entity* e) {
    auto c = std::make_unique<T>();
    T* ptr = c.get();
    e->addComponent(std::move(c));
    return ptr;
}

} // namespace atlas::test
