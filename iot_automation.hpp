#pragma once

/**
 * ============================================================================
 * FLUENT RULES ENGINE, SCHEDULER & SIGNAL COMPENSATION (iot_automation.hpp)
 * ============================================================================
 * - Nernst Equation & EC Standard Temperature Compensation (25 C Reference)
 * - Inrush Current Staggered Relay Startup Engine (Anti-Brownout)
 * - Declarative Fluent Automation Rules (`iot::rule(...).when(...).then(...)`)
 * - RTC Time & Weekly Day Bitmask Scheduler (`iot::schedule(...).days(...).at(H, M).run(...)`)
 * ============================================================================
 */

#include <cstdint>
#include <cstdio>
#include <string_view>
#include <functional>
#include <span>
#include <array>

#include "config.hpp"
#include "iot_core.hpp"

namespace iot::automation {

// ============================================================================
// 1. SCIENTIFIC TEMPERATURE COMPENSATION (NERNST & EC25)
// ============================================================================

class TemperatureCompensation {
public:
    // Standarisasi EC ke 25 C: EC25 = ECT / (1 + 0.0191 * (T - 25))
    [[nodiscard]] static float compensate_ec(float raw_ec, float water_temp_c) noexcept {
        if (water_temp_c <= 0.0f || water_temp_c >= 70.0f) return raw_ec;
        const float alpha = 0.0191f;
        const float factor = 1.0f + (alpha * (water_temp_c - 25.0f));
        return (factor > 0.1f) ? (raw_ec / factor) : raw_ec;
    }

    // Kompensasi pH Slope Nernst terhadap Suhu: pH25 = pHT + 0.003 * (25 - T) * (pHT - 7.0)
    [[nodiscard]] static float compensate_ph(float raw_ph, float water_temp_c) noexcept {
        if (water_temp_c <= 0.0f || water_temp_c >= 70.0f) return raw_ph;
        const float delta_t = 25.0f - water_temp_c;
        const float offset = 0.003f * delta_t * (raw_ph - 7.0f);
        return raw_ph + offset;
    }
};

// ============================================================================
// 2. INRUSH CURRENT STAGGERED RELAY SEQUENCER
// ============================================================================

template <typename Config>
class StaggeredRelaySequencer {
public:
    static void energize_staggered(std::span<const uint8_t> channels, uint32_t stagger_delay_ms = 150) noexcept {
        (void)stagger_delay_ms;
        for (const uint8_t ch : channels) {
            ActuatorEngine<Config>::set_relay(ch, true);
#if defined(ESP_PLATFORM)
            vTaskDelay(pdMS_TO_TICKS(stagger_delay_ms));
#endif
        }
    }
};

// ============================================================================
// 3. FLUENT DECLARATIVE AUTOMATION RULES ENGINE
// ============================================================================

struct SensorSnapshot {
    float temp;
    float hum;
    float ph;
    float ec;
    float volume;
    float oxygen;
    int   tank_pct;
    bool  online;
};

class RuleBuilder {
public:
    using ConditionFn = bool(*)(const SensorSnapshot& data);
    using ActionFn    = void(*)();

    RuleBuilder& when(ConditionFn condition) noexcept {
        condition_ = condition;
        return *this;
    }

    void then(ActionFn action) noexcept {
        action_ = action;
    }

    void evaluate(const SensorSnapshot& data) const noexcept {
        if (condition_ && action_) {
            if (condition_(data)) {
                action_();
            }
        }
    }

private:
    ConditionFn condition_{nullptr};
    ActionFn    action_{nullptr};
};

class RulesEngine {
public:
    static constexpr size_t MAX_RULES = 8;

    static RuleBuilder& create_rule(std::string_view name) noexcept {
        (void)name;
        if (rule_count_ < MAX_RULES) {
            return rules_[rule_count_++];
        }
        return rules_[0];
    }

    static void evaluate_all(const SensorSnapshot& data) noexcept {
        for (size_t i = 0; i < rule_count_; ++i) {
            rules_[i].evaluate(data);
        }
    }

private:
    static inline std::array<RuleBuilder, MAX_RULES> rules_{};
    static inline size_t rule_count_{0};
};

// ============================================================================
// 4. TIME & WEEKLY DAY BITMASK JOB SCHEDULER
// ============================================================================

namespace Days {
    inline constexpr uint8_t MON = 1 << 0;
    inline constexpr uint8_t TUE = 1 << 1;
    inline constexpr uint8_t WED = 1 << 2;
    inline constexpr uint8_t THU = 1 << 3;
    inline constexpr uint8_t FRI = 1 << 4;
    inline constexpr uint8_t SAT = 1 << 5;
    inline constexpr uint8_t SUN = 1 << 6;
    inline constexpr uint8_t ALL = 0x7F;
}

class ScheduleBuilder {
public:
    using JobFn = void(*)();

    ScheduleBuilder& days(uint8_t day_mask) noexcept {
        day_mask_ = day_mask;
        return *this;
    }

    ScheduleBuilder& at(uint8_t hour, uint8_t minute) noexcept {
        hour_ = hour;
        minute_ = minute;
        return *this;
    }

    ScheduleBuilder& time(uint8_t hour, uint8_t minute) noexcept {
        return at(hour, minute);
    }

    void run(JobFn job) noexcept {
        job_ = job;
    }

    void on_trigger(JobFn job) noexcept {
        run(job);
    }

    void check_and_execute(uint8_t current_hour, uint8_t current_minute, uint8_t current_day_index = 0) noexcept {
        const uint8_t today_bit = 1 << (current_day_index % 7);
        const bool day_matches = (day_mask_ & today_bit) != 0;

        if (day_matches && job_ && current_hour == hour_ && current_minute == minute_ && !executed_today_) {
            executed_today_ = true;
            job_();
        } else if (current_hour != hour_) {
            executed_today_ = false;
        }
    }

private:
    uint8_t day_mask_{Days::ALL};
    uint8_t hour_{0};
    uint8_t minute_{0};
    bool    executed_today_{false};
    JobFn   job_{nullptr};
};

class JobScheduler {
public:
    static constexpr size_t MAX_JOBS = 6;

    static ScheduleBuilder& create_schedule(std::string_view name) noexcept {
        (void)name;
        if (job_count_ < MAX_JOBS) {
            return jobs_[job_count_++];
        }
        return jobs_[0];
    }

    static void update(uint8_t current_hour, uint8_t current_minute, uint8_t current_day_idx = 0) noexcept {
        for (size_t i = 0; i < job_count_; ++i) {
            jobs_[i].check_and_execute(current_hour, current_minute, current_day_idx);
        }
    }

private:
    static inline std::array<ScheduleBuilder, MAX_JOBS> jobs_{};
    static inline size_t job_count_{0};
};

} // namespace iot::automation
