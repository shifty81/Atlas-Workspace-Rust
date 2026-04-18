// Tests/Workspace/test_phase22_drag_drop.cpp
// Phase 22 — Workspace Drag and Drop System
//
// Tests for:
//   1. DragPayloadType — enum name helpers
//   2. DragPayload     — typed content carrier
//   3. DragSessionState — enum name helpers
//   4. DragSession     — lifecycle state machine
//   5. DropZone        — named zone with type mask
//   6. DragDropManager — session orchestrator with zone registry and observers
//   7. Integration     — full drag-drop pipeline

#include <catch2/catch_test_macros.hpp>
#include "NF/Workspace/WorkspaceDragDrop.h"
#include <string>
#include <vector>

using namespace NF;

// ═════════════════════════════════════════════════════════════════
// SECTION 1 — DragPayloadType enum
// ═════════════════════════════════════════════════════════════════

TEST_CASE("dragPayloadTypeName returns correct strings", "[Phase22][DragPayloadType]") {
    CHECK(std::string(dragPayloadTypeName(DragPayloadType::None))   == "None");
    CHECK(std::string(dragPayloadTypeName(DragPayloadType::Text))   == "Text");
    CHECK(std::string(dragPayloadTypeName(DragPayloadType::Path))   == "Path");
    CHECK(std::string(dragPayloadTypeName(DragPayloadType::Asset))  == "Asset");
    CHECK(std::string(dragPayloadTypeName(DragPayloadType::Entity)) == "Entity");
    CHECK(std::string(dragPayloadTypeName(DragPayloadType::Json))   == "Json");
    CHECK(std::string(dragPayloadTypeName(DragPayloadType::Custom)) == "Custom");
}

// ═════════════════════════════════════════════════════════════════
// SECTION 2 — DragPayload
// ═════════════════════════════════════════════════════════════════

TEST_CASE("DragPayload default is invalid", "[Phase22][DragPayload]") {
    DragPayload p;
    CHECK_FALSE(p.isValid());
    CHECK(p.type == DragPayloadType::None);
    CHECK(p.content.empty());
}

TEST_CASE("DragPayload valid text payload", "[Phase22][DragPayload]") {
    DragPayload p{DragPayloadType::Text, "hello"};
    CHECK(p.isValid());
    CHECK(p.type == DragPayloadType::Text);
    CHECK(p.content == "hello");
}

TEST_CASE("DragPayload None type with content is invalid", "[Phase22][DragPayload]") {
    DragPayload p{DragPayloadType::None, "data"};
    CHECK_FALSE(p.isValid());
}

TEST_CASE("DragPayload valid type with empty content is invalid", "[Phase22][DragPayload]") {
    DragPayload p{DragPayloadType::Asset, ""};
    CHECK_FALSE(p.isValid());
}

TEST_CASE("DragPayload equality", "[Phase22][DragPayload]") {
    DragPayload a{DragPayloadType::Text, "x"};
    DragPayload b{DragPayloadType::Text, "x"};
    DragPayload c{DragPayloadType::Path, "x"};
    CHECK(a == b);
    CHECK(a != c);
}

// ═════════════════════════════════════════════════════════════════
// SECTION 3 — DragSessionState enum
// ═════════════════════════════════════════════════════════════════

TEST_CASE("dragSessionStateName returns correct strings", "[Phase22][DragSessionState]") {
    CHECK(std::string(dragSessionStateName(DragSessionState::Idle))      == "Idle");
    CHECK(std::string(dragSessionStateName(DragSessionState::Active))    == "Active");
    CHECK(std::string(dragSessionStateName(DragSessionState::Hovering))  == "Hovering");
    CHECK(std::string(dragSessionStateName(DragSessionState::Dropped))   == "Dropped");
    CHECK(std::string(dragSessionStateName(DragSessionState::Cancelled)) == "Cancelled");
}

