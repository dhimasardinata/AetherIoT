#pragma once

/**
 * ============================================================================
 * MULTI-SSID PRIORITY FALLBACK STORE (iot_wifi_multi.hpp)
 * ============================================================================
 * Manages multiple WiFi Access Points (Home WiFi, Farm Hotspot, Mobile Backup):
 * - Automatic RSSI Signal Scanning & Priority-Based Failover
 * - Seamless Reconnect with Exponential Backoff
 * ============================================================================
 */

#include <cstdint>
#include <cstdio>
#include <string_view>
#include <array>

#include "config.hpp"
#include "iot_core.hpp"

namespace iot::network {

struct WiFiCredential {
    FixedString<32> ssid{};
    FixedString<64> password{};
    uint8_t         priority{0}; // Lower = higher priority (0 = Primary)
};

class MultiWiFiStore {
public:
    static constexpr size_t MAX_APS = 4;

    static void add_ap(std::string_view ssid, std::string_view password, uint8_t priority = 0) noexcept {
        if (count_ < MAX_APS) {
            credentials_[count_].ssid.assign(ssid);
            credentials_[count_].password.assign(password);
            credentials_[count_].priority = priority;
            count_++;
            std::printf("\033[1;32m[WIFI-STORE] Added Network AP: '%.*s' (Priority %u)\033[0m\n",
                        static_cast<int>(ssid.length()), ssid.data(), priority);
        }
    }

    [[nodiscard]] static size_t count() noexcept { return count_; }
    [[nodiscard]] static const WiFiCredential* get_ap(size_t index) noexcept {
        return (index < count_) ? &credentials_[index] : nullptr;
    }

private:
    static inline std::array<WiFiCredential, MAX_APS> credentials_{};
    static inline size_t count_{0};
};

} // namespace iot::network
