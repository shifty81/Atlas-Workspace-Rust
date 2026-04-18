#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"

using namespace NF;

// --- BuildTarget names ---

TEST_CASE("BuildTarget names cover all 8 values", "[Editor][S18]") {
    REQUIRE(std::string(buildTargetName(BuildTarget::Executable))  == "Executable");
    REQUIRE(std::string(buildTargetName(BuildTarget::SharedLib))   == "SharedLib");
    REQUIRE(std::string(buildTargetName(BuildTarget::StaticLib))   == "StaticLib");
    REQUIRE(std::string(buildTargetName(BuildTarget::HeaderOnly))  == "HeaderOnly");
    REQUIRE(std::string(buildTargetName(BuildTarget::TestSuite))   == "TestSuite");
    REQUIRE(std::string(buildTargetName(BuildTarget::Plugin))      == "Plugin");
    REQUIRE(std::string(buildTargetName(BuildTarget::Shader))      == "Shader");
    REQUIRE(std::string(buildTargetName(BuildTarget::ContentPack)) == "ContentPack");
}

// --- BuildPlatform names ---

TEST_CASE("BuildPlatform names cover all 5 values", "[Editor][S18]") {
    REQUIRE(std::string(buildPlatformName(BuildPlatform::Windows)) == "Windows");
    REQUIRE(std::string(buildPlatformName(BuildPlatform::Linux))   == "Linux");
    REQUIRE(std::string(buildPlatformName(BuildPlatform::MacOS))   == "MacOS");
    REQUIRE(std::string(buildPlatformName(BuildPlatform::WebAsm))  == "WebAsm");
    REQUIRE(std::string(buildPlatformName(BuildPlatform::Console)) == "Console");
}

// --- BuildConfig ---

TEST_CASE("BuildConfig isDebug/isRelease predicates", "[Editor][S18]") {
    BuildConfig cfg;
    cfg.name = "dbg";
    cfg.debugSymbols = true;
    cfg.optimized    = false;
    REQUIRE(cfg.isDebug());
    REQUIRE_FALSE(cfg.isRelease());

    cfg.debugSymbols = false;
    cfg.optimized    = true;
    REQUIRE(cfg.isRelease());
    REQUIRE_FALSE(cfg.isDebug());

    // both on = neither pure debug nor pure release
    cfg.debugSymbols = true;
    cfg.optimized    = true;
    REQUIRE_FALSE(cfg.isDebug());
    REQUIRE_FALSE(cfg.isRelease());
}

TEST_CASE("BuildConfig addDefine rejects duplicates", "[Editor][S18]") {
    BuildConfig cfg;
    cfg.name = "test";
    REQUIRE(cfg.addDefine("NF_DEBUG"));
    REQUIRE(cfg.addDefine("NF_EDITOR"));
    REQUIRE_FALSE(cfg.addDefine("NF_DEBUG")); // duplicate
    REQUIRE(cfg.defineCount() == 2);
}

TEST_CASE("BuildConfig addIncludePath rejects duplicates", "[Editor][S18]") {
    BuildConfig cfg;
    cfg.name = "test";
    REQUIRE(cfg.addIncludePath("src/core"));
    REQUIRE(cfg.addIncludePath("src/engine"));
    REQUIRE_FALSE(cfg.addIncludePath("src/core")); // duplicate
    REQUIRE(cfg.includePathCount() == 2);
}

// --- BuildProfile ---

TEST_CASE("BuildProfile addConfig + duplicate rejection", "[Editor][S18]") {
    BuildProfile profile;
    BuildConfig c1; c1.name = "Debug";
    BuildConfig c2; c2.name = "Debug"; // duplicate

    REQUIRE(profile.addConfig(c1));
    REQUIRE_FALSE(profile.addConfig(c2));
    REQUIRE(profile.configCount() == 1);
}

