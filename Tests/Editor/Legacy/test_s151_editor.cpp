// S151 editor tests: SceneTreeEditorV1, ComponentInspectorV1, EntityQueryV1
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/EntityQueryV1.h"
#include "NF/Editor/SceneTreeEditorV1.h"

using namespace NF;
using Catch::Approx;

// ── SceneTreeEditorV1 ─────────────────────────────────────────────────────────

TEST_CASE("SteNodeType names", "[Editor][S151]") {
    REQUIRE(std::string(steNodeTypeName(SteNodeType::Entity)) == "Entity");
    REQUIRE(std::string(steNodeTypeName(SteNodeType::Group))  == "Group");
    REQUIRE(std::string(steNodeTypeName(SteNodeType::Prefab)) == "Prefab");
    REQUIRE(std::string(steNodeTypeName(SteNodeType::Camera)) == "Camera");
    REQUIRE(std::string(steNodeTypeName(SteNodeType::Light))  == "Light");
    REQUIRE(std::string(steNodeTypeName(SteNodeType::Volume)) == "Volume");
}

TEST_CASE("SteNode validity and isRoot", "[Editor][S151]") {
    SteNode n;
    REQUIRE(!n.isValid());
    n.id = 1; n.name = "Player";
    REQUIRE(n.isValid());
    REQUIRE(n.isRoot());
    n.parentId = 2;
    REQUIRE(!n.isRoot());
}

TEST_CASE("SceneTreeEditorV1 addNode and nodeCount", "[Editor][S151]") {
    SceneTreeEditorV1 ste;
    SteNode n; n.id = 1; n.name = "Root";
    REQUIRE(ste.addNode(n));
    REQUIRE(ste.nodeCount() == 1);
}

TEST_CASE("SceneTreeEditorV1 reject duplicate node", "[Editor][S151]") {
    SceneTreeEditorV1 ste;
    SteNode n; n.id = 2; n.name = "Env";
    REQUIRE(ste.addNode(n));
    REQUIRE(!ste.addNode(n));
}

TEST_CASE("SceneTreeEditorV1 removeNode", "[Editor][S151]") {
    SceneTreeEditorV1 ste;
    SteNode n; n.id = 3; n.name = "HUD";
    ste.addNode(n);
    REQUIRE(ste.removeNode(3));
    REQUIRE(ste.nodeCount() == 0);
    REQUIRE(!ste.removeNode(3));
}

TEST_CASE("SceneTreeEditorV1 addNode updates parent children", "[Editor][S151]") {
    SceneTreeEditorV1 ste;
    SteNode parent; parent.id = 1; parent.name = "World";
    SteNode child;  child.id  = 2; child.name  = "Rock"; child.parentId = 1;
    ste.addNode(parent);
    ste.addNode(child);
    // find parent and check children
    const SteNode* p = ste.findNode("World");
    REQUIRE(p != nullptr);
    REQUIRE(p->children.size() == 1);
    REQUIRE(p->children[0] == 2);
}

TEST_CASE("SceneTreeEditorV1 removeNode updates parent children", "[Editor][S151]") {
    SceneTreeEditorV1 ste;
    SteNode parent; parent.id = 1; parent.name = "Scene";
    SteNode child;  child.id  = 2; child.name  = "NPC"; child.parentId = 1;
    ste.addNode(parent); ste.addNode(child);
    ste.removeNode(2);
    const SteNode* p = ste.findNode("Scene");
    REQUIRE(p != nullptr);
    REQUIRE(p->children.empty());
}

TEST_CASE("SceneTreeEditorV1 rootNodes", "[Editor][S151]") {
    SceneTreeEditorV1 ste;
    SteNode r1; r1.id = 1; r1.name = "R1";
    SteNode r2; r2.id = 2; r2.name = "R2";
    SteNode c;  c.id  = 3; c.name  = "C"; c.parentId = 1;
    ste.addNode(r1); ste.addNode(r2); ste.addNode(c);
    auto roots = ste.rootNodes();
    REQUIRE(roots.size() == 2);
}

TEST_CASE("SceneTreeEditorV1 reparent", "[Editor][S151]") {
    SceneTreeEditorV1 ste;
    SteNode p1; p1.id = 1; p1.name = "Parent1";
    SteNode p2; p2.id = 2; p2.name = "Parent2";
    SteNode ch; ch.id = 3; ch.name = "Child"; ch.parentId = 1;
    ste.addNode(p1); ste.addNode(p2); ste.addNode(ch);
    REQUIRE(ste.reparent(3, 2));
    const SteNode* np1 = ste.findNode("Parent1");
    const SteNode* np2 = ste.findNode("Parent2");
    REQUIRE(np1->children.empty());
    REQUIRE(np2->children.size() == 1);
}

TEST_CASE("SceneTreeEditorV1 setVisibility", "[Editor][S151]") {
    SceneTreeEditorV1 ste;
    SteNode n; n.id = 4; n.name = "Tree";
    ste.addNode(n);
    REQUIRE(ste.setVisibility(4, false));
    const SteNode* found = ste.findNode("Tree");
    REQUIRE(found != nullptr);
    REQUIRE(!found->visible);
}

