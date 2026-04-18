// S148 editor tests: CommandPaletteV1, HotkeyRegistryV1, GestureRecognizerV1
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"

using namespace NF;
using Catch::Approx;

// ── CommandPaletteV1 ──────────────────────────────────────────────────────────

TEST_CASE("Cpv1ItemType names", "[Editor][S148]") {
    REQUIRE(std::string(cpv1ItemTypeName(Cpv1ItemType::Command)) == "Command");
    REQUIRE(std::string(cpv1ItemTypeName(Cpv1ItemType::File))    == "File");
    REQUIRE(std::string(cpv1ItemTypeName(Cpv1ItemType::Symbol))  == "Symbol");
    REQUIRE(std::string(cpv1ItemTypeName(Cpv1ItemType::Action))  == "Action");
    REQUIRE(std::string(cpv1ItemTypeName(Cpv1ItemType::Snippet)) == "Snippet");
    REQUIRE(std::string(cpv1ItemTypeName(Cpv1ItemType::Setting)) == "Setting");
}

TEST_CASE("Cpv1Item validity", "[Editor][S148]") {
    Cpv1Item item;
    REQUIRE(!item.isValid());
    item.id = 1; item.label = "Build";
    REQUIRE(item.isValid());
}

TEST_CASE("CommandPaletteV1 addItem and itemCount", "[Editor][S148]") {
    CommandPaletteV1 cp;
    REQUIRE(cp.itemCount() == 0);
    Cpv1Item item; item.id = 1; item.label = "Format";
    REQUIRE(cp.addItem(item));
    REQUIRE(cp.itemCount() == 1);
}

TEST_CASE("CommandPaletteV1 reject duplicate id", "[Editor][S148]") {
    CommandPaletteV1 cp;
    Cpv1Item item; item.id = 5; item.label = "Run";
    REQUIRE(cp.addItem(item));
    REQUIRE(!cp.addItem(item));
    REQUIRE(cp.itemCount() == 1);
}

TEST_CASE("CommandPaletteV1 reject invalid item", "[Editor][S148]") {
    CommandPaletteV1 cp;
    Cpv1Item bad;
    REQUIRE(!cp.addItem(bad));
}

TEST_CASE("CommandPaletteV1 removeItem", "[Editor][S148]") {
    CommandPaletteV1 cp;
    Cpv1Item item; item.id = 3; item.label = "Open";
    cp.addItem(item);
    REQUIRE(cp.removeItem(3));
    REQUIRE(cp.itemCount() == 0);
    REQUIRE(!cp.removeItem(3));
}

TEST_CASE("CommandPaletteV1 open and close", "[Editor][S148]") {
    CommandPaletteV1 cp;
    REQUIRE(!cp.isOpen());
    cp.open();
    REQUIRE(cp.isOpen());
    cp.close();
    REQUIRE(!cp.isOpen());
}

TEST_CASE("CommandPaletteV1 open clears query", "[Editor][S148]") {
    CommandPaletteV1 cp;
    Cpv1Item item; item.id = 1; item.label = "test";
    cp.addItem(item);
    cp.open();
    cp.search("test");
    REQUIRE(cp.query() == "test");
    cp.open();
    REQUIRE(cp.query().empty());
}

TEST_CASE("CommandPaletteV1 search by label", "[Editor][S148]") {
    CommandPaletteV1 cp;
    Cpv1Item a; a.id = 1; a.label = "Build Project";
    Cpv1Item b; b.id = 2; b.label = "Run Tests";
    cp.addItem(a); cp.addItem(b);
    auto r = cp.search("Build");
    REQUIRE(r.size() == 1);
    REQUIRE(r[0] == 1);
}

TEST_CASE("CommandPaletteV1 search empty query returns all", "[Editor][S148]") {
    CommandPaletteV1 cp;
    Cpv1Item a; a.id = 1; a.label = "A";
    Cpv1Item b; b.id = 2; b.label = "B";
    cp.addItem(a); cp.addItem(b);
    auto r = cp.search("");
    REQUIRE(r.size() == 2);
}

TEST_CASE("CommandPaletteV1 executeItem fires callback and closes", "[Editor][S148]") {
    CommandPaletteV1 cp;
    Cpv1Item item; item.id = 7; item.label = "Deploy";
    cp.addItem(item);
    cp.open();
    uint32_t fired = 0;
    cp.setOnExecute([&](const Cpv1Item& i){ fired = i.id; });
    REQUIRE(cp.executeItem(7));
    REQUIRE(fired == 7);
    REQUIRE(!cp.isOpen());
}

TEST_CASE("CommandPaletteV1 executeItem unknown id returns false", "[Editor][S148]") {
    CommandPaletteV1 cp;
    REQUIRE(!cp.executeItem(999));
}

TEST_CASE("CommandPaletteV1 findItem", "[Editor][S148]") {
    CommandPaletteV1 cp;
    Cpv1Item item; item.id = 2; item.label = "Reload";
    cp.addItem(item);
    REQUIRE(cp.findItem(2) != nullptr);
    REQUIRE(cp.findItem(99) == nullptr);
}

// ── HotkeyRegistryV1 ─────────────────────────────────────────────────────────

TEST_CASE("HkModifier names", "[Editor][S148]") {
    REQUIRE(std::string(hkModifierName(HkModifier::None))         == "None");
    REQUIRE(std::string(hkModifierName(HkModifier::Ctrl))         == "Ctrl");
    REQUIRE(std::string(hkModifierName(HkModifier::CtrlShift))    == "CtrlShift");
    REQUIRE(std::string(hkModifierName(HkModifier::CtrlShiftAlt)) == "CtrlShiftAlt");
}