TEST_CASE("BuildProfile removeConfig", "[Editor][S18]") {
    BuildProfile profile;
    BuildConfig c1; c1.name = "Rel";
    profile.addConfig(c1);
    REQUIRE(profile.removeConfig("Rel"));
    REQUIRE(profile.configCount() == 0);
    REQUIRE_FALSE(profile.removeConfig("Rel")); // already gone
}

TEST_CASE("BuildProfile findConfig", "[Editor][S18]") {
    BuildProfile profile;
    BuildConfig c1; c1.name = "Debug"; c1.debugSymbols = true;
    profile.addConfig(c1);

    auto* found = profile.findConfig("Debug");
    REQUIRE(found != nullptr);
    REQUIRE(found->debugSymbols == true);
    REQUIRE(profile.findConfig("missing") == nullptr);
}

TEST_CASE("BuildProfile debugConfigCount + releaseConfigCount", "[Editor][S18]") {
    BuildProfile profile;
    BuildConfig d; d.name = "DbgCfg"; d.debugSymbols = true; d.optimized = false;
    BuildConfig r; r.name = "RelCfg"; r.debugSymbols = false; r.optimized = true;
    BuildConfig m; m.name = "MixCfg"; m.debugSymbols = true;  m.optimized = true;
    profile.addConfig(d);
    profile.addConfig(r);
    profile.addConfig(m);

    REQUIRE(profile.debugConfigCount()   == 1);
    REQUIRE(profile.releaseConfigCount() == 1);
}

// --- BuildConfigurationSystem ---

TEST_CASE("BuildConfigurationSystem init/shutdown lifecycle", "[Editor][S18]") {
    BuildConfigurationSystem sys;
    REQUIRE_FALSE(sys.isInitialized());
    sys.init();
    REQUIRE(sys.isInitialized());
    sys.shutdown();
    REQUIRE_FALSE(sys.isInitialized());
}

TEST_CASE("BuildConfigurationSystem createProfile before init returns false", "[Editor][S18]") {
    BuildConfigurationSystem sys;
    REQUIRE_FALSE(sys.createProfile("dev"));
}

TEST_CASE("BuildConfigurationSystem createProfile + duplicate rejection", "[Editor][S18]") {
    BuildConfigurationSystem sys;
    sys.init();
    REQUIRE(sys.createProfile("dev"));
    REQUIRE(sys.createProfile("prod"));
    REQUIRE_FALSE(sys.createProfile("dev")); // duplicate
    REQUIRE(sys.profileCount() == 2);
}

TEST_CASE("BuildConfigurationSystem setActiveProfile + activeProfile", "[Editor][S18]") {
    BuildConfigurationSystem sys;
    sys.init();
    sys.createProfile("dev");
    REQUIRE(sys.activeProfile() == nullptr);

    REQUIRE(sys.setActiveProfile("dev"));
    REQUIRE(sys.activeProfileName() == "dev");
    REQUIRE(sys.activeProfile() != nullptr);

    REQUIRE_FALSE(sys.setActiveProfile("nonexistent"));
}

TEST_CASE("BuildConfigurationSystem removeProfile clears active", "[Editor][S18]") {
    BuildConfigurationSystem sys;
    sys.init();
    sys.createProfile("staging");
    sys.setActiveProfile("staging");
    REQUIRE(sys.removeProfile("staging"));
    REQUIRE(sys.activeProfileName().empty());
    REQUIRE(sys.profileCount() == 0);
}

TEST_CASE("BuildConfigurationSystem totalConfigCount across profiles", "[Editor][S18]") {
    BuildConfigurationSystem sys;
    sys.init();
    sys.createProfile("p1");
    sys.createProfile("p2");

    BuildConfig c1; c1.name = "c1";
    BuildConfig c2; c2.name = "c2";
    BuildConfig c3; c3.name = "c3";

    sys.findProfile("p1")->addConfig(c1);
    sys.findProfile("p1")->addConfig(c2);
    sys.findProfile("p2")->addConfig(c3);

    REQUIRE(sys.totalConfigCount() == 3);
}
