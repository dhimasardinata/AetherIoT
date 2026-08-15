#include "../../iot_framework.hpp"

/**
 * ============================================================================
 * EXAMPLE 08: MULTI-SERVO MATRIX (RELAY-LIKE) & HIGH-DENSITY SENSOR MUX
 * ============================================================================
 * Target: ESP32-S3 / ESP32 Classic / ESP32-C6
 * Features:
 * - 16-32 Multi-Servo Matrix with Relay-like API:
 *   `servo_open()`, `servo_close()`, `servo_toggle()`, `servo_name()`
 * - TCA9548A 8-Channel I2C Bus Multiplexer
 * - 1-Wire DS18B20 16-Drop Multi-Temperature Array on 1 GPIO
 * - CD74HC4067 16-Channel Analog Sensor Multiplexer
 * ============================================================================
 */

IOT_APP(app) {
    // 1. Assign Descriptive Named Aliases to Servo Valves & Dampers
    iot::servo_name(0, "MainPipeValve");
    iot::servo_name(1, "InletValveA");
    iot::servo_name(2, "InletValveB");
    iot::servo_name(3, "ExhaustDamper");

    // 2. Calibrate Closed / Open Angles (e.g. Closed: 0 deg, Open: 90 deg)
    iot::servo_set_calib(0, 0.0f, 90.0f);
    iot::servo_set_calib(3, 10.0f, 85.0f);

    // 3. Control Servos using Declarative Relay-Style Operations
    iot::servo_open("MainPipeValve");   // Moves servo to 90 deg
    iot::servo_close("InletValveA");    // Moves servo to 0 deg
    iot::servo_toggle("ExhaustDamper"); // Inverts current servo position

    // 4. Periodic High-Density Sensor Multiplexing Sampling
    app.on_data([](auto data) {
        // A. Read 1-Wire Multi-Drop Temperature Array (16 Temperature Probes on 1 GPIO Pin)
        const float temp_zone1 = iot::read_temp_probe(0);
        const float temp_zone2 = iot::read_temp_probe(1);
        const float temp_zone3 = iot::read_temp_probe(2);

        // B. Read Analog Sensors via CD74HC4067 16-Channel MUX
        const float analog_v0 = iot::read_analog_mux_voltage(0);
        const float analog_v1 = iot::read_analog_mux_voltage(1);

        // C. Switch I2C Channel via TCA9548A Multiplexer (Channel 0 to 7)
        iot::i2c_channel(0);
        const float lux_bus0 = iot::read_lux();

        iot::log_info("[MULTI-SENSOR] Z1: %.1f C | Z2: %.1f C | Z3: %.1f C | V0: %.2fV | V1: %.2fV | Lux: %.0f | Bat: %u%%",
                      temp_zone1, temp_zone2, temp_zone3, analog_v0, analog_v1, lux_bus0, data.battery_pct);

        // Automated Servo Venting Logic based on Zone 1 Temperature
        if (temp_zone1 > 30.0f) {
            if (!iot::servo_is_open(3)) {
                iot::servo_open("RoofVentilation");
                iot::log_warn("Zone 1 Overheating (%.1f C) -> Roof Ventilation OPENED automatically!", temp_zone1);
            }
        }
    });
}
