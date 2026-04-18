// S133 editor tests: InputActionEditor, AxisMappingEditor, ControlSchemeEditor
#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/ControlSchemeEditor.h"
#include "NF/Editor/AxisMappingEditor.h"
#include "NF/Editor/InputActionEditor.h"

using namespace NF;

// ── InputActionEditor ─────────────────────────────────────────────────────────

TEST_CASE("InputActType names", "[Editor][S133]") {
    REQUIRE(std::string(inputActTypeName(InputActType::Button))  == "Button");
    REQUIRE(std::string(inputActTypeName(InputActType::Axis))    == "Axis");
    REQUIRE(std::string(inputActTypeName(InputActType::Gesture)) == "Gesture");
    REQUIRE(std::string(inputActTypeName(InputActType::Touch))   == "Touch");
    REQUIRE(std::string(inputActTypeName(InputActType::Custom))  == "Custom");
}

TEST_CASE("InputActionTrigger names", "[Editor][S133]") {
    REQUIRE(std::string(inputActionTriggerName(InputActionTrigger::Pressed))   == "Pressed");
    REQUIRE(std::string(inputActionTriggerName(InputActionTrigger::Released))  == "Released");
    REQUIRE(std::string(inputActionTriggerName(InputActionTrigger::Held))      == "Held");
    REQUIRE(std::string(inputActionTriggerName(InputActionTrigger::DoubleTap)) == "DoubleTap");
    REQUIRE(std::string(inputActionTriggerName(InputActionTrigger::LongPress)) == "LongPress");
}

TEST_CASE("InputActionDef defaults", "[Editor][S133]") {
    InputActionDef a(1, "jump", InputActType::Button, InputActionTrigger::Pressed);
    REQUIRE(a.id()           == 1u);
    REQUIRE(a.name()         == "jump");
    REQUIRE(a.type()         == InputActType::Button);
    REQUIRE(a.trigger()      == InputActionTrigger::Pressed);
    REQUIRE(a.consumeInput());
    REQUIRE(a.repeatDelay()  == 0.5f);
    REQUIRE(a.isEnabled());
}

TEST_CASE("InputActionDef mutation", "[Editor][S133]") {
    InputActionDef a(2, "move", InputActType::Axis, InputActionTrigger::Held);
    a.setConsumeInput(false);
    a.setRepeatDelay(0.2f);
    a.setIsEnabled(false);
    REQUIRE(!a.consumeInput());
    REQUIRE(a.repeatDelay() == 0.2f);
    REQUIRE(!a.isEnabled());
}

TEST_CASE("InputActionEditor defaults", "[Editor][S133]") {
    InputActionEditor ed;
    REQUIRE(!ed.isShowDisabled());
    REQUIRE(ed.isGroupByType());
    REQUIRE(ed.defaultRepeatDelay() == 0.3f);
    REQUIRE(ed.actionCount()        == 0u);
}

TEST_CASE("InputActionEditor add/remove actions", "[Editor][S133]") {
    InputActionEditor ed;
    REQUIRE(ed.addAction(InputActionDef(1, "jump",  InputActType::Button, InputActionTrigger::Pressed)));
    REQUIRE(ed.addAction(InputActionDef(2, "move",  InputActType::Axis,   InputActionTrigger::Held)));
    REQUIRE(ed.addAction(InputActionDef(3, "swipe", InputActType::Gesture,InputActionTrigger::Released)));
    REQUIRE(!ed.addAction(InputActionDef(1, "jump", InputActType::Button, InputActionTrigger::Pressed)));
    REQUIRE(ed.actionCount() == 3u);
    REQUIRE(ed.removeAction(2));
    REQUIRE(ed.actionCount() == 2u);
    REQUIRE(!ed.removeAction(99));
}

