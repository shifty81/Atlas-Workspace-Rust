#include "../engine/ui/UIScrollState.h"
#include <iostream>
#include <cassert>
#include <cmath>

using namespace atlas::ui;

void test_scroll_state_defaults() {
    UIScrollState scroll;
    assert(scroll.GetOffset() == 0.0f);
    assert(scroll.GetContentHeight() == 0.0f);
    assert(scroll.GetViewportHeight() == 0.0f);
    assert(scroll.MaxOffset() == 0.0f);
    assert(!scroll.IsScrollable());
    std::cout << "[PASS] test_scroll_state_defaults" << std::endl;
}

void test_scroll_state_content_smaller_than_viewport() {
    UIScrollState scroll;
    scroll.SetContentHeight(100.0f);
    scroll.SetViewportHeight(200.0f);
    assert(!scroll.IsScrollable());
    assert(scroll.MaxOffset() == 0.0f);

    // Scrolling should have no effect
    scroll.Scroll(50.0f);
    assert(scroll.GetOffset() == 0.0f);
    std::cout << "[PASS] test_scroll_state_content_smaller_than_viewport" << std::endl;
}

void test_scroll_state_scrollable() {
    UIScrollState scroll;
    scroll.SetContentHeight(500.0f);
    scroll.SetViewportHeight(200.0f);
    assert(scroll.IsScrollable());
    assert(scroll.MaxOffset() == 300.0f);
    std::cout << "[PASS] test_scroll_state_scrollable" << std::endl;
}

void test_scroll_state_scroll_down() {
    UIScrollState scroll;
    scroll.SetContentHeight(500.0f);
    scroll.SetViewportHeight(200.0f);

    scroll.Scroll(100.0f);
    assert(scroll.GetOffset() == 100.0f);

    scroll.Scroll(100.0f);
    assert(scroll.GetOffset() == 200.0f);
    std::cout << "[PASS] test_scroll_state_scroll_down" << std::endl;
}

void test_scroll_state_scroll_up() {
    UIScrollState scroll;
    scroll.SetContentHeight(500.0f);
    scroll.SetViewportHeight(200.0f);

    scroll.SetOffset(200.0f);
    scroll.Scroll(-50.0f);
    assert(scroll.GetOffset() == 150.0f);
    std::cout << "[PASS] test_scroll_state_scroll_up" << std::endl;
}

void test_scroll_state_clamp_top() {
    UIScrollState scroll;
    scroll.SetContentHeight(500.0f);
    scroll.SetViewportHeight(200.0f);

    scroll.Scroll(-100.0f);
    assert(scroll.GetOffset() == 0.0f);
    std::cout << "[PASS] test_scroll_state_clamp_top" << std::endl;
}

void test_scroll_state_clamp_bottom() {
    UIScrollState scroll;
    scroll.SetContentHeight(500.0f);
    scroll.SetViewportHeight(200.0f);

    scroll.Scroll(1000.0f);
    assert(scroll.GetOffset() == 300.0f);
    std::cout << "[PASS] test_scroll_state_clamp_bottom" << std::endl;
}

void test_scroll_state_scroll_to_top() {
    UIScrollState scroll;
    scroll.SetContentHeight(500.0f);
    scroll.SetViewportHeight(200.0f);

    scroll.SetOffset(150.0f);
    scroll.ScrollToTop();
    assert(scroll.GetOffset() == 0.0f);
    std::cout << "[PASS] test_scroll_state_scroll_to_top" << std::endl;
}

void test_scroll_state_scroll_to_bottom() {
    UIScrollState scroll;
    scroll.SetContentHeight(500.0f);
    scroll.SetViewportHeight(200.0f);

    scroll.ScrollToBottom();
    assert(scroll.GetOffset() == 300.0f);
    std::cout << "[PASS] test_scroll_state_scroll_to_bottom" << std::endl;
}

void test_scroll_state_viewport_resize_clamps() {
    UIScrollState scroll;
    scroll.SetContentHeight(500.0f);
    scroll.SetViewportHeight(200.0f);
    scroll.SetOffset(300.0f);

    // Increase viewport — max offset shrinks, offset gets clamped
    scroll.SetViewportHeight(400.0f);
    assert(scroll.MaxOffset() == 100.0f);
    assert(scroll.GetOffset() == 100.0f);
    std::cout << "[PASS] test_scroll_state_viewport_resize_clamps" << std::endl;
}
