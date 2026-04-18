#include "../engine/ui/TextRenderer.h"
#include <iostream>
#include <cassert>

using namespace atlas::ui;

void test_null_text_renderer() {
    NullTextRenderer renderer;

    // All operations should be no-ops
    auto handle = renderer.LoadFontAtlas("test.json");
    assert(handle == 0);
    assert(!renderer.IsValidFont(handle));
    assert(renderer.MeasureText(handle, "Hello") == 0.0f);

    // Should not crash
    renderer.BeginFrame();
    renderer.DrawText(handle, "test", 0, 0, {255, 255, 255, 255});
    renderer.DestroyFont(handle);
    renderer.RebuildFontTexture(handle);
    renderer.EndFrame();

    std::cout << "[PASS] test_null_text_renderer" << std::endl;
}

void test_glyph_default() {
    Glyph g;
    assert(g.x == 0.0f);
    assert(g.y == 0.0f);
    assert(g.w == 0.0f);
    assert(g.h == 0.0f);
    assert(g.advance == 0.0f);
    std::cout << "[PASS] test_glyph_default" << std::endl;
}

void test_font_atlas_default() {
    FontAtlas atlas;
    assert(atlas.textureId == 0);
    assert(atlas.fontSize == 0.0f);
    assert(atlas.lineHeight == 0.0f);
    assert(atlas.glyphs.empty());
    std::cout << "[PASS] test_font_atlas_default" << std::endl;
}
