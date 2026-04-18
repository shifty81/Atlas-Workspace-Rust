// S135 editor tests: ProfilingSessionEditor, CpuProfilerEditor, GpuProfilerEditor
#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"
#include "NF/Editor/GpuProfilerEditor.h"
#include "NF/Editor/CpuProfilerEditor.h"
#include "NF/Editor/ProfilingSessionEditor.h"

using namespace NF;

// ── ProfilingSessionEditor ────────────────────────────────────────────────────

TEST_CASE("ProfSessionMode names", "[Editor][S135]") {
    REQUIRE(std::string(profSessionModeName(ProfSessionMode::Realtime)) == "Realtime");
    REQUIRE(std::string(profSessionModeName(ProfSessionMode::Capture))  == "Capture");
    REQUIRE(std::string(profSessionModeName(ProfSessionMode::Replay))   == "Replay");
    REQUIRE(std::string(profSessionModeName(ProfSessionMode::Compare))  == "Compare");
    REQUIRE(std::string(profSessionModeName(ProfSessionMode::Export))   == "Export");
}

TEST_CASE("ProfSessionState names", "[Editor][S135]") {
    REQUIRE(std::string(profSessionStateName(ProfSessionState::Idle))      == "Idle");
    REQUIRE(std::string(profSessionStateName(ProfSessionState::Running))   == "Running");
    REQUIRE(std::string(profSessionStateName(ProfSessionState::Paused))    == "Paused");
    REQUIRE(std::string(profSessionStateName(ProfSessionState::Stopped))   == "Stopped");
    REQUIRE(std::string(profSessionStateName(ProfSessionState::Analyzing)) == "Analyzing");
}

TEST_CASE("ProfilingSession defaults", "[Editor][S135]") {
    ProfilingSession s(1, "frame_capture", ProfSessionMode::Capture, ProfSessionState::Idle);
    REQUIRE(s.id()              == 1u);
    REQUIRE(s.name()            == "frame_capture");
    REQUIRE(s.mode()            == ProfSessionMode::Capture);
    REQUIRE(s.state()           == ProfSessionState::Idle);
    REQUIRE(s.maxDurationSecs() == 60.0f);
    REQUIRE(s.sampleRateHz()    == 60u);
    REQUIRE(s.isEnabled());
}

TEST_CASE("ProfilingSession mutation", "[Editor][S135]") {
    ProfilingSession s(2, "long_run", ProfSessionMode::Realtime, ProfSessionState::Running);
    s.setMaxDurationSecs(300.0f);
    s.setSampleRateHz(120u);
    s.setIsEnabled(false);
    REQUIRE(s.maxDurationSecs() == 300.0f);
    REQUIRE(s.sampleRateHz()    == 120u);
    REQUIRE(!s.isEnabled());
}

TEST_CASE("ProfilingSessionEditor defaults", "[Editor][S135]") {
    ProfilingSessionEditor ed;
    REQUIRE(!ed.isShowDisabled());
    REQUIRE(!ed.isGroupByMode());
    REQUIRE(ed.defaultSampleRateHz() == 120u);
    REQUIRE(ed.sessionCount()        == 0u);
}

TEST_CASE("ProfilingSessionEditor add/remove sessions", "[Editor][S135]") {
    ProfilingSessionEditor ed;
    REQUIRE(ed.addSession(ProfilingSession(1, "s_a", ProfSessionMode::Capture,  ProfSessionState::Idle)));
    REQUIRE(ed.addSession(ProfilingSession(2, "s_b", ProfSessionMode::Realtime, ProfSessionState::Running)));
    REQUIRE(ed.addSession(ProfilingSession(3, "s_c", ProfSessionMode::Replay,   ProfSessionState::Stopped)));
    REQUIRE(!ed.addSession(ProfilingSession(1, "s_a", ProfSessionMode::Capture, ProfSessionState::Idle)));
    REQUIRE(ed.sessionCount() == 3u);
    REQUIRE(ed.removeSession(2));
    REQUIRE(ed.sessionCount() == 2u);
    REQUIRE(!ed.removeSession(99));
}

