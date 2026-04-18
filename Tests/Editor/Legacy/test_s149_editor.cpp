// S149 editor tests: BuildPipelineEditorV1, CompilerSettingsV1, LinkerSettingsV1
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/LinkerSettingsV1.h"
#include "NF/Editor/CompilerSettingsV1.h"
#include "NF/Editor/BuildPipelineEditorV1.h"

using namespace NF;
using Catch::Approx;

// ── BuildPipelineEditorV1 ─────────────────────────────────────────────────────

TEST_CASE("Bpev1BuildTarget names", "[Editor][S149]") {
    REQUIRE(std::string(bpev1BuildTargetName(Bpev1BuildTarget::Game))     == "Game");
    REQUIRE(std::string(bpev1BuildTargetName(Bpev1BuildTarget::Editor))   == "Editor");
    REQUIRE(std::string(bpev1BuildTargetName(Bpev1BuildTarget::Server))   == "Server");
    REQUIRE(std::string(bpev1BuildTargetName(Bpev1BuildTarget::Client))   == "Client");
    REQUIRE(std::string(bpev1BuildTargetName(Bpev1BuildTarget::Shipping)) == "Shipping");
}

TEST_CASE("Bpev1BuildStep validity", "[Editor][S149]") {
    Bpev1BuildStep s;
    REQUIRE(!s.isValid());
    s.id = 1; s.name = "Compile";
    REQUIRE(s.isValid());
}

TEST_CASE("BuildPipelineEditorV1 addStep and stepCount", "[Editor][S149]") {
    BuildPipelineEditorV1 bpe;
    Bpev1BuildStep step; step.id = 1; step.name = "Preprocess";
    REQUIRE(bpe.addStep(step));
    REQUIRE(bpe.stepCount() == 1);
}

TEST_CASE("BuildPipelineEditorV1 reject duplicate step", "[Editor][S149]") {
    BuildPipelineEditorV1 bpe;
    Bpev1BuildStep step; step.id = 2; step.name = "Link";
    REQUIRE(bpe.addStep(step));
    REQUIRE(!bpe.addStep(step));
}

TEST_CASE("BuildPipelineEditorV1 removeStep", "[Editor][S149]") {
    BuildPipelineEditorV1 bpe;
    Bpev1BuildStep step; step.id = 3; step.name = "Package";
    bpe.addStep(step);
    REQUIRE(bpe.removeStep(3));
    REQUIRE(bpe.stepCount() == 0);
    REQUIRE(!bpe.removeStep(3));
}

TEST_CASE("BuildPipelineEditorV1 startBuild changes status", "[Editor][S149]") {
    BuildPipelineEditorV1 bpe;
    REQUIRE(bpe.currentStatus() == Bpev1BuildStatus::Idle);
    REQUIRE(bpe.startBuild(Bpev1BuildTarget::Editor));
    REQUIRE(bpe.currentStatus() == Bpev1BuildStatus::Preparing);
    REQUIRE(bpe.activeTarget() == Bpev1BuildTarget::Editor);
}

TEST_CASE("BuildPipelineEditorV1 cancelBuild sets Failed", "[Editor][S149]") {
    BuildPipelineEditorV1 bpe;
    bpe.startBuild(Bpev1BuildTarget::Game);
    bpe.cancelBuild();
    REQUIRE(bpe.currentStatus() == Bpev1BuildStatus::Failed);
}

TEST_CASE("BuildPipelineEditorV1 cancelBuild from Idle has no effect", "[Editor][S149]") {
    BuildPipelineEditorV1 bpe;
    bpe.cancelBuild();
    REQUIRE(bpe.currentStatus() == Bpev1BuildStatus::Idle);
}

TEST_CASE("BuildPipelineEditorV1 totalProgress", "[Editor][S149]") {
    BuildPipelineEditorV1 bpe;
    REQUIRE(bpe.totalProgress() == Approx(0.f));
    Bpev1BuildStep s1; s1.id = 1; s1.name = "A"; s1.progressPct = 50.f;
    Bpev1BuildStep s2; s2.id = 2; s2.name = "B"; s2.progressPct = 100.f;
    bpe.addStep(s1); bpe.addStep(s2);
    REQUIRE(bpe.totalProgress() == Approx(75.f));
}

TEST_CASE("BuildPipelineEditorV1 onStatusChange callback", "[Editor][S149]") {
    BuildPipelineEditorV1 bpe;
    Bpev1BuildStatus last = Bpev1BuildStatus::Idle;
    bpe.setOnStatusChange([&](Bpev1BuildStatus s){ last = s; });
    bpe.startBuild(Bpev1BuildTarget::Shipping);
    REQUIRE(last == Bpev1BuildStatus::Preparing);
}

// ── CompilerSettingsV1 ────────────────────────────────────────────────────────

