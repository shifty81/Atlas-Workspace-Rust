// S186 editor tests: GamepadMapperV1, KeyBindingEditorV1, InputProfileEditorV1
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/GamepadMapperV1.h"
#include "NF/Editor/KeyBindingEditorV1.h"
#include "NF/Editor/InputProfileEditorV1.h"

using namespace NF;
using Catch::Approx;

// ── GamepadMapperV1 ──────────────────────────────────────────────────────────

TEST_CASE("Gpmv1Binding validity", "[Editor][S186]") {
    Gpmv1Binding b;
    REQUIRE(!b.isValid());
    b.id = 1; b.action = "Jump";
    REQUIRE(b.isValid());
}

TEST_CASE("GamepadMapperV1 addBinding and bindingCount", "[Editor][S186]") {
    GamepadMapperV1 gm;
    REQUIRE(gm.bindingCount() == 0);
    Gpmv1Binding b; b.id = 1; b.action = "Jump";
    REQUIRE(gm.addBinding(b));
    REQUIRE(gm.bindingCount() == 1);
}

TEST_CASE("GamepadMapperV1 addBinding invalid fails", "[Editor][S186]") {
    GamepadMapperV1 gm;
    REQUIRE(!gm.addBinding(Gpmv1Binding{}));
}

TEST_CASE("GamepadMapperV1 addBinding duplicate fails", "[Editor][S186]") {
    GamepadMapperV1 gm;
    Gpmv1Binding b; b.id = 1; b.action = "A";
    gm.addBinding(b);
    REQUIRE(!gm.addBinding(b));
}

TEST_CASE("GamepadMapperV1 removeBinding", "[Editor][S186]") {
    GamepadMapperV1 gm;
    Gpmv1Binding b; b.id = 2; b.action = "Run";
    gm.addBinding(b);
    REQUIRE(gm.removeBinding(2));
    REQUIRE(gm.bindingCount() == 0);
    REQUIRE(!gm.removeBinding(2));
}

TEST_CASE("GamepadMapperV1 findBinding", "[Editor][S186]") {
    GamepadMapperV1 gm;
    Gpmv1Binding b; b.id = 3; b.action = "Dodge";
    gm.addBinding(b);
    REQUIRE(gm.findBinding(3) != nullptr);
    REQUIRE(gm.findBinding(99) == nullptr);
}

TEST_CASE("GamepadMapperV1 addProfile and profileCount", "[Editor][S186]") {
    GamepadMapperV1 gm;
    Gpmv1Profile p; p.id = 1; p.name = "Default";
    REQUIRE(gm.addProfile(p));
    REQUIRE(gm.profileCount() == 1);
}

TEST_CASE("GamepadMapperV1 removeProfile", "[Editor][S186]") {
    GamepadMapperV1 gm;
    Gpmv1Profile p; p.id = 1; p.name = "Custom";
    gm.addProfile(p);
    REQUIRE(gm.removeProfile(1));
    REQUIRE(gm.profileCount() == 0);
}

TEST_CASE("GamepadMapperV1 boundCount", "[Editor][S186]") {
    GamepadMapperV1 gm;
    Gpmv1Binding b1; b1.id = 1; b1.action = "A"; b1.mode = Gpmv1BindingMode::Press;
    Gpmv1Binding b2; b2.id = 2; b2.action = "B"; b2.mode = Gpmv1BindingMode::None;
    gm.addBinding(b1); gm.addBinding(b2);
    REQUIRE(gm.boundCount() == 1);
}

TEST_CASE("GamepadMapperV1 countByButton", "[Editor][S186]") {
    GamepadMapperV1 gm;
    Gpmv1Binding b1; b1.id = 1; b1.action = "A"; b1.button = Gpmv1ButtonType::FaceA;
    Gpmv1Binding b2; b2.id = 2; b2.action = "B"; b2.button = Gpmv1ButtonType::FaceB;
    Gpmv1Binding b3; b3.id = 3; b3.action = "C"; b3.button = Gpmv1ButtonType::FaceA;
    gm.addBinding(b1); gm.addBinding(b2); gm.addBinding(b3);
    REQUIRE(gm.countByButton(Gpmv1ButtonType::FaceA) == 2);
    REQUIRE(gm.countByButton(Gpmv1ButtonType::FaceB) == 1);
}

