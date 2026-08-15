#include "../../iot_framework.hpp"

/**
 * ============================================================================
 * EXAMPLE 05: SMART GREENHOUSE THREAD & MATTER 802.15.4 MESH SYSTEM
 * ============================================================================
 * Target: ESP32-C6 / ESP32-H2 (Native 802.15.4 Thread / Matter Radio)
 * Radio:
 * - IEEE 802.15.4 Thread Mesh Router / Border Router + Wi-Fi 6 Coexistence
 * - Native CoAP / IPv6 Telemetry Broadcasts
 * Actuators:
 * - PCA9685 16-Channel 12-Bit PWM (Full Spectrum LED Grow Light Dimming)
 * - Dual-Coil Bistable Latching Solenoid Valves (0mA holding current)
 * ============================================================================
 */

IOT_APP(app) {
    // 1. Configure Full Spectrum LED Grow Lights via PCA9685 12-Bit PWM
    // Ch 0: Deep Red 660nm (75% Duty Cycle)
    // Ch 1: Royal Blue 450nm (60% Duty Cycle)
    // Ch 2: Far Red 730nm (30% Duty Cycle)
    iot::dim_pwm(0, 75.0f);
    iot::dim_pwm(1, 60.0f);
    iot::dim_pwm(2, 30.0f);

    // 2. Open Zone 1 Irrigation Valve using 50ms Pulse (Latching Solenoid)
    iot::latch_open(21, 50); // Pulse Pin 21 High for 50ms
    iot::log_info("Bistable Latching Solenoid Zone 1 OPENED (0mA Static Holding Power)");

    // 3. Telemetry Stream & Thread CoAP Broadcast
    app.on_data([](auto data) {
        iot::log_info("Matter Greenhouse | Temp: %.1f C | Humidity: %.1f %%RH | Light: %.0f Lux | Target: %s",
                      data.temp, data.hum, data.lux, iot::target_chip_name().data());

        // Broadcast JSON packet over Thread Mesh via CoAP
        iot::thread_send("/aether/greenhouse/zone1", "{\"temp\":27.5,\"hum\":65,\"status\":\"OK\"}");
    });
}
