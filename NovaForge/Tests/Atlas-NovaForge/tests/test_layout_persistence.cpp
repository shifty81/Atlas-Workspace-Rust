#include <iostream>
#include <cassert>
#include <filesystem>
#include <fstream>

#include "../editor/ui/LayoutPersistence.h"
#include "../editor/panels/ConsolePanel.h"
#include "../editor/panels/ECSInspectorPanel.h"

using namespace atlas::editor;

// Simple test panels
class TestPanelA : public EditorPanel {
public:
    const char* Name() const override { return "PanelA"; }
    void Draw() override {}
};

class TestPanelB : public EditorPanel {
public:
    const char* Name() const override { return "PanelB"; }
    void Draw() override {}
};

void test_layout_persistence_serialize_empty() {
    EditorLayout layout;
    LayoutPersistence persistence;

    std::string json = persistence.Serialize(layout);
    assert(!json.empty());
    assert(json.find("panels") != std::string::npos);
    assert(json.find("dockTree") != std::string::npos);

    std::cout << "[PASS] test_layout_persistence_serialize_empty" << std::endl;
}

void test_layout_persistence_roundtrip() {
    TestPanelA panelA;
    TestPanelB panelB;
    panelB.SetVisible(false);

    EditorLayout layout;
    layout.RegisterPanel(&panelA);
    layout.RegisterPanel(&panelB);

    // Set up dock tree
    layout.Root().split = DockSplit::Horizontal;
    layout.Root().splitRatio = 0.3f;
    layout.Root().a = std::make_unique<DockNode>();
    layout.Root().b = std::make_unique<DockNode>();
    layout.Root().a->panel = &panelA;
    layout.Root().b->panel = &panelB;

    LayoutPersistence persistence;
    std::string json = persistence.Serialize(layout);

    // Create a new layout with same panels and deserialize
    EditorLayout layout2;
    TestPanelA panelA2;
    TestPanelB panelB2;
    layout2.RegisterPanel(&panelA2);
    layout2.RegisterPanel(&panelB2);

    bool ok = persistence.Deserialize(layout2, json);
    assert(ok);

    // PanelB was set to invisible
    assert(panelA2.IsVisible());
    assert(!panelB2.IsVisible());

    // Dock tree should be restored
    assert(layout2.Root().split == DockSplit::Horizontal);
    assert(layout2.Root().splitRatio > 0.29f && layout2.Root().splitRatio < 0.31f);
    assert(layout2.Root().a != nullptr);
    assert(layout2.Root().b != nullptr);
    assert(layout2.Root().a->panel != nullptr);
    assert(layout2.Root().b->panel != nullptr);

    std::cout << "[PASS] test_layout_persistence_roundtrip" << std::endl;
}

void test_layout_persistence_save_load_file() {
    const std::string path = "/tmp/atlas_layout_test.json";
    std::remove(path.c_str());

    TestPanelA panelA;
    EditorLayout layout;
    layout.RegisterPanel(&panelA);
    layout.Root().panel = &panelA;

    LayoutPersistence persistence;
    assert(persistence.SaveLayout(layout, path));
    assert(std::filesystem::exists(path));

    // Load into new layout
    TestPanelA panelA2;
    EditorLayout layout2;
    layout2.RegisterPanel(&panelA2);

    assert(persistence.LoadLayout(layout2, path));
    assert(layout2.Root().panel != nullptr);

    std::remove(path.c_str());
    std::cout << "[PASS] test_layout_persistence_save_load_file" << std::endl;
}

void test_layout_persistence_load_nonexistent() {
    EditorLayout layout;
    LayoutPersistence persistence;

    assert(!persistence.LoadLayout(layout, "/tmp/nonexistent_layout.json"));
    assert(!persistence.LastError().empty());

    std::cout << "[PASS] test_layout_persistence_load_nonexistent" << std::endl;
}

void test_layout_persistence_panel_closable() {
    TestPanelA panelA;
    panelA.SetClosable(false);

    EditorLayout layout;
    layout.RegisterPanel(&panelA);

    LayoutPersistence persistence;
    std::string json = persistence.Serialize(layout);
    assert(json.find("\"closable\": false") != std::string::npos);

    // Deserialize into new layout
    TestPanelA panelA2;
    EditorLayout layout2;
    layout2.RegisterPanel(&panelA2);

    persistence.Deserialize(layout2, json);
    assert(!panelA2.IsClosable());

    std::cout << "[PASS] test_layout_persistence_panel_closable" << std::endl;
}

void test_layout_persistence_vertical_split() {
    TestPanelA panelA;
    TestPanelB panelB;

    EditorLayout layout;
    layout.RegisterPanel(&panelA);
    layout.RegisterPanel(&panelB);

    layout.Root().split = DockSplit::Vertical;
    layout.Root().splitRatio = 0.7f;
    layout.Root().a = std::make_unique<DockNode>();
    layout.Root().b = std::make_unique<DockNode>();
    layout.Root().a->panel = &panelA;
    layout.Root().b->panel = &panelB;

    LayoutPersistence persistence;
    std::string json = persistence.Serialize(layout);

    TestPanelA panelA2;
    TestPanelB panelB2;
    EditorLayout layout2;
    layout2.RegisterPanel(&panelA2);
    layout2.RegisterPanel(&panelB2);

    assert(persistence.Deserialize(layout2, json));
    assert(layout2.Root().split == DockSplit::Vertical);
    assert(layout2.Root().splitRatio > 0.69f && layout2.Root().splitRatio < 0.71f);

    std::cout << "[PASS] test_layout_persistence_vertical_split" << std::endl;
}