// ═════════════════════════════════════════════════════════════════
// SECTION 4 — DragSession
// ═════════════════════════════════════════════════════════════════

TEST_CASE("DragSession default state is Idle", "[Phase22][DragSession]") {
    DragSession s;
    CHECK(s.state() == DragSessionState::Idle);
    CHECK_FALSE(s.isActive());
    CHECK_FALSE(s.isCompleted());
}

TEST_CASE("DragSession begin transitions to Active", "[Phase22][DragSession]") {
    DragPayload p{DragPayloadType::Text, "data"};
    DragSession s(p, "zone_src");
    CHECK(s.begin());
    CHECK(s.state() == DragSessionState::Active);
    CHECK(s.isActive());
    CHECK(s.sourceZoneId() == "zone_src");
    CHECK(s.payload() == p);
}

TEST_CASE("DragSession begin fails with invalid payload", "[Phase22][DragSession]") {
    DragPayload p;  // invalid
    DragSession s(p);
    CHECK_FALSE(s.begin());
    CHECK(s.state() == DragSessionState::Idle);
}

TEST_CASE("DragSession begin fails if already active", "[Phase22][DragSession]") {
    DragPayload p{DragPayloadType::Asset, "asset_id"};
    DragSession s(p);
    s.begin();
    CHECK_FALSE(s.begin());
}

TEST_CASE("DragSession setHovering transitions to Hovering", "[Phase22][DragSession]") {
    DragPayload p{DragPayloadType::Entity, "ent_1"};
    DragSession s(p);
    s.begin();
    CHECK(s.setHovering("zone_a"));
    CHECK(s.state() == DragSessionState::Hovering);
    CHECK(s.hoverZoneId() == "zone_a");
}

TEST_CASE("DragSession drop from Active", "[Phase22][DragSession]") {
    DragPayload p{DragPayloadType::Json, "{}"};
    DragSession s(p);
    s.begin();
    CHECK(s.drop());
    CHECK(s.state() == DragSessionState::Dropped);
    CHECK(s.isCompleted());
    CHECK_FALSE(s.isActive());
}

TEST_CASE("DragSession drop from Hovering", "[Phase22][DragSession]") {
    DragPayload p{DragPayloadType::Path, "/a/b"};
    DragSession s(p);
    s.begin();
    s.setHovering("z1");
    CHECK(s.drop());
    CHECK(s.state() == DragSessionState::Dropped);
}

TEST_CASE("DragSession cancel from Active", "[Phase22][DragSession]") {
    DragPayload p{DragPayloadType::Custom, "raw"};
    DragSession s(p);
    s.begin();
    CHECK(s.cancel());
    CHECK(s.state() == DragSessionState::Cancelled);
    CHECK(s.isCompleted());
}

TEST_CASE("DragSession cancel from Dropped fails", "[Phase22][DragSession]") {
    DragPayload p{DragPayloadType::Text, "t"};
    DragSession s(p);
    s.begin();
    s.drop();
    CHECK_FALSE(s.cancel());
}

TEST_CASE("DragSession reset returns to Idle", "[Phase22][DragSession]") {
    DragPayload p{DragPayloadType::Text, "t"};
    DragSession s(p);
    s.begin();
    s.reset();
    CHECK(s.state() == DragSessionState::Idle);
    CHECK(s.hoverZoneId().empty());
}

// ═════════════════════════════════════════════════════════════════
// SECTION 5 — DropZone
// ═════════════════════════════════════════════════════════════════

TEST_CASE("DropZone default is invalid", "[Phase22][DropZone]") {
    DropZone z;
    CHECK_FALSE(z.isValid());
    CHECK(z.id().empty());
}

TEST_CASE("DropZone valid construction", "[Phase22][DropZone]") {
    DropZone z("zone_1", "Asset Drop Zone", 0xFF);
    CHECK(z.isValid());
    CHECK(z.id() == "zone_1");
    CHECK(z.label() == "Asset Drop Zone");
    CHECK(z.acceptMask() == 0xFF);
}

