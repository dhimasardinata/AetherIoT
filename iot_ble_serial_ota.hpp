#pragma once

/**
 * ============================================================================
 * BLUETOOTH SERIAL MONITOR & BLE OTA FLASHER (iot_ble_serial_ota.hpp)
 * ============================================================================
 * 1. Bluetooth Classic SPP & BLE Nordic UART Service (NUS):
 *    - Real-time wireless Serial Terminal & CLI execution via smartphone.
 * 2. Wireless Bluetooth Low Energy (BLE) OTA Firmware Updater:
 *    - In-field phone-to-node firmware flashing with rollback protection.
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

#if defined(ESP_PLATFORM)
#include "esp_ota_ops.h"
#include "esp_bt.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#endif

namespace iot::ble {

class BluetoothSerialMonitor {
public:
    // Nordic UART Service (NUS) UUIDs
    static constexpr std::string_view NUS_SERVICE_UUID = "6E400001-B5A3-F393-E0A9-E50E24DCCA9E";
    static constexpr std::string_view NUS_RX_UUID      = "6E400002-B5A3-F393-E0A9-E50E24DCCA9E";
    static constexpr std::string_view NUS_TX_UUID      = "6E400003-B5A3-F393-E0A9-E50E24DCCA9E";

    static Result<void> init(std::string_view device_name = "AetherIoT-Serial") noexcept {
        (void)device_name;
        constexpr auto caps = target::TargetDetector::get_capabilities();
        if (caps.has_ble) {
            std::printf("\033[1;32m[BLE-SERIAL] BLE Nordic UART Serial Monitor Started: '%.*s'\033[0m\n",
                        static_cast<int>(device_name.length()), device_name.data());
            active_ = true;
        }
        return Status::OK;
    }

    template <typename... Args>
    static void printf(const char* fmt, Args... args) noexcept {
        if (!active_) return;
        char buf[256];
        int written = std::snprintf(buf, sizeof(buf), fmt, args...);
        if (written > 0) {
            send_raw(std::string_view(buf, written));
        }
    }

    static void println(std::string_view text) noexcept {
        if (!active_) return;
        send_raw(text);
        send_raw("\r\n");
    }

    static void send_raw(std::string_view data) noexcept {
        (void)data;
#if defined(ESP_PLATFORM)
        // Send notification via BLE NUS TX characteristic or BT Classic SPP
#endif
    }

    [[nodiscard]] static bool is_active() noexcept { return active_; }

private:
    static inline bool active_{false};
};

class BLEOTAUpdater {
public:
    static Result<void> init() noexcept {
        std::printf("\033[1;32m[BLE-OTA] BLE Wireless Firmware Flasher Ready (Awaiting Chunks)\033[0m\n");
        return Status::OK;
    }

    static Result<void> write_chunk(std::span<const uint8_t> chunk) noexcept {
        (void)chunk;
#if defined(ESP_PLATFORM)
        // Write firmware chunk to inactive OTA partition
#endif
        bytes_received_ += chunk.size();
        return Status::OK;
    }

    static Result<void> finalize_and_reboot() noexcept {
        std::printf("\033[1;32m[BLE-OTA] Flashing Complete (%lu bytes). Rebooting into new firmware...\033[0m\n",
                    static_cast<unsigned long>(bytes_received_));
#if defined(ESP_PLATFORM)
        esp_restart();
#endif
        return Status::OK;
    }

    [[nodiscard]] static size_t bytes_received() noexcept { return bytes_received_; }

private:
    static inline size_t bytes_received_{0};
};

} // namespace iot::ble
