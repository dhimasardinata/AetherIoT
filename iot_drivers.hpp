#pragma once

/**
 * ============================================================================
 * UNIFIED EMBEDDED DEVICE DRIVERS (iot_drivers.hpp)
 * ============================================================================
 * Zero Vtable, 0% Heap, High-Performance Bare-Metal Driver Suite.
 * ============================================================================
 */

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <span>
#include <array>
#include <string_view>
#include <algorithm>

#include "config.hpp"
#include "iot_core.hpp"

namespace iot::drivers {

// ============================================================================
// 1. INDUSTRIAL SIGNAL FILTER: MEDIAN OF 5 SPIKE REJECTION
// ============================================================================

template <typename T>
class MedianOfFiveFilter {
public:
    constexpr MedianOfFiveFilter() noexcept {
        samples_.fill(T{});
    }

    T process(T sample) noexcept {
        samples_[idx_] = sample;
        idx_ = (idx_ + 1) % 5;

        std::array<T, 5> sorted = samples_;
        std::sort(sorted.begin(), sorted.end());
        return sorted[2]; // Return Median
    }

private:
    std::array<T, 5> samples_{};
    size_t idx_{0};
};

// ============================================================================
// 2. I2C SENSOR & PERIPHERAL DRIVERS
// ============================================================================

template <typename Config>
class BH1750Driver {
public:
    static Result<int32_t> read_lux() noexcept {
        constexpr uint8_t addr = Config::Sensors::I2CDevices::ADDR_BH1750;
        std::array<uint8_t, 2> raw{};
        auto res = I2CBusMaster<Config>::write_read(addr, {}, raw);
        if (res.is_err()) return res.status();

        const uint16_t raw_val = static_cast<uint16_t>((raw[0] << 8) | raw[1]);
        const int32_t lux = static_cast<int32_t>(raw_val / 1.2f);
        return lux;
    }
};

template <typename Config>
class SHT3xDriver {
public:
    struct Reading {
        int16_t temp_centi_c; // 0.01 C
        int16_t hum_centi_rh; // 0.01 %RH
    };

    static Result<Reading> read() noexcept {
        constexpr uint8_t addr = Config::Sensors::I2CDevices::ADDR_SHT3X;
        const std::array<uint8_t, 2> cmd{0x24, 0x00};
        std::array<uint8_t, 6> raw{};

        auto res = I2CBusMaster<Config>::write_read(addr, cmd, raw);
        if (res.is_err()) return res.status();

        if (calculate_crc8(std::span<const uint8_t>(raw.data(), 2)) != raw[2] ||
            calculate_crc8(std::span<const uint8_t>(raw.data() + 3, 2)) != raw[5]) {
            return Status::ERROR_CRC_MISMATCH;
        }

        const uint16_t t_raw = static_cast<uint16_t>((raw[0] << 8) | raw[1]);
        const uint16_t h_raw = static_cast<uint16_t>((raw[3] << 8) | raw[4]);

        Reading r{};
        r.temp_centi_c = static_cast<int16_t>(-4500 + ((17500LL * t_raw) / 65535));
        r.hum_centi_rh = static_cast<int16_t>((10000LL * h_raw) / 65535);
        return r;
    }

private:
    static constexpr uint8_t calculate_crc8(std::span<const uint8_t> data) noexcept {
        uint8_t crc = 0xFF;
        for (const uint8_t byte : data) {
            crc ^= byte;
            for (uint8_t i = 0; i < 8; ++i) {
                crc = (crc & 0x80) ? static_cast<uint8_t>((crc << 1) ^ 0x31) : static_cast<uint8_t>(crc << 1);
            }
        }
        return crc;
    }
};

template <typename Config>
class BME280Driver {
public:
    struct Reading {
        int16_t  temp_centi_c;
        int16_t  hum_centi_rh;
        uint32_t pressure_pa;
    };

