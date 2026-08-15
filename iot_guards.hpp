#pragma once

/**
 * ============================================================================
 * FLUENT SAFETY RANGE GUARDS & ANOMALY WATCHDOGS (iot_guards.hpp)
 * ============================================================================
 * Provides declarative range boundaries with built-in hysteresis and breach callbacks:
 * Example:
 *   iot::guard("pH").between(5.5f, 7.5f).on_breach([](float v) { ... });
 *   iot::guard("Suhu").max(35.0f).on_breach([](float v) { ... });
 * ============================================================================
 */

#include <cstdint>
#include <cstdio>
#include <string_view>
#include <array>

#include "config.hpp"
#include "iot_core.hpp"

namespace iot::guards {

class RangeGuard {
public:
    using BreachCallback = void(*)(std::string_view name, float current_value);

    RangeGuard& between(float min_val, float max_val) noexcept {
        min_ = min_val;
        max_ = max_val;
        has_min_ = true;
        has_max_ = true;
        return *this;
    }

    RangeGuard& min(float min_val) noexcept {
        min_ = min_val;
        has_min_ = true;
        return *this;
    }

    RangeGuard& max(float max_val) noexcept {
        max_ = max_val;
        has_max_ = true;
        return *this;
    }

    RangeGuard& hysteresis(float h) noexcept {
        hysteresis_ = h;
        return *this;
    }

    void on_breach(BreachCallback cb) noexcept {
        breach_cb_ = cb;
    }

    void evaluate(float current_value) noexcept {
        bool in_breach = false;

        if (has_min_ && current_value < (min_ - hysteresis_)) {
            in_breach = true;
        }
        if (has_max_ && current_value > (max_ + hysteresis_)) {
            in_breach = true;
        }

        if (in_breach && !was_in_breach_) {
            was_in_breach_ = true;
            if (breach_cb_) {
                breach_cb_(name_.string_view(), current_value);
            }
        } else if (!in_breach && was_in_breach_) {
            // Recovered
            if (has_min_ && current_value >= min_ && (!has_max_ || current_value <= max_)) {
                was_in_breach_ = false;
            }
        }
    }

    void set_name(std::string_view name) noexcept {
        name_.assign(name);
    }

    [[nodiscard]] std::string_view name() const noexcept { return name_.string_view(); }

private:
    FixedString<24> name_{"Guard"};
    float min_{0.0f};
    float max_{100.0f};
    float hysteresis_{0.0f};
    bool  has_min_{false};
    bool  has_max_{false};
    bool  was_in_breach_{false};
    BreachCallback breach_cb_{nullptr};
};

class GuardManager {
public:
    static constexpr size_t MAX_GUARDS = 8;

    static RangeGuard& create(std::string_view name) noexcept {
        for (size_t i = 0; i < count_; ++i) {
            if (guards_[i].name() == name) return guards_[i];
        }
        if (count_ < MAX_GUARDS) {
            guards_[count_].set_name(name);
            return guards_[count_++];
        }
        return guards_[0];
    }

    static void check(std::string_view name, float value) noexcept {
        for (size_t i = 0; i < count_; ++i) {
            if (guards_[i].name() == name) {
                guards_[i].evaluate(value);
                break;
            }
        }
    }

private:
    static inline std::array<RangeGuard, MAX_GUARDS> guards_{};
    static inline size_t count_{0};
};

} // namespace iot::guards
