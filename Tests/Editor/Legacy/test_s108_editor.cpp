// S108 editor tests: UIDesignEditor, HUDEditor, MenuLayoutEditor
#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/MenuLayoutEditor.h"
#include "NF/Editor/HUDEditor.h"
#include "NF/Editor/UIDesignEditor.h"

using namespace NF;

// ── UIDesignEditor ───────────────────────────────────────────────────────────

TEST_CASE("UIDesignElementType names", "[Editor][S108]") {
    REQUIRE(std::string(uiDesignElementTypeName(UIDesignElementType::Panel))       == "Panel");
    REQUIRE(std::string(uiDesignElementTypeName(UIDesignElementType::Button))      == "Button");
    REQUIRE(std::string(uiDesignElementTypeName(UIDesignElementType::Label))       == "Label");
    REQUIRE(std::string(uiDesignElementTypeName(UIDesignElementType::Image))       == "Image");
    REQUIRE(std::string(uiDesignElementTypeName(UIDesignElementType::ProgressBar)) == "ProgressBar");
    REQUIRE(std::string(uiDesignElementTypeName(UIDesignElementType::Slider))      == "Slider");
    REQUIRE(std::string(uiDesignElementTypeName(UIDesignElementType::InputField))  == "InputField");
    REQUIRE(std::string(uiDesignElementTypeName(UIDesignElementType::Checkbox))    == "Checkbox");
    REQUIRE(std::string(uiDesignElementTypeName(UIDesignElementType::Dropdown))    == "Dropdown");
    REQUIRE(std::string(uiDesignElementTypeName(UIDesignElementType::List))        == "List");
}

TEST_CASE("UIAnchorPreset names", "[Editor][S108]") {
    REQUIRE(std::string(uiAnchorPresetName(UIAnchorPreset::TopLeft))      == "TopLeft");
    REQUIRE(std::string(uiAnchorPresetName(UIAnchorPreset::TopCenter))    == "TopCenter");
    REQUIRE(std::string(uiAnchorPresetName(UIAnchorPreset::TopRight))     == "TopRight");
    REQUIRE(std::string(uiAnchorPresetName(UIAnchorPreset::MiddleLeft))   == "MiddleLeft");
    REQUIRE(std::string(uiAnchorPresetName(UIAnchorPreset::MiddleCenter)) == "MiddleCenter");
    REQUIRE(std::string(uiAnchorPresetName(UIAnchorPreset::MiddleRight))  == "MiddleRight");
    REQUIRE(std::string(uiAnchorPresetName(UIAnchorPreset::BottomLeft))   == "BottomLeft");
    REQUIRE(std::string(uiAnchorPresetName(UIAnchorPreset::BottomCenter)) == "BottomCenter");
    REQUIRE(std::string(uiAnchorPresetName(UIAnchorPreset::BottomRight))  == "BottomRight");
    REQUIRE(std::string(uiAnchorPresetName(UIAnchorPreset::FullStretch))  == "FullStretch");
}

TEST_CASE("UIScreenResolution names", "[Editor][S108]") {
    REQUIRE(std::string(uiScreenResolutionName(UIScreenResolution::R1920x1080)) == "1920x1080");
    REQUIRE(std::string(uiScreenResolutionName(UIScreenResolution::R2560x1440)) == "2560x1440");
    REQUIRE(std::string(uiScreenResolutionName(UIScreenResolution::R3840x2160)) == "3840x2160");
    REQUIRE(std::string(uiScreenResolutionName(UIScreenResolution::R1280x720))  == "1280x720");
    REQUIRE(std::string(uiScreenResolutionName(UIScreenResolution::R1366x768))  == "1366x768");
    REQUIRE(std::string(uiScreenResolutionName(UIScreenResolution::R375x812))   == "375x812");
}

TEST_CASE("UIDesignElement defaults", "[Editor][S108]") {
    UIDesignElement el(1, "start_button", UIDesignElementType::Button);
    REQUIRE(el.id()         == 1u);
    REQUIRE(el.name()       == "start_button");
    REQUIRE(el.type()       == UIDesignElementType::Button);
    REQUIRE(el.anchor()     == UIAnchorPreset::MiddleCenter);
    REQUIRE(el.isVisible());
    REQUIRE(!el.isLocked());
    REQUIRE(!el.isSelected());
    REQUIRE(el.width()      == 100.0f);
    REQUIRE(el.height()     == 30.0f);
    REQUIRE(el.zOrder()     == 0);
}