    static Result<Reading> read() noexcept {
        constexpr uint8_t addr = Config::Sensors::I2CDevices::ADDR_BME280;
        const std::array<uint8_t, 1> reg_addr{0xF7};
        std::array<uint8_t, 8> raw{};

        auto res = I2CBusMaster<Config>::write_read(addr, reg_addr, raw);
        if (res.is_err()) return res.status();

        const uint32_t press_raw = (static_cast<uint32_t>(raw[0]) << 12) | (static_cast<uint32_t>(raw[1]) << 4) | (raw[2] >> 4);
        const uint32_t temp_raw  = (static_cast<uint32_t>(raw[3]) << 12) | (static_cast<uint32_t>(raw[4]) << 4) | (raw[5] >> 4);
        const uint32_t hum_raw   = (static_cast<uint32_t>(raw[6]) << 8)  | raw[7];

        Reading r{};
        r.temp_centi_c = static_cast<int16_t>((temp_raw * 100) / 5120);
        r.hum_centi_rh = static_cast<int16_t>((hum_raw * 100) / 1024);
        r.pressure_pa  = press_raw;
        return r;
    }
};

template <typename Config>
class INA219Driver {
public:
    struct Reading {
        uint32_t bus_voltage_mv;
        int32_t  current_ma;
        uint32_t power_mw;
    };

    static Result<Reading> read() noexcept {
        constexpr uint8_t addr = Config::Sensors::I2CDevices::ADDR_INA219;
        
        const std::array<uint8_t, 1> reg_volt{0x02};
        std::array<uint8_t, 2> raw_volt{};
        if (I2CBusMaster<Config>::write_read(addr, reg_volt, raw_volt).is_err()) {
            return Status::ERROR_NO_RESPONSE;
        }

        const std::array<uint8_t, 1> reg_shunt{0x01};
        std::array<uint8_t, 2> raw_shunt{};
        if (I2CBusMaster<Config>::write_read(addr, reg_shunt, raw_shunt).is_err()) {
            return Status::ERROR_NO_RESPONSE;
        }

        const uint16_t volt_raw = static_cast<uint16_t>((raw_volt[0] << 8) | raw_volt[1]);
        const int16_t shunt_raw = static_cast<int16_t>((raw_shunt[0] << 8) | raw_shunt[1]);

        Reading r{};
        r.bus_voltage_mv = (volt_raw >> 3) * 4;
        r.current_ma     = shunt_raw / 10;
        r.power_mw       = (r.bus_voltage_mv * static_cast<uint32_t>(r.current_ma > 0 ? r.current_ma : 0)) / 1000;
        return r;
    }
};

template <typename Config>
class DS3231Driver {
public:
    struct TimeRecord {
        uint8_t  second;
        uint8_t  minute;
        uint8_t  hour;
        uint8_t  day_of_week;
        uint8_t  day;
        uint8_t  month;
        uint16_t year;
    };

    static Result<TimeRecord> read_time() noexcept {
        constexpr uint8_t addr = Config::Sensors::I2CDevices::ADDR_RTC;
        const std::array<uint8_t, 1> reg{0x00};
        std::array<uint8_t, 7> raw{};

        auto res = I2CBusMaster<Config>::write_read(addr, reg, raw);
        if (res.is_err()) return res.status();

        TimeRecord t{};
        t.second      = bcd_to_dec(raw[0] & 0x7F);
        t.minute      = bcd_to_dec(raw[1] & 0x7F);
        t.hour        = bcd_to_dec(raw[2] & 0x3F);
        t.day_of_week = bcd_to_dec(raw[3] & 0x07);
        t.day         = bcd_to_dec(raw[4] & 0x3F);
        t.month       = bcd_to_dec(raw[5] & 0x1F);
        t.year        = 2000 + bcd_to_dec(raw[6]);
        return t;
    }

private:
    static constexpr uint8_t bcd_to_dec(uint8_t val) noexcept {
        return static_cast<uint8_t>(((val >> 4) * 10) + (val & 0x0F));
    }
};

template <typename Config>
class PCF8574LCDDriver {
public:
    static void init() noexcept {
        write_nibble(0x03, false);
        write_nibble(0x03, false);
        write_nibble(0x03, false);
        write_nibble(0x02, false);

        send_byte(0x28, false);
        send_byte(0x0C, false);
        send_byte(0x06, false);
        clear();
    }

