#include "DesyncReproducer.h"
#include "SaveSystem.h"
#include <filesystem>
#include <fstream>

namespace atlas::sim {

void DesyncReproducer::SetOutputDirectory(const std::string& dir) {
    m_outputDir = dir;
}

const std::string& DesyncReproducer::OutputDirectory() const {
    return m_outputDir;
}

DesyncCapture DesyncReproducer::CaptureDesync(
    uint64_t tick,
    uint64_t localHash,
    uint64_t remoteHash,
    const std::vector<uint8_t>& ecsData,
    uint32_t tickRate,
    uint64_t seed,
    const std::vector<uint8_t>& replayData) {

    DesyncCapture capture;
    capture.tick = tick;
    capture.localHash = localHash;
    capture.remoteHash = remoteHash;

    std::filesystem::create_directories(m_outputDir);

    std::string prefix = m_outputDir + "/desync_tick_" + std::to_string(tick);

    // Write save file
    capture.savePath = prefix + ".asav";
    {
        SaveSystem saver;
        auto result = saver.Save(capture.savePath, tick, tickRate, seed, ecsData);
        if (result != SaveResult::Success) {
            capture.valid = false;
            m_captures.push_back(capture);
            return capture;
        }
    }

    // Write replay data
    capture.replayPath = prefix + ".rply";
    {
        std::ofstream out(capture.replayPath, std::ios::binary);
        if (!out.is_open()) {
            capture.valid = false;
            m_captures.push_back(capture);
            return capture;
        }
        out.write(reinterpret_cast<const char*>(replayData.data()),
                  static_cast<std::streamsize>(replayData.size()));
    }

    // Write human-readable report
    capture.reportPath = prefix + "_report.txt";
    {
        std::ofstream out(capture.reportPath);
        if (!out.is_open()) {
            capture.valid = false;
            m_captures.push_back(capture);
            return capture;
        }
        out << "=== Atlas Desync Report ===" << std::endl;
        out << "Tick:        " << tick << std::endl;
        out << "Local Hash:  0x" << std::hex << localHash << std::dec << std::endl;
        out << "Remote Hash: 0x" << std::hex << remoteHash << std::dec << std::endl;
        out << "Save File:   " << capture.savePath << std::endl;
        out << "Replay File: " << capture.replayPath << std::endl;
        out << std::endl;
        out << "Reproduce with:" << std::endl;
        out << "  " << GenerateReproCommand(capture) << std::endl;
    }

    capture.valid = true;
    m_captures.push_back(capture);
    return capture;
}

std::string DesyncReproducer::GenerateReproCommand(const DesyncCapture& capture) {
    return "./AtlasServer --repro --save \"" + capture.savePath +
           "\" --replay \"" + capture.replayPath + "\"";
}

size_t DesyncReproducer::CaptureCount() const {
    return m_captures.size();
}

const std::vector<DesyncCapture>& DesyncReproducer::Captures() const {
    return m_captures;
}

void DesyncReproducer::SetEngineVersion(const std::string& version) {
    m_engineVersion = version;
}

const std::string& DesyncReproducer::EngineVersion() const {
    return m_engineVersion;
}

void DesyncReproducer::SetPlatformId(const std::string& platformId) {
    m_platformId = platformId;
}

const std::string& DesyncReproducer::PlatformId() const {
    return m_platformId;
}

DesyncReproducer::CrashReportBundle DesyncReproducer::BuildCrashBundle(
        uint32_t tickRate, uint64_t seed) {
    if (m_captures.empty()) {
        CrashReportBundle bundle;
        bundle.valid = false;
        return bundle;
    }
    return BuildCrashBundleAt(m_captures.size() - 1, tickRate, seed);
}

DesyncReproducer::CrashReportBundle DesyncReproducer::BuildCrashBundleAt(
        size_t index, uint32_t tickRate, uint64_t seed) {
    CrashReportBundle bundle;
    if (index >= m_captures.size()) {
        bundle.valid = false;
        return bundle;
    }

    const auto& capture = m_captures[index];
    if (!capture.valid) {
        bundle.valid = false;
        return bundle;
    }

    bundle.capture = capture;
    bundle.engineVersion = m_engineVersion;
    bundle.platformId = m_platformId;
    bundle.tickRate = tickRate;
    bundle.seed = seed;

    // Write a .atlascrash manifest
    std::string manifestPath = m_outputDir + "/desync_tick_"
                               + std::to_string(capture.tick) + ".atlascrash";
    {
        std::ofstream out(manifestPath);
        if (!out.is_open()) {
            bundle.valid = false;
            return bundle;
        }
        out << "atlas_crash_bundle_v1" << std::endl;
        out << "engine_version=" << m_engineVersion << std::endl;
        out << "platform=" << m_platformId << std::endl;
        out << "tick=" << capture.tick << std::endl;
        out << "tick_rate=" << tickRate << std::endl;
        out << "seed=" << seed << std::endl;
        out << "local_hash=0x" << std::hex << capture.localHash << std::dec << std::endl;
        out << "remote_hash=0x" << std::hex << capture.remoteHash << std::dec << std::endl;
        out << "save=" << capture.savePath << std::endl;
        out << "replay=" << capture.replayPath << std::endl;
        out << "report=" << capture.reportPath << std::endl;
        out << "repro=" << GenerateReproCommand(capture) << std::endl;
    }

    bundle.bundlePath = manifestPath;
    bundle.valid = true;
    m_bundles.push_back(bundle);
    return bundle;
}

const std::vector<DesyncReproducer::CrashReportBundle>& DesyncReproducer::Bundles() const {
    return m_bundles;
}

}  // namespace atlas::sim
