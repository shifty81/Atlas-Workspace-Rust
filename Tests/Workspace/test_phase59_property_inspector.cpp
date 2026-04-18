// Tests/Workspace/test_phase59_property_inspector.cpp
// Phase 59 — WorkspacePropertyInspector: PropertyType, PropertyEntry,
//             PropertyCategory, PropertyInspector
#include <catch2/catch_test_macros.hpp>
#include "NF/Workspace/WorkspacePropertyInspector.h"

// ═════════════════════════════════════════════════════════════════
// PropertyType
// ═════════════════════════════════════════════════════════════════

TEST_CASE("PropertyType: name helpers", "[property][type]") {
    REQUIRE(std::string(NF::propertyTypeName(NF::PropertyType::String)) == "String");
    REQUIRE(std::string(NF::propertyTypeName(NF::PropertyType::Int))    == "Int");
    REQUIRE(std::string(NF::propertyTypeName(NF::PropertyType::Float))  == "Float");
    REQUIRE(std::string(NF::propertyTypeName(NF::PropertyType::Bool))   == "Bool");
    REQUIRE(std::string(NF::propertyTypeName(NF::PropertyType::Color))  == "Color");
    REQUIRE(std::string(NF::propertyTypeName(NF::PropertyType::Vec2))   == "Vec2");
    REQUIRE(std::string(NF::propertyTypeName(NF::PropertyType::Vec3))   == "Vec3");
    REQUIRE(std::string(NF::propertyTypeName(NF::PropertyType::Enum))   == "Enum");
    REQUIRE(std::string(NF::propertyTypeName(NF::PropertyType::Custom)) == "Custom");
}

// ═════════════════════════════════════════════════════════════════
// PropertyEntry
// ═════════════════════════════════════════════════════════════════

static NF::PropertyEntry makeProp(const std::string& id, const std::string& name,
                                   NF::PropertyType type = NF::PropertyType::String,
                                   const std::string& value = "default") {
    NF::PropertyEntry p;
    p.id    = id;
    p.name  = name;
    p.type  = type;
    p.value = value;
    return p;
}

TEST_CASE("PropertyEntry: default invalid", "[property][entry]") {
    NF::PropertyEntry p;
    REQUIRE_FALSE(p.isValid());
}

TEST_CASE("PropertyEntry: valid with id and name", "[property][entry]") {
    auto p = makeProp("p1", "Position");
    REQUIRE(p.isValid());
}

TEST_CASE("PropertyEntry: invalid without name", "[property][entry]") {
    NF::PropertyEntry p;
    p.id = "x";
    REQUIRE_FALSE(p.isValid());
}

TEST_CASE("PropertyEntry: equality by id", "[property][entry]") {
    auto a = makeProp("a", "X");
    auto b = makeProp("a", "Y");
    auto c = makeProp("c", "Z");
    REQUIRE(a == b);
    REQUIRE(a != c);
}

TEST_CASE("PropertyEntry: readOnly default false", "[property][entry]") {
    auto p = makeProp("p1", "Name");
    REQUIRE_FALSE(p.readOnly);
}

// ═════════════════════════════════════════════════════════════════
// PropertyCategory
// ═════════════════════════════════════════════════════════════════

static NF::PropertyCategory makeCat(const std::string& id, const std::string& name) {
    NF::PropertyCategory cat;
    cat.id   = id;
    cat.name = name;
    return cat;
}

TEST_CASE("PropertyCategory: default empty", "[property][category]") {
    auto cat = makeCat("transform", "Transform");
    REQUIRE(cat.isValid());
    REQUIRE(cat.empty());
    REQUIRE(cat.count() == 0);
}

TEST_CASE("PropertyCategory: invalid without id", "[property][category]") {
    NF::PropertyCategory cat;
    cat.name = "Transform";
    REQUIRE_FALSE(cat.isValid());
}

TEST_CASE("PropertyCategory: addProperty", "[property][category]") {
    auto cat = makeCat("transform", "Transform");
    REQUIRE(cat.addProperty(makeProp("x", "X")));
    REQUIRE(cat.count() == 1);
    REQUIRE_FALSE(cat.empty());
}

TEST_CASE("PropertyCategory: addProperty rejects invalid", "[property][category]") {
    auto cat = makeCat("transform", "Transform");
    NF::PropertyEntry p;
    REQUIRE_FALSE(cat.addProperty(p));
}

TEST_CASE("PropertyCategory: addProperty rejects duplicate", "[property][category]") {
    auto cat = makeCat("transform", "Transform");
    cat.addProperty(makeProp("x", "X"));
    REQUIRE_FALSE(cat.addProperty(makeProp("x", "X2")));
}

