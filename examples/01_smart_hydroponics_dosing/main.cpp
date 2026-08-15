#include "../../iot_framework.hpp"

/**
 * ============================================================================
 * EXAMPLE 01: SMART HYDROPONICS AUTOMATED DOSING SYSTEM
 * ============================================================================
 * Target: ESP32 Classic / ESP32-S3 (Industrial Hydroponics Gateway)
 * Sensors:
 * - Industrial Modbus RS-485 pH (Nernst Temperature Compensated)
 * - Industrial Modbus RS-485 EC (Standard EC25 Temperature Compensated)
 * - Waterproof Ultrasonic Tank Level Sensor (A02YYUW / JSN-SR04T)
 * - Ambient SHT3x Temperature & Humidity
 * Actuators:
 * - Ch 1: Peristaltic Pump Nutrient A
 * - Ch 2: Peristaltic Pump Nutrient B
 * - Ch 3: Peristaltic Pump pH Down Acid
 * - Ch 4: Main Water Inlet Solenoid Valve
 * Security & Network:
 * - Decoupled AES-256-CBC Encryption + Anti-Replay Verification
 * ============================================================================
 */

namespace ProjectChannel {
    inline constexpr uint8_t NUTRIENT_A  = 1;
    inline constexpr uint8_t NUTRIENT_B  = 2;
    inline constexpr uint8_t PH_DOWN     = 3;
    inline constexpr uint8_t WATER_INLET = 4;
}

IOT_APP(app) {
    // 1. Configure Symmetric Encryption and Anti-Replay Protection
    iot::set_encryption(iot::Security::AES_256_CBC, "SecretKey256BitPanjang32Karakter!");
    iot::enable_anti_replay(true, 60);

    // 2. Multi-SSID Priority Fallback Configuration
    iot::add_wifi_ap("Hydroponic_Farm_Primary", "PasswordPrimary123", 0);
    iot::add_wifi_ap("Backup_Cellular_Hotspot", "BackupPassword123", 1);

    // 3. Declarative Fluent Safety Guards
    iot::guard("pH")
        .between(5.5f, 6.5f)
        .on_breach([](auto name, auto val) {
            iot::log_warn("GUARD [%s]: pH %.2f outside target optimal range (5.5 - 6.5)!", name.data(), val);
            if (val > 6.5f) {
                // Dispense 10 mL pH Down with daily budget quota protection
                iot::dose_ml(ProjectChannel::PH_DOWN, 10.0f);
            }
        });

    iot::guard("EC")
        .between(1200.0f, 2000.0f) // 1.2 - 2.0 mS/cm
        .on_breach([](auto name, auto val) {
            iot::log_warn("GUARD [%s]: EC %.0f uS/cm below target threshold!", name.data(), val);
            if (val < 1200.0f) {
                // Dispense 15 mL AB Mix Nutrients
                iot::dose_ml(ProjectChannel::NUTRIENT_A, 15.0f);
                iot::dose_ml(ProjectChannel::NUTRIENT_B, 15.0f);
            }
        });

    // 4. Real-time Telemetry Callback
    app.on_data([](auto data) {
        iot::log_info("Hydroponics | pH: %.2f | EC: %.0f uS/cm | Temp: %.1f C | Tank: %d%% (%.0f L)",
                      data.ph, data.ec, data.temp, data.tank_pct, data.volume);

        // Auto refill tank if water level drops below 20%
        if (data.tank_pct < 20 && !iot::is_on(ProjectChannel::WATER_INLET)) {
            iot::on(ProjectChannel::WATER_INLET);
            iot::log_info("Water tank level low -> Solenoid Water Inlet OPEN");
        } else if (data.tank_pct >= 90 && iot::is_on(ProjectChannel::WATER_INLET)) {
            iot::off(ProjectChannel::WATER_INLET);
            iot::log_info("Water tank full -> Solenoid Water Inlet CLOSED");
        }
    });
}
