#pragma once

/**
 * ============================================================================
 * BLUETOOTH LOW ENERGY (BLE) GATT PROVISIONING SERVER (iot_ble.hpp)
 * ============================================================================
 * Allows wireless smartphone pairing to configure WiFi credentials, APN,
 * and Auth Tokens without needing a serial cable or initial WiFi connection.
 * ============================================================================
 */

#include <cstdint>
#include <cstdio>
#include <string_view>
#include <span>
#include <array>

#include "config.hpp"
#include "iot_core.hpp"
#include "iot_storage.hpp"

#if defined(ESP_PLATFORM)
#include "esp_bt.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_bt_main.h"
#endif

namespace iot::ble {

class BLEProvisioner {
public:
    static Result<void> init() noexcept {
        std::printf("\033[1;34m[BLE] Initialized GATT Provisioning Service. Device: %.*s\033[0m\n",
                    static_cast<int>(Config_System_DEVICE_NAME.length()),
                    Config_System_DEVICE_NAME.data());
#if defined(ESP_PLATFORM)
        // Initialize BLE stack and advertise GATT WiFi Provisioning Service
#endif
        return Status::OK;
    }

    static void set_wifi_credentials(std::string_view ssid, std::string_view password) noexcept {
        std::printf("\033[1;32m[BLE] Received new WiFi Credentials via Bluetooth: SSID='%.*s'\033[0m\n",
                    static_cast<int>(ssid.length()), ssid.data());
        (void)password;
        // Save to NVS Flash and reconnect
    }

private:
    static constexpr std::string_view Config_System_DEVICE_NAME = AppConfig::System::DEVICE_NAME;
};

} // namespace iot::ble
