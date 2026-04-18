#include "../engine/ui/UIStyle.h"
#include <iostream>
#include <cassert>

using namespace atlas::ui;

void test_ui_style_dark_defaults() {
    UIStyle s = UIStyle::Dark();
    assert(s.background.r == 30);
    assert(s.background.g == 30);
    assert(s.background.b == 30);
    assert(s.background.a == 255);
    assert(s.textPrimary.r == 220);
    assert(s.accent.r == 0);
    assert(s.accent.g == 122);
    assert(s.accent.b == 204);
    std::cout << "[PASS] test_ui_style_dark_defaults" << std::endl;
}

void test_ui_style_light_theme() {
    UIStyle s = UIStyle::Light();
    assert(s.background.r == 240);
    assert(s.background.g == 240);
    assert(s.textPrimary.r == 30);
    assert(s.textPrimary.g == 30);
    std::cout << "[PASS] test_ui_style_light_theme" << std::endl;
}

void test_ui_style_spacing_defaults() {
    UIStyle s;
    assert(s.paddingSmall == 4);
    assert(s.paddingMedium == 8);
    assert(s.paddingLarge == 16);
    assert(s.itemSpacing == 4);
    assert(s.sectionGap == 12);
    std::cout << "[PASS] test_ui_style_spacing_defaults" << std::endl;
}

void test_ui_style_typography_defaults() {
    UIStyle s;
    assert(s.fontSizeSmall > 0.0f);
    assert(s.fontSizeNormal > s.fontSizeSmall);
    assert(s.fontSizeLarge > s.fontSizeNormal);
    assert(s.fontSizeHeading > s.fontSizeLarge);
    assert(s.lineHeight > 0);
    std::cout << "[PASS] test_ui_style_typography_defaults" << std::endl;
}

void test_ui_style_border_defaults() {
    UIStyle s;
    assert(s.borderThickness == 1);
    assert(s.cornerRadius == 4);
    std::cout << "[PASS] test_ui_style_border_defaults" << std::endl;
}

void test_ui_style_status_colors() {
    UIStyle s;
    // OK should be green-ish
    assert(s.statusOk.g > s.statusOk.r);
    // Warning should be yellow-ish
    assert(s.statusWarning.r > s.statusWarning.b);
    // Error should be red-ish
    assert(s.statusError.r > s.statusError.g);
    std::cout << "[PASS] test_ui_style_status_colors" << std::endl;
}

void test_ui_style_dark_light_differ() {
    UIStyle dark = UIStyle::Dark();
    UIStyle light = UIStyle::Light();
    // Background should differ significantly
    assert(dark.background.r != light.background.r);
    assert(dark.textPrimary.r != light.textPrimary.r);
    std::cout << "[PASS] test_ui_style_dark_light_differ" << std::endl;
}
