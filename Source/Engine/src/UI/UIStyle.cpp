#include "UIStyle.h"

namespace atlas::ui {

UIStyle UIStyle::Dark() {
    return UIStyle{};  // defaults are already dark theme
}

UIStyle UIStyle::Light() {
    UIStyle s;
    s.background        = {240, 240, 240, 255};
    s.panelBackground   = {250, 250, 250, 255};
    s.headerBackground  = {230, 230, 235, 255};
    s.tooltipBackground = {255, 255, 240, 240};

    s.textPrimary       = {30,  30,  30,  255};
    s.textSecondary     = {80,  80,  80,  255};
    s.textDisabled      = {160, 160, 160, 255};
    s.textHighlight     = {0,   0,   0,   255};

    s.accent            = {0,   100, 200, 255};
    s.accentHover       = {0,   120, 230, 255};
    s.accentPressed     = {0,   80,  170, 255};

    s.buttonNormal      = {220, 220, 220, 255};
    s.buttonHover       = {200, 200, 200, 255};
    s.buttonPressed     = {180, 180, 180, 255};
    s.buttonDisabled    = {230, 230, 230, 255};

    s.selectionBg       = {0,   100, 200, 60};
    s.hoverBg           = {0,   0,   0,   15};

    s.border            = {200, 200, 200, 255};
    s.borderFocused     = {0,   100, 200, 255};

    s.statusOk          = {40,  160, 120, 255};
    s.statusWarning     = {200, 160, 20,  255};
    s.statusError       = {200, 50,  50,  255};

    return s;
}

} // namespace atlas::ui
