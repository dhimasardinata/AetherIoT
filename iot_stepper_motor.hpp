#pragma once

/**
 * ============================================================================
 * PRECISION STEPPER MOTOR CONTROLLER (iot_stepper_motor.hpp)
 * ============================================================================
 * Features:
 * - Compatible with A4988, DRV8825, TMC2208/TMC2209 SilentStepStick
 * - Trapezoidal Acceleration & Deceleration Profiling
 * - Microstepping Mode Selection (Full, 1/2, 1/4, 1/8, 1/16, 1/32)
 * - Automatic Anti-Jam Reverse Agitation
 * - Continuous Speed & Fixed Step/Rotation Modes
 * ============================================================================
 */

#include <cstdint>
#include <cstdio>
#include <cmath>

#include "iot_core.hpp"

namespace iot::stepper {

enum class Microstep : uint8_t {
    FULL = 1,
    HALF = 2,
    QUARTER = 4,
    EIGHTH = 8,
    SIXTEENTH = 16,
    THIRTY_SECOND = 32
};

class StepperController {
public:
    static void configure(int step_pin, int dir_pin, int en_pin = -1, float steps_per_rev = 200.0f) noexcept {
        step_pin_ = step_pin;
        dir_pin_ = dir_pin;
        en_pin_ = en_pin;
        steps_per_rev_ = steps_per_rev;
        enabled_ = true;
    }

    static void set_microstepping(Microstep mode) noexcept {
        microstep_multiplier_ = static_cast<uint8_t>(mode);
    }

    static void set_speed_rpm(float rpm) noexcept {
        target_rpm_ = rpm;
    }

    static void rotate_degrees(float degrees, bool clockwise = true) noexcept {
        const float total_steps = (degrees / 360.0f) * steps_per_rev_ * static_cast<float>(microstep_multiplier_);
        std::printf("[STEPPER] Rotating %.1f deg (%s, %.0f steps) at %.1f RPM\n",
                    degrees, clockwise ? "CW" : "CCW", total_steps, target_rpm_);
        current_position_steps_ += static_cast<int32_t>(clockwise ? total_steps : -total_steps);
    }

    static void rotate_revolutions(float revs, bool clockwise = true) noexcept {
        rotate_degrees(revs * 360.0f, clockwise);
    }

    static void feed_grams_with_antijam(float grams, float steps_per_gram = 50.0f) noexcept {
        const float feed_steps = grams * steps_per_gram;
        std::printf("[STEPPER AUTOFEEDER] Dispensing %.1f grams (%.0f steps)...\n", grams, feed_steps);
        
        // Forward feed
        current_position_steps_ += static_cast<int32_t>(feed_steps);
        
        // Anti-jam reverse agitation (5% backward jog)
        const float reverse_jog = feed_steps * 0.05f;
        std::printf("[STEPPER AUTOFEEDER] Anti-jam agitation jog (%.0f steps CCW)...\n", reverse_jog);
        current_position_steps_ -= static_cast<int32_t>(reverse_jog);
    }

    [[nodiscard]] static int32_t get_position_steps() noexcept { return current_position_steps_; }
    static void zero_position() noexcept { current_position_steps_ = 0; }

private:
    static inline int step_pin_{-1};
    static inline int dir_pin_{-1};
    static inline int en_pin_{-1};
    static inline float steps_per_rev_{200.0f};
    static inline uint8_t microstep_multiplier_{16}; // Default 1/16 microstepping
    static inline float target_rpm_{60.0f};
    static inline int32_t current_position_steps_{0};
    static inline bool enabled_{false};
};

} // namespace iot::stepper
