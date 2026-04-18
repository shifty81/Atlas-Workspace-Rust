#pragma once
#include <string>

namespace atlas::editor {

/// Returns the default editor layout expressed in the Atlas GUI DSL.
/// This is the bootstrap layout used when no user-customized layout
/// exists.  The editor UI is self-hosted: the editor shell itself is
/// described in the same DSL format used for game UI.
///
/// Layout: Left sidebar (Asset Browser), centre viewport, right
/// inspector, bottom console — a project-agnostic starting point.
inline std::string DefaultEditorDSL() {
    return R"(layout "DefaultEditor" {
    split horizontal 0.18 {
        panel "AssetBrowser"
        split vertical 0.75 {
            panel "Viewport"
            split horizontal 0.60 {
                panel "Console"
                panel "Inspector"
            }
        }
    }
})";
}

} // namespace atlas::editor