TEST_CASE("SceneTreeEditorV1 toggleLock", "[Editor][S151]") {
    SceneTreeEditorV1 ste;
    SteNode n; n.id = 5; n.name = "Ground";
    ste.addNode(n);
    REQUIRE(!ste.findNode("Ground")->locked);
    REQUIRE(ste.toggleLock(5));
    REQUIRE(ste.findNode("Ground")->locked);
    REQUIRE(ste.toggleLock(5));
    REQUIRE(!ste.findNode("Ground")->locked);
}

TEST_CASE("SceneTreeEditorV1 selectNode fires callback", "[Editor][S151]") {
    SceneTreeEditorV1 ste;
    uint32_t selected = 0;
    ste.setOnSelect([&](uint32_t id){ selected = id; });
    ste.selectNode(42);
    REQUIRE(selected == 42);
}

// ── ComponentInspectorV1 ─────────────────────────────────────────────────────

TEST_CASE("CiProperty validity and isAtDefault", "[Editor][S151]") {
    CiProperty p;
    REQUIRE(!p.isValid());
    p.id = 1; p.name = "speed"; p.value = "10"; p.defaultValue = "5";
    REQUIRE(p.isValid());
    REQUIRE(!p.isAtDefault());
    p.value = "5";
    REQUIRE(p.isAtDefault());
}

TEST_CASE("ComponentInspectorV1 setTarget and addComponent", "[Editor][S151]") {
    ComponentInspectorV1 ci;
    ci.setTarget(100);
    REQUIRE(ci.targetEntity() == 100);
    CiComponent comp; comp.id = 1; comp.typeName = "Transform";
    REQUIRE(ci.addComponent(comp));
    REQUIRE(ci.componentCount() == 1);
}

TEST_CASE("ComponentInspectorV1 setTarget clears components", "[Editor][S151]") {
    ComponentInspectorV1 ci;
    ci.setTarget(1);
    CiComponent comp; comp.id = 1; comp.typeName = "Mesh";
    ci.addComponent(comp);
    ci.setTarget(2);
    REQUIRE(ci.componentCount() == 0);
}

TEST_CASE("ComponentInspectorV1 removeComponent", "[Editor][S151]") {
    ComponentInspectorV1 ci;
    ci.setTarget(1);
    CiComponent comp; comp.id = 2; comp.typeName = "Light";
    ci.addComponent(comp);
    REQUIRE(ci.removeComponent(2));
    REQUIRE(ci.componentCount() == 0);
}

TEST_CASE("ComponentInspectorV1 setPropertyValue and getProperty", "[Editor][S151]") {
    ComponentInspectorV1 ci;
    ci.setTarget(1);
    CiComponent comp; comp.id = 1; comp.typeName = "Physics";
    CiProperty prop; prop.id = 1; prop.name = "mass"; prop.value = "1.0"; prop.defaultValue = "1.0";
    comp.properties.push_back(prop);
    ci.addComponent(comp);
    REQUIRE(ci.setPropertyValue(1, "mass", "5.0"));
    auto* p = ci.getProperty(1, "mass");
    REQUIRE(p != nullptr);
    REQUIRE(p->value == "5.0");
}

TEST_CASE("ComponentInspectorV1 setPropertyValue read-only rejected", "[Editor][S151]") {
    ComponentInspectorV1 ci;
    ci.setTarget(1);
    CiComponent comp; comp.id = 1; comp.typeName = "Transform";
    CiProperty prop; prop.id = 1; prop.name = "instanceId"; prop.readOnly = true; prop.value = "42";
    comp.properties.push_back(prop);
    ci.addComponent(comp);
    REQUIRE(!ci.setPropertyValue(1, "instanceId", "99"));
}

// ── EntityQueryV1 ─────────────────────────────────────────────────────────────

TEST_CASE("EntityQueryV1 addFilter and filterCount", "[Editor][S151]") {
    EntityQueryV1 eq;
    EqFilter f; f.op = EqFilterOp::HasComponent; f.arg = "Transform";
    eq.addFilter(f);
    REQUIRE(eq.filterCount() == 1);
}

TEST_CASE("EntityQueryV1 clearFilters", "[Editor][S151]") {
    EntityQueryV1 eq;
    EqFilter f; f.op = EqFilterOp::HasTag; f.arg = "player";
    eq.addFilter(f);
    eq.clearFilters();
    REQUIRE(eq.filterCount() == 0);
}

TEST_CASE("EntityQueryV1 execute with no filters returns all", "[Editor][S151]") {
    EntityQueryV1 eq;
    auto result = eq.execute({1, 2, 3});
    REQUIRE(result.entityIds.size() == 3);
    REQUIRE(result.totalChecked == 3);
}

TEST_CASE("EntityQueryV1 lastResult and callback", "[Editor][S151]") {
    EntityQueryV1 eq;
    bool called = false;
    eq.setOnResult([&](const EqResult&){ called = true; });
    eq.execute({10, 20});
    REQUIRE(called);
    REQUIRE(eq.lastResult().totalChecked == 2);
}
