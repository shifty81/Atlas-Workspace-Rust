#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/PrefabEditor.h"

using namespace NF;

TEST_CASE("PrefabCategory names cover all 5 values", "[Editor][S42]") {
    REQUIRE(std::string(prefabCategoryName(PrefabCategory::Prop))      == "Prop");
    REQUIRE(std::string(prefabCategoryName(PrefabCategory::Character)) == "Character");
    REQUIRE(std::string(prefabCategoryName(PrefabCategory::Vehicle))   == "Vehicle");
    REQUIRE(std::string(prefabCategoryName(PrefabCategory::Structure)) == "Structure");
    REQUIRE(std::string(prefabCategoryName(PrefabCategory::Effect))    == "Effect");
}

TEST_CASE("PrefabState names cover all 5 values", "[Editor][S42]") {
    REQUIRE(std::string(prefabStateName(PrefabState::Unloaded))  == "Unloaded");
    REQUIRE(std::string(prefabStateName(PrefabState::Loaded))    == "Loaded");
    REQUIRE(std::string(prefabStateName(PrefabState::Instanced)) == "Instanced");
    REQUIRE(std::string(prefabStateName(PrefabState::Modified))  == "Modified");
    REQUIRE(std::string(prefabStateName(PrefabState::Broken))    == "Broken");
}

TEST_CASE("PrefabLOD names cover all 5 values", "[Editor][S42]") {
    REQUIRE(std::string(prefabLODName(PrefabLOD::Full))   == "Full");
    REQUIRE(std::string(prefabLODName(PrefabLOD::High))   == "High");
    REQUIRE(std::string(prefabLODName(PrefabLOD::Medium)) == "Medium");
    REQUIRE(std::string(prefabLODName(PrefabLOD::Low))    == "Low");
    REQUIRE(std::string(prefabLODName(PrefabLOD::Proxy))  == "Proxy");
}

TEST_CASE("PrefabAsset default values", "[Editor][S42]") {
    PrefabAsset p("tree_01");
    REQUIRE(p.name()           == "tree_01");
    REQUIRE(p.componentCount() == 0u);
    REQUIRE(p.childCount()     == 0u);
    REQUIRE(p.category()       == PrefabCategory::Prop);
    REQUIRE(p.state()          == PrefabState::Unloaded);
    REQUIRE(p.lod()            == PrefabLOD::Full);
    REQUIRE_FALSE(p.isOverridable());
    REQUIRE_FALSE(p.isNested());
    REQUIRE_FALSE(p.isDirty());
    REQUIRE_FALSE(p.isInstanced());
    REQUIRE_FALSE(p.isModified());
    REQUIRE_FALSE(p.isBroken());
    REQUIRE_FALSE(p.isComplex());
}

TEST_CASE("PrefabAsset setters round-trip", "[Editor][S42]") {
    PrefabAsset p("vehicle_car", 5, 3);
    p.setCategory(PrefabCategory::Vehicle);
    p.setState(PrefabState::Instanced);
    p.setLOD(PrefabLOD::Medium);
    p.setComponentCount(15);
    p.setChildCount(8);
    p.setOverridable(true);
    p.setNested(true);
    p.setDirty(true);

    REQUIRE(p.category()       == PrefabCategory::Vehicle);
    REQUIRE(p.state()          == PrefabState::Instanced);
    REQUIRE(p.lod()            == PrefabLOD::Medium);
    REQUIRE(p.componentCount() == 15u);
    REQUIRE(p.childCount()     == 8u);
    REQUIRE(p.isOverridable());
    REQUIRE(p.isNested());
    REQUIRE(p.isDirty());
    REQUIRE(p.isInstanced());
    REQUIRE_FALSE(p.isModified());
    REQUIRE_FALSE(p.isBroken());
    REQUIRE(p.isComplex());
}

TEST_CASE("PrefabAsset isComplex requires componentCount>=10", "[Editor][S42]") {
    PrefabAsset p("simple");
    p.setComponentCount(9);
    REQUIRE_FALSE(p.isComplex());

    p.setComponentCount(10);
    REQUIRE(p.isComplex());
}