TEST_CASE("DropZone accepts checks type mask", "[Phase22][DropZone]") {
    // Accept only Text (bit 1) and Path (bit 2)
    uint8_t mask = (1u << static_cast<uint8_t>(DragPayloadType::Text))
                 | (1u << static_cast<uint8_t>(DragPayloadType::Path));
    DropZone z("z", "Z", mask);
    CHECK(z.accepts(DragPayloadType::Text));
    CHECK(z.accepts(DragPayloadType::Path));
    CHECK_FALSE(z.accepts(DragPayloadType::Asset));
    CHECK_FALSE(z.accepts(DragPayloadType::Entity));
}

TEST_CASE("DropZone tryAccept accepts matching payload", "[Phase22][DropZone]") {
    DropZone z("z", "Z", 0xFF);
    DragPayload p{DragPayloadType::Asset, "asset_x"};
    DragSession s(p);
    s.begin();
    CHECK(z.tryAccept(s));
    CHECK(z.acceptCount() == 1);
    CHECK(z.lastAccepted() == p);
    CHECK(s.hoverZoneId() == "z");
}

TEST_CASE("DropZone tryAccept rejects incompatible type", "[Phase22][DropZone]") {
    // Accept only Text
    uint8_t mask = (1u << static_cast<uint8_t>(DragPayloadType::Text));
    DropZone z("z", "Z", mask);
    DragPayload p{DragPayloadType::Asset, "asset"};
    DragSession s(p);
    s.begin();
    CHECK_FALSE(z.tryAccept(s));
    CHECK(z.acceptCount() == 0);
}

TEST_CASE("DropZone tryAccept rejects inactive session", "[Phase22][DropZone]") {
    DropZone z("z", "Z", 0xFF);
    DragPayload p{DragPayloadType::Text, "t"};
    DragSession s(p);
    // Not started
    CHECK_FALSE(z.tryAccept(s));
}

TEST_CASE("DropZone clear resets accept stats", "[Phase22][DropZone]") {
    DropZone z("z", "Z", 0xFF);
    DragPayload p{DragPayloadType::Text, "x"};
    DragSession s(p);
    s.begin();
    z.tryAccept(s);
    CHECK(z.acceptCount() == 1);
    z.clear();
    CHECK(z.acceptCount() == 0);
    CHECK_FALSE(z.lastAccepted().isValid());
}

// ═════════════════════════════════════════════════════════════════
// SECTION 6 — DragDropManager
// ═════════════════════════════════════════════════════════════════

TEST_CASE("DragDropManager empty state", "[Phase22][DragDropManager]") {
    DragDropManager m;
    CHECK(m.allZoneIds().empty());
    CHECK_FALSE(m.hasActiveSession());
    CHECK(m.dropCount() == 0);
}

TEST_CASE("DragDropManager registerZone", "[Phase22][DragDropManager]") {
    DragDropManager m;
    DropZone z("zone_a", "A", 0xFF);
    CHECK(m.registerZone(z));
    CHECK(m.findZone("zone_a") != nullptr);
    CHECK(m.allZoneIds().size() == 1);
}

TEST_CASE("DragDropManager duplicate zone rejected", "[Phase22][DragDropManager]") {
    DragDropManager m;
    DropZone z("zone_a", "A", 0xFF);
    CHECK(m.registerZone(z));
    CHECK_FALSE(m.registerZone(z));
}

TEST_CASE("DragDropManager invalid zone rejected", "[Phase22][DragDropManager]") {
    DragDropManager m;
    DropZone z;  // invalid (empty id)
    CHECK_FALSE(m.registerZone(z));
}

TEST_CASE("DragDropManager unregisterZone", "[Phase22][DragDropManager]") {
    DragDropManager m;
    DropZone z("zone_a", "A", 0xFF);
    m.registerZone(z);
    CHECK(m.unregisterZone("zone_a"));
    CHECK(m.findZone("zone_a") == nullptr);
}

