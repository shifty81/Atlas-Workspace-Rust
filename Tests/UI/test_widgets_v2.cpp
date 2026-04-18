#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include "NF/UI/AtlasUI/DrawPrimitives.h"
#include "NF/UI/AtlasUI/Interfaces.h"
#include "NF/UI/AtlasUI/WidgetBase.h"
#include "NF/UI/AtlasUI/WidgetHelpers.h"

#include "NF/UI/AtlasUI/Widgets/ContextMenu.h"
#include "NF/UI/AtlasUI/Widgets/TreeView.h"
#include "NF/UI/AtlasUI/Widgets/ScrollView.h"
#include "NF/UI/AtlasUI/Widgets/PropertyGrid.h"
#include "NF/UI/AtlasUI/Widgets/TableView.h"
#include "NF/UI/AtlasUI/Widgets/WidgetKit.h"
#include "NF/UI/AtlasUI/Widgets/Label.h"

using namespace NF::UI::AtlasUI;

// ── Test mock contexts ───────────────────────────────────────────

struct TestLayoutCtx : ILayoutContext {
    float dpiScale() const override { return 1.0f; }
    NF::Vec2 availableSize() const override { return {800.f, 600.f}; }
    NF::Vec2 measureText(std::string_view text, float) const override {
        return {static_cast<float>(text.size()) * 8.f, 16.f};
    }
    void invalidateLayout() override {}
};

struct TestPaintCtx : IPaintContext {
    DrawList dl;
    void drawRect(const NF::Rect& r, Color c) override { dl.push(DrawRectCmd{r, c}); }
    void fillRect(const NF::Rect& r, Color c) override { dl.push(FillRectCmd{r, c}); }
    void drawText(const NF::Rect& r, std::string_view t, FontId f, Color c) override {
        dl.push(DrawTextCmd{r, std::string(t), f, c});
    }
    void drawImage(const NF::Rect& r, uint32_t textureId, Color tint) override {
        dl.push(DrawImageCmd{r, textureId, tint});
    }
    void pushClip(const NF::Rect&) override {}
    void popClip() override {}
    DrawList& drawList() override { return dl; }
};

struct TestInputCtx : IInputContext {
    NF::Vec2 pos{};
    bool primary = false;
    bool secondary = false;
    std::string typed;
    NF::Vec2 mousePosition() const override { return pos; }
    bool primaryDown() const override { return primary; }
    bool secondaryDown() const override { return secondary; }
    bool keyDown(int) const override { return false; }
    std::string_view typedText() const override { return typed; }
    void requestFocus(IWidget*) override {}
    void capturePointer(IWidget*) override {}
    void releasePointer(IWidget*) override {}
};

// ════════════════════════════════════════════════════════════════
// ContextMenu Tests
// ════════════════════════════════════════════════════════════════

TEST_CASE("ContextMenu default state", "[AtlasUI][ContextMenu]") {
    ContextMenu menu;
    REQUIRE_FALSE(menu.isOpen());
    REQUIRE(menu.itemCount() == 0);
    REQUIRE(menu.hoveredIndex() == -1);
}

TEST_CASE("ContextMenu construction with items", "[AtlasUI][ContextMenu]") {
    std::vector<MenuItem> items;
    items.push_back({"Cut", "Ctrl+X", []{}, true, false, {}});
    items.push_back({"Copy", "Ctrl+C", []{}, true, false, {}});
    ContextMenu menu(std::move(items));
    REQUIRE(menu.itemCount() == 2);
    REQUIRE(menu.items()[0].label == "Cut");
}

TEST_CASE("ContextMenu open/close", "[AtlasUI][ContextMenu]") {
    ContextMenu menu;
    menu.addItem({"Paste", "", []{}, true, false, {}});
    menu.open({100.f, 200.f});
    REQUIRE(menu.isOpen());
    menu.close();
    REQUIRE_FALSE(menu.isOpen());
}

TEST_CASE("ContextMenu addItem and clear", "[AtlasUI][ContextMenu]") {
    ContextMenu menu;
    menu.addItem({"A", "", nullptr, true, false, {}});
    menu.addItem({"B", "", nullptr, true, false, {}});
    REQUIRE(menu.itemCount() == 2);
    menu.clear();
    REQUIRE(menu.itemCount() == 0);
}

TEST_CASE("ContextMenu separator", "[AtlasUI][ContextMenu]") {
    ContextMenu menu;
    menu.addItem({"Cut", "", nullptr, true, false, {}});
    menu.addItem(MenuItem::Separator());
    menu.addItem({"Paste", "", nullptr, true, false, {}});
    REQUIRE(menu.itemCount() == 3);
    REQUIRE(menu.items()[1].isSeparator());
}

