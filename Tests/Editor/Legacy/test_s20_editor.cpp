#include <catch2/catch_test_macros.hpp>
#include "NF/Editor/Editor.h"

using namespace NF;

// --- ResourceMonitorMetric names ---

TEST_CASE("ResourceMonitorMetric names cover all 8 values", "[Editor][S20]") {
    REQUIRE(std::string(resourceMonitorMetricName(ResourceMonitorMetric::CPU))        == "CPU");
    REQUIRE(std::string(resourceMonitorMetricName(ResourceMonitorMetric::GPU))        == "GPU");
    REQUIRE(std::string(resourceMonitorMetricName(ResourceMonitorMetric::Memory))     == "Memory");
    REQUIRE(std::string(resourceMonitorMetricName(ResourceMonitorMetric::DiskIO))     == "DiskIO");
    REQUIRE(std::string(resourceMonitorMetricName(ResourceMonitorMetric::NetworkIO))  == "NetworkIO");
    REQUIRE(std::string(resourceMonitorMetricName(ResourceMonitorMetric::FrameTime))  == "FrameTime");
    REQUIRE(std::string(resourceMonitorMetricName(ResourceMonitorMetric::DrawCalls))  == "DrawCalls");
    REQUIRE(std::string(resourceMonitorMetricName(ResourceMonitorMetric::ThreadLoad)) == "ThreadLoad");
}

// --- ResourceMonitorSample predicates ---

TEST_CASE("ResourceMonitorSample level predicates", "[Editor][S20]") {
    ResourceMonitorSample s;
    s.level = ResourceMonitorLevel::Normal;
    REQUIRE(s.isHealthy());
    REQUIRE_FALSE(s.isWarning());
    REQUIRE_FALSE(s.isCritical());
    REQUIRE_FALSE(s.isOverflow());

    s.level = ResourceMonitorLevel::Warning;
    REQUIRE(s.isWarning());

    s.level = ResourceMonitorLevel::Critical;
    REQUIRE(s.isCritical());

    s.level = ResourceMonitorLevel::Overflow;
    REQUIRE(s.isOverflow());
}

TEST_CASE("ResourceMonitorSample computeLevel thresholding", "[Editor][S20]") {
    ResourceMonitorSample s;
    s.value = 40.0f;
    REQUIRE(s.computeLevel(50.0f, 80.0f) == ResourceMonitorLevel::Normal);

    s.value = 60.0f;
    REQUIRE(s.computeLevel(50.0f, 80.0f) == ResourceMonitorLevel::Warning);

    s.value = 90.0f;
    REQUIRE(s.computeLevel(50.0f, 80.0f) == ResourceMonitorLevel::Critical);
}

// --- ResourceMonitorChannel ---

TEST_CASE("ResourceMonitorChannel push + wrong-metric rejection", "[Editor][S20]") {
    ResourceMonitorChannel ch(ResourceMonitorMetric::CPU);
    ResourceMonitorSample good; good.metric = ResourceMonitorMetric::CPU;  good.value = 30.0f;
    ResourceMonitorSample bad;  bad.metric  = ResourceMonitorMetric::GPU;  bad.value  = 50.0f;

    REQUIRE(ch.push(good));
    REQUIRE_FALSE(ch.push(bad));
    REQUIRE(ch.sampleCount() == 1);
}

TEST_CASE("ResourceMonitorChannel latest returns most recent sample", "[Editor][S20]") {
    ResourceMonitorChannel ch(ResourceMonitorMetric::Memory);
    REQUIRE(ch.latest() == nullptr);

    ResourceMonitorSample s1; s1.metric = ResourceMonitorMetric::Memory; s1.value = 10.0f;
    ResourceMonitorSample s2; s2.metric = ResourceMonitorMetric::Memory; s2.value = 20.0f;
    ch.push(s1);
    ch.push(s2);

    REQUIRE(ch.latest()->value == 20.0f);
}

TEST_CASE("ResourceMonitorChannel average + peak", "[Editor][S20]") {
    ResourceMonitorChannel ch(ResourceMonitorMetric::GPU);
    ResourceMonitorSample s1; s1.metric = ResourceMonitorMetric::GPU; s1.value = 10.0f;
    ResourceMonitorSample s2; s2.metric = ResourceMonitorMetric::GPU; s2.value = 30.0f;
    ch.push(s1);
    ch.push(s2);

    REQUIRE(ch.average() == 20.0f);
    REQUIRE(ch.peak()    == 30.0f);
}

