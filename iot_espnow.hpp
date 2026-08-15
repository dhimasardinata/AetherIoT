#pragma once

/**
 * ============================================================================
 * ESP-NOW LOW-LATENCY WIRELESS MESH AGGREGATOR (iot_espnow.hpp)
 * ============================================================================
 * Ultra-fast sub-millisecond wireless telemetry packet ingestion.
 * ============================================================================
 */

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <span>
#include <array>

#include "config.hpp"
#include "iot_core.hpp"

#if defined(ESP_PLATFORM)
#include "esp_now.h"
#include "esp_wifi.h"
#endif

namespace iot::mesh {

struct __attribute__((packed)) ESPNowSensorPacket {
    uint8_t  preamble{0xAA};
    uint8_t  node_id{0};
    uint16_t battery_mv{3300};
    int16_t  temperature_centi_c{2500};
    int16_t  humidity_centi_rh{6000};
    int32_t  sensor_val_primary{0};
    uint16_t crc16{0};
};

template <typename Config>
class ESPNowAggregator {
public:
    static Result<void> init() noexcept {
        if constexpr (!Config::Features::ENABLE_ESP_NOW) return Status::OK;
#if defined(ESP_PLATFORM)
        if (esp_now_init() != ESP_OK) return Status::ERROR_HARDWARE_FAIL;
        esp_now_register_recv_cb(on_data_recv);
#endif
        return Status::OK;
    }

    static bool pop_packet(ESPNowSensorPacket& out) noexcept {
        return queue_.pop(out);
    }

private:
#if defined(ESP_PLATFORM)
    static void on_data_recv(const esp_now_recv_info_t* info, const uint8_t* data, int len) noexcept {
        (void)info;
        if (len == sizeof(ESPNowSensorPacket)) {
            ESPNowSensorPacket pkt{};
            std::memcpy(&pkt, data, sizeof(ESPNowSensorPacket));
            
            // Validate CRC16
            const uint16_t calc_crc = crc::calculate_crc16(
                std::span<const uint8_t>(reinterpret_cast<const uint8_t*>(&pkt), sizeof(ESPNowSensorPacket) - 2)
            );
            if (calc_crc == pkt.crc16 && pkt.preamble == 0xAA) {
                queue_.push(pkt);
            }
        }
    }
#endif

    static inline RingBuffer<ESPNowSensorPacket, 16> queue_{};
};

} // namespace iot::mesh
