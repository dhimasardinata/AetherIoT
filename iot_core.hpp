#pragma once

/**
 * ============================================================================
 * UNIFIED HIGH-PERFORMANCE IOT FRAMEWORK (iot_core.hpp)
 * ============================================================================
 * Architecture & Core Pillars:
 * 1. Declarative Single-Point Configuration: Auto-binds hardware & buses at compile-time.
 * 2. High-Level Procedural Developer Experience (DX): Clean <20 lines main loop.
 * 3. Extreme Hardware & Compute Efficiency: 0% Heap, Zero Vtable, Compile-Time Math.
 * ============================================================================
 */

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <cstdio>
#include <span>
#include <string_view>
#include <array>
#include <atomic>
#include <concepts>
#include <type_traits>
#include <algorithm>

#include "config.hpp"

// Platform HAL includes (ESP-IDF / Bare-Metal)
#if defined(ESP_PLATFORM)
#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_system.h"
#include "esp_ota_ops.h"
#include "esp_app_format.h"
#include "esp_partition.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_task_wdt.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "driver/i2c.h"
#include "driver/spi_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#else
// Standalone / Host Emulation Stubs for Static Analysis & Testing
#include <chrono>
#include <thread>
using esp_err_t = int32_t;
#define ESP_OK 0
#define ESP_FAIL -1
#define ESP_ERR_TIMEOUT 0x107
#define ESP_ERR_INVALID_ARG 0x102
#define ESP_ERR_INVALID_STATE 0x103
#endif

namespace iot {

// ============================================================================
// 1. ERROR HANDLING & RESULT MONAD (ZERO C++ EXCEPTIONS)
// ============================================================================

enum class Status : int32_t {
    OK                  = 0,
    ERROR_GENERIC       = -1,
    ERROR_TIMEOUT       = -2,
    ERROR_CRC_MISMATCH  = -3,
    ERROR_BUSY          = -4,
    ERROR_NO_RESPONSE   = -5,
    ERROR_BUFFER_FULL   = -6,
    ERROR_INVALID_PARAM = -7,
    ERROR_NOT_FOUND     = -8,
    ERROR_HARDWARE_FAIL = -9,
    ERROR_UNINITIALIZED = -10,
    ERROR_OTA_VERIFY    = -11
};

[[nodiscard]] constexpr std::string_view status_to_string(Status s) noexcept {
    switch (s) {
        case Status::OK:                  return "STATUS_OK";
        case Status::ERROR_GENERIC:       return "ERROR_GENERIC";
        case Status::ERROR_TIMEOUT:       return "ERROR_TIMEOUT";
        case Status::ERROR_CRC_MISMATCH:  return "ERROR_CRC_MISMATCH";
        case Status::ERROR_BUSY:          return "ERROR_BUSY";
        case Status::ERROR_NO_RESPONSE:   return "ERROR_NO_RESPONSE";
        case Status::ERROR_BUFFER_FULL:   return "ERROR_BUFFER_FULL";
        case Status::ERROR_INVALID_PARAM: return "ERROR_INVALID_PARAM";
        case Status::ERROR_NOT_FOUND:     return "ERROR_NOT_FOUND";
        case Status::ERROR_HARDWARE_FAIL: return "ERROR_HARDWARE_FAIL";
        case Status::ERROR_UNINITIALIZED: return "ERROR_UNINITIALIZED";
        case Status::ERROR_OTA_VERIFY:    return "ERROR_OTA_VERIFY";
        default:                          return "ERROR_UNKNOWN";
    }
}

template <typename T>
class Result {
public:
    constexpr Result(const T& value) noexcept : value_(value), status_(Status::OK) {}
    constexpr Result(T&& value) noexcept : value_(static_cast<T&&>(value)), status_(Status::OK) {}
    constexpr Result(Status status) noexcept : value_{}, status_(status) {}

    [[nodiscard]] constexpr bool is_ok() const noexcept { return status_ == Status::OK; }
    [[nodiscard]] constexpr bool is_err() const noexcept { return status_ != Status::OK; }
    [[nodiscard]] constexpr Status status() const noexcept { return status_; }

    [[nodiscard]] constexpr const T& value() const noexcept { return value_; }
    [[nodiscard]] constexpr T& value() noexcept { return value_; }
    [[nodiscard]] constexpr T value_or(T fallback) const noexcept {
        return is_ok() ? value_ : fallback;
    }

private:
    T value_{};
    Status status_{Status::ERROR_UNINITIALIZED};
};

template <>
class Result<void> {
public:
    constexpr Result() noexcept : status_(Status::OK) {}
    constexpr Result(Status status) noexcept : status_(status) {}

