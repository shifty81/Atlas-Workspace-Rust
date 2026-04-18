#include <cassert>
#include <iostream>
#include "../engine/ai/RelationshipModel.h"

using namespace atlas::ai;

void test_relationship_set_get() {
    RelationshipModel model;
    model.Set(1, 2, 0.75f);
    assert(model.Get(1, 2) >= 0.74f && model.Get(1, 2) <= 0.76f);
    // Symmetric: A→B == B→A
    assert(model.Get(2, 1) >= 0.74f && model.Get(2, 1) <= 0.76f);
    std::cout << "[PASS] test_relationship_set_get" << std::endl;
}

void test_relationship_default_zero() {
    RelationshipModel model;
    assert(model.Get(1, 2) == 0.0f);
    std::cout << "[PASS] test_relationship_default_zero" << std::endl;
}

void test_relationship_modify() {
    RelationshipModel model;
    model.Set(1, 2, 0.5f);
    model.Modify(1, 2, 0.3f);
    assert(model.Get(1, 2) >= 0.79f && model.Get(1, 2) <= 0.81f);
    std::cout << "[PASS] test_relationship_modify" << std::endl;
}

void test_relationship_has() {
    RelationshipModel model;
    assert(!model.Has(1, 2));
    model.Set(1, 2, 0.5f);
    assert(model.Has(1, 2));
    assert(model.Has(2, 1));
    std::cout << "[PASS] test_relationship_has" << std::endl;
}

void test_relationship_remove() {
    RelationshipModel model;
    model.Set(1, 2, 0.5f);
    model.Remove(1, 2);
    assert(!model.Has(1, 2));
    std::cout << "[PASS] test_relationship_remove" << std::endl;
}

void test_relationship_clear() {
    RelationshipModel model;
    model.Set(1, 2, 0.5f);
    model.Set(3, 4, 0.8f);
    assert(model.Count() == 2);
    model.Clear();
    assert(model.Count() == 0);
    std::cout << "[PASS] test_relationship_clear" << std::endl;
}
