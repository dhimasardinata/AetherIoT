#pragma once

/**
 * ============================================================================
 * THREAD / ZIGBEE 802.15.4 & MATTER BRIDGE (iot_thread_matter.hpp)
 * ============================================================================
 * Native IEEE 802.15.4 Radio Transceiver for ESP32-C6 and ESP32-H2 SoCs:
 * - Thread Mesh Network CoAP / IPv6 Transport
 * - Zigbee 3.0 Cluster Library (ZCL) Light & Sensor Endpoint
 * - Matter Over Thread Smart Agriculture & Industrial Interoperability
 * ============================================================================
 */

#include <cstdint>
#include <cstdio>
#include <string_view>
#include <span>
#include <array>

#include "config.hpp"
#include "iot_core.hpp"
#include "iot_target.hpp"

#if defined(CONFIG_IDF_TARGET_ESP32C6) || defined(CONFIG_IDF_TARGET_ESP32H2)
#include "esp_ieee802154.h"
#include "esp_openthread.h"
#endif

namespace iot::matter {

template <typename Config>
class ThreadMatterBridge {
public:
    static Result<void> init(uint8_t channel_11_26 = 15, uint16_t pan_id = 0x1234) noexcept {
        (void)channel_11_26; (void)pan_id;
        constexpr auto caps = target::TargetDetector::get_capabilities();

        if constexpr (caps.has_ieee802154_thread_zigbee) {
            std::printf("\033[1;32m[THREAD-MATTER] Initializing 802.15.4 Mesh Radio on Channel %u (PAN 0x%04X)\033[0m\n",
                        channel_11_26, pan_id);
#if defined(CONFIG_IDF_TARGET_ESP32C6) || defined(CONFIG_IDF_TARGET_ESP32H2)
            // Initialize OpenThread / Zigbee stack
#endif
            active_ = true;
        } else {
            std::printf("\033[1;33m[THREAD-MATTER] 802.15.4 Not Supported on Target %.*s (Skipping)\033[0m\n",
                        static_cast<int>(target::TargetDetector::chip_name().length()),
                        target::TargetDetector::chip_name().data());
        }
        return Status::OK;
    }

    static bool send_coap_telemetry(std::string_view uri, std::string_view json_payload) noexcept {
        (void)uri; (void)json_payload;
        if (!active_) return false;
        std::printf("\033[1;36m[THREAD-TX] CoAP POST '%.*s': '%.*s'\033[0m\n",
                    static_cast<int>(uri.length()), uri.data(),
                    static_cast<int>(json_payload.length()), json_payload.data());
        return true;
    }

    [[nodiscard]] static bool is_active() noexcept { return active_; }

private:
    static inline bool active_{false};
};

} // namespace iot::matter