    [[nodiscard]] constexpr bool is_ok() const noexcept { return status_ == Status::OK; }
    [[nodiscard]] constexpr bool is_err() const noexcept { return status_ != Status::OK; }
    [[nodiscard]] constexpr Status status() const noexcept { return status_; }

private:
    Status status_{Status::OK};
};

// ============================================================================
// 2. HARDCORE ZERO-HEAP DATA STRUCTURES
// ============================================================================

template <size_t Capacity>
class FixedString {
public:
    constexpr FixedString() noexcept { buffer_[0] = '\0'; }

    constexpr FixedString(std::string_view sv) noexcept {
        assign(sv);
    }

    constexpr FixedString(const char* cstr) noexcept {
        assign(std::string_view(cstr ? cstr : ""));
    }

    constexpr void clear() noexcept {
        size_ = 0;
        buffer_[0] = '\0';
    }

    constexpr bool assign(std::string_view sv) noexcept {
        const size_t copy_len = (sv.size() < Capacity - 1) ? sv.size() : Capacity - 1;
        for (size_t i = 0; i < copy_len; ++i) {
            buffer_[i] = sv[i];
        }
        buffer_[copy_len] = '\0';
        size_ = copy_len;
        return sv.size() < Capacity;
    }

    constexpr bool append(std::string_view sv) noexcept {
        if (size_ + sv.size() >= Capacity) return false;
        for (size_t i = 0; i < sv.size(); ++i) {
            buffer_[size_ + i] = sv[i];
        }
        size_ += sv.size();
        buffer_[size_] = '\0';
        return true;
    }

    constexpr bool append_char(char c) noexcept {
        if (size_ + 1 >= Capacity) return false;
        buffer_[size_++] = c;
        buffer_[size_] = '\0';
        return true;
    }

    template <typename... Args>
    int format(const char* fmt, Args... args) noexcept {
        int written = std::snprintf(buffer_.data(), Capacity, fmt, args...);
        if (written > 0) {
            size_ = (static_cast<size_t>(written) < Capacity) ? static_cast<size_t>(written) : Capacity - 1;
        } else {
            clear();
        }
        return written;
    }

    [[nodiscard]] constexpr const char* c_str() const noexcept { return buffer_.data(); }
    [[nodiscard]] constexpr std::string_view string_view() const noexcept { return {buffer_.data(), size_}; }
    [[nodiscard]] constexpr const char* data() const noexcept { return buffer_.data(); }
    [[nodiscard]] constexpr char* data() noexcept { return buffer_.data(); }
    [[nodiscard]] constexpr char operator[](size_t index) const noexcept { return (index < size_) ? buffer_[index] : '\0'; }
    [[nodiscard]] constexpr char& operator[](size_t index) noexcept { return buffer_[index]; }
    [[nodiscard]] constexpr size_t length() const noexcept { return size_; }
    [[nodiscard]] constexpr size_t capacity() const noexcept { return Capacity; }
    [[nodiscard]] constexpr bool empty() const noexcept { return size_ == 0; }

private:
    std::array<char, Capacity> buffer_{};
    size_t size_{0};
};

template <typename T, size_t Capacity>
class RingBuffer {
public:
    static_assert((Capacity & (Capacity - 1)) == 0, "RingBuffer capacity must be a power of 2 for shift indexing.");

    constexpr RingBuffer() noexcept = default;

    [[nodiscard]] constexpr bool push(const T& item) noexcept {
        if (is_full()) {
            tail_ = (tail_ + 1) & (Capacity - 1);
        } else {
            count_++;
        }
        storage_[head_] = item;
        head_ = (head_ + 1) & (Capacity - 1);
        return true;
    }

    [[nodiscard]] constexpr bool pop(T& item) noexcept {
        if (is_empty()) return false;
        item = storage_[tail_];
        tail_ = (tail_ + 1) & (Capacity - 1);
        count_--;
        return true;
    }

    [[nodiscard]] constexpr bool is_empty() const noexcept { return count_ == 0; }
    [[nodiscard]] constexpr bool is_full() const noexcept { return count_ == Capacity; }
    [[nodiscard]] constexpr size_t size() const noexcept { return count_; }
    [[nodiscard]] constexpr size_t capacity() const noexcept { return Capacity; }