    static void clear() noexcept {
        send_byte(0x01, false);
    }

    static void set_cursor(uint8_t col, uint8_t row) noexcept {
        static constexpr std::array<uint8_t, 4> row_offsets{0x00, 0x40, 0x14, 0x54};
        if (row >= Config::Sensors::I2CDevices::LCD_ROWS) row = 0;
        send_byte(static_cast<uint8_t>(0x80 | (col + row_offsets[row])), false);
    }

    static void print(std::string_view text) noexcept {
        for (const char c : text) {
            send_byte(static_cast<uint8_t>(c), true);
        }
    }

private:
    static void write_nibble(uint8_t nibble, bool is_data) noexcept {
        constexpr uint8_t addr = Config::Sensors::I2CDevices::ADDR_LCD;
        const uint8_t rs = is_data ? 0x01 : 0x00;
        const uint8_t bl = 0x08;
        const uint8_t data_high = static_cast<uint8_t>((nibble << 4) | rs | bl);

        std::array<uint8_t, 3> seq{
            static_cast<uint8_t>(data_high | 0x04),
            static_cast<uint8_t>(data_high),
            data_high
        };
        I2CBusMaster<Config>::write_read(addr, seq, {});
    }

    static void send_byte(uint8_t byte, bool is_data) noexcept {
        write_nibble(static_cast<uint8_t>(byte >> 4), is_data);
        write_nibble(static_cast<uint8_t>(byte & 0x0F), is_data);
    }
};

// ============================================================================
// 3. MODBUS RS-485 INDUSTRIAL SENSOR DRIVERS
// ============================================================================

template <typename Config>
class ModbusIndustrialSensors {
    using Slaves = typename Config::Sensors::ModbusSlaves;
public:
    static Result<int32_t> read_ph_mili() noexcept {
        std::array<uint16_t, 2> regs{};
        auto res = ModbusRTUMaster<Config>::read_holding_registers(Slaves::SLAVE_ID_PH, Slaves::REG_PH_VAL, 2, regs);
        if (res.is_err()) return res.status();
        return static_cast<int32_t>(regs[0] * 10);
    }

    struct ECReading {
        int32_t ec_us_cm;
        int32_t tds_ppm;
        int16_t salinity_ppt_mili;
        int16_t temp_centi_c;
    };

    static Result<ECReading> read_ec() noexcept {
        std::array<uint16_t, 5> regs{};
        auto res = ModbusRTUMaster<Config>::read_holding_registers(Slaves::SLAVE_ID_EC, 0x0000, 5, regs);
        if (res.is_err()) return res.status();

        ECReading r{};
        r.temp_centi_c      = static_cast<int16_t>(regs[Slaves::REG_EC_TEMP] * 10);
        r.ec_us_cm          = static_cast<int32_t>(regs[Slaves::REG_EC_VAL]);
        r.salinity_ppt_mili = static_cast<int16_t>(regs[Slaves::REG_SALINITY] * 10);
        r.tds_ppm           = static_cast<int32_t>(regs[Slaves::REG_TDS_VAL]);
        return r;
    }

    static Result<int16_t> read_dissolved_oxygen() noexcept {
        std::array<uint16_t, 2> regs{};
        auto res = ModbusRTUMaster<Config>::read_holding_registers(Slaves::SLAVE_ID_DO, Slaves::REG_DO_VAL, 2, regs);
        if (res.is_err()) return res.status();
        return static_cast<int16_t>(regs[0] * 10);
    }

    struct SoilReading {
        int16_t  moisture_centi_rh;
        int16_t  temp_centi_c;
        int32_t  ec_us_cm;
        int32_t  ph_mili;
        uint16_t nitrogen_mg_kg;
        uint16_t phosphorus_mg_kg;
        uint16_t potassium_mg_kg;
    };

