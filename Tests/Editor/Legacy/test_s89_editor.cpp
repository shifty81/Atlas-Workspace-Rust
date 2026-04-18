#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/TouchInputMapper.h"
#include "NF/Editor/GamepadConfigurator.h"
#include "NF/Editor/InputBindingsEditor.h"

using namespace NF;

// ── S89: InputBindingsEditor + GamepadConfigurator + TouchInputMapper ─

// ── InputBindingsEditor ──────────────────────────────────────────

TEST_CASE("InputActionType names are correct", "[Editor][S89]") {
    REQUIRE(std::string(inputActionTypeName(InputActionType::Button))  == "Button");
    REQUIRE(std::string(inputActionTypeName(InputActionType::Axis1D))  == "Axis1D");
    REQUIRE(std::string(inputActionTypeName(InputActionType::Axis2D))  == "Axis2D");
    REQUIRE(std::string(inputActionTypeName(InputActionType::Analog))  == "Analog");
    REQUIRE(std::string(inputActionTypeName(InputActionType::Gesture)) == "Gesture");
}

TEST_CASE("InputDevice names are correct", "[Editor][S89]") {
    REQUIRE(std::string(inputDeviceName(InputDevice::Keyboard)) == "Keyboard");
    REQUIRE(std::string(inputDeviceName(InputDevice::Mouse))    == "Mouse");
    REQUIRE(std::string(inputDeviceName(InputDevice::Gamepad))  == "Gamepad");
    REQUIRE(std::string(inputDeviceName(InputDevice::Touch))    == "Touch");
    REQUIRE(std::string(inputDeviceName(InputDevice::Joystick)) == "Joystick");
    REQUIRE(std::string(inputDeviceName(InputDevice::VR))       == "VR");
}

TEST_CASE("InputBindingConflict names are correct", "[Editor][S89]") {
    REQUIRE(std::string(inputBindingConflictName(InputBindingConflict::None))    == "None");
    REQUIRE(std::string(inputBindingConflictName(InputBindingConflict::Warning)) == "Warning");
    REQUIRE(std::string(inputBindingConflictName(InputBindingConflict::Error))   == "Error");
}

TEST_CASE("InputActionBinding stores properties", "[Editor][S89]") {
    InputActionBinding b("Jump", InputActionType::Button, InputDevice::Keyboard, "Space");
    b.setScale(1.5f);
    b.setConflict(InputBindingConflict::Warning);

    REQUIRE(b.actionName() == "Jump");
    REQUIRE(b.type()       == InputActionType::Button);
    REQUIRE(b.device()     == InputDevice::Keyboard);
    REQUIRE(b.key()        == "Space");
    REQUIRE(b.scale()      == 1.5f);
    REQUIRE(b.hasConflict());
    REQUIRE(b.isEnabled());
}

TEST_CASE("InputBindingsEditor add find and remove binding", "[Editor][S89]") {
    InputBindingsEditor editor;
    InputActionBinding b1("Jump",  InputActionType::Button, InputDevice::Keyboard, "Space");
    InputActionBinding b2("Crouch",InputActionType::Button, InputDevice::Keyboard, "LCtrl");
    REQUIRE(editor.addBinding(b1));
    REQUIRE(editor.addBinding(b2));
    REQUIRE(editor.bindingCount()      == 2);
    REQUIRE(editor.findBinding("Jump") != nullptr);
    editor.removeBinding("Jump");
    REQUIRE(editor.bindingCount() == 1);
}

TEST_CASE("InputBindingsEditor rejects duplicate action+device", "[Editor][S89]") {
    InputBindingsEditor editor;
    InputActionBinding b("Fire", InputActionType::Button, InputDevice::Mouse, "LMB");
    editor.addBinding(b);
    REQUIRE_FALSE(editor.addBinding(b));
}

TEST_CASE("InputBindingsEditor countByDevice and countByType", "[Editor][S89]") {
    InputBindingsEditor editor;
    InputActionBinding b1("Move",  InputActionType::Axis2D,  InputDevice::Keyboard, "WASD");
    InputActionBinding b2("Look",  InputActionType::Axis2D,  InputDevice::Mouse,    "Delta");
    InputActionBinding b3("Fire",  InputActionType::Button,  InputDevice::Mouse,    "LMB");
    editor.addBinding(b1); editor.addBinding(b2); editor.addBinding(b3);
    REQUIRE(editor.countByDevice(InputDevice::Mouse)       == 2);
    REQUIRE(editor.countByType(InputActionType::Axis2D)    == 2);
    REQUIRE(editor.countByType(InputActionType::Button)    == 1);
}

