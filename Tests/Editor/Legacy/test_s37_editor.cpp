#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/IconEditor.h"

using namespace NF;

TEST_CASE("IconSize names cover all 5 values", "[Editor][S37]") {
    REQUIRE(std::string(iconSizeName(IconSize::XSmall)) == "XSmall");
    REQUIRE(std::string(iconSizeName(IconSize::Small))  == "Small");
    REQUIRE(std::string(iconSizeName(IconSize::Medium)) == "Medium");
    REQUIRE(std::string(iconSizeName(IconSize::Large))  == "Large");
    REQUIRE(std::string(iconSizeName(IconSize::XLarge)) == "XLarge");
}

TEST_CASE("IconTheme names cover all 4 values", "[Editor][S37]") {
    REQUIRE(std::string(iconThemeName(IconTheme::Light))        == "Light");
    REQUIRE(std::string(iconThemeName(IconTheme::Dark))         == "Dark");
    REQUIRE(std::string(iconThemeName(IconTheme::HighContrast)) == "HighContrast");
    REQUIRE(std::string(iconThemeName(IconTheme::Monochrome))   == "Monochrome");
}

TEST_CASE("IconState names cover all 5 values", "[Editor][S37]") {
    REQUIRE(std::string(iconStateName(IconState::Normal))   == "Normal");
    REQUIRE(std::string(iconStateName(IconState::Hover))    == "Hover");
    REQUIRE(std::string(iconStateName(IconState::Pressed))  == "Pressed");
    REQUIRE(std::string(iconStateName(IconState::Disabled)) == "Disabled");
    REQUIRE(std::string(iconStateName(IconState::Selected)) == "Selected");
}

TEST_CASE("IconAsset default values", "[Editor][S37]") {
    IconAsset icon("save");
    REQUIRE(icon.name()         == "save");
    REQUIRE(icon.size()         == IconSize::Medium);
    REQUIRE(icon.theme()        == IconTheme::Light);
    REQUIRE(icon.state()        == IconState::Normal);
    REQUIRE(icon.pixelDensity() == Catch::Approx(1.0f));
    REQUIRE_FALSE(icon.isScalable());
    REQUIRE_FALSE(icon.isDirty());
    REQUIRE_FALSE(icon.isDisabled());
    REQUIRE_FALSE(icon.isSelected());
    REQUIRE_FALSE(icon.isHighDPI());
}

TEST_CASE("IconAsset isDisabled and isSelected detect correct states", "[Editor][S37]") {
    IconAsset icon("delete");
    icon.setState(IconState::Disabled);
    REQUIRE(icon.isDisabled());
    REQUIRE_FALSE(icon.isSelected());

    icon.setState(IconState::Selected);
    REQUIRE(icon.isSelected());
    REQUIRE_FALSE(icon.isDisabled());
}

TEST_CASE("IconAsset isHighDPI threshold at 2.0", "[Editor][S37]") {
    IconAsset icon("copy");
    icon.setPixelDensity(1.0f);
    REQUIRE_FALSE(icon.isHighDPI());

    icon.setPixelDensity(2.0f);
    REQUIRE(icon.isHighDPI());

    icon.setPixelDensity(3.0f);
    REQUIRE(icon.isHighDPI());
}

TEST_CASE("IconAsset setters round-trip", "[Editor][S37]") {
    IconAsset icon("paste", IconSize::Small);
    icon.setTheme(IconTheme::Dark);
    icon.setState(IconState::Hover);
    icon.setSize(IconSize::Large);
    icon.setScalable(true);
    icon.setDirty(true);
    icon.setPixelDensity(2.5f);

    REQUIRE(icon.theme()        == IconTheme::Dark);
    REQUIRE(icon.state()        == IconState::Hover);
    REQUIRE(icon.size()         == IconSize::Large);
    REQUIRE(icon.isScalable());
    REQUIRE(icon.isDirty());
    REQUIRE(icon.pixelDensity() == Catch::Approx(2.5f));
}

