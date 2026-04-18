#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"

using namespace NF;

TEST_CASE("ThemeMode names cover all 4 values", "[Editor][S27]") {
    REQUIRE(std::string(themeModeName(ThemeMode::Light))       == "Light");
    REQUIRE(std::string(themeModeName(ThemeMode::Dark))        == "Dark");
    REQUIRE(std::string(themeModeName(ThemeMode::HighContrast))== "HighContrast");
    REQUIRE(std::string(themeModeName(ThemeMode::Custom))      == "Custom");
}

TEST_CASE("ThemeColor names cover all 8 values", "[Editor][S27]") {
    REQUIRE(std::string(themeColorName(ThemeColor::Background)) == "Background");
    REQUIRE(std::string(themeColorName(ThemeColor::Foreground)) == "Foreground");
    REQUIRE(std::string(themeColorName(ThemeColor::Primary))    == "Primary");
    REQUIRE(std::string(themeColorName(ThemeColor::Secondary))  == "Secondary");
    REQUIRE(std::string(themeColorName(ThemeColor::Accent))     == "Accent");
    REQUIRE(std::string(themeColorName(ThemeColor::Border))     == "Border");
    REQUIRE(std::string(themeColorName(ThemeColor::Error))      == "Error");
    REQUIRE(std::string(themeColorName(ThemeColor::Warning))    == "Warning");
}

TEST_CASE("ThemeToken matches returns correct result", "[Editor][S27]") {
    ThemeToken t;
    t.key   = "color-primary";
    t.value = "#FF0000";
    t.mode  = ThemeMode::Dark;

    REQUIRE(t.matches(ThemeMode::Dark));
    REQUIRE_FALSE(t.matches(ThemeMode::Light));
}

TEST_CASE("ThemeToken update changes value", "[Editor][S27]") {
    ThemeToken t;
    t.key   = "color-bg";
    t.value = "#000000";

    t.update("#FFFFFF");
    REQUIRE(t.value == "#FFFFFF");
}

TEST_CASE("Theme addToken and duplicate rejection", "[Editor][S27]") {
    Theme theme;
    theme.name = "dark-theme";

    ThemeToken a; a.key = "bg";   a.value = "#111";
    ThemeToken b; b.key = "fg";   b.value = "#EEE";
    ThemeToken dup; dup.key = "bg"; dup.value = "#222";

    REQUIRE(theme.addToken(a));
    REQUIRE(theme.addToken(b));
    REQUIRE_FALSE(theme.addToken(dup));
    REQUIRE(theme.tokenCount() == 2);
}

TEST_CASE("Theme removeToken returns correct result", "[Editor][S27]") {
    Theme theme;
    theme.name = "light-theme";

    ThemeToken t; t.key = "accent"; t.value = "#0066FF";
    theme.addToken(t);

    REQUIRE(theme.removeToken("accent"));
    REQUIRE_FALSE(theme.removeToken("accent"));
    REQUIRE(theme.tokenCount() == 0);
}

TEST_CASE("Theme findToken returns correct pointer or nullptr", "[Editor][S27]") {
    Theme theme;
    theme.name = "custom-theme";

    ThemeToken t; t.key = "border"; t.value = "#888";
    theme.addToken(t);

    REQUIRE(theme.findToken("border") != nullptr);
    REQUIRE(theme.findToken("border")->value == "#888");
    REQUIRE(theme.findToken("missing") == nullptr);
}

TEST_CASE("Theme bumpVersion increments version", "[Editor][S27]") {
    Theme theme;
    theme.name = "versioned";
    REQUIRE(theme.version == 1);
    theme.bumpVersion();
    REQUIRE(theme.version == 2);
    theme.bumpVersion();
    REQUIRE(theme.version == 3);
}

TEST_CASE("ThemeManager addTheme and duplicate rejection", "[Editor][S27]") {
    ThemeManager mgr;

    Theme a; a.name = "dark";
    Theme b; b.name = "light";
    Theme dup; dup.name = "dark";

    REQUIRE(mgr.addTheme(a));
    REQUIRE(mgr.addTheme(b));
    REQUIRE_FALSE(mgr.addTheme(dup));
    REQUIRE(mgr.themeCount() == 2);
}

TEST_CASE("ThemeManager setActive and find returns correct pointer", "[Editor][S27]") {
    ThemeManager mgr;

    Theme a; a.name = "solarized";
    mgr.addTheme(a);

    REQUIRE(mgr.setActive("solarized"));
    REQUIRE(mgr.active() != nullptr);
    REQUIRE(mgr.active()->name == "solarized");
    REQUIRE_FALSE(mgr.setActive("nonexistent"));
}

TEST_CASE("ThemeManager hasActive is false until setActive", "[Editor][S27]") {
    ThemeManager mgr;

    REQUIRE_FALSE(mgr.hasActive());

    Theme t; t.name = "monokai";
    mgr.addTheme(t);
    mgr.setActive("monokai");

    REQUIRE(mgr.hasActive());
}

TEST_CASE("ThemeManager applyMode changes active theme mode", "[Editor][S27]") {
    ThemeManager mgr;

    Theme t; t.name = "adaptable"; t.m_mode = ThemeMode::Dark;
    mgr.addTheme(t);
    mgr.setActive("adaptable");

    REQUIRE(mgr.active()->mode() == ThemeMode::Dark);
    REQUIRE(mgr.applyMode(ThemeMode::Light));
    REQUIRE(mgr.active()->mode() == ThemeMode::Light);
}

TEST_CASE("ThemeManager removeTheme reduces count", "[Editor][S27]") {
    ThemeManager mgr;

    Theme a; a.name = "alpha";
    Theme b; b.name = "beta";
    mgr.addTheme(a);
    mgr.addTheme(b);

    REQUIRE(mgr.themeCount() == 2);
    REQUIRE(mgr.removeTheme("alpha"));
    REQUIRE(mgr.themeCount() == 1);
    REQUIRE_FALSE(mgr.removeTheme("alpha"));
}

TEST_CASE("ThemeManager MAX_THEMES limit is enforced", "[Editor][S27]") {
    ThemeManager mgr;

    for (size_t i = 0; i < ThemeManager::MAX_THEMES; ++i) {
        Theme t; t.name = "theme-" + std::to_string(i);
        REQUIRE(mgr.addTheme(t));
    }

    Theme overflow; overflow.name = "overflow";
    REQUIRE_FALSE(mgr.addTheme(overflow));
    REQUIRE(mgr.themeCount() == ThemeManager::MAX_THEMES);
}
