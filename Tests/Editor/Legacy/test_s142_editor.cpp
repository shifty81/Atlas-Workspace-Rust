// S142 editor tests: PropertyGridV1, TreeViewV1, TableViewV1
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"

using namespace NF;
using Catch::Approx;

// ── PropertyGridV1 ────────────────────────────────────────────────────────────

TEST_CASE("PropValueType names", "[Editor][S142]") {
    REQUIRE(std::string(propValueTypeName(PropValueType::Bool))     == "Bool");
    REQUIRE(std::string(propValueTypeName(PropValueType::Int))      == "Int");
    REQUIRE(std::string(propValueTypeName(PropValueType::Float))    == "Float");
    REQUIRE(std::string(propValueTypeName(PropValueType::String))   == "String");
    REQUIRE(std::string(propValueTypeName(PropValueType::Color))    == "Color");
    REQUIRE(std::string(propValueTypeName(PropValueType::Vec2))     == "Vec2");
    REQUIRE(std::string(propValueTypeName(PropValueType::Vec3))     == "Vec3");
    REQUIRE(std::string(propValueTypeName(PropValueType::Enum))     == "Enum");
    REQUIRE(std::string(propValueTypeName(PropValueType::Asset))    == "Asset");
    REQUIRE(std::string(propValueTypeName(PropValueType::ReadOnly)) == "ReadOnly");
}

TEST_CASE("PropertyRow isValid and displayValue", "[Editor][S142]") {
    PropertyRow r;
    REQUIRE(!r.isValid());
    r.id = 1; r.key = "pos"; r.label = "Position";
    REQUIRE(r.isValid());

    r.valueType = PropValueType::String;
    r.value = std::string("hello");
    REQUIRE(r.displayValue() == "hello");

    r.value = true;
    REQUIRE(r.displayValue() == "true");
    r.value = false;
    REQUIRE(r.displayValue() == "false");

    r.value = 42;
    REQUIRE(r.displayValue() == "42");
}

TEST_CASE("PropertyRow displayValue Vec2 and Vec3", "[Editor][S142]") {
    PropertyRow r;
    r.id = 1; r.key = "k";
    r.value = PropVec2{1.f, 2.f};
    std::string s = r.displayValue();
    REQUIRE(s.find("1.") != std::string::npos);
    REQUIRE(s.find("2.") != std::string::npos);

    r.value = PropVec3{1.f, 2.f, 3.f};
    s = r.displayValue();
    REQUIRE(s.find("3.") != std::string::npos);
}

TEST_CASE("PropertyGroup isValid and findRow", "[Editor][S142]") {
    PropertyGroup g;
    REQUIRE(!g.isValid());
    g.id = 1; g.label = "Transform";
    REQUIRE(g.isValid());
    REQUIRE(g.rowCount() == 0u);

    PropertyRow r; r.id = 1; r.key = "pos"; r.label = "Pos";
    g.rows.push_back(r);
    REQUIRE(g.rowCount() == 1u);
    REQUIRE(g.findRow("pos") != nullptr);
    REQUIRE(g.findRow("nonexistent") == nullptr);
}

TEST_CASE("PropertyGridV1 addGroup and addRow", "[Editor][S142]") {
    PropertyGridV1 grid;
    PropertyGroup g; g.id = 1; g.label = "Transform";
    REQUIRE(grid.addGroup(g));
    REQUIRE(grid.groupCount() == 1u);

    PropertyRow r; r.id = 1; r.key = "pos"; r.label = "Position";
    r.valueType = PropValueType::Vec3; r.value = PropVec3{0,0,0};
    REQUIRE(grid.addRow(1, r));
    REQUIRE(grid.totalRowCount() == 1u);
}

TEST_CASE("PropertyGridV1 reject duplicate group id", "[Editor][S142]") {
    PropertyGridV1 grid;
    PropertyGroup g; g.id = 1; g.label = "G";
    REQUIRE(grid.addGroup(g));
    REQUIRE(!grid.addGroup(g));
}

