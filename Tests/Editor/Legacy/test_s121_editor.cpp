// S121 editor tests: RuntimeConfigEditor, FeatureFlagEditor, ExperimentEditor
#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/ExperimentEditor.h"
#include "NF/Editor/FeatureFlagEditor.h"
#include "NF/Editor/RuntimeConfigEditor.h"

using namespace NF;

// ── RuntimeConfigEditor ───────────────────────────────────────────────────────

TEST_CASE("RtConfigScope names", "[Editor][S121]") {
    REQUIRE(std::string(rtConfigScopeName(RtConfigScope::Global))   == "Global");
    REQUIRE(std::string(rtConfigScopeName(RtConfigScope::Session))  == "Session");
    REQUIRE(std::string(rtConfigScopeName(RtConfigScope::User))     == "User");
    REQUIRE(std::string(rtConfigScopeName(RtConfigScope::Project))  == "Project");
    REQUIRE(std::string(rtConfigScopeName(RtConfigScope::Platform)) == "Platform");
}

TEST_CASE("RtConfigValueType names", "[Editor][S121]") {
    REQUIRE(std::string(rtConfigValueTypeName(RtConfigValueType::Bool))   == "Bool");
    REQUIRE(std::string(rtConfigValueTypeName(RtConfigValueType::Int))    == "Int");
    REQUIRE(std::string(rtConfigValueTypeName(RtConfigValueType::Float))  == "Float");
    REQUIRE(std::string(rtConfigValueTypeName(RtConfigValueType::String)) == "String");
    REQUIRE(std::string(rtConfigValueTypeName(RtConfigValueType::Json))   == "Json");
}

TEST_CASE("RuntimeConfigEntry defaults", "[Editor][S121]") {
    RuntimeConfigEntry e(1, "app.debug", RtConfigScope::Global, RtConfigValueType::Bool);
    REQUIRE(e.id()            == 1u);
    REQUIRE(e.key()           == "app.debug");
    REQUIRE(e.scope()         == RtConfigScope::Global);
    REQUIRE(e.valueType()     == RtConfigValueType::Bool);
    REQUIRE(!e.isOverridable());
}

TEST_CASE("RuntimeConfigEntry mutation", "[Editor][S121]") {
    RuntimeConfigEntry e(2, "app.maxPlayers", RtConfigScope::Session, RtConfigValueType::Int);
    e.setIsOverridable(true);
    REQUIRE(e.isOverridable());
}

TEST_CASE("RuntimeConfigEditor defaults", "[Editor][S121]") {
    RuntimeConfigEditor ed;
    REQUIRE(!ed.isShowOverridesOnly());
    REQUIRE(!ed.isGroupByScope());
    REQUIRE(ed.searchQuery()  == "");
    REQUIRE(ed.entryCount()   == 0u);
}

TEST_CASE("RuntimeConfigEditor add/remove entries", "[Editor][S121]") {
    RuntimeConfigEditor ed;
    REQUIRE(ed.addEntry(RuntimeConfigEntry(1, "key.a", RtConfigScope::Global,  RtConfigValueType::Bool)));
    REQUIRE(ed.addEntry(RuntimeConfigEntry(2, "key.b", RtConfigScope::User,    RtConfigValueType::Int)));
    REQUIRE(ed.addEntry(RuntimeConfigEntry(3, "key.c", RtConfigScope::Project, RtConfigValueType::Float)));
    REQUIRE(!ed.addEntry(RuntimeConfigEntry(1, "key.a", RtConfigScope::Global, RtConfigValueType::Bool)));
    REQUIRE(ed.entryCount() == 3u);
    REQUIRE(ed.removeEntry(2));
    REQUIRE(ed.entryCount() == 2u);
    REQUIRE(!ed.removeEntry(99));
}

