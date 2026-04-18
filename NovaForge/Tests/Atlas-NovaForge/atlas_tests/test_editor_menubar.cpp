/**
 * Tests for the EditorMenuBar and the menuBar widget.
 */

#include <cassert>
#include <string>
#include <vector>
#include "../editor/ui/EditorMenuBar.h"
#include "../editor/ui/EditorLayout.h"
#include "../editor/ui/EditorPanel.h"
#include "../cpp_client/include/ui/atlas/atlas_widgets.h"

using namespace atlas;
using namespace atlas::editor;

// ── Helper panel ─────────────────────────────────────────────────

class MenuTestPanel : public EditorPanel {
public:
    explicit MenuTestPanel(const char* name) : m_name(name) {}
    const char* Name() const override { return m_name; }
    void Draw() override { ++drawCount; }
    int drawCount = 0;
private:
    const char* m_name;
};

// ══════════════════════════════════════════════════════════════════
// EditorMenuBar structure tests
// ══════════════════════════════════════════════════════════════════

void test_menubar_empty() {
    EditorMenuBar bar;
    bar.Build();
    // File menu is always present; View has no items but still exists
    assert(bar.Menus().size() >= 2);
    assert(bar.Menus()[0].label == "File");
    assert(bar.Menus()[1].label == "View");
}

void test_menubar_file_menu_items() {
    EditorMenuBar bar;
    bar.Build();
    const auto& fileItems = bar.Menus()[0].items;
    assert(fileItems.size() == 4);
    assert(fileItems[0].label == "Save Layout");
    assert(fileItems[1].label == "Load Layout");
    assert(fileItems[2].label.empty()); // separator
    assert(fileItems[3].label == "Exit");
}

void test_menubar_view_menu_lists_panels() {
    MenuTestPanel vp("Viewport");
    MenuTestPanel con("Console");
    EditorMenuBar bar;
    bar.RegisterPanel(&vp);
    bar.RegisterPanel(&con);
    bar.Build();

    const auto& viewItems = bar.Menus()[1].items;
    assert(viewItems.size() == 2);
    assert(viewItems[0].label == "Viewport");
    assert(viewItems[1].label == "Console");
}

void test_menubar_pcg_menu_filters_panels() {
    MenuTestPanel vp("Viewport");
    MenuTestPanel pcg("PCG Preview");
    MenuTestPanel ship("Ship Archetype");
    MenuTestPanel gen("Generation Style");
    MenuTestPanel con("Console");
    EditorMenuBar bar;
    bar.RegisterPanel(&vp);
    bar.RegisterPanel(&pcg);
    bar.RegisterPanel(&ship);
    bar.RegisterPanel(&gen);
    bar.RegisterPanel(&con);
    bar.Build();

    assert(bar.Menus().size() == 3);
    assert(bar.Menus()[2].label == "PCG Content");

    const auto& pcgItems = bar.Menus()[2].items;
    assert(pcgItems.size() == 3);
    assert(pcgItems[0].label == "PCG Preview");
    assert(pcgItems[1].label == "Ship Archetype");
    assert(pcgItems[2].label == "Generation Style");
}

void test_menubar_no_pcg_menu_if_no_pcg_panels() {
    MenuTestPanel vp("Viewport");
    MenuTestPanel con("Console");
    EditorMenuBar bar;
    bar.RegisterPanel(&vp);
    bar.RegisterPanel(&con);
    bar.Build();

    // Only File and View — no PCG Content
    assert(bar.Menus().size() == 2);
}

void test_menubar_view_checked_state() {
    MenuTestPanel vp("Viewport");
    MenuTestPanel con("Console");
    vp.SetVisible(true);
    con.SetVisible(false);

    EditorMenuBar bar;
    bar.RegisterPanel(&vp);
    bar.RegisterPanel(&con);
    bar.Build();

    const auto& viewItems = bar.Menus()[1].items;
    assert(viewItems[0].checked == true);
    assert(viewItems[1].checked == false);
}

void test_menubar_draw_returns_height() {
    EditorMenuBar bar;
    bar.Build();
    float h = bar.Draw(nullptr, 1600.0f);
    assert(h == EditorMenuBar::MenuBarHeight);
}

void test_menubar_initial_state() {
    EditorMenuBar bar;
    assert(bar.State().openMenu == -1);
}

// ══════════════════════════════════════════════════════════════════
// atlas::Menu / MenuItem structure tests
// ══════════════════════════════════════════════════════════════════

void test_menu_item_defaults() {
    MenuItem item;
    assert(item.label.empty());
    assert(item.enabled == true);
    assert(item.checked == false);
}

void test_menu_bar_state_defaults() {
    MenuBarState state;
    assert(state.openMenu == -1);
}

// ══════════════════════════════════════════════════════════════════
// EditorLayout integration tests
// ══════════════════════════════════════════════════════════════════

void test_layout_menubar_panels_registered() {
    MenuTestPanel vp("Viewport");
    MenuTestPanel con("Console");

    EditorLayout layout;
    layout.RegisterPanel(&vp);
    layout.RegisterPanel(&con);
    layout.MenuBar().Build();

    assert(layout.MenuBar().Panels().size() == 2);
    assert(layout.MenuBar().Menus().size() >= 2);
}

void test_layout_draw_with_menubar() {
    MenuTestPanel vp("Viewport");
    EditorLayout layout;
    layout.RegisterPanel(&vp);
    layout.MenuBar().Build();

    auto& root = layout.Root();
    root.split = DockSplit::None;
    root.panel = &vp;

    // Drawing in headless mode should not crash
    layout.Draw();
    assert(vp.drawCount == 1);
}
