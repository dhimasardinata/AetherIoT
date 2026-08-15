#include "../../iot_framework.hpp"

/**
 * ============================================================================
 * EXAMPLE 04: PRECISION AQUACULTURE POOL & AUTOFEEDER SYSTEM
 * ============================================================================
 * Target: ESP32-S3 / ESP32 Classic (Fish & Shrimp Pond Automation)
 * Sensors:
 * - Industrial Modbus Dissolved Oxygen (DO) Probe (0 - 20 mg/L)
 * - Industrial Modbus Water Salinity & Temperature Probe
 * Actuators:
 * - Ch 8: Pond Surface Aerator Oxygenator (with Inrush Current Staggering)
 * - Ch 9: Precision Stepper Feeder Motor (Anti-Jam Reverse Agitation)
 * Automation:
 * - Weekly Day Bitmask Scheduler (e.g. 07:00 and 16:00 feeding schedules)
 * ============================================================================
 */

namespace ProjectChannel {
    inline constexpr uint8_t AERATOR = 8;
}

IOT_APP(app) {
    // 1. Dissolved Oxygen (DO) Safety Range Guard
    iot::guard("DissolvedOxygen")
        .min(4.0f) // Minimum 4.0 mg/L dissolved oxygen for healthy fish
        .on_breach([](auto name, auto val) {
            iot::log_warn("AQUACULTURE ALERT [%s]: Low DO level (%.2f mg/L)! Activating Emergency Aerator...", name.data(), val);
            iot::on(ProjectChannel::AERATOR);
        });

    // 2. Weekly Feeding Job Scheduler (Monday, Wednesday, Friday, Saturday, Sunday)
    iot::schedule("MorningFeeding")
        .days(iot::Days::MON | iot::Days::WED | iot::Days::FRI | iot::Days::SAT | iot::Days::SUN)
        .time(7, 0) // 07:00 AM
        .on_trigger([]() {
            iot::log_info("SCHEDULE TRIGGER: Dispensing 50g feed with anti-jam agitation...");
            iot::feed_grams(50.0f);
        });

    // 3. Real-Time Water Quality Monitoring
    app.on_data([](auto data) {
        iot::log_info("Aquaculture Pond | DO: %.2f mg/L | Salinity: %.1f PPT | Water Temp: %.1f C | Aerator: %s",
                      data.oxygen, data.salinity, data.water_temp, iot::is_on(ProjectChannel::AERATOR) ? "RUNNING" : "STANDBY");

        // Turn off aerator when DO levels recover above 6.5 mg/L
        if (data.oxygen >= 6.5f && iot::is_on(ProjectChannel::AERATOR)) {
            iot::off(ProjectChannel::AERATOR);
            iot::log_info("DO levels recovered (%.2f mg/L) -> Aerator turned OFF to conserve energy", data.oxygen);
        }
    });
}