TEST_CASE("RuntimeConfigEditor counts and find", "[Editor][S121]") {
    RuntimeConfigEditor ed;
    RuntimeConfigEntry e1(1, "k1", RtConfigScope::Global,  RtConfigValueType::Bool);
    RuntimeConfigEntry e2(2, "k2", RtConfigScope::Global,  RtConfigValueType::Int);   e2.setIsOverridable(true);
    RuntimeConfigEntry e3(3, "k3", RtConfigScope::User,    RtConfigValueType::Bool);
    RuntimeConfigEntry e4(4, "k4", RtConfigScope::Project, RtConfigValueType::Float); e4.setIsOverridable(true);
    ed.addEntry(e1); ed.addEntry(e2); ed.addEntry(e3); ed.addEntry(e4);
    REQUIRE(ed.countByScope(RtConfigScope::Global)         == 2u);
    REQUIRE(ed.countByScope(RtConfigScope::User)           == 1u);
    REQUIRE(ed.countByScope(RtConfigScope::Platform)       == 0u);
    REQUIRE(ed.countByType(RtConfigValueType::Bool)        == 2u);
    REQUIRE(ed.countByType(RtConfigValueType::Float)       == 1u);
    REQUIRE(ed.countByType(RtConfigValueType::Json)        == 0u);
    REQUIRE(ed.countOverridable()                          == 2u);
    auto* found = ed.findEntry(3);
    REQUIRE(found != nullptr);
    REQUIRE(found->scope() == RtConfigScope::User);
    REQUIRE(ed.findEntry(99) == nullptr);
}

TEST_CASE("RuntimeConfigEditor settings mutation", "[Editor][S121]") {
    RuntimeConfigEditor ed;
    ed.setIsShowOverridesOnly(true);
    ed.setIsGroupByScope(true);
    ed.setSearchQuery("debug");
    REQUIRE(ed.isShowOverridesOnly());
    REQUIRE(ed.isGroupByScope());
    REQUIRE(ed.searchQuery() == "debug");
}

// ── FeatureFlagEditor ─────────────────────────────────────────────────────────

TEST_CASE("FlagState names", "[Editor][S121]") {
    REQUIRE(std::string(featureFlagStateName(FlagState::Disabled))   == "Disabled");
    REQUIRE(std::string(featureFlagStateName(FlagState::Enabled))    == "Enabled");
    REQUIRE(std::string(featureFlagStateName(FlagState::RollingOut)) == "RollingOut");
    REQUIRE(std::string(featureFlagStateName(FlagState::Deprecated)) == "Deprecated");
}

TEST_CASE("FlagTarget names", "[Editor][S121]") {
    REQUIRE(std::string(featureFlagTargetName(FlagTarget::All))      == "All");
    REQUIRE(std::string(featureFlagTargetName(FlagTarget::Internal)) == "Internal");
    REQUIRE(std::string(featureFlagTargetName(FlagTarget::Beta))     == "Beta");
    REQUIRE(std::string(featureFlagTargetName(FlagTarget::Canary))   == "Canary");
    REQUIRE(std::string(featureFlagTargetName(FlagTarget::Custom))   == "Custom");
}

TEST_CASE("FeatureFlagEntry defaults", "[Editor][S121]") {
    FeatureFlagEntry f(1, "new_ui", FlagState::Disabled, FlagTarget::All);
    REQUIRE(f.id()             == 1u);
    REQUIRE(f.name()           == "new_ui");
    REQUIRE(f.state()          == FlagState::Disabled);
    REQUIRE(f.target()         == FlagTarget::All);
    REQUIRE(f.rolloutPercent() == 0.0f);
    REQUIRE(f.isLogged());
}

TEST_CASE("FeatureFlagEntry mutation", "[Editor][S121]") {
    FeatureFlagEntry f(2, "beta_feature", FlagState::Disabled, FlagTarget::Beta);
    f.setState(FlagState::RollingOut);
    f.setTarget(FlagTarget::Canary);
    f.setRolloutPercent(25.0f);
    f.setIsLogged(false);
    REQUIRE(f.state()          == FlagState::RollingOut);
    REQUIRE(f.target()         == FlagTarget::Canary);
    REQUIRE(f.rolloutPercent() == 25.0f);
    REQUIRE(!f.isLogged());
}

TEST_CASE("FeatureFlagEditor defaults", "[Editor][S121]") {
    FeatureFlagEditor ed;
    REQUIRE(!ed.isShowDeprecated());
    REQUIRE(!ed.isGroupByTarget());
    REQUIRE(ed.rolloutThreshold() == 50.0f);
    REQUIRE(ed.flagCount()        == 0u);
}

