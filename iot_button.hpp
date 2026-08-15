#pragma once

/**
 * ============================================================================
 * HARDWARE PUSH BUTTON & INTERRUPT DEBOUNCER (iot_button.hpp)
 * ============================================================================
 * Features:
 * 1. Hardware Debounced Digital Inputs & Pushbuttons (Active LOW / HIGH).
 * 2. Fluent Chained Event Callbacks (on_press, on_release, on_long_press).
 * 3. Zero Dynamic Memory Allocation (0% Heap).
 * ============================================================================
 */

#include <cstdint>
#include <cstdio>
#include <array>

#include "config.hpp"
#include "iot_core.hpp"

#if defined(ESP_PLATFORM)
#include "driver/gpio.h"
#endif

namespace iot::input {

using ButtonCallback = void(*)();

class ButtonHandler {
public:
    ButtonHandler() = default;

    ButtonHandler& pin(int8_t gpio_pin, bool active_low = true, bool pullup = true) noexcept {
        pin_ = gpio_pin;
        active_low_ = active_low;
        pullup_ = pullup;
        configured_ = true;

        if (pin_ >= 0) {
#if defined(ESP_PLATFORM)
            gpio_reset_pin(static_cast<gpio_num_t>(pin_));
            gpio_set_direction(static_cast<gpio_num_t>(pin_), GPIO_MODE_INPUT);
            if (pullup_) {
                gpio_set_pull_mode(static_cast<gpio_num_t>(pin_), active_low_ ? GPIO_PULLUP_ONLY : GPIO_PULLDOWN_ONLY);
            }
#endif
        }
        return *this;
    }

    ButtonHandler& on_press(ButtonCallback cb) noexcept {
        press_cb_ = cb;
        return *this;
    }

    ButtonHandler& on_release(ButtonCallback cb) noexcept {
        release_cb_ = cb;
        return *this;
    }

    ButtonHandler& on_long_press(uint32_t duration_ms, ButtonCallback cb) noexcept {
        long_press_duration_ms_ = duration_ms;
        long_press_cb_ = cb;
        return *this;
    }

    void update(uint32_t now_ms) noexcept {
        if (!configured_ || pin_ < 0) return;

        bool raw_state = false;
#if defined(ESP_PLATFORM)
        int level = gpio_get_level(static_cast<gpio_num_t>(pin_));
        raw_state = active_low_ ? (level == 0) : (level == 1);
#endif

        if (raw_state != last_raw_state_) {
            last_debounce_ms_ = now_ms;
            last_raw_state_ = raw_state;
        }

        if ((now_ms - last_debounce_ms_) > DEBOUNCE_THRESHOLD_MS) {
            if (raw_state != debounced_state_) {
                debounced_state_ = raw_state;

                if (debounced_state_) {
                    press_time_ms_ = now_ms;
                    long_pressed_ = false;
                    if (press_cb_) press_cb_();
                } else {
                    if (release_cb_) release_cb_();
                }
            }

            // Long-press evaluation
            if (debounced_state_ && !long_pressed_ && long_press_cb_) {
                if ((now_ms - press_time_ms_) >= long_press_duration_ms_) {
                    long_pressed_ = true;
                    long_press_cb_();
                }
            }
        }
    }

    [[nodiscard]] bool is_pressed() const noexcept { return debounced_state_; }
    [[nodiscard]] int8_t get_pin() const noexcept { return pin_; }

private:
    static constexpr uint32_t DEBOUNCE_THRESHOLD_MS = 50;

    int8_t pin_{-1};
    bool active_low_{true};
    bool pullup_{true};
    bool configured_{false};
    bool last_raw_state_{false};
    bool debounced_state_{false};
    bool long_pressed_{false};
    uint32_t last_debounce_ms_{0};
    uint32_t press_time_ms_{0};
    uint32_t long_press_duration_ms_{2000};

    ButtonCallback press_cb_{nullptr};
    ButtonCallback release_cb_{nullptr};
    ButtonCallback long_press_cb_{nullptr};
};

class ButtonManager {
public:
    static constexpr size_t MAX_BUTTONS = 8;

    static ButtonHandler& button(int8_t pin, bool active_low = true, bool pullup = true) noexcept {
        for (size_t i = 0; i < count_; ++i) {
            if (buttons_[i].get_pin() == pin) {
                return buttons_[i];
            }
        }
        if (count_ < MAX_BUTTONS) {
            buttons_[count_].pin(pin, active_low, pullup);
            return buttons_[count_++];
        }
        return buttons_[0];
    }

    static void update_all(uint32_t now_ms) noexcept {
        for (size_t i = 0; i < count_; ++i) {
            buttons_[i].update(now_ms);
        }
    }

private:
    static inline std::array<ButtonHandler, MAX_BUTTONS> buttons_{};
    static inline size_t count_{0};
};

} // namespace iot::input