TEST_CASE("UIDesignElement mutation", "[Editor][S108]") {
    UIDesignElement el(2, "health_bar", UIDesignElementType::ProgressBar);
    el.setAnchor(UIAnchorPreset::TopLeft);
    el.setVisible(false);
    el.setLocked(true);
    el.setSelected(true);
    el.setWidth(200.0f);
    el.setHeight(20.0f);
    el.setZOrder(5);
    REQUIRE(el.anchor()     == UIAnchorPreset::TopLeft);
    REQUIRE(!el.isVisible());
    REQUIRE(el.isLocked());
    REQUIRE(el.isSelected());
    REQUIRE(el.width()      == 200.0f);
    REQUIRE(el.height()     == 20.0f);
    REQUIRE(el.zOrder()     == 5);
}

TEST_CASE("UIDesignEditor defaults", "[Editor][S108]") {
    UIDesignEditor ed;
    REQUIRE(ed.previewResolution() == UIScreenResolution::R1920x1080);
    REQUIRE(ed.isShowGrid());
    REQUIRE(ed.isShowGuides());
    REQUIRE(ed.isSnapToGrid());
    REQUIRE(ed.elementCount()      == 0u);
    REQUIRE(ed.selectedCount()     == 0u);
}

TEST_CASE("UIDesignEditor add/remove elements", "[Editor][S108]") {
    UIDesignEditor ed;
    REQUIRE(ed.addElement(UIDesignElement(1, "bg",     UIDesignElementType::Panel)));
    REQUIRE(ed.addElement(UIDesignElement(2, "title",  UIDesignElementType::Label)));
    REQUIRE(ed.addElement(UIDesignElement(3, "start",  UIDesignElementType::Button)));
    REQUIRE(!ed.addElement(UIDesignElement(1, "bg",    UIDesignElementType::Panel)));
    REQUIRE(ed.elementCount() == 3u);
    REQUIRE(ed.removeElement(2));
    REQUIRE(ed.elementCount() == 2u);
    REQUIRE(!ed.removeElement(99));
}

TEST_CASE("UIDesignEditor counts and find", "[Editor][S108]") {
    UIDesignEditor ed;
    UIDesignElement e1(1, "bg",     UIDesignElementType::Panel);   e1.setSelected(true);
    UIDesignElement e2(2, "title",  UIDesignElementType::Label);   e2.setSelected(true);
    UIDesignElement e3(3, "start",  UIDesignElementType::Button);
    UIDesignElement e4(4, "quit",   UIDesignElementType::Button);
    ed.addElement(e1); ed.addElement(e2); ed.addElement(e3); ed.addElement(e4);
    REQUIRE(ed.selectedCount()                              == 2u);
    REQUIRE(ed.countByType(UIDesignElementType::Button)     == 2u);
    REQUIRE(ed.countByType(UIDesignElementType::Label)      == 1u);
    REQUIRE(ed.countByType(UIDesignElementType::Image)      == 0u);
    auto* found = ed.findElement(3);
    REQUIRE(found != nullptr);
    REQUIRE(found->type() == UIDesignElementType::Button);
    REQUIRE(ed.findElement(99) == nullptr);
}

TEST_CASE("UIDesignEditor mutation", "[Editor][S108]") {
    UIDesignEditor ed;
    ed.setPreviewResolution(UIScreenResolution::R2560x1440);
    ed.setShowGrid(false);
    ed.setShowGuides(false);
    ed.setSnapToGrid(false);
    REQUIRE(ed.previewResolution() == UIScreenResolution::R2560x1440);
    REQUIRE(!ed.isShowGrid());
    REQUIRE(!ed.isShowGuides());
    REQUIRE(!ed.isSnapToGrid());
}

// ── HUDEditor ────────────────────────────────────────────────────────────────

TEST_CASE("HUDElementCategory names", "[Editor][S108]") {
    REQUIRE(std::string(hudElementCategoryName(HUDElementCategory::Health))       == "Health");
    REQUIRE(std::string(hudElementCategoryName(HUDElementCategory::Stamina))      == "Stamina");
    REQUIRE(std::string(hudElementCategoryName(HUDElementCategory::Ammo))         == "Ammo");
    REQUIRE(std::string(hudElementCategoryName(HUDElementCategory::Minimap))      == "Minimap");
    REQUIRE(std::string(hudElementCategoryName(HUDElementCategory::Crosshair))    == "Crosshair");
    REQUIRE(std::string(hudElementCategoryName(HUDElementCategory::Objective))    == "Objective");
    REQUIRE(std::string(hudElementCategoryName(HUDElementCategory::Score))        == "Score");
    REQUIRE(std::string(hudElementCategoryName(HUDElementCategory::Timer))        == "Timer");
    REQUIRE(std::string(hudElementCategoryName(HUDElementCategory::Notification)) == "Notification");
    REQUIRE(std::string(hudElementCategoryName(HUDElementCategory::Debug))        == "Debug");
}