TEST_CASE("InputActionEditor counts and find", "[Editor][S133]") {
    InputActionEditor ed;
    InputActionDef a1(1, "a", InputActType::Button,  InputActionTrigger::Pressed);
    InputActionDef a2(2, "b", InputActType::Button,  InputActionTrigger::Released);
    InputActionDef a3(3, "c", InputActType::Axis,    InputActionTrigger::Held);
    InputActionDef a4(4, "d", InputActType::Touch,   InputActionTrigger::DoubleTap); a4.setIsEnabled(false);
    ed.addAction(a1); ed.addAction(a2); ed.addAction(a3); ed.addAction(a4);
    REQUIRE(ed.countByType(InputActType::Button)              == 2u);
    REQUIRE(ed.countByType(InputActType::Axis)                == 1u);
    REQUIRE(ed.countByType(InputActType::Custom)              == 0u);
    REQUIRE(ed.countByTrigger(InputActionTrigger::Pressed)    == 1u);
    REQUIRE(ed.countByTrigger(InputActionTrigger::Released)   == 1u);
    REQUIRE(ed.countEnabled()                                 == 3u);
    auto* found = ed.findAction(3);
    REQUIRE(found != nullptr);
    REQUIRE(found->type() == InputActType::Axis);
    REQUIRE(ed.findAction(99) == nullptr);
}

TEST_CASE("InputActionEditor settings mutation", "[Editor][S133]") {
    InputActionEditor ed;
    ed.setIsShowDisabled(true);
    ed.setIsGroupByType(false);
    ed.setDefaultRepeatDelay(1.0f);
    REQUIRE(ed.isShowDisabled());
    REQUIRE(!ed.isGroupByType());
    REQUIRE(ed.defaultRepeatDelay() == 1.0f);
}

// ── AxisMappingEditor ─────────────────────────────────────────────────────────

TEST_CASE("AxisMappingSource names", "[Editor][S133]") {
    REQUIRE(std::string(axisMappingSourceName(AxisMappingSource::Gamepad))  == "Gamepad");
    REQUIRE(std::string(axisMappingSourceName(AxisMappingSource::Mouse))    == "Mouse");
    REQUIRE(std::string(axisMappingSourceName(AxisMappingSource::Keyboard)) == "Keyboard");
    REQUIRE(std::string(axisMappingSourceName(AxisMappingSource::Touch))    == "Touch");
    REQUIRE(std::string(axisMappingSourceName(AxisMappingSource::VR))       == "VR");
}

TEST_CASE("AxisMappingMode names", "[Editor][S133]") {
    REQUIRE(std::string(axisMappingModeName(AxisMappingMode::Raw))        == "Raw");
    REQUIRE(std::string(axisMappingModeName(AxisMappingMode::Normalized)) == "Normalized");
    REQUIRE(std::string(axisMappingModeName(AxisMappingMode::Deadzone))   == "Deadzone");
    REQUIRE(std::string(axisMappingModeName(AxisMappingMode::Smooth))     == "Smooth");
    REQUIRE(std::string(axisMappingModeName(AxisMappingMode::Inverted))   == "Inverted");
}

TEST_CASE("AxisMapping defaults", "[Editor][S133]") {
    AxisMapping m(1, "look_x", AxisMappingSource::Mouse, AxisMappingMode::Raw);
    REQUIRE(m.id()       == 1u);
    REQUIRE(m.name()     == "look_x");
    REQUIRE(m.source()   == AxisMappingSource::Mouse);
    REQUIRE(m.mode()     == AxisMappingMode::Raw);
    REQUIRE(m.scale()    == 1.0f);
    REQUIRE(m.deadzone() == 0.1f);
    REQUIRE(m.isEnabled());
}

TEST_CASE("AxisMapping mutation", "[Editor][S133]") {
    AxisMapping m(2, "move_y", AxisMappingSource::Gamepad, AxisMappingMode::Deadzone);
    m.setScale(2.0f);
    m.setDeadzone(0.25f);
    m.setIsEnabled(false);
    REQUIRE(m.scale()    == 2.0f);
    REQUIRE(m.deadzone() == 0.25f);
    REQUIRE(!m.isEnabled());
}

TEST_CASE("AxisMappingEditor defaults", "[Editor][S133]") {
    AxisMappingEditor ed;
    REQUIRE(!ed.isShowDisabled());
    REQUIRE(!ed.isGroupBySource());
    REQUIRE(ed.defaultDeadzone() == 0.15f);
    REQUIRE(ed.mappingCount()    == 0u);
}