TEST_CASE("InputBindingsEditor conflictCount", "[Editor][S89]") {
    InputBindingsEditor editor;
    InputActionBinding b1("A", InputActionType::Button, InputDevice::Keyboard, "A");
    InputActionBinding b2("B", InputActionType::Button, InputDevice::Gamepad,  "A");
    b2.setConflict(InputBindingConflict::Error);
    editor.addBinding(b1); editor.addBinding(b2);
    REQUIRE(editor.conflictCount() == 1);
}

TEST_CASE("InputBindingsEditor MAX_BINDINGS is 512", "[Editor][S89]") {
    REQUIRE(InputBindingsEditor::MAX_BINDINGS == 512);
}

// ── GamepadConfigurator ──────────────────────────────────────────

TEST_CASE("GamepadLayout names are correct", "[Editor][S89]") {
    REQUIRE(std::string(gamepadLayoutName(GamepadLayout::Xbox))        == "Xbox");
    REQUIRE(std::string(gamepadLayoutName(GamepadLayout::PlayStation)) == "PlayStation");
    REQUIRE(std::string(gamepadLayoutName(GamepadLayout::Switch))      == "Switch");
    REQUIRE(std::string(gamepadLayoutName(GamepadLayout::Generic))     == "Generic");
    REQUIRE(std::string(gamepadLayoutName(GamepadLayout::Custom))      == "Custom");
}

TEST_CASE("GamepadButton names are correct", "[Editor][S89]") {
    REQUIRE(std::string(gamepadButtonName(GamepadButton::A))        == "A");
    REQUIRE(std::string(gamepadButtonName(GamepadButton::B))        == "B");
    REQUIRE(std::string(gamepadButtonName(GamepadButton::Start))    == "Start");
    REQUIRE(std::string(gamepadButtonName(GamepadButton::DPadUp))   == "DPadUp");
    REQUIRE(std::string(gamepadButtonName(GamepadButton::DPadLeft)) == "DPadLeft");
}

TEST_CASE("AnalogDeadzone names are correct", "[Editor][S89]") {
    REQUIRE(std::string(analogDeadzoneName(AnalogDeadzone::None))   == "None");
    REQUIRE(std::string(analogDeadzoneName(AnalogDeadzone::Radial)) == "Radial");
    REQUIRE(std::string(analogDeadzoneName(AnalogDeadzone::Axial))  == "Axial");
    REQUIRE(std::string(analogDeadzoneName(AnalogDeadzone::Cross))  == "Cross");
    REQUIRE(std::string(analogDeadzoneName(AnalogDeadzone::Circle)) == "Circle");
}

TEST_CASE("GamepadProfile stores properties", "[Editor][S89]") {
    GamepadProfile profile("Player1", GamepadLayout::Xbox);
    profile.setDeadzone(AnalogDeadzone::Radial);
    profile.setDeadzoneSize(0.15f);
    profile.setVibration(true);
    profile.setLookSensitivity(2.0f);

    REQUIRE(profile.name()             == "Player1");
    REQUIRE(profile.layout()           == GamepadLayout::Xbox);
    REQUIRE(profile.deadzone()         == AnalogDeadzone::Radial);
    REQUIRE(profile.deadzoneSize()     == 0.15f);
    REQUIRE(profile.vibrationEnabled());
    REQUIRE(profile.lookSensitivity()  == 2.0f);
}

TEST_CASE("GamepadConfiguratorPanel add setActive remove", "[Editor][S89]") {
    GamepadConfiguratorPanel panel;
    GamepadProfile p1("P1", GamepadLayout::Xbox);
    GamepadProfile p2("P2", GamepadLayout::PlayStation);
    REQUIRE(panel.addProfile(p1));
    REQUIRE(panel.addProfile(p2));
    REQUIRE(panel.profileCount() == 2);
    REQUIRE(panel.setActiveProfile("P1"));
    REQUIRE(panel.activeProfile() == "P1");
    panel.removeProfile("P1");
    REQUIRE(panel.activeProfile().empty());
}

TEST_CASE("GamepadConfiguratorPanel rejects duplicate name", "[Editor][S89]") {
    GamepadConfiguratorPanel panel;
    GamepadProfile p("DupProfile", GamepadLayout::Generic);
    panel.addProfile(p);
    REQUIRE_FALSE(panel.addProfile(p));
}

TEST_CASE("GamepadConfiguratorPanel countByLayout", "[Editor][S89]") {
    GamepadConfiguratorPanel panel;
    panel.addProfile(GamepadProfile("A", GamepadLayout::Xbox));
    panel.addProfile(GamepadProfile("B", GamepadLayout::Xbox));
    panel.addProfile(GamepadProfile("C", GamepadLayout::PlayStation));
    REQUIRE(panel.countByLayout(GamepadLayout::Xbox)        == 2);
    REQUIRE(panel.countByLayout(GamepadLayout::PlayStation) == 1);
}

