#pragma once
// NF::Editor — Editor theme
#include "NF/Core/Core.h"
#include "NF/Engine/Engine.h"
#include "NF/Renderer/Renderer.h"
#include "NF/UI/UI.h"
#include "NF/GraphVM/GraphVM.h"
#include "NF/Input/Input.h"
#include "NF/UI/UIWidgets.h"
#include <filesystem>
#include <fstream>
#include <set>
#include <map>
#include <deque>
#include <unordered_map>

namespace NF {

struct EditorTheme {
    // Panel colors
    uint32_t panelBackground     = 0x2B2B2BFF;
    uint32_t panelHeader         = 0x3C3C3CFF;
    uint32_t panelBorder         = 0x555555FF;
    uint32_t panelText           = 0xDCDCDCFF;

    // Selection colors
    uint32_t selectionHighlight  = 0x264F78FF;
    uint32_t selectionBorder     = 0x007ACCFF;
    uint32_t hoverHighlight      = 0x383838FF;

    // Input field colors
    uint32_t inputBackground     = 0x1E1E1EFF;
    uint32_t inputBorder         = 0x474747FF;
    uint32_t inputText           = 0xD4D4D4FF;
    uint32_t inputFocusBorder    = 0x007ACCFF;

    // Button colors
    uint32_t buttonBackground    = 0x3C3C3CFF;
    uint32_t buttonHover         = 0x505050FF;
    uint32_t buttonPressed       = 0x007ACCFF;
    uint32_t buttonText          = 0xCCCCCCFF;
    uint32_t buttonDisabledText  = 0x666666FF;

    // Toolbar
    uint32_t toolbarBackground   = 0x333333FF;
    uint32_t toolbarSeparator    = 0x4A4A4AFF;

    // Status bar
    uint32_t statusBarBackground = 0x007ACCFF;
    uint32_t statusBarText       = 0xFFFFFFFF;

    // Viewport
    uint32_t viewportBackground  = 0x1A1A1AFF;
    uint32_t gridColor           = 0x333333FF;

    // Inspector
    uint32_t propertyLabel       = 0xBBBBBBFF;
    uint32_t propertyValue       = 0xE0E0E0FF;
    uint32_t propertySeparator   = 0x404040FF;
    uint32_t dirtyIndicator      = 0xE8A435FF;

    // Font sizing
    float fontSize               = 14.f;
    float headerFontSize         = 16.f;
    float smallFontSize          = 12.f;

    // Spacing
    float panelPadding           = 8.f;
    float itemSpacing            = 4.f;
    float sectionSpacing         = 12.f;

    static EditorTheme dark() { return {}; }

    static EditorTheme light() {
        EditorTheme t;
        t.panelBackground     = 0xF0F0F0FF;
        t.panelHeader         = 0xE0E0E0FF;
        t.panelBorder         = 0xCCCCCCFF;
        t.panelText           = 0x1E1E1EFF;
        t.selectionHighlight  = 0xCCE8FFFF;
        t.selectionBorder     = 0x0078D4FF;
        t.hoverHighlight      = 0xE5E5E5FF;
        t.inputBackground     = 0xFFFFFFFF;
        t.inputBorder         = 0xCCCCCCFF;
        t.inputText           = 0x1E1E1EFF;
        t.buttonBackground    = 0xE0E0E0FF;
        t.buttonHover         = 0xD0D0D0FF;
        t.buttonText          = 0x1E1E1EFF;
        t.toolbarBackground   = 0xE8E8E8FF;
        t.statusBarBackground = 0x0078D4FF;
        t.viewportBackground  = 0xD4D4D4FF;
        t.gridColor           = 0xBBBBBBFF;
        t.propertyLabel       = 0x444444FF;
        t.propertyValue       = 0x1E1E1EFF;
        return t;
    }

    UITheme toUITheme() const {
        UITheme t;
        t.panelBackground    = panelBackground;
        t.panelHeader        = panelHeader;
        t.panelBorder        = panelBorder;
        t.panelText          = panelText;
        t.selectionHighlight = selectionHighlight;
        t.selectionBorder    = selectionBorder;
        t.hoverHighlight     = hoverHighlight;
        t.inputBackground    = inputBackground;
        t.inputBorder        = inputBorder;
        t.inputText          = inputText;
        t.inputFocusBorder   = inputFocusBorder;
        t.buttonBackground   = buttonBackground;
        t.buttonHover        = buttonHover;
        t.buttonPressed      = buttonPressed;
        t.buttonText         = buttonText;
        t.buttonDisabledText = buttonDisabledText;
        t.toolbarBackground  = toolbarBackground;
        t.toolbarSeparator   = toolbarSeparator;
        t.statusBarBackground= statusBarBackground;
        t.statusBarText      = statusBarText;
        t.viewportBackground = viewportBackground;
        t.gridColor          = gridColor;
        t.propertyLabel      = propertyLabel;
        t.propertyValue      = propertyValue;
        t.propertySeparator  = propertySeparator;
        t.dirtyIndicator     = dirtyIndicator;
        t.fontSize           = fontSize;
        t.headerFontSize     = headerFontSize;
        t.smallFontSize      = smallFontSize;
        t.panelPadding       = panelPadding;
        t.itemSpacing        = itemSpacing;
        t.sectionSpacing     = sectionSpacing;
        return t;
    }
};

// ── DockLayout ───────────────────────────────────────────────────


} // namespace NF
