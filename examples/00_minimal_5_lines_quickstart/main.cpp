#include "../../iot_framework.hpp"

/**
 * ============================================================================
 * EXAMPLE 00: ULTRA-MINIMAL QUICKSTART (5 LINES OF CODE)
 * ============================================================================
 * Zero Configuration Needed!
 * - Automatic Wi-Fi connection, Sensor reading, Real-time Web Dashboard,
 *   Bluetooth Serial Monitor, and Declarative Safety Guards.
 * ============================================================================
 */

IOT_APP(app) {
    // 1. Declarative Safety Guard
    iot::guard("Temperature").max(35.0f).on_breach([](auto, auto v) {
        iot::log_error("ALERT: Temperature %.1f C exceeded safe limit!", v);
    });

    // 2. Real-Time Telemetry Stream
    app.on_data([](auto d) {
        iot::log_info("Temp: %.1f C | Humidity: %.1f %%RH | pH: %.2f", d.temp, d.hum, d.ph);
    });
}
