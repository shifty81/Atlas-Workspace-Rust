#include "NF/UI/AtlasUI/Theme/ThemeDefaults.h"

namespace NF::UI::AtlasUI {

static constexpr ColorRGBA C(float r, float g, float b, float a = 1.0f) {
    return ColorRGBA{r, g, b, a};
}

AtlasTheme MakeAtlasDarkTheme() {
    AtlasTheme theme{};

    theme.Colors.WindowBg       = C(0.10f, 0.11f, 0.13f);
    theme.Colors.PanelBg        = C(0.14f, 0.15f, 0.17f);
    theme.Colors.PanelBgAlt     = C(0.12f, 0.13f, 0.15f);
    theme.Colors.Surface        = C(0.16f, 0.17f, 0.19f);
    theme.Colors.SurfaceRaised  = C(0.20f, 0.21f, 0.24f);
    theme.Colors.SurfaceOverlay = C(0.08f, 0.09f, 0.10f, 0.85f);
    theme.Colors.HeaderBg       = C(0.12f, 0.13f, 0.16f);
    theme.Colors.PopupBg        = C(0.18f, 0.19f, 0.22f);
    theme.Colors.TooltipBg      = C(0.10f, 0.10f, 0.12f, 0.95f);

    theme.Colors.Border         = C(0.22f, 0.23f, 0.26f);
    theme.Colors.BorderStrong   = C(0.35f, 0.36f, 0.40f);
    theme.Colors.Separator      = C(0.20f, 0.21f, 0.23f);
    theme.Colors.FocusRing      = C(0.30f, 0.55f, 0.85f, 0.80f);

    theme.Colors.TextPrimary    = C(0.90f, 0.92f, 0.95f);
    theme.Colors.TextSecondary  = C(0.65f, 0.68f, 0.72f);
    theme.Colors.TextMuted      = C(0.45f, 0.48f, 0.52f);
    theme.Colors.TextDisabled   = C(0.35f, 0.37f, 0.40f);
    theme.Colors.TextInverse    = C(0.08f, 0.09f, 0.11f);
    theme.Colors.TextLink       = C(0.44f, 0.73f, 1.0f);

    theme.Colors.ButtonBg           = C(0.18f, 0.20f, 0.23f);
    theme.Colors.ButtonHover        = C(0.23f, 0.25f, 0.29f);
    theme.Colors.ButtonPressed      = C(0.15f, 0.17f, 0.20f);
    theme.Colors.ButtonDisabled     = C(0.14f, 0.15f, 0.17f);
    theme.Colors.ButtonText         = theme.Colors.TextPrimary;
    theme.Colors.ButtonTextDisabled = theme.Colors.TextDisabled;

    theme.Colors.TabBg         = C(0.14f, 0.15f, 0.18f);
    theme.Colors.TabHover      = C(0.19f, 0.21f, 0.25f);
    theme.Colors.TabActive     = C(0.22f, 0.24f, 0.29f);
    theme.Colors.TabInactive   = C(0.14f, 0.15f, 0.18f);
    theme.Colors.TabText       = theme.Colors.TextSecondary;
    theme.Colors.TabTextActive = theme.Colors.TextPrimary;

    theme.Colors.InputBg          = C(0.10f, 0.11f, 0.13f);
    theme.Colors.InputBorder      = theme.Colors.Border;
    theme.Colors.InputBorderFocus = theme.Colors.FocusRing;
    theme.Colors.SelectionBg      = C(0.20f, 0.39f, 0.66f);
    theme.Colors.SelectionText    = theme.Colors.TextPrimary;
    theme.Colors.RowHover         = C(0.20f, 0.22f, 0.26f);

    theme.Colors.Accent  = C(0.30f, 0.55f, 0.85f);
    theme.Colors.Success = C(0.30f, 0.75f, 0.45f);
    theme.Colors.Warning = C(0.85f, 0.65f, 0.25f);
    theme.Colors.Error   = C(0.85f, 0.30f, 0.30f);
    theme.Colors.Info    = C(0.33f, 0.62f, 0.95f);

    theme.Colors.ToastBg              = theme.Colors.SurfaceRaised;
    theme.Colors.ToastBorder          = theme.Colors.BorderStrong;
    theme.Colors.NotificationInfo     = theme.Colors.Info;
    theme.Colors.NotificationWarning  = theme.Colors.Warning;
    theme.Colors.NotificationError    = theme.Colors.Error;
    theme.Colors.NotificationAtlasAI  = C(0.46f, 0.54f, 1.0f);

    theme.Typography.Caption   = {"Segoe UI", 11.0f, 400, 14.0f};
    theme.Typography.Body      = {"Segoe UI", 13.0f, 400, 16.0f};
    theme.Typography.BodyStrong = {"Segoe UI", 13.0f, 600, 16.0f};
    theme.Typography.Subtitle  = {"Segoe UI", 14.0f, 600, 18.0f};
    theme.Typography.Title     = {"Segoe UI", 16.0f, 600, 20.0f};
    theme.Typography.Mono      = {"Consolas", 12.0f, 400, 15.0f};
    theme.Typography.Tooltip   = {"Segoe UI", 12.0f, 400, 15.0f};
    theme.Typography.Tab       = {"Segoe UI", 12.0f, 600, 15.0f};
    theme.Typography.Menu      = {"Segoe UI", 13.0f, 400, 16.0f};

    return theme;
}

} // namespace NF::UI::AtlasUI