TEST_CASE("AxisMappingEditor add/remove mappings", "[Editor][S133]") {
    AxisMappingEditor ed;
    REQUIRE(ed.addMapping(AxisMapping(1, "m_a", AxisMappingSource::Mouse,    AxisMappingMode::Raw)));
    REQUIRE(ed.addMapping(AxisMapping(2, "m_b", AxisMappingSource::Gamepad,  AxisMappingMode::Deadzone)));
    REQUIRE(ed.addMapping(AxisMapping(3, "m_c", AxisMappingSource::Keyboard, AxisMappingMode::Smooth)));
    REQUIRE(!ed.addMapping(AxisMapping(1, "m_a", AxisMappingSource::Mouse,   AxisMappingMode::Raw)));
    REQUIRE(ed.mappingCount() == 3u);
    REQUIRE(ed.removeMapping(2));
    REQUIRE(ed.mappingCount() == 2u);
    REQUIRE(!ed.removeMapping(99));
}

TEST_CASE("AxisMappingEditor counts and find", "[Editor][S133]") {
    AxisMappingEditor ed;
    AxisMapping m1(1, "a", AxisMappingSource::Mouse,   AxisMappingMode::Raw);
    AxisMapping m2(2, "b", AxisMappingSource::Mouse,   AxisMappingMode::Normalized);
    AxisMapping m3(3, "c", AxisMappingSource::Gamepad, AxisMappingMode::Deadzone);
    AxisMapping m4(4, "d", AxisMappingSource::VR,      AxisMappingMode::Smooth); m4.setIsEnabled(false);
    ed.addMapping(m1); ed.addMapping(m2); ed.addMapping(m3); ed.addMapping(m4);
    REQUIRE(ed.countBySource(AxisMappingSource::Mouse)        == 2u);
    REQUIRE(ed.countBySource(AxisMappingSource::Gamepad)      == 1u);
    REQUIRE(ed.countBySource(AxisMappingSource::Keyboard)     == 0u);
    REQUIRE(ed.countByMode(AxisMappingMode::Raw)              == 1u);
    REQUIRE(ed.countByMode(AxisMappingMode::Deadzone)         == 1u);
    REQUIRE(ed.countEnabled()                                 == 3u);
    auto* found = ed.findMapping(3);
    REQUIRE(found != nullptr);
    REQUIRE(found->source() == AxisMappingSource::Gamepad);
    REQUIRE(ed.findMapping(99) == nullptr);
}

TEST_CASE("AxisMappingEditor settings mutation", "[Editor][S133]") {
    AxisMappingEditor ed;
    ed.setIsShowDisabled(true);
    ed.setIsGroupBySource(true);
    ed.setDefaultDeadzone(0.3f);
    REQUIRE(ed.isShowDisabled());
    REQUIRE(ed.isGroupBySource());
    REQUIRE(ed.defaultDeadzone() == 0.3f);
}

// ── ControlSchemeEditor ───────────────────────────────────────────────────────

TEST_CASE("CtrlSchemeDevice names", "[Editor][S133]") {
    REQUIRE(std::string(ctrlSchemeDeviceName(CtrlSchemeDevice::Keyboard)) == "Keyboard");
    REQUIRE(std::string(ctrlSchemeDeviceName(CtrlSchemeDevice::Gamepad))  == "Gamepad");
    REQUIRE(std::string(ctrlSchemeDeviceName(CtrlSchemeDevice::Mobile))   == "Mobile");
    REQUIRE(std::string(ctrlSchemeDeviceName(CtrlSchemeDevice::VR))       == "VR");
    REQUIRE(std::string(ctrlSchemeDeviceName(CtrlSchemeDevice::Custom))   == "Custom");
}

TEST_CASE("CtrlSchemeState names", "[Editor][S133]") {
    REQUIRE(std::string(ctrlSchemeSateName(CtrlSchemeState::Inactive))   == "Inactive");
    REQUIRE(std::string(ctrlSchemeSateName(CtrlSchemeState::Active))     == "Active");
    REQUIRE(std::string(ctrlSchemeSateName(CtrlSchemeState::Conflicted)) == "Conflicted");
    REQUIRE(std::string(ctrlSchemeSateName(CtrlSchemeState::Disabled))   == "Disabled");
}