TEST_CASE("GamepadMapperV1 countByMode", "[Editor][S186]") {
    GamepadMapperV1 gm;
    Gpmv1Binding b1; b1.id = 1; b1.action = "A"; b1.mode = Gpmv1BindingMode::Hold;
    Gpmv1Binding b2; b2.id = 2; b2.action = "B"; b2.mode = Gpmv1BindingMode::Press;
    gm.addBinding(b1); gm.addBinding(b2);
    REQUIRE(gm.countByMode(Gpmv1BindingMode::Hold) == 1);
    REQUIRE(gm.countByMode(Gpmv1BindingMode::Press) == 1);
}

TEST_CASE("GamepadMapperV1 onChange callback", "[Editor][S186]") {
    GamepadMapperV1 gm;
    uint64_t notified = 0;
    gm.setOnChange([&](uint64_t id) { notified = id; });
    Gpmv1Binding b; b.id = 5; b.action = "Interact";
    gm.addBinding(b);
    REQUIRE(notified == 5);
}

TEST_CASE("Gpmv1Binding isBound when mode is not None", "[Editor][S186]") {
    Gpmv1Binding b; b.id = 1; b.action = "X";
    b.mode = Gpmv1BindingMode::Toggle; REQUIRE(b.isBound());
    b.mode = Gpmv1BindingMode::None;   REQUIRE(!b.isBound());
}

TEST_CASE("gpmv1BindingModeName all values", "[Editor][S186]") {
    REQUIRE(std::string(gpmv1BindingModeName(Gpmv1BindingMode::None))    == "None");
    REQUIRE(std::string(gpmv1BindingModeName(Gpmv1BindingMode::Press))   == "Press");
    REQUIRE(std::string(gpmv1BindingModeName(Gpmv1BindingMode::Hold))    == "Hold");
    REQUIRE(std::string(gpmv1BindingModeName(Gpmv1BindingMode::Release)) == "Release");
    REQUIRE(std::string(gpmv1BindingModeName(Gpmv1BindingMode::Toggle))  == "Toggle");
}

// ── KeyBindingEditorV1 ───────────────────────────────────────────────────────

TEST_CASE("Kbev1KeyBinding validity", "[Editor][S186]") {
    Kbev1KeyBinding kb;
    REQUIRE(!kb.isValid());
    kb.id = 1; kb.action = "MoveForward";
    REQUIRE(kb.isValid());
}

TEST_CASE("KeyBindingEditorV1 addBinding and bindingCount", "[Editor][S186]") {
    KeyBindingEditorV1 kbe;
    REQUIRE(kbe.bindingCount() == 0);
    Kbev1KeyBinding kb; kb.id = 1; kb.action = "Jump";
    REQUIRE(kbe.addBinding(kb));
    REQUIRE(kbe.bindingCount() == 1);
}

TEST_CASE("KeyBindingEditorV1 addBinding invalid fails", "[Editor][S186]") {
    KeyBindingEditorV1 kbe;
    REQUIRE(!kbe.addBinding(Kbev1KeyBinding{}));
}

TEST_CASE("KeyBindingEditorV1 addBinding duplicate fails", "[Editor][S186]") {
    KeyBindingEditorV1 kbe;
    Kbev1KeyBinding kb; kb.id = 1; kb.action = "A";
    kbe.addBinding(kb);
    REQUIRE(!kbe.addBinding(kb));
}

TEST_CASE("KeyBindingEditorV1 removeBinding", "[Editor][S186]") {
    KeyBindingEditorV1 kbe;
    Kbev1KeyBinding kb; kb.id = 2; kb.action = "Run";
    kbe.addBinding(kb);
    REQUIRE(kbe.removeBinding(2));
    REQUIRE(kbe.bindingCount() == 0);
    REQUIRE(!kbe.removeBinding(2));
}

