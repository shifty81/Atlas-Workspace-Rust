#include "SimMirror.h"

namespace atlas::sim {

void SimMirrorController::SetServer(ISimulation* server) {
    m_server = server;
}

void SimMirrorController::SetClient(ISimulation* client) {
    m_client = client;
}

bool SimMirrorController::Step(const std::vector<uint8_t>& inputFrame) {
    if (!m_enabled || !m_server || !m_client) return true;

    m_server->Step(inputFrame);
    m_client->Step(inputFrame);
    m_frameCount++;

    uint64_t serverHash = m_server->WorldHash();
    uint64_t clientHash = m_client->WorldHash();

    if (serverHash != clientHash) {
        MirrorDesyncEvent event;
        event.tick = m_server->CurrentTick();
        event.serverHash = serverHash;
        event.clientHash = clientHash;
        m_desyncs.push_back(event);

        if (m_desyncCallback) {
            m_desyncCallback(event);
        }
        return false;
    }
    return true;
}

uint64_t SimMirrorController::RunFrames(
    const std::vector<std::vector<uint8_t>>& inputs) {
    for (const auto& input : inputs) {
        if (!Step(input)) {
            return m_desyncs.back().tick;
        }
    }
    return 0;
}

bool SimMirrorController::HasDesync() const {
    return !m_desyncs.empty();
}

const std::vector<MirrorDesyncEvent>& SimMirrorController::Desyncs() const {
    return m_desyncs;
}

const MirrorDesyncEvent* SimMirrorController::FirstDesync() const {
    return m_desyncs.empty() ? nullptr : &m_desyncs.front();
}

uint64_t SimMirrorController::FrameCount() const {
    return m_frameCount;
}

void SimMirrorController::Reset() {
    m_desyncs.clear();
    m_frameCount = 0;
}

void SimMirrorController::SetDesyncCallback(DesyncCallback cb) {
    m_desyncCallback = std::move(cb);
}

bool SimMirrorController::IsEnabled() const {
    return m_enabled;
}

void SimMirrorController::SetEnabled(bool enabled) {
    m_enabled = enabled;
}

}  // namespace atlas::sim