TEST_CASE("IconEditor addIcon and duplicate rejection", "[Editor][S37]") {
    IconEditor editor;
    IconAsset a("open");
    IconAsset b("close");
    IconAsset dup("open");

    REQUIRE(editor.addIcon(a));
    REQUIRE(editor.addIcon(b));
    REQUIRE_FALSE(editor.addIcon(dup));
    REQUIRE(editor.iconCount() == 2);
}

TEST_CASE("IconEditor removeIcon clears activeIcon", "[Editor][S37]") {
    IconEditor editor;
    IconAsset a("cut");
    editor.addIcon(a);
    editor.setActiveIcon("cut");
    REQUIRE(editor.activeIcon() == "cut");

    editor.removeIcon("cut");
    REQUIRE(editor.iconCount()       == 0);
    REQUIRE(editor.activeIcon().empty());
}

TEST_CASE("IconEditor findIcon returns pointer or nullptr", "[Editor][S37]") {
    IconEditor editor;
    IconAsset a("undo");
    editor.addIcon(a);

    REQUIRE(editor.findIcon("undo") != nullptr);
    REQUIRE(editor.findIcon("undo")->name() == "undo");
    REQUIRE(editor.findIcon("ghost") == nullptr);
}

TEST_CASE("IconEditor setActiveIcon returns false for missing icon", "[Editor][S37]") {
    IconEditor editor;
    REQUIRE_FALSE(editor.setActiveIcon("missing"));
    REQUIRE(editor.activeIcon().empty());
}

TEST_CASE("IconEditor dirtyCount, scalableCount, disabledCount, highDPICount", "[Editor][S37]") {
    IconEditor editor;
    IconAsset a("a"); a.setDirty(true); a.setScalable(true); a.setState(IconState::Disabled); a.setPixelDensity(2.0f);
    IconAsset b("b"); b.setDirty(true); b.setPixelDensity(3.0f);
    IconAsset c("c"); c.setScalable(true); c.setState(IconState::Disabled);

    editor.addIcon(a);
    editor.addIcon(b);
    editor.addIcon(c);

    REQUIRE(editor.dirtyCount()    == 2);
    REQUIRE(editor.scalableCount() == 2);
    REQUIRE(editor.disabledCount() == 2);
    REQUIRE(editor.highDPICount()  == 2);
}

TEST_CASE("IconEditor countByTheme and countBySize", "[Editor][S37]") {
    IconEditor editor;
    IconAsset a("a"); a.setTheme(IconTheme::Dark);  a.setSize(IconSize::Large);
    IconAsset b("b"); a.setTheme(IconTheme::Dark);  b.setSize(IconSize::Large);
    IconAsset c("c"); c.setTheme(IconTheme::Light); c.setSize(IconSize::Small);

    editor.addIcon(a);
    editor.addIcon(b);
    editor.addIcon(c);

    REQUIRE(editor.countByTheme(IconTheme::Dark)  >= 1);
    REQUIRE(editor.countByTheme(IconTheme::Light) >= 1);
    REQUIRE(editor.countBySize(IconSize::Small)   == 1);
}

TEST_CASE("IconEditor MAX_ICONS limit enforced", "[Editor][S37]") {
    IconEditor editor;
    for (size_t i = 0; i < IconEditor::MAX_ICONS; ++i) {
        IconAsset ic("Icon" + std::to_string(i));
        REQUIRE(editor.addIcon(ic));
    }
    IconAsset overflow("Overflow");
    REQUIRE_FALSE(editor.addIcon(overflow));
    REQUIRE(editor.iconCount() == IconEditor::MAX_ICONS);
}

TEST_CASE("IconAsset pixel density defaults to 1.0", "[Editor][S37]") {
    IconAsset icon("zoom", IconSize::XLarge);
    REQUIRE(icon.pixelDensity() == Catch::Approx(1.0f));
    icon.setPixelDensity(4.0f);
    REQUIRE(icon.pixelDensity() == Catch::Approx(4.0f));
    REQUIRE(icon.isHighDPI());
}
