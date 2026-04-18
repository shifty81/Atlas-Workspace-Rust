#include "NF/UI/AtlasUI/Commands/CommandTypes.h"

namespace NF::UI::AtlasUI {

std::string KeyChord::ToString() const {
    if (!IsValid()) return {};
    std::string result;
    if (Ctrl) result += "Ctrl+";
    if (Alt) result += "Alt+";
    if (Shift) result += "Shift+";
    if (Win) result += "Win+";
    if (Key >= 32 && Key < 127) {
        result += static_cast<char>(Key);
    } else {
        result += "Key(" + std::to_string(Key) + ")";
    }
    return result;
}

} // namespace NF::UI::AtlasUI