TEST_CASE("ControlScheme defaults", "[Editor][S133]") {
    ControlScheme s(1, "pc_default", CtrlSchemeDevice::Keyboard, CtrlSchemeState::Inactive);
    REQUIRE(s.id()        == 1u);
    REQUIRE(s.name()      == "pc_default");
    REQUIRE(s.device()    == CtrlSchemeDevice::Keyboard);
    REQUIRE(s.state()     == CtrlSchemeState::Inactive);
    REQUIRE(s.priority()  == 0u);
    REQUIRE(!s.isDefault());
    REQUIRE(s.isEnabled());
}

TEST_CASE("ControlScheme mutation", "[Editor][S133]") {
    ControlScheme s(2, "gamepad", CtrlSchemeDevice::Gamepad, CtrlSchemeState::Active);
    s.setPriority(10u);
    s.setIsDefault(true);
    s.setIsEnabled(false);
    REQUIRE(s.priority()  == 10u);
    REQUIRE(s.isDefault());
    REQUIRE(!s.isEnabled());
}

TEST_CASE("ControlSchemeEditor defaults", "[Editor][S133]") {
    ControlSchemeEditor ed;
    REQUIRE(!ed.isShowDisabled());
    REQUIRE(ed.isGroupByDevice());
    REQUIRE(!ed.allowMultipleActive());
    REQUIRE(ed.schemeCount() == 0u);
}

TEST_CASE("ControlSchemeEditor add/remove schemes", "[Editor][S133]") {
    ControlSchemeEditor ed;
    REQUIRE(ed.addScheme(ControlScheme(1, "s_a", CtrlSchemeDevice::Keyboard, CtrlSchemeState::Active)));
    REQUIRE(ed.addScheme(ControlScheme(2, "s_b", CtrlSchemeDevice::Gamepad,  CtrlSchemeState::Inactive)));
    REQUIRE(ed.addScheme(ControlScheme(3, "s_c", CtrlSchemeDevice::Mobile,   CtrlSchemeState::Disabled)));
    REQUIRE(!ed.addScheme(ControlScheme(1, "s_a", CtrlSchemeDevice::Keyboard, CtrlSchemeState::Active)));
    REQUIRE(ed.schemeCount() == 3u);
    REQUIRE(ed.removeScheme(2));
    REQUIRE(ed.schemeCount() == 2u);
    REQUIRE(!ed.removeScheme(99));
}

TEST_CASE("ControlSchemeEditor counts and find", "[Editor][S133]") {
    ControlSchemeEditor ed;
    ControlScheme s1(1, "a", CtrlSchemeDevice::Keyboard, CtrlSchemeState::Active);
    ControlScheme s2(2, "b", CtrlSchemeDevice::Keyboard, CtrlSchemeState::Inactive);
    ControlScheme s3(3, "c", CtrlSchemeDevice::Gamepad,  CtrlSchemeState::Active);
    ControlScheme s4(4, "d", CtrlSchemeDevice::VR,       CtrlSchemeState::Disabled); s4.setIsEnabled(false);
    ed.addScheme(s1); ed.addScheme(s2); ed.addScheme(s3); ed.addScheme(s4);
    REQUIRE(ed.countByDevice(CtrlSchemeDevice::Keyboard)      == 2u);
    REQUIRE(ed.countByDevice(CtrlSchemeDevice::Gamepad)       == 1u);
    REQUIRE(ed.countByDevice(CtrlSchemeDevice::Mobile)        == 0u);
    REQUIRE(ed.countByState(CtrlSchemeState::Active)          == 2u);
    REQUIRE(ed.countByState(CtrlSchemeState::Inactive)        == 1u);
    REQUIRE(ed.countEnabled()                                 == 3u);
    auto* found = ed.findScheme(3);
    REQUIRE(found != nullptr);
    REQUIRE(found->device() == CtrlSchemeDevice::Gamepad);
    REQUIRE(ed.findScheme(99) == nullptr);
}

TEST_CASE("ControlSchemeEditor settings mutation", "[Editor][S133]") {
    ControlSchemeEditor ed;
    ed.setIsShowDisabled(true);
    ed.setIsGroupByDevice(false);
    ed.setAllowMultipleActive(true);
    REQUIRE(ed.isShowDisabled());
    REQUIRE(!ed.isGroupByDevice());
    REQUIRE(ed.allowMultipleActive());
}
