#pragma once

/**
 * ============================================================================
 * CUSTOMIZABLE STATUS LED & VISUAL INDICATOR PATTERNS (iot_indicator.hpp)
 * ============================================================================
 * Features:
 * 1. Configurable GPIO Pin & Active High/Low Polarity.
 * 2. Non-blocking state-driven blink patterns (Normal, Connecting, Alert, OTA).
 * 3. 100% User-Customizable Millisecond Pulse Timing.
 * ============================================================================
 */

#include <cstdint>
#include <cstdio>
#include "config.hpp"
#include "iot_core.hpp"

#if defined(ESP_PLATFORM)
#include "driver/gpio.h"
#endif

namespace iot::indicator {

enum class LEDPattern : uint8_t {
    OFF              = 0,
    SOLID_ON         = 1,
    NORMAL_HEARTBEAT = 2, // 1500ms period, 50ms pulse
    CONNECTING       = 3, // 250ms on, 250ms off
    ALERT_ERROR      = 4, // 100ms on, 100ms off rapid strobe
    OTA_FLASHING     = 5  // 40ms on, 40ms off ultra-fast toggle
};

class StatusLED {
public:
    static void configure(int8_t pin, bool active_high = true) noexcept {
        pin_ = pin;
        active_high_ = active_high;
        if (pin_ >= 0) {
#if defined(ESP_PLATFORM)
            gpio_reset_pin(static_cast<gpio_num_t>(pin_));
            gpio_set_direction(static_cast<gpio_num_t>(pin_), GPIO_MODE_OUTPUT);
#endif
            apply_state(false);
        }
    }

    static void set_pattern(LEDPattern pattern) noexcept {
        pattern_ = pattern;
    }

    static void set_custom_timing(uint16_t on_ms, uint16_t off_ms) noexcept {
        custom_on_ms_ = on_ms;
        custom_off_ms_ = off_ms;
    }

    static void update(uint32_t now_ms) noexcept {
        if (pin_ < 0) return;

        switch (pattern_) {
            case LEDPattern::OFF:
                apply_state(false);
                break;

            case LEDPattern::SOLID_ON:
                apply_state(true);
                break;

            case LEDPattern::NORMAL_HEARTBEAT:
                tick_cycle(now_ms, 50, 1500);
                break;

            case LEDPattern::CONNECTING:
                tick_cycle(now_ms, 250, 250);
                break;

            case LEDPattern::ALERT_ERROR:
                tick_cycle(now_ms, 100, 100);
                break;

            case LEDPattern::OTA_FLASHING:
                tick_cycle(now_ms, 40, 40);
                break;
        }
    }

private:
    static void tick_cycle(uint32_t now_ms, uint16_t on_ms, uint16_t off_ms) noexcept {
        const uint32_t total_period = on_ms + off_ms;
        if (total_period == 0) return;

        const uint32_t elapsed = (now_ms - last_toggle_ms_) % total_period;
        const bool should_be_on = elapsed < on_ms;
        apply_state(should_be_on);
    }

    static void apply_state(bool on) noexcept {
        if (pin_ < 0) return;
        const bool level = active_high_ ? on : !on;
#if defined(ESP_PLATFORM)
        gpio_set_level(static_cast<gpio_num_t>(pin_), level ? 1 : 0);
#else
        (void)level;
#endif
    }

    static inline int8_t pin_{-1};
    static inline bool active_high_{true};
    static inline LEDPattern pattern_{LEDPattern::NORMAL_HEARTBEAT};
    static inline uint32_t last_toggle_ms_{0};
    static inline uint16_t custom_on_ms_{500};
    static inline uint16_t custom_off_ms_{500};
};

} // namespace iot::indicator
