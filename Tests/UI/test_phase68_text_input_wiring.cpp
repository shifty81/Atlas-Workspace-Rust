// Tests/UI/test_phase68_text_input_wiring.cpp
// Phase 68 — TextInput typed-text wiring
//
// Verifies that the fixes from auditxtras.md Patch 4 are correctly wired:
//   1. IInputContext::typedText() is now part of the interface
//   2. BasicInputContext exposes setTypedText() / typedText()
//   3. TextInput::handleInput() consumes typed chars, handles backspace/enter
//   4. onChange callback fires on text mutations
//   5. WorkspaceInputBridge::sync() forwards state.textInput

#include <catch2/catch_test_macros.hpp>
#include "NF/UI/AtlasUI/Widgets/TextInput.h"
#include "NF/UI/AtlasUI/Contexts.h"
#include "NF/Workspace/WorkspaceInputBridge.h"
#include "NF/Input/Input.h"

using namespace NF;
using namespace NF::UI::AtlasUI;

// ── Minimal test input context ────────────────────────────────────

struct TestIn : IInputContext {
    NF::Vec2  pos{};
    bool      primary   = false;
    bool      secondary = false;
    std::string typed;

    NF::Vec2        mousePosition()  const override { return pos;      }
    bool            primaryDown()    const override { return primary;   }
    bool            secondaryDown()  const override { return secondary; }
    bool            keyDown(int)     const override { return false;     }
    std::string_view typedText()     const override { return typed;     }
    void requestFocus(IWidget*)      override {}
    void capturePointer(IWidget*)    override {}
    void releasePointer(IWidget*)    override {}
};

// Helper: click inside a widget at the given coordinates.
static void clickAt(TestIn& ctx, float x, float y) {
    ctx.pos     = {x, y};
    ctx.primary = true;
}
static void release(TestIn& ctx) { ctx.primary = false; }

// ─────────────────────────────────────────────────────────────────
// 1. IInputContext interface — typedText() is available
// ─────────────────────────────────────────────────────────────────

TEST_CASE("IInputContext typedText returns empty by default", "[Phase68][Interface]") {
    TestIn ctx;
    CHECK(ctx.typedText().empty());
}

TEST_CASE("IInputContext typedText returns assigned text", "[Phase68][Interface]") {
    TestIn ctx;
    ctx.typed = "hello";
    CHECK(ctx.typedText() == "hello");
}

// ─────────────────────────────────────────────────────────────────
// 2. BasicInputContext
// ─────────────────────────────────────────────────────────────────

TEST_CASE("BasicInputContext typedText defaults to empty", "[Phase68][BasicInputContext]") {
    BasicInputContext ctx;
    CHECK(ctx.typedText().empty());
}

TEST_CASE("BasicInputContext setTypedText stores and returns text", "[Phase68][BasicInputContext]") {
    BasicInputContext ctx;
    ctx.setTypedText("abc");
    CHECK(ctx.typedText() == "abc");
}

TEST_CASE("BasicInputContext setTypedText with empty clears", "[Phase68][BasicInputContext]") {
    BasicInputContext ctx;
    ctx.setTypedText("abc");
    ctx.setTypedText("");
    CHECK(ctx.typedText().empty());
}

// ─────────────────────────────────────────────────────────────────
// 3. TextInput — click to focus, typed chars appended
// ─────────────────────────────────────────────────────────────────

TEST_CASE("TextInput unfocused ignores typed text", "[Phase68][TextInput]") {
    TextInput ti;
    ti.arrange({0, 0, 200, 28});
    TestIn ctx;
    // No click → not focused
    ctx.typed = "hello";
    ti.handleInput(ctx);
    CHECK(ti.text().empty());
}

TEST_CASE("TextInput gains focus on click inside", "[Phase68][TextInput]") {
    TextInput ti;
    ti.arrange({0, 0, 200, 28});
    TestIn ctx;
    clickAt(ctx, 50, 14);
    bool consumed = ti.handleInput(ctx);
    CHECK(consumed);  // inside bounds
}

TEST_CASE("TextInput appends typed printable chars when focused", "[Phase68][TextInput]") {
    TextInput ti;
    ti.arrange({0, 0, 200, 28});
    TestIn ctx;

    // Click to focus
    clickAt(ctx, 50, 14);
    ti.handleInput(ctx);
    release(ctx);

    // Type characters
    ctx.typed = "Hi";
    ti.handleInput(ctx);
    CHECK(ti.text() == "Hi");
}

TEST_CASE("TextInput appends multiple typed calls sequentially", "[Phase68][TextInput]") {
    TextInput ti;
    ti.arrange({0, 0, 200, 28});
    TestIn ctx;

    clickAt(ctx, 50, 14);
    ti.handleInput(ctx);
    release(ctx);

    ctx.typed = "Fo";
    ti.handleInput(ctx);
    ctx.typed = "o";
    ti.handleInput(ctx);
    CHECK(ti.text() == "Foo");
}

TEST_CASE("TextInput handles backspace removing last char", "[Phase68][TextInput]") {
    TextInput ti;
    ti.arrange({0, 0, 200, 28});
    TestIn ctx;

    clickAt(ctx, 50, 14);
    ti.handleInput(ctx);
    release(ctx);

    ctx.typed = "abc";
    ti.handleInput(ctx);
    CHECK(ti.text() == "abc");

    ctx.typed = std::string(1, '\b');
    ti.handleInput(ctx);
    CHECK(ti.text() == "ab");
}