TEST_CASE("ContextMenu measure calculates width", "[AtlasUI][ContextMenu]") {
    ContextMenu menu;
    menu.addItem({"Short", "", nullptr, true, false, {}});
    menu.addItem({"A longer menu item text", "", nullptr, true, false, {}});
    menu.open({10.f, 20.f});

    TestLayoutCtx layout;
    menu.measure(layout);

    auto b = menu.bounds();
    REQUIRE(b.w >= 160.f); // at least minWidth
    REQUIRE(b.h > 0.f);
}

TEST_CASE("ContextMenu paint emits draw commands when open", "[AtlasUI][ContextMenu]") {
    ContextMenu menu;
    menu.addItem({"Action", "", nullptr, true, false, {}});
    menu.open({10.f, 20.f});

    TestLayoutCtx layout;
    menu.measure(layout);

    TestPaintCtx paint;
    menu.paint(paint);
    REQUIRE_FALSE(paint.dl.empty());
}

TEST_CASE("ContextMenu paint does nothing when closed", "[AtlasUI][ContextMenu]") {
    ContextMenu menu;
    menu.addItem({"Action", "", nullptr, true, false, {}});

    TestPaintCtx paint;
    menu.paint(paint);
    REQUIRE(paint.dl.empty());
}

TEST_CASE("ContextMenu handleInput invokes action on click", "[AtlasUI][ContextMenu]") {
    bool triggered = false;
    ContextMenu menu;
    menu.addItem({"Action", "", [&]{ triggered = true; }, true, false, {}});
    menu.open({0.f, 0.f});

    TestLayoutCtx layout;
    menu.measure(layout);

    TestInputCtx input;
    input.pos = {80.f, 12.f}; // inside first item
    input.primary = true;
    menu.handleInput(input);
    REQUIRE(triggered);
    REQUIRE_FALSE(menu.isOpen()); // closes after action
}

TEST_CASE("ContextMenu click outside closes", "[AtlasUI][ContextMenu]") {
    ContextMenu menu;
    menu.addItem({"Action", "", []{}, true, false, {}});
    menu.open({0.f, 0.f});

    TestLayoutCtx layout;
    menu.measure(layout);

    TestInputCtx input;
    input.pos = {500.f, 500.f}; // outside
    input.primary = true;
    menu.handleInput(input);
    REQUIRE_FALSE(menu.isOpen());
}

TEST_CASE("ContextMenu disabled item doesn't trigger", "[AtlasUI][ContextMenu]") {
    bool triggered = false;
    ContextMenu menu;
    menu.addItem({"Disabled", "", [&]{ triggered = true; }, false, false, {}});
    menu.open({0.f, 0.f});

    TestLayoutCtx layout;
    menu.measure(layout);

    TestInputCtx input;
    input.pos = {80.f, 12.f};
    input.primary = true;
    menu.handleInput(input);
    REQUIRE_FALSE(triggered);
}

TEST_CASE("ContextMenu setItems replaces items", "[AtlasUI][ContextMenu]") {
    ContextMenu menu;
    menu.addItem({"Old", "", nullptr, true, false, {}});
    REQUIRE(menu.itemCount() == 1);
    menu.setItems({{"New1", "", nullptr, true, false, {}}, {"New2", "", nullptr, true, false, {}}});
    REQUIRE(menu.itemCount() == 2);
    REQUIRE(menu.items()[0].label == "New1");
}

TEST_CASE("ContextMenu shortcut hint renders", "[AtlasUI][ContextMenu]") {
    ContextMenu menu;
    menu.addItem({"Copy", "Ctrl+C", []{}, true, false, {}});
    menu.open({0.f, 0.f});

    TestLayoutCtx layout;
    menu.measure(layout);

    TestPaintCtx paint;
    menu.paint(paint);
    // Should have text commands including shortcut hint
    REQUIRE(paint.dl.size() >= 3);
}

TEST_CASE("MenuItem hasChildren", "[AtlasUI][ContextMenu]") {
    MenuItem item{"Parent", "", nullptr, true, false, {}};
    REQUIRE_FALSE(item.hasChildren());
    item.children.push_back({"Child", "", nullptr, true, false, {}});
    REQUIRE(item.hasChildren());
}

// ════════════════════════════════════════════════════════════════
// TreeView Tests
// ════════════════════════════════════════════════════════════════

