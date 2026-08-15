#pragma once

/**
 * ============================================================================
 * MULTI-NODE CLUSTER REGISTRY (iot_mesh_registry.hpp)
 * ============================================================================
 * Tracks up to 16 remote field sensor nodes reporting via LoRa or ESP-NOW:
 * - Node ID, MAC Address, Last Seen Heartbeat, Battery %, RSSI (dBm), SNR (dB).
 * - Automatic Offline Node Detection & Alerting.
 * ============================================================================
 */

#include <cstdint>
#include <cstdio>
#include <string_view>
#include <array>

#include "config.hpp"
#include "iot_core.hpp"

namespace iot::mesh {

struct RemoteNodeEntry {
    uint16_t node_id{0};
    uint8_t  battery_pct{0};
    int8_t   rssi{0};
    float    snr{0.0f};
    uint32_t last_seen_ms{0};
    bool     online{false};
    float    last_val_1{0.0f};
    float    last_val_2{0.0f};
};

class ClusterRegistry {
public:
    static constexpr size_t MAX_NODES = 16;

    static void update_node(uint16_t node_id, uint8_t battery_pct, int8_t rssi, float snr, uint32_t now_ms, float v1 = 0.0f, float v2 = 0.0f) noexcept {
        for (size_t i = 0; i < count_; ++i) {
            if (nodes_[i].node_id == node_id) {
                nodes_[i].battery_pct = battery_pct;
                nodes_[i].rssi = rssi;
                nodes_[i].snr = snr;
                nodes_[i].last_seen_ms = now_ms;
                nodes_[i].online = true;
                nodes_[i].last_val_1 = v1;
                nodes_[i].last_val_2 = v2;
                return;
            }
        }

        if (count_ < MAX_NODES) {
            nodes_[count_] = RemoteNodeEntry{
                .node_id = node_id,
                .battery_pct = battery_pct,
                .rssi = rssi,
                .snr = snr,
                .last_seen_ms = now_ms,
                .online = true,
                .last_val_1 = v1,
                .last_val_2 = v2
            };
            count_++;
        }
    }

    static void evaluate_timeouts(uint32_t now_ms, uint32_t timeout_ms = 60000) noexcept {
        for (size_t i = 0; i < count_; ++i) {
            if (nodes_[i].online && (now_ms - nodes_[i].last_seen_ms > timeout_ms)) {
                nodes_[i].online = false;
                std::printf("\033[1;33m[CLUSTER] Node %u TIMED OUT (Offline)\033[0m\n", nodes_[i].node_id);
            }
        }
    }

    [[nodiscard]] static size_t node_count() noexcept { return count_; }
    [[nodiscard]] static const std::array<RemoteNodeEntry, MAX_NODES>& nodes() noexcept { return nodes_; }

private:
    static inline std::array<RemoteNodeEntry, MAX_NODES> nodes_{};
    static inline size_t count_{0};
};

} // namespace iot::mesh