TEST_CASE("DragDropManager unregister unknown fails", "[Phase22][DragDropManager]") {
    DragDropManager m;
    CHECK_FALSE(m.unregisterZone("nope"));
}

TEST_CASE("DragDropManager beginDrag starts session", "[Phase22][DragDropManager]") {
    DragDropManager m;
    DragPayload p{DragPayloadType::Text, "hello"};
    CHECK(m.beginDrag(p, "src"));
    CHECK(m.hasActiveSession());
    CHECK(m.activeSession().state() == DragSessionState::Active);
}

TEST_CASE("DragDropManager beginDrag fails if already active", "[Phase22][DragDropManager]") {
    DragDropManager m;
    DragPayload p{DragPayloadType::Text, "x"};
    m.beginDrag(p);
    CHECK_FALSE(m.beginDrag(p));
}

TEST_CASE("DragDropManager cancelDrag cancels session", "[Phase22][DragDropManager]") {
    DragDropManager m;
    DragPayload p{DragPayloadType::Text, "t"};
    m.beginDrag(p);
    CHECK(m.cancelDrag());
    CHECK_FALSE(m.hasActiveSession());
}

TEST_CASE("DragDropManager cancelDrag fails with no active session", "[Phase22][DragDropManager]") {
    DragDropManager m;
    CHECK_FALSE(m.cancelDrag());
}

TEST_CASE("DragDropManager commitDrop to accepting zone", "[Phase22][DragDropManager]") {
    DragDropManager m;
    DropZone z("zone_a", "A", 0xFF);
    m.registerZone(z);
    DragPayload p{DragPayloadType::Asset, "asset_1"};
    m.beginDrag(p, "src");
    CHECK(m.commitDrop("zone_a"));
    CHECK(m.dropCount() == 1);
    CHECK_FALSE(m.hasActiveSession());
}

TEST_CASE("DragDropManager commitDrop to unknown zone fails", "[Phase22][DragDropManager]") {
    DragDropManager m;
    DragPayload p{DragPayloadType::Text, "t"};
    m.beginDrag(p);
    CHECK_FALSE(m.commitDrop("no_zone"));
}

TEST_CASE("DragDropManager commitDrop rejects incompatible type", "[Phase22][DragDropManager]") {
    DragDropManager m;
    // Zone accepts only Text
    uint8_t mask = (1u << static_cast<uint8_t>(DragPayloadType::Text));
    DropZone z("zone_a", "A", mask);
    m.registerZone(z);
    DragPayload p{DragPayloadType::Asset, "a"};
    m.beginDrag(p);
    CHECK_FALSE(m.commitDrop("zone_a"));
}

TEST_CASE("DragDropManager observer fires on begin", "[Phase22][DragDropManager]") {
    DragDropManager m;
    int calls = 0;
    m.addObserver([&](const DragSession&) { ++calls; });
    DragPayload p{DragPayloadType::Text, "t"};
    m.beginDrag(p);
    CHECK(calls == 1);
}

TEST_CASE("DragDropManager observer fires on cancel", "[Phase22][DragDropManager]") {
    DragDropManager m;
    int calls = 0;
    m.addObserver([&](const DragSession&) { ++calls; });
    DragPayload p{DragPayloadType::Text, "t"};
    m.beginDrag(p);
    m.cancelDrag();
    CHECK(calls == 2);
}

TEST_CASE("DragDropManager removeObserver stops notifications", "[Phase22][DragDropManager]") {
    DragDropManager m;
    int calls = 0;
    uint32_t id = m.addObserver([&](const DragSession&) { ++calls; });
    m.removeObserver(id);
    DragPayload p{DragPayloadType::Text, "t"};
    m.beginDrag(p);
    CHECK(calls == 0);
}