TEST_CASE("TreeView default state", "[AtlasUI][TreeView]") {
    TreeView tree;
    REQUIRE(tree.rootCount() == 0);
    REQUIRE(tree.selectedId().empty());
    REQUIRE(tree.visibleNodeCount() == 0);
}

TEST_CASE("TreeView addRoot and setRoots", "[AtlasUI][TreeView]") {
    TreeView tree;
    tree.addRoot({"n1", "Node 1"});
    tree.addRoot({"n2", "Node 2"});
    REQUIRE(tree.rootCount() == 2);

    tree.setRoots({{"n3", "Node 3"}});
    REQUIRE(tree.rootCount() == 1);
}

TEST_CASE("TreeView clear", "[AtlasUI][TreeView]") {
    TreeView tree;
    tree.addRoot({"n1", "Node 1"});
    tree.clear();
    REQUIRE(tree.rootCount() == 0);
    REQUIRE(tree.selectedId().empty());
}

TEST_CASE("TreeView selectNode", "[AtlasUI][TreeView]") {
    TreeView tree;
    tree.addRoot({"n1", "Node 1"});
    tree.addRoot({"n2", "Node 2"});

    std::string selectedId;
    tree.setOnSelect([&](const std::string& id) { selectedId = id; });

    REQUIRE(tree.selectNode("n2"));
    REQUIRE(tree.selectedId() == "n2");
    REQUIRE(selectedId == "n2");
}

TEST_CASE("TreeView selectNode returns false for invalid id", "[AtlasUI][TreeView]") {
    TreeView tree;
    tree.addRoot({"n1", "Node 1"});
    REQUIRE_FALSE(tree.selectNode("nonexistent"));
}

TEST_CASE("TreeView toggleNode expand/collapse", "[AtlasUI][TreeView]") {
    TreeNode root;
    root.id = "root";
    root.label = "Root";
    root.expanded = false;
    root.children.push_back({"child1", "Child 1"});
    root.children.push_back({"child2", "Child 2"});

    TreeView tree;
    tree.addRoot(std::move(root));

    REQUIRE(tree.visibleNodeCount() == 1); // collapsed, only root

    std::string expandedId;
    bool wasExpanded = false;
    tree.setOnExpand([&](const std::string& id, bool exp) { expandedId = id; wasExpanded = exp; });

    REQUIRE(tree.toggleNode("root"));
    REQUIRE(tree.visibleNodeCount() == 3); // root + 2 children
    REQUIRE(expandedId == "root");
    REQUIRE(wasExpanded);
}

TEST_CASE("TreeView expandAll and collapseAll", "[AtlasUI][TreeView]") {
    TreeNode root;
    root.id = "root";
    root.label = "Root";
    root.expanded = false;
    root.children.push_back({"child", "Child"});

    TreeView tree;
    tree.addRoot(std::move(root));

    REQUIRE(tree.visibleNodeCount() == 1);

    tree.expandAll();
    REQUIRE(tree.visibleNodeCount() == 2);

    tree.collapseAll();
    REQUIRE(tree.visibleNodeCount() == 1);
}

TEST_CASE("TreeView visibleNodeCount with nested hierarchy", "[AtlasUI][TreeView]") {
    TreeNode root;
    root.id = "root";
    root.label = "Root";
    root.expanded = true;

    TreeNode sub;
    sub.id = "sub";
    sub.label = "Sub";
    sub.expanded = true;
    sub.children.push_back({"leaf", "Leaf"});

    root.children.push_back(std::move(sub));

    TreeView tree;
    tree.addRoot(std::move(root));

    REQUIRE(tree.visibleNodeCount() == 3); // root, sub, leaf
}

TEST_CASE("TreeView leaf isLeaf", "[AtlasUI][TreeView]") {
    TreeNode leaf;
    leaf.id = "leaf";
    leaf.label = "Leaf";
    REQUIRE(leaf.isLeaf());
    REQUIRE(leaf.childCount() == 0);

    leaf.children.push_back({"child", "Child"});
    REQUIRE_FALSE(leaf.isLeaf());
    REQUIRE(leaf.childCount() == 1);
}

TEST_CASE("TreeView measure sets bounds", "[AtlasUI][TreeView]") {
    TreeView tree;
    tree.addRoot({"n1", "A"});
    tree.addRoot({"n2", "B"});

    TestLayoutCtx layout;
    tree.measure(layout);
    REQUIRE(tree.bounds().h > 0.f);
}