TEST_CASE("ProfilingSessionEditor counts and find", "[Editor][S135]") {
    ProfilingSessionEditor ed;
    ProfilingSession s1(1, "a", ProfSessionMode::Capture,  ProfSessionState::Idle);
    ProfilingSession s2(2, "b", ProfSessionMode::Capture,  ProfSessionState::Running);
    ProfilingSession s3(3, "c", ProfSessionMode::Realtime, ProfSessionState::Paused);
    ProfilingSession s4(4, "d", ProfSessionMode::Export,   ProfSessionState::Stopped); s4.setIsEnabled(false);
    ed.addSession(s1); ed.addSession(s2); ed.addSession(s3); ed.addSession(s4);
    REQUIRE(ed.countByMode(ProfSessionMode::Capture)          == 2u);
    REQUIRE(ed.countByMode(ProfSessionMode::Realtime)         == 1u);
    REQUIRE(ed.countByMode(ProfSessionMode::Compare)          == 0u);
    REQUIRE(ed.countByState(ProfSessionState::Idle)           == 1u);
    REQUIRE(ed.countByState(ProfSessionState::Running)        == 1u);
    REQUIRE(ed.countEnabled()                                 == 3u);
    auto* found = ed.findSession(3);
    REQUIRE(found != nullptr);
    REQUIRE(found->mode() == ProfSessionMode::Realtime);
    REQUIRE(ed.findSession(99) == nullptr);
}

TEST_CASE("ProfilingSessionEditor settings mutation", "[Editor][S135]") {
    ProfilingSessionEditor ed;
    ed.setIsShowDisabled(true);
    ed.setIsGroupByMode(true);
    ed.setDefaultSampleRateHz(240u);
    REQUIRE(ed.isShowDisabled());
    REQUIRE(ed.isGroupByMode());
    REQUIRE(ed.defaultSampleRateHz() == 240u);
}

// ── CpuProfilerEditor ─────────────────────────────────────────────────────────

TEST_CASE("CpuProfScope names", "[Editor][S135]") {
    REQUIRE(std::string(cpuProfScopeName(CpuProfScope::Function)) == "Function");
    REQUIRE(std::string(cpuProfScopeName(CpuProfScope::Block))    == "Block");
    REQUIRE(std::string(cpuProfScopeName(CpuProfScope::Thread))   == "Thread");
    REQUIRE(std::string(cpuProfScopeName(CpuProfScope::Frame))    == "Frame");
    REQUIRE(std::string(cpuProfScopeName(CpuProfScope::Custom))   == "Custom");
}

TEST_CASE("CpuProfSampler names", "[Editor][S135]") {
    REQUIRE(std::string(cpuProfSamplerName(CpuProfSampler::Instrumented)) == "Instrumented");
    REQUIRE(std::string(cpuProfSamplerName(CpuProfSampler::Statistical))  == "Statistical");
    REQUIRE(std::string(cpuProfSamplerName(CpuProfSampler::Hardware))     == "Hardware");
    REQUIRE(std::string(cpuProfSamplerName(CpuProfSampler::Hybrid))       == "Hybrid");
}

TEST_CASE("CpuProfilerConfig defaults", "[Editor][S135]") {
    CpuProfilerConfig c(1, "main_thread", CpuProfScope::Thread, CpuProfSampler::Instrumented);
    REQUIRE(c.id()            == 1u);
    REQUIRE(c.name()          == "main_thread");
    REQUIRE(c.scope()         == CpuProfScope::Thread);
    REQUIRE(c.sampler()       == CpuProfSampler::Instrumented);
    REQUIRE(c.maxCallDepth()  == 64u);
    REQUIRE(!c.includeKernel());
    REQUIRE(c.isEnabled());
}

TEST_CASE("CpuProfilerConfig mutation", "[Editor][S135]") {
    CpuProfilerConfig c(2, "full_trace", CpuProfScope::Function, CpuProfSampler::Statistical);
    c.setMaxCallDepth(128u);
    c.setIncludeKernel(true);
    c.setIsEnabled(false);
    REQUIRE(c.maxCallDepth()  == 128u);
    REQUIRE(c.includeKernel());
    REQUIRE(!c.isEnabled());
}

TEST_CASE("CpuProfilerEditor defaults", "[Editor][S135]") {
    CpuProfilerEditor ed;
    REQUIRE(!ed.isShowDisabled());
    REQUIRE(ed.isGroupByScope());
    REQUIRE(ed.defaultMaxCallDepth() == 32u);
    REQUIRE(ed.configCount()         == 0u);
}