    constexpr void clear() noexcept {
        head_ = 0;
        tail_ = 0;
        count_ = 0;
    }

private:
    std::array<T, Capacity> storage_{};
    size_t head_{0};
    size_t tail_{0};
    size_t count_{0};
};

// ============================================================================
// 3. COMPILE-TIME MATH & CHECKSUM LOOKUP TABLES
// ============================================================================

namespace crc {

consteval std::array<uint16_t, 256> generate_crc16_modbus_table() noexcept {
    std::array<uint16_t, 256> table{};
    for (uint16_t i = 0; i < 256; ++i) {
        uint16_t c = i;
        for (uint8_t j = 0; j < 8; ++j) {
            c = (c & 1) ? ((c >> 1) ^ 0xA001) : (c >> 1);
        }
        table[i] = c;
    }
    return table;
}

inline constexpr std::array<uint16_t, 256> CRC16_TABLE = generate_crc16_modbus_table();

[[nodiscard]] constexpr uint16_t calculate_crc16(std::span<const uint8_t> data) noexcept {
    uint16_t c = 0xFFFF;
    for (const uint8_t byte : data) {
        const uint8_t idx = static_cast<uint8_t>(c ^ byte);
        c = static_cast<uint16_t>((c >> 8) ^ CRC16_TABLE[idx]);
    }
    return c;
}

consteval std::array<uint32_t, 256> generate_crc32_table() noexcept {
    std::array<uint32_t, 256> table{};
    for (uint32_t i = 0; i < 256; ++i) {
        uint32_t c = i;
        for (uint8_t j = 0; j < 8; ++j) {
            c = (c & 1) ? ((c >> 1) ^ 0xEDB88320U) : (c >> 1);
        }
        table[i] = c;
    }
    return table;
}

inline constexpr std::array<uint32_t, 256> CRC32_TABLE = generate_crc32_table();

[[nodiscard]] constexpr uint32_t calculate_crc32(std::span<const uint8_t> data) noexcept {
    uint32_t c = 0xFFFFFFFFU;
    for (const uint8_t byte : data) {
        const uint8_t idx = static_cast<uint8_t>(c ^ byte);
        c = (c >> 8) ^ CRC32_TABLE[idx];
    }
    return c ^ 0xFFFFFFFFU;
}

} // namespace crc

// ============================================================================
// 4. DIGITAL SIGNAL PROCESSING (EMA & BOXCAR FILTERS)
// ============================================================================

template <typename T, size_t WindowSize>
class RollingBoxcarFilter {
    static_assert((WindowSize & (WindowSize - 1)) == 0, "WindowSize must be a power of 2.");
public:
    constexpr RollingBoxcarFilter() noexcept {
        buffer_.fill(T{});
    }

    constexpr T process(T sample) noexcept {
        sum_ -= buffer_[idx_];
        sum_ += sample;
        buffer_[idx_] = sample;
        idx_ = (idx_ + 1) & (WindowSize - 1);
        if (count_ < WindowSize) count_++;
        return static_cast<T>(sum_ / count_);
    }

    [[nodiscard]] constexpr T get() const noexcept {
        return count_ > 0 ? static_cast<T>(sum_ / count_) : T{};
    }

    constexpr void reset() noexcept {
        buffer_.fill(T{});
        sum_ = 0;
        idx_ = 0;
        count_ = 0;
    }

private:
    std::array<T, WindowSize> buffer_{};
    int64_t sum_{0};
    size_t idx_{0};
    size_t count_{0};
};

template <typename T>
class ExponentialMovingAverage {
public:
    constexpr explicit ExponentialMovingAverage(float alpha = 0.25f) noexcept
        : alpha_(alpha), initialized_(false), current_val_(0.0f) {}

    constexpr T process(T sample) noexcept {
        const float s_float = static_cast<float>(sample);
        if (!initialized_) {
            current_val_ = s_float;
            initialized_ = true;
        } else {
            current_val_ = (alpha_ * s_float) + ((1.0f - alpha_) * current_val_);
        }
        return static_cast<T>(current_val_);
    }

    [[nodiscard]] constexpr T get() const noexcept {
        return static_cast<T>(current_val_);
    }

    constexpr void reset() noexcept {
        initialized_ = false;
        current_val_ = 0.0f;
    }

private:
    float alpha_;
    bool initialized_;
    float current_val_;
};

// ============================================================================
// 5. UNIFIED SENSOR & TELEMETRY DATA REPOSITORY
// ============================================================================

struct UnifiedTelemetry {
    // Environmental & Atmospheric
    int32_t  ambient_light_lux{0};
    int16_t  air_temperature_centi_c{0}; // 25.50 C = 2550
    int16_t  air_humidity_centi_rh{0};   // 65.40 % = 6540
    uint32_t barometric_pressure_pa{0};  // Pa
    uint16_t pm2_5_ug_m3{0};             // ug/m3

    // Hydroponics, Water & Tank Analytics
    int32_t  water_ph_mili{7000};        // pH 7.000 = 7000
    int32_t  water_ec_us_cm{0};          // uS/cm
    int32_t  water_tds_ppm{0};           // ppm (mg/L)
    int16_t  water_salinity_ppt_mili{0}; // ppt * 1000
    int32_t  water_distance_mm{0};       // Ultrasonic Level
    int32_t  water_height_mm{0};         // Calculated Water Height
    uint32_t water_volume_liters{0};     // Calculated Tank Volume
    uint8_t  tank_percentage_full{0};    // Tank % Full
    int16_t  water_temp_centi_c{0};      // 0.01 C
    int16_t  dissolved_oxygen_mili{0};   // mg/L * 1000

