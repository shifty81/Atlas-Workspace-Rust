// Tests/Workspace/test_phase20_clipboard.cpp
// Phase 20 — Workspace Clipboard System
//
// Tests for:
//   1. ClipboardFormat — enum name helpers
//   2. ClipboardEntry — typed clip with format/data/timestamp
//   3. ClipboardBuffer — newest-first ring buffer
//   4. ClipboardChannel — named channel wrapping a buffer
//   5. ClipboardManager — workspace-scoped multi-channel clipboard with observers
//   6. Integration — full clipboard pipeline

#include <catch2/catch_test_macros.hpp>
#include "NF/Workspace/WorkspaceClipboard.h"
#include <string>
#include <vector>

using namespace NF;

// ═════════════════════════════════════════════════════════════════
// SECTION 1 — Enums
// ═════════════════════════════════════════════════════════════════

TEST_CASE("clipboardFormatName returns correct strings", "[Phase20][ClipboardFormat]") {
    CHECK(std::string(clipboardFormatName(ClipboardFormat::None))     == "None");
    CHECK(std::string(clipboardFormatName(ClipboardFormat::Text))     == "Text");
    CHECK(std::string(clipboardFormatName(ClipboardFormat::RichText)) == "RichText");
    CHECK(std::string(clipboardFormatName(ClipboardFormat::Path))     == "Path");
    CHECK(std::string(clipboardFormatName(ClipboardFormat::EntityId)) == "EntityId");
    CHECK(std::string(clipboardFormatName(ClipboardFormat::JsonBlob)) == "JsonBlob");
    CHECK(std::string(clipboardFormatName(ClipboardFormat::Binary))   == "Binary");
    CHECK(std::string(clipboardFormatName(ClipboardFormat::Custom))   == "Custom");
}

// ═════════════════════════════════════════════════════════════════
// SECTION 2 — ClipboardEntry
// ═════════════════════════════════════════════════════════════════

TEST_CASE("ClipboardEntry default is invalid", "[Phase20][ClipboardEntry]") {
    ClipboardEntry e;
    CHECK_FALSE(e.isValid());
    CHECK(e.format == ClipboardFormat::None);
    CHECK(e.data.empty());
    CHECK(e.isEmpty());
}

TEST_CASE("ClipboardEntry valid text entry", "[Phase20][ClipboardEntry]") {
    ClipboardEntry e{ClipboardFormat::Text, "Hello World", 100};
    CHECK(e.isValid());
    CHECK_FALSE(e.isEmpty());
    CHECK(e.format == ClipboardFormat::Text);
    CHECK(e.data == "Hello World");
    CHECK(e.timestamp == 100);
}

TEST_CASE("ClipboardEntry isEmpty for valid but empty data", "[Phase20][ClipboardEntry]") {
    ClipboardEntry e{ClipboardFormat::Text, "", 0};
    CHECK(e.isValid());
    CHECK(e.isEmpty());
}

TEST_CASE("ClipboardEntry equality", "[Phase20][ClipboardEntry]") {
    ClipboardEntry a{ClipboardFormat::Text, "abc", 1};
    ClipboardEntry b{ClipboardFormat::Text, "abc", 2}; // different timestamp, same content
    ClipboardEntry c{ClipboardFormat::Text, "xyz", 1};
    CHECK(a == b);
    CHECK(a != c);
}

// ═════════════════════════════════════════════════════════════════
// SECTION 3 — ClipboardBuffer
// ═════════════════════════════════════════════════════════════════

TEST_CASE("ClipboardBuffer empty state", "[Phase20][ClipboardBuffer]") {
    ClipboardBuffer buf;
    CHECK(buf.count() == 0);
    CHECK(buf.peek() == nullptr);
    CHECK(buf.peekAt(0) == nullptr);
    CHECK_FALSE(buf.pop());
}

TEST_CASE("ClipboardBuffer push and peek", "[Phase20][ClipboardBuffer]") {
    ClipboardBuffer buf;
    ClipboardEntry e{ClipboardFormat::Text, "hello", 0};
    CHECK(buf.push(e));
    CHECK(buf.count() == 1);
    auto* p = buf.peek();
    REQUIRE(p != nullptr);
    CHECK(p->data == "hello");
}

TEST_CASE("ClipboardBuffer push invalid rejected", "[Phase20][ClipboardBuffer]") {
    ClipboardBuffer buf;
    CHECK_FALSE(buf.push({}));
    CHECK(buf.count() == 0);
}

TEST_CASE("ClipboardBuffer pop", "[Phase20][ClipboardBuffer]") {
    ClipboardBuffer buf;
    buf.push({ClipboardFormat::Text, "a", 0});
    buf.push({ClipboardFormat::Text, "b", 0});
    CHECK(buf.count() == 2);
    CHECK(buf.pop());
    CHECK(buf.count() == 1);
    CHECK(buf.peek()->data == "a");
    CHECK(buf.pop());
    CHECK(buf.count() == 0);
    CHECK_FALSE(buf.pop());
}