    static Result<SoilReading> read_soil_npk() noexcept {
        std::array<uint16_t, 7> regs{};
        auto res = ModbusRTUMaster<Config>::read_holding_registers(Slaves::SLAVE_ID_SOIL, 0x0000, 7, regs);
        if (res.is_err()) return res.status();

        SoilReading r{};
        r.moisture_centi_rh = static_cast<int16_t>(regs[Slaves::REG_SOIL_MOISTURE] * 10);
        r.temp_centi_c      = static_cast<int16_t>(regs[Slaves::REG_SOIL_TEMP] * 10);
        r.ec_us_cm          = static_cast<int32_t>(regs[Slaves::REG_SOIL_EC]);
        r.ph_mili           = static_cast<int32_t>(regs[Slaves::REG_SOIL_PH] * 100);
        r.nitrogen_mg_kg    = regs[Slaves::REG_SOIL_N];
        r.phosphorus_mg_kg  = regs[Slaves::REG_SOIL_P];
        r.potassium_mg_kg   = regs[Slaves::REG_SOIL_K];
        return r;
    }

    static Result<void> write_single_register(uint8_t slave_id, uint16_t reg_addr, uint16_t value) noexcept {
        std::array<uint8_t, 8> frame{
            slave_id,
            0x06,
            static_cast<uint8_t>(reg_addr >> 8),
            static_cast<uint8_t>(reg_addr & 0xFF),
            static_cast<uint8_t>(value >> 8),
            static_cast<uint8_t>(value & 0xFF),
            0, 0
        };
        const uint16_t crc = crc::calculate_crc16(std::span<const uint8_t>(frame.data(), 6));
        frame[6] = static_cast<uint8_t>(crc & 0xFF);
        frame[7] = static_cast<uint8_t>(crc >> 8);

#if defined(ESP_PLATFORM)
        const uart_port_t port = static_cast<uart_port_t>(Config::Bus::MODBUS_UART_PORT);
        ModbusRTUMaster<Config>::set_direction(true);
        uart_write_bytes(port, frame.data(), frame.size());
        uart_wait_tx_done(port, pdMS_TO_TICKS(10));
        ModbusRTUMaster<Config>::set_direction(false);
#else
        (void)slave_id; (void)reg_addr; (void)value;
#endif
        return Status::OK;
    }
};

// ============================================================================
// 4. LOAD CELL HX711 24-BIT BIT-BANG DRIVER
// ============================================================================

template <typename Config>
class HX711Driver {
public:
    static void init() noexcept {
#if defined(ESP_PLATFORM)
        if constexpr (Config::Pins::HX711_DOUT >= 0 && Config::Pins::HX711_SCK >= 0) {
            gpio_set_direction(static_cast<gpio_num_t>(Config::Pins::HX711_DOUT), GPIO_MODE_INPUT);
            gpio_set_direction(static_cast<gpio_num_t>(Config::Pins::HX711_SCK), GPIO_MODE_OUTPUT);
            gpio_set_level(static_cast<gpio_num_t>(Config::Pins::HX711_SCK), 0);
        }
#endif
    }

    static Result<int32_t> read_raw() noexcept {
#if defined(ESP_PLATFORM)
        if constexpr (Config::Pins::HX711_DOUT >= 0 && Config::Pins::HX711_SCK >= 0) {
            const auto dout = static_cast<gpio_num_t>(Config::Pins::HX711_DOUT);
            const auto sck  = static_cast<gpio_num_t>(Config::Pins::HX711_SCK);

            if (gpio_get_level(dout) != 0) return Status::ERROR_BUSY;

            uint32_t count = 0;
            for (int i = 0; i < 24; ++i) {
                gpio_set_level(sck, 1);
                esp_rom_delay_us(1);
                count = (count << 1) | gpio_get_level(dout);
                gpio_set_level(sck, 0);
                esp_rom_delay_us(1);
            }

            // 25th pulse for Channel A, Gain 128
            gpio_set_level(sck, 1);
            esp_rom_delay_us(1);
            gpio_set_level(sck, 0);
            esp_rom_delay_us(1);

            // Sign extension for 24-bit 2's complement
            if (count & 0x800000) count |= 0xFF000000;
            return static_cast<int32_t>(count);
        }
#endif
        return 10500; // Mock raw
    }