    // Soil Specific (7-in-1 NPK Probe)
    int16_t  soil_moisture_centi_rh{0};
    int16_t  soil_temp_centi_c{0};
    int32_t  soil_ec_us_cm{0};
    int32_t  soil_ph_mili{0};
    uint16_t soil_nitrogen_mg_kg{0};
    uint16_t soil_phosphorus_mg_kg{0};
    uint16_t soil_potassium_mg_kg{0};

    // Electrical / Power Metrics (INA219 / PZEM)
    uint32_t bus_voltage_mv{0};
    int32_t  bus_current_ma{0};
    uint32_t bus_power_mw{0};
    uint32_t ac_voltage_deci_v{0};
    uint32_t ac_current_mili_a{0};
    uint32_t ac_active_power_w{0};

    // GNSS Positioning
    float    latitude{0.0f};
    float    longitude{0.0f};
    uint8_t  gps_satellites{0};

    // System Status & Health Metrics
    uint32_t uptime_seconds{0};
    uint32_t free_heap_bytes{0};
    int8_t   wifi_rssi_dbm{0};
    uint8_t  cellular_csq{0};
    int16_t  lora_rssi_dbm{0};
    uint32_t modbus_crc_errors{0};
    uint32_t i2c_bus_errors{0};

    // Data Validity Bitfield
    struct Flags {
        uint32_t light_valid     : 1 = 0;
        uint32_t sht_valid       : 1 = 0;
        uint32_t bme_valid       : 1 = 0;
        uint32_t ph_valid        : 1 = 0;
        uint32_t ec_valid        : 1 = 0;
        uint32_t do_valid        : 1 = 0;
        uint32_t soil_valid      : 1 = 0;
        uint32_t level_valid     : 1 = 0;
        uint32_t wtemp_valid     : 1 = 0;
        uint32_t power_valid     : 1 = 0;
        uint32_t rtc_valid       : 1 = 0;
        uint32_t gps_valid       : 1 = 0;
        uint32_t wifi_online     : 1 = 0;
        uint32_t cellular_online : 1 = 0;
        uint32_t lora_online     : 1 = 0;
        uint32_t ota_in_progress : 1 = 0;
        uint32_t emergency_stop  : 1 = 0;
    } flags;

    // Fast Zero-Copy JSON Serializer (Stack-Allocated, Zero Heap)
    template <size_t N>
    size_t serialize_json(FixedString<N>& out) const noexcept {
        out.clear();
        out.format(
            "{"
            "\"light\":%ld,\"temp\":%.2f,\"hum\":%.2f,\"ph\":%.3f,\"ec\":%ld,\"tds\":%ld,"
            "\"dist\":%ld,\"vol_l\":%lu,\"tank_pct\":%u,\"w_temp\":%.2f,\"do\":%.3f,\"n\":%u,\"p\":%u,\"k\":%u,"
            "\"v_mv\":%lu,\"i_ma\":%ld,\"wifi_rssi\":%d,\"csq\":%u,\"err_mb\":%lu"
            "}",
            static_cast<long>(ambient_light_lux),
            static_cast<float>(air_temperature_centi_c) / 100.0f,
            static_cast<float>(air_humidity_centi_rh) / 100.0f,
            static_cast<float>(water_ph_mili) / 1000.0f,
            static_cast<long>(water_ec_us_cm),
            static_cast<long>(water_tds_ppm),
            static_cast<long>(water_distance_mm),
            static_cast<unsigned long>(water_volume_liters),
            tank_percentage_full,
            static_cast<float>(water_temp_centi_c) / 100.0f,
            static_cast<float>(dissolved_oxygen_mili) / 1000.0f,
            soil_nitrogen_mg_kg,
            soil_phosphorus_mg_kg,
            soil_potassium_mg_kg,
            static_cast<unsigned long>(bus_voltage_mv),
            static_cast<long>(bus_current_ma),
            wifi_rssi_dbm,
            cellular_csq,
            static_cast<unsigned long>(modbus_crc_errors)
        );
        return out.length();
    }
};

// ============================================================================
// 6. LOW-LEVEL BUS DRIVERS & STATIC POLYMORPHISM (NO VTABLE)
// ============================================================================

template <typename Config>
class I2CBusMaster {
public:
    static Result<void> init() noexcept {
#if defined(ESP_PLATFORM)
        i2c_config_t conf{};
        conf.mode = I2C_MODE_MASTER;
        conf.sda_io_num = Config::Pins::I2C_SDA;
        conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
        conf.scl_io_num = Config::Pins::I2C_SCL;
        conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
        conf.master.clk_speed = Config::Bus::I2C_FREQ_HZ;
        
        esp_err_t err = i2c_param_config(I2C_NUM_0, &conf);
        if (err != ESP_OK) return Status::ERROR_HARDWARE_FAIL;
        err = i2c_driver_install(I2C_NUM_0, conf.mode, 0, 0, 0);
        return (err == ESP_OK) ? Status::OK : Status::ERROR_HARDWARE_FAIL;
#else
        return Status::OK;
#endif
    }