TEST_CASE("FeatureFlagEditor add/remove flags", "[Editor][S121]") {
    FeatureFlagEditor ed;
    REQUIRE(ed.addFlag(FeatureFlagEntry(1, "flag_a", FlagState::Enabled,    FlagTarget::All)));
    REQUIRE(ed.addFlag(FeatureFlagEntry(2, "flag_b", FlagState::Disabled,   FlagTarget::Beta)));
    REQUIRE(ed.addFlag(FeatureFlagEntry(3, "flag_c", FlagState::RollingOut, FlagTarget::Canary)));
    REQUIRE(!ed.addFlag(FeatureFlagEntry(1, "flag_a", FlagState::Enabled,   FlagTarget::All)));
    REQUIRE(ed.flagCount() == 3u);
    REQUIRE(ed.removeFlag(2));
    REQUIRE(ed.flagCount() == 2u);
    REQUIRE(!ed.removeFlag(99));
}

TEST_CASE("FeatureFlagEditor counts and find", "[Editor][S121]") {
    FeatureFlagEditor ed;
    FeatureFlagEntry f1(1, "f1", FlagState::Enabled,    FlagTarget::All);
    FeatureFlagEntry f2(2, "f2", FlagState::Enabled,    FlagTarget::Beta);
    FeatureFlagEntry f3(3, "f3", FlagState::Disabled,   FlagTarget::All);
    FeatureFlagEntry f4(4, "f4", FlagState::Deprecated, FlagTarget::Internal);
    ed.addFlag(f1); ed.addFlag(f2); ed.addFlag(f3); ed.addFlag(f4);
    REQUIRE(ed.countByState(FlagState::Enabled)      == 2u);
    REQUIRE(ed.countByState(FlagState::Disabled)     == 1u);
    REQUIRE(ed.countByState(FlagState::RollingOut)   == 0u);
    REQUIRE(ed.countByTarget(FlagTarget::All)        == 2u);
    REQUIRE(ed.countByTarget(FlagTarget::Beta)       == 1u);
    REQUIRE(ed.countByTarget(FlagTarget::Canary)     == 0u);
    REQUIRE(ed.countEnabled()                        == 2u);
    auto* found = ed.findFlag(3);
    REQUIRE(found != nullptr);
    REQUIRE(found->state() == FlagState::Disabled);
    REQUIRE(ed.findFlag(99) == nullptr);
}

TEST_CASE("FeatureFlagEditor settings mutation", "[Editor][S121]") {
    FeatureFlagEditor ed;
    ed.setIsShowDeprecated(true);
    ed.setIsGroupByTarget(true);
    ed.setRolloutThreshold(75.0f);
    REQUIRE(ed.isShowDeprecated());
    REQUIRE(ed.isGroupByTarget());
    REQUIRE(ed.rolloutThreshold() == 75.0f);
}

// ── ExperimentEditor ──────────────────────────────────────────────────────────

TEST_CASE("ExperimentStatus names", "[Editor][S121]") {
    REQUIRE(std::string(experimentStatusName(ExperimentStatus::Draft))     == "Draft");
    REQUIRE(std::string(experimentStatusName(ExperimentStatus::Active))    == "Active");
    REQUIRE(std::string(experimentStatusName(ExperimentStatus::Paused))    == "Paused");
    REQUIRE(std::string(experimentStatusName(ExperimentStatus::Concluded)) == "Concluded");
    REQUIRE(std::string(experimentStatusName(ExperimentStatus::Archived))  == "Archived");
}

TEST_CASE("ExperimentType names", "[Editor][S121]") {
    REQUIRE(std::string(experimentTypeName(ExperimentType::BinaryAB))     == "BinaryAB");
    REQUIRE(std::string(experimentTypeName(ExperimentType::Multivariate)) == "Multivariate");
    REQUIRE(std::string(experimentTypeName(ExperimentType::Bandit))       == "Bandit");
    REQUIRE(std::string(experimentTypeName(ExperimentType::HoldOut))      == "HoldOut");
    REQUIRE(std::string(experimentTypeName(ExperimentType::Custom))       == "Custom");
}