TEST_CASE("TreeView paint produces draw commands", "[AtlasUI][TreeView]") {
    TreeView tree;
    tree.addRoot({"n1", "Node"});

    TestLayoutCtx layout;
    tree.measure(layout);
    tree.arrange({0.f, 0.f, 300.f, 200.f});

    TestPaintCtx paint;
    tree.paint(paint);
    REQUIRE_FALSE(paint.dl.empty());
}

TEST_CASE("TreeView handleInput selects node on click", "[AtlasUI][TreeView]") {
    TreeView tree;
    tree.addRoot({"n1", "Node 1"});
    tree.addRoot({"n2", "Node 2"});

    TestLayoutCtx layout;
    tree.measure(layout);
    tree.arrange({0.f, 0.f, 300.f, 200.f});

    TestInputCtx input;
    input.pos = {100.f, 12.f}; // first row
    input.primary = true;
    tree.handleInput(input);
    REQUIRE(tree.selectedId() == "n1");
}

TEST_CASE("TreeView hidden doesn't handle input", "[AtlasUI][TreeView]") {
    TreeView tree;
    tree.addRoot({"n1", "Node"});
    tree.setVisible(false);
    tree.arrange({0.f, 0.f, 300.f, 200.f});

    TestInputCtx input;
    input.pos = {100.f, 12.f};
    input.primary = true;
    REQUIRE_FALSE(tree.handleInput(input));
}

// ════════════════════════════════════════════════════════════════
// ScrollView Tests
// ════════════════════════════════════════════════════════════════

TEST_CASE("ScrollView default state", "[AtlasUI][ScrollView]") {
    ScrollView scroll;
    REQUIRE(scroll.scrollOffset() == 0.f);
    REQUIRE(scroll.contentHeight() == 0.f);
    REQUIRE(scroll.content() == nullptr);
}

TEST_CASE("ScrollView setContent", "[AtlasUI][ScrollView]") {
    ScrollView scroll;
    auto label = std::make_shared<Label>("Hello");
    scroll.setContent(label);
    REQUIRE(scroll.content() != nullptr);
}

TEST_CASE("ScrollView scrollBy", "[AtlasUI][ScrollView]") {
    ScrollView scroll;
    scroll.arrange({0.f, 0.f, 200.f, 100.f});
    scroll.setContentHeight(500.f);

    scroll.scrollBy(50.f);
    REQUIRE(scroll.scrollOffset() == 50.f);

    scroll.scrollBy(50.f);
    REQUIRE(scroll.scrollOffset() == 100.f);
}

TEST_CASE("ScrollView clamps scroll offset", "[AtlasUI][ScrollView]") {
    ScrollView scroll;
    scroll.arrange({0.f, 0.f, 200.f, 100.f});
    scroll.setContentHeight(300.f);

    scroll.setScrollOffset(-50.f);
    REQUIRE(scroll.scrollOffset() == 0.f);

    scroll.setScrollOffset(500.f);
    REQUIRE(scroll.scrollOffset() == 200.f); // max = contentHeight - viewportHeight
}

TEST_CASE("ScrollView scrollToTop and scrollToBottom", "[AtlasUI][ScrollView]") {
    ScrollView scroll;
    scroll.arrange({0.f, 0.f, 200.f, 100.f});
    scroll.setContentHeight(500.f);

    scroll.scrollToBottom();
    REQUIRE(scroll.scrollOffset() == 400.f);

    scroll.scrollToTop();
    REQUIRE(scroll.scrollOffset() == 0.f);
}

TEST_CASE("ScrollView canScroll", "[AtlasUI][ScrollView]") {
    ScrollView scroll;
    scroll.arrange({0.f, 0.f, 200.f, 100.f});

    scroll.setContentHeight(50.f);
    REQUIRE_FALSE(scroll.canScroll());

    scroll.setContentHeight(200.f);
    REQUIRE(scroll.canScroll());
}

TEST_CASE("ScrollView scrollFraction", "[AtlasUI][ScrollView]") {
    ScrollView scroll;
    scroll.arrange({0.f, 0.f, 200.f, 100.f});
    scroll.setContentHeight(300.f);

    scroll.setScrollOffset(0.f);
    REQUIRE(scroll.scrollFraction() == 0.f);

    scroll.setScrollOffset(200.f);
    REQUIRE(scroll.scrollFraction() == 1.f);

    scroll.setScrollOffset(100.f);
    REQUIRE(scroll.scrollFraction() == Catch::Approx(0.5f));
}