TEST_CASE("ClipboardBuffer peekAt index", "[Phase20][ClipboardBuffer]") {
    ClipboardBuffer buf;
    buf.push({ClipboardFormat::Text, "first", 0});
    buf.push({ClipboardFormat::Text, "second", 0});
    // second was pushed last, so index 0 = second, index 1 = first
    auto* p0 = buf.peekAt(0);
    auto* p1 = buf.peekAt(1);
    REQUIRE(p0 != nullptr);
    REQUIRE(p1 != nullptr);
    CHECK(p0->data == "second");
    CHECK(p1->data == "first");
    CHECK(buf.peekAt(2) == nullptr);
}

TEST_CASE("ClipboardBuffer count", "[Phase20][ClipboardBuffer]") {
    ClipboardBuffer buf;
    CHECK(buf.count() == 0);
    buf.push({ClipboardFormat::Text, "a", 0});
    CHECK(buf.count() == 1);
    buf.push({ClipboardFormat::Text, "b", 0});
    CHECK(buf.count() == 2);
}

TEST_CASE("ClipboardBuffer capacity respected — oldest dropped", "[Phase20][ClipboardBuffer]") {
    ClipboardBuffer buf(2);
    CHECK(buf.capacity() == 2);
    buf.push({ClipboardFormat::Text, "a", 0});
    buf.push({ClipboardFormat::Text, "b", 0});
    buf.push({ClipboardFormat::Text, "c", 0}); // drops "a"
    CHECK(buf.count() == 2);
    CHECK(buf.peek()->data == "c");
    CHECK(buf.peekAt(1)->data == "b");
}

TEST_CASE("ClipboardBuffer clear", "[Phase20][ClipboardBuffer]") {
    ClipboardBuffer buf;
    buf.push({ClipboardFormat::Text, "a", 0});
    buf.clear();
    CHECK(buf.count() == 0);
    CHECK(buf.peek() == nullptr);
}

TEST_CASE("ClipboardBuffer push after clear", "[Phase20][ClipboardBuffer]") {
    ClipboardBuffer buf;
    buf.push({ClipboardFormat::Text, "a", 0});
    buf.clear();
    buf.push({ClipboardFormat::Text, "b", 0});
    CHECK(buf.count() == 1);
    CHECK(buf.peek()->data == "b");
}

TEST_CASE("ClipboardBuffer multiple pushes newest first", "[Phase20][ClipboardBuffer]") {
    ClipboardBuffer buf;
    for (int i = 0; i < 5; ++i)
        buf.push({ClipboardFormat::Text, std::to_string(i), 0});
    CHECK(buf.count() == 5);
    CHECK(buf.peek()->data == "4"); // last pushed is at front
}

// ═════════════════════════════════════════════════════════════════
// SECTION 4 — ClipboardChannel
// ═════════════════════════════════════════════════════════════════

TEST_CASE("ClipboardChannel default is invalid", "[Phase20][ClipboardChannel]") {
    ClipboardChannel ch;
    CHECK_FALSE(ch.isValid());
    CHECK(ch.name().empty());
    CHECK(ch.count() == 0);
}

TEST_CASE("ClipboardChannel valid construction", "[Phase20][ClipboardChannel]") {
    ClipboardChannel ch("main", 4);
    CHECK(ch.isValid());
    CHECK(ch.name() == "main");
    CHECK(ch.count() == 0);
}

TEST_CASE("ClipboardChannel push and peek", "[Phase20][ClipboardChannel]") {
    ClipboardChannel ch("main");
    ClipboardEntry e{ClipboardFormat::Text, "hello", 0};
    CHECK(ch.push(e));
    auto* p = ch.peek();
    REQUIRE(p != nullptr);
    CHECK(p->data == "hello");
}

TEST_CASE("ClipboardChannel pop", "[Phase20][ClipboardChannel]") {
    ClipboardChannel ch("main");
    ch.push({ClipboardFormat::Text, "a", 0});
    ch.push({ClipboardFormat::Text, "b", 0});
    CHECK(ch.count() == 2);
    CHECK(ch.pop());
    CHECK(ch.count() == 1);
}

TEST_CASE("ClipboardChannel count", "[Phase20][ClipboardChannel]") {
    ClipboardChannel ch("main");
    CHECK(ch.count() == 0);
    ch.push({ClipboardFormat::Text, "x", 0});
    CHECK(ch.count() == 1);
}

// ═════════════════════════════════════════════════════════════════
// SECTION 5 — ClipboardManager
// ═════════════════════════════════════════════════════════════════

