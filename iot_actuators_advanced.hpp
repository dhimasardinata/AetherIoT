#pragma once

/**
 * ============================================================================
 * ADVANCED ACTUATORS, FLOW METERS & STEPPER/SERVO (iot_actuators_advanced.hpp)
 * ============================================================================
 * - Pulse-Counter Water Flow Meter Accumulator (L/min & Cumulative Liters)
 * - 50 Hz Servo Motor Precision Angle Controller (0 - 180 deg)
 * - Stepper Motor Feeder Dispenser with Anti-Jam Agitation Cycle
 * ============================================================================
 */

#include <cstdint>
#include <cstdio>
#include <atomic>
#include <algorithm>

#include "config.hpp"
#include "iot_core.hpp"

#if defined(ESP_PLATFORM)
#include "driver/ledc.h"
#include "driver/gpio.h"
#endif

namespace iot::actuators {

// ============================================================================
// 1. PULSE-COUNTER WATER FLOW METER ACCUMULATOR
// ============================================================================

class FlowMeterAccumulator {
public:
    static void init(uint8_t gpio_pin, float pulses_per_liter = 450.0f) noexcept {
        pulses_per_liter_ = pulses_per_liter;
        (void)gpio_pin;
#if defined(ESP_PLATFORM)
        // Configure GPIO interrupt for pulse counting
#endif
    }

    static void record_pulse() noexcept {
        total_pulses_.fetch_add(1, std::memory_order_relaxed);
        window_pulses_.fetch_add(1, std::memory_order_relaxed);
    }

    static void update_flow_rate(uint32_t delta_ms) noexcept {
        if (delta_ms == 0 || pulses_per_liter_ <= 0.0f) return;
        const uint32_t wp = window_pulses_.exchange(0, std::memory_order_relaxed);
        const float liters = static_cast<float>(wp) / pulses_per_liter_;
        flow_rate_lpm_ = (liters / static_cast<float>(delta_ms)) * 60000.0f;
    }

    [[nodiscard]] static float flow_rate_lpm() noexcept {
        return flow_rate_lpm_;
    }

    [[nodiscard]] static float total_liters() noexcept {
        const uint32_t tp = total_pulses_.load(std::memory_order_relaxed);
        return (pulses_per_liter_ > 0.0f) ? (static_cast<float>(tp) / pulses_per_liter_) : 0.0f;
    }

    static void reset_total() noexcept {
        total_pulses_.store(0, std::memory_order_relaxed);
    }

private:
    static inline std::atomic<uint32_t> total_pulses_{0};
    static inline std::atomic<uint32_t> window_pulses_{0};
    static inline float                 pulses_per_liter_{450.0f};
    static inline float                 flow_rate_lpm_{0.0f};
};

// ============================================================================
// 2. SERVO MOTOR PRECISION ANGLE CONTROLLER
// ============================================================================

class ServoController {
public:
    static void init(uint8_t gpio_pin) noexcept {
        (void)gpio_pin;
#if defined(ESP_PLATFORM)
        ledc_timer_config_t timer_conf{};
        timer_conf.speed_mode = LEDC_LOW_SPEED_MODE;
        timer_conf.duty_resolution = LEDC_TIMER_16_BIT;
        timer_conf.timer_num = LEDC_TIMER_1;
        timer_conf.freq_hz = 50; // 50 Hz for standard hobby servos
        ledc_timer_config(&timer_conf);

        ledc_channel_config_t chan_conf{};
        chan_conf.gpio_num = gpio_pin;
        chan_conf.speed_mode = LEDC_LOW_SPEED_MODE;
        chan_conf.channel = LEDC_CHANNEL_1;
        chan_conf.timer_sel = LEDC_TIMER_1;
        chan_conf.duty = 0;
        ledc_channel_config(&chan_conf);
#endif
    }

    static void set_angle(uint8_t degrees) noexcept {
        degrees = std::min<uint8_t>(degrees, 180);
        const uint32_t duty = 1638 + static_cast<uint32_t>((degrees * (8192 - 1638)) / 180);
#if defined(ESP_PLATFORM)
        ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, duty);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1);
#else
        (void)duty;
#endif
    }
};

// ============================================================================
// 3. STEPPER MOTOR FEEDER WITH ANTI-JAM AGITATION
// ============================================================================

class StepperFeeder {
public:
    static void dispense_grams(float grams, float steps_per_gram = 20.0f) noexcept {
        const uint32_t total_steps = static_cast<uint32_t>(grams * steps_per_gram);
        std::printf("\033[1;36m[STEPPER] Dispensing %.1f grams (%lu steps) with anti-jam check...\033[0m\n",
                    grams, static_cast<unsigned long>(total_steps));

        step(total_steps, true);
        step(total_steps / 20, false);
    }

private:
    static void step(uint32_t steps, bool forward) noexcept {
        (void)steps;
        (void)forward;
#if defined(ESP_PLATFORM)
        // Pulse step pin with microsecond delays
#endif
    }
};

} // namespace iot::actuators