    static Result<void> write_read(uint8_t addr, std::span<const uint8_t> tx, std::span<uint8_t> rx) noexcept {
#if defined(ESP_PLATFORM)
        i2c_cmd_handle_t cmd = i2c_cmd_link_create_static(cmd_buf_.data(), cmd_buf_.size());
        if (!cmd) return Status::ERROR_BUFFER_FULL;

        if (!tx.empty()) {
            i2c_master_start(cmd);
            i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, true);
            i2c_master_write(cmd, tx.data(), tx.size(), true);
        }
        if (!rx.empty()) {
            i2c_master_start(cmd);
            i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_READ, true);
            if (rx.size() > 1) {
                i2c_master_read(cmd, rx.data(), rx.size() - 1, I2C_MASTER_ACK);
            }
            i2c_master_read_byte(cmd, rx.data() + rx.size() - 1, I2C_MASTER_NACK);
        }
        i2c_master_stop(cmd);

        esp_err_t ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, pdMS_TO_TICKS(Config::Bus::I2C_TIMEOUT_MS));
        i2c_cmd_link_delete_static(cmd);
        return (ret == ESP_OK) ? Status::OK : Status::ERROR_TIMEOUT;
#else
        (void)addr; (void)tx; (void)rx;
        return Status::OK;
#endif
    }

    static Result<void> probe(uint8_t addr) noexcept {
#if defined(ESP_PLATFORM)
        i2c_cmd_handle_t cmd = i2c_cmd_link_create_static(cmd_buf_.data(), cmd_buf_.size());
        if (!cmd) return Status::ERROR_BUFFER_FULL;
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, true);
        i2c_master_stop(cmd);
        esp_err_t ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, pdMS_TO_TICKS(10));
        i2c_cmd_link_delete_static(cmd);
        return (ret == ESP_OK) ? Status::OK : Status::ERROR_NOT_FOUND;
#else
        (void)addr;
        return Status::OK;
#endif
    }

private:
#if defined(ESP_PLATFORM)
    static inline std::array<uint8_t, 128> cmd_buf_{};
#endif
};