TEST_CASE("CpuProfilerEditor add/remove configs", "[Editor][S135]") {
    CpuProfilerEditor ed;
    REQUIRE(ed.addConfig(CpuProfilerConfig(1, "c_a", CpuProfScope::Thread,   CpuProfSampler::Instrumented)));
    REQUIRE(ed.addConfig(CpuProfilerConfig(2, "c_b", CpuProfScope::Function, CpuProfSampler::Statistical)));
    REQUIRE(ed.addConfig(CpuProfilerConfig(3, "c_c", CpuProfScope::Frame,    CpuProfSampler::Hardware)));
    REQUIRE(!ed.addConfig(CpuProfilerConfig(1, "c_a", CpuProfScope::Thread,  CpuProfSampler::Instrumented)));
    REQUIRE(ed.configCount() == 3u);
    REQUIRE(ed.removeConfig(2));
    REQUIRE(ed.configCount() == 2u);
    REQUIRE(!ed.removeConfig(99));
}

TEST_CASE("CpuProfilerEditor counts and find", "[Editor][S135]") {
    CpuProfilerEditor ed;
    CpuProfilerConfig c1(1, "a", CpuProfScope::Thread,   CpuProfSampler::Instrumented);
    CpuProfilerConfig c2(2, "b", CpuProfScope::Thread,   CpuProfSampler::Statistical);
    CpuProfilerConfig c3(3, "c", CpuProfScope::Function, CpuProfSampler::Hardware);
    CpuProfilerConfig c4(4, "d", CpuProfScope::Custom,   CpuProfSampler::Hybrid); c4.setIsEnabled(false);
    ed.addConfig(c1); ed.addConfig(c2); ed.addConfig(c3); ed.addConfig(c4);
    REQUIRE(ed.countByScope(CpuProfScope::Thread)               == 2u);
    REQUIRE(ed.countByScope(CpuProfScope::Function)             == 1u);
    REQUIRE(ed.countByScope(CpuProfScope::Block)                == 0u);
    REQUIRE(ed.countBySampler(CpuProfSampler::Instrumented)     == 1u);
    REQUIRE(ed.countBySampler(CpuProfSampler::Statistical)      == 1u);
    REQUIRE(ed.countEnabled()                                   == 3u);
    auto* found = ed.findConfig(3);
    REQUIRE(found != nullptr);
    REQUIRE(found->scope() == CpuProfScope::Function);
    REQUIRE(ed.findConfig(99) == nullptr);
}

TEST_CASE("CpuProfilerEditor settings mutation", "[Editor][S135]") {
    CpuProfilerEditor ed;
    ed.setIsShowDisabled(true);
    ed.setIsGroupByScope(false);
    ed.setDefaultMaxCallDepth(64u);
    REQUIRE(ed.isShowDisabled());
    REQUIRE(!ed.isGroupByScope());
    REQUIRE(ed.defaultMaxCallDepth() == 64u);
}

// ── GpuProfilerEditor ─────────────────────────────────────────────────────────

TEST_CASE("GpuProfPass names", "[Editor][S135]") {
    REQUIRE(std::string(gpuProfPassName(GpuProfPass::Shadow))      == "Shadow");
    REQUIRE(std::string(gpuProfPassName(GpuProfPass::Depth))       == "Depth");
    REQUIRE(std::string(gpuProfPassName(GpuProfPass::GBuffer))     == "GBuffer");
    REQUIRE(std::string(gpuProfPassName(GpuProfPass::Lighting))    == "Lighting");
    REQUIRE(std::string(gpuProfPassName(GpuProfPass::PostProcess)) == "PostProcess");
    REQUIRE(std::string(gpuProfPassName(GpuProfPass::UI))          == "UI");
}

TEST_CASE("GpuProfCounter names", "[Editor][S135]") {
    REQUIRE(std::string(gpuProfCounterName(GpuProfCounter::DrawCalls))         == "DrawCalls");
    REQUIRE(std::string(gpuProfCounterName(GpuProfCounter::Triangles))         == "Triangles");
    REQUIRE(std::string(gpuProfCounterName(GpuProfCounter::Pixels))            == "Pixels");
    REQUIRE(std::string(gpuProfCounterName(GpuProfCounter::ComputeDispatches)) == "ComputeDispatches");
    REQUIRE(std::string(gpuProfCounterName(GpuProfCounter::MemBandwidth))      == "MemBandwidth");
}

