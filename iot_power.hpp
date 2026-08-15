#pragma once

/**
 * ============================================================================
 * POWER MANAGEMENT, BATTERY GAUGING & DEEP SLEEP (iot_power.hpp)
 * ============================================================================
 * - Li-Ion / LiFePO4 Battery Discharge Curve Calculation (0 - 100%)
 * - Low-Battery Critical Cutoff & Solar Charge State Detector
 * - Deep Sleep & RTC Wake-Stub Management (< 15 uA ultra-low power)
 * ============================================================================
 */

#include <cstdint>
#include <cstdio>
#include <cmath>
#include <algorithm>

#include "config.hpp"
#include "iot_core.hpp"

#if defined(ESP_PLATFORM)
#include "esp_sleep.h"
#include "driver/rtc_io.h"
#endif

namespace iot::power {

enum class BatteryChemistry : uint8_t {
    LI_ION_18650,   // 3.00V (0%) - 4.20V (100%)
    LIFEPO4,        // 2.80V (0%) - 3.65V (100%)
    LEAD_ACID_12V   // 10.5V (0%) - 12.8V (100%)
};

class BatteryFuelGauge {
public:
    [[nodiscard]] static uint8_t calculate_percentage(float voltage_v, BatteryChemistry chem = BatteryChemistry::LI_ION_18650) noexcept {
        float min_v = 3.00f;
        float max_v = 4.20f;

        switch (chem) {
            case BatteryChemistry::LI_ION_18650:
                min_v = 3.20f; max_v = 4.20f;
                break;
            case BatteryChemistry::LIFEPO4:
                min_v = 2.90f; max_v = 3.45f;
                break;
            case BatteryChemistry::LEAD_ACID_12V:
                min_v = 11.50f; max_v = 12.80f;
                break;
        }

        if (voltage_v <= min_v) return 0;
        if (voltage_v >= max_v) return 100;

        // Polynomial curve approximation for realistic discharge plateau
        const float normalized = (voltage_v - min_v) / (max_v - min_v);
        const float pct = (normalized * normalized * (3.0f - 2.0f * normalized)) * 100.0f;
        return static_cast<uint8_t>(std::clamp(pct, 0.0f, 100.0f));
    }
};

class SleepManager {
public:
    static void enter_deep_sleep(uint32_t seconds) noexcept {
        (void)seconds;
        std::printf("\033[1;33m[POWER] Entering Deep Sleep for %lu seconds...\033[0m\n",
                    static_cast<unsigned long>(seconds));
#if defined(ESP_PLATFORM)
        esp_sleep_enable_timer_wakeup(static_cast<uint64_t>(seconds) * 1000000ULL);
        esp_deep_sleep_start();
#endif
    }

    static void enter_light_sleep(uint32_t milliseconds) noexcept {
        (void)milliseconds;
#if defined(ESP_PLATFORM)
        esp_sleep_enable_timer_wakeup(static_cast<uint64_t>(milliseconds) * 1000ULL);
        esp_light_sleep_start();
#endif
    }
};

} // namespace iot::power