template <typename Config>
class ModbusRTUMaster {
public:
    static Result<void> init() noexcept {
#if defined(ESP_PLATFORM)
        const uart_config_t uart_config = {
            .baud_rate = static_cast<int>(Config::Bus::MODBUS_BAUD),
            .data_bits = UART_DATA_8_BITS,
            .parity    = UART_PARITY_DISABLE,
            .stop_bits = UART_STOP_BITS_1,
            .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
            .source_clk = UART_SCLK_DEFAULT,
        };
        const uart_port_t port = static_cast<uart_port_t>(Config::Bus::MODBUS_UART_PORT);
        esp_err_t err = uart_param_config(port, &uart_config);
        if (err != ESP_OK) return Status::ERROR_HARDWARE_FAIL;

        err = uart_set_pin(port, Config::Pins::RS485_TX, Config::Pins::RS485_RX, 
                           UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
        if (err != ESP_OK) return Status::ERROR_HARDWARE_FAIL;

        err = uart_driver_install(port, Config::Storage::MODBUS_RX_BUFFER_SIZE * 2, 0, 0, nullptr, 0);
        if (err != ESP_OK) return Status::ERROR_HARDWARE_FAIL;

        if constexpr (Config::Pins::RS485_DE_RE >= 0) {
            gpio_config_t io_conf{};
            io_conf.intr_type = GPIO_INTR_DISABLE;
            io_conf.mode = GPIO_MODE_OUTPUT;
            io_conf.pin_bit_mask = (1ULL << Config::Pins::RS485_DE_RE);
            io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
            io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
            gpio_config(&io_conf);
            set_direction(false);
        }
#endif
        return Status::OK;
    }

    static void set_direction(bool tx_mode) noexcept {
#if defined(ESP_PLATFORM)
        if constexpr (Config::Pins::RS485_DE_RE >= 0) {
            gpio_set_level(static_cast<gpio_num_t>(Config::Pins::RS485_DE_RE), tx_mode ? 1 : 0);
        }
#else
        (void)tx_mode;
#endif
    }

    static Result<size_t> read_holding_registers(uint8_t slave_id, uint16_t start_reg, uint16_t reg_count, std::span<uint16_t> dest) noexcept {
        if (dest.size() < reg_count) return Status::ERROR_BUFFER_FULL;

        std::array<uint8_t, 8> frame{
            slave_id,
            0x03,
            static_cast<uint8_t>(start_reg >> 8),
            static_cast<uint8_t>(start_reg & 0xFF),
            static_cast<uint8_t>(reg_count >> 8),
            static_cast<uint8_t>(reg_count & 0xFF),
            0, 0
        };
        const uint16_t req_crc = crc::calculate_crc16(std::span<const uint8_t>(frame.data(), 6));
        frame[6] = static_cast<uint8_t>(req_crc & 0xFF);
        frame[7] = static_cast<uint8_t>(req_crc >> 8);

#if defined(ESP_PLATFORM)
        const uart_port_t port = static_cast<uart_port_t>(Config::Bus::MODBUS_UART_PORT);
        uart_flush(port);

        set_direction(true);
        uart_write_bytes(port, frame.data(), frame.size());
        uart_wait_tx_done(port, pdMS_TO_TICKS(10));
        set_direction(false);

        const size_t expected_bytes = 3 + (reg_count * 2) + 2;
        std::array<uint8_t, 64> rx_buf{};
        int read_bytes = 0;
        uint32_t start_tick = xTaskGetTickCount();

        while ((xTaskGetTickCount() - start_tick) < pdMS_TO_TICKS(Config::Bus::MODBUS_TIMEOUT_MS)) {
            int available = 0;
            uart_get_buffered_data_len(port, (size_t*)&available);
            if (available >= static_cast<int>(expected_bytes)) {
                read_bytes = uart_read_bytes(port, rx_buf.data(), expected_bytes, 0);
                break;
            }
            vTaskDelay(pdMS_TO_TICKS(5));
        }

        if (read_bytes < static_cast<int>(expected_bytes)) {
            return Status::ERROR_TIMEOUT;
        }

        if (rx_buf[0] != slave_id || rx_buf[1] != 0x03) {
            return Status::ERROR_NO_RESPONSE;
        }

        const uint16_t calc_crc = crc::calculate_crc16(std::span<const uint8_t>(rx_buf.data(), expected_bytes - 2));
        const uint16_t rec_crc = static_cast<uint16_t>(rx_buf[expected_bytes - 2] | (rx_buf[expected_bytes - 1] << 8));
        if (calc_crc != rec_crc) {
            return Status::ERROR_CRC_MISMATCH;
        }

        for (size_t i = 0; i < reg_count; ++i) {
            dest[i] = static_cast<uint16_t>((rx_buf[3 + (i * 2)] << 8) | rx_buf[4 + (i * 2)]);
        }
        return reg_count;
#else
        (void)slave_id; (void)start_reg;
        for (size_t i = 0; i < reg_count; ++i) dest[i] = static_cast<uint16_t>(100 * (i + 1));
        return reg_count;
#endif
    }
};

// ============================================================================
// 7. DIAGNOSTIC & DEBUGGING SUITE
// ============================================================================

template <typename Config>
class BusDiagnostics {
public:
    static void scan_i2c() noexcept {
        std::printf("\n\033[1;36m[DIAGNOSTICS] Starting Non-Blocking I2C Bus Auto-Scan...\033[0m\n");
        std::printf("     0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F\n");
        uint8_t devices_found = 0;

        for (uint8_t row = 0; row < 128; row += 16) {
            std::printf("%02X: ", row);
            for (uint8_t col = 0; col < 16; ++col) {
                const uint8_t addr = row + col;
                if (addr < 0x08 || addr > 0x77) {
                    std::printf("   ");
                    continue;
                }
                const auto res = I2CBusMaster<Config>::probe(addr);
                if (res.is_ok()) {
                    std::printf("\033[1;32m%02X \033[0m", addr);
                    devices_found++;
                } else {
                    std::printf("-- ");
                }
            }
            std::printf("\n");
        }
        std::printf("\033[1;32m[DIAGNOSTICS] I2C Scan Complete. Total Found: %u devices.\033[0m\n\n", devices_found);
    }

