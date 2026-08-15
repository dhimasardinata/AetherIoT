#pragma once

/**
 * ============================================================================
 * FULLY CUSTOMIZABLE DYNAMIC LCD DISPLAY ENGINE (iot_lcd.hpp)
 * ============================================================================
 * Features:
 * - Zero Dynamic Memory Allocation (0% Heap)
 * - User-Defined Custom Multi-Page Flipper
 * - Dynamic Sensor Telemetry Callbacks per Line
 * - Direct Cursor & Text Printing (16x2, 20x4, OLED, PCF8574)
 * - Clean Placeholder Fallback (Fully Replaceable)
 * ============================================================================
 */

#include <cstdint>
#include <cstdio>
#include <string_view>
#include <span>
#include <array>

#include "config.hpp"
#include "iot_core.hpp"
#include "iot_drivers.hpp"

namespace iot::display {

// Forward Declaration for Unified Telemetry
struct LCDContext {
    float temp{0.0f};
    float hum{0.0f};
    float pressure{1013.2f};
    float ph{7.0f};
    float ec{0.0f};
    float voltage{0.0f};
    float current{0.0f};
    uint8_t battery_pct{100};
    bool online{false};
};

using LineFormatterFn = void(*)(const LCDContext& ctx, FixedString<32>& out);

class PageBuilder {
public:
    PageBuilder() = default;

    PageBuilder& title(std::string_view page_title) noexcept {
        title_.assign(page_title);
        return *this;
    }

    PageBuilder& line(uint8_t row, std::string_view static_text) noexcept {
        if (row < 4) {
            static_lines_[row].assign(static_text);
            formatters_[row] = nullptr;
        }
        return *this;
    }

    PageBuilder& line(uint8_t row, LineFormatterFn formatter) noexcept {
        if (row < 4) {
            formatters_[row] = formatter;
            static_lines_[row].clear();
        }
        return *this;
    }

    void render(const LCDContext& ctx, std::array<FixedString<32>, 4>& out_lines) const noexcept {
        for (uint8_t r = 0; r < 4; ++r) {
            if (formatters_[r]) {
                out_lines[r].clear();
                formatters_[r](ctx, out_lines[r]);
            } else if (!static_lines_[r].empty()) {
                out_lines[r].assign(static_lines_[r].string_view());
            } else if (r == 0 && !title_.empty()) {
                out_lines[r].assign(title_.string_view());
            } else {
                out_lines[r].clear();
            }
        }
    }

    [[nodiscard]] bool is_configured() const noexcept {
        return !title_.empty() || !static_lines_[0].empty() || formatters_[0] != nullptr;
    }

private:
    FixedString<24> title_{};
    std::array<FixedString<32>, 4> static_lines_{};
    std::array<LineFormatterFn, 4> formatters_{};
};

class CustomDisplayManager {
public:
    static constexpr size_t MAX_PAGES = 8;

    static PageBuilder& page(std::string_view title = "") noexcept {
        if (page_count_ < MAX_PAGES) {
            pages_[page_count_].title(title);
            return pages_[page_count_++];
        }
        return pages_[0];
    }

    static void clear_pages() noexcept {
        page_count_ = 0;
        current_page_idx_ = 0;
    }

    static void set_page_rotation_ms(uint32_t ms) noexcept {
        rotation_interval_ms_ = ms;
    }

    template <typename Config>
    static void update(const UnifiedTelemetry& raw, uint32_t now_ms) noexcept {
        if constexpr (!Config::Sensors::I2CDevices::ENABLE_LCD_PCF8574) return;

        if (now_ms - last_flip_ms_ < rotation_interval_ms_) return;
        last_flip_ms_ = now_ms;

        using LCD = drivers::PCF8574LCDDriver<Config>;

        LCDContext ctx{};
        ctx.temp = static_cast<float>(raw.air_temperature_centi_c) / 100.0f;
        ctx.hum = static_cast<float>(raw.air_humidity_centi_rh) / 100.0f;
        ctx.pressure = static_cast<float>(raw.barometric_pressure_pa) / 100.0f;
        ctx.ph = static_cast<float>(raw.water_ph_mili) / 1000.0f;
        ctx.ec = static_cast<float>(raw.water_ec_us_cm);
        ctx.voltage = static_cast<float>(raw.bus_voltage_mv) / 1000.0f;
        ctx.current = static_cast<float>(raw.bus_current_ma) / 1000.0f;
        ctx.battery_pct = (raw.bus_voltage_mv >= 12000) ? 100 : 
                          ((raw.bus_voltage_mv > 10000) ? static_cast<uint8_t>((raw.bus_voltage_mv - 10000) / 20) : 0);
        ctx.online = raw.flags.wifi_online != 0;

        std::array<FixedString<32>, 4> lines;

        if (page_count_ > 0) {
            current_page_idx_ = current_page_idx_ % page_count_;
            pages_[current_page_idx_].render(ctx, lines);
            current_page_idx_ = (current_page_idx_ + 1) % page_count_;
        } else {
            // Minimal Generic Placeholder (Clean, English, Decoupled)
            lines[0].format("AetherIoT Node");
            lines[1].format("T:%.1fC H:%.0f%%", ctx.temp, ctx.hum);
            lines[2].format("V:%.1fV Bat:%u%%", ctx.voltage, ctx.battery_pct);
            lines[3].format("Net:%s", ctx.online ? "ONLINE" : "STANDBY");
        }

        LCD::clear();
        for (uint8_t r = 0; r < 4; ++r) {
            if (!lines[r].empty()) {
                LCD::set_cursor(0, r);
                LCD::print(lines[r].string_view());
            }
        }
    }

    template <typename Config>
    static void print_direct(uint8_t col, uint8_t row, std::string_view text) noexcept {
        using LCD = drivers::PCF8574LCDDriver<Config>;
        LCD::set_cursor(col, row);
        LCD::print(text);
    }

    template <typename Config>
    static void clear_screen() noexcept {
        using LCD = drivers::PCF8574LCDDriver<Config>;
        LCD::clear();
    }

private:
    static inline std::array<PageBuilder, MAX_PAGES> pages_{};
    static inline size_t page_count_{0};
    static inline size_t current_page_idx_{0};
    static inline uint32_t last_flip_ms_{0};
    static inline uint32_t rotation_interval_ms_{3000};
};

} // namespace iot::display