TEST_CASE("PropertyGridV1 setValue triggers onChange", "[Editor][S142]") {
    PropertyGridV1 grid;
    PropertyGroup g; g.id = 1; g.label = "G";
    grid.addGroup(g);
    PropertyRow r; r.id = 1; r.key = "x"; r.label = "X";
    r.valueType = PropValueType::Float; r.value = 0.f;
    grid.addRow(1, r);

    bool called = false;
    PropVariant oldCaptured;
    grid.setOnChange([&](const PropertyRow& row, const PropVariant& old) {
        called = true;
        oldCaptured = old;
    });

    REQUIRE(grid.setValue(1, PropVariant{42.f}));
    REQUIRE(called);
    REQUIRE(std::get<float>(oldCaptured) == Approx(0.f));
    REQUIRE(grid.changeCount() == 1u);
    REQUIRE(grid.dirtyRowCount() == 1u);
}

TEST_CASE("PropertyGridV1 readOnly row rejects setValue", "[Editor][S142]") {
    PropertyGridV1 grid;
    PropertyGroup g; g.id = 1; g.label = "G";
    grid.addGroup(g);
    PropertyRow r; r.id = 1; r.key = "ro"; r.label = "ReadOnly";
    r.readOnly = true; r.value = std::string("locked");
    grid.addRow(1, r);
    REQUIRE(!grid.setValue(1, PropVariant{std::string("changed")}));
    REQUIRE(grid.changeCount() == 0u);
}

TEST_CASE("PropertyGridV1 clearDirty", "[Editor][S142]") {
    PropertyGridV1 grid;
    PropertyGroup g; g.id = 1; g.label = "G";
    grid.addGroup(g);
    PropertyRow r; r.id = 1; r.key = "v"; r.label = "V"; r.value = 0;
    grid.addRow(1, r);
    grid.setValue(1, PropVariant{5});
    REQUIRE(grid.dirtyRowCount() == 1u);
    grid.clearDirty();
    REQUIRE(grid.dirtyRowCount() == 0u);
}

TEST_CASE("PropertyGridV1 removeGroup", "[Editor][S142]") {
    PropertyGridV1 grid;
    PropertyGroup g; g.id = 1; g.label = "G";
    grid.addGroup(g);
    REQUIRE(grid.groupCount() == 1u);
    REQUIRE(grid.removeGroup(1));
    REQUIRE(grid.groupCount() == 0u);
}

TEST_CASE("PropertyGridV1 setGroupExpanded", "[Editor][S142]") {
    PropertyGridV1 grid;
    PropertyGroup g; g.id = 1; g.label = "G"; g.expanded = true;
    grid.addGroup(g);
    grid.setGroupExpanded(1, false);
    REQUIRE(!grid.findGroup(1)->expanded);
    grid.setGroupExpanded(1, true);
    REQUIRE(grid.findGroup(1)->expanded);
}

TEST_CASE("PropertyGridV1 findRowByKey", "[Editor][S142]") {
    PropertyGridV1 grid;
    PropertyGroup g; g.id = 1; g.label = "G";
    grid.addGroup(g);
    PropertyRow r; r.id = 1; r.key = "mykey"; r.label = "K"; r.value = std::string("v");
    grid.addRow(1, r);
    REQUIRE(grid.findRowByKey("mykey") != nullptr);
    REQUIRE(grid.findRowByKey("other") == nullptr);
}

// ── TreeViewV1 ────────────────────────────────────────────────────────────────

TEST_CASE("TreeNodeFlag operations", "[Editor][S142]") {
    TreeNodeFlag f = TreeNodeFlag::None;
    REQUIRE(!hasFlag(f, TreeNodeFlag::Expanded));
    f = f | TreeNodeFlag::Expanded;
    REQUIRE(hasFlag(f, TreeNodeFlag::Expanded));
}

TEST_CASE("TreeNode isValid and flags", "[Editor][S142]") {
    TreeNode n;
    REQUIRE(!n.isValid());
    n.id = 1; n.label = "Root";
    REQUIRE(n.isValid());
    REQUIRE(n.isRoot());
    REQUIRE(!n.isExpanded());
    n.setFlag(TreeNodeFlag::Expanded);
    REQUIRE(n.isExpanded());
    n.clearFlag(TreeNodeFlag::Expanded);
    REQUIRE(!n.isExpanded());
}

TEST_CASE("TreeViewV1 addNode basic", "[Editor][S142]") {
    TreeViewV1 tree;
    TreeNode r; r.id = 1; r.label = "Root";
    REQUIRE(tree.addNode(r));
    REQUIRE(tree.nodeCount() == 1u);
    REQUIRE(tree.findNode(1) != nullptr);
}