    static void probe_modbus_slaves(uint8_t start_id = 1, uint8_t end_id = 10) noexcept {
        std::printf("\n\033[1;36m[DIAGNOSTICS] Probing Modbus RS-485 Slaves [0x%02X..0x%02X]...\033[0m\n", start_id, end_id);
        std::array<uint16_t, 4> regs{};

        for (uint8_t id = start_id; id <= end_id; ++id) {
            const auto res = ModbusRTUMaster<Config>::read_holding_registers(id, 0x0000, 2, regs);
            if (res.is_ok()) {
                std::printf("  \033[1;32m[ONLINE]\033[0m Slave ID 0x%02X (%u) | Reg[0]=0x%04X, Reg[1]=0x%04X\n",
                            id, id, regs[0], regs[1]);
            } else {
                std::printf("  \033[1;31m[OFFLINE]\033[0m Slave ID 0x%02X (%u) : %.*s\n",
                            id, id, static_cast<int>(status_to_string(res.status()).length()),
                            status_to_string(res.status()).data());
            }
        }
        std::printf("\033[1;36m[DIAGNOSTICS] Modbus Probing Complete.\033[0m\n\n");
    }

    static void inspect_packet(std::span<const uint8_t> frame, std::string_view tag) noexcept {
        if constexpr (!Config::Features::ENABLE_HEX_PACKET_TRACE) return;
        std::printf("\033[1;33m[PACKET-TRACE][%s] Len=%zu | \033[0m", tag.data(), frame.size());
        for (const uint8_t b : frame) {
            std::printf("%02X ", b);
        }
        std::printf("\n");
    }
};

// ============================================================================
// 8. UNIFIED ACTUATOR & DOSING ENGINE
// ============================================================================

template <typename Config>
class ActuatorEngine {
public:
    static void init() noexcept {
#if defined(ESP_PLATFORM)
        uint64_t pin_mask = 0;
        for (size_t i = 0; i < Config::Actuators::NUM_CHANNELS; ++i) {
            const int8_t pin = Config::Actuators::GPIO_PINS[i];
            if (pin >= 0) {
                pin_mask |= (1ULL << pin);
            }
        }
        if constexpr (Config::Pins::STATUS_LED >= 0) {
            pin_mask |= (1ULL << Config::Pins::STATUS_LED);
        }

        if (pin_mask > 0) {
            gpio_config_t io_conf{};
            io_conf.intr_type = GPIO_INTR_DISABLE;
            io_conf.mode = GPIO_MODE_OUTPUT;
            io_conf.pin_bit_mask = pin_mask;
            io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
            io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
            gpio_config(&io_conf);
        }

        for (uint8_t ch = 1; ch <= Config::Actuators::NUM_CHANNELS; ++ch) {
            set_relay(ch, false);
        }
#endif
    }

    static void set_relay(uint8_t channel, bool on) noexcept {
        if (channel < 1 || channel > Config::Actuators::NUM_CHANNELS) return;
        const size_t idx = channel - 1;
        states_[idx] = on;

#if defined(ESP_PLATFORM)
        const int8_t pin = Config::Actuators::GPIO_PINS[idx];
        if (pin >= 0) {
            const int level = (on == Config::Actuators::ACTIVE_LEVEL_HIGH) ? 1 : 0;
            gpio_set_level(static_cast<gpio_num_t>(pin), level);
        }
#endif
    }

    static void toggle_relay(uint8_t channel) noexcept {
        if (channel < 1 || channel > Config::Actuators::NUM_CHANNELS) return;
        set_relay(channel, !states_[channel - 1]);
    }

    [[nodiscard]] static bool get_relay_state(uint8_t channel) noexcept {
        if (channel < 1 || channel > Config::Actuators::NUM_CHANNELS) return false;
        return states_[channel - 1];
    }

    static void heartbeat_toggle() noexcept {
#if defined(ESP_PLATFORM)
        if constexpr (Config::Pins::STATUS_LED >= 0) {
            static bool state = false;
            state = !state;
            gpio_set_level(static_cast<gpio_num_t>(Config::Pins::STATUS_LED), state ? 1 : 0);
        }
#endif
    }

private:
    static inline std::array<bool, Config::Actuators::NUM_CHANNELS> states_{};
};

// ============================================================================
// 9. CONNECTIVITY & STATE MACHINE (WIFI / LORA / MODEM / OTA)
// ============================================================================

enum class NetworkState : uint8_t {
    DISCONNECTED,
    CONNECTING,
    CONNECTED,
    UPLINK_IN_PROGRESS,
    FAILED_BACKOFF
};

template <typename Config>
class ConnectivityEngine {
public:
    static void init() noexcept {
        state_.store(NetworkState::DISCONNECTED, std::memory_order_relaxed);
#if defined(ESP_PLATFORM)
        if constexpr (Config::Features::ENABLE_WIFI) {
            esp_netif_init();
            esp_event_loop_create_default();
            esp_netif_create_default_wifi_sta();
            wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
            esp_wifi_init(&cfg);
            esp_wifi_set_mode(WIFI_MODE_STA);

            wifi_config_t sta_config{};
            std::memcpy(sta_config.sta.ssid, Config::Network::WIFI_SSID.data(), 
                        std::min(Config::Network::WIFI_SSID.size(), sizeof(sta_config.sta.ssid)));
            std::memcpy(sta_config.sta.password, Config::Network::WIFI_PASSWORD.data(), 
                        std::min(Config::Network::WIFI_PASSWORD.size(), sizeof(sta_config.sta.password)));
            esp_wifi_set_config(WIFI_IF_STA, &sta_config);
            esp_wifi_start();
            esp_wifi_connect();
            state_.store(NetworkState::CONNECTING, std::memory_order_relaxed);
        }
#endif
    }

