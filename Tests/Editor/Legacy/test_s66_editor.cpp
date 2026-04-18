#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"

using namespace NF;
using Catch::Approx;

// ── ScrollState ──────────────────────────────────────────────────

TEST_CASE("ScrollState defaults to zero", "[Editor][S66]") {
    ScrollState s;
    REQUIRE(s.offsetX  == 0.f);
    REQUIRE(s.offsetY  == 0.f);
    REQUIRE(s.contentW == 0.f);
    REQUIRE(s.contentH == 0.f);
    REQUIRE(s.viewW    == 0.f);
    REQUIRE(s.viewH    == 0.f);
}

TEST_CASE("ScrollState canScrollY true when content exceeds view", "[Editor][S66]") {
    ScrollState s;
    s.contentH = 500.f;
    s.viewH    = 200.f;
    REQUIRE(s.canScrollY());
    REQUIRE_FALSE(s.canScrollX());
}

TEST_CASE("ScrollState canScrollX true when content exceeds view width", "[Editor][S66]") {
    ScrollState s;
    s.contentW = 800.f;
    s.viewW    = 400.f;
    REQUIRE(s.canScrollX());
}

TEST_CASE("ScrollState maxOffsetY computed correctly", "[Editor][S66]") {
    ScrollState s;
    s.contentH = 500.f;
    s.viewH    = 200.f;
    REQUIRE(s.maxOffsetY() == Approx(300.f));
}

TEST_CASE("ScrollState maxOffsetY is zero when content fits", "[Editor][S66]") {
    ScrollState s;
    s.contentH = 100.f;
    s.viewH    = 200.f;
    REQUIRE(s.maxOffsetY() == 0.f);
}

TEST_CASE("ScrollState clamp keeps offset in range", "[Editor][S66]") {
    ScrollState s;
    s.contentH = 500.f;
    s.viewH    = 200.f;
    s.offsetY  = 400.f;  // exceeds maxOffset (300)
    s.clamp();
    REQUIRE(s.offsetY == Approx(300.f));
}

TEST_CASE("ScrollState clamp fixes negative offset", "[Editor][S66]") {
    ScrollState s;
    s.contentH = 500.f;
    s.viewH    = 200.f;
    s.offsetY  = -50.f;
    s.clamp();
    REQUIRE(s.offsetY == 0.f);
}

TEST_CASE("ScrollState scrollBy moves offset and clamps", "[Editor][S66]") {
    ScrollState s;
    s.contentH = 500.f;
    s.viewH    = 200.f;
    s.scrollBy(0.f, 100.f);
    REQUIRE(s.offsetY == Approx(100.f));
    s.scrollBy(0.f, 300.f);   // would go to 400, max is 300
    REQUIRE(s.offsetY == Approx(300.f));
}

TEST_CASE("ScrollState scrollToTop resets offsetY", "[Editor][S66]") {
    ScrollState s;
    s.contentH = 500.f;
    s.viewH    = 200.f;
    s.offsetY  = 150.f;
    s.scrollToTop();
    REQUIRE(s.offsetY == 0.f);
}

TEST_CASE("ScrollState scrollToBottom sets offsetY to max", "[Editor][S66]") {
    ScrollState s;
    s.contentH = 500.f;
    s.viewH    = 200.f;
    s.scrollToBottom();
    REQUIRE(s.offsetY == Approx(s.maxOffsetY()));
}

TEST_CASE("ScrollState normalizedY is 0 at top", "[Editor][S66]") {
    ScrollState s;
    s.contentH = 500.f;
    s.viewH    = 200.f;
    REQUIRE(s.normalizedY() == Approx(0.f));
}

TEST_CASE("ScrollState normalizedY is 1 at bottom", "[Editor][S66]") {
    ScrollState s;
    s.contentH = 500.f;
    s.viewH    = 200.f;
    s.scrollToBottom();
    REQUIRE(s.normalizedY() == Approx(1.f));
}

TEST_CASE("ScrollState normalizedY is 0 when no scroll possible", "[Editor][S66]") {
    ScrollState s;
    s.contentH = 100.f;
    s.viewH    = 200.f;
    REQUIRE(s.normalizedY() == Approx(0.f));
}

// ── VirtualList ──────────────────────────────────────────────────

TEST_CASE("VirtualList starts empty", "[Editor][S66]") {
    VirtualList vl;
    REQUIRE(vl.itemCount() == 0);
    REQUIRE(vl.visibleItemCount() == 0);
}

TEST_CASE("VirtualList setItemCount and itemCount", "[Editor][S66]") {
    VirtualList vl;
    vl.setItemCount(100);
    REQUIRE(vl.itemCount() == 100);
}

TEST_CASE("VirtualList content height scales with item count", "[Editor][S66]") {
    VirtualList vl(20.f);
    vl.setItemCount(50);
    REQUIRE(vl.scroll().contentH == Approx(1000.f));
}

TEST_CASE("VirtualList firstVisibleIndex is 0 before scrolling", "[Editor][S66]") {
    VirtualList vl(20.f);
    vl.setItemCount(100);
    vl.setViewHeight(200.f);
    REQUIRE(vl.firstVisibleIndex() == 0);
}

TEST_CASE("VirtualList firstVisibleIndex updates after scroll", "[Editor][S66]") {
    VirtualList vl(20.f);
    vl.setItemCount(100);
    vl.setViewHeight(200.f);
    vl.scroll().scrollBy(0.f, 100.f);  // scroll down by 100 = 5 items
    REQUIRE(vl.firstVisibleIndex() == 5);
}

