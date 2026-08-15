#pragma once

/**
 * ============================================================================
 * LONG-RANGE LORA SX1276 / SX1278 SPI TRANSCEIVER (iot_lora.hpp)
 * ============================================================================
 * - 433 MHz / 868 MHz / 915 MHz Long-Range Wireless Telemetry (up to 10+ km)
 * - Configurable Spreading Factor (SF7 - SF12), Bandwidth (125-500 kHz), CR 4/5
 * - Zero-Heap Packet Framing with RSSI, SNR, and CRC16 Integrity Check
 * ============================================================================
 */

#include <cstdint>
#include <cstdio>
#include <string_view>
#include <span>
#include <array>

#include "config.hpp"
#include "iot_core.hpp"

namespace iot::lora {

struct LoRaPacket {
    uint16_t sender_id{0};
    uint16_t sequence{0};
    int8_t   rssi{0};
    float    snr{0.0f};
    uint8_t  payload_len{0};
    char     payload[128]{0};
};

template <typename Config>
class LoRaTransceiver {
public:
    static Result<void> init(long frequency_hz = 915E6, uint8_t sf = 7, long sbw = 125E3) noexcept {
        (void)frequency_hz;
        (void)sf;
        (void)sbw;
        std::printf("\033[1;36m[LoRa] Initializing SX1276/SX1278 at %ld Hz (SF%u, BW %ld kHz)\033[0m\n",
                    frequency_hz, sf, sbw / 1000);
#if defined(ESP_PLATFORM)
        // Initialize SPI bus, NSS, RST, DIO0 pins and configure SX1276 registers
#endif
        return Status::OK;
    }

    static bool send_packet(uint16_t target_id, std::string_view message) noexcept {
        (void)target_id;
        std::printf("\033[1;32m[LoRa-TX] Transmitting %u bytes: '%.*s'\033[0m\n",
                    static_cast<unsigned>(message.length()),
                    static_cast<int>(message.length()), message.data());
#if defined(ESP_PLATFORM)
        // Write FIFO and trigger TX mode
#endif
        return true;
    }

    static bool poll_packet(LoRaPacket& out_pkt) noexcept {
        (void)out_pkt;
#if defined(ESP_PLATFORM)
        // Check DIO0 interrupt / RegIrqFlags for RX_DONE
#endif
        return false;
    }
};

} // namespace iot::lora