TEST_CASE("PrefabAsset isModified and isBroken states", "[Editor][S42]") {
    PrefabAsset p("rock");
    p.setState(PrefabState::Modified);
    REQUIRE(p.isModified());
    REQUIRE_FALSE(p.isInstanced());

    p.setState(PrefabState::Broken);
    REQUIRE(p.isBroken());
    REQUIRE_FALSE(p.isModified());
}

TEST_CASE("ScenePrefabEditor addPrefab and duplicate rejection", "[Editor][S42]") {
    ScenePrefabEditor editor;
    PrefabAsset a("a"), b("b"), dup("a");
    REQUIRE(editor.addPrefab(a));
    REQUIRE(editor.addPrefab(b));
    REQUIRE_FALSE(editor.addPrefab(dup));
    REQUIRE(editor.prefabCount() == 2);
}

TEST_CASE("ScenePrefabEditor removePrefab clears activePrefab", "[Editor][S42]") {
    ScenePrefabEditor editor;
    PrefabAsset p("hero");
    editor.addPrefab(p);
    editor.setActivePrefab("hero");
    REQUIRE(editor.activePrefab() == "hero");

    editor.removePrefab("hero");
    REQUIRE(editor.prefabCount() == 0);
    REQUIRE(editor.activePrefab().empty());
}

TEST_CASE("ScenePrefabEditor findPrefab returns pointer or nullptr", "[Editor][S42]") {
    ScenePrefabEditor editor;
    PrefabAsset p("chest");
    editor.addPrefab(p);

    REQUIRE(editor.findPrefab("chest") != nullptr);
    REQUIRE(editor.findPrefab("chest")->name() == "chest");
    REQUIRE(editor.findPrefab("missing") == nullptr);
}

TEST_CASE("ScenePrefabEditor aggregate counts", "[Editor][S42]") {
    ScenePrefabEditor editor;

    PrefabAsset a("a"); a.setDirty(true); a.setState(PrefabState::Instanced); a.setOverridable(true); a.setNested(true); a.setComponentCount(20);
    PrefabAsset b("b"); b.setDirty(true); b.setComponentCount(3);
    PrefabAsset c("c"); c.setOverridable(true); c.setNested(true); c.setComponentCount(12);

    editor.addPrefab(a); editor.addPrefab(b); editor.addPrefab(c);

    REQUIRE(editor.dirtyCount()       == 2);
    REQUIRE(editor.instancedCount()   == 1);
    REQUIRE(editor.overridableCount() == 2);
    REQUIRE(editor.nestedCount()      == 2);
    REQUIRE(editor.complexCount()     == 2); // a and c
}

TEST_CASE("ScenePrefabEditor countByCategory and countByState", "[Editor][S42]") {
    ScenePrefabEditor editor;

    PrefabAsset a("a"); a.setCategory(PrefabCategory::Character); a.setState(PrefabState::Instanced);
    PrefabAsset b("b"); b.setCategory(PrefabCategory::Character); b.setState(PrefabState::Unloaded);
    PrefabAsset c("c"); c.setCategory(PrefabCategory::Vehicle);   c.setState(PrefabState::Instanced);

    editor.addPrefab(a); editor.addPrefab(b); editor.addPrefab(c);

    REQUIRE(editor.countByCategory(PrefabCategory::Character)   == 2);
    REQUIRE(editor.countByCategory(PrefabCategory::Vehicle)     == 1);
    REQUIRE(editor.countByState(PrefabState::Instanced)         == 2);
    REQUIRE(editor.countByState(PrefabState::Unloaded)          == 1);
}

TEST_CASE("ScenePrefabEditor setActivePrefab returns false for missing", "[Editor][S42]") {
    ScenePrefabEditor editor;
    REQUIRE_FALSE(editor.setActivePrefab("ghost"));
    REQUIRE(editor.activePrefab().empty());
}

TEST_CASE("ScenePrefabEditor MAX_PREFABS limit enforced", "[Editor][S42]") {
    ScenePrefabEditor editor;
    for (size_t i = 0; i < ScenePrefabEditor::MAX_PREFABS; ++i) {
        PrefabAsset p("Prefab" + std::to_string(i));
        REQUIRE(editor.addPrefab(p));
    }
    PrefabAsset overflow("Overflow");
    REQUIRE_FALSE(editor.addPrefab(overflow));
    REQUIRE(editor.prefabCount() == ScenePrefabEditor::MAX_PREFABS);
}