TEST_CASE("KeyBindingEditorV1 findBinding", "[Editor][S186]") {
    KeyBindingEditorV1 kbe;
    Kbev1KeyBinding kb; kb.id = 3; kb.action = "Crouch";
    kbe.addBinding(kb);
    REQUIRE(kbe.findBinding(3) != nullptr);
    REQUIRE(kbe.findBinding(99) == nullptr);
}

TEST_CASE("KeyBindingEditorV1 addBindingSet and bindingSetCount", "[Editor][S186]") {
    KeyBindingEditorV1 kbe;
    Kbev1BindingSet s; s.id = 1; s.name = "Default";
    REQUIRE(kbe.addBindingSet(s));
    REQUIRE(kbe.bindingSetCount() == 1);
}

TEST_CASE("KeyBindingEditorV1 removeBindingSet", "[Editor][S186]") {
    KeyBindingEditorV1 kbe;
    Kbev1BindingSet s; s.id = 1; s.name = "Custom";
    kbe.addBindingSet(s);
    REQUIRE(kbe.removeBindingSet(1));
    REQUIRE(kbe.bindingSetCount() == 0);
}

TEST_CASE("KeyBindingEditorV1 boundCount", "[Editor][S186]") {
    KeyBindingEditorV1 kbe;
    Kbev1KeyBinding kb1; kb1.id = 1; kb1.action = "A"; kb1.state = Kbev1ChordState::Bound;
    Kbev1KeyBinding kb2; kb2.id = 2; kb2.action = "B";
    kbe.addBinding(kb1); kbe.addBinding(kb2);
    REQUIRE(kbe.boundCount() == 1);
}

TEST_CASE("KeyBindingEditorV1 conflictingCount", "[Editor][S186]") {
    KeyBindingEditorV1 kbe;
    Kbev1KeyBinding kb1; kb1.id = 1; kb1.action = "A"; kb1.state = Kbev1ChordState::Conflicting;
    Kbev1KeyBinding kb2; kb2.id = 2; kb2.action = "B"; kb2.state = Kbev1ChordState::Bound;
    kbe.addBinding(kb1); kbe.addBinding(kb2);
    REQUIRE(kbe.conflictingCount() == 1);
}

TEST_CASE("KeyBindingEditorV1 countByCategory", "[Editor][S186]") {
    KeyBindingEditorV1 kbe;
    Kbev1KeyBinding kb1; kb1.id = 1; kb1.action = "A"; kb1.category = Kbev1KeyCategory::Movement;
    Kbev1KeyBinding kb2; kb2.id = 2; kb2.action = "B"; kb2.category = Kbev1KeyCategory::Combat;
    Kbev1KeyBinding kb3; kb3.id = 3; kb3.action = "C"; kb3.category = Kbev1KeyCategory::Movement;
    kbe.addBinding(kb1); kbe.addBinding(kb2); kbe.addBinding(kb3);
    REQUIRE(kbe.countByCategory(Kbev1KeyCategory::Movement) == 2);
    REQUIRE(kbe.countByCategory(Kbev1KeyCategory::Combat) == 1);
}

TEST_CASE("KeyBindingEditorV1 onChange callback", "[Editor][S186]") {
    KeyBindingEditorV1 kbe;
    uint64_t notified = 0;
    kbe.setOnChange([&](uint64_t id) { notified = id; });
    Kbev1KeyBinding kb; kb.id = 7; kb.action = "Sprint";
    kbe.addBinding(kb);
    REQUIRE(notified == 7);
}

TEST_CASE("Kbev1KeyBinding state helpers", "[Editor][S186]") {
    Kbev1KeyBinding kb; kb.id = 1; kb.action = "X";
    kb.state = Kbev1ChordState::Bound;       REQUIRE(kb.isBound());
    kb.state = Kbev1ChordState::Conflicting; REQUIRE(kb.isConflicting());
    kb.state = Kbev1ChordState::Disabled;    REQUIRE(kb.isDisabled());
}