TEST_CASE("PropertyCategory: removeProperty", "[property][category]") {
    auto cat = makeCat("transform", "Transform");
    cat.addProperty(makeProp("x", "X"));
    REQUIRE(cat.removeProperty("x"));
    REQUIRE(cat.empty());
}

TEST_CASE("PropertyCategory: removeProperty unknown", "[property][category]") {
    auto cat = makeCat("transform", "Transform");
    REQUIRE_FALSE(cat.removeProperty("missing"));
}

TEST_CASE("PropertyCategory: findProperty", "[property][category]") {
    auto cat = makeCat("transform", "Transform");
    cat.addProperty(makeProp("x", "X", NF::PropertyType::Float, "1.0"));
    const auto* p = cat.findProperty("x");
    REQUIRE(p != nullptr);
    REQUIRE(p->name == "X");
    REQUIRE(p->value == "1.0");
}

TEST_CASE("PropertyCategory: findPropertyMut", "[property][category]") {
    auto cat = makeCat("transform", "Transform");
    cat.addProperty(makeProp("x", "X", NF::PropertyType::Float, "0.0"));
    auto* p = cat.findPropertyMut("x");
    REQUIRE(p != nullptr);
    p->value = "5.0";
    REQUIRE(cat.findProperty("x")->value == "5.0");
}

TEST_CASE("PropertyCategory: clear", "[property][category]") {
    auto cat = makeCat("transform", "Transform");
    cat.addProperty(makeProp("x", "X"));
    cat.addProperty(makeProp("y", "Y"));
    cat.clear();
    REQUIRE(cat.empty());
}

TEST_CASE("PropertyCategory: collapsed default false", "[property][category]") {
    auto cat = makeCat("transform", "Transform");
    REQUIRE_FALSE(cat.collapsed);
}

// ═════════════════════════════════════════════════════════════════
// PropertyInspector
// ═════════════════════════════════════════════════════════════════

TEST_CASE("PropertyInspector: default empty", "[property][inspector]") {
    NF::PropertyInspector insp;
    REQUIRE(insp.categoryCount() == 0);
    REQUIRE(insp.totalProperties() == 0);
}

TEST_CASE("PropertyInspector: addCategory", "[property][inspector]") {
    NF::PropertyInspector insp;
    REQUIRE(insp.addCategory(makeCat("transform", "Transform")));
    REQUIRE(insp.categoryCount() == 1);
    REQUIRE(insp.hasCategory("transform"));
}

TEST_CASE("PropertyInspector: addCategory rejects invalid", "[property][inspector]") {
    NF::PropertyInspector insp;
    NF::PropertyCategory cat;
    REQUIRE_FALSE(insp.addCategory(cat));
}

TEST_CASE("PropertyInspector: addCategory rejects duplicate", "[property][inspector]") {
    NF::PropertyInspector insp;
    insp.addCategory(makeCat("transform", "Transform"));
    REQUIRE_FALSE(insp.addCategory(makeCat("transform", "Transform 2")));
}

TEST_CASE("PropertyInspector: removeCategory", "[property][inspector]") {
    NF::PropertyInspector insp;
    insp.addCategory(makeCat("transform", "Transform"));
    REQUIRE(insp.removeCategory("transform"));
    REQUIRE(insp.categoryCount() == 0);
}

TEST_CASE("PropertyInspector: removeCategory unknown", "[property][inspector]") {
    NF::PropertyInspector insp;
    REQUIRE_FALSE(insp.removeCategory("missing"));
}

TEST_CASE("PropertyInspector: addProperty shortcut", "[property][inspector]") {
    NF::PropertyInspector insp;
    insp.addCategory(makeCat("transform", "Transform"));
    REQUIRE(insp.addProperty("transform", makeProp("x", "X")));
    REQUIRE(insp.totalProperties() == 1);
}

TEST_CASE("PropertyInspector: addProperty unknown category", "[property][inspector]") {
    NF::PropertyInspector insp;
    REQUIRE_FALSE(insp.addProperty("missing", makeProp("x", "X")));
}

TEST_CASE("PropertyInspector: removeProperty shortcut", "[property][inspector]") {
    NF::PropertyInspector insp;
    insp.addCategory(makeCat("transform", "Transform"));
    insp.addProperty("transform", makeProp("x", "X"));
    REQUIRE(insp.removeProperty("transform", "x"));
    REQUIRE(insp.totalProperties() == 0);
}