TEST_CASE("HUDVisibilityMode names", "[Editor][S108]") {
    REQUIRE(std::string(hudVisibilityModeName(HUDVisibilityMode::AlwaysVisible))   == "AlwaysVisible");
    REQUIRE(std::string(hudVisibilityModeName(HUDVisibilityMode::OnDamage))        == "OnDamage");
    REQUIRE(std::string(hudVisibilityModeName(HUDVisibilityMode::OnLowHealth))     == "OnLowHealth");
    REQUIRE(std::string(hudVisibilityModeName(HUDVisibilityMode::WhenAiming))      == "WhenAiming");
    REQUIRE(std::string(hudVisibilityModeName(HUDVisibilityMode::WhenInteracting)) == "WhenInteracting");
    REQUIRE(std::string(hudVisibilityModeName(HUDVisibilityMode::Custom))          == "Custom");
}

TEST_CASE("HUDLayoutPreset names", "[Editor][S108]") {
    REQUIRE(std::string(hudLayoutPresetName(HUDLayoutPreset::Standard))      == "Standard");
    REQUIRE(std::string(hudLayoutPresetName(HUDLayoutPreset::Minimal))       == "Minimal");
    REQUIRE(std::string(hudLayoutPresetName(HUDLayoutPreset::Immersive))     == "Immersive");
    REQUIRE(std::string(hudLayoutPresetName(HUDLayoutPreset::Accessibility)) == "Accessibility");
    REQUIRE(std::string(hudLayoutPresetName(HUDLayoutPreset::Custom))        == "Custom");
}

TEST_CASE("HUDElement defaults", "[Editor][S108]") {
    HUDElement el(1, "healthbar", HUDElementCategory::Health);
    REQUIRE(el.id()             == 1u);
    REQUIRE(el.name()           == "healthbar");
    REQUIRE(el.category()       == HUDElementCategory::Health);
    REQUIRE(el.visibilityMode() == HUDVisibilityMode::AlwaysVisible);
    REQUIRE(el.isEnabled());
    REQUIRE(el.opacity()        == 1.0f);
    REQUIRE(el.scale()          == 1.0f);
}

TEST_CASE("HUDElement mutation", "[Editor][S108]") {
    HUDElement el(2, "ammo_counter", HUDElementCategory::Ammo);
    el.setVisibilityMode(HUDVisibilityMode::WhenAiming);
    el.setEnabled(false);
    el.setOpacity(0.7f);
    el.setScale(1.2f);
    REQUIRE(el.visibilityMode() == HUDVisibilityMode::WhenAiming);
    REQUIRE(!el.isEnabled());
    REQUIRE(el.opacity()        == 0.7f);
    REQUIRE(el.scale()          == 1.2f);
}

TEST_CASE("HUDEditor defaults", "[Editor][S108]") {
    HUDEditor ed;
    REQUIRE(ed.layoutPreset()      == HUDLayoutPreset::Standard);
    REQUIRE(!ed.isPreviewEnabled());
    REQUIRE(ed.isSafeZoneEnabled());
    REQUIRE(ed.elementCount()      == 0u);
    REQUIRE(ed.enabledCount()      == 0u);
}

TEST_CASE("HUDEditor add/remove elements", "[Editor][S108]") {
    HUDEditor ed;
    REQUIRE(ed.addElement(HUDElement(1, "health",    HUDElementCategory::Health)));
    REQUIRE(ed.addElement(HUDElement(2, "ammo",      HUDElementCategory::Ammo)));
    REQUIRE(ed.addElement(HUDElement(3, "minimap",   HUDElementCategory::Minimap)));
    REQUIRE(!ed.addElement(HUDElement(1, "health",   HUDElementCategory::Health)));
    REQUIRE(ed.elementCount() == 3u);
    REQUIRE(ed.removeElement(2));
    REQUIRE(ed.elementCount() == 2u);
    REQUIRE(!ed.removeElement(99));
}