TEST_CASE("DragDropManager clear resets everything", "[Phase22][DragDropManager]") {
    DragDropManager m;
    DropZone z("z", "Z", 0xFF);
    m.registerZone(z);
    DragPayload p{DragPayloadType::Text, "t"};
    m.beginDrag(p);
    m.clear();
    CHECK_FALSE(m.hasActiveSession());
    CHECK(m.allZoneIds().empty());
    CHECK(m.dropCount() == 0);
}

// ═════════════════════════════════════════════════════════════════
// SECTION 7 — Integration
// ═════════════════════════════════════════════════════════════════

TEST_CASE("Full drag-drop pipeline: begin → hover → drop → observer", "[Phase22][Integration]") {
    DragDropManager m;
    DropZone content("content_browser", "Content Browser", 0xFF);
    DropZone viewport("viewport", "Viewport",
        1u << static_cast<uint8_t>(DragPayloadType::Asset));
    m.registerZone(content);
    m.registerZone(viewport);

    std::string lastEvent;
    m.addObserver([&](const DragSession& s) {
        lastEvent = dragSessionStateName(s.state());
    });

    DragPayload p{DragPayloadType::Asset, "mesh_01"};
    CHECK(m.beginDrag(p, "outliner"));
    CHECK(lastEvent == "Active");

    CHECK(m.commitDrop("viewport"));
    CHECK(lastEvent == "Dropped");
    CHECK(m.dropCount() == 1);
}

TEST_CASE("Multiple zones: only accepting zone gets the drop", "[Phase22][Integration]") {
    DragDropManager m;
    // Zone A accepts Text only
    uint8_t maskA = 1u << static_cast<uint8_t>(DragPayloadType::Text);
    // Zone B accepts Asset only
    uint8_t maskB = 1u << static_cast<uint8_t>(DragPayloadType::Asset);
    m.registerZone(DropZone("zone_a", "A", maskA));
    m.registerZone(DropZone("zone_b", "B", maskB));

    DragPayload p{DragPayloadType::Asset, "asset_x"};
    m.beginDrag(p);
    CHECK_FALSE(m.commitDrop("zone_a"));  // zone_a rejects Asset
    CHECK(m.hasActiveSession());           // still active
    CHECK(m.commitDrop("zone_b"));         // zone_b accepts
    CHECK(m.dropCount() == 1);
}

TEST_CASE("Cancel mid-drag does not increment dropCount", "[Phase22][Integration]") {
    DragDropManager m;
    DragPayload p{DragPayloadType::Text, "t"};
    m.beginDrag(p);
    m.cancelDrag();
    CHECK(m.dropCount() == 0);
}

TEST_CASE("Multiple sequential drags", "[Phase22][Integration]") {
    DragDropManager m;
    m.registerZone(DropZone("z", "Z", 0xFF));

    DragPayload p1{DragPayloadType::Text, "first"};
    DragPayload p2{DragPayloadType::Path, "/some/path"};

    m.beginDrag(p1);
    m.commitDrop("z");
    CHECK(m.dropCount() == 1);

    m.beginDrag(p2);
    m.commitDrop("z");
    CHECK(m.dropCount() == 2);
}

TEST_CASE("allZoneIds reflects registration state", "[Phase22][Integration]") {
    DragDropManager m;
    m.registerZone(DropZone("a", "A", 0xFF));
    m.registerZone(DropZone("b", "B", 0xFF));
    m.registerZone(DropZone("c", "C", 0xFF));
    CHECK(m.allZoneIds().size() == 3);
    m.unregisterZone("b");
    CHECK(m.allZoneIds().size() == 2);
}

TEST_CASE("clearObservers stops all notifications", "[Phase22][Integration]") {
    DragDropManager m;
    int calls = 0;
    m.addObserver([&](const DragSession&) { ++calls; });
    m.clearObservers();
    DragPayload p{DragPayloadType::Text, "t"};
    m.beginDrag(p);
    CHECK(calls == 0);
}