TEST_CASE("ClipboardManager empty state", "[Phase20][ClipboardManager]") {
    ClipboardManager mgr;
    CHECK(mgr.allChannels().empty());
    CHECK_FALSE(mgr.isRegistered("main"));
}

TEST_CASE("ClipboardManager registerChannel", "[Phase20][ClipboardManager]") {
    ClipboardManager mgr;
    CHECK(mgr.registerChannel("main"));
    CHECK(mgr.isRegistered("main"));
    CHECK(mgr.allChannels().size() == 1);
}

TEST_CASE("ClipboardManager duplicate registration rejected", "[Phase20][ClipboardManager]") {
    ClipboardManager mgr;
    mgr.registerChannel("main");
    CHECK_FALSE(mgr.registerChannel("main"));
    CHECK(mgr.allChannels().size() == 1);
}

TEST_CASE("ClipboardManager empty name rejected", "[Phase20][ClipboardManager]") {
    ClipboardManager mgr;
    CHECK_FALSE(mgr.registerChannel(""));
    CHECK(mgr.allChannels().empty());
}

TEST_CASE("ClipboardManager unregisterChannel", "[Phase20][ClipboardManager]") {
    ClipboardManager mgr;
    mgr.registerChannel("main");
    CHECK(mgr.unregisterChannel("main"));
    CHECK_FALSE(mgr.isRegistered("main"));
    CHECK_FALSE(mgr.unregisterChannel("nonexistent"));
}

TEST_CASE("ClipboardManager isRegistered", "[Phase20][ClipboardManager]") {
    ClipboardManager mgr;
    CHECK_FALSE(mgr.isRegistered("main"));
    mgr.registerChannel("main");
    CHECK(mgr.isRegistered("main"));
}

TEST_CASE("ClipboardManager findChannel", "[Phase20][ClipboardManager]") {
    ClipboardManager mgr;
    mgr.registerChannel("main");
    auto* ch = mgr.findChannel("main");
    REQUIRE(ch != nullptr);
    CHECK(ch->name() == "main");
    CHECK(mgr.findChannel("other") == nullptr);
}

TEST_CASE("ClipboardManager push and peek", "[Phase20][ClipboardManager]") {
    ClipboardManager mgr;
    mgr.registerChannel("main");
    ClipboardEntry e{ClipboardFormat::Text, "data", 0};
    CHECK(mgr.push("main", e));
    auto* p = mgr.peek("main");
    REQUIRE(p != nullptr);
    CHECK(p->data == "data");
}

TEST_CASE("ClipboardManager pop", "[Phase20][ClipboardManager]") {
    ClipboardManager mgr;
    mgr.registerChannel("main");
    mgr.push("main", {ClipboardFormat::Text, "a", 0});
    CHECK(mgr.pop("main"));
    CHECK(mgr.peek("main") == nullptr);
    CHECK_FALSE(mgr.pop("nonexistent"));
}

TEST_CASE("ClipboardManager copyText", "[Phase20][ClipboardManager]") {
    ClipboardManager mgr;
    mgr.registerChannel("main");
    CHECK(mgr.copyText("main", "Hello"));
    auto* p = mgr.peek("main");
    REQUIRE(p != nullptr);
    CHECK(p->format == ClipboardFormat::Text);
    CHECK(p->data == "Hello");
}

TEST_CASE("ClipboardManager copyPath", "[Phase20][ClipboardManager]") {
    ClipboardManager mgr;
    mgr.registerChannel("main");
    CHECK(mgr.copyPath("main", "assets/texture.png"));
    auto* p = mgr.peek("main");
    REQUIRE(p != nullptr);
    CHECK(p->format == ClipboardFormat::Path);
    CHECK(p->data == "assets/texture.png");
}

TEST_CASE("ClipboardManager copyEntity", "[Phase20][ClipboardManager]") {
    ClipboardManager mgr;
    mgr.registerChannel("main");
    CHECK(mgr.copyEntity("main", "entity_42"));
    auto* p = mgr.peek("main");
    REQUIRE(p != nullptr);
    CHECK(p->format == ClipboardFormat::EntityId);
    CHECK(p->data == "entity_42");
}

TEST_CASE("ClipboardManager copyJson", "[Phase20][ClipboardManager]") {
    ClipboardManager mgr;
    mgr.registerChannel("main");
    CHECK(mgr.copyJson("main", R"({"key":"value"})"));
    auto* p = mgr.peek("main");
    REQUIRE(p != nullptr);
    CHECK(p->format == ClipboardFormat::JsonBlob);
    CHECK(p->data == R"({"key":"value"})");
}

TEST_CASE("ClipboardManager allChannels", "[Phase20][ClipboardManager]") {
    ClipboardManager mgr;
    mgr.registerChannel("ch1");
    mgr.registerChannel("ch2");
    mgr.registerChannel("ch3");
    auto channels = mgr.allChannels();
    CHECK(channels.size() == 3);
}

