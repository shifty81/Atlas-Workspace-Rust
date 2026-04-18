#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"

using namespace NF;

// ── S75: BuildConfiguration ──────────────────────────────────────

TEST_CASE("BuildTarget names are correct", "[Editor][S75]") {
    REQUIRE(std::string(buildTargetName(BuildTarget::Executable))  == "Executable");
    REQUIRE(std::string(buildTargetName(BuildTarget::SharedLib))   == "SharedLib");
    REQUIRE(std::string(buildTargetName(BuildTarget::StaticLib))   == "StaticLib");
    REQUIRE(std::string(buildTargetName(BuildTarget::HeaderOnly))  == "HeaderOnly");
    REQUIRE(std::string(buildTargetName(BuildTarget::TestSuite))   == "TestSuite");
    REQUIRE(std::string(buildTargetName(BuildTarget::Plugin))      == "Plugin");
    REQUIRE(std::string(buildTargetName(BuildTarget::Shader))      == "Shader");
    REQUIRE(std::string(buildTargetName(BuildTarget::ContentPack)) == "ContentPack");
}

TEST_CASE("BuildPlatform names are correct", "[Editor][S75]") {
    REQUIRE(std::string(buildPlatformName(BuildPlatform::Windows)) == "Windows");
    REQUIRE(std::string(buildPlatformName(BuildPlatform::Linux))   == "Linux");
    REQUIRE(std::string(buildPlatformName(BuildPlatform::MacOS))   == "MacOS");
    REQUIRE(std::string(buildPlatformName(BuildPlatform::WebAsm))  == "WebAsm");
    REQUIRE(std::string(buildPlatformName(BuildPlatform::Console)) == "Console");
}

TEST_CASE("BuildConfig defaults to Executable target and Windows platform", "[Editor][S75]") {
    BuildConfig cfg;
    REQUIRE(cfg.target == BuildTarget::Executable);
    REQUIRE(cfg.platform == BuildPlatform::Windows);
}

TEST_CASE("BuildConfig isDebug true when debugSymbols on and not optimized", "[Editor][S75]") {
    BuildConfig cfg;
    cfg.debugSymbols = true;
    cfg.optimized = false;
    REQUIRE(cfg.isDebug());
    REQUIRE_FALSE(cfg.isRelease());
}

TEST_CASE("BuildConfig isRelease true when optimized on and no debug symbols", "[Editor][S75]") {
    BuildConfig cfg;
    cfg.debugSymbols = false;
    cfg.optimized = true;
    REQUIRE(cfg.isRelease());
    REQUIRE_FALSE(cfg.isDebug());
}

TEST_CASE("BuildConfig addDefine adds a define", "[Editor][S75]") {
    BuildConfig cfg;
    REQUIRE(cfg.addDefine("NDEBUG"));
    REQUIRE(cfg.defineCount() == 1);
}

TEST_CASE("BuildConfig addDefine rejects duplicate", "[Editor][S75]") {
    BuildConfig cfg;
    cfg.addDefine("NDEBUG");
    REQUIRE_FALSE(cfg.addDefine("NDEBUG"));
    REQUIRE(cfg.defineCount() == 1);
}

TEST_CASE("BuildConfig addIncludePath adds a path", "[Editor][S75]") {
    BuildConfig cfg;
    REQUIRE(cfg.addIncludePath("/usr/include"));
    REQUIRE(cfg.includePathCount() == 1);
}

TEST_CASE("BuildConfig addIncludePath rejects duplicate", "[Editor][S75]") {
    BuildConfig cfg;
    cfg.addIncludePath("/usr/include");
    REQUIRE_FALSE(cfg.addIncludePath("/usr/include"));
    REQUIRE(cfg.includePathCount() == 1);
}

TEST_CASE("BuildProfile starts empty", "[Editor][S75]") {
    BuildProfile profile;
    REQUIRE(profile.configCount() == 0);
}

TEST_CASE("BuildProfile addConfig increases count", "[Editor][S75]") {
    BuildProfile profile;
    BuildConfig cfg;
    cfg.name = "Debug";
    cfg.debugSymbols = true;
    REQUIRE(profile.addConfig(cfg));
    REQUIRE(profile.configCount() == 1);
}

TEST_CASE("BuildProfile addConfig rejects duplicate name", "[Editor][S75]") {
    BuildProfile profile;
    BuildConfig cfg;
    cfg.name = "Debug";
    profile.addConfig(cfg);
    REQUIRE_FALSE(profile.addConfig(cfg));
    REQUIRE(profile.configCount() == 1);
}

TEST_CASE("BuildProfile removeConfig removes entry", "[Editor][S75]") {
    BuildProfile profile;
    BuildConfig cfg;
    cfg.name = "Debug";
    profile.addConfig(cfg);
    REQUIRE(profile.removeConfig("Debug"));
    REQUIRE(profile.configCount() == 0);
}

TEST_CASE("BuildProfile removeConfig returns false for missing", "[Editor][S75]") {
    BuildProfile profile;
    REQUIRE_FALSE(profile.removeConfig("ghost"));
}

TEST_CASE("BuildProfile findConfig returns correct config", "[Editor][S75]") {
    BuildProfile profile;
    BuildConfig cfg;
    cfg.name = "Release";
    cfg.optimized = true;
    profile.addConfig(cfg);
    REQUIRE(profile.findConfig("Release") != nullptr);
    REQUIRE(profile.findConfig("Release")->optimized == true);
    REQUIRE(profile.findConfig("missing") == nullptr);
}