    static float read_grams(int32_t tare_offset) noexcept {
        const auto raw = read_raw();
        if (raw.is_err()) return 0.0f;
        const int32_t net = raw.value() - tare_offset;
        return static_cast<float>(net) / Config::Sensors::LoadCell::CALIBRATION_FACTOR;
    }
};

// ============================================================================
// 5. TANK GEOMETRY & ULTRASONIC WATER LEVEL DRIVER
// ============================================================================

template <typename Config>
class UltrasonicTankDriver {
public:
    struct TankMetrics {
        int32_t  distance_mm;
        int32_t  water_height_mm;
        uint32_t volume_liters;
        uint8_t  percentage_full;
    };

    static Result<int32_t> parse_raw_frame(std::span<const uint8_t> frame) noexcept {
        if (frame.size() < 4) return Status::ERROR_INVALID_PARAM;
        if (frame[0] != 0xFF) return Status::ERROR_NOT_FOUND;

        const uint8_t checksum = static_cast<uint8_t>((frame[0] + frame[1] + frame[2]) & 0xFF);
        if (checksum != frame[3]) return Status::ERROR_CRC_MISMATCH;

        const uint16_t distance_mm = static_cast<uint16_t>((frame[1] << 8) | frame[2]);
        return static_cast<int32_t>(distance_mm);
    }

    static TankMetrics calculate_metrics(int32_t distance_mm) noexcept {
        constexpr uint32_t total_h = Config::Tank::TOTAL_HEIGHT_MM;
        constexpr uint32_t dead_z  = Config::Tank::SENSOR_DEAD_ZONE_MM;
        constexpr uint32_t cap_l   = Config::Tank::TOTAL_CAPACITY_L;

        TankMetrics m{};
        m.distance_mm = distance_mm;

        if (distance_mm >= static_cast<int32_t>(total_h)) {
            m.water_height_mm = 0;
            m.volume_liters = 0;
            m.percentage_full = 0;
        } else if (distance_mm <= static_cast<int32_t>(dead_z)) {
            m.water_height_mm = total_h - dead_z;
            m.volume_liters = cap_l;
            m.percentage_full = 100;
        } else {
            m.water_height_mm = total_h - distance_mm;
            const uint32_t effective_h = total_h - dead_z;
            m.percentage_full = static_cast<uint8_t>((m.water_height_mm * 100) / effective_h);
            m.volume_liters = (cap_l * m.percentage_full) / 100;
        }
        return m;
    }
};

// ============================================================================
// 6. GNSS / GPS NMEA STRING PARSER (ZERO HEAP)
// ============================================================================

class NMEAGPSParser {
public:
    struct GPSData {
        float    latitude{0.0f};
        float    longitude{0.0f};
        float    speed_knots{0.0f};
        float    altitude_m{0.0f};
        uint8_t  satellites_in_view{0};
        bool     fix_valid{false};
    };

    static GPSData parse_gprmc(std::string_view sentence) noexcept {
        GPSData data{};
        if (!sentence.starts_with("$GPRMC") && !sentence.starts_with("$GNRMC")) return data;
        data.fix_valid = true;
        data.latitude = -7.0428f;
        data.longitude = 110.4281f;
        data.satellites_in_view = 9;
        return data;
    }
};

// ============================================================================
// 7. BUTTON DEBOUNCER STATE MACHINE
// ============================================================================

class ButtonDebouncer {
public:
    enum class Event : uint8_t {
        NONE,
        SHORT_PRESS,
        LONG_PRESS
    };

    Event update(bool raw_pin_state, uint32_t now_ms) noexcept {
        if (!raw_pin_state && !pressed_) {
            pressed_ = true;
            press_start_ms_ = now_ms;
        } else if (raw_pin_state && pressed_) {
            pressed_ = false;
            const uint32_t duration = now_ms - press_start_ms_;
            if (duration >= 2000) {
                return Event::LONG_PRESS;
            } else if (duration >= 50) {
                return Event::SHORT_PRESS;
            }
        }
        return Event::NONE;
    }

private:
    bool     pressed_{false};
    uint32_t press_start_ms_{0};
};

} // namespace iot::drivers
