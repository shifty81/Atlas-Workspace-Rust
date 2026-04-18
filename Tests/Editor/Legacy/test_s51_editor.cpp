#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"

using namespace NF;

// ── LayoutPreset ─────────────────────────────────────────────────

TEST_CASE("LayoutPreset default is invalid", "[Editor][S51]") {
    LayoutPreset p;
    REQUIRE_FALSE(p.isValid());
    REQUIRE_FALSE(p.isBuiltIn);
    REQUIRE_FALSE(p.isModified);
}

TEST_CASE("LayoutPreset valid when populated", "[Editor][S51]") {
    LayoutPreset p;
    p.name = "Default";
    p.serializedData = "layout:Default\n";
    REQUIRE(p.isValid());
}

TEST_CASE("LayoutPreset markModified/clearModified", "[Editor][S51]") {
    LayoutPreset p;
    p.name = "Test";
    p.serializedData = "data";
    p.markModified();
    REQUIRE(p.isModified);
    p.clearModified();
    REQUIRE_FALSE(p.isModified);
}

// ── LayoutSerializer ─────────────────────────────────────────────

TEST_CASE("LayoutSerializer serialize produces output", "[Editor][S51]") {
    WorkspaceLayout layout("TestLayout");
    auto data = LayoutSerializer::serialize(layout);
    REQUIRE_FALSE(data.empty());
    REQUIRE(data.find("layout:TestLayout") != std::string::npos);
}

TEST_CASE("LayoutSerializer serialize includes counts", "[Editor][S51]") {
    WorkspaceLayout layout("WithPanels");
    layout.addPanel({"p1", "Panel1", LayoutPanelType::Viewport, LayoutDockZone::Left, 100, 200, true, false});
    auto data = LayoutSerializer::serialize(layout);
    REQUIRE(data.find("panel_count:1") != std::string::npos);
}

TEST_CASE("LayoutSerializer deserialize valid data", "[Editor][S51]") {
    WorkspaceLayout layout("Dest");
    bool ok = LayoutSerializer::deserialize("layout:Source\npanel_count:0\n", layout);
    REQUIRE(ok);
}

TEST_CASE("LayoutSerializer deserialize empty fails", "[Editor][S51]") {
    WorkspaceLayout layout("Dest");
    REQUIRE_FALSE(LayoutSerializer::deserialize("", layout));
}

TEST_CASE("LayoutSerializer deserialize no layout tag fails", "[Editor][S51]") {
    WorkspaceLayout layout("Dest");
    REQUIRE_FALSE(LayoutSerializer::deserialize("panel_count:0\n", layout));
}

// ── LayoutPersistenceManager ─────────────────────────────────────

TEST_CASE("LayoutPersistenceManager starts empty", "[Editor][S51]") {
    LayoutPersistenceManager mgr;
    REQUIRE(mgr.presetCount() == 0);
    REQUIRE(mgr.builtInCount() == 0);
    REQUIRE(mgr.userCount() == 0);
    REQUIRE_FALSE(mgr.autoSave());
}

TEST_CASE("LayoutPersistenceManager savePreset", "[Editor][S51]") {
    LayoutPersistenceManager mgr;
    WorkspaceLayout layout("MyLayout");
    REQUIRE(mgr.savePreset("Preset1", layout));
    REQUIRE(mgr.presetCount() == 1);
    REQUIRE(mgr.userCount() == 1);
}

TEST_CASE("LayoutPersistenceManager savePreset overwrites existing", "[Editor][S51]") {
    LayoutPersistenceManager mgr;
    WorkspaceLayout layout1("Layout1");
    WorkspaceLayout layout2("Layout2");
    mgr.savePreset("Preset", layout1);
    REQUIRE(mgr.savePreset("Preset", layout2));
    REQUIRE(mgr.presetCount() == 1); // not duplicated
}

TEST_CASE("LayoutPersistenceManager loadPreset", "[Editor][S51]") {
    LayoutPersistenceManager mgr;
    WorkspaceLayout srcLayout("Source");
    mgr.savePreset("TestPreset", srcLayout);
    WorkspaceLayout dest("Dest");
    REQUIRE(mgr.loadPreset("TestPreset", dest));
}

TEST_CASE("LayoutPersistenceManager loadPreset nonexistent", "[Editor][S51]") {
    LayoutPersistenceManager mgr;
    WorkspaceLayout dest("Dest");
    REQUIRE_FALSE(mgr.loadPreset("nonexistent", dest));
}

TEST_CASE("LayoutPersistenceManager removePreset", "[Editor][S51]") {
    LayoutPersistenceManager mgr;
    WorkspaceLayout layout("L");
    mgr.savePreset("P", layout);
    REQUIRE(mgr.removePreset("P"));
    REQUIRE(mgr.presetCount() == 0);
}

TEST_CASE("LayoutPersistenceManager cannot remove built-in", "[Editor][S51]") {
    LayoutPersistenceManager mgr;
    mgr.addBuiltInPreset("Default", "layout:Default\n");
    REQUIRE_FALSE(mgr.removePreset("Default"));
    REQUIRE(mgr.presetCount() == 1);
}

TEST_CASE("LayoutPersistenceManager renamePreset", "[Editor][S51]") {
    LayoutPersistenceManager mgr;
    WorkspaceLayout layout("L");
    mgr.savePreset("Old", layout);
    REQUIRE(mgr.renamePreset("Old", "New"));
    REQUIRE(mgr.findPreset("New") != nullptr);
    REQUIRE(mgr.findPreset("Old") == nullptr);
}

TEST_CASE("LayoutPersistenceManager renamePreset fails for duplicate", "[Editor][S51]") {
    LayoutPersistenceManager mgr;
    WorkspaceLayout l1("L1"), l2("L2");
    mgr.savePreset("A", l1);
    mgr.savePreset("B", l2);
    REQUIRE_FALSE(mgr.renamePreset("A", "B"));
}

TEST_CASE("LayoutPersistenceManager built-in presets", "[Editor][S51]") {
    LayoutPersistenceManager mgr;
    mgr.addBuiltInPreset("Default", "layout:Default\n");
    mgr.addBuiltInPreset("Debug", "layout:Debug\n");
    REQUIRE(mgr.builtInCount() == 2);
    REQUIRE(mgr.userCount() == 0);
}

TEST_CASE("LayoutPersistenceManager autoSave", "[Editor][S51]") {
    LayoutPersistenceManager mgr;
    mgr.setAutoSave(true);
    REQUIRE(mgr.autoSave());
}

TEST_CASE("LayoutPersistenceManager lastUsedPreset", "[Editor][S51]") {
    LayoutPersistenceManager mgr;
    mgr.setLastUsedPreset("MyPreset");
    REQUIRE(mgr.lastUsedPreset() == "MyPreset");
}

TEST_CASE("LayoutPersistenceManager MAX_PRESETS limit", "[Editor][S51]") {
    LayoutPersistenceManager mgr;
    WorkspaceLayout layout("L");
    for (size_t i = 0; i < LayoutPersistenceManager::MAX_PRESETS; ++i) {
        mgr.savePreset("P" + std::to_string(i), layout);
    }
    REQUIRE(mgr.presetCount() == LayoutPersistenceManager::MAX_PRESETS);
    REQUIRE_FALSE(mgr.savePreset("OneMore", layout));
}
