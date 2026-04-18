// Atlas Workspace — Catch2 test entry point.
// Linked into every test executable so main() is compiled directly
// into the binary (avoids MSVC static-lib extraction issue with
// Catch2::Catch2WithMain on the Visual Studio generator).
#include <catch2/catch_session.hpp>

int main(int argc, char* argv[]) {
    return Catch::Session().run(argc, argv);
}
