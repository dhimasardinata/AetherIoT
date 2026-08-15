#include "iot_framework.hpp"

/**
 * ============================================================================
 * UNIFIED ENTERPRISE IOT APPLICATION (main.cpp)
 * ============================================================================
 * Comprehensive Multi-Target & High-Density Industrial IoT Controller:
 * 1. Espressif Silicon Support (ESP32, S2, S3, C2, C3, C6, H2, P4, ESP8266).
 * 2. PCA9685 16-Channel 12-Bit PWM Dimming (Variable Frequency Drive / Fans).
 * 3. 74HC595 16-64+ Channel Daisy-Chain Relay Shift Register Matrix.
 * 4. Bistable Latching Solenoid Pulse Control (Zero Holding Current).
 * 5. Native 802.15.4 Thread / Matter Radio on ESP32-C6 / ESP32-H2.
 * 6. MicroSD FAT32 CSV Logger with Daily Auto-Rotation (/logs/YYYY-MM-DD.csv).
 * 7. Multi-SSID Priority Wi-Fi Fallback Store (Primary, Backup, Hotspot).
 * 8. Weekly Day Bitmask Scheduler (Days::MON | Days::WED | Days::FRI).
 * 9. Long-Range LoRa SX1276 Telemetry + MPU6050 6-Axis Motion & Tilt Guard.
 * 10. Multi-Scheme Payload Encryption (AES-256-CBC, ChaCha20, Anti-Replay).
 * 11. Custom Extensible CLI Engine & Dynamic Multi-Page LCD Display.
 * ============================================================================
 */

IOT_APP(app) {
    // 1. Configure Symmetric Encryption and Anti-Replay Layer
    iot::set_encryption(iot::Security::AES_256_CBC, "IndustrialSecretKey32BytesLong!!");
    iot::enable_anti_replay(true, 60);

    // 2. Add Multi-SSID Wi-Fi Networks with Priority Order
    iot::add_wifi_ap("Factory_Industrial_Net", "FactoryPass2026", 0); // Priority 0 (Primary)
    iot::add_wifi_ap("Field_Tech_Hotspot",     "TechHotspot99",   1); // Priority 1 (Backup)

    // 3. Customize Named Channel Aliases for Industrial Actuators
    iot::set_relay_name(1, "MainInletPump");
    iot::set_relay_name(2, "CoolingFan");
    iot::set_relay_name(3, "ThermalHeater");
    iot::set_relay_name(4, "WarningSiren");

    // 4. Configure Dynamic Multi-Page LCD
    iot::lcd_page("Main Telemetry")
        .line(0, "== INDUSTRIAL NODE ==")
        .line(1, [](auto& d, auto& out) { out.format("Temp: %.1fC  Pres: %.0f", d.temp, d.pressure); })
        .line(2, [](auto& d, auto& out) { out.format("Volt: %.1fV  Curr: %.2fA", d.voltage, d.current); })
        .line(3, [](auto& d, auto& out) { out.format("Status: %s", d.online ? "ONLINE" : "STANDBY"); });

    // 5. Register Custom High-Level CLI Command
    iot::cli("valve")
        .description("Control primary inlet valve actuator")
        .usage("valve <open|close|toggle>")
        .on_execute([](auto& ctx) {
            auto action = ctx.arg(0);
            if (action == "open") {
                iot::on("MainInletPump");
                ctx.respond_ok("Main inlet pump opened.");
            } else if (action == "close") {
                iot::off("MainInletPump");
                ctx.respond_ok("Main inlet pump closed.");
            } else if (action == "toggle") {
                iot::toggle("MainInletPump");
                ctx.respond_ok("Main inlet pump state toggled.");
            } else {
                ctx.respond_error("Invalid action '%s'. Use: open, close, or toggle.", action.data());
            }
        });

    // 6. Declarative Safety Guards with Hysteresis
    iot::guard("Temperature")
        .max(45.0f) // Max 45.0 C operating threshold
        .hysteresis(1.0f)
        .on_breach([](auto name, auto val) {
            iot::log_error("ALERT OVERHEAT [%s]: Temp %.1f C! Engaging cooling fan 100%%.", name.data(), val);
            iot::dim_pwm(0, 100.0f); // PCA9685 Ch 0 to 100% PWM
            iot::on("CoolingFan");
        });

    iot::guard("StructureTilt")
        .max(15.0f) // Max 15.0 deg tilt angle
        .on_breach([](auto name, auto val) {
            iot::log_error("ALERT TOWER TILT [%s]: Tilt %.1f deg! Dispatching emergency SMS.", name.data(), val);
            iot::send_sms("+1234567890", "ALERT: Telemetry tower tilt threshold exceeded!");
        });

    // 7. Weekly Day Bitmask Job Scheduler
    iot::schedule("MaintenanceCycle")
        .days(iot::Days::MON | iot::Days::WED | iot::Days::FRI)
        .time(2, 0) // 02:00 AM
        .on_trigger([]() {
            iot::log_info("SCHEDULE TRIGGER: Executing automated diagnostic maintenance routine...");
            iot::run_diagnostics();
        });

    // 8. Real-Time Telemetry Processing Stream
    app.on_data([](auto data) {
        iot::log_info("Node Data | Temp: %.1f C | Pressure: %.1f hPa | Bus: %.2f V, %.2f A | Link: %s",
                      data.temp, data.pressure, data.voltage, data.current, data.online ? "ONLINE" : "STANDBY");

        // Transmit JSON telemetry over LoRa SX1276 long-range radio
        iot::lora_send("{\"node_id\":1,\"temp\":28.5,\"volt\":12.4,\"status\":\"OK\"}");
    });

    // 9. Real-Time Alert Event Handler
    app.on_alert([](auto msg) {
        iot::log_error("CRITICAL SYSTEM EVENT: %s", msg);
    });
}
