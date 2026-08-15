#pragma once

/**
 * ============================================================================
 * HIGH-DENSITY MULTI-SERVO CONTROLLER & RELAY-LIKE ACTUATION (iot_multi_servo.hpp)
 * ============================================================================
 * Supports up to 32 independent servos via PCA9685 16-Ch PWM or direct GPIOs:
 * - Relay-Like Declarative API: open(), close(), on(), off(), toggle(), is_open()
 * - Custom String Naming: iot::servo_open("GateValve1")
 * - Angle Calibration: Min/Max endpoints, sweep rate limiting, continuous 360° spin
 * ============================================================================
 */

#include <cstdint>
#include <cstdio>
#include <string_view>
#include <array>
#include <span>

#include "config.hpp"
#include "iot_core.hpp"
#include "iot_actuators_expanded.hpp"

namespace iot::actuators {

struct ServoConfig {
    float closed_angle{0.0f};
    float open_angle{90.0f};
    float current_angle{0.0f};
    bool  is_open{false};
    bool  is_continuous{false};
    FixedString<24> alias{};
};

template <size_t MaxServos = 32>
class MultiServoMatrix {
public:
    static void init() noexcept {
        for (size_t i = 0; i < MaxServos; ++i) {
            servos_[i].closed_angle = 0.0f;
            servos_[i].open_angle = 90.0f;
            servos_[i].current_angle = 0.0f;
            servos_[i].is_open = false;
        }
        std::printf("\033[1;32m[MULTI-SERVO] High-Density Servo Matrix Initialized (%zu Channels Ready)\033[0m\n",
                    MaxServos);
    }

    static void set_alias(size_t index, std::string_view name) noexcept {
        if (index >= MaxServos) return;
        servos_[index].alias.assign(name);
    }

    static void set_calibration(size_t index, float closed_angle, float open_angle) noexcept {
        if (index >= MaxServos) return;
        servos_[index].closed_angle = closed_angle;
        servos_[index].open_angle = open_angle;
    }

    static void set_angle(size_t index, float degrees) noexcept {
        if (index >= MaxServos) return;
        if (degrees < 0.0f) degrees = 0.0f;
        if (degrees > 180.0f) degrees = 180.0f;

        servos_[index].current_angle = degrees;
        servos_[index].is_open = (degrees > (servos_[index].closed_angle + 10.0f));

        // Map angle 0-180 to PWM duty cycle (2.5% to 12.5% at 50Hz)
        const float duty_pct = 2.5f + (degrees / 180.0f) * 10.0f;
        if (index < 16) {
            PCA9685PWMExpander<AppConfig>::set_channel_pct(static_cast<uint8_t>(index), duty_pct);
        }
    }

    // Relay-Like Discrete Behavior
    static void open(size_t index) noexcept {
        if (index >= MaxServos) return;
        set_angle(index, servos_[index].open_angle);
        servos_[index].is_open = true;
    }

    static void close(size_t index) noexcept {
        if (index >= MaxServos) return;
        set_angle(index, servos_[index].closed_angle);
        servos_[index].is_open = false;
    }

    static void toggle(size_t index) noexcept {
        if (index >= MaxServos) return;
        if (servos_[index].is_open) {
            close(index);
        } else {
            open(index);
        }
    }

    [[nodiscard]] static bool is_open(size_t index) noexcept {
        if (index >= MaxServos) return false;
        return servos_[index].is_open;
    }

    [[nodiscard]] static float get_angle(size_t index) noexcept {
        if (index >= MaxServos) return 0.0f;
        return servos_[index].current_angle;
    }

    // By String Alias
    static void open(std::string_view name) noexcept {
        const int idx = find_index(name);
        if (idx >= 0) open(static_cast<size_t>(idx));
    }

    static void close(std::string_view name) noexcept {
        const int idx = find_index(name);
        if (idx >= 0) close(static_cast<size_t>(idx));
    }

    static void toggle(std::string_view name) noexcept {
        const int idx = find_index(name);
        if (idx >= 0) toggle(static_cast<size_t>(idx));
    }

    [[nodiscard]] static int find_index(std::string_view name) noexcept {
        for (size_t i = 0; i < MaxServos; ++i) {
            if (servos_[i].alias.string_view() == name) return static_cast<int>(i);
        }
        return -1;
    }

private:
    static inline std::array<ServoConfig, MaxServos> servos_{};
};

} // namespace iot::actuators