TEST_CASE("BuildProfile debugConfigCount counts debug configs", "[Editor][S75]") {
    BuildProfile profile;
    BuildConfig dbg; dbg.name = "Debug"; dbg.debugSymbols = true; dbg.optimized = false;
    BuildConfig rel; rel.name = "Release"; rel.debugSymbols = false; rel.optimized = true;
    BuildConfig other; other.name = "Other";
    profile.addConfig(dbg);
    profile.addConfig(rel);
    profile.addConfig(other);
    REQUIRE(profile.debugConfigCount() == 1);
    REQUIRE(profile.releaseConfigCount() == 1);
}

TEST_CASE("BuildProfile MAX_CONFIGS is 64", "[Editor][S75]") {
    REQUIRE(BuildProfile::MAX_CONFIGS == 64);
}

TEST_CASE("BuildConfigurationSystem starts uninitialized", "[Editor][S75]") {
    BuildConfigurationSystem sys;
    REQUIRE_FALSE(sys.isInitialized());
    REQUIRE(sys.profileCount() == 0);
}

TEST_CASE("BuildConfigurationSystem init sets initialized", "[Editor][S75]") {
    BuildConfigurationSystem sys;
    sys.init();
    REQUIRE(sys.isInitialized());
}

TEST_CASE("BuildConfigurationSystem createProfile before init returns false", "[Editor][S75]") {
    BuildConfigurationSystem sys;
    REQUIRE_FALSE(sys.createProfile("Default"));
}

TEST_CASE("BuildConfigurationSystem createProfile creates a profile", "[Editor][S75]") {
    BuildConfigurationSystem sys;
    sys.init();
    REQUIRE(sys.createProfile("Default"));
    REQUIRE(sys.profileCount() == 1);
}

TEST_CASE("BuildConfigurationSystem createProfile rejects duplicate name", "[Editor][S75]") {
    BuildConfigurationSystem sys;
    sys.init();
    sys.createProfile("Default");
    REQUIRE_FALSE(sys.createProfile("Default"));
    REQUIRE(sys.profileCount() == 1);
}

TEST_CASE("BuildConfigurationSystem removeProfile removes entry", "[Editor][S75]") {
    BuildConfigurationSystem sys;
    sys.init();
    sys.createProfile("Default");
    REQUIRE(sys.removeProfile("Default"));
    REQUIRE(sys.profileCount() == 0);
}

TEST_CASE("BuildConfigurationSystem removeProfile returns false for missing", "[Editor][S75]") {
    BuildConfigurationSystem sys;
    sys.init();
    REQUIRE_FALSE(sys.removeProfile("ghost"));
}

TEST_CASE("BuildConfigurationSystem findProfile returns correct profile", "[Editor][S75]") {
    BuildConfigurationSystem sys;
    sys.init();
    sys.createProfile("Project");
    REQUIRE(sys.findProfile("Project") != nullptr);
    REQUIRE(sys.findProfile("missing") == nullptr);
}

TEST_CASE("BuildConfigurationSystem setActiveProfile updates active", "[Editor][S75]") {
    BuildConfigurationSystem sys;
    sys.init();
    sys.createProfile("Profile1");
    REQUIRE(sys.setActiveProfile("Profile1"));
    REQUIRE(sys.activeProfileName() == "Profile1");
}

TEST_CASE("BuildConfigurationSystem setActiveProfile fails for missing profile", "[Editor][S75]") {
    BuildConfigurationSystem sys;
    sys.init();
    REQUIRE_FALSE(sys.setActiveProfile("ghost"));
}

TEST_CASE("BuildConfigurationSystem activeProfile returns nullptr when none set", "[Editor][S75]") {
    BuildConfigurationSystem sys;
    sys.init();
    REQUIRE(sys.activeProfile() == nullptr);
}

TEST_CASE("BuildConfigurationSystem activeProfile returns profile after set", "[Editor][S75]") {
    BuildConfigurationSystem sys;
    sys.init();
    sys.createProfile("P");
    sys.setActiveProfile("P");
    REQUIRE(sys.activeProfile() != nullptr);
}

TEST_CASE("BuildConfigurationSystem removeProfile clears active if same", "[Editor][S75]") {
    BuildConfigurationSystem sys;
    sys.init();
    sys.createProfile("P");
    sys.setActiveProfile("P");
    sys.removeProfile("P");
    REQUIRE(sys.activeProfileName().empty());
    REQUIRE(sys.activeProfile() == nullptr);
}

TEST_CASE("BuildConfigurationSystem totalConfigCount sums configs", "[Editor][S75]") {
    BuildConfigurationSystem sys;
    sys.init();
    sys.createProfile("P1");
    sys.createProfile("P2");
    BuildConfig c1; c1.name = "Debug";
    BuildConfig c2; c2.name = "Release";
    BuildConfig c3; c3.name = "Ship";
    sys.findProfile("P1")->addConfig(c1);
    sys.findProfile("P1")->addConfig(c2);
    sys.findProfile("P2")->addConfig(c3);
    REQUIRE(sys.totalConfigCount() == 3);
}

TEST_CASE("BuildConfigurationSystem shutdown clears profiles", "[Editor][S75]") {
    BuildConfigurationSystem sys;
    sys.init();
    sys.createProfile("P");
    sys.shutdown();
    REQUIRE_FALSE(sys.isInitialized());
    REQUIRE(sys.profileCount() == 0);
}