TEST_CASE("ExperimentEntry defaults", "[Editor][S121]") {
    ExperimentEntry e(1, "button_color_ab", ExperimentStatus::Draft, ExperimentType::BinaryAB);
    REQUIRE(e.id()             == 1u);
    REQUIRE(e.name()           == "button_color_ab");
    REQUIRE(e.status()         == ExperimentStatus::Draft);
    REQUIRE(e.type()           == ExperimentType::BinaryAB);
    REQUIRE(e.trafficPercent() == 10.0f);
    REQUIRE(!e.isAnalyzed());
}

TEST_CASE("ExperimentEntry mutation", "[Editor][S121]") {
    ExperimentEntry e(2, "pricing_mv", ExperimentStatus::Draft, ExperimentType::Multivariate);
    e.setStatus(ExperimentStatus::Active);
    e.setType(ExperimentType::Bandit);
    e.setTrafficPercent(20.0f);
    e.setIsAnalyzed(true);
    REQUIRE(e.status()         == ExperimentStatus::Active);
    REQUIRE(e.type()           == ExperimentType::Bandit);
    REQUIRE(e.trafficPercent() == 20.0f);
    REQUIRE(e.isAnalyzed());
}

TEST_CASE("ExperimentEditor defaults", "[Editor][S121]") {
    ExperimentEditor ed;
    REQUIRE(!ed.isShowArchived());
    REQUIRE(!ed.isGroupByType());
    REQUIRE(ed.minTrafficPercent() == 5.0f);
    REQUIRE(ed.experimentCount()   == 0u);
}

TEST_CASE("ExperimentEditor add/remove experiments", "[Editor][S121]") {
    ExperimentEditor ed;
    REQUIRE(ed.addExperiment(ExperimentEntry(1, "exp_a", ExperimentStatus::Active, ExperimentType::BinaryAB)));
    REQUIRE(ed.addExperiment(ExperimentEntry(2, "exp_b", ExperimentStatus::Draft,  ExperimentType::Multivariate)));
    REQUIRE(ed.addExperiment(ExperimentEntry(3, "exp_c", ExperimentStatus::Paused, ExperimentType::Bandit)));
    REQUIRE(!ed.addExperiment(ExperimentEntry(1, "exp_a", ExperimentStatus::Active, ExperimentType::BinaryAB)));
    REQUIRE(ed.experimentCount() == 3u);
    REQUIRE(ed.removeExperiment(2));
    REQUIRE(ed.experimentCount() == 2u);
    REQUIRE(!ed.removeExperiment(99));
}

TEST_CASE("ExperimentEditor counts and find", "[Editor][S121]") {
    ExperimentEditor ed;
    ExperimentEntry e1(1, "e1", ExperimentStatus::Active,   ExperimentType::BinaryAB);
    ExperimentEntry e2(2, "e2", ExperimentStatus::Active,   ExperimentType::Multivariate);
    ExperimentEntry e3(3, "e3", ExperimentStatus::Draft,    ExperimentType::BinaryAB);
    ExperimentEntry e4(4, "e4", ExperimentStatus::Archived, ExperimentType::Custom);
    ed.addExperiment(e1); ed.addExperiment(e2); ed.addExperiment(e3); ed.addExperiment(e4);
    REQUIRE(ed.countByStatus(ExperimentStatus::Active)   == 2u);
    REQUIRE(ed.countByStatus(ExperimentStatus::Draft)    == 1u);
    REQUIRE(ed.countByStatus(ExperimentStatus::Paused)   == 0u);
    REQUIRE(ed.countByType(ExperimentType::BinaryAB)     == 2u);
    REQUIRE(ed.countByType(ExperimentType::Multivariate) == 1u);
    REQUIRE(ed.countByType(ExperimentType::HoldOut)      == 0u);
    REQUIRE(ed.countActive()                             == 2u);
    auto* found = ed.findExperiment(3);
    REQUIRE(found != nullptr);
    REQUIRE(found->status() == ExperimentStatus::Draft);
    REQUIRE(ed.findExperiment(99) == nullptr);
}

TEST_CASE("ExperimentEditor settings mutation", "[Editor][S121]") {
    ExperimentEditor ed;
    ed.setIsShowArchived(true);
    ed.setIsGroupByType(true);
    ed.setMinTrafficPercent(15.0f);
    REQUIRE(ed.isShowArchived());
    REQUIRE(ed.isGroupByType());
    REQUIRE(ed.minTrafficPercent() == 15.0f);
}
