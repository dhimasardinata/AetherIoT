#include "../../iot_framework.hpp"

/**
 * ============================================================================
 * EXAMPLE 03: INDUSTRIAL AC ENERGY MONITORING PZEM GATEWAY
 * ============================================================================
 * Target: ESP32-S2 / ESP32 Classic (Factory AC Power Quality Monitor)
 * Sensors:
 * - Modbus RTU PZEM-004T AC Power Meter (Voltage, Current, Active Power, Energy kWh)
 * Storage & Security:
 * - MicroSD FAT32 CSV Daily Datalogger (/logs/YYYY-MM-DD.csv)
 * - Decoupled AES-256-CBC Encrypted WebSocket Real-time Telemetry
 * Safety:
 * - Voltage Sag/Swell Anomaly Protection + Overcurrent Safety Tripping
 * ============================================================================
 */

IOT_APP(app) {
    // 1. Configure Symmetric Encryption
    iot::set_encryption(iot::Security::AES_256_CBC, "IndustrialEnergyKey32BytesLong!!");
    iot::enable_anti_replay(true, 30);

    // 2. Voltage & Current Safety Range Guards
    iot::guard("ACVoltage")
        .between(180.0f, 250.0f) // 220V nominal (+/- 15%)
        .on_breach([](auto name, auto val) {
            iot::log_error("POWER QUALITY ANOMALY [%s]: Voltage %.1f V outside safe threshold (180-250V)!", name.data(), val);
        });

    iot::guard("ACCurrent")
        .max(30.0f) // Max 30 Ampere breaker rating
        .on_breach([](auto name, auto val) {
            iot::log_error("OVERCURRENT BREAKER TRIP [%s]: Load Current %.2f A exceeded 30A limit! Disconnecting...", name.data(), val);
            iot::emergency_stop(); // Instant safety interlock shutdown
        });

    // 3. Telemetry Stream & CSV Logging
    app.on_data([](auto data) {
        iot::log_info("AC Energy Meter | V: %.1f V | I: %.2f A | P: %.1f W | Total: %.2f kWh",
                      data.voltage, data.current, data.power, data.energy);

        // Write summary CSV record to MicroSD card
        iot::FixedString<128> csv_row;
        csv_row.format("ENERGY,%.1f,%.2f,%.1f,%.2f\n", data.voltage, data.current, data.power, data.energy);
        iot::log_sd(csv_row.string_view());
    });
}