TEST_CASE("CsOptLevel names", "[Editor][S149]") {
    REQUIRE(std::string(csOptLevelName(CsOptLevel::Debug)) == "Debug");
    REQUIRE(std::string(csOptLevelName(CsOptLevel::O1))    == "O1");
    REQUIRE(std::string(csOptLevelName(CsOptLevel::O2))    == "O2");
    REQUIRE(std::string(csOptLevelName(CsOptLevel::O3))    == "O3");
    REQUIRE(std::string(csOptLevelName(CsOptLevel::Size))  == "Size");
}

TEST_CASE("CompilerSettingsV1 addFlag and flagCount", "[Editor][S149]") {
    CompilerSettingsV1 cs;
    CsFlag f; f.id = 1; f.name = "-Wall";
    REQUIRE(cs.addFlag(f));
    REQUIRE(cs.flagCount() == 1);
}

TEST_CASE("CompilerSettingsV1 reject duplicate flag", "[Editor][S149]") {
    CompilerSettingsV1 cs;
    CsFlag f; f.id = 2; f.name = "-Wextra";
    REQUIRE(cs.addFlag(f));
    REQUIRE(!cs.addFlag(f));
}

TEST_CASE("CompilerSettingsV1 removeFlag", "[Editor][S149]") {
    CompilerSettingsV1 cs;
    CsFlag f; f.id = 3; f.name = "-std=c++17";
    cs.addFlag(f);
    REQUIRE(cs.removeFlag(3));
    REQUIRE(cs.flagCount() == 0);
}

TEST_CASE("CompilerSettingsV1 setOptLevel and generateCommandLine", "[Editor][S149]") {
    CompilerSettingsV1 cs;
    cs.setOptLevel(CsOptLevel::O2);
    REQUIRE(cs.getOptLevel() == CsOptLevel::O2);
    auto cmd = cs.generateCommandLine();
    REQUIRE(cmd.find("-O2") != std::string::npos);
}

TEST_CASE("CompilerSettingsV1 generateCommandLine with flags", "[Editor][S149]") {
    CompilerSettingsV1 cs;
    cs.setOptLevel(CsOptLevel::O3);
    CsFlag f; f.id = 1; f.name = "-DNDEBUG"; f.enabled = true;
    cs.addFlag(f);
    auto cmd = cs.generateCommandLine();
    REQUIRE(cmd.find("-O3") != std::string::npos);
    REQUIRE(cmd.find("-DNDEBUG") != std::string::npos);
}

TEST_CASE("CompilerSettingsV1 findFlag", "[Editor][S149]") {
    CompilerSettingsV1 cs;
    CsFlag f; f.id = 5; f.name = "-march=native";
    cs.addFlag(f);
    REQUIRE(cs.findFlag("-march=native") != nullptr);
    REQUIRE(cs.findFlag("-funroll") == nullptr);
}

// ── LinkerSettingsV1 ──────────────────────────────────────────────────────────

TEST_CASE("LsOutputType names", "[Editor][S149]") {
    REQUIRE(std::string(lsOutputTypeName(LsOutputType::Executable)) == "Executable");
    REQUIRE(std::string(lsOutputTypeName(LsOutputType::SharedLib))  == "SharedLib");
    REQUIRE(std::string(lsOutputTypeName(LsOutputType::StaticLib))  == "StaticLib");
    REQUIRE(std::string(lsOutputTypeName(LsOutputType::Module))     == "Module");
}

TEST_CASE("LinkerSettingsV1 addLibRef and libRefCount", "[Editor][S149]") {
    LinkerSettingsV1 ls;
    LsLibRef lib; lib.id = 1; lib.name = "pthread";
    REQUIRE(ls.addLibRef(lib));
    REQUIRE(ls.libRefCount() == 1);
}

TEST_CASE("LinkerSettingsV1 removeLibRef", "[Editor][S149]") {
    LinkerSettingsV1 ls;
    LsLibRef lib; lib.id = 2; lib.name = "dl";
    ls.addLibRef(lib);
    REQUIRE(ls.removeLibRef(2));
    REQUIRE(ls.libRefCount() == 0);
    REQUIRE(!ls.removeLibRef(2));
}

TEST_CASE("LinkerSettingsV1 setOutputType and getOutputType", "[Editor][S149]") {
    LinkerSettingsV1 ls;
    ls.setOutputType(LsOutputType::SharedLib);
    REQUIRE(ls.getOutputType() == LsOutputType::SharedLib);
}

TEST_CASE("LinkerSettingsV1 generateArgs with path and output", "[Editor][S149]") {
    LinkerSettingsV1 ls;
    LsLibRef lib; lib.id = 1; lib.name = "mylib"; lib.path = "/usr/local/lib";
    ls.addLibRef(lib);
    ls.setOutputPath("bin/app");
    auto args = ls.generateArgs();
    REQUIRE(args.find("-L/usr/local/lib") != std::string::npos);
    REQUIRE(args.find("-lmylib") != std::string::npos);
    REQUIRE(args.find("-o bin/app") != std::string::npos);
}

TEST_CASE("LinkerSettingsV1 setOutputPath", "[Editor][S149]") {
    LinkerSettingsV1 ls;
    ls.setOutputPath("out/editor");
    REQUIRE(ls.getOutputPath() == "out/editor");
}
