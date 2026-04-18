#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"

using namespace NF;

// ── EditorTheme dark() ───────────────────────────────────────────

TEST_CASE("EditorTheme dark default panel colors", "[Editor][S58]") {
    EditorTheme t = EditorTheme::dark();
    REQUIRE(t.panelBackground == 0x2B2B2BFF);
    REQUIRE(t.panelHeader     == 0x3C3C3CFF);
    REQUIRE(t.panelText       == 0xDCDCDCFF);
}

TEST_CASE("EditorTheme dark selection colors", "[Editor][S58]") {
    EditorTheme t = EditorTheme::dark();
    REQUIRE(t.selectionHighlight == 0x264F78FF);
    REQUIRE(t.selectionBorder    == 0x007ACCFF);
}

TEST_CASE("EditorTheme dark input colors", "[Editor][S58]") {
    EditorTheme t = EditorTheme::dark();
    REQUIRE(t.inputBackground == 0x1E1E1EFF);
    REQUIRE(t.inputText       == 0xD4D4D4FF);
}

TEST_CASE("EditorTheme dark viewport colors", "[Editor][S58]") {
    EditorTheme t = EditorTheme::dark();
    REQUIRE(t.viewportBackground == 0x1A1A1AFF);
    REQUIRE(t.gridColor          == 0x333333FF);
}

TEST_CASE("EditorTheme dark dirty indicator color", "[Editor][S58]") {
    EditorTheme t = EditorTheme::dark();
    REQUIRE(t.dirtyIndicator == 0xE8A435FF);
}

TEST_CASE("EditorTheme dark font sizes", "[Editor][S58]") {
    EditorTheme t = EditorTheme::dark();
    REQUIRE(t.fontSize       == 14.f);
    REQUIRE(t.headerFontSize == 16.f);
    REQUIRE(t.smallFontSize  == 12.f);
}

TEST_CASE("EditorTheme dark spacing values", "[Editor][S58]") {
    EditorTheme t = EditorTheme::dark();
    REQUIRE(t.panelPadding   == 8.f);
    REQUIRE(t.itemSpacing    == 4.f);
    REQUIRE(t.sectionSpacing == 12.f);
}

// ── EditorTheme light() ──────────────────────────────────────────

TEST_CASE("EditorTheme light panel background is bright", "[Editor][S58]") {
    EditorTheme t = EditorTheme::light();
    REQUIRE(t.panelBackground == 0xF0F0F0FF);
}

TEST_CASE("EditorTheme light panel text is dark", "[Editor][S58]") {
    EditorTheme t = EditorTheme::light();
    REQUIRE(t.panelText == 0x1E1E1EFF);
}

TEST_CASE("EditorTheme light differs from dark", "[Editor][S58]") {
    EditorTheme d = EditorTheme::dark();
    EditorTheme l = EditorTheme::light();
    REQUIRE(d.panelBackground != l.panelBackground);
    REQUIRE(d.panelText       != l.panelText);
    REQUIRE(d.inputBackground != l.inputBackground);
}

TEST_CASE("EditorTheme light viewport and grid colors differ from dark", "[Editor][S58]") {
    EditorTheme d = EditorTheme::dark();
    EditorTheme l = EditorTheme::light();
    REQUIRE(d.viewportBackground != l.viewportBackground);
    REQUIRE(d.gridColor          != l.gridColor);
}

// ── EditorTheme toUITheme ────────────────────────────────────────

TEST_CASE("EditorTheme toUITheme copies panel colors", "[Editor][S58]") {
    EditorTheme et = EditorTheme::dark();
    UITheme ut = et.toUITheme();
    REQUIRE(ut.panelBackground == et.panelBackground);
    REQUIRE(ut.panelHeader     == et.panelHeader);
    REQUIRE(ut.panelText       == et.panelText);
}

TEST_CASE("EditorTheme toUITheme copies selection colors", "[Editor][S58]") {
    EditorTheme et = EditorTheme::dark();
    UITheme ut = et.toUITheme();
    REQUIRE(ut.selectionHighlight == et.selectionHighlight);
    REQUIRE(ut.selectionBorder    == et.selectionBorder);
}

TEST_CASE("EditorTheme toUITheme copies viewport colors", "[Editor][S58]") {
    EditorTheme et = EditorTheme::dark();
    UITheme ut = et.toUITheme();
    REQUIRE(ut.viewportBackground == et.viewportBackground);
    REQUIRE(ut.gridColor          == et.gridColor);
}

TEST_CASE("EditorTheme toUITheme copies font sizes", "[Editor][S58]") {
    EditorTheme et = EditorTheme::dark();
    UITheme ut = et.toUITheme();
    REQUIRE(ut.fontSize       == et.fontSize);
    REQUIRE(ut.headerFontSize == et.headerFontSize);
    REQUIRE(ut.smallFontSize  == et.smallFontSize);
}

// ── EditorTheme mutability ───────────────────────────────────────

TEST_CASE("EditorTheme fields can be overridden", "[Editor][S58]") {
    EditorTheme t = EditorTheme::dark();
    t.panelBackground = 0x111111FF;
    t.fontSize = 18.f;
    REQUIRE(t.panelBackground == 0x111111FF);
    REQUIRE(t.fontSize == 18.f);
}