TEST_CASE("ClipboardManager clear", "[Phase20][ClipboardManager]") {
    ClipboardManager mgr;
    mgr.registerChannel("ch1");
    mgr.registerChannel("ch2");
    mgr.clear();
    CHECK(mgr.allChannels().empty());
}

TEST_CASE("ClipboardManager push to unknown channel fails", "[Phase20][ClipboardManager]") {
    ClipboardManager mgr;
    CHECK_FALSE(mgr.push("nonexistent", {ClipboardFormat::Text, "data", 0}));
}

// ═════════════════════════════════════════════════════════════════
// SECTION 6 — Integration
// ═════════════════════════════════════════════════════════════════

TEST_CASE("Integration: multi-channel isolation", "[Phase20][Integration]") {
    ClipboardManager mgr;
    mgr.registerChannel("text_ch");
    mgr.registerChannel("path_ch");
    mgr.copyText("text_ch", "Hello");
    mgr.copyPath("path_ch", "assets/foo.png");
    CHECK(mgr.peek("text_ch")->format == ClipboardFormat::Text);
    CHECK(mgr.peek("path_ch")->format == ClipboardFormat::Path);
    CHECK(mgr.peek("text_ch")->data == "Hello");
    CHECK(mgr.peek("path_ch")->data == "assets/foo.png");
}

TEST_CASE("Integration: copyText + peek + pop lifecycle", "[Phase20][Integration]") {
    ClipboardManager mgr;
    mgr.registerChannel("main");
    mgr.copyText("main", "item1");
    mgr.copyText("main", "item2");
    CHECK(mgr.peek("main")->data == "item2");
    mgr.pop("main");
    CHECK(mgr.peek("main")->data == "item1");
    mgr.pop("main");
    CHECK(mgr.peek("main") == nullptr);
}

TEST_CASE("Integration: observer notification on push", "[Phase20][Integration]") {
    ClipboardManager mgr;
    mgr.registerChannel("main");
    std::string lastChannel;
    std::string lastData;
    mgr.addObserver([&](const std::string& ch, const ClipboardEntry& e) {
        lastChannel = ch;
        lastData = e.data;
    });
    mgr.copyText("main", "observed text");
    CHECK(lastChannel == "main");
    CHECK(lastData == "observed text");
}

TEST_CASE("Integration: observer not called after removeObserver", "[Phase20][Integration]") {
    ClipboardManager mgr;
    mgr.registerChannel("main");
    int count = 0;
    auto id = mgr.addObserver([&](const std::string&, const ClipboardEntry&) { ++count; });
    mgr.copyText("main", "a");
    int countAfter = count;
    mgr.removeObserver(id);
    mgr.copyText("main", "b");
    CHECK(count == countAfter);
}

TEST_CASE("Integration: copyJson round-trip", "[Phase20][Integration]") {
    ClipboardManager mgr;
    mgr.registerChannel("json_ch");
    std::string json = R"({"name":"Atlas","version":1})";
    mgr.copyJson("json_ch", json);
    auto* p = mgr.peek("json_ch");
    REQUIRE(p != nullptr);
    CHECK(p->format == ClipboardFormat::JsonBlob);
    CHECK(p->data == json);
}

TEST_CASE("Integration: push fills capacity then oldest dropped", "[Phase20][Integration]") {
    ClipboardManager mgr;
    mgr.registerChannel("small", 3);
    mgr.copyText("small", "a");
    mgr.copyText("small", "b");
    mgr.copyText("small", "c");
    mgr.copyText("small", "d"); // drops "a"
    auto* ch = mgr.findChannel("small");
    REQUIRE(ch != nullptr);
    CHECK(ch->count() == 3);
    CHECK(mgr.peek("small")->data == "d");
}

TEST_CASE("Integration: multi-format in same channel", "[Phase20][Integration]") {
    ClipboardManager mgr;
    mgr.registerChannel("mixed");
    mgr.copyText("mixed", "hello");
    mgr.copyPath("mixed", "path/to/file");
    mgr.copyEntity("mixed", "entity_1");
    auto* ch = mgr.findChannel("mixed");
    REQUIRE(ch != nullptr);
    CHECK(ch->count() == 3);
    CHECK(mgr.peek("mixed")->format == ClipboardFormat::EntityId);
}

TEST_CASE("Integration: clearObservers stops all notifications", "[Phase20][Integration]") {
    ClipboardManager mgr;
    mgr.registerChannel("main");
    int count = 0;
    mgr.addObserver([&](const std::string&, const ClipboardEntry&) { ++count; });
    mgr.addObserver([&](const std::string&, const ClipboardEntry&) { ++count; });
    mgr.copyText("main", "a");
    int countAfter = count;
    mgr.clearObservers();
    mgr.copyText("main", "b");
    CHECK(count == countAfter);
}
