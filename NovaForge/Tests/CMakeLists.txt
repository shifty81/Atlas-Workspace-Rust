# ── NovaForge game test suite ──────────────────────────────────────
# Engine subsystem tests live in the workspace Tests/.
# This suite covers NovaForge-specific game modules.

include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.5.2
)
FetchContent_MakeAvailable(Catch2)

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(CTest)
include(Catch)

# ── Route all test executables into bin/Tests/ ────────────────────
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/Tests)
foreach(CFG Debug Release RelWithDebInfo MinSizeRel)
    string(TOUPPER ${CFG} CFG_UPPER)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CFG_UPPER} ${CMAKE_BINARY_DIR}/bin/${CFG}/Tests)
endforeach()

# Game tests
add_executable(NF_GameTests
    Game/test_game.cpp
    catch_main.cpp
)
target_link_libraries(NF_GameTests PRIVATE NF::Game NF::Engine NF::Core Catch2::Catch2)
catch_discover_tests(NF_GameTests)
