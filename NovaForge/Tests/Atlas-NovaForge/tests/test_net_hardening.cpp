#include <iostream>
#include <cassert>
#include <vector>
#include <string>

#include "../engine/net/NetHardening.h"

using namespace atlas::net;

void test_net_hardening_default_config() {
    NetHardening hardening;
    assert(hardening.State() == ConnectionState::Disconnected);
    assert(hardening.Stats().bytesSent == 0);
    assert(hardening.Stats().bytesReceived == 0);

    std::cout << "[PASS] test_net_hardening_default_config" << std::endl;
}

void test_net_hardening_configure() {
    NetHardening hardening;
    NetHardeningConfig config;
    config.connectionTimeoutMs = 5000;
    config.maxReconnectAttempts = 3;
    config.maxBandwidthBytesPerSec = 100000;
    hardening.Configure(config);

    assert(hardening.Config().connectionTimeoutMs == 5000);
    assert(hardening.Config().maxReconnectAttempts == 3);
    assert(hardening.Config().maxBandwidthBytesPerSec == 100000);

    std::cout << "[PASS] test_net_hardening_configure" << std::endl;
}

void test_net_hardening_connect_disconnect() {
    NetHardening hardening;
    hardening.Connect();
    assert(hardening.State() == ConnectionState::Connecting);

    hardening.Disconnect();
    assert(hardening.State() == ConnectionState::Disconnected);

    std::cout << "[PASS] test_net_hardening_connect_disconnect" << std::endl;
}

void test_net_hardening_connection_timeout() {
    NetHardening hardening;
    NetHardeningConfig config;
    config.connectionTimeoutMs = 100;
    config.maxReconnectAttempts = 0; // no reconnect
    hardening.Configure(config);

    hardening.Connect();
    assert(hardening.State() == ConnectionState::Connecting);

    // Simulate time passing beyond timeout
    hardening.Update(150.0f);
    assert(hardening.State() == ConnectionState::TimedOut);

    std::cout << "[PASS] test_net_hardening_connection_timeout" << std::endl;
}

void test_net_hardening_reconnect() {
    NetHardening hardening;
    NetHardeningConfig config;
    config.connectionTimeoutMs = 50;
    config.maxReconnectAttempts = 3;
    config.reconnectDelayMs = 100;
    hardening.Configure(config);

    hardening.Connect();
    assert(hardening.State() == ConnectionState::Connecting);

    // Timeout → should enter reconnecting state
    hardening.Update(60.0f);
    assert(hardening.State() == ConnectionState::Reconnecting);

    // After reconnect delay → back to connecting
    hardening.Update(110.0f);
    assert(hardening.State() == ConnectionState::Connecting);

    std::cout << "[PASS] test_net_hardening_reconnect" << std::endl;
}

void test_net_hardening_packet_received_connects() {
    NetHardening hardening;
    hardening.Connect();
    assert(hardening.State() == ConnectionState::Connecting);

    hardening.RecordPacketReceived();
    assert(hardening.State() == ConnectionState::Connected);

    std::cout << "[PASS] test_net_hardening_packet_received_connects" << std::endl;
}

void test_net_hardening_bandwidth_limit() {
    NetHardening hardening;
    NetHardeningConfig config;
    config.maxBandwidthBytesPerSec = 1000;
    hardening.Configure(config);

    assert(hardening.CanSendBytes(500));
    hardening.RecordBytesSent(800);
    assert(hardening.CanSendBytes(100));
    assert(!hardening.CanSendBytes(300));

    // After 1 second, budget should reset
    hardening.Update(1000.0f);
    assert(hardening.CanSendBytes(500));

    std::cout << "[PASS] test_net_hardening_bandwidth_limit" << std::endl;
}

void test_net_hardening_bandwidth_unlimited() {
    NetHardening hardening;
    NetHardeningConfig config;
    config.maxBandwidthBytesPerSec = 0; // unlimited
    hardening.Configure(config);

    assert(hardening.CanSendBytes(1000000));

    std::cout << "[PASS] test_net_hardening_bandwidth_unlimited" << std::endl;
}