TEST_CASE("kbev1KeyCategoryName all values", "[Editor][S186]") {
    REQUIRE(std::string(kbev1KeyCategoryName(Kbev1KeyCategory::Movement)) == "Movement");
    REQUIRE(std::string(kbev1KeyCategoryName(Kbev1KeyCategory::Combat))   == "Combat");
    REQUIRE(std::string(kbev1KeyCategoryName(Kbev1KeyCategory::UI))       == "UI");
    REQUIRE(std::string(kbev1KeyCategoryName(Kbev1KeyCategory::Camera))   == "Camera");
    REQUIRE(std::string(kbev1KeyCategoryName(Kbev1KeyCategory::Debug))    == "Debug");
    REQUIRE(std::string(kbev1KeyCategoryName(Kbev1KeyCategory::Custom))   == "Custom");
}

// ── InputProfileEditorV1 ─────────────────────────────────────────────────────

TEST_CASE("Ipev1Profile validity", "[Editor][S186]") {
    Ipev1Profile p;
    REQUIRE(!p.isValid());
    p.id = 1; p.name = "KeyboardDefault";
    REQUIRE(p.isValid());
}

TEST_CASE("InputProfileEditorV1 addProfile and profileCount", "[Editor][S186]") {
    InputProfileEditorV1 ipe;
    REQUIRE(ipe.profileCount() == 0);
    Ipev1Profile p; p.id = 1; p.name = "P1";
    REQUIRE(ipe.addProfile(p));
    REQUIRE(ipe.profileCount() == 1);
}

TEST_CASE("InputProfileEditorV1 addProfile invalid fails", "[Editor][S186]") {
    InputProfileEditorV1 ipe;
    REQUIRE(!ipe.addProfile(Ipev1Profile{}));
}

TEST_CASE("InputProfileEditorV1 addProfile duplicate fails", "[Editor][S186]") {
    InputProfileEditorV1 ipe;
    Ipev1Profile p; p.id = 1; p.name = "A";
    ipe.addProfile(p);
    REQUIRE(!ipe.addProfile(p));
}

TEST_CASE("InputProfileEditorV1 removeProfile", "[Editor][S186]") {
    InputProfileEditorV1 ipe;
    Ipev1Profile p; p.id = 2; p.name = "B";
    ipe.addProfile(p);
    REQUIRE(ipe.removeProfile(2));
    REQUIRE(ipe.profileCount() == 0);
    REQUIRE(!ipe.removeProfile(2));
}

TEST_CASE("InputProfileEditorV1 findProfile", "[Editor][S186]") {
    InputProfileEditorV1 ipe;
    Ipev1Profile p; p.id = 3; p.name = "C";
    ipe.addProfile(p);
    REQUIRE(ipe.findProfile(3) != nullptr);
    REQUIRE(ipe.findProfile(99) == nullptr);
}

TEST_CASE("InputProfileEditorV1 addPreset and presetCount", "[Editor][S186]") {
    InputProfileEditorV1 ipe;
    Ipev1Preset pr; pr.id = 1; pr.name = "QWERTY"; pr.profileId = 1;
    REQUIRE(ipe.addPreset(pr));
    REQUIRE(ipe.presetCount() == 1);
}

TEST_CASE("InputProfileEditorV1 removePreset", "[Editor][S186]") {
    InputProfileEditorV1 ipe;
    Ipev1Preset pr; pr.id = 1; pr.name = "AZERTY"; pr.profileId = 1;
    ipe.addPreset(pr);
    REQUIRE(ipe.removePreset(1));
    REQUIRE(ipe.presetCount() == 0);
}