TEST_CASE("ResourceMonitorChannel warningCount counts warning+critical", "[Editor][S20]") {
    ResourceMonitorChannel ch(ResourceMonitorMetric::CPU);
    ResourceMonitorSample n; n.metric = ResourceMonitorMetric::CPU; n.level = ResourceMonitorLevel::Normal;
    ResourceMonitorSample w; w.metric = ResourceMonitorMetric::CPU; w.level = ResourceMonitorLevel::Warning;
    ResourceMonitorSample c; c.metric = ResourceMonitorMetric::CPU; c.level = ResourceMonitorLevel::Critical;
    ch.push(n);
    ch.push(w);
    ch.push(c);

    REQUIRE(ch.warningCount() == 2);
}

TEST_CASE("ResourceMonitorChannel clear resets samples", "[Editor][S20]") {
    ResourceMonitorChannel ch(ResourceMonitorMetric::DiskIO);
    ResourceMonitorSample s; s.metric = ResourceMonitorMetric::DiskIO;
    ch.push(s);
    REQUIRE_FALSE(ch.empty());
    ch.clear();
    REQUIRE(ch.empty());
    REQUIRE(ch.sampleCount() == 0);
}

// --- ResourceMonitorSystem ---

TEST_CASE("ResourceMonitorSystem init creates 8 channels", "[Editor][S20]") {
    ResourceMonitorSystem sys;
    REQUIRE_FALSE(sys.isInitialized());
    sys.init();
    REQUIRE(sys.isInitialized());
    REQUIRE(sys.channelCount() == 8);
}

TEST_CASE("ResourceMonitorSystem shutdown clears channels", "[Editor][S20]") {
    ResourceMonitorSystem sys;
    sys.init();
    sys.shutdown();
    REQUIRE_FALSE(sys.isInitialized());
    REQUIRE(sys.channelCount() == 0);
}

TEST_CASE("ResourceMonitorSystem record blocked before init", "[Editor][S20]") {
    ResourceMonitorSystem sys;
    ResourceMonitorSample s; s.metric = ResourceMonitorMetric::CPU;
    REQUIRE_FALSE(sys.record(s));
}

TEST_CASE("ResourceMonitorSystem record routes to correct channel", "[Editor][S20]") {
    ResourceMonitorSystem sys;
    sys.init();

    ResourceMonitorSample s; s.metric = ResourceMonitorMetric::CPU; s.value = 55.0f;
    REQUIRE(sys.record(s));

    auto* ch = sys.channelFor(ResourceMonitorMetric::CPU);
    REQUIRE(ch != nullptr);
    REQUIRE(ch->sampleCount() == 1);
    REQUIRE(ch->latest()->value == 55.0f);
}

TEST_CASE("ResourceMonitorSystem totalSamples + totalWarnings", "[Editor][S20]") {
    ResourceMonitorSystem sys;
    sys.init();

    ResourceMonitorSample s1; s1.metric = ResourceMonitorMetric::CPU;    s1.level = ResourceMonitorLevel::Normal;
    ResourceMonitorSample s2; s2.metric = ResourceMonitorMetric::GPU;    s2.level = ResourceMonitorLevel::Warning;
    ResourceMonitorSample s3; s3.metric = ResourceMonitorMetric::Memory; s3.level = ResourceMonitorLevel::Critical;
    sys.record(s1);
    sys.record(s2);
    sys.record(s3);

    REQUIRE(sys.totalSamples()  == 3);
    REQUIRE(sys.totalWarnings() == 2);
}

TEST_CASE("ResourceMonitorSystem clearAll resets all channels", "[Editor][S20]") {
    ResourceMonitorSystem sys;
    sys.init();

    ResourceMonitorSample s; s.metric = ResourceMonitorMetric::CPU;
    sys.record(s);
    REQUIRE(sys.totalSamples() == 1);
    sys.clearAll();
    REQUIRE(sys.totalSamples() == 0);
}
