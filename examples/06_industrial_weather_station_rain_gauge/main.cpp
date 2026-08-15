#include "../../iot_framework.hpp"

/**
 * ============================================================================
 * EXAMPLE 06: INDUSTRIAL WEATHER STATION & RAIN GAUGE MONITOR
 * ============================================================================
 * Target: ESP32-C3 / ESP32 Classic (Outdoor Solar Weather Station)
 * Sensors:
 * - Pulse Tipping Bucket Rain Gauge (0.2 mm / tip)
 * - Anemometer Wind Speed & Direction, Solar Radiation Pyranometer
 * - BME280 / SHT3x Barometric Pressure, Temp, & Humidity
 * Storage & Telemetry:
 * - Tiered Caching (RAM -> Flash -> MicroSD CSV)
 * - LoRa 915 MHz long-range telemetry + Cellular SMS Disaster Alerts
 * ============================================================================
 */

IOT_APP(app) {
    // 1. Extreme Rainfall Disaster Guard (Rainfall > 50 mm/h)
    iot::guard("RainfallRate")
        .max(50.0f) // 50 mm/h
        .on_breach([](auto name, auto val) {
            iot::log_error("FLOOD / LANDSLIDE EARLY WARNING [%s]: Extreme Rainfall %.1f mm/h! Dispatching SMS alert...", name.data(), val);
            iot::send_sms("+1234567890", "EARLY WARNING: Extreme Rainfall Detected > 50 mm/h!");
        });

    // 2. Weather Station Monitoring Loop
    app.on_data([](auto data) {
        // Log weather metrics
        iot::log_info("Weather Station | Rain: %.1f mm/h (Today: %.1f mm) | Temp: %.1f C | Pressure: %.1f hPa",
                      data.rain_rate_mm_h, data.rain_daily_mm, data.temp, data.pressure);

        // Transmit long-range LoRa packet
        iot::FixedString<128> packet;
        packet.format("RAIN:%.1f,DAILY:%.1f,T:%.1f,P:%.1f",
                      data.rain_rate_mm_h, data.rain_daily_mm, data.temp, data.pressure);
        iot::lora_send(1, packet.string_view());
    });
}