TEST_CASE("HkBinding validity and toString", "[Editor][S148]") {
    HkBinding b;
    REQUIRE(!b.isValid());
    b.id = 1; b.actionId = "save"; b.keyCode = "S"; b.modifier = HkModifier::Ctrl;
    REQUIRE(b.isValid());
    REQUIRE(b.toString() == "Ctrl+S");
}

TEST_CASE("HotkeyRegistryV1 addBinding and bindingCount", "[Editor][S148]") {
    HotkeyRegistryV1 reg;
    HkBinding b; b.id = 1; b.actionId = "undo"; b.keyCode = "Z"; b.modifier = HkModifier::Ctrl;
    REQUIRE(reg.addBinding(b));
    REQUIRE(reg.bindingCount() == 1);
}

TEST_CASE("HotkeyRegistryV1 reject duplicate", "[Editor][S148]") {
    HotkeyRegistryV1 reg;
    HkBinding b; b.id = 1; b.actionId = "redo"; b.keyCode = "Y"; b.modifier = HkModifier::Ctrl;
    REQUIRE(reg.addBinding(b));
    REQUIRE(!reg.addBinding(b));
}

TEST_CASE("HotkeyRegistryV1 removeBinding", "[Editor][S148]") {
    HotkeyRegistryV1 reg;
    HkBinding b; b.id = 2; b.actionId = "cut"; b.keyCode = "X"; b.modifier = HkModifier::Ctrl;
    reg.addBinding(b);
    REQUIRE(reg.removeBinding(2));
    REQUIRE(reg.bindingCount() == 0);
    REQUIRE(!reg.removeBinding(2));
}

TEST_CASE("HotkeyRegistryV1 findByKey and findByAction", "[Editor][S148]") {
    HotkeyRegistryV1 reg;
    HkBinding b; b.id = 3; b.actionId = "paste"; b.keyCode = "V"; b.modifier = HkModifier::Ctrl;
    reg.addBinding(b);
    REQUIRE(reg.findByKey("V", HkModifier::Ctrl) != nullptr);
    REQUIRE(reg.findByKey("V", HkModifier::None) == nullptr);
    REQUIRE(reg.findByAction("paste") != nullptr);
    REQUIRE(reg.findByAction("copy") == nullptr);
}

TEST_CASE("HotkeyRegistryV1 dispatch fires callback", "[Editor][S148]") {
    HotkeyRegistryV1 reg;
    HkBinding b; b.id = 4; b.actionId = "save"; b.keyCode = "S"; b.modifier = HkModifier::Ctrl;
    reg.addBinding(b);
    std::string dispatched;
    reg.setOnDispatch([&](const HkBinding& hb){ dispatched = hb.actionId; });
    REQUIRE(reg.dispatch("S", HkModifier::Ctrl));
    REQUIRE(dispatched == "save");
}

TEST_CASE("HotkeyRegistryV1 dispatch unknown returns false", "[Editor][S148]") {
    HotkeyRegistryV1 reg;
    REQUIRE(!reg.dispatch("Q", HkModifier::CtrlAlt));
}

// ── GestureRecognizerV1 ───────────────────────────────────────────────────────

TEST_CASE("GestureEvent validity and isActive", "[Editor][S148]") {
    GestureEvent ev;
    REQUIRE(!ev.isValid());
    ev.id = 1; ev.state = GestureState::Began;
    REQUIRE(ev.isValid());
    REQUIRE(ev.isActive());
    ev.state = GestureState::Ended;
    REQUIRE(!ev.isActive());
}

TEST_CASE("GestureRecognizerV1 addGesture and gestureCount", "[Editor][S148]") {
    GestureRecognizerV1 gr;
    GestureEvent ev; ev.id = 1; ev.type = GestureType::Tap;
    REQUIRE(gr.addGesture(ev));
    REQUIRE(gr.gestureCount() == 1);
}

TEST_CASE("GestureRecognizerV1 reject duplicate gesture", "[Editor][S148]") {
    GestureRecognizerV1 gr;
    GestureEvent ev; ev.id = 2;
    REQUIRE(gr.addGesture(ev));
    REQUIRE(!gr.addGesture(ev));
}

TEST_CASE("GestureRecognizerV1 removeGesture", "[Editor][S148]") {
    GestureRecognizerV1 gr;
    GestureEvent ev; ev.id = 3;
    gr.addGesture(ev);
    REQUIRE(gr.removeGesture(3));
    REQUIRE(gr.gestureCount() == 0);
    REQUIRE(!gr.removeGesture(3));
}

TEST_CASE("GestureRecognizerV1 recognizeGesture adds to history and fires callback", "[Editor][S148]") {
    GestureRecognizerV1 gr;
    uint32_t fired = 0;
    gr.setOnGesture([&](const GestureEvent& e){ fired = e.id; });
    GestureEvent ev; ev.id = 10; ev.type = GestureType::Swipe;
    REQUIRE(gr.recognizeGesture(ev));
    REQUIRE(gr.eventCount() == 1);
    REQUIRE(fired == 10);
}

TEST_CASE("GestureRecognizerV1 clearHistory", "[Editor][S148]") {
    GestureRecognizerV1 gr;
    GestureEvent ev; ev.id = 5;
    gr.recognizeGesture(ev);
    REQUIRE(gr.eventCount() == 1);
    gr.clearHistory();
    REQUIRE(gr.eventCount() == 0);
}
