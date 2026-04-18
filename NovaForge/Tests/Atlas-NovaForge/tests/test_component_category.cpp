#include "../engine/ecs/ComponentCategory.h"
#include "../engine/ecs/BuiltinComponents.h"
#include <iostream>
#include <cassert>

using namespace atlas::ecs;
using namespace atlas::sim;

void test_component_category_defaults() {
    // Unspecialized type defaults to Simulated
    struct UnknownComp { int x; };
    static_assert(component_category_v<UnknownComp> == StateCategory::Simulated);
    std::cout << "[PASS] test_component_category_defaults" << std::endl;
}

void test_component_category_simulated() {
    static_assert(component_category_v<TransformComponent> == StateCategory::Simulated);
    static_assert(component_category_v<VelocityComponent> == StateCategory::Simulated);
    static_assert(component_category_v<HealthComponent> == StateCategory::Simulated);
    static_assert(component_category_v<NameComponent> == StateCategory::Simulated);
    std::cout << "[PASS] test_component_category_simulated" << std::endl;
}

void test_component_category_presentation() {
    static_assert(component_category_v<RenderHintComponent> == StateCategory::Presentation);
    std::cout << "[PASS] test_component_category_presentation" << std::endl;
}

void test_component_category_debug() {
    static_assert(component_category_v<DebugLabelComponent> == StateCategory::Debug);
    std::cout << "[PASS] test_component_category_debug" << std::endl;
}

void test_component_category_derived() {
    static_assert(component_category_v<PathCacheComponent> == StateCategory::Derived);
    std::cout << "[PASS] test_component_category_derived" << std::endl;
}

void test_component_category_assert_simulated() {
    // This should compile fine for simulated components
    assert_simulated<TransformComponent>();
    assert_simulated<VelocityComponent>();
    std::cout << "[PASS] test_component_category_assert_simulated" << std::endl;
}

void test_component_category_assert_not_presentation() {
    // These should compile fine for non-presentation components
    assert_not_presentation<TransformComponent>();
    assert_not_presentation<HealthComponent>();
    assert_not_presentation<DebugLabelComponent>();
    std::cout << "[PASS] test_component_category_assert_not_presentation" << std::endl;
}

void test_component_category_runtime_query() {
    // Verify that categories can be queried at runtime too
    auto cat1 = component_category_v<TransformComponent>;
    auto cat2 = component_category_v<RenderHintComponent>;
    auto cat3 = component_category_v<DebugLabelComponent>;
    auto cat4 = component_category_v<PathCacheComponent>;

    assert(cat1 == StateCategory::Simulated);
    assert(cat2 == StateCategory::Presentation);
    assert(cat3 == StateCategory::Debug);
    assert(cat4 == StateCategory::Derived);

    std::cout << "[PASS] test_component_category_runtime_query" << std::endl;
}