TEST_CASE("VirtualList visibleItemCount covers view plus buffer", "[Editor][S66]") {
    VirtualList vl(20.f);
    vl.setItemCount(100);
    vl.setViewHeight(200.f);   // exactly 10 items + 2 buffer = 12
    size_t visible = vl.visibleItemCount();
    REQUIRE(visible >= 10);
    REQUIRE(visible <= 12);
}

TEST_CASE("VirtualList buildVisibleItems returns correct count", "[Editor][S66]") {
    VirtualList vl(20.f);
    vl.setItemCount(50);
    vl.setViewHeight(200.f);
    auto items = vl.buildVisibleItems();
    REQUIRE(items.size() == vl.visibleItemCount());
}

TEST_CASE("VirtualList buildVisibleItems indices are sequential", "[Editor][S66]") {
    VirtualList vl(20.f);
    vl.setItemCount(50);
    vl.setViewHeight(100.f);
    auto items = vl.buildVisibleItems();
    for (size_t i = 0; i < items.size(); ++i)
        REQUIRE(items[i].index == i);
}

TEST_CASE("VirtualList ensureVisible scrolls item into view from below", "[Editor][S66]") {
    VirtualList vl(20.f);
    vl.setItemCount(100);
    vl.setViewHeight(200.f);
    vl.ensureVisible(50);
    // item 50 is at y=1000; with viewH=200, offset should be 1000+20-200=820
    REQUIRE(vl.scroll().offsetY > 0.f);
    size_t first = vl.firstVisibleIndex();
    size_t last  = first + vl.visibleItemCount();
    REQUIRE(50 >= first);
    REQUIRE(50 < last);
}

TEST_CASE("VirtualList ensureVisible no-ops for already-visible item", "[Editor][S66]") {
    VirtualList vl(20.f);
    vl.setItemCount(100);
    vl.setViewHeight(200.f);
    float before = vl.scroll().offsetY;
    vl.ensureVisible(0);
    REQUIRE(vl.scroll().offsetY == before);
}

TEST_CASE("VirtualList itemHeight is adjustable", "[Editor][S66]") {
    VirtualList vl;
    vl.setItemHeight(30.f);
    REQUIRE(vl.itemHeight() == 30.f);
}

TEST_CASE("VirtualList setItemHeight rejects zero/negative", "[Editor][S66]") {
    VirtualList vl(20.f);
    vl.setItemHeight(0.f);
    REQUIRE(vl.itemHeight() == VirtualList::DEFAULT_ITEM_HEIGHT);
}

// ── ScrollableContainer ──────────────────────────────────────────

TEST_CASE("ScrollableContainer starts at zero offset", "[Editor][S66]") {
    ScrollableContainer sc;
    REQUIRE(sc.state().offsetY == 0.f);
    REQUIRE(sc.state().offsetX == 0.f);
}

TEST_CASE("ScrollableContainer default axis is Vertical", "[Editor][S66]") {
    ScrollableContainer sc;
    REQUIRE(sc.axis() == ScrollAxis::Vertical);
}

TEST_CASE("ScrollableContainer Vertical axis ignores horizontal scroll", "[Editor][S66]") {
    ScrollableContainer sc(ScrollAxis::Vertical);
    sc.setContentSize(2000.f, 2000.f);
    sc.setViewSize(400.f, 400.f);
    sc.scroll(200.f, 100.f);
    REQUIRE(sc.state().offsetX == 0.f);   // not scrolled
    REQUIRE(sc.state().offsetY == Approx(100.f));
}

TEST_CASE("ScrollableContainer Horizontal axis ignores vertical scroll", "[Editor][S66]") {
    ScrollableContainer sc(ScrollAxis::Horizontal);
    sc.setContentSize(2000.f, 2000.f);
    sc.setViewSize(400.f, 400.f);
    sc.scroll(100.f, 200.f);
    REQUIRE(sc.state().offsetY == 0.f);
    REQUIRE(sc.state().offsetX == Approx(100.f));
}

TEST_CASE("ScrollableContainer Both axis scrolls both directions", "[Editor][S66]") {
    ScrollableContainer sc(ScrollAxis::Both);
    sc.setContentSize(2000.f, 2000.f);
    sc.setViewSize(400.f, 400.f);
    sc.scroll(100.f, 150.f);
    REQUIRE(sc.state().offsetX == Approx(100.f));
    REQUIRE(sc.state().offsetY == Approx(150.f));
}

TEST_CASE("ScrollableContainer needsScrollbarY and needsScrollbarX", "[Editor][S66]") {
    ScrollableContainer sc(ScrollAxis::Both);
    sc.setContentSize(800.f, 600.f);
    sc.setViewSize(400.f, 400.f);
    REQUIRE(sc.needsScrollbarX());
    REQUIRE(sc.needsScrollbarY());
}

TEST_CASE("ScrollableContainer no scrollbars needed when content fits", "[Editor][S66]") {
    ScrollableContainer sc(ScrollAxis::Both);
    sc.setContentSize(200.f, 200.f);
    sc.setViewSize(400.f, 400.f);
    REQUIRE_FALSE(sc.needsScrollbarX());
    REQUIRE_FALSE(sc.needsScrollbarY());
}
