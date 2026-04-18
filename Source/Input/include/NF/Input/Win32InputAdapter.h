#pragma once
// NF::Win32InputAdapter — Maps Win32 window messages to NF::InputSystem calls.
//
// Usage in the Win32 message pump (Editor main.cpp):
//
//   NF::InputSystem input;
//   NF::Win32InputAdapter adapter(input);
//
//   // In WndProc:
//   if (adapter.processMessage(hwnd, msg, wParam, lParam)) return 0;
//
// This file is only compiled on Windows (_WIN32).

#ifdef _WIN32

#ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#  define NOMINMAX
#endif
#include <windows.h>

#include "NF/Input/Input.h"

namespace NF {

// ── VKey → KeyCode mapping ───────────────────────────────────────

[[nodiscard]] inline KeyCode win32VKeyToKeyCode(WPARAM vk) noexcept {
    // Letters A–Z
    if (vk >= 'A' && vk <= 'Z')
        return static_cast<KeyCode>(static_cast<int>(KeyCode::A) + (vk - 'A'));
    // Digits 0–9
    if (vk >= '0' && vk <= '9')
        return static_cast<KeyCode>(static_cast<int>(KeyCode::Num0) + (vk - '0'));
    // Function keys F1–F12
    if (vk >= VK_F1 && vk <= VK_F12)
        return static_cast<KeyCode>(static_cast<int>(KeyCode::F1) + (vk - VK_F1));

    switch (vk) {
        case VK_LEFT:    return KeyCode::Left;
        case VK_RIGHT:   return KeyCode::Right;
        case VK_UP:      return KeyCode::Up;
        case VK_DOWN:    return KeyCode::Down;
        case VK_SPACE:   return KeyCode::Space;
        case VK_ESCAPE:  return KeyCode::Escape;
        case VK_RETURN:  return KeyCode::Enter;
        case VK_TAB:     return KeyCode::Tab;
        case VK_BACK:    return KeyCode::Backspace;
        case VK_DELETE:  return KeyCode::Delete;
        case VK_LSHIFT:  return KeyCode::LShift;
        case VK_RSHIFT:  return KeyCode::RShift;
        case VK_LCONTROL:return KeyCode::LCtrl;
        case VK_RCONTROL:return KeyCode::RCtrl;
        case VK_LMENU:   return KeyCode::LAlt;
        case VK_RMENU:   return KeyCode::RAlt;
        // Distinguish left/right shift and control from the generic VK codes
        case VK_SHIFT:   return KeyCode::LShift;
        case VK_CONTROL: return KeyCode::LCtrl;
        case VK_MENU:    return KeyCode::LAlt;
        default:         return KeyCode::Unknown;
    }
}

// ── Win32InputAdapter ────────────────────────────────────────────

class Win32InputAdapter {
public:
    explicit Win32InputAdapter(InputSystem& input) : m_input(input) {}

    // Call this from your WndProc before DefWindowProc.
    // Returns true if the message was consumed (caller should return 0).
    bool processMessage(HWND /*hwnd*/, UINT msg, WPARAM wParam, LPARAM lParam) noexcept {
        switch (msg) {
        // ── Keyboard ──────────────────────────────────────────────
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN: {
            const KeyCode key = win32VKeyToKeyCode(wParam);
            if (key != KeyCode::Unknown)
                m_input.setKeyDown(key);
            return false; // don't consume — let DefWindowProc handle system keys
        }
        case WM_KEYUP:
        case WM_SYSKEYUP: {
            const KeyCode key = win32VKeyToKeyCode(wParam);
            if (key != KeyCode::Unknown)
                m_input.setKeyUp(key);
            return false;
        }

        // ── Mouse movement ────────────────────────────────────────
        case WM_MOUSEMOVE: {
            const float x = static_cast<float>(LOWORD(lParam));
            const float y = static_cast<float>(HIWORD(lParam));
            m_input.setMousePosition(x, y);
            return false;
        }

        // ── Mouse buttons ─────────────────────────────────────────
        case WM_LBUTTONDOWN:
            m_input.setKeyDown(KeyCode::Mouse1);
            if (m_hwnd) SetCapture(m_hwnd);
            return false;
        case WM_LBUTTONUP:
            m_input.setKeyUp(KeyCode::Mouse1);
            ReleaseCapture();
            return false;
        case WM_RBUTTONDOWN:
            m_input.setKeyDown(KeyCode::Mouse2);
            return false;
        case WM_RBUTTONUP:
            m_input.setKeyUp(KeyCode::Mouse2);
            return false;
        case WM_MBUTTONDOWN:
            m_input.setKeyDown(KeyCode::Mouse3);
            return false;
        case WM_MBUTTONUP:
            m_input.setKeyUp(KeyCode::Mouse3);
            return false;
        case WM_XBUTTONDOWN: {
            const WORD btn = GET_XBUTTON_WPARAM(wParam);
            m_input.setKeyDown(btn == XBUTTON1 ? KeyCode::Mouse4 : KeyCode::Mouse5);
            return false;
        }
        case WM_XBUTTONUP: {
            const WORD btn = GET_XBUTTON_WPARAM(wParam);
            m_input.setKeyUp(btn == XBUTTON1 ? KeyCode::Mouse4 : KeyCode::Mouse5);
            return false;
        }

        // ── Text input (typed characters) ─────────────────────────
        // WM_CHAR delivers the translated character after TranslateMessage.
        // '\b' (0x08) = Backspace, '\r' (0x0D) = Enter, 0x20-0x7E = printable ASCII.
        case WM_CHAR: {
            const wchar_t wc = static_cast<wchar_t>(wParam);
            if (wc == L'\b' || wc == L'\r') {
                m_input.appendTextInput(static_cast<char>(wc));
            } else if (wc >= 0x20 && wc < 0x7F) {
                m_input.appendTextInput(static_cast<char>(wc));
            }
            return false;
        }

        // ── Scroll wheel ──────────────────────────────────────────
        case WM_MOUSEWHEEL: {
            const float delta = static_cast<float>(GET_WHEEL_DELTA_WPARAM(wParam)) / WHEEL_DELTA;
            m_input.setScrollDelta(delta);
            return false;
        }

        // ── Focus loss — release all keys ─────────────────────────
        case WM_KILLFOCUS:
            resetAllKeys();
            return false;

        default:
            return false;
        }
    }

    // Store HWND for mouse capture (call after window creation)
    void setWindowHandle(HWND hwnd) noexcept { m_hwnd = hwnd; }

    // Release all held keys — call when the window loses focus or is minimized.
    void resetAllKeys() noexcept {
        for (int i = 0; i < static_cast<int>(KeyCode::COUNT); ++i)
            m_input.setKeyUp(static_cast<KeyCode>(i));
    }

private:
    InputSystem& m_input;
    HWND m_hwnd = nullptr;
};

} // namespace NF

#endif // _WIN32