TEST_CASE("TreeViewV1 reject duplicate node id", "[Editor][S142]") {
    TreeViewV1 tree;
    TreeNode n; n.id = 1; n.label = "N";
    REQUIRE(tree.addNode(n));
    REQUIRE(!tree.addNode(n));
}

TEST_CASE("TreeViewV1 parent-child linking", "[Editor][S142]") {
    TreeViewV1 tree;
    TreeNode root; root.id = 1; root.label = "Root";
    TreeNode child; child.id = 2; child.label = "Child"; child.parentId = 1;
    tree.addNode(root);
    tree.addNode(child);

    const auto* rootNode = tree.findNode(1);
    REQUIRE(rootNode->childCount() == 1u);
    REQUIRE(rootNode->childIds[0] == 2u);
    REQUIRE(!tree.findNode(2)->isRoot());
}

TEST_CASE("TreeViewV1 select and deselect", "[Editor][S142]") {
    TreeViewV1 tree;
    TreeNode a; a.id = 1; a.label = "A";
    TreeNode b; b.id = 2; b.label = "B";
    tree.addNode(a); tree.addNode(b);

    REQUIRE(tree.select(1));
    REQUIRE(tree.selectedCount() == 1u);
    REQUIRE(tree.findNode(1)->isSelected());

    tree.select(2, true); // exclusive
    REQUIRE(tree.selectedCount() == 1u);
    REQUIRE(!tree.findNode(1)->isSelected());

    tree.deselect(2);
    REQUIRE(tree.selectedCount() == 0u);
}

TEST_CASE("TreeViewV1 multi-select (non-exclusive)", "[Editor][S142]") {
    TreeViewV1 tree;
    TreeNode a; a.id = 1; a.label = "A";
    TreeNode b; b.id = 2; b.label = "B";
    tree.addNode(a); tree.addNode(b);
    tree.select(1, false);
    tree.select(2, false);
    REQUIRE(tree.selectedCount() == 2u);
}

TEST_CASE("TreeViewV1 clearSelection", "[Editor][S142]") {
    TreeViewV1 tree;
    TreeNode n; n.id = 1; n.label = "N";
    tree.addNode(n);
    tree.select(1);
    tree.clearSelection();
    REQUIRE(tree.selectedCount() == 0u);
}

TEST_CASE("TreeViewV1 expand and collapse", "[Editor][S142]") {
    TreeViewV1 tree;
    TreeNode p; p.id = 1; p.label = "P";
    TreeNode c; c.id = 2; c.label = "C"; c.parentId = 1;
    tree.addNode(p); tree.addNode(c);

    REQUIRE(tree.expand(1, true));
    REQUIRE(tree.findNode(1)->isExpanded());
    REQUIRE(tree.expand(1, false));
    REQUIRE(!tree.findNode(1)->isExpanded());
}

TEST_CASE("TreeViewV1 leaf node cannot expand", "[Editor][S142]") {
    TreeViewV1 tree;
    TreeNode n; n.id = 1; n.label = "Leaf";
    n.setFlag(TreeNodeFlag::Leaf);
    tree.addNode(n);
    REQUIRE(!tree.expand(1, true));
    REQUIRE(!tree.findNode(1)->isExpanded());
}

TEST_CASE("TreeViewV1 expandAll and collapseAll", "[Editor][S142]") {
    TreeViewV1 tree;
    TreeNode a; a.id = 1; a.label = "A";
    TreeNode b; b.id = 2; b.label = "B"; b.parentId = 1;
    tree.addNode(a); tree.addNode(b);
    tree.expandAll();
    REQUIRE(tree.findNode(1)->isExpanded());
    tree.collapseAll();
    REQUIRE(!tree.findNode(1)->isExpanded());
}

TEST_CASE("TreeViewV1 rename", "[Editor][S142]") {
    TreeViewV1 tree;
    TreeNode n; n.id = 1; n.label = "Old";
    tree.addNode(n);
    REQUIRE(tree.rename(1, "New"));
    REQUIRE(tree.findNode(1)->label == "New");
    REQUIRE(!tree.rename(1, ""));  // empty label rejected
}