TEST_CASE("GamepadConfiguratorPanel MAX_PROFILES is 32", "[Editor][S89]") {
    REQUIRE(GamepadConfiguratorPanel::MAX_PROFILES == 32);
}

// ── TouchInputMapper ─────────────────────────────────────────────

TEST_CASE("TouchGestureType names are correct", "[Editor][S89]") {
    REQUIRE(std::string(touchGestureTypeName(TouchGestureType::Tap))       == "Tap");
    REQUIRE(std::string(touchGestureTypeName(TouchGestureType::DoubleTap)) == "DoubleTap");
    REQUIRE(std::string(touchGestureTypeName(TouchGestureType::LongPress)) == "LongPress");
    REQUIRE(std::string(touchGestureTypeName(TouchGestureType::Swipe))     == "Swipe");
    REQUIRE(std::string(touchGestureTypeName(TouchGestureType::Pinch))     == "Pinch");
    REQUIRE(std::string(touchGestureTypeName(TouchGestureType::Rotate))    == "Rotate");
    REQUIRE(std::string(touchGestureTypeName(TouchGestureType::Pan))       == "Pan");
    REQUIRE(std::string(touchGestureTypeName(TouchGestureType::EdgeSwipe)) == "EdgeSwipe");
}

TEST_CASE("TouchZoneShape names are correct", "[Editor][S89]") {
    REQUIRE(std::string(touchZoneShapeName(TouchZoneShape::FullScreen)) == "FullScreen");
    REQUIRE(std::string(touchZoneShapeName(TouchZoneShape::Rect))       == "Rect");
    REQUIRE(std::string(touchZoneShapeName(TouchZoneShape::Circle))     == "Circle");
    REQUIRE(std::string(touchZoneShapeName(TouchZoneShape::Strip))      == "Strip");
    REQUIRE(std::string(touchZoneShapeName(TouchZoneShape::Corner))     == "Corner");
}

TEST_CASE("TouchFingerCount names are correct", "[Editor][S89]") {
    REQUIRE(std::string(touchFingerCountName(TouchFingerCount::One))   == "One");
    REQUIRE(std::string(touchFingerCountName(TouchFingerCount::Two))   == "Two");
    REQUIRE(std::string(touchFingerCountName(TouchFingerCount::Three)) == "Three");
}

TEST_CASE("TouchBinding stores properties", "[Editor][S89]") {
    TouchBinding b("Jump", TouchGestureType::Tap, TouchZoneShape::Rect);
    b.setFingerCount(TouchFingerCount::One);
    b.setThreshold(0.2f);

    REQUIRE(b.actionName()  == "Jump");
    REQUIRE(b.gesture()     == TouchGestureType::Tap);
    REQUIRE(b.zone()        == TouchZoneShape::Rect);
    REQUIRE(b.fingerCount() == TouchFingerCount::One);
    REQUIRE(b.threshold()   == 0.2f);
    REQUIRE(b.isEnabled());
}

TEST_CASE("TouchInputMapperPanel add find remove", "[Editor][S89]") {
    TouchInputMapperPanel panel;
    TouchBinding b1("Attack", TouchGestureType::Tap,   TouchZoneShape::Rect);
    TouchBinding b2("Zoom",   TouchGestureType::Pinch, TouchZoneShape::FullScreen);
    REQUIRE(panel.addBinding(b1));
    REQUIRE(panel.addBinding(b2));
    REQUIRE(panel.bindingCount()         == 2);
    REQUIRE(panel.findBinding("Attack")  != nullptr);
    panel.removeBinding("Attack");
    REQUIRE(panel.bindingCount() == 1);
}

TEST_CASE("TouchInputMapperPanel rejects duplicate action+gesture", "[Editor][S89]") {
    TouchInputMapperPanel panel;
    TouchBinding b("Run", TouchGestureType::Swipe, TouchZoneShape::Strip);
    panel.addBinding(b);
    REQUIRE_FALSE(panel.addBinding(b));
}

TEST_CASE("TouchInputMapperPanel countByGesture and countByZone", "[Editor][S89]") {
    TouchInputMapperPanel panel;
    panel.addBinding(TouchBinding("A", TouchGestureType::Tap,  TouchZoneShape::Rect));
    panel.addBinding(TouchBinding("B", TouchGestureType::Tap,  TouchZoneShape::Circle));
    panel.addBinding(TouchBinding("C", TouchGestureType::Pan,  TouchZoneShape::Rect));
    REQUIRE(panel.countByGesture(TouchGestureType::Tap)  == 2);
    REQUIRE(panel.countByZone(TouchZoneShape::Rect)      == 2);
}

TEST_CASE("TouchInputMapperPanel MAX_BINDINGS is 128", "[Editor][S89]") {
    REQUIRE(TouchInputMapperPanel::MAX_BINDINGS == 128);
}