TEST_CASE("ScrollView thumbHeight", "[AtlasUI][ScrollView]") {
    ScrollView scroll;
    scroll.arrange({0.f, 0.f, 200.f, 100.f});
    scroll.setContentHeight(400.f);

    float th = scroll.thumbHeight();
    REQUIRE(th >= 20.f);
    REQUIRE(th <= 100.f);
}

TEST_CASE("ScrollView maxScrollOffset", "[AtlasUI][ScrollView]") {
    ScrollView scroll;
    scroll.arrange({0.f, 0.f, 200.f, 100.f});
    scroll.setContentHeight(300.f);
    REQUIRE(scroll.maxScrollOffset() == 200.f);

    scroll.setContentHeight(50.f);
    REQUIRE(scroll.maxScrollOffset() == 0.f);
}

TEST_CASE("ScrollView viewportHeight", "[AtlasUI][ScrollView]") {
    ScrollView scroll;
    scroll.arrange({0.f, 0.f, 200.f, 150.f});
    REQUIRE(scroll.viewportHeight() == 150.f);
}

TEST_CASE("ScrollView paint with scrollbar", "[AtlasUI][ScrollView]") {
    ScrollView scroll;
    auto label = std::make_shared<Label>("Content");
    scroll.setContent(label);
    scroll.setContentHeight(500.f);
    scroll.arrange({0.f, 0.f, 200.f, 100.f});

    TestPaintCtx paint;
    scroll.paint(paint);
    REQUIRE_FALSE(paint.dl.empty());
}

TEST_CASE("ScrollView paint without scrollbar", "[AtlasUI][ScrollView]") {
    ScrollView scroll;
    scroll.setContentHeight(50.f);
    scroll.arrange({0.f, 0.f, 200.f, 100.f});

    TestPaintCtx paint;
    scroll.paint(paint);
    // Only clip commands when no content
    REQUIRE(paint.dl.size() <= 2);
}

TEST_CASE("ScrollView hidden doesn't paint", "[AtlasUI][ScrollView]") {
    ScrollView scroll;
    scroll.setVisible(false);

    TestPaintCtx paint;
    scroll.paint(paint);
    REQUIRE(paint.dl.empty());
}

TEST_CASE("ScrollView scrollBarWidth", "[AtlasUI][ScrollView]") {
    ScrollView scroll;
    REQUIRE(scroll.scrollBarWidth() == 10.f);
}

// ════════════════════════════════════════════════════════════════
// PropertyGrid Tests
// ════════════════════════════════════════════════════════════════

TEST_CASE("PropertyGrid default state", "[AtlasUI][PropertyGrid]") {
    PropertyGrid grid;
    REQUIRE(grid.itemCount() == 0);
    REQUIRE(grid.visibleRowCount() == 0);
    REQUIRE(grid.labelWidth() == 140.f);
}

TEST_CASE("PropertyGrid addItem and setItems", "[AtlasUI][PropertyGrid]") {
    PropertyGrid grid;
    grid.addItem({"Name", PropertyValue("Test"), "", false, true, {}});
    REQUIRE(grid.itemCount() == 1);

    grid.setItems({
        {"X", PropertyValue(1.0f), "", false, true, {}},
        {"Y", PropertyValue(2.0f), "", false, true, {}},
    });
    REQUIRE(grid.itemCount() == 2);
}

TEST_CASE("PropertyGrid clear", "[AtlasUI][PropertyGrid]") {
    PropertyGrid grid;
    grid.addItem({"X", PropertyValue(1.0f), "", false, true, {}});
    grid.clear();
    REQUIRE(grid.itemCount() == 0);
    REQUIRE(grid.visibleRowCount() == 0);
}

TEST_CASE("PropertyGrid setLabelWidth", "[AtlasUI][PropertyGrid]") {
    PropertyGrid grid;
    grid.setLabelWidth(200.f);
    REQUIRE(grid.labelWidth() == 200.f);
}

TEST_CASE("PropertyValue asString for string", "[AtlasUI][PropertyGrid]") {
    PropertyValue v("hello");
    REQUIRE(v.asString() == "hello");
}

TEST_CASE("PropertyValue asString for int", "[AtlasUI][PropertyGrid]") {
    PropertyValue v(42);
    REQUIRE(v.asString() == "42");
}

TEST_CASE("PropertyValue asString for float", "[AtlasUI][PropertyGrid]") {
    PropertyValue v(3.14f);
    REQUIRE_FALSE(v.asString().empty());
}

