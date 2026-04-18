#include "../engine/production/BuildProfile.h"
#include <iostream>
#include <cassert>

using namespace atlas::production;

void test_profile_default_debug() {
    BuildProfileConfig config = BuildProfile::DefaultFor(ProfileType::Debug);

    assert(config.type == ProfileType::Debug);
    assert(config.optimizationsEnabled == false);
    assert(config.debugSymbols == true);
    assert(config.assertionsEnabled == true);
    assert(config.stripEditorData == false);
    assert(config.stripTools == false);
    assert(config.enableProfiling == true);
    assert(config.enableLogging == true);
    assert(config.compilerFlags == "-O0 -g");

    std::cout << "[PASS] test_profile_default_debug" << std::endl;
}

void test_profile_default_development() {
    BuildProfileConfig config = BuildProfile::DefaultFor(ProfileType::Development);

    assert(config.type == ProfileType::Development);
    assert(config.optimizationsEnabled == true);
    assert(config.debugSymbols == true);
    assert(config.assertionsEnabled == true);
    assert(config.stripEditorData == false);
    assert(config.stripTools == false);
    assert(config.compilerFlags == "-O2 -g");

    std::cout << "[PASS] test_profile_default_development" << std::endl;
}

void test_profile_default_release() {
    BuildProfileConfig config = BuildProfile::DefaultFor(ProfileType::Release);

    assert(config.type == ProfileType::Release);
    assert(config.optimizationsEnabled == true);
    assert(config.debugSymbols == false);
    assert(config.assertionsEnabled == false);
    assert(config.stripEditorData == true);
    assert(config.stripTools == true);
    assert(config.enableProfiling == false);
    assert(config.enableLogging == false);
    assert(config.linkerFlags == "-s");

    std::cout << "[PASS] test_profile_default_release" << std::endl;
}

void test_profile_name() {
    assert(std::string(BuildProfile::ProfileName(ProfileType::Debug)) == "Debug");
    assert(std::string(BuildProfile::ProfileName(ProfileType::Development)) == "Development");
    assert(std::string(BuildProfile::ProfileName(ProfileType::Release)) == "Release");

    std::cout << "[PASS] test_profile_name" << std::endl;
}

void test_profile_set_active() {
    BuildProfile profile;

    profile.SetActive(ProfileType::Release);
    assert(profile.ActiveType() == ProfileType::Release);
    assert(profile.ActiveConfig().stripEditorData == true);
    assert(profile.ActiveConfig().stripTools == true);

    profile.SetActive(ProfileType::Debug);
    assert(profile.ActiveType() == ProfileType::Debug);
    assert(profile.ActiveConfig().stripEditorData == false);
    assert(profile.ActiveConfig().stripTools == false);

    std::cout << "[PASS] test_profile_set_active" << std::endl;
}

void test_profile_set_custom_config() {
    BuildProfile profile;

    BuildProfileConfig custom;
    custom.type = ProfileType::Development;
    custom.optimizationsEnabled = false;
    custom.compilerFlags = "-O1";

    profile.SetActiveConfig(custom);
    assert(profile.ActiveType() == ProfileType::Development);
    assert(profile.ActiveConfig().optimizationsEnabled == false);
    assert(profile.ActiveConfig().compilerFlags == "-O1");

    std::cout << "[PASS] test_profile_set_custom_config" << std::endl;
}

void test_profile_overrides() {
    BuildProfile profile;

    assert(profile.OverrideCount() == 0);
    assert(!profile.HasOverride("MY_DEFINE"));

    profile.SetOverride("MY_DEFINE", "1");
    assert(profile.HasOverride("MY_DEFINE"));
    assert(profile.GetOverride("MY_DEFINE") == "1");
    assert(profile.OverrideCount() == 1);

    profile.SetOverride("ANOTHER", "value");
    assert(profile.OverrideCount() == 2);

    profile.ClearOverrides();
    assert(profile.OverrideCount() == 0);
    assert(!profile.HasOverride("MY_DEFINE"));

    std::cout << "[PASS] test_profile_overrides" << std::endl;
}

void test_profile_override_missing_key() {
    BuildProfile profile;

    assert(profile.GetOverride("nonexistent") == "");

    std::cout << "[PASS] test_profile_override_missing_key" << std::endl;
}