TEST_CASE("TreeViewV1 filterByLabel", "[Editor][S142]") {
    TreeViewV1 tree;
    TreeNode a; a.id = 1; a.label = "Alpha";
    TreeNode b; b.id = 2; b.label = "Beta";
    TreeNode c; c.id = 3; c.label = "Alphax";
    tree.addNode(a); tree.addNode(b); tree.addNode(c);
    auto result = tree.filterByLabel("Alpha");
    REQUIRE(result.size() == 2u);
}

TEST_CASE("TreeViewV1 rootIds", "[Editor][S142]") {
    TreeViewV1 tree;
    TreeNode a; a.id = 1; a.label = "A";
    TreeNode b; b.id = 2; b.label = "B";
    TreeNode c; c.id = 3; c.label = "C"; c.parentId = 1;
    tree.addNode(a); tree.addNode(b); tree.addNode(c);
    REQUIRE(tree.rootIds().size() == 2u);
}

TEST_CASE("TreeViewV1 removeNode disconnects from parent", "[Editor][S142]") {
    TreeViewV1 tree;
    TreeNode p; p.id = 1; p.label = "P";
    TreeNode c; c.id = 2; c.label = "C"; c.parentId = 1;
    tree.addNode(p); tree.addNode(c);
    REQUIRE(tree.removeNode(2));
    REQUIRE(tree.nodeCount() == 1u);
    REQUIRE(tree.findNode(1)->childCount() == 0u);
}

TEST_CASE("TreeViewV1 select callback", "[Editor][S142]") {
    TreeViewV1 tree;
    TreeNode n; n.id = 1; n.label = "N";
    tree.addNode(n);
    uint32_t called = 0;
    tree.setOnSelect([&](uint32_t id) { called = id; });
    tree.select(1);
    REQUIRE(called == 1u);
}

// ── TableViewV1 ───────────────────────────────────────────────────────────────

TEST_CASE("SortDirection names", "[Editor][S142]") {
    REQUIRE(std::string(sortDirectionName(SortDirection::None))       == "None");
    REQUIRE(std::string(sortDirectionName(SortDirection::Ascending))  == "Ascending");
    REQUIRE(std::string(sortDirectionName(SortDirection::Descending)) == "Descending");
}

TEST_CASE("tableCellToString conversions", "[Editor][S142]") {
    REQUIRE(tableCellToString(TableCellValue{true})         == "true");
    REQUIRE(tableCellToString(TableCellValue{false})        == "false");
    REQUIRE(tableCellToString(TableCellValue{42})           == "42");
    REQUIRE(tableCellToString(TableCellValue{std::string("hello")}) == "hello");
}

TEST_CASE("TableColumnDesc validity", "[Editor][S142]") {
    TableColumnDesc col;
    REQUIRE(!col.isValid());
    col.id = 1; col.header = "Name";
    REQUIRE(col.isValid());
}

TEST_CASE("TableViewV1 addColumn and addRow", "[Editor][S142]") {
    TableViewV1 table;
    TableColumnDesc c1; c1.id = 1; c1.header = "Name";
    TableColumnDesc c2; c2.id = 2; c2.header = "Value";
    REQUIRE(table.addColumn(c1));
    REQUIRE(table.addColumn(c2));
    REQUIRE(table.columnCount() == 2u);

    TableRow row; row.id = 1;
    row.cells = { TableCellValue{std::string("item")}, TableCellValue{100} };
    REQUIRE(table.addRow(row));
    REQUIRE(table.rowCount() == 1u);
}

TEST_CASE("TableViewV1 reject duplicate column/row id", "[Editor][S142]") {
    TableViewV1 table;
    TableColumnDesc c; c.id = 1; c.header = "H";
    REQUIRE(table.addColumn(c));
    REQUIRE(!table.addColumn(c));

    TableRow r; r.id = 1;
    REQUIRE(table.addRow(r));
    REQUIRE(!table.addRow(r));
}

TEST_CASE("TableViewV1 getCellValue and setCellValue", "[Editor][S142]") {
    TableViewV1 table;
    TableColumnDesc c; c.id = 1; c.header = "V";
    table.addColumn(c);
    TableRow r; r.id = 1; r.cells = { TableCellValue{10} };
    table.addRow(r);

    REQUIRE(table.setCellValue(1, 1, TableCellValue{99}));
    const auto* val = table.getCellValue(1, 1);
    REQUIRE(val != nullptr);
    REQUIRE(std::get<int>(*val) == 99);
}

