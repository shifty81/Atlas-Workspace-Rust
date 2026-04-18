# ── Executables ────────────────────────────────────────────────────

# Editor
if(NF_BUILD_EDITOR)
    add_executable(NovaForgeEditor NovaForgeEditor/main.cpp)
    target_link_libraries(NovaForgeEditor PRIVATE
        NF::NovaForgeAdapter
        NF::Editor NF::Game NF::World NF::AI NF::GraphVM
        NF::Renderer NF::Physics NF::Audio NF::Animation
        NF::Input NF::Networking NF::UI NF::Engine NF::Core
        NF_CompileOptions
    )
    # On Windows, suppress the console window by using the WINDOWS subsystem.
    # The editor already creates its own Win32 window via CreateWindowExW.
    if(WIN32)
        set_target_properties(NovaForgeEditor PROPERTIES
            WIN32_EXECUTABLE TRUE
        )
        # Redirect main() to WinMain via /ENTRY:mainCRTStartup so the existing
        # int main(int, char**) entry point works with the WINDOWS subsystem.
        target_link_options(NovaForgeEditor PRIVATE
            $<$<CXX_COMPILER_ID:MSVC>:/ENTRY:mainCRTStartup>
            $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wl,--subsystem,windows>
        )
    endif()
endif()

# Game Client
if(NF_BUILD_GAME)
    add_executable(NovaForgeGame NovaForgeGame/main.cpp)
    target_link_libraries(NovaForgeGame PRIVATE
        NF::Game NF::World NF::GraphVM
        NF::Renderer NF::Physics NF::Audio NF::Animation
        NF::Input NF::Networking NF::UI NF::Engine NF::Core
        NF_CompileOptions
    )
endif()

# Dedicated Server
if(NF_BUILD_SERVER)
    add_executable(NovaForgeServer NovaForgeServer/main.cpp)
    target_link_libraries(NovaForgeServer PRIVATE
        NF::Game NF::World NF::AI NF::GraphVM
        NF::Networking NF::Physics NF::Engine NF::Core
        NF_CompileOptions
    )
endif()