TEST_CASE("PropertyInspector: setValue and getValue", "[property][inspector]") {
    NF::PropertyInspector insp;
    insp.addCategory(makeCat("transform", "Transform"));
    insp.addProperty("transform", makeProp("x", "X", NF::PropertyType::Float, "0.0"));

    REQUIRE(insp.setValue("transform", "x", "5.0"));
    const auto* val = insp.getValue("transform", "x");
    REQUIRE(val != nullptr);
    REQUIRE(*val == "5.0");
}

TEST_CASE("PropertyInspector: setValue on readOnly fails", "[property][inspector]") {
    NF::PropertyInspector insp;
    insp.addCategory(makeCat("info", "Info"));
    auto prop = makeProp("type", "Type", NF::PropertyType::String, "Mesh");
    prop.readOnly = true;
    insp.addProperty("info", prop);

    REQUIRE_FALSE(insp.setValue("info", "type", "Light"));
    REQUIRE(*insp.getValue("info", "type") == "Mesh");
}

TEST_CASE("PropertyInspector: setValue unknown category", "[property][inspector]") {
    NF::PropertyInspector insp;
    REQUIRE_FALSE(insp.setValue("missing", "x", "1.0"));
}

TEST_CASE("PropertyInspector: setValue same value no-op", "[property][inspector]") {
    NF::PropertyInspector insp;
    insp.addCategory(makeCat("transform", "Transform"));
    insp.addProperty("transform", makeProp("x", "X", NF::PropertyType::Float, "5.0"));

    int callCount = 0;
    insp.addObserver([&](const std::string&, const std::string&, const std::string&) { ++callCount; });
    REQUIRE(insp.setValue("transform", "x", "5.0"));
    REQUIRE(callCount == 0);
}

TEST_CASE("PropertyInspector: getValue unknown", "[property][inspector]") {
    NF::PropertyInspector insp;
    REQUIRE(insp.getValue("missing", "x") == nullptr);
}

TEST_CASE("PropertyInspector: searchByName", "[property][inspector]") {
    NF::PropertyInspector insp;
    insp.addCategory(makeCat("transform", "Transform"));
    insp.addProperty("transform", makeProp("pos_x", "Position X"));
    insp.addProperty("transform", makeProp("pos_y", "Position Y"));
    insp.addProperty("transform", makeProp("rot", "Rotation"));

    auto results = insp.searchByName("position");
    REQUIRE(results.size() == 2);
}

TEST_CASE("PropertyInspector: searchByName empty", "[property][inspector]") {
    NF::PropertyInspector insp;
    insp.addCategory(makeCat("transform", "Transform"));
    insp.addProperty("transform", makeProp("x", "X"));
    REQUIRE(insp.searchByName("").empty());
}

TEST_CASE("PropertyInspector: filterByType", "[property][inspector]") {
    NF::PropertyInspector insp;
    insp.addCategory(makeCat("transform", "Transform"));
    insp.addProperty("transform", makeProp("x", "X", NF::PropertyType::Float));
    insp.addProperty("transform", makeProp("y", "Y", NF::PropertyType::Float));
    insp.addProperty("transform", makeProp("name", "Name", NF::PropertyType::String));

    auto floats = insp.filterByType(NF::PropertyType::Float);
    REQUIRE(floats.size() == 2);
}

TEST_CASE("PropertyInspector: observer on setValue", "[property][inspector]") {
    NF::PropertyInspector insp;
    insp.addCategory(makeCat("transform", "Transform"));
    insp.addProperty("transform", makeProp("x", "X", NF::PropertyType::Float, "0.0"));

    std::string observedProp, observedOld, observedNew;
    insp.addObserver([&](const std::string& id, const std::string& o, const std::string& n) {
        observedProp = id;
        observedOld  = o;
        observedNew  = n;
    });

    insp.setValue("transform", "x", "10.0");
    REQUIRE(observedProp == "x");
    REQUIRE(observedOld == "0.0");
    REQUIRE(observedNew == "10.0");
}

TEST_CASE("PropertyInspector: clearObservers", "[property][inspector]") {
    NF::PropertyInspector insp;
    insp.addCategory(makeCat("transform", "Transform"));
    insp.addProperty("transform", makeProp("x", "X", NF::PropertyType::Float, "0.0"));

    int callCount = 0;
    insp.addObserver([&](const std::string&, const std::string&, const std::string&) { ++callCount; });
    insp.clearObservers();
    insp.setValue("transform", "x", "5.0");
    REQUIRE(callCount == 0);
}

TEST_CASE("PropertyInspector: serialize empty", "[property][serial]") {
    NF::PropertyInspector insp;
    REQUIRE(insp.serialize().empty());
}