TEST_CASE("InputProfileEditorV1 activeCount", "[Editor][S186]") {
    InputProfileEditorV1 ipe;
    Ipev1Profile p1; p1.id = 1; p1.name = "A"; p1.state = Ipev1ProfileState::Active;
    Ipev1Profile p2; p2.id = 2; p2.name = "B";
    ipe.addProfile(p1); ipe.addProfile(p2);
    REQUIRE(ipe.activeCount() == 1);
}

TEST_CASE("InputProfileEditorV1 lockedCount", "[Editor][S186]") {
    InputProfileEditorV1 ipe;
    Ipev1Profile p1; p1.id = 1; p1.name = "A"; p1.state = Ipev1ProfileState::Locked;
    Ipev1Profile p2; p2.id = 2; p2.name = "B"; p2.state = Ipev1ProfileState::Active;
    ipe.addProfile(p1); ipe.addProfile(p2);
    REQUIRE(ipe.lockedCount() == 1);
}

TEST_CASE("InputProfileEditorV1 countByDevice", "[Editor][S186]") {
    InputProfileEditorV1 ipe;
    Ipev1Profile p1; p1.id = 1; p1.name = "A"; p1.deviceType = Ipev1DeviceType::Gamepad;
    Ipev1Profile p2; p2.id = 2; p2.name = "B"; p2.deviceType = Ipev1DeviceType::Keyboard;
    Ipev1Profile p3; p3.id = 3; p3.name = "C"; p3.deviceType = Ipev1DeviceType::Gamepad;
    ipe.addProfile(p1); ipe.addProfile(p2); ipe.addProfile(p3);
    REQUIRE(ipe.countByDevice(Ipev1DeviceType::Gamepad) == 2);
    REQUIRE(ipe.countByDevice(Ipev1DeviceType::Keyboard) == 1);
}

TEST_CASE("InputProfileEditorV1 presetsForProfile", "[Editor][S186]") {
    InputProfileEditorV1 ipe;
    Ipev1Preset pr1; pr1.id = 1; pr1.name = "A"; pr1.profileId = 10;
    Ipev1Preset pr2; pr2.id = 2; pr2.name = "B"; pr2.profileId = 10;
    Ipev1Preset pr3; pr3.id = 3; pr3.name = "C"; pr3.profileId = 20;
    ipe.addPreset(pr1); ipe.addPreset(pr2); ipe.addPreset(pr3);
    REQUIRE(ipe.presetsForProfile(10) == 2);
    REQUIRE(ipe.presetsForProfile(20) == 1);
}

TEST_CASE("InputProfileEditorV1 onChange callback", "[Editor][S186]") {
    InputProfileEditorV1 ipe;
    uint64_t notified = 0;
    ipe.setOnChange([&](uint64_t id) { notified = id; });
    Ipev1Profile p; p.id = 6; p.name = "F";
    ipe.addProfile(p);
    REQUIRE(notified == 6);
}

TEST_CASE("Ipev1Profile state helpers", "[Editor][S186]") {
    Ipev1Profile p; p.id = 1; p.name = "X";
    p.state = Ipev1ProfileState::Active;     REQUIRE(p.isActive());
    p.state = Ipev1ProfileState::Locked;     REQUIRE(p.isLocked());
    p.state = Ipev1ProfileState::Deprecated; REQUIRE(p.isDeprecated());
}

TEST_CASE("ipev1DeviceTypeName all values", "[Editor][S186]") {
    REQUIRE(std::string(ipev1DeviceTypeName(Ipev1DeviceType::Keyboard)) == "Keyboard");
    REQUIRE(std::string(ipev1DeviceTypeName(Ipev1DeviceType::Mouse))    == "Mouse");
    REQUIRE(std::string(ipev1DeviceTypeName(Ipev1DeviceType::Gamepad))  == "Gamepad");
    REQUIRE(std::string(ipev1DeviceTypeName(Ipev1DeviceType::Touch))    == "Touch");
    REQUIRE(std::string(ipev1DeviceTypeName(Ipev1DeviceType::Joystick)) == "Joystick");
    REQUIRE(std::string(ipev1DeviceTypeName(Ipev1DeviceType::Custom))   == "Custom");
}