void test_net_hardening_packet_size_validation() {
    NetHardening hardening;
    NetHardeningConfig config;
    config.maxPacketSize = 1400;
    hardening.Configure(config);

    assert(hardening.IsPacketSizeValid(1000));
    assert(hardening.IsPacketSizeValid(1400));
    assert(!hardening.IsPacketSizeValid(1401));

    std::cout << "[PASS] test_net_hardening_packet_size_validation" << std::endl;
}

void test_net_hardening_rtt_tracking() {
    NetHardening hardening;

    hardening.RecordRtt(50.0f);
    assert(hardening.Stats().averageRttMs > 49.0f && hardening.Stats().averageRttMs < 51.0f);
    assert(hardening.Stats().peakRttMs > 49.0f);

    hardening.RecordRtt(100.0f);
    assert(hardening.Stats().peakRttMs > 99.0f);
    // Average should be between 50 and 100
    assert(hardening.Stats().averageRttMs > 50.0f);

    std::cout << "[PASS] test_net_hardening_rtt_tracking" << std::endl;
}

void test_net_hardening_heartbeat_timeout() {
    NetHardening hardening;
    NetHardeningConfig config;
    config.heartbeatIntervalMs = 100;
    config.heartbeatMissThreshold = 3;
    config.maxReconnectAttempts = 0;
    hardening.Configure(config);

    hardening.Connect();
    hardening.RecordPacketReceived(); // connected
    assert(hardening.State() == ConnectionState::Connected);

    // Miss heartbeats beyond threshold (100ms * 3 = 300ms)
    hardening.Update(350.0f);
    assert(hardening.State() == ConnectionState::TimedOut);

    std::cout << "[PASS] test_net_hardening_heartbeat_timeout" << std::endl;
}

void test_net_hardening_heartbeat_reset() {
    NetHardening hardening;
    NetHardeningConfig config;
    config.heartbeatIntervalMs = 100;
    config.heartbeatMissThreshold = 3;
    hardening.Configure(config);

    hardening.Connect();
    hardening.RecordPacketReceived();
    assert(hardening.State() == ConnectionState::Connected);

    // Advance 200ms without heartbeat
    hardening.Update(200.0f);
    assert(hardening.State() == ConnectionState::Connected);

    // Receive heartbeat, resetting timer
    hardening.RecordHeartbeat();
    hardening.Update(200.0f);
    assert(hardening.State() == ConnectionState::Connected);

    std::cout << "[PASS] test_net_hardening_heartbeat_reset" << std::endl;
}

void test_net_hardening_stats_tracking() {
    NetHardening hardening;

    hardening.RecordBytesSent(100);
    hardening.RecordBytesSent(200);
    hardening.RecordBytesReceived(50);
    hardening.RecordPacketSent();
    hardening.RecordPacketSent();
    hardening.RecordPacketReceived();
    hardening.RecordPacketDropped();

    assert(hardening.Stats().bytesSent == 300);
    assert(hardening.Stats().bytesReceived == 50);
    assert(hardening.Stats().packetsSent == 2);
    assert(hardening.Stats().packetsReceived == 1);
    assert(hardening.Stats().packetsDropped == 1);

    std::cout << "[PASS] test_net_hardening_stats_tracking" << std::endl;
}

void test_net_hardening_reset_stats() {
    NetHardening hardening;
    hardening.RecordBytesSent(100);
    hardening.RecordPacketSent();
    hardening.Connect();

    hardening.ResetStats();
    assert(hardening.Stats().bytesSent == 0);
    assert(hardening.Stats().packetsSent == 0);
    // State should be preserved
    assert(hardening.State() == ConnectionState::Connecting);

    std::cout << "[PASS] test_net_hardening_reset_stats" << std::endl;
}

void test_net_hardening_state_callback() {
    NetHardening hardening;

    std::vector<std::pair<ConnectionState, ConnectionState>> transitions;
    hardening.SetStateCallback(
        [&transitions](ConnectionState oldState, ConnectionState newState) {
            transitions.push_back({oldState, newState});
        });

    hardening.Connect();
    assert(transitions.size() == 1);
    assert(transitions[0].first == ConnectionState::Disconnected);
    assert(transitions[0].second == ConnectionState::Connecting);

    hardening.Disconnect();
    assert(transitions.size() == 2);
    assert(transitions[1].second == ConnectionState::Disconnected);

    std::cout << "[PASS] test_net_hardening_state_callback" << std::endl;
}