TEST_CASE("TextInput backspace on empty string is safe", "[Phase68][TextInput]") {
    TextInput ti;
    ti.arrange({0, 0, 200, 28});
    TestIn ctx;

    clickAt(ctx, 50, 14);
    ti.handleInput(ctx);
    release(ctx);

    ctx.typed = std::string(1, '\b');
    ti.handleInput(ctx);  // must not crash
    CHECK(ti.text().empty());
}

TEST_CASE("TextInput enter key loses focus", "[Phase68][TextInput]") {
    TextInput ti;
    ti.arrange({0, 0, 200, 28});
    TestIn ctx;

    clickAt(ctx, 50, 14);
    ti.handleInput(ctx);
    release(ctx);

    ctx.typed = "abc";
    ti.handleInput(ctx);

    // Enter should defocus — subsequent typed text should be ignored
    ctx.typed = "\r";
    ti.handleInput(ctx);

    ctx.typed = "xyz";
    ti.handleInput(ctx);
    CHECK(ti.text() == "abc");  // no more appending after Enter
}

TEST_CASE("TextInput click outside clears focus", "[Phase68][TextInput]") {
    TextInput ti;
    ti.arrange({10, 10, 200, 28});
    TestIn ctx;

    // Click inside to focus
    clickAt(ctx, 50, 20);
    ti.handleInput(ctx);
    release(ctx);

    // Type a char
    ctx.typed = "x";
    ti.handleInput(ctx);
    CHECK(ti.text() == "x");

    // Click outside
    clickAt(ctx, 5, 5);
    ti.handleInput(ctx);
    release(ctx);

    // Further typing should not append
    ctx.typed = "y";
    ti.handleInput(ctx);
    CHECK(ti.text() == "x");
}

TEST_CASE("TextInput setText overrides text and resets cursor", "[Phase68][TextInput]") {
    TextInput ti;
    ti.arrange({0, 0, 200, 28});
    TestIn ctx;

    ti.setText("preset");
    CHECK(ti.text() == "preset");

    // Focus and type — cursor should be at end
    clickAt(ctx, 50, 14);
    ti.handleInput(ctx);
    release(ctx);

    ctx.typed = "!";
    ti.handleInput(ctx);
    CHECK(ti.text() == "preset!");
}

// ─────────────────────────────────────────────────────────────────
// 4. onChange callback
// ─────────────────────────────────────────────────────────────────

TEST_CASE("TextInput onChange fires when text changes via typing", "[Phase68][TextInput]") {
    TextInput ti;
    ti.arrange({0, 0, 200, 28});
    TestIn ctx;

    std::string captured;
    ti.setOnChange([&](const std::string& s) { captured = s; });

    clickAt(ctx, 50, 14);
    ti.handleInput(ctx);
    release(ctx);

    ctx.typed = "a";
    ti.handleInput(ctx);
    CHECK(captured == "a");
}

TEST_CASE("TextInput onChange fires on backspace", "[Phase68][TextInput]") {
    TextInput ti;
    ti.arrange({0, 0, 200, 28});
    TestIn ctx;

    ti.setText("ab");
    std::string captured;
    ti.setOnChange([&](const std::string& s) { captured = s; });

    clickAt(ctx, 50, 14);
    ti.handleInput(ctx);
    release(ctx);

    ctx.typed = std::string(1, '\b');
    ti.handleInput(ctx);
    CHECK(captured == "a");
}

TEST_CASE("TextInput onChange not fired when Enter pressed (focus loss only)", "[Phase68][TextInput]") {
    TextInput ti;
    ti.arrange({0, 0, 200, 28});
    TestIn ctx;

    int calls = 0;
    ti.setOnChange([&](const std::string&) { ++calls; });

    clickAt(ctx, 50, 14);
    ti.handleInput(ctx);
    release(ctx);

    ctx.typed = "\r";
    ti.handleInput(ctx);
    CHECK(calls == 0);  // Enter does not change text — no callback
}

// ─────────────────────────────────────────────────────────────────
// 5. WorkspaceInputBridge forwards textInput
// ─────────────────────────────────────────────────────────────────

TEST_CASE("WorkspaceInputBridge syncs empty textInput", "[Phase68][Bridge]") {
    InputSystem input;
    BasicInputContext ctx;
    NF::WorkspaceInputBridge{}.sync(input, ctx);
    CHECK(ctx.typedText().empty());
}

TEST_CASE("WorkspaceInputBridge syncs typed text from InputSystem", "[Phase68][Bridge]") {
    InputSystem input;
    // Simulate WM_CHAR events appending to textInput
    input.appendTextInput('H');
    input.appendTextInput('i');
    CHECK(input.state().textInput == "Hi");

    BasicInputContext ctx;
    NF::WorkspaceInputBridge{}.sync(input, ctx);
    CHECK(ctx.typedText() == "Hi");
}

TEST_CASE("WorkspaceInputBridge syncs backspace character", "[Phase68][Bridge]") {
    InputSystem input;
    input.appendTextInput('\b');

    BasicInputContext ctx;
    NF::WorkspaceInputBridge{}.sync(input, ctx);
    CHECK(ctx.typedText() == std::string(1, '\b'));
}

TEST_CASE("WorkspaceInputBridge syncs enter character", "[Phase68][Bridge]") {
    InputSystem input;
    input.appendTextInput('\r');

    BasicInputContext ctx;
    NF::WorkspaceInputBridge{}.sync(input, ctx);
    CHECK(ctx.typedText() == std::string(1, '\r'));
}
