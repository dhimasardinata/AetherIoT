#include "../../iot_framework.hpp"

/**
 * ============================================================================
 * EXAMPLE 09: CUSTOM EXTENSIBLE CLI ENGINE & ADVANCED INDUSTRIAL SENSORS
 * ============================================================================
 * Features:
 * 1. Fully Customizable High-Level CLI Commands:
 *    - Custom names, descriptions, usage strings, and typed argument parsing.
 *    - Auto-generated interactive 'help' menu.
 * 2. Advanced Environmental & Industrial Sensors:
 *    - BME680 Gas Resistance & Indoor Air Quality (IAQ Index)
 *    - SCD30/SCD40 True Optical NDIR CO2 Sensor
 *    - MAX31865 RTD PT100 Platinum Interface
 *    - JSN-SR04T Waterproof Ultrasonic Level Sensor
 *    - SCT-013 Split-Core Current Transformer
 * 3. Actuation:
 *    - Stepper Motor with Anti-Jam Agitation
 *    - Industrial 4-20mA Process Loop Output
 * ============================================================================
 */

IOT_APP(app) {
    // ------------------------------------------------------------------------
    // 1. REGISTER CUSTOM CLI COMMANDS (FLUENT & TYPED ARGUMENTS)
    // ------------------------------------------------------------------------

    // Custom Command A: Process Pump Volume Control
    iot::cli("pump")
        .description("Control industrial process pump")
        .usage("pump <on|off|dose> [volume_ml]")
        .on_execute([](auto& ctx) {
            auto action = ctx.arg(0);
            float volume = ctx.arg_float(1, 10.0f);

            if (action == "dose") {
                iot::dose_ml(iot::Relay::PUMP_MAIN, volume);
                ctx.respond_ok("Dosed %.1f mL through primary process pump.", volume);
            } else if (action == "on") {
                iot::on(iot::Relay::PUMP_MAIN);
                ctx.respond_ok("Primary process pump turned ON.");
            } else if (action == "off") {
                iot::off(iot::Relay::PUMP_MAIN);
                ctx.respond_ok("Primary process pump turned OFF.");
            } else {
                ctx.respond_error("Invalid action '%s'. Use 'on', 'off', or 'dose'.", action.data());
            }
        });

    // Custom Command B: Stepper Motor Dispenser
    iot::cli("feed")
        .description("Dispense material payload with anti-jam agitation")
        .usage("feed <grams>")
        .on_execute([](auto& ctx) {
            float grams = ctx.arg_float(0, 25.0f);
            iot::stepper::StepperController::feed_grams_with_antijam(grams);
            ctx.respond_ok("Dispensed %.1f grams payload with anti-jam agitation.", grams);
        });

    // Custom Command C: 4-20mA Industrial Loop Transmitter Output
    iot::cli("loop")
        .description("Set industrial 4-20mA analog current loop output")
        .usage("loop <channel: 0-3> <current_ma: 4.0-20.0>")
        .on_execute([](auto& ctx) {
            int ch = ctx.arg_int(0, 0);
            float ma = ctx.arg_float(1, 4.0f);
            iot::set_loop_4_20ma(static_cast<uint8_t>(ch), ma);
            ctx.respond_ok("Current Loop Ch %d set to %.2f mA.", ch, ma);
        });

    // ------------------------------------------------------------------------
    // 2. PERIODIC TELEMETRY STREAM & ADVANCED SENSORS
    // ------------------------------------------------------------------------
    app.on_data([](auto data) {
        // Read BME680 Environmental Gas & IAQ
        auto bme = iot::read_bme680();

        // Read SCD40 Optical NDIR CO2 Sensor
        auto scd = iot::read_co2_scd();

        // Read High-Precision RTD PT100 via MAX31865
        float pt100_temp = iot::read_pt100(5);

        // Read Waterproof Ultrasonic Level
        float water_dist = iot::read_ultrasonic_distance_mm(12, 13, bme.temperature_c);

        iot::log_info("BME680 IAQ: %.0f | SCD CO2: %.0f ppm | PT100: %.2f C | Level: %.0f mm | Bat: %u%%",
                      bme.iaq_score, scd.co2_ppm, pt100_temp, water_dist, data.battery_pct);
    });
}