TEST_CASE("HUDEditor counts and find", "[Editor][S108]") {
    HUDEditor ed;
    HUDElement e1(1, "health",     HUDElementCategory::Health);
    HUDElement e2(2, "stamina",    HUDElementCategory::Stamina);  e2.setEnabled(false);
    HUDElement e3(3, "crosshair",  HUDElementCategory::Crosshair);
    HUDElement e4(4, "minimap",    HUDElementCategory::Minimap);
    ed.addElement(e1); ed.addElement(e2); ed.addElement(e3); ed.addElement(e4);
    REQUIRE(ed.enabledCount()                                  == 3u);
    REQUIRE(ed.countByCategory(HUDElementCategory::Health)    == 1u);
    REQUIRE(ed.countByCategory(HUDElementCategory::Crosshair) == 1u);
    REQUIRE(ed.countByCategory(HUDElementCategory::Ammo)      == 0u);
    auto* found = ed.findElement(3);
    REQUIRE(found != nullptr);
    REQUIRE(found->category() == HUDElementCategory::Crosshair);
    REQUIRE(ed.findElement(99) == nullptr);
}

TEST_CASE("HUDEditor mutation", "[Editor][S108]") {
    HUDEditor ed;
    ed.setLayoutPreset(HUDLayoutPreset::Minimal);
    ed.setPreviewEnabled(true);
    ed.setSafeZoneEnabled(false);
    REQUIRE(ed.layoutPreset()      == HUDLayoutPreset::Minimal);
    REQUIRE(ed.isPreviewEnabled());
    REQUIRE(!ed.isSafeZoneEnabled());
}

// ── MenuLayoutEditor ─────────────────────────────────────────────────────────

TEST_CASE("MenuScreenType names", "[Editor][S108]") {
    REQUIRE(std::string(menuScreenTypeName(MenuScreenType::MainMenu))      == "MainMenu");
    REQUIRE(std::string(menuScreenTypeName(MenuScreenType::PauseMenu))     == "PauseMenu");
    REQUIRE(std::string(menuScreenTypeName(MenuScreenType::OptionsMenu))   == "OptionsMenu");
    REQUIRE(std::string(menuScreenTypeName(MenuScreenType::LoadingScreen)) == "LoadingScreen");
    REQUIRE(std::string(menuScreenTypeName(MenuScreenType::Credits))       == "Credits");
    REQUIRE(std::string(menuScreenTypeName(MenuScreenType::HUDOverlay))    == "HUDOverlay");
    REQUIRE(std::string(menuScreenTypeName(MenuScreenType::GameOver))      == "GameOver");
    REQUIRE(std::string(menuScreenTypeName(MenuScreenType::Victory))       == "Victory");
}

TEST_CASE("MenuTransitionEffect names", "[Editor][S108]") {
    REQUIRE(std::string(menuTransitionEffectName(MenuTransitionEffect::None))       == "None");
    REQUIRE(std::string(menuTransitionEffectName(MenuTransitionEffect::Fade))       == "Fade");
    REQUIRE(std::string(menuTransitionEffectName(MenuTransitionEffect::SlideLeft))  == "SlideLeft");
    REQUIRE(std::string(menuTransitionEffectName(MenuTransitionEffect::SlideRight)) == "SlideRight");
    REQUIRE(std::string(menuTransitionEffectName(MenuTransitionEffect::SlideUp))    == "SlideUp");
    REQUIRE(std::string(menuTransitionEffectName(MenuTransitionEffect::SlideDown))  == "SlideDown");
    REQUIRE(std::string(menuTransitionEffectName(MenuTransitionEffect::Zoom))       == "Zoom");
    REQUIRE(std::string(menuTransitionEffectName(MenuTransitionEffect::Dissolve))   == "Dissolve");
}

TEST_CASE("MenuNavigationMode names", "[Editor][S108]") {
    REQUIRE(std::string(menuNavigationModeName(MenuNavigationMode::Mouse))    == "Mouse");
    REQUIRE(std::string(menuNavigationModeName(MenuNavigationMode::Keyboard)) == "Keyboard");
    REQUIRE(std::string(menuNavigationModeName(MenuNavigationMode::Gamepad))  == "Gamepad");
    REQUIRE(std::string(menuNavigationModeName(MenuNavigationMode::Touch))    == "Touch");
    REQUIRE(std::string(menuNavigationModeName(MenuNavigationMode::Combined)) == "Combined");
}

