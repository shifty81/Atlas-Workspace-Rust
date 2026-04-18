#pragma once

#include "NF/UI/AtlasUI/Commands/CommandTypes.h"

namespace NF::UI::AtlasUI {

[[nodiscard]] inline KeyChord MakeChord(int key, bool ctrl = false,
                                         bool alt = false, bool shift = false,
                                         bool win = false) noexcept {
    return {ctrl, alt, shift, win, key};
}

} // namespace NF::UI::AtlasUI