TEST_CASE("PropertyValue asString for bool", "[AtlasUI][PropertyGrid]") {
    REQUIRE(PropertyValue(true).asString() == "true");
    REQUIRE(PropertyValue(false).asString() == "false");
}

TEST_CASE("PropertyItem isGroup", "[AtlasUI][PropertyGrid]") {
    PropertyItem leaf{"Leaf", PropertyValue(1), "", false, true, {}};
    REQUIRE_FALSE(leaf.isGroup());

    PropertyItem group{"Group", PropertyValue(""), "", false, true, {leaf}};
    REQUIRE(group.isGroup());
    REQUIRE(group.childCount() == 1);
}

TEST_CASE("PropertyGrid with groups", "[AtlasUI][PropertyGrid]") {
    PropertyGrid grid;

    PropertyItem group;
    group.name = "Transform";
    group.expanded = true;
    group.children.push_back({"X", PropertyValue(1.0f), "", false, true, {}});
    group.children.push_back({"Y", PropertyValue(2.0f), "", false, true, {}});

    grid.addItem(std::move(group));
    REQUIRE(grid.visibleRowCount() == 3); // header + 2 children
}

TEST_CASE("PropertyGrid collapsed group hides children", "[AtlasUI][PropertyGrid]") {
    PropertyGrid grid;

    PropertyItem group;
    group.name = "Transform";
    group.expanded = false;
    group.children.push_back({"X", PropertyValue(1.0f), "", false, true, {}});
    group.children.push_back({"Y", PropertyValue(2.0f), "", false, true, {}});

    grid.addItem(std::move(group));
    REQUIRE(grid.visibleRowCount() == 1); // only header
}

TEST_CASE("PropertyGrid toggleGroup", "[AtlasUI][PropertyGrid]") {
    PropertyGrid grid;

    PropertyItem group;
    group.name = "Transform";
    group.expanded = true;
    group.children.push_back({"X", PropertyValue(1.0f), "", false, true, {}});

    grid.addItem(std::move(group));
    REQUIRE(grid.visibleRowCount() == 2);

    REQUIRE(grid.toggleGroup("Transform"));
    REQUIRE(grid.visibleRowCount() == 1);

    REQUIRE(grid.toggleGroup("Transform"));
    REQUIRE(grid.visibleRowCount() == 2);
}

TEST_CASE("PropertyGrid toggleGroup returns false for invalid name", "[AtlasUI][PropertyGrid]") {
    PropertyGrid grid;
    REQUIRE_FALSE(grid.toggleGroup("nonexistent"));
}

TEST_CASE("PropertyGrid measure sets height", "[AtlasUI][PropertyGrid]") {
    PropertyGrid grid;
    grid.addItem({"A", PropertyValue("1"), "", false, true, {}});
    grid.addItem({"B", PropertyValue("2"), "", false, true, {}});

    TestLayoutCtx layout;
    grid.measure(layout);
    REQUIRE(grid.bounds().h > 0.f);
}

TEST_CASE("PropertyGrid paint produces draw commands", "[AtlasUI][PropertyGrid]") {
    PropertyGrid grid;
    grid.addItem({"Name", PropertyValue("Test"), "", false, true, {}});

    TestLayoutCtx layout;
    grid.measure(layout);
    grid.arrange({0.f, 0.f, 400.f, 200.f});

    TestPaintCtx paint;
    grid.paint(paint);
    REQUIRE_FALSE(paint.dl.empty());
}

TEST_CASE("PropertyGrid handleInput hover detection", "[AtlasUI][PropertyGrid]") {
    PropertyGrid grid;
    grid.addItem({"Name", PropertyValue("Value"), "", false, true, {}});

    TestLayoutCtx layout;
    grid.measure(layout);
    grid.arrange({0.f, 0.f, 400.f, 200.f});

    TestInputCtx input;
    input.pos = {100.f, 12.f}; // inside first row
    REQUIRE(grid.handleInput(input));
}

TEST_CASE("PropertyGrid handleInput outside returns false", "[AtlasUI][PropertyGrid]") {
    PropertyGrid grid;
    grid.addItem({"Name", PropertyValue("Value"), "", false, true, {}});
    grid.arrange({0.f, 0.f, 400.f, 200.f});

    TestInputCtx input;
    input.pos = {500.f, 500.f};
    REQUIRE_FALSE(grid.handleInput(input));
}

