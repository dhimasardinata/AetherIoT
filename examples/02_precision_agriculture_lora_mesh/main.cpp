#include "../../iot_framework.hpp"

/**
 * ============================================================================
 * EXAMPLE 02: SOLAR-POWERED PRECISION AGRICULTURE LORA MESH NODE
 * ============================================================================
 * Target: ESP32-C3 / ESP32 Classic (Ultra-Low-Power Outdoor Node)
 * Sensors:
 * - RS-485 Modbus 7-in-1 Soil Multi-Parameter Probe (N, P, K, pH, EC, Temp, Hum)
 * - MPU6050 6-Axis IMU (Pole Structural Tilt & Vibration Anomaly Guard)
 * - INA219 Solar Panel Voltage & Battery Fuel Gauge
 * Telemetry:
 * - Long-Range SPI LoRa 915 MHz SX1276 Star/Mesh Network Broadcast
 * - Ultra-Low-Power Deep Sleep (< 15 uA current consumption)
 * ============================================================================
 */

IOT_APP(app) {
    // 1. Pole Tilt Guard (Structural damage alert if pole tilts > 25 degrees)
    iot::guard("PoleTilt")
        .max(25.0f) // degrees
        .on_breach([](auto name, auto val) {
            iot::log_error("CRITICAL STRUCTURAL ALERT [%s]: Field Pole Tilted %.1f deg! Possible fallen pole or landslide.", name.data(), val);
        });

    // 2. Process Telemetry, Transmit LoRa Packet, and Enter Deep Sleep
    app.on_data([](auto data) {
        iot::log_info("Solar Agri Node | Bat: %u%% (%.2fV) | NPK: [%d, %d, %d] | Soil Temp: %.1f C | Tilt: %.1f deg",
                      data.battery_pct, data.voltage, data.nitrogen, data.phosphorus, data.potassium, data.soil_temp, data.tilt_pitch);

        // Format and transmit telemetry packet via LoRa to Base Gateway (Node ID 1)
        iot::FixedString<128> packet;
        packet.format("N:%d,P:%d,K:%d,BAT:%u,TILT:%.1f",
                      data.nitrogen, data.phosphorus, data.potassium, data.battery_pct, data.tilt_pitch);

        iot::lora_send(1, packet.string_view());
        iot::log_info("LoRa Telemetry Dispatched -> Payload: '%.*s'", static_cast<int>(packet.length()), packet.data());

        // Sleep for 300 seconds (5 minutes) to conserve solar battery
        iot::log_info("Entering ULP Deep Sleep for 300 seconds...");
        iot::deep_sleep(300);
    });
}
