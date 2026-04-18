#include "../engine/ui/UIDrawList.h"
#include <iostream>
#include <cassert>

using namespace atlas::ui;

void test_draw_list_empty() {
    UIDrawList list;
    assert(list.CommandCount() == 0);
    assert(list.Commands().empty());
    std::cout << "[PASS] test_draw_list_empty" << std::endl;
}

void test_draw_list_draw_rect() {
    UIDrawList list;
    list.DrawRect({10, 20, 100, 50}, {255, 0, 0, 255});
    assert(list.CommandCount() == 1);
    assert(list.Commands()[0].kind == UIDrawCmd::Kind::Rect);
    assert(list.Commands()[0].rect.x == 10);
    assert(list.Commands()[0].color.r == 255);
    std::cout << "[PASS] test_draw_list_draw_rect" << std::endl;
}

void test_draw_list_draw_text() {
    UIDrawList list;
    list.DrawText({0, 0, 200, 20}, "Hello", {255, 255, 255, 255});
    assert(list.CommandCount() == 1);
    assert(list.Commands()[0].kind == UIDrawCmd::Kind::Text);
    assert(list.Commands()[0].text == "Hello");
    std::cout << "[PASS] test_draw_list_draw_text" << std::endl;
}

void test_draw_list_draw_icon() {
    UIDrawList list;
    list.DrawIcon({0, 0, 32, 32}, 42, {255, 255, 255, 255});
    assert(list.CommandCount() == 1);
    assert(list.Commands()[0].kind == UIDrawCmd::Kind::Icon);
    assert(list.Commands()[0].resourceId == 42);
    std::cout << "[PASS] test_draw_list_draw_icon" << std::endl;
}

void test_draw_list_draw_border() {
    UIDrawList list;
    list.DrawBorder({0, 0, 200, 100}, 2, {128, 128, 128, 255});
    assert(list.CommandCount() == 1);
    assert(list.Commands()[0].kind == UIDrawCmd::Kind::Border);
    assert(list.Commands()[0].thickness == 2);
    std::cout << "[PASS] test_draw_list_draw_border" << std::endl;
}

void test_draw_list_draw_image() {
    UIDrawList list;
    list.DrawImage({0, 0, 256, 256}, 99, {255, 255, 255, 128});
    assert(list.CommandCount() == 1);
    assert(list.Commands()[0].kind == UIDrawCmd::Kind::Image);
    assert(list.Commands()[0].resourceId == 99);
    std::cout << "[PASS] test_draw_list_draw_image" << std::endl;
}

void test_draw_list_multiple_commands() {
    UIDrawList list;
    list.DrawRect({0, 0, 100, 100}, {255, 0, 0, 255});
    list.DrawText({0, 0, 100, 20}, "Test", {255, 255, 255, 255});
    list.DrawBorder({0, 0, 100, 100}, 1, {0, 255, 0, 255});
    assert(list.CommandCount() == 3);
    std::cout << "[PASS] test_draw_list_multiple_commands" << std::endl;
}

void test_draw_list_clear() {
    UIDrawList list;
    list.DrawRect({0, 0, 100, 100}, {255, 0, 0, 255});
    list.DrawText({0, 0, 100, 20}, "Test", {255, 255, 255, 255});
    assert(list.CommandCount() == 2);

    list.Clear();
    assert(list.CommandCount() == 0);
    assert(list.Commands().empty());
    std::cout << "[PASS] test_draw_list_clear" << std::endl;
}

void test_draw_list_flush() {
    UIDrawList list;
    list.DrawRect({0, 0, 100, 100}, {255, 0, 0, 255});
    list.DrawText({0, 0, 100, 20}, "Test", {255, 255, 255, 255});

    NullUIRenderer renderer;
    // Flushing to NullUIRenderer should not crash
    list.Flush(&renderer);
    std::cout << "[PASS] test_draw_list_flush" << std::endl;
}

void test_draw_list_flush_null_renderer() {
    UIDrawList list;
    list.DrawRect({0, 0, 100, 100}, {255, 0, 0, 255});

    // Flushing to null should not crash
    list.Flush(nullptr);
    std::cout << "[PASS] test_draw_list_flush_null_renderer" << std::endl;
}