TEST_CASE("PropertyGrid hidden doesn't paint", "[AtlasUI][PropertyGrid]") {
    PropertyGrid grid;
    grid.addItem({"A", PropertyValue("B"), "", false, true, {}});
    grid.setVisible(false);

    TestPaintCtx paint;
    grid.paint(paint);
    REQUIRE(paint.dl.empty());
}

TEST_CASE("PropertyGrid readOnly items render differently", "[AtlasUI][PropertyGrid]") {
    PropertyGrid grid;
    grid.addItem({"ReadOnly", PropertyValue("locked"), "", true, true, {}});
    grid.addItem({"Editable", PropertyValue("free"), "", false, true, {}});

    TestLayoutCtx layout;
    grid.measure(layout);
    grid.arrange({0.f, 0.f, 400.f, 200.f});

    TestPaintCtx paint;
    grid.paint(paint);
    REQUIRE(paint.dl.size() >= 4); // multiple draw commands for 2 rows
}

// ════════════════════════════════════════════════════════════════
// TableView Tests
// ════════════════════════════════════════════════════════════════

TEST_CASE("TableView default state", "[AtlasUI][TableView]") {
    TableView table;
    REQUIRE(table.columnCount() == 0);
    REQUIRE(table.rowCount() == 0);
    REQUIRE(table.selectedRow() == -1);
    REQUIRE(table.headerVisible());
    REQUIRE(table.sortDirection() == SortDirection::None);
}

TEST_CASE("TableView setColumns and addColumn", "[AtlasUI][TableView]") {
    TableView table;
    table.addColumn({"Name", 150.f, true, true});
    table.addColumn({"Value", 100.f, false, true});
    REQUIRE(table.columnCount() == 2);

    table.setColumns({{"ID", 50.f, false, false}});
    REQUIRE(table.columnCount() == 1);
}

TEST_CASE("TableView setRows and addRow", "[AtlasUI][TableView]") {
    TableView table;
    table.setColumns({{"Name", 100.f}, {"Value", 100.f}});
    table.addRow({{"Alice", "100"}});
    table.addRow({{"Bob", "200"}});
    REQUIRE(table.rowCount() == 2);

    table.setRows({{{"Charlie", "300"}}});
    REQUIRE(table.rowCount() == 1);
}

TEST_CASE("TableView clearRows", "[AtlasUI][TableView]") {
    TableView table;
    table.addRow({{"A", "1"}});
    table.clearRows();
    REQUIRE(table.rowCount() == 0);
    REQUIRE(table.selectedRow() == -1);
}

TEST_CASE("TableView clear", "[AtlasUI][TableView]") {
    TableView table;
    table.addColumn({"Name", 100.f});
    table.addRow({{"A"}});
    table.clear();
    REQUIRE(table.columnCount() == 0);
    REQUIRE(table.rowCount() == 0);
}

TEST_CASE("TableView selectRow", "[AtlasUI][TableView]") {
    TableView table;
    table.addRow({{"A"}});
    table.addRow({{"B"}});

    size_t selectedIdx = 999;
    table.setOnSelect([&](size_t idx) { selectedIdx = idx; });

    REQUIRE(table.selectRow(1));
    REQUIRE(table.selectedRow() == 1);
    REQUIRE(selectedIdx == 1);
    REQUIRE(table.rows()[1].selected);
}

TEST_CASE("TableView selectRow deselects previous", "[AtlasUI][TableView]") {
    TableView table;
    table.addRow({{"A"}});
    table.addRow({{"B"}});

    table.selectRow(0);
    REQUIRE(table.rows()[0].selected);

    table.selectRow(1);
    REQUIRE_FALSE(table.rows()[0].selected);
    REQUIRE(table.rows()[1].selected);
}

TEST_CASE("TableView selectRow invalid index", "[AtlasUI][TableView]") {
    TableView table;
    table.addRow({{"A"}});
    REQUIRE_FALSE(table.selectRow(-1));
    REQUIRE_FALSE(table.selectRow(5));
}

TEST_CASE("TableView sort", "[AtlasUI][TableView]") {
    TableView table;
    table.addColumn({"Name", 100.f, true});

    size_t sortedCol = 999;
    SortDirection sortedDir = SortDirection::None;
    table.setOnSort([&](size_t col, SortDirection dir) {
        sortedCol = col;
        sortedDir = dir;
    });

    table.sort(0, SortDirection::Ascending);
    REQUIRE(table.sortColumn() == 0);
    REQUIRE(table.sortDirection() == SortDirection::Ascending);
    REQUIRE(sortedCol == 0);
    REQUIRE(sortedDir == SortDirection::Ascending);
}