TEST_CASE("TableViewV1 selectRow exclusive", "[Editor][S142]") {
    TableViewV1 table;
    TableRow a; a.id = 1;
    TableRow b; b.id = 2;
    table.addRow(a); table.addRow(b);
    table.selectRow(1);
    REQUIRE(table.selectedCount() == 1u);
    table.selectRow(2, true); // exclusive
    REQUIRE(table.selectedCount() == 1u);
    REQUIRE(table.findRow(1)->selected == false);
}

TEST_CASE("TableViewV1 selectRow non-exclusive", "[Editor][S142]") {
    TableViewV1 table;
    TableRow a; a.id = 1;
    TableRow b; b.id = 2;
    table.addRow(a); table.addRow(b);
    table.selectRow(1, false);
    table.selectRow(2, false);
    REQUIRE(table.selectedCount() == 2u);
}

TEST_CASE("TableViewV1 clearSelection", "[Editor][S142]") {
    TableViewV1 table;
    TableRow r; r.id = 1;
    table.addRow(r);
    table.selectRow(1);
    table.clearSelection();
    REQUIRE(table.selectedCount() == 0u);
}

TEST_CASE("TableViewV1 sort ascending", "[Editor][S142]") {
    TableViewV1 table;
    TableColumnDesc c; c.id = 1; c.header = "Name";
    table.addColumn(c);

    auto addRow = [&](uint32_t id, const std::string& name) {
        TableRow r; r.id = id;
        r.cells = { TableCellValue{name} };
        table.addRow(r);
    };
    addRow(1, "Charlie");
    addRow(2, "Alice");
    addRow(3, "Bob");

    table.sort(1, SortDirection::Ascending);
    REQUIRE(table.sortDir()  == SortDirection::Ascending);
    REQUIRE(table.sortColId() == 1u);
    // First row should be "Alice"
    REQUIRE(std::get<std::string>(*table.rows()[0].cell(0)) == "Alice");
}

TEST_CASE("TableViewV1 sort descending", "[Editor][S142]") {
    TableViewV1 table;
    TableColumnDesc c; c.id = 1; c.header = "Name";
    table.addColumn(c);
    auto addRow = [&](uint32_t id, const std::string& name) {
        TableRow r; r.id = id; r.cells = { TableCellValue{name} }; table.addRow(r);
    };
    addRow(1, "Alice"); addRow(2, "Charlie"); addRow(3, "Bob");
    table.sort(1, SortDirection::Descending);
    REQUIRE(std::get<std::string>(*table.rows()[0].cell(0)) == "Charlie");
}

TEST_CASE("TableViewV1 removeRow and removeColumn", "[Editor][S142]") {
    TableViewV1 table;
    TableColumnDesc c; c.id = 1; c.header = "C";
    table.addColumn(c);
    TableRow r; r.id = 1; r.cells = {TableCellValue{1}};
    table.addRow(r);

    REQUIRE(table.removeRow(1));
    REQUIRE(table.rowCount() == 0u);

    table.addRow(r);
    REQUIRE(table.removeColumn(1));
    REQUIRE(table.columnCount() == 0u);
    REQUIRE(table.rows()[0].cells.empty());
}

TEST_CASE("TableViewV1 setColumnVisible and setColumnWidth", "[Editor][S142]") {
    TableViewV1 table;
    TableColumnDesc c; c.id = 1; c.header = "C"; c.widthPx = 100.f;
    table.addColumn(c);
    REQUIRE(table.setColumnVisible(1, false));
    REQUIRE(!table.columns()[0].visible);
    REQUIRE(table.setColumnWidth(1, 200.f));
    REQUIRE(table.columns()[0].widthPx == Approx(200.f));
    // Minimum enforcement
    REQUIRE(table.setColumnWidth(1, 5.f));
    REQUIRE(table.columns()[0].widthPx == Approx(10.f));
}

TEST_CASE("TableViewV1 onSelect callback", "[Editor][S142]") {
    TableViewV1 table;
    TableRow r; r.id = 1;
    table.addRow(r);
    uint32_t called = 0;
    table.setOnSelect([&](uint32_t id) { called = id; });
    table.selectRow(1);
    REQUIRE(called == 1u);
}
