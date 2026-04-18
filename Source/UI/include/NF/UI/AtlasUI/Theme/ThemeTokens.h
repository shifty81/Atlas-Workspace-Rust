#pragma once
#include "NF/UI/AtlasUI/Theme/ThemeTypes.h"

namespace NF::UI::AtlasUI {

struct ThemeColors {
    ColorRGBA WindowBg;
    ColorRGBA PanelBg;
    ColorRGBA PanelBgAlt;
    ColorRGBA Surface;
    ColorRGBA SurfaceRaised;
    ColorRGBA SurfaceOverlay;
    ColorRGBA HeaderBg;
    ColorRGBA PopupBg;
    ColorRGBA TooltipBg;

    ColorRGBA Border;
    ColorRGBA BorderStrong;
    ColorRGBA Separator;
    ColorRGBA FocusRing;

    ColorRGBA TextPrimary;
    ColorRGBA TextSecondary;
    ColorRGBA TextMuted;
    ColorRGBA TextDisabled;
    ColorRGBA TextInverse;
    ColorRGBA TextLink;

    ColorRGBA ButtonBg;
    ColorRGBA ButtonHover;
    ColorRGBA ButtonPressed;
    ColorRGBA ButtonDisabled;
    ColorRGBA ButtonText;
    ColorRGBA ButtonTextDisabled;

    ColorRGBA TabBg;
    ColorRGBA TabHover;
    ColorRGBA TabActive;
    ColorRGBA TabInactive;
    ColorRGBA TabText;
    ColorRGBA TabTextActive;

    ColorRGBA InputBg;
    ColorRGBA InputBorder;
    ColorRGBA InputBorderFocus;
    ColorRGBA SelectionBg;
    ColorRGBA SelectionText;
    ColorRGBA RowHover;

    ColorRGBA Accent;
    ColorRGBA Success;
    ColorRGBA Warning;
    ColorRGBA Error;
    ColorRGBA Info;

    ColorRGBA ToastBg;
    ColorRGBA ToastBorder;
    ColorRGBA NotificationInfo;
    ColorRGBA NotificationWarning;
    ColorRGBA NotificationError;
    ColorRGBA NotificationAtlasAI;
};

struct ThemeSpacing {
    float XXS = 2.0f;
    float XS = 4.0f;
    float SM = 6.0f;
    float MD = 8.0f;
    float LG = 12.0f;
    float XL = 16.0f;
    float XXL = 24.0f;

    float PanelPadding = 8.0f;
    float ControlPaddingX = 10.0f;
    float ControlPaddingY = 6.0f;
    float ToolbarGap = 6.0f;
    float TabGap = 4.0f;
    float TooltipPadding = 8.0f;
    float SectionGap = 12.0f;
    float RowGap = 6.0f;
};

struct ThemeRadii {
    float None = 0.0f;
    float SM = 3.0f;
    float MD = 5.0f;
    float LG = 6.0f;
    float XL = 8.0f;

    float Panel = 6.0f;
    float Button = 5.0f;
    float Tab = 6.0f;
    float Tooltip = 5.0f;
    float Popup = 6.0f;
    float Input = 5.0f;
    float Toast = 6.0f;
};

struct ThemeTypography {
    FontRole Caption;
    FontRole Body;
    FontRole BodyStrong;
    FontRole Subtitle;
    FontRole Title;
    FontRole Mono;
    FontRole Tooltip;
    FontRole Tab;
    FontRole Menu;
};

struct ThemeMotion {
    float HoverFadeMs = 100.0f;
    float PressMs = 80.0f;
    float TabActivateMs = 120.0f;
    float TooltipFadeMs = 90.0f;
    float ToastFadeMs = 140.0f;
};

struct ThemeSizes {
    float TitleBarHeight = 32.0f;
    float TabHeight = 30.0f;
    float ToolbarHeight = 32.0f;
    float ButtonHeight = 28.0f;
    float InputHeight = 28.0f;
    float RowHeight = 24.0f;
    float IconSM = 14.0f;
    float IconMD = 16.0f;
    float IconLG = 20.0f;
    float Scrollbar = 10.0f;
    float Splitter = 4.0f;
};

struct ThemeBorders {
    float Hairline = 1.0f;
    float Standard = 1.0f;
    float Emphasis = 2.0f;
};

struct ThemeLayers {
    int Base = 0;
    int PanelChrome = 100;
    int Popup = 200;
    int Tooltip = 300;
    int Toast = 400;
    int Modal = 500;
    int DebugOverlay = 600;
};

} // namespace NF::UI::AtlasUI