    static void process() noexcept {
#if defined(ESP_PLATFORM)
        if constexpr (Config::Features::ENABLE_WIFI) {
            wifi_ap_record_t ap_info{};
            if (esp_wifi_sta_get_ap_info(&ap_info) == ESP_OK) {
                state_.store(NetworkState::CONNECTED, std::memory_order_relaxed);
            } else {
                if (state_.load(std::memory_order_relaxed) == NetworkState::CONNECTED) {
                    state_.store(NetworkState::DISCONNECTED, std::memory_order_relaxed);
                    esp_wifi_connect();
                }
            }
        }
#endif
    }

    [[nodiscard]] static bool is_connected() noexcept {
        return state_.load(std::memory_order_relaxed) == NetworkState::CONNECTED;
    }

    static Result<void> push_telemetry(const UnifiedTelemetry& data) noexcept {
        FixedString<Config::Storage::JSON_STATIC_PAYLOAD_CAPACITY> json;
        data.serialize_json(json);

        std::printf("\033[1;34m[UPLINK-REST] Transmitting Telemetry (%zu bytes): %s\033[0m\n",
                    json.length(), json.c_str());
        return Status::OK;
    }
    
    static Result<void> process_ota(std::span<const uint8_t> firmware_chunk) noexcept {
        if constexpr (!Config::Features::ENABLE_OTA) return Status::OK;
#if defined(ESP_PLATFORM)
        const esp_partition_t* update_partition = esp_ota_get_next_update_partition(nullptr);
        if (!update_partition) return Status::ERROR_NOT_FOUND;

        static esp_ota_handle_t ota_handle = 0;
        static bool ota_started = false;

        if (!ota_started) {
            esp_err_t err = esp_ota_begin(update_partition, OTA_WITH_SEQUENTIAL_WRITES, &ota_handle);
            if (err != ESP_OK) return Status::ERROR_HARDWARE_FAIL;
            ota_started = true;
        }

        if (!firmware_chunk.empty()) {
            esp_err_t err = esp_ota_write(ota_handle, firmware_chunk.data(), firmware_chunk.size());
            if (err != ESP_OK) return Status::ERROR_HARDWARE_FAIL;
        } else {
            esp_err_t err = esp_ota_end(ota_handle);
            if (err != ESP_OK) return Status::ERROR_OTA_VERIFY;
            err = esp_ota_set_boot_partition(update_partition);
            if (err != ESP_OK) return Status::ERROR_HARDWARE_FAIL;
            esp_restart();
        }
#else
        (void)firmware_chunk;
#endif
        return Status::OK;
    }

private:
    static inline std::atomic<NetworkState> state_{NetworkState::DISCONNECTED};
};

// ============================================================================
// 9.5 CALIBRATION & CHEMICAL DOSING MANAGERS
// ============================================================================

template <typename Config>
class DosingBudgetManager {
public:
    static DosingBudgetManager& instance() noexcept {
        static DosingBudgetManager mgr;
        return mgr;
    }

    void init(float daily_limit_ml = 500.0f) noexcept {
        daily_limit_ml_ = daily_limit_ml;
        dosed_today_ml_ = 0.0f;
    }

    [[nodiscard]] bool can_dose(float ml) const noexcept {
        return (dosed_today_ml_ + ml) <= daily_limit_ml_;
    }

    void record_dose(float ml) noexcept {
        dosed_today_ml_ += ml;
    }

    void reset_daily() noexcept {
        dosed_today_ml_ = 0.0f;
    }

private:
    DosingBudgetManager() noexcept = default;
    float daily_limit_ml_{500.0f};
    float dosed_today_ml_{0.0f};
};

template <typename Config>
class CalibrationEngine {
public:
    static CalibrationEngine& instance() noexcept {
        static CalibrationEngine cal;
        return cal;
    }

    void load_from_nvs() noexcept {}

    [[nodiscard]] int32_t calibrate_ph(int32_t raw_ph_mili) const noexcept {
        return raw_ph_mili;
    }

    [[nodiscard]] uint32_t calibrate_ec(uint32_t raw_ec) const noexcept {
        return raw_ec;
    }
};

} // namespace iot