TEST_CASE("TableView setHeaderVisible", "[AtlasUI][TableView]") {
    TableView table;
    table.setHeaderVisible(false);
    REQUIRE_FALSE(table.headerVisible());
}

TEST_CASE("TableView setRowHeight", "[AtlasUI][TableView]") {
    TableView table;
    table.setRowHeight(32.f);
    table.addRow({{"A"}});

    TestLayoutCtx layout;
    table.measure(layout);
    // Height should account for new row height
    REQUIRE(table.bounds().h > 0.f);
}

TEST_CASE("TableView measure calculates height", "[AtlasUI][TableView]") {
    TableView table;
    table.addColumn({"Name", 100.f});
    table.addRow({{"A"}});
    table.addRow({{"B"}});

    TestLayoutCtx layout;
    table.measure(layout);
    REQUIRE(table.bounds().h > 0.f);
}

TEST_CASE("TableView paint with header and rows", "[AtlasUI][TableView]") {
    TableView table;
    table.addColumn({"Name", 100.f, true});
    table.addColumn({"Value", 100.f});
    table.addRow({{"Alice", "100"}});
    table.addRow({{"Bob", "200"}});
    table.arrange({0.f, 0.f, 400.f, 200.f});

    TestPaintCtx paint;
    table.paint(paint);
    REQUIRE_FALSE(paint.dl.empty());
}

TEST_CASE("TableView paint without header", "[AtlasUI][TableView]") {
    TableView table;
    table.setHeaderVisible(false);
    table.addColumn({"Name", 100.f});
    table.addRow({{"A"}});
    table.arrange({0.f, 0.f, 400.f, 200.f});

    TestPaintCtx paint;
    table.paint(paint);
    REQUIRE_FALSE(paint.dl.empty());
}

TEST_CASE("TableView handleInput selects row on click", "[AtlasUI][TableView]") {
    TableView table;
    table.addColumn({"Name", 100.f});
    table.addRow({{"A"}});
    table.addRow({{"B"}});
    table.arrange({0.f, 0.f, 400.f, 200.f});

    TestLayoutCtx layout;
    table.measure(layout);

    TestInputCtx input;
    input.pos = {50.f, 40.f}; // after header (28px), first row area
    input.primary = true;
    table.handleInput(input);
    REQUIRE(table.selectedRow() == 0);
}

TEST_CASE("TableView handleInput sort on header click", "[AtlasUI][TableView]") {
    TableView table;
    table.addColumn({"Name", 100.f, true}); // sortable
    table.addRow({{"A"}});
    table.arrange({0.f, 0.f, 400.f, 200.f});

    TestInputCtx input;
    input.pos = {50.f, 14.f}; // header area
    input.primary = true;
    table.handleInput(input);
    REQUIRE(table.sortDirection() == SortDirection::Ascending);

    // Click again toggles to descending
    table.handleInput(input);
    REQUIRE(table.sortDirection() == SortDirection::Descending);
}

TEST_CASE("TableView hidden doesn't paint", "[AtlasUI][TableView]") {
    TableView table;
    table.addColumn({"Name", 100.f});
    table.addRow({{"A"}});
    table.setVisible(false);

    TestPaintCtx paint;
    table.paint(paint);
    REQUIRE(paint.dl.empty());
}

TEST_CASE("TableView handleInput outside returns false", "[AtlasUI][TableView]") {
    TableView table;
    table.addColumn({"Name", 100.f});
    table.addRow({{"A"}});
    table.arrange({0.f, 0.f, 400.f, 200.f});

    TestInputCtx input;
    input.pos = {500.f, 500.f};
    REQUIRE_FALSE(table.handleInput(input));
}

TEST_CASE("TableView selected row paint uses selection color", "[AtlasUI][TableView]") {
    TableView table;
    table.addColumn({"Name", 100.f});
    table.addRow({{"A"}});
    table.selectRow(0);
    table.arrange({0.f, 0.f, 400.f, 200.f});

    TestPaintCtx paint;
    table.paint(paint);
    // Should include selection fill rect
    REQUIRE(paint.dl.size() >= 3);
}

// ════════════════════════════════════════════════════════════════
// WidgetKit umbrella includes
// ════════════════════════════════════════════════════════════════

TEST_CASE("WidgetKit includes all new widgets", "[AtlasUI][WidgetKit]") {
    // If this compiles, the umbrella header works
    ContextMenu menu;
    TreeView tree;
    ScrollView scroll;
    PropertyGrid grid;
    TableView table;
    REQUIRE(true);
}