TEST_CASE("MenuScreen defaults", "[Editor][S108]") {
    MenuScreen screen(1, "MainMenu", MenuScreenType::MainMenu);
    REQUIRE(screen.id()                 == 1u);
    REQUIRE(screen.name()               == "MainMenu");
    REQUIRE(screen.type()               == MenuScreenType::MainMenu);
    REQUIRE(screen.transitionIn()       == MenuTransitionEffect::Fade);
    REQUIRE(screen.transitionOut()      == MenuTransitionEffect::Fade);
    REQUIRE(screen.transitionDuration() == 0.3f);
    REQUIRE(!screen.isBlurBackground());
    REQUIRE(!screen.isPauseGame());
}

TEST_CASE("MenuScreen mutation", "[Editor][S108]") {
    MenuScreen screen(2, "Pause", MenuScreenType::PauseMenu);
    screen.setTransitionIn(MenuTransitionEffect::SlideUp);
    screen.setTransitionOut(MenuTransitionEffect::SlideDown);
    screen.setTransitionDuration(0.5f);
    screen.setBlurBackground(true);
    screen.setPauseGame(true);
    REQUIRE(screen.transitionIn()       == MenuTransitionEffect::SlideUp);
    REQUIRE(screen.transitionOut()      == MenuTransitionEffect::SlideDown);
    REQUIRE(screen.transitionDuration() == 0.5f);
    REQUIRE(screen.isBlurBackground());
    REQUIRE(screen.isPauseGame());
}

TEST_CASE("MenuLayoutEditor defaults", "[Editor][S108]") {
    MenuLayoutEditor ed;
    REQUIRE(ed.navigationMode()  == MenuNavigationMode::Combined);
    REQUIRE(ed.activeScreenId()  == 0u);
    REQUIRE(ed.isShowSafeArea());
    REQUIRE(!ed.isShowOverlap());
    REQUIRE(ed.screenCount()     == 0u);
}

TEST_CASE("MenuLayoutEditor add/remove screens", "[Editor][S108]") {
    MenuLayoutEditor ed;
    REQUIRE(ed.addScreen(MenuScreen(1, "Main",    MenuScreenType::MainMenu)));
    REQUIRE(ed.addScreen(MenuScreen(2, "Pause",   MenuScreenType::PauseMenu)));
    REQUIRE(ed.addScreen(MenuScreen(3, "Options", MenuScreenType::OptionsMenu)));
    REQUIRE(!ed.addScreen(MenuScreen(1, "Main",   MenuScreenType::MainMenu)));
    REQUIRE(ed.screenCount() == 3u);
    REQUIRE(ed.removeScreen(2));
    REQUIRE(ed.screenCount() == 2u);
    REQUIRE(!ed.removeScreen(99));
}

TEST_CASE("MenuLayoutEditor counts and find", "[Editor][S108]") {
    MenuLayoutEditor ed;
    MenuScreen s1(1, "Main",    MenuScreenType::MainMenu);
    MenuScreen s2(2, "Pause",   MenuScreenType::PauseMenu);   s2.setPauseGame(true);
    MenuScreen s3(3, "Options", MenuScreenType::OptionsMenu);
    MenuScreen s4(4, "GameOver",MenuScreenType::GameOver);    s4.setPauseGame(true);
    ed.addScreen(s1); ed.addScreen(s2); ed.addScreen(s3); ed.addScreen(s4);
    REQUIRE(ed.countByType(MenuScreenType::MainMenu)    == 1u);
    REQUIRE(ed.countByType(MenuScreenType::PauseMenu)   == 1u);
    REQUIRE(ed.countByType(MenuScreenType::Credits)     == 0u);
    REQUIRE(ed.countPausingGame()                       == 2u);
    auto* found = ed.findScreen(3);
    REQUIRE(found != nullptr);
    REQUIRE(found->type() == MenuScreenType::OptionsMenu);
    REQUIRE(ed.findScreen(99) == nullptr);
}

TEST_CASE("MenuLayoutEditor mutation", "[Editor][S108]") {
    MenuLayoutEditor ed;
    ed.setNavigationMode(MenuNavigationMode::Gamepad);
    ed.setActiveScreenId(2);
    ed.setShowSafeArea(false);
    ed.setShowOverlap(true);
    REQUIRE(ed.navigationMode()  == MenuNavigationMode::Gamepad);
    REQUIRE(ed.activeScreenId()  == 2u);
    REQUIRE(!ed.isShowSafeArea());
    REQUIRE(ed.isShowOverlap());
}