TEST_CASE("GpuProfilerConfig defaults", "[Editor][S135]") {
    GpuProfilerConfig c(1, "gbuffer_pass", GpuProfPass::GBuffer, GpuProfCounter::DrawCalls);
    REQUIRE(c.id()             == 1u);
    REQUIRE(c.name()           == "gbuffer_pass");
    REQUIRE(c.pass()           == GpuProfPass::GBuffer);
    REQUIRE(c.counter()        == GpuProfCounter::DrawCalls);
    REQUIRE(c.maxMarkerDepth() == 32u);
    REQUIRE(c.includeCompute());
    REQUIRE(c.isEnabled());
}

TEST_CASE("GpuProfilerConfig mutation", "[Editor][S135]") {
    GpuProfilerConfig c(2, "lighting", GpuProfPass::Lighting, GpuProfCounter::Triangles);
    c.setMaxMarkerDepth(16u);
    c.setIncludeCompute(false);
    c.setIsEnabled(false);
    REQUIRE(c.maxMarkerDepth() == 16u);
    REQUIRE(!c.includeCompute());
    REQUIRE(!c.isEnabled());
}

TEST_CASE("GpuProfilerEditor defaults", "[Editor][S135]") {
    GpuProfilerEditor ed;
    REQUIRE(!ed.isShowDisabled());
    REQUIRE(ed.isGroupByPass());
    REQUIRE(ed.defaultMaxMarkerDepth() == 16u);
    REQUIRE(ed.configCount()           == 0u);
}

TEST_CASE("GpuProfilerEditor add/remove configs", "[Editor][S135]") {
    GpuProfilerEditor ed;
    REQUIRE(ed.addConfig(GpuProfilerConfig(1, "c_a", GpuProfPass::Shadow,   GpuProfCounter::DrawCalls)));
    REQUIRE(ed.addConfig(GpuProfilerConfig(2, "c_b", GpuProfPass::GBuffer,  GpuProfCounter::Triangles)));
    REQUIRE(ed.addConfig(GpuProfilerConfig(3, "c_c", GpuProfPass::Lighting, GpuProfCounter::Pixels)));
    REQUIRE(!ed.addConfig(GpuProfilerConfig(1, "c_a", GpuProfPass::Shadow,  GpuProfCounter::DrawCalls)));
    REQUIRE(ed.configCount() == 3u);
    REQUIRE(ed.removeConfig(2));
    REQUIRE(ed.configCount() == 2u);
    REQUIRE(!ed.removeConfig(99));
}

TEST_CASE("GpuProfilerEditor counts and find", "[Editor][S135]") {
    GpuProfilerEditor ed;
    GpuProfilerConfig c1(1, "a", GpuProfPass::Shadow,      GpuProfCounter::DrawCalls);
    GpuProfilerConfig c2(2, "b", GpuProfPass::Shadow,      GpuProfCounter::Triangles);
    GpuProfilerConfig c3(3, "c", GpuProfPass::Lighting,    GpuProfCounter::Pixels);
    GpuProfilerConfig c4(4, "d", GpuProfPass::PostProcess, GpuProfCounter::MemBandwidth); c4.setIsEnabled(false);
    ed.addConfig(c1); ed.addConfig(c2); ed.addConfig(c3); ed.addConfig(c4);
    REQUIRE(ed.countByPass(GpuProfPass::Shadow)                         == 2u);
    REQUIRE(ed.countByPass(GpuProfPass::Lighting)                       == 1u);
    REQUIRE(ed.countByPass(GpuProfPass::Depth)                          == 0u);
    REQUIRE(ed.countByCounter(GpuProfCounter::DrawCalls)                == 1u);
    REQUIRE(ed.countByCounter(GpuProfCounter::Triangles)                == 1u);
    REQUIRE(ed.countEnabled()                                           == 3u);
    auto* found = ed.findConfig(3);
    REQUIRE(found != nullptr);
    REQUIRE(found->pass() == GpuProfPass::Lighting);
    REQUIRE(ed.findConfig(99) == nullptr);
}

TEST_CASE("GpuProfilerEditor settings mutation", "[Editor][S135]") {
    GpuProfilerEditor ed;
    ed.setIsShowDisabled(true);
    ed.setIsGroupByPass(false);
    ed.setDefaultMaxMarkerDepth(64u);
    REQUIRE(ed.isShowDisabled());
    REQUIRE(!ed.isGroupByPass());
    REQUIRE(ed.defaultMaxMarkerDepth() == 64u);
}
