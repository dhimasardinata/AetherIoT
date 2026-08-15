#include "../../iot_framework.hpp"

/**
 * ============================================================================
 * EXAMPLE 07: BLUETOOTH SERIAL MONITOR & WIRELESS BLE IN-FIELD OTA
 * ============================================================================
 * Target: ESP32 Classic / ESP32-S3 / ESP32-C3 / ESP32-C6
 * Features:
 * - Bluetooth Classic SPP & BLE Nordic UART Service (NUS) Serial Terminal.
 * - Wireless smartphone debug monitor & CLI configuration without Wi-Fi.
 * - BLE in-field firmware flashing via smartphone with partition rollback guard.
 * ============================================================================
 */

IOT_APP(app) {
    // 1. Welcome Message Dispatched to Technician Smartphone via Bluetooth Terminal
    iot::ble_println("==========================================");
    iot::ble_println("  AetherIoT Bluetooth Terminal Ready!     ");
    iot::ble_println("  Type 'help' for full CLI command suite  ");
    iot::ble_println("==========================================");

    // 2. Continuous Monitoring Stream Directly to Phone
    app.on_data([](auto data) {
        // Stream telemetry directly over BLE Nordic UART to technician phone
        iot::ble_printf("[BLE-STREAM] Temp: %.1f C | pH: %.2f | Bat: %u%%\r\n",
                        data.temp, data.ph, data.battery_pct);

        iot::log_info("Bluetooth Serial Terminal active. Stream connected to technician smartphone.");
    });
}
