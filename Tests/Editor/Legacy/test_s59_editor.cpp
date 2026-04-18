#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"

using namespace NF;

// ── InspectorPanel ───────────────────────────────────────────────

TEST_CASE("InspectorPanel default name is Inspector", "[Editor][S59]") {
    InspectorPanel panel;
    REQUIRE(panel.name() == "Inspector");
}

TEST_CASE("InspectorPanel default slot is Right", "[Editor][S59]") {
    InspectorPanel panel;
    REQUIRE(panel.slot() == DockSlot::Right);
}

TEST_CASE("InspectorPanel starts with no selection service", "[Editor][S59]") {
    InspectorPanel panel;
    REQUIRE(panel.selectionService() == nullptr);
}

TEST_CASE("InspectorPanel setSelectionService stores pointer", "[Editor][S59]") {
    InspectorPanel panel;
    SelectionService svc;
    panel.setSelectionService(&svc);
    REQUIRE(panel.selectionService() == &svc);
}

TEST_CASE("InspectorPanel starts with no type registry", "[Editor][S59]") {
    InspectorPanel panel;
    REQUIRE(panel.typeRegistry() == nullptr);
}

TEST_CASE("InspectorPanel setTypeRegistry stores pointer", "[Editor][S59]") {
    InspectorPanel panel;
    TypeRegistry& reg = TypeRegistry::instance();
    panel.setTypeRegistry(&reg);
    REQUIRE(panel.typeRegistry() == &reg);
}

TEST_CASE("InspectorPanel constructor with selection and registry", "[Editor][S59]") {
    SelectionService svc;
    TypeRegistry& reg = TypeRegistry::instance();
    InspectorPanel panel(&svc, &reg);
    REQUIRE(panel.selectionService() == &svc);
    REQUIRE(panel.typeRegistry() == &reg);
}

TEST_CASE("InspectorPanel is an EditorPanel", "[Editor][S59]") {
    InspectorPanel panel;
    EditorPanel* base = &panel;
    REQUIRE(base != nullptr);
    REQUIRE(base->name() == "Inspector");
}

// ── HierarchyPanel ───────────────────────────────────────────────

TEST_CASE("HierarchyPanel default name is Hierarchy", "[Editor][S59]") {
    HierarchyPanel panel;
    REQUIRE(panel.name() == "Hierarchy");
}

TEST_CASE("HierarchyPanel default slot is Left", "[Editor][S59]") {
    HierarchyPanel panel;
    REQUIRE(panel.slot() == DockSlot::Left);
}

TEST_CASE("HierarchyPanel starts with no selection service", "[Editor][S59]") {
    HierarchyPanel panel;
    REQUIRE(panel.selectionService() == nullptr);
}

TEST_CASE("HierarchyPanel setSelectionService stores pointer", "[Editor][S59]") {
    HierarchyPanel panel;
    SelectionService svc;
    panel.setSelectionService(&svc);
    REQUIRE(panel.selectionService() == &svc);
}

TEST_CASE("HierarchyPanel constructor with selection service", "[Editor][S59]") {
    SelectionService svc;
    HierarchyPanel panel(&svc);
    REQUIRE(panel.selectionService() == &svc);
}

TEST_CASE("HierarchyPanel default search filter is empty", "[Editor][S59]") {
    HierarchyPanel panel;
    REQUIRE(panel.searchFilter().empty());
}

TEST_CASE("HierarchyPanel setSearchFilter stores filter string", "[Editor][S59]") {
    HierarchyPanel panel;
    panel.setSearchFilter("Player");
    REQUIRE(panel.searchFilter() == "Player");
}

TEST_CASE("HierarchyPanel setEntityList stores entity IDs", "[Editor][S59]") {
    HierarchyPanel panel;
    std::vector<EntityID> ids = {1u, 2u, 3u, 10u};
    panel.setEntityList(ids);
    REQUIRE(panel.entityList().size() == 4);
    REQUIRE(panel.entityList()[0] == 1u);
    REQUIRE(panel.entityList()[3] == 10u);
}

TEST_CASE("HierarchyPanel setEntityList replaces previous list", "[Editor][S59]") {
    HierarchyPanel panel;
    panel.setEntityList({1u, 2u, 3u});
    panel.setEntityList({5u, 6u});
    REQUIRE(panel.entityList().size() == 2);
    REQUIRE(panel.entityList()[0] == 5u);
}

TEST_CASE("HierarchyPanel is an EditorPanel", "[Editor][S59]") {
    HierarchyPanel panel;
    EditorPanel* base = &panel;
    REQUIRE(base != nullptr);
    REQUIRE(base->name() == "Hierarchy");
}
