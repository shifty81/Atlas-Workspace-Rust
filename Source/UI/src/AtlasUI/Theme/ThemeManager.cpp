#include "NF/UI/AtlasUI/Theme/ThemeManager.h"
#include "NF/UI/AtlasUI/Theme/ThemeDefaults.h"

namespace NF::UI::AtlasUI {

ThemeManager& ThemeManager::Get() {
    static ThemeManager instance;
    return instance;
}

ThemeManager::ThemeManager()
    : m_current(MakeAtlasDarkTheme()) {
}

const AtlasTheme& ThemeManager::Current() const {
    return m_current;
}

void ThemeManager::SetTheme(const AtlasTheme& theme) {
    m_current = theme;
}

} // namespace NF::UI::AtlasUI