TEST_CASE("PropertyInspector: serialize round-trip", "[property][serial]") {
    NF::PropertyInspector insp;
    insp.addCategory(makeCat("transform", "Transform"));
    insp.addProperty("transform", makeProp("x", "X", NF::PropertyType::Float, "1.5"));
    auto prop2 = makeProp("name", "Name", NF::PropertyType::String, "MyObj");
    prop2.readOnly = true;
    prop2.description = "Object name";
    insp.addProperty("transform", prop2);

    std::string data = insp.serialize();
    REQUIRE_FALSE(data.empty());

    NF::PropertyInspector insp2;
    REQUIRE(insp2.deserialize(data));
    REQUIRE(insp2.categoryCount() == 1);
    REQUIRE(insp2.totalProperties() == 2);

    const auto* cat = insp2.findCategory("transform");
    REQUIRE(cat != nullptr);
    const auto* p1 = cat->findProperty("x");
    REQUIRE(p1 != nullptr);
    REQUIRE(p1->value == "1.5");
    REQUIRE(p1->type == NF::PropertyType::Float);

    const auto* p2 = cat->findProperty("name");
    REQUIRE(p2 != nullptr);
    REQUIRE(p2->readOnly == true);
    REQUIRE(p2->description == "Object name");
}

TEST_CASE("PropertyInspector: serialize pipe in value", "[property][serial]") {
    NF::PropertyInspector insp;
    insp.addCategory(makeCat("data", "Data"));
    insp.addProperty("data", makeProp("opts", "Options", NF::PropertyType::Enum, "a|b|c"));

    std::string data = insp.serialize();
    NF::PropertyInspector insp2;
    insp2.deserialize(data);
    auto* cat = insp2.findCategory("data");
    REQUIRE(cat != nullptr);
    REQUIRE(cat->findProperty("opts")->value == "a|b|c");
}

TEST_CASE("PropertyInspector: deserialize empty", "[property][serial]") {
    NF::PropertyInspector insp;
    REQUIRE(insp.deserialize(""));
    REQUIRE(insp.categoryCount() == 0);
}

TEST_CASE("PropertyInspector: clear", "[property][inspector]") {
    NF::PropertyInspector insp;
    insp.addCategory(makeCat("transform", "Transform"));
    insp.addProperty("transform", makeProp("x", "X"));
    int calls = 0;
    insp.addObserver([&](const std::string&, const std::string&, const std::string&) { ++calls; });
    insp.clear();
    REQUIRE(insp.categoryCount() == 0);
    REQUIRE(insp.totalProperties() == 0);
}

// ═════════════════════════════════════════════════════════════════
// Integration
// ═════════════════════════════════════════════════════════════════

TEST_CASE("Integration: multi-category inspector with search", "[property][integration]") {
    NF::PropertyInspector insp;
    insp.addCategory(makeCat("transform", "Transform"));
    insp.addCategory(makeCat("material", "Material"));

    insp.addProperty("transform", makeProp("pos_x", "Position X", NF::PropertyType::Float, "0.0"));
    insp.addProperty("transform", makeProp("pos_y", "Position Y", NF::PropertyType::Float, "0.0"));
    insp.addProperty("transform", makeProp("scale", "Scale", NF::PropertyType::Float, "1.0"));
    insp.addProperty("material", makeProp("color", "Color", NF::PropertyType::Color, "FF0000"));
    insp.addProperty("material", makeProp("shader", "Shader Name", NF::PropertyType::String, "PBR"));

    REQUIRE(insp.totalProperties() == 5);
    REQUIRE(insp.searchByName("position").size() == 2);
    REQUIRE(insp.filterByType(NF::PropertyType::Float).size() == 3);
    REQUIRE(insp.filterByType(NF::PropertyType::Color).size() == 1);
}

TEST_CASE("Integration: serialize/deserialize preserves categories and properties", "[property][integration]") {
    NF::PropertyInspector insp;
    auto cat = makeCat("transform", "Transform");
    cat.collapsed = true;
    insp.addCategory(cat);
    insp.addProperty("transform", makeProp("x", "X", NF::PropertyType::Float, "1.0"));

    insp.addCategory(makeCat("info", "Info"));
    auto roProp = makeProp("type", "Type", NF::PropertyType::String, "Mesh");
    roProp.readOnly = true;
    insp.addProperty("info", roProp);

    std::string data = insp.serialize();
    NF::PropertyInspector insp2;
    insp2.deserialize(data);

    REQUIRE(insp2.categoryCount() == 2);
    REQUIRE(insp2.findCategory("transform")->collapsed == true);
    REQUIRE(insp2.findCategory("info") != nullptr);
    REQUIRE(insp2.findCategory("info")->findProperty("type")->readOnly == true);
}
